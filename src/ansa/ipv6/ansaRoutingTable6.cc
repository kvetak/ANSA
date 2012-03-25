//
// Marek Cerny, 2MSK
// FIT VUT 2011
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

#include <algorithm>

#include "IPv6InterfaceData.h"
#include "InterfaceTableAccess.h"

#include "ansaRoutingTable6.h"

Define_Module(AnsaRoutingTable6);

/*****************************************************************************/
/* AnsaIPv6Route class                                                       */

/*
 * Prints routing table in Cisco IOS "show ipv6 route" format
 */
std::string AnsaIPv6Route::printRoute() const {

   std::stringstream out;

   if (!isActive()){
      out << "(down) ";
   }

   // first letter describes type of the route
   // C - directly connected
   // A - own advertise prefix
   // S - static
   // N - from NDP router advertisements
   // P - routing protocol (RoutingTable6 does not have entries for specific protocols)
   switch (getSrc()){

      case STATIC:
         if (getMetric() == 0){
            out << "C";
         }else{
            out << "S";
         }
         break;

      case OWN_ADV_PREFIX:
         out << "A";
         break;

      case FROM_RA:
         out << "N";
         break;

      case ROUTING_PROT:
         out << "P";
         break;
   }

   out << "  ";
   if (getDestPrefix() == IPv6Address::UNSPECIFIED_ADDRESS){
      out << "::";
   }else{
      out << getDestPrefix();
   }

   out << "/" << getPrefixLength() << " ";
   out << "[" << getMetric() << "] ";
   out << "via ";

   if (getNextHop() == IPv6Address::UNSPECIFIED_ADDRESS){
      out << "::";
   }else{
      out << getNextHop();
   }

   if (getInterfaceId() != -1){
      out << ", " << getInterfaceName();
   }

   return out.str();
}

std::ostream& operator<<(std::ostream& os, const AnsaIPv6Route& e){
   os << e.printRoute();
   return os;
}

std::ostream& operator<<(std::ostream& os, const IPv6Route& e);


/*****************************************************************************/
/* AnsaRoutingTable6 class                                                   */

AnsaRoutingTable6::AnsaRoutingTable6(){
   routes = (std::vector<AnsaIPv6Route*> *) &routeList;
}

void AnsaRoutingTable6::initialize(int stage){

   if (stage == 1){
      ift = InterfaceTableAccess().get();
      nb = NotificationBoardAccess().get();

      if (ift == NULL){
         throw cRuntimeError("AnsaInterfaceTable not found");
      }

      if (nb == NULL){
         throw cRuntimeError("NotificationBoard not found");
      }

      nb->subscribe(this, NF_INTERFACE_STATE_CHANGED);

      WATCH_PTRVECTOR(*routes);
      WATCH_PTRVECTOR(routeList);
      WATCH_MAP(destCache);
      isrouter = par("isRouter");
      WATCH(isrouter);

      // add IPv6InterfaceData to interfaces
      for (int i = 0; i < ift->getNumInterfaces(); i++){
         InterfaceEntry *ie = ift->getInterface(i);
         configureInterfaceForIPv6(ie);
      }

      if (isrouter){
         // add globally routable prefixes to routing table
         for (int x = 0; x < ift->getNumInterfaces(); x++){
            InterfaceEntry *ie = ift->getInterface(x);

            if (ie->isLoopback())
               continue;

            for (int y = 0; y < ie->ipv6Data()->getNumAdvPrefixes(); y++){
               if (ie->ipv6Data()->getAdvPrefix(y).prefix.isGlobal()){
                  addOrUpdateOwnAdvPrefix(ie->ipv6Data()->getAdvPrefix(y).prefix, ie->ipv6Data()->getAdvPrefix(y).prefixLength, ie->getInterfaceId(), 0);
               }
            }
         }
      }
   }else if (stage == 4){
      // configurator adds routes only in stage==3
      updateDisplayString();
   }
}

