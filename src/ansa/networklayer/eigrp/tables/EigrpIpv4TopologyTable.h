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

#include "EigrpRoute.h"


/**
 * V tabulce muze byt vice zaznamu do stejneho cile pres ruzne next hopy. Vyhledavat se musi podle
 * souseda (co treba handle?) a adresy a masky cesty (bez masky to asi nepujde).
 */
class EigrpIpv4TopologyTable : public cSimpleModule
{
  private:
    typedef typename std::vector<EigrpRouteSource<IPv4Address> *> RouteVector;

    RouteVector routeVec;    /**< Table with routes. */

    int routeIdCounter;         /**< Counter for route ID */

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

  public:
    EigrpIpv4TopologyTable() { routeIdCounter = 1; }
    virtual ~EigrpIpv4TopologyTable();

    void addRoute(EigrpRouteSource<IPv4Address> *source);
    EigrpRouteSource<IPv4Address> *findRoute(IPv4Address& routeAddr, IPv4Address& routeMask, int nextHopId);
    int getNumRoutes() const { return routeVec.size(); }
    EigrpRouteSource<IPv4Address> *getRoute(int k) { return routeVec[k]; }
    EigrpRouteSource<IPv4Address> *removeRoute(EigrpRouteSource<IPv4Address> *source);
    EigrpRouteSource<IPv4Address> * findOrCreateRoute(IPv4Address routeAddr, IPv4Address routeMask,
            int ifaceId, int nextHopId, bool& sourceNewResult);

    uint32_t findRouteDMin(EigrpRoute<IPv4Address> *route);
    EigrpRouteSource<IPv4Address> * findSuccessor(EigrpRoute<IPv4Address> *route, uint32_t dmin);
    bool hasFeasibleSuccessor(EigrpRoute<IPv4Address> *route, uint32_t &resultDmin);
    bool checkFeasibleSuccessor(EigrpRoute<IPv4Address> *route);
    EigrpRouteSource<IPv4Address> *getFirstSuccessor(EigrpRoute<IPv4Address> *route);
};

#endif
