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

#ifndef __INET_EIGRPIPV4TOPOLOGYTABLE_H_
#define __INET_EIGRPIPV4TOPOLOGYTABLE_H_

#include <omnetpp.h>

#include "ModuleAccess.h"

#include "EigrpRoute.h"
#include "EigrpInterfaceTable.h"

/**
 * Class represents EIGRP Topology Table.
 */
class EigrpIpv4TopologyTable : public cSimpleModule
{
  private:
    typedef typename std::vector<EigrpRouteSource<IPv4Address> *> RouteVector;
    typedef typename std::vector<EigrpRoute<IPv4Address> *> RouteInfoVector;

    RouteVector routeVec;       /**< Table with routes. */
    RouteInfoVector routeInfoVec;/**< Table with info about routes. */

    IPv4Address routerID;       /**< Router ID of this router */

    int routeIdCounter;         /**< Counter for route ID */
    int sourceIdCounter;         /**< Counter for source ID */

    RouteVector::iterator removeRoute(RouteVector::iterator routeIt);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

  public:
    EigrpIpv4TopologyTable() { routeIdCounter = 1; sourceIdCounter = 1; }
    virtual ~EigrpIpv4TopologyTable();

    //-- Methods for routes
    void addRoute(EigrpRouteSource<IPv4Address> *source);
    EigrpRouteSource<IPv4Address> *findRoute(const IPv4Address& routeAddr, const IPv4Address& routeMask, const IPv4Address& nextHop);
    EigrpRouteSource<IPv4Address> *findRoute(const IPv4Address& routeAddr, const IPv4Address& routeMask, int nextHopId);
    int getNumRoutes() const { return routeVec.size(); }
    EigrpRouteSource<IPv4Address> *getRoute(int k) { return routeVec[k]; }
    EigrpRouteSource<IPv4Address> *removeRoute(EigrpRouteSource<IPv4Address> *source);
    EigrpRouteSource<IPv4Address> *findRouteById(int sourceId);
    EigrpRouteSource<IPv4Address> *findRouteByNextHop(int routeId, int nextHopId);
    /**
     * Finds and returns source with given address or create one.
     * @param sourceNew return parameter, it is true if source was created. Else false.
     */
    EigrpRouteSource<IPv4Address> * findOrCreateRoute(IPv4Address& routeAddr, IPv4Address& routeMask, IPv4Address& routerId, EigrpInterface *eigrpIface, int nextHopId, bool *sourceNew);
    /**
     * Deletes unreachable routes from the topology table.
     */
    void purgeTable();
    void delayedRemove(int neighId);


    uint64_t findRouteDMin(EigrpRoute<IPv4Address> *route);
    bool hasFeasibleSuccessor(EigrpRoute<IPv4Address> *route, uint64_t &resultDmin);
    /**
     * Returns best successor to the destination.
     */
    EigrpRouteSource<IPv4Address> *getBestSuccessor(EigrpRoute<IPv4Address> *route);
    /**
     * Returns first successor on specified interface.
     */
    EigrpRouteSource<IPv4Address> *getBestSuccessorByIf(EigrpRoute<IPv4Address> *route, int ifaceId);

    //-- Methods for destination networks
    int getNumRouteInfo() const { return routeInfoVec.size(); }
    EigrpRoute<IPv4Address> *getRouteInfo(int k) { return routeInfoVec[k]; }
    void addRouteInfo(EigrpRoute<IPv4Address> *route) { route->setRouteId(routeIdCounter); routeInfoVec.push_back(route); routeIdCounter++; }
    EigrpRoute<IPv4Address> *removeRouteInfo(EigrpRoute<IPv4Address> *route);
    EigrpRoute<IPv4Address> *findRouteInfo(const IPv4Address& routeAddr, const IPv4Address& routeMask);
    EigrpRoute<IPv4Address> *findRouteInfoById(int routeId);

    IPv4Address& getRouterId() { return routerID; }
    void setRouterId(IPv4Address& routerID) { this->routerID = routerID; }
};

class INET_API Eigrpv4TopolTableAccess : public ModuleAccess<EigrpIpv4TopologyTable>
{
    public:
    Eigrpv4TopolTableAccess() : ModuleAccess<EigrpIpv4TopologyTable>("eigrpIpv4TopologyTable") {}
};

#endif
