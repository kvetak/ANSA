//
// Copyright (C) 2009 - today, Brno University of Technology, Czech Republic
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
/**
* @file CDPODRRouteTable.h
* @author Tomas Rajca
*/

#ifndef CDPODRROUTETABLE_H_
#define CDPODRROUTETABLE_H_

#include "ansa/linklayer/cdp/CDPTimer_m.h"
#include "inet/networklayer/contract/IRoute.h"
#include "inet/networklayer/ipv4/Ipv4RoutingTable.h"


namespace inet {

/**
 * Class holding information about a ODR route.
 */
class INET_API CDPODRRoute : public cObject
{
    friend class CDPODRRouteTable;

    private:
      InterfaceEntry *ie = nullptr;    // outgoing interface of the route
      IRoute *route = nullptr;         // the route in the host routing table that is associated with this route, may be nullptr if deleted
      L3Address dest;                  // destination of the route
      int prefixLength = 0;            // prefix length of the destination
      L3Address nextHop;               // next hop of the route

      bool invalide = false;
      bool noUpdates = false;
      simtime_t lastUpdateTime;    // time of the last change
      CDPTimer *ODRInvalideTime;
      CDPTimer *ODRHolddown;
      CDPTimer *ODRFlush;

      void createTimers();

    public:
      CDPODRRoute(IRoute *route);
      CDPODRRoute(L3Address des, int pre, L3Address nex, InterfaceEntry *i);
      virtual ~CDPODRRoute();
      virtual std::string info() const override;
      friend std::ostream& operator<<(std::ostream& os, const CDPODRRoute& e)
      {
          return os << e.info();
      }

      /**
       * delete all route timers
       */
      void deleteTimer(CDPTimer *timer);

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

/**
 * Class holding informatation about learned ODR routes.
 */
class INET_API CDPODRRouteTable : public cSimpleModule
{
protected:
  std::vector<CDPODRRoute *> routes;

  virtual void initialize(int stage) override;
  virtual void handleMessage(cMessage *) override;

public:
  virtual ~CDPODRRouteTable();

  std::vector<CDPODRRoute *>& getRoutes() {return routes;}

  /**
   * Returns the ODR route with specified destination network and next hope.
   */
  CDPODRRoute *findRoute(const L3Address& destination, int prefixLength, const L3Address& nextHope);

  /**
   * Returns the ODR route that is identified by route
   */
  CDPODRRoute *findRoute(const IRoute *route);

  /**
   * Adds the a ODR route to the table. The operation might fail
   * if route is already in the table.
   *
   * @param   route   route to add
   */
  void addRoute(CDPODRRoute * route);

  /**
   * Remove all ODR routes from the table.
   */
  void removeRoutes();

  /**
   * Remove a ODR route from the ODR table.
   * If the route was not found in the table then it is untouched,
   * otherwise deleted.
   *
   * @param   route   route to delete
   */
  void removeRoute(CDPODRRoute * route);

  /**
   * Count routes to specified network
   *
   * @param   destination
   * @param   prefixLength
   * @return  number of paths
   */
  int countDestinationPaths(const L3Address& destination, uint8_t prefixLength);

};

} /* namespace inet */

#endif /* CDPODRROUTETABLE_H_ */
