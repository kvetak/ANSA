// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#include <algorithm>

#include "opp_utils.h"

#include "ANSARoutingTable6.h"

#include "IPv6InterfaceData.h"
#include "InterfaceTableAccess.h"

#include "IPv6TunnelingAccess.h"

Define_Module(ANSARoutingTable6);

ANSAIPv6Route::ANSAIPv6Route(IPv6Address destPrefix, int length, RouteSrc src)
    : IPv6Route(destPrefix, length, src)
{
    _adminDist = Unknown;

    ift = InterfaceTableAccess().get();
}

std::string ANSAIPv6Route::info() const
{
    std::stringstream out;

    out << routeSrcName(getSrc());
    out << " " << getDestPrefix() << "/" << getPrefixLength();
    out << " [" << getAdminDist() << "/" << getMetric() << "]";
    out << " via " << getNextHop();
    out << ", " << ift->getInterfaceById(getInterfaceId())->getName();
    if (getExpiryTime()>0)
        out << " exp:" << getExpiryTime();

    return out.str();
}

std::string ANSAIPv6Route::detailedInfo() const
{
    return std::string();
}

ANSARoutingTable6::ANSARoutingTable6()
{
}

ANSARoutingTable6::~ANSARoutingTable6()
{
}

ANSAIPv6Route *ANSARoutingTable6::findRoute(const IPv6Address& prefix, int prefixLength)
{
    ANSAIPv6Route *route = NULL;
    for (RouteList::iterator it=routeList.begin(); it!=routeList.end(); it++)
    {
        if ((*it)->getDestPrefix()==prefix && (*it)->getPrefixLength()==prefixLength)
        {
            route = dynamic_cast<ANSAIPv6Route *>(*it);
            break;
        }
    }

    return route;
}

bool ANSARoutingTable6::prepareForAddRoute(ANSAIPv6Route *route)
{
    ANSAIPv6Route *routeInTable = findRoute(route->getDestPrefix(), route->getPrefixLength());
    if (routeInTable != NULL)
    {
        if (routeInTable->getAdminDist() > route->getAdminDist())
        {
            removeRoute(routeInTable);
        }
        else if(routeInTable->getAdminDist() == route->getAdminDist())
        {
            if (routeInTable->getMetric() > route->getMetric())
                removeRoute(routeInTable);
        }
        else
        {
            return false;
        }
    }

    /*XXX: this deletes some cache entries we want to keep, but the node MUST update
     the Destination Cache in such a way that all entries will use the latest
     route information.*/
    purgeDestCache();

    return true;
}

void ANSARoutingTable6::addOrUpdateOnLinkPrefix(const IPv6Address& destPrefix, int prefixLength,
        int interfaceId, simtime_t expiryTime)
{
    // see if prefix exists in table
    ANSAIPv6Route *route = NULL;
    for (RouteList::iterator it=routeList.begin(); it!=routeList.end(); it++)
    {
        if ((*it)->getSrc()==IPv6Route::FROM_RA && (*it)->getDestPrefix()==destPrefix && (*it)->getPrefixLength()==prefixLength)
        {
            route = dynamic_cast<ANSAIPv6Route *>(*it);
            break;
        }
    }

    if (route==NULL)
    {
        // create new route object
        ANSAIPv6Route *route = new ANSAIPv6Route(destPrefix, prefixLength, IPv6Route::FROM_RA);
        route->setInterfaceId(interfaceId);
        route->setExpiryTime(expiryTime);
        route->setMetric(0);
        route->setAdminDist(ANSAIPv6Route::DirectlyConnected);

        // then add it
        addRoute(route);
    }
    else
    {
        // update existing one; notification-wise, we pretend the route got removed then re-added
        nb->fireChangeNotification(NF_IPv6_ROUTE_DELETED, route);
        route->setInterfaceId(interfaceId);
        route->setExpiryTime(expiryTime);
        nb->fireChangeNotification(NF_IPv6_ROUTE_ADDED, route);
    }

    updateDisplayString();
}

void ANSARoutingTable6::addOrUpdateOwnAdvPrefix(const IPv6Address& destPrefix, int prefixLength,
        int interfaceId, simtime_t expiryTime)
{
    // FIXME this is very similar to the one above -- refactor!!

    // see if prefix exists in table
    ANSAIPv6Route *route = NULL;
    for (RouteList::iterator it=routeList.begin(); it!=routeList.end(); it++)
    {
        if ((*it)->getSrc()==IPv6Route::OWN_ADV_PREFIX && (*it)->getDestPrefix()==destPrefix && (*it)->getPrefixLength()==prefixLength)
        {
            route = dynamic_cast<ANSAIPv6Route *>(*it);
            break;
        }
    }

    if (route==NULL)
    {
        // create new route object
        ANSAIPv6Route *route = new ANSAIPv6Route(destPrefix, prefixLength, IPv6Route::OWN_ADV_PREFIX);
        route->setInterfaceId(interfaceId);
        route->setExpiryTime(expiryTime);
        route->setMetric(0);
        route->setAdminDist(ANSAIPv6Route::DirectlyConnected);

        // then add it
        addRoute(route);
    }
    else
    {
        // update existing one; notification-wise, we pretend the route got removed then re-added
        nb->fireChangeNotification(NF_IPv6_ROUTE_DELETED, route);
        route->setInterfaceId(interfaceId);
        route->setExpiryTime(expiryTime);
        nb->fireChangeNotification(NF_IPv6_ROUTE_ADDED, route);
    }

    updateDisplayString();
}

