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

#include "deviceConfigurator.h"

#include "IPv6Address.h"
#include "IPv6InterfaceData.h"
#include "xmlParser.h"

Define_Module(DeviceConfigurator);

using namespace std;

void DeviceConfigurator::initialize(int stage){

   // interfaces and routing table are not ready before stage 2
   if (stage == 2){

      // get table of interfaces of this device
      ift = InterfaceTableAccess().get();
      if (ift == NULL){
         throw cRuntimeError("AnsaInterfaceTable not found");
      }

      // get routing table of this device
      rt6 = AnsaRoutingTable6Access().get();
      if (rt6 == NULL){
         throw cRuntimeError("RoutingTable6 not found");
      }

      // RFC 4861 specifies that sending RAs should be disabled by default
      for (int i = 0; i < ift->getNumInterfaces(); i++){
         ift->getInterface(i)->ipv6Data()->setAdvSendAdvertisements(false);
      }


      // get access to device node from XML
      const char *deviceType = par("deviceType");
      const char *deviceId = par("deviceId");
      const char *configFile = par("configFile");

      cXMLElement *device = xmlParser::GetDevice(deviceType, deviceId, configFile);
      if (device == NULL){
         ev << "No configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
         return;
      }


      // configure interfaces - addressing
      cXMLElement *iface = xmlParser::GetInterface(NULL, device);
      if (iface == NULL){
         ev << "No interface configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
      }else{
         loadInterfaceConfig(iface);
      }


      // configure static routing
      cXMLElement *route = xmlParser::GetStaticRoute6(NULL, device);
      if (route == NULL && strcmp(deviceType, "Router") == 0){
         ev << "No static routing configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
      }else{
         loadStaticRouting(route);
      }


      // Adding default route requires routing table lookup to pick the right output
      // interface. This needs to be performed when all IPv6 addresses are already assigned
      // and there are matching records in the routing table.
      cXMLElement *gateway = device->getFirstChildWithTag("DefaultRouter");
      if (gateway == NULL && strcmp(deviceType, "Host") == 0){
         ev << "No default-router configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
      }else{
         loadDefaultRouter(gateway);
      }
   }
}


void DeviceConfigurator::loadInterfaceConfig(cXMLElement *iface){

   // for each interface node
   while (iface != NULL){

      // get interface name and find matching interface in interface table
      const char *ifaceName = iface->getAttribute("name");
      InterfaceEntry *ie = ift->getInterfaceByName(ifaceName);
      if (ie == NULL){
         throw cRuntimeError("No interface called %s on this device", ifaceName);
      }

      // for each IPv6 address
      cXMLElement *addr = xmlParser::GetIPv6Address(NULL, iface);
      while (addr != NULL){

         // get address string
         string addrFull = addr->getNodeValue();
         IPv6Address ipv6;
         int prefixLen;

         // check if it's a valid IPv6 address string with prefix and get prefix
         if (!ipv6.tryParseAddrWithPrefix(addrFull.c_str(), prefixLen)){
            throw cRuntimeError("Unable to set IPv6 address %s on interface %s", addrFull.c_str(), ifaceName);
         }

         ipv6 = addrFull.substr(0, addrFull.find_last_of('/')).c_str();

         // IPv6NeighbourDiscovery doesn't implement DAD for non-link-local addresses
         // -> we have to set the address as non-tentative
         ie->ipv6Data()->assignAddress(ipv6, false, 0, 0);

         // adding directly connected route to the routing table
         rt6->addDirectRoute(ipv6.getPrefix(prefixLen), prefixLen, ie->getInterfaceId());


         // get next IPv6 address
         addr = xmlParser::GetIPv6Address(addr, NULL);
      }


      // for each parameter
      for (cXMLElement *param = iface->getFirstChild(); param; param = param->getNextSibling()){

         if(strcmp(param->getTagName(), "NdpAdvSendAdvertisements") == 0){
            bool value = false;
            if (!xmlParser::Str2Bool(&value, param->getNodeValue())){
               throw cRuntimeError("Invalid NdpAdvSendAdvertisements value on interface %s", ie->getName());
            }
            ie->ipv6Data()->setAdvSendAdvertisements(value);
         }

         if(strcmp(param->getTagName(), "NdpMaxRtrAdvInterval") == 0){
            int value = 0;
            if (!xmlParser::Str2Int(&value, param->getNodeValue())){
               throw cRuntimeError("Unable to parse valid NdpMaxRtrAdvInterval %s on interface %s", value, ifaceName);
            }
            if (value < 4 || value > 1800){
               value = 600;
            }
            ie->ipv6Data()->setMaxRtrAdvInterval(value);
         }

         if(strcmp(param->getTagName(), "NdpMinRtrAdvInterval") == 0){
            int value = 0;
            if (!xmlParser::Str2Int(&value, param->getNodeValue())){
               throw cRuntimeError("Unable to parse valid NdpMinRtrAdvInterval %s on interface %s", value, ifaceName);
            }
            if (value < 3 || value > 1350){
               value = 200;
            }
            ie->ipv6Data()->setMinRtrAdvInterval(value);
         }
      }



      // for each IPv6 prefix
      cXMLElement *prefix = xmlParser::GetAdvPrefix(NULL, iface);
      while (prefix != NULL){

         // get address string
         string addrFull = prefix->getNodeValue();
         IPv6InterfaceData::AdvPrefix advPrefix;
         int prefixLen;

         // check if it's a valid IPv6 address string with prefix and get prefix
         if (!advPrefix.prefix.tryParseAddrWithPrefix(addrFull.c_str(), prefixLen)){
            throw cRuntimeError("Unable to parse IPv6 prefix %s on interface %s", addrFull.c_str(), ifaceName);
         }
         advPrefix.prefix = addrFull.substr(0, addrFull.find_last_of('/')).c_str();
         advPrefix.prefixLength = prefixLen;

         const char *validLifeTime = prefix->getAttribute("valid");
         const char *preferredLifeTime = prefix->getAttribute("preferred");
         int value;

         value = 2592000;
         if (validLifeTime != NULL){
            if (!xmlParser::Str2Int(&value, validLifeTime)){
               throw cRuntimeError("Unable to parse valid lifetime %s on IPv6 prefix %s on interface %s", validLifeTime, addrFull.c_str(), ifaceName);
            }
            advPrefix.advValidLifetime = value;
         }

         value = 604800;
         if (preferredLifeTime != NULL){
            if (!xmlParser::Str2Int(&value, preferredLifeTime)){
               throw cRuntimeError("Unable to parse preferred lifetime %s on IPv6 prefix %s on interface %s", preferredLifeTime, addrFull.c_str(), ifaceName);
            }
            advPrefix.advPreferredLifetime = value;
         }

         advPrefix.advOnLinkFlag = true;
         advPrefix.advAutonomousFlag = true;

         // adding prefix
         ie->ipv6Data()->addAdvPrefix(advPrefix);

         // get next IPv6 address
         prefix = xmlParser::GetAdvPrefix(prefix, NULL);
      }



      // get next interface
      iface = xmlParser::GetInterface(iface, NULL);
   }
}


