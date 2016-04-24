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

#ifndef CDPODRROUTETABLE_H_
#define CDPODRROUTETABLE_H_

#include "ansa/linklayer/cdp/CDPTimer_m.h"
#include "inet/networklayer/contract/IRoute.h"
#include "inet/networklayer/ipv4/IPv4RoutingTable.h"


namespace inet {


class CDPODRRoute : public cObject
{
    private:
      IRoute *route = nullptr;    // the route in the host routing table that is associated with this route, may be nullptr if deleted
      L3Address dest;    // destination of the route
      int prefixLength = 0;    // prefix length of the destination
      L3Address nextHop;    // next hop of the route
      InterfaceEntry *ie = nullptr;    // outgoing interface of the route
      int ifaceId;          // because of delete interface
      bool invalide = false;    // true if the route has changed since the update
      bool noUpdates = false;    // true if the route has changed since the update
      simtime_t lastUpdateTime;    // time of the last change, only for RTE routes
      CDPTimer *ODRInvalideTime;
      CDPTimer *ODRHolddown;
      CDPTimer *ODRFlush;

      void createTimers();

    public:
      CDPODRRoute(IRoute *route);
      CDPODRRoute(L3Address des, int pre, L3Address nex, InterfaceEntry *i);
      virtual ~CDPODRRoute();
      virtual std::string info() const override;

      void deleteTimer(CDPTimer *timer);
      void removeODRRoutes();

      // getters
      IRoute *getRoute() const { return route; }
      L3Address getDestination() const { return dest; }
      int getPrefixLength() const { return prefixLength; }
      L3Address getNextHop() const { return nextHop; }
      InterfaceEntry *getInterface() const { return ie; }
      bool isInvalide() const { return invalide; }
      bool isNoUpdates() const { return noUpdates; }
      simtime_t getLastUpdateTime() const { return lastUpdateTime; }
      CDPTimer *getODRInvalideTime() const { return ODRInvalideTime; }
      CDPTimer *getODRHolddown() const { return ODRHolddown; }
      CDPTimer *getODRFlush() const { return ODRFlush; }

      // setters
      void setRoute(IRoute *route) { this->route = route; }
      void setDestination(const L3Address& dest) { this->dest = dest; }
      void setPrefixLength(int prefixLength) { this->prefixLength = prefixLength; }
      void setNextHop(const L3Address& nextHop) { this->nextHop = nextHop; route->setNextHop(nextHop); }
      void setInterface(InterfaceEntry *ie) { this->ie = ie; route->setInterface(ie); }
      void setInvalide(bool invalide) { this->invalide = invalide; }
      void setNoUpdates(bool noUpdates) { this->noUpdates = noUpdates; }
      void setLastUpdateTime(simtime_t time) { lastUpdateTime = time; }
      void setODRInvalideTime(CDPTimer *time) { ODRInvalideTime = time; }
      void setODRHolddown(CDPTimer *time) { ODRHolddown = time; }
      void setODRFlush(CDPTimer *time) { ODRFlush = time; }
};

class CDPODRRouteTable {
protected:
  std::vector<CDPODRRoute *> routes;

public:
  virtual ~CDPODRRouteTable();

  std::vector<CDPODRRoute *>& getRoutes() {return routes;}
  CDPODRRoute *findRoute(const L3Address& destination, int prefixLength, const L3Address& nextHope);
  CDPODRRoute *findRoute(const IRoute *route);
  int countDestinationPaths(const L3Address& destination, uint8_t prefixLength);
  void addRoute(CDPODRRoute * route);
  void removeRoutes();
  void removeRoute(CDPODRRoute * route);
};

} /* namespace inet */

#endif /* CDPODRROUTETABLE_H_ */