void ANSARoutingTable6::addStaticRoute(const IPv6Address& destPrefix, int prefixLength,
                    unsigned int interfaceId, const IPv6Address& nextHop,
                    int metric)
{
    // create route object
    ANSAIPv6Route *route = new ANSAIPv6Route(destPrefix, prefixLength, IPv6Route::STATIC);
    route->setInterfaceId(interfaceId);
    route->setNextHop(nextHop);
    if (metric==0)
        metric = 10; // TBD should be filled from interface metric
    route->setMetric(metric);
    route->setAdminDist(ANSAIPv6Route::Static);

    // then add it
    addRoute(route);
}

void ANSARoutingTable6::addDefaultRoute(const IPv6Address& nextHop, unsigned int ifID,
        simtime_t routerLifetime)
{
    // create route object
    ANSAIPv6Route *route = new ANSAIPv6Route(IPv6Address(), 0, IPv6Route::FROM_RA);
    route->setInterfaceId(ifID);
    route->setNextHop(nextHop);
    route->setMetric(10); //FIXME:should be filled from interface metric
    route->setAdminDist(ANSAIPv6Route::Static);

#ifdef WITH_xMIPv6
    route->setExpiryTime(routerLifetime); // lifetime useful after transitioning to new AR // 27.07.08 - CB
#endif /* WITH_xMIPv6 */

    // then add it
    addRoute(route);
}

void ANSARoutingTable6::addRoutingProtocolRoute(ANSAIPv6Route *route)
{
    ASSERT(route->getSrc()==IPv6Route::ROUTING_PROT);
    addRoute(route);
}

void ANSARoutingTable6::addRoute(ANSAIPv6Route *route)
{
    routeList.push_back(route);

    // we keep entries sorted by prefix length in routeList, so that we can
    // stop at the first match when doing the longest prefix matching
    std::sort(routeList.begin(), routeList.end(), routeLessThan);

    updateDisplayString();

    nb->fireChangeNotification(NF_IPv6_ROUTE_ADDED, route);
}

void ANSARoutingTable6::updateRoute(ANSAIPv6Route *route, int newInterfaceID, const IPv6Address &newNextHop, int newMetric)
{
    ASSERT(route != NULL);

    bool bPurgeDestCache = false;
    bool bRouteChanged = false;

    if ((newInterfaceID != route->getInterfaceId()) || (newNextHop != route->getNextHop()))
        bPurgeDestCache = true;

    if (bPurgeDestCache || (route->getMetric() != newMetric))
        bRouteChanged = true;

    route->setInterfaceId(newInterfaceID);
    route->setNextHop(newNextHop);
    route->setMetric(newMetric);

    /*XXX: this deletes some cache entries we want to keep, but the node MUST update
     the Destination Cache in such a way that all entries will use the latest
     route information.*/
    if (bPurgeDestCache)
        purgeDestCache();

    if (bRouteChanged)
        nb->fireChangeNotification(NF_IPv6_ROUTE_CHANGED, route);
}


void ANSARoutingTable6::removeRoute(ANSAIPv6Route *route)
{
    RouteList::iterator it = std::find(routeList.begin(), routeList.end(), route);
    ASSERT(it!=routeList.end());

    nb->fireChangeNotification(NF_IPv6_ROUTE_DELETED, route); // rather: going to be deleted

    routeList.erase(it);

    /*XXX: this deletes some cache entries we want to keep, but the node MUST update
     the Destination Cache in such a way that all entries using the next-hop from
     the deleted route perform next-hop determination again rather than continue
     sending traffic using that deleted route next-hop.*/
    purgeDestCache();

    delete route;

    updateDisplayString();
}

void ANSARoutingTable6::receiveChangeNotification(int category, const cObject *details)
{
    if (simulation.getContextType()==CTX_INITIALIZE)
        return;  // ignore notifications during initialize

    Enter_Method_Silent();
    printNotificationBanner(category, details);

    if (category==NF_INTERFACE_CREATED)
    {
        //TODO something like this:
        //InterfaceEntry *ie = check_and_cast<InterfaceEntry*>(details);
        //configureInterfaceForIPv6(ie);
    }
    else if (category==NF_INTERFACE_DELETED)
    {
        //TODO remove all routes that point to that interface (?)
        InterfaceEntry *interfaceEntry = check_and_cast<InterfaceEntry*>(details);
        int interfaceEntryId = interfaceEntry->getInterfaceId();
        purgeDestCacheForInterfaceID(interfaceEntryId);
    }
    else if (category==NF_INTERFACE_STATE_CHANGED)
    {
        InterfaceEntry *interfaceEntry = check_and_cast<InterfaceEntry*>(details);
        int interfaceEntryId = interfaceEntry->getInterfaceId();

        // an interface went down
        if (interfaceEntry->isDown())
        {
            purgeDestCacheForInterfaceID(interfaceEntryId);
        }
    }
    else if (category==NF_INTERFACE_CONFIG_CHANGED)
    {
        //TODO invalidate routing cache (?)
    }
    else if (category==NF_INTERFACE_IPv6CONFIG_CHANGED)
    {
        //TODO
    }
}
