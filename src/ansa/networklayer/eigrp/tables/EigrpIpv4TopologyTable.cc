//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "IPv4Address.h"

#include <algorithm>

#include "EigrpIpv4TopologyTable.h"
#include "EigrpMessage_m.h"

#define EIGRP_TT_DEBUG

Define_Module(EigrpIpv4TopologyTable);


std::ostream& operator<<(std::ostream& os, const EigrpRouteSource<IPv4Address>& source)
{
    EigrpRoute<IPv4Address> *route = source.getRouteInfo();

    const char *state = route->isActive() ? "active" : "passive";
    //const char *source = route.isInternal()() ? "internal" : "external";

    os << "ID:" << source.getSourceId() << "  ";
    os << route->getRouteAddress() << "/" << route->getRouteMask().getNetmaskLength();
    os << "  FD:" << route->getFd();
    if (source.getNextHop().isUnspecified())
        os << "  is connected ";
    else
        os << "  via " << source.getNextHop();
    os << " (" << source.getMetric() << "/" << source.getRd() << ")";
    if (source.isSuccessor()) os << "  is successor";
    os << ",  IF ID:" << source.getIfaceId();
    os << "  state:" << state;
    //os << "  source:" << source;

    return os;
}

std::ostream& operator<<(std::ostream& os, const EigrpRoute<IPv4Address>& route)
{
    os << "ID: " << route.getRouteId() << "  ";
    os << route.getRouteAddress() << "/" << route.getRouteMask().getNetmaskLength();
    os << "  queryOrigin:" << route.getQueryOrigin();
    os << "  replyStatus:" << route.getReplyStatusSum();

    return os;
}

void EigrpIpv4TopologyTable::initialize()
{
#ifdef EIGRP_TT_DEBUG
    WATCH_PTRVECTOR(routeVec);
#endif

    WATCH_PTRVECTOR(routeInfoVec);
}

void EigrpIpv4TopologyTable::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module does not process messages");
}

EigrpIpv4TopologyTable::~EigrpIpv4TopologyTable()
{
    int cnt = routeVec.size();
    EigrpRouteSource<IPv4Address> *rt;

    for (int i = 0; i < cnt; i++)
    {
        rt = routeVec[i];
        routeVec[i] = NULL;
        delete rt;
    }
    routeVec.clear();
}

/**
 */
EigrpRouteSource<IPv4Address> *EigrpIpv4TopologyTable::findRoute(IPv4Address& routeAddr, IPv4Address& routeMask, int nextHopId)
{
    RouteVector::iterator it;
    EigrpRoute<IPv4Address> *route;

    for (it = routeVec.begin(); it != routeVec.end(); it++)
    {
        route = (*it)->getRouteInfo();
        if (route->getRouteAddress() == routeAddr && route->getRouteMask() == routeMask &&
                (*it)->getNexthopId() == nextHopId)
        {
            return *it;
        }
    }

    return NULL;
}

uint32_t EigrpIpv4TopologyTable::findRouteDMin(EigrpRoute<IPv4Address> *route)
{
    uint32_t dmin = UINT32_MAX;
    uint32_t tempD;
    RouteVector::iterator it;
    int routeId = route->getRouteId();

    for (it = routeVec.begin(); it != routeVec.end(); it++)
    {
        if ((*it)->getRouteId() == routeId /* do not check FC here */)
        {
            tempD = (*it)->getMetric();
            if (tempD < dmin)
            {
                dmin = tempD;
            }
        }
    }

    return dmin;
}

EigrpRouteSource<IPv4Address> *EigrpIpv4TopologyTable::getFirstSuccessor(EigrpRoute<IPv4Address> *route)
{
    RouteVector::iterator it;
    int routeId = route->getRouteId();

    for (it = routeVec.begin(); it != routeVec.end(); it++)
    {
        if ((*it)->getRouteId() == routeId && (*it)->isSuccessor())
        {
            return *it;
        }
    }

    return NULL;
}

/**
 * Finds feasible successor and minimal distance to the destination.
 *
 * @params resultDmin Return value with minimal distance of all FS.
 */
