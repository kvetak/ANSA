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

Define_Module(EigrpIpv4TopologyTable);


std::ostream& operator<<(std::ostream& os, const EigrpRouteSource<IPv4Address>& source)
{
    //const char *state = route.isActive() ? "active" : "passive";
    //const char *source = route.isInternal()() ? "internal" : "external";

    EigrpRoute<IPv4Address> *routeInfo = source.getRouteInfo();

    const char *conn = "Connected";

    os << routeInfo->getRouteAddress() << "/" << routeInfo->getRouteMask().getNetmaskLength();
    os << ", FD is " << routeInfo->getFd() << ", via ";
    if (source.getNextHop().isUnspecified())
        os << conn;
    else
        os << source.getNextHop();

    os << " (" << source.getMetric() << "/" << source.getRd() << "), IF ID = " << source.getIfaceId();
    if (source.isSuccessor()) os << ", is successor";

    return os;
}

void EigrpIpv4TopologyTable::initialize()
{
    WATCH_PTRVECTOR(routeVec);
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
    uint32_t fd = route->getFd();

    for (it = routeVec.begin(); it != routeVec.end(); it++)
    {
        if ((*it)->getRouteId() == routeId && (*it)->getRd() < fd)
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

EigrpRouteSource<IPv4Address> * EigrpIpv4TopologyTable::findSuccessor(EigrpRoute<IPv4Address> *route, uint32_t dmin)
{
    RouteVector::iterator it;
    EigrpRouteSource<IPv4Address> *fs;
    int routeId = route->getRouteId();

    // Find new successor
    for (it = routeVec.begin(); it != routeVec.end(); it++)
    {
        fs = *it;
        if (fs->getRouteId() == routeId && fs->getMetric() == dmin)
        {
            //newSuccessors.push_back(fs);
            return fs;
        }
    }

    return NULL;
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
 * Checks if exists feasible successor.
 *
 */
bool EigrpIpv4TopologyTable::checkFeasibleSuccessor(EigrpRoute<IPv4Address> *route)
{
    RouteVector::iterator it;
    int routeId = route->getRouteId();

    for (it = routeVec.begin(); it != routeVec.end(); it++)
    {
        if ((*it)->getRouteId() == routeId && (*it)->getRd() < route->getFd())
        {
            return true;
        }
    }

    return false;
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
    EV << "Search feasible successor for route " << route->getRouteAddress();
    EV << ", FD is " << route->getFd() << endl;

    for (it = routeVec.begin(); it != routeVec.end(); it++)
    {
        if ((*it)->getRouteId() == routeId)
        {
            EV << "    Next hop " << (*it)->getNextHop() << " (" << (*it)->getMetric();
            EV << "/" << (*it)->getRd() << ") ";

            if ((*it)->getRd() < route->getFd())
            {
                EV << "satisfies" << endl;
                hasFs = true;

                tempD = (*it)->getMetric();
                if (tempD < resultDmin)
                    resultDmin = tempD;
            }
            else
                EV << "not satisfies" << endl;
        }
    }

    return hasFs;
}

void EigrpIpv4TopologyTable::addRoute(EigrpRouteSource<IPv4Address> *source)
{
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

/**
 * @param sourceNewResult return value.
 */
EigrpRouteSource<IPv4Address> * EigrpIpv4TopologyTable::findOrCreateRoute(IPv4Address routeAddr, IPv4Address routeMask,
        int ifaceId, int nextHopId, bool& sourceNewResult)
{
    EigrpRoute<IPv4Address> *route = NULL;
    EigrpRouteSource<IPv4Address> *src = NULL;
    sourceNewResult = false;

    RouteVector::iterator it;

    // Find route
    for (it = routeVec.begin(); it != routeVec.end(); it++)
    {
        if ((*it)->getRouteInfo()->getRouteAddress() == routeAddr &&
                (*it)->getRouteInfo()->getRouteMask() == routeMask)
        {
            // Store route info
            if (route == NULL) route = (*it)->getRouteInfo();

            if ((*it)->getNexthopId() == nextHopId)
            { // Store TT entry
                src = *it;
                break;
            }
        }
    }

    if (src == NULL)
    {
        if (route == NULL)
        {
            route = new EigrpRoute<IPv4Address>(routeAddr, routeMask, routeIdCounter++);
        }

        src = new EigrpRouteSource<IPv4Address>(ifaceId, nextHopId, route->getRouteId(), route);
        sourceNewResult = true;
    }

    return src;
}
