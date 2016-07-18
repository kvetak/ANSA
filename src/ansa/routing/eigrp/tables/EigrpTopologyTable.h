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

#ifndef __INET_EIGRPTOPOLOGYTABLE_H_
#define __INET_EIGRPTOPOLOGYTABLE_H_

//Ipv6 ready - mozny problem s originator (address vs. routerID)!!!

#include <omnetpp.h>

#include "inet/common/ModuleAccess.h"

#include "ansa/routing/eigrp/tables/EigrpRoute.h"
#include "ansa/routing/eigrp/tables/EigrpInterfaceTable.h"
#include "ansa/routing/eigrp/EigrpDualStack.h"
namespace inet {
/**
 * Class represents EIGRP Topology Table.
 */
template<typename IPAddress>
class EigrpTopologyTable : public cSimpleModule
{
  private:
    typedef typename std::vector<EigrpRouteSource<IPAddress> *> RouteVector;
    typedef typename std::vector<EigrpRoute<IPAddress> *> RouteInfoVector;

    RouteVector routeVec;       /**< Table with routes. */
    RouteInfoVector routeInfoVec;/**< Table with info about routes. */

    IPv4Address routerID;       /**< Router ID of this router, number represented as IPv4 address. INDEPENDENT on routed protocol (Ipv4/IPv6)! */

    int routeIdCounter;         /**< Counter for route ID */
    int sourceIdCounter;         /**< Counter for source ID */

    typename RouteVector::iterator removeRoute(typename RouteVector::iterator routeIt);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

  public:
    EigrpTopologyTable() { routeIdCounter = 1; sourceIdCounter = 1; }
    virtual ~EigrpTopologyTable();

    //-- Methods for routes
    void addRoute(EigrpRouteSource<IPAddress> *source);
    EigrpRouteSource<IPAddress> *findRoute(const IPAddress& routeAddr, const IPAddress& routeMask, const IPAddress& nextHop);
    EigrpRouteSource<IPAddress> *findRoute(const IPAddress& routeAddr, const IPAddress& routeMask, int nextHopId);
    int getNumRoutes() const { return routeVec.size(); }
    EigrpRouteSource<IPAddress> *getRoute(int k) { return routeVec[k]; }
    EigrpRouteSource<IPAddress> *removeRoute(EigrpRouteSource<IPAddress> *source);
    EigrpRouteSource<IPAddress> *findRouteById(int sourceId);
    EigrpRouteSource<IPAddress> *findRouteByNextHop(int routeId, int nextHopId);
    /**
     * Finds and returns source with given address or create one.
     * @param sourceNew return parameter, it is true if source was created. Else false.
     */
    EigrpRouteSource<IPAddress> * findOrCreateRoute(IPAddress& routeAddr, IPAddress& routeMask, IPv4Address& routerId, EigrpInterface *eigrpIface, int nextHopId, bool *sourceNew);
    /**
     * Deletes unreachable routes from the topology table.
     */
    void purgeTable();
    void delayedRemove(int neighId);


    uint64_t findRouteDMin(EigrpRoute<IPAddress> *route);
    bool hasFeasibleSuccessor(EigrpRoute<IPAddress> *route, uint64_t &resultDmin);
    /**
     * Returns best successor to the destination.
     */
    EigrpRouteSource<IPAddress> *getBestSuccessor(EigrpRoute<IPAddress> *route);
    /**
     * Returns first successor on specified interface.
     */
    EigrpRouteSource<IPAddress> *getBestSuccessorByIf(EigrpRoute<IPAddress> *route, int ifaceId);

    //-- Methods for destination networks
    int getNumRouteInfo() const { return routeInfoVec.size(); }
    EigrpRoute<IPAddress> *getRouteInfo(int k) { return routeInfoVec[k]; }
    void addRouteInfo(EigrpRoute<IPAddress> *route) { route->setRouteId(routeIdCounter); routeInfoVec.push_back(route); routeIdCounter++; }
    EigrpRoute<IPAddress> *removeRouteInfo(EigrpRoute<IPAddress> *route);
    EigrpRoute<IPAddress> *findRouteInfo(const IPAddress& routeAddr, const IPAddress& routeMask);
    EigrpRoute<IPAddress> *findRouteInfoById(int routeId);

    IPv4Address& getRouterId() { return routerID; }
    void setRouterId(IPv4Address& routerID) { this->routerID = routerID; }
};

class EigrpIpv4TopologyTable : public EigrpTopologyTable<IPv4Address>
{
//container class for IPv4TT, must exist because of Define_Module()
public:
    virtual ~EigrpIpv4TopologyTable() {};
};

class EigrpIpv6TopologyTable : public EigrpTopologyTable<IPv6Address>
{
//container class for IPv6TT, must exist because of Define_Module()
public:
    virtual ~EigrpIpv6TopologyTable() {};
};
/*
class INET_API Eigrpv4TopolTableAccess : public ModuleAccess<EigrpIpv4TopologyTable>
{
    public:
    Eigrpv4TopolTableAccess() : ModuleAccess<EigrpIpv4TopologyTable>("eigrpIpv4TopologyTable") {}
};

class INET_API Eigrpv6TopolTableAccess : public ModuleAccess<EigrpIpv6TopologyTable>
{
    public:
    Eigrpv6TopolTableAccess() : ModuleAccess<EigrpIpv6TopologyTable>("eigrpIpv6TopologyTable") {}
};
*/
}
#endif