bool EigrpIpv4TopologyTable::hasFeasibleSuccessor(EigrpRoute<IPv4Address> *route, uint32_t& resultDmin)
{
    RouteVector::iterator it;
    int routeId = route->getRouteId();
    bool hasFs = false;
    uint32_t tempD;

    resultDmin = UINT32_MAX;
    ev << "Search feasible successor for route " << route->getRouteAddress();
    ev << ", FD is " << route->getFd() << endl;

    for (it = routeVec.begin(); it != routeVec.end(); it++)
    {
        if ((*it)->getRouteId() == routeId)
        {
            ev << "    Next hop " << (*it)->getNextHop();
            ev << " (" << (*it)->getMetric() << "/" << (*it)->getRd() << ") ";

            tempD = (*it)->getMetric();
            if (tempD < resultDmin)
            {
                hasFs = false;  // FS must have minimal distance
                resultDmin = tempD;
            }

            if ((*it)->getRd() < route->getFd() && tempD == resultDmin)
            {
                ev << "satisfies FC" << endl;
                hasFs = true;
            }
            else
                ev << "not satisfies FC" << endl;
        }
    }
    if (hasFs) ev << "     FS found, dmin is " << resultDmin << endl;
    else ev << "     FS not found, because dmin is " << resultDmin << endl;

    return hasFs;
}

void EigrpIpv4TopologyTable::addRoute(EigrpRouteSource<IPv4Address> *source)
{
    source->setSourceId(sourceIdCounter);
    sourceIdCounter++;
    this->routeVec.push_back(source);
}

/**
 * Removes neighbor form the table, but the record still exists.
 */
EigrpRouteSource<IPv4Address> *EigrpIpv4TopologyTable::removeRoute(EigrpRouteSource<IPv4Address> *source)
{
    RouteVector::iterator it;

    if ((it = std::find(routeVec.begin(), routeVec.end(), source)) != routeVec.end())
    {
        routeVec.erase(it);
        return source;
    }

    return NULL;
}

void EigrpIpv4TopologyTable::purge(int routeId)
{
    RouteVector::iterator it;
    EigrpRouteSource<IPv4Address> *source = NULL;
    EigrpRoute<IPv4Address> *route = NULL;

    for (it = routeVec.begin(); it != routeVec.end(); )
    {
        if ((*it)->getRouteId() == routeId && (*it)->isUnreachable())
        {  // Remove unreachable source from TT
            source = *it;
            route = source->getRouteInfo();

            EV << "EIGRP remove route " << route->getRouteAddress();
            EV << " via " << source->getNextHop() << " from TT" << endl;

            if (route->getRefCnt() == 1)
                removeRouteInfo(route);

            it = routeVec.erase(it);
            delete source;
        }
        else
            ++it;
    }
}

EigrpRouteSource<IPv4Address> *EigrpIpv4TopologyTable::findRouteById(int sourceId)
{
    RouteVector::iterator it;
    for (it = routeVec.begin(); it != routeVec.end(); it++)
    {
        if ((*it)->getSourceId() == sourceId)
        return *it;
    }
    return NULL;
}

EigrpRouteSource<IPv4Address> *EigrpIpv4TopologyTable::findRouteByNextHop(int routeId, int nextHopId)
{
    RouteVector::iterator it;
    for (it = routeVec.begin(); it != routeVec.end(); it++)
    {
        if ((*it)->getRouteId() == routeId && (*it)->getNexthopId() == nextHopId)
        return *it;
    }
    return NULL;
}



/**
 * @param sourceNewResult return value.
 */
EigrpRouteSource<IPv4Address> * EigrpIpv4TopologyTable::findOrCreateRoute(IPv4Address routeAddr, IPv4Address routeMask,
        int ifaceId, int nextHopId, bool *sourceNew)
{
    EigrpRoute<IPv4Address> *route = NULL;
    bool sourceFound = false;
    EigrpRouteSource<IPv4Address> *source = NULL;
    (*sourceNew) = false;

    RouteVector::iterator it;

    // Find route
    for (it = routeVec.begin(); it != routeVec.end(); it++)
    {
        source = *it;
        if (source->getRouteInfo()->getRouteAddress() == routeAddr &&
                source->getRouteInfo()->getRouteMask() == routeMask)
        {
            // Store route info
            if (route == NULL) route = source->getRouteInfo();

            if (source->getNexthopId() == nextHopId)
            { // Store TT entry
                sourceFound = true;
                break;
            }
        }
    }

    if (!sourceFound)
    { // Create route
        if (route == NULL)
        {
            route = new EigrpRoute<IPv4Address>(routeAddr, routeMask);
            addRouteInfo(route);
        }
        source = new EigrpRouteSource<IPv4Address>(ifaceId, nextHopId, route->getRouteId(), route);
        (*sourceNew) = true;
        addRoute(source);
    }

    return source;
}

EigrpRoute<IPv4Address> *EigrpIpv4TopologyTable::removeRouteInfo(EigrpRoute<IPv4Address> *route)
{
    RouteInfoVector::iterator it;

    if ((it = std::find(routeInfoVec.begin(), routeInfoVec.end(), route)) != routeInfoVec.end())
    {
        routeInfoVec.erase(it);
        return route;
    }

    return NULL;
}