void AnsaRoutingTable6::receiveChangeNotification(int category, const cPolymorphic *details){

   if (simulation.getContextType() == CTX_INITIALIZE)
      return;

   Enter_Method_Silent();

   if (category == NF_INTERFACE_STATE_CHANGED){

      InterfaceEntry *ie;
      AnsaIPv6Route *r;

      // Walk thru the whole routing table and try to get interface for each route.
      // If interface does not exist in InterfaceTable, it's down and we should
      // disable the route.
      // If interface of a disabled route does exist in InterfaceTable, it's up
      // and we should enable the route again.
      for (RouteList::const_iterator it = routeList.begin(); it != routeList.end(); it++){
         r = (AnsaIPv6Route *)(*it);
         ie = ift->getInterfaceById(r->getInterfaceId());

         if (ie == NULL && r->isActive()){
            r->setActive(false);
         }else if (ie != NULL && ie->isDown() && r->isActive()){
            r->setActive(false);
         }else if (ie != NULL && !ie->isDown() && !r->isActive()){
            r->setActive(true);
         }
      }

      std::sort(routeList.begin(), routeList.end(), routeLessThan);
      purgeDestCache();
   }
}


const IPv6Route *AnsaRoutingTable6::doLongestPrefixMatch(const IPv6Address& dest){

   Enter_Method("doLongestPrefixMatch(%s)", dest.str().c_str());

   AnsaIPv6Route *route;

   // we'll just stop at the first match, because the table is sorted
   // by prefix lengths and metric (see addRoute())
   for (RouteList::const_iterator it = routeList.begin(); it != routeList.end(); it++){
      if (dest.matches((*it)->getDestPrefix(), (*it)->getPrefixLength())){

         route = (AnsaIPv6Route *)(*it);
         if (!route->isActive()){
            return NULL;
         }

         bool entryExpired = false;
         if (simTime() > (*it)->getExpiryTime() && (*it)->getExpiryTime() != 0){
            EV<< "Expired prefix detected!!" << endl;
            removeOnLinkPrefix((*it)->getDestPrefix(), (*it)->getPrefixLength());
            entryExpired = true;
            continue;
         }else{
            return *it;
         }
      }
   }
   return NULL;
}


void AnsaRoutingTable6::addDirectRoute(const IPv6Address& destPrefix, int prefixLength, unsigned int interfaceId){

   // create route object
   IPv6Route *route = new IPv6Route(destPrefix, prefixLength, IPv6Route::STATIC);
   route->setInterfaceId(interfaceId);
   route->setNextHop(IPv6Address::UNSPECIFIED_ADDRESS);
   route->setMetric(0);

   // then add it
   addRoute(route);
}

bool AnsaRoutingTable6::routeLessThan(const IPv6Route *a, const IPv6Route *b){

   // Helper for sort() in addRoute().

   // We want routes with longer prefixes to be at front,
   // so we compare them as "less". For metric, a smaller
   // value is better (we report that as "less"). Disabled
   // routes are "more" than enabled routes.

   AnsaIPv6Route *ax = (AnsaIPv6Route *) a;
   AnsaIPv6Route *bx = (AnsaIPv6Route *) b;

   if (ax->isActive() && !bx->isActive())
      return true;
   if (!ax->isActive() && bx->isActive())
      return false;

   if (a->getPrefixLength() != b->getPrefixLength())
      return a->getPrefixLength() > b->getPrefixLength();

   return a->getMetric() < b->getMetric();
}

void AnsaRoutingTable6::addRoute(IPv6Route *route){


   AnsaIPv6Route *route6 = new AnsaIPv6Route(route->getDestPrefix(),
                                             route->getPrefixLength(),
                                             route->getSrc());

   route6->setNextHop(route->getNextHop());
   route6->setMetric(route->getMetric());
   route6->setInterfaceId(route->getInterfaceId());
   route6->setExpiryTime(route->getExpiryTime());
   route6->setActive(true);

   if (route->getInterfaceId() != -1){
      InterfaceEntry *ie = ift->getInterfaceById(route->getInterfaceId());
      if (ie != NULL){
         route6->setInterfaceName(ie->getName());
      }
   }

   delete route;
   routeList.push_back(route6);

   // we keep entries sorted by prefix length and metric in routeList,
   // so that we can stop at the first match when doing the longest prefix matching
   std::sort(routeList.begin(), routeList.end(), routeLessThan);

   updateDisplayString();

   nb->fireChangeNotification(NF_IPv6_ROUTE_ADDED, route6);
}