void DeviceConfigurator::loadStaticRouting(cXMLElement *route){

   // for each static route
   while (route != NULL){

      // get network address string with prefix
      cXMLElement *network = route->getFirstChildWithTag("NetworkAddress");
      if (network == NULL){
         throw cRuntimeError("IPv6 network address for static route not set");
      }

      string addrFull = network->getNodeValue();
      IPv6Address addrNetwork;
      int prefixLen;

      // check if it's a valid IPv6 address string with prefix and get prefix
      if (!addrNetwork.tryParseAddrWithPrefix(addrFull.c_str(), prefixLen)){
         throw cRuntimeError("Unable to set IPv6 network address %s for static route", addrFull.c_str());
      }

      addrNetwork = addrFull.substr(0, addrFull.find_last_of('/')).c_str();


      // get IPv6 next hop address string without prefix
      cXMLElement *nextHop = route->getFirstChildWithTag("NextHopAddress");
      if (nextHop == NULL){
         throw cRuntimeError("IPv6 next hop address for static route not set");
      }

      IPv6Address addrNextHop = nextHop->getNodeValue();


      // optinal argument - administrative distance is set to 1 if not set
      cXMLElement *distance = route->getFirstChildWithTag("AdministrativeDistance");
      int adminDistance = 1;
      if (distance != NULL){
         if (!xmlParser::Str2Int(&adminDistance, distance->getNodeValue())){
            adminDistance = 0;
         }
      }

      if (adminDistance < 1 || adminDistance > 255){
         throw cRuntimeError("Invalid administrative distance for static route (%d)", adminDistance);
      }


      // current INET routing lookup implementation is not recursive
      // -> nextHop needs to be known network and we have to set output interface manually

      // browse connected routes and find one that matches next hop address
      const IPv6Route *record = rt6->doLongestPrefixMatch(addrNextHop);
      if (record == NULL){
         ev << "No directly connected route for IPv6 next hop address " << addrNextHop << " found" << endl;
      }else{
         // add static route
         rt6->addStaticRoute(addrNetwork, prefixLen, record->getInterfaceId(), addrNextHop, adminDistance);
      }


      // get next static route
      route = xmlParser::GetStaticRoute6(route, NULL);
   }
}


void DeviceConfigurator::loadDefaultRouter(cXMLElement *gateway){

   if (gateway == NULL)
      return;

   // get default-router address string (without prefix)
   IPv6Address nextHop = gateway->getNodeValue();

   // browse routing table to find the best route to default-router
   const IPv6Route *route = rt6->doLongestPrefixMatch(nextHop);
   if (route == NULL){
      return;
   }

   // add default static route
   rt6->addStaticRoute(IPv6Address::UNSPECIFIED_ADDRESS, 0, route->getInterfaceId(), nextHop, 1);
}


void DeviceConfigurator::handleMessage(cMessage *msg){
   throw cRuntimeError("This module does not receive messages");
   delete msg;
}
