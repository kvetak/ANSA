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
// Copyright (C) 2011 - 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
 * @file deviceConfigurator.cc
 * @author Marek Cerny, Jiri Trhlik (mailto:jiritm@gmail.com), Tomas Prochazka (mailto:xproch21@stud.fit.vutbr.cz), .. DOPLNTE Marcel Marek (mailto:xscrew02@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 2011
 * @brief
 * @detail
 * @todo Z9
 */

#include "deviceConfigurator.h"

#include "IPv6Address.h"
#include "IPv6InterfaceData.h"
#include "IPv4Address.h"
#include "xmlParser.h"

Define_Module(DeviceConfigurator);

using namespace std;

void DeviceConfigurator::initialize(int stage){

    if (stage == 0)
    {
        /* these variables needs to be set only once */
        deviceType = par("deviceType");
        deviceId = par("deviceId");
        configFile = par("configFile");
        /*
         * doesn't need to be performed anywhere else,
         * if it's NULL then behaviour depends on device type
         */
        device = xmlParser::GetDevice(deviceType, deviceId, configFile);
        if (device == NULL)
        {
            ev << "No configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
            return;
        }
    }
    else if (stage == 2)
    {// interfaces and routing table are not ready before stage 2
        // get table of interfaces of this device
        ift = InterfaceTableAccess().get();
        if (ift == NULL)
            throw cRuntimeError("InterfaceTable not found");

        //////////////////////////
        /// IPv4 Configuration ///
        //////////////////////////
        // get routing table of this device
        rt = RoutingTableAccess().getIfExists();
        if (rt != NULL)
        {
            for (int i=0; i<ift->getNumInterfaces(); ++i)
            {
                //ift->getInterface(i)->setMulticast(true);
                rt->configureInterfaceForIPv4(ift->getInterface(i));
            }

            if (device != NULL)
            {
                cXMLElement *iface = xmlParser::GetInterface(NULL, device);
                if (iface == NULL)
                    ev << "No IPv4 interface configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
                else
                    loadInterfaceConfig(iface);

                // configure static routing
                cXMLElement *route = xmlParser::GetStaticRoute(NULL, device);
                if (route == NULL && strcmp(deviceType, "Router") == 0)
                    ev << "No IPv4 static routing configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
                else
                    loadStaticRouting(route);
            }
        }

        //////////////////////////
        /// IPv6 Configuration ///
        //////////////////////////
        // get routing table of this device
        rt6 = RoutingTable6Access().getIfExists();
        if (rt6 != NULL)
        {
            // RFC 4861 specifies that sending RAs should be disabled by default
            for (int i = 0; i < ift->getNumInterfaces(); i++)
                ift->getInterface(i)->ipv6Data()->setAdvSendAdvertisements(false);

            if (device == NULL)
                return;

            // configure interfaces - addressing
            cXMLElement *iface = xmlParser::GetInterface(NULL, device);
            if (iface == NULL)
                ev << "No IPv6 interface configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
            else
                loadInterfaceConfig6(iface);

            // configure static routing
            cXMLElement *route = xmlParser::GetStaticRoute6(NULL, device);
            if (route == NULL && strcmp(deviceType, "Router") == 0)
                ev << "No IPv6 static routing configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
            else
                loadStaticRouting6(route);

            // Adding default route requires routing table lookup to pick the right output
            // interface. This needs to be performed when all IPv6 addresses are already assigned
            // and there are matching records in the routing table.
            cXMLElement *gateway = device->getFirstChildWithTag("DefaultRouter6");
            if (gateway == NULL && strcmp(deviceType, "Host") == 0)
                ev << "No IPv6 default-router configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
            else
                loadDefaultRouter6(gateway);
        }
    }
    else if(stage == 3)
    {
        if (device == NULL)
            return;

        if (xmlParser::isMulticastEnabled(device))
        {
            // get PIM interface table of this device
            pimIft = PimInterfaceTableAccess().get();
            if (pimIft == NULL)
                throw cRuntimeError("PimInterfaces not found");

            // fill pim interfaces table from config file
            cXMLElement *iface = xmlParser::GetInterface(NULL, device);
            if (iface != NULL)
            {
                loadPimInterfaceConfig(iface);

            }
            else
                EV<< "No PIM interface is configured for this device (" << deviceType << " id=" << deviceId << ")" << endl;

        }
        else
        {
            EV<< "Multicast routing is not enable for this device (" << deviceType << " id=" << deviceId << ")" << endl;
        }

    }
    else if(stage == 4)
    {
        if (device == NULL)
            return;

        //////////////////////////
        /// IPv4 Configuration ///
        //////////////////////////
        // Adding default route requires routing table lookup to pick the right output
        // interface. This needs to be performed when all IPv4 addresses are already assigned
        // and there are matching records in the routing table.
        // ANSARoutingTable: dir. conn. routes are added by deviceConfigurator in stage 2
        // "inet" RoutingTable: dir. conn. routes are added by initialite() in the RoutingTable stage 3
        cXMLElement *gateway = device->getFirstChildWithTag("DefaultRouter");
        if (gateway == NULL && strcmp(deviceType, "Host") == 0)
            ev << "No IPv4 default-router configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
        else
            loadDefaultRouter(gateway);


    }
    else if(stage == 10)
    {
        if(device == NULL)
            return;

        cXMLElement *iface = xmlParser::GetInterface(NULL, device);
        addIPv4MulticastGroups(iface);
        addIPv6MulticastGroups(iface);
    }
}

void DeviceConfigurator::loadInterfaceConfig6(cXMLElement *iface){

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

         ipv6 = IPv6Address(addrFull.substr(0, addrFull.find_last_of('/')).c_str());

         // IPv6NeighbourDiscovery doesn't implement DAD for non-link-local addresses
         // -> we have to set the address as non-tentative
         ie->ipv6Data()->assignAddress(ipv6, false, 0, 0);

         //If rt6 is ANSARoutingTable6, than overridden addStaticRoute is called and ANSAIPv6Route is added
         //else inet IPv6Route is added
         // adding directly connected route to the routing table
         rt6->addStaticRoute(ipv6.getPrefix(prefixLen), prefixLen, ie->getInterfaceId(), IPv6Address::UNSPECIFIED_ADDRESS, 0);

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
         advPrefix.prefix =  IPv6Address(addrFull.substr(0, addrFull.find_last_of('/')).c_str());
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


void DeviceConfigurator::loadStaticRouting6(cXMLElement *route){

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

      addrNetwork = IPv6Address(addrFull.substr(0, addrFull.find_last_of('/')).c_str());


      // get IPv6 next hop address string without prefix
      cXMLElement *nextHop = route->getFirstChildWithTag("NextHopAddress");
      if (nextHop == NULL){
         throw cRuntimeError("IPv6 next hop address for static route not set");
      }

      IPv6Address addrNextHop = IPv6Address(nextHop->getNodeValue());


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


void DeviceConfigurator::loadDefaultRouter6(cXMLElement *gateway){

   if (gateway == NULL)
      return;

   // get default-router address string (without prefix)
   IPv6Address nextHop;
   nextHop = IPv6Address(gateway->getNodeValue());

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

void DeviceConfigurator::loadInterfaceConfig(cXMLElement* iface)
{
    AnsaRoutingTable *ANSArt = dynamic_cast<AnsaRoutingTable *>(rt);

    while (iface != NULL)
    {
        // get interface name and find matching interface in interface table
        const char *ifaceName = iface->getAttribute("name");
        InterfaceEntry *ie = ift->getInterfaceByName(ifaceName);
        if (ie == NULL)
           throw cRuntimeError("No interface called %s on this device", ifaceName);

        std::string ifaceType = std::string(ifaceName).substr(0,3);

        //implicitne nastavenia
        if (ifaceType=="eth")
            ie->setBroadcast(true);
        if (ifaceType=="ppp")
            ie->setPointToPoint(true);

        //register multicast groups
        if (strcmp("Router", deviceType) == 0)
        {//TODO: ???
            //ie->ipv4Data()->addMulticastListener(IPv4Address("224.0.0.1"));
            //ie->ipv4Data()->addMulticastListener(IPv4Address("224.0.0.2"));
        }

        ie->ipv4Data()->setMetric(1);
        ie->setMtu(1500);
        setInterfaceParamters(ie);

        int tempNumber;

        cXMLElementList ifDetails = iface->getChildren();
        for (cXMLElementList::iterator ifElemIt = ifDetails.begin(); ifElemIt != ifDetails.end(); ifElemIt++)
        {
            std::string nodeName = (*ifElemIt)->getTagName();

            if (nodeName=="IPAddress")
                ie->ipv4Data()->setIPAddress(IPv4Address((*ifElemIt)->getNodeValue()));

            if (nodeName=="Mask")
                ie->ipv4Data()->setNetmask(IPv4Address((*ifElemIt)->getNodeValue()));

            if (nodeName=="MTU")
                ie->setMtu(atoi((*ifElemIt)->getNodeValue()));

            if (nodeName=="Bandwidth")
            { // Bandwidth in kbps
                if (!xmlParser::Str2Int(&tempNumber, (*ifElemIt)->getNodeValue()))
                    throw cRuntimeError("Bad value for bandwidth on interface %s", ifaceName);
                ie->setBandwidth(tempNumber);
            }

            if (nodeName=="Delay")
            { // Delay in tens of microseconds
                if (!xmlParser::Str2Int(&tempNumber, (*ifElemIt)->getNodeValue()))
                    throw cRuntimeError("Bad value for delay on interface %s", ifaceName);
                ie->setDelay(tempNumber * 10);
            }
        }

        //Add directly connected routes to the ANSA rt version only
        //--- inet routing table adds dir. conn. routes in its own initialize method
        if (ANSArt != NULL)
        {
            ANSAIPv4Route *route = new ANSAIPv4Route();
            route->setSource(IPv4Route::IFACENETMASK);
            route->setDestination(ie->ipv4Data()->getIPAddress().doAnd(ie->ipv4Data()->getNetmask()));
            route->setNetmask(ie->ipv4Data()->getNetmask());
            route->setGateway(IPv4Address());
            route->setMetric(ie->ipv4Data()->getMetric());
            route->setAdminDist(ANSAIPv4Route::dDirectlyConnected);
            route->setInterface(ie);

            ANSArt->addRoute(route);
        }

        iface = xmlParser::GetInterface(iface, NULL);
    }
}

void DeviceConfigurator::setInterfaceParamters(InterfaceEntry *interface)
{
    int gateId = interface->getNodeOutputGateId();
    cModule *host = findContainingNode(interface->getInterfaceModule());
    cGate *outGw;
    cChannel *link;

    if (host != NULL && gateId != -1)
    { // Get link type
        outGw = host->gate(gateId);
        if ((link = outGw->getChannel()) == NULL)
            return;

        const char *linkType = link->getChannelType()->getName();
        cPar bwPar = link->par("datarate");

        // Bandwidth in kbps
        interface->setBandwidth(bwPar.doubleValue() / 1000);
        interface->setDelay(getDefaultDelay(linkType));
    }
}

double DeviceConfigurator::getDefaultDelay(const char *linkType)
{
    if (!strcmp(linkType, "Eth10M"))
        return 1000;
    if (!strcmp(linkType, "Eth100M"))
        return 100;
    if (!strcmp(linkType, "Eth1G"))
        return 10;
    if (!strcmp(linkType, "Eth10G"))
        return 10;
    if (!strcmp(linkType, "Eth40G"))
        return 10;
    if (!strcmp(linkType, "Eth100G"))
        return 10;

    return 1000;    // ethernet 10M
}

void DeviceConfigurator::loadStaticRouting(cXMLElement* route)
{
    AnsaRoutingTable *ANSArt = dynamic_cast<AnsaRoutingTable *>(rt);

    while (route != NULL)
    {
        ANSAIPv4Route *ANSAStaticRoute = new ANSAIPv4Route();

        cXMLElementList ifDetails = route->getChildren();
        for (cXMLElementList::iterator routeElemIt = ifDetails.begin(); routeElemIt != ifDetails.end(); routeElemIt++)
        {
            std::string nodeName = (*routeElemIt)->getTagName();

            if (nodeName=="NetworkAddress")
            {
                ANSAStaticRoute->setDestination(IPv4Address((*routeElemIt)->getNodeValue()));
            }
            else if (nodeName=="NetworkMask")
            {
                ANSAStaticRoute->setNetmask(IPv4Address((*routeElemIt)->getNodeValue()));
            }
            else if (nodeName=="NextHopAddress")
            {
                ANSAStaticRoute->setGateway(IPv4Address((*routeElemIt)->getNodeValue()));
                InterfaceEntry *intf=NULL;
                for (int i=0; i<ift->getNumInterfaces(); i++)
                {
                    intf = ift->getInterface(i);
                    if ( ((intf->ipv4Data()->getIPAddress()).doAnd(intf->ipv4Data()->getNetmask())) ==
                         ((ANSAStaticRoute->getGateway()).doAnd(intf->ipv4Data()->getNetmask())) )
                        break;
                }

                if (intf)
                    ANSAStaticRoute->setInterface(intf);
                else
                    throw cRuntimeError("No exit interface found for the static route %s next hop.", (*routeElemIt)->getNodeValue());

                ANSAStaticRoute->setMetric(1);
            }
            else if (nodeName=="ExitInterface")
            {
                InterfaceEntry *ie=ift->getInterfaceByName((*routeElemIt)->getNodeValue());
                if (!ie)
                    throw cRuntimeError("Interface %s does not exists.", (*routeElemIt)->getNodeValue());

                ANSAStaticRoute->setInterface(ie);
                ANSAStaticRoute->setGateway(IPv4Address::UNSPECIFIED_ADDRESS);
                ANSAStaticRoute->setMetric(0);
            }
            else if (nodeName=="StaticRouteMetric")
            {
                ANSAStaticRoute->setMetric(atoi((*routeElemIt)->getNodeValue()));
            }
        }

        ANSAStaticRoute->setSource(IPv4Route::MANUAL);
        ANSAStaticRoute->setAdminDist(ANSAIPv4Route::dDirectlyConnected);

        //To the ANSA RoutingTable add ANSAIPv4Route, to the inet RoutingTable add IPv4Route
        if (ANSArt != NULL)
        {
            rt->addRoute(ANSAStaticRoute);
        }
        else
        {
            IPv4Route *staticRoute = new IPv4Route();
            staticRoute->setSource(ANSAStaticRoute->getSource());
            staticRoute->setDestination(ANSAStaticRoute->getDestination());
            staticRoute->setNetmask(ANSAStaticRoute->getNetmask());
            staticRoute->setGateway(ANSAStaticRoute->getGateway());
            staticRoute->setInterface(ANSAStaticRoute->getInterface());
            staticRoute->setMetric(ANSAStaticRoute->getMetric());

            rt->addRoute(staticRoute);
            delete ANSAStaticRoute;
        }

        route = xmlParser::GetStaticRoute(route, NULL);
    }
}

void DeviceConfigurator::loadDefaultRouter(cXMLElement *gateway)
{
    if (gateway == NULL)
      return;

    // get default-router address string (without prefix)
    IPv4Address nextHop;
    nextHop = IPv4Address(gateway->getNodeValue());

    // browse routing table to find the best route to default-router
    const IPv4Route *route = rt->findBestMatchingRoute(nextHop);
    if (route == NULL)
      return;

    AnsaRoutingTable *ANSArt = dynamic_cast<AnsaRoutingTable *>(rt);

    //To the ANSA RoutingTable add ANSAIPv4Route, to the inet RoutingTable add IPv4Route
    if (ANSArt != NULL)
    {
        ANSAIPv4Route *defaultRoute = new ANSAIPv4Route();
        defaultRoute->setSource(IPv4Route::MANUAL);
        defaultRoute->setDestination(IPv4Address());
        defaultRoute->setNetmask(IPv4Address());
        defaultRoute->setGateway(nextHop);
        defaultRoute->setInterface(route->getInterface());
        defaultRoute->setMetric(0);
        defaultRoute->setAdminDist(ANSAIPv4Route::dStatic);

        rt->addRoute(defaultRoute);
    }
    else
    {
        IPv4Route *defaultRoute = new IPv4Route();
        defaultRoute->setSource(IPv4Route::MANUAL);
        defaultRoute->setDestination(IPv4Address());
        defaultRoute->setNetmask(IPv4Address());
        defaultRoute->setGateway(nextHop);
        defaultRoute->setInterface(route->getInterface());
        defaultRoute->setMetric(0);

        rt->addRoute(defaultRoute);
    }
}

//
//
//- configuration for RIPng
//
//
void DeviceConfigurator::loadRIPngConfig(RIPngRouting *RIPngModule)
{
    ASSERT(RIPngModule != NULL);

    // get access to device node from XML
    const char *deviceType = par("deviceType");
    const char *deviceId = par("deviceId");
    const char *configFile = par("configFile");
    cXMLElement *device = xmlParser::GetDevice(deviceType, deviceId, configFile);

    if (device == NULL)
    {
        ev << "No RIPng configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
             return;
    }

    ev << "Configuring RIPng on this device (" << deviceType << " id=" << deviceId << ")" << endl;

    const char *RIPngProcessName;
    const char *RIPngParamString;
    RIPngProcess *RIPngProcess;
    cXMLElement *RIPngProcessTimersElement;

      cXMLElement *RIPngProcessElement = xmlParser::GetRIPngProcess(NULL, device);
      if (RIPngProcessElement == NULL)
          ev << "   No RIPng configuration found." << endl;

      while(RIPngProcessElement != NULL)
      {
          RIPngProcessName = RIPngProcessElement->getAttribute("name");
          if (RIPngProcessName == NULL)
          {
              ev << "   RIPng process name not found." << endl;
          }
          else
          {
              RIPngProcess = RIPngModule->addProcess(RIPngProcessName);

              RIPngParamString = xmlParser::GetNodeParamConfig(RIPngProcessElement, "Port", "");
              if (strcmp(RIPngParamString, "") != 0)
              {// set port and address
                  int port;
                  if (xmlParser::Str2Int(&port, RIPngParamString))
                  {
                      RIPngParamString = xmlParser::GetNodeParamConfig(RIPngProcessElement, "Address", "");
                      if (strcmp(RIPngParamString, "") != 0)
                      {
                          IPv6Address RIPngAddress = IPv6Address(RIPngParamString);
                          RIPngModule->setPortAndAddress(RIPngProcess, port, RIPngAddress);
                      }
                  }
              }

              RIPngParamString = xmlParser::GetNodeParamConfig(RIPngProcessElement, "Distance", "");
              if (strcmp(RIPngParamString, "") != 0)
              {// set distance (AD)
                  int distance;
                  if (xmlParser::Str2Int(&distance, RIPngParamString))
                  {
                      RIPngModule->setDistance(RIPngProcess, distance);
                  }
              }

              RIPngProcessTimersElement = xmlParser::GetRIPngProcessTimers(RIPngProcessElement);
              if (RIPngProcessTimersElement != NULL)
              {// set timers
                  int update, route, garbage;
                  RIPngParamString = xmlParser::GetNodeParamConfig(RIPngProcessTimersElement, "Update", "-1");
                  if (!xmlParser::Str2Int(&update, RIPngParamString))
                      update = -1;
                  RIPngParamString = xmlParser::GetNodeParamConfig(RIPngProcessTimersElement, "Route", "-1");
                  if (!xmlParser::Str2Int(&route, RIPngParamString))
                      route = -1;
                  RIPngParamString = xmlParser::GetNodeParamConfig(RIPngProcessTimersElement, "Garbage", "-1");
                  if (!xmlParser::Str2Int(&garbage, RIPngParamString))
                      garbage = -1;

                  RIPngModule->setTimers(RIPngProcess, update, route, garbage);
              }

              //Must be set before any INTERFACE PoisonReverse command
              RIPngParamString = xmlParser::GetNodeParamConfig(RIPngProcessElement, "PoisonReverse", "");
              if (strcmp(RIPngParamString, "enable") == 0)
              {// enable Poison Reverse (Poison Reverse is disabled by default)
                  RIPngModule->setPoisonReverse(RIPngProcess, true);
              }

              //Must be set before any INTERFACE SplitHorizon command
              RIPngParamString = xmlParser::GetNodeParamConfig(RIPngProcessElement, "SplitHorizon", "");
              if (strcmp(RIPngParamString, "disable") == 0)
              {// // disable Split Horizon (Split Horizon is enabled by default)
                  RIPngModule->setSplitHorizon(RIPngProcess, false);
              }
          }

          RIPngProcessElement = xmlParser::GetRIPngProcess(RIPngProcessElement, NULL);
      }

    // interfaces config
    cXMLElement *interfaceElement;
    cXMLElement *interfaceRIPngProcessElement;
    cXMLElement *interfaceRIPngDefaultInfElement;
    const char *interfaceRIPngProcessName;
    const char *interfaceRIPngParamString;
    RIPng::Interface *ripngInterface;

      //get first router's interface
      interfaceElement = xmlParser::GetInterface(NULL, device);
      while (interfaceElement != NULL)
      {// process all interfaces in config file
          const char *interfaceName = interfaceElement->getAttribute("name");
          interfaceRIPngProcessElement = xmlParser::GetInterfaceRIPngProcess(NULL, interfaceElement);
          while (interfaceRIPngProcessElement != NULL)
          {
              interfaceRIPngProcessName = interfaceRIPngProcessElement->getAttribute("name");

              if (interfaceRIPngProcessName != NULL && strcmp(interfaceRIPngProcessName, "") != 0)
              {
                  ripngInterface = RIPngModule->enableRIPngOnInterface(interfaceRIPngProcessName, interfaceName);
                  if (ripngInterface == NULL)
                  {
                      ev << "   Interface with name " << interfaceName << " not found." << endl;
                      continue;
                  }

                  RIPngProcess = ripngInterface->getProcess();

                  // add prefixes from int to the RIPng routing table
                  loadPrefixesFromInterfaceToRIPngRT(RIPngProcess, interfaceElement);

                  interfaceRIPngParamString = xmlParser::GetInterfaceRIPngPassiveStatus(interfaceRIPngProcessElement);
                  if (strcmp(interfaceRIPngParamString, "enable") == 0)
                  {// set the interface as passive (interface is "active" by default)
                      RIPngProcess->setInterfacePassiveStatus(ripngInterface, true);
                  }

                  interfaceRIPngParamString = xmlParser::GetInterfaceRIPngSplitHorizon(interfaceRIPngProcessElement);
                  if (strcmp(interfaceRIPngParamString, "disable") == 0)
                  {// disable Split Horizon on the interface (Split Horizon is enabled by default)
                      RIPngProcess->setInterfaceSplitHorizon(ripngInterface, false);
                  }

                  interfaceRIPngParamString = xmlParser::GetInterfaceRIPngPoisonReverse(interfaceRIPngProcessElement);
                  if (strcmp(interfaceRIPngParamString, "enable") == 0)
                  {// enable Poison Reverse on the interface (Poison Reverse is disabled by default)
                      RIPngProcess->setInterfacePoisonReverse(ripngInterface, true);
                  }

                  int metricOffset;
                  interfaceRIPngParamString = xmlParser::GetInterfaceRIPngMetricOffset(interfaceRIPngProcessElement);
                  if (xmlParser::Str2Int(&metricOffset, interfaceRIPngParamString) && metricOffset != 0)
                  {// metric-offset
                      RIPngProcess->setInterfaceMetricOffset(ripngInterface, metricOffset);
                  }

                  interfaceRIPngDefaultInfElement = xmlParser::GetInterfaceRIPngDefaultInformation(interfaceRIPngProcessElement);
                  if (interfaceRIPngDefaultInfElement != NULL)
                  {// default-information
                      bool defRouteOnly = false;
                      interfaceRIPngParamString = xmlParser::GetNodeParamConfig(interfaceRIPngDefaultInfElement, "DefaultOnly", "false");
                      xmlParser::Str2Bool(&defRouteOnly, interfaceRIPngParamString);

                      int metric = 0;
                      interfaceRIPngParamString = xmlParser::GetNodeParamConfig(interfaceRIPngDefaultInfElement, "Metric", "0");
                      xmlParser::Str2Int(&metric, interfaceRIPngParamString);

                      RIPngProcess->setInterfaceDefaultInformation(ripngInterface, true, defRouteOnly, metric);
                  }

              }

              // process next RIPng on the interface
              interfaceRIPngProcessElement = xmlParser::GetInterfaceRIPngProcess(interfaceRIPngProcessElement, NULL);

          }

          // process next interface
          interfaceElement = xmlParser::GetInterface(interfaceElement, NULL);
      }
}

void DeviceConfigurator::loadPrefixesFromInterfaceToRIPngRT(RIPngProcess *process, cXMLElement *interface)
{
    const char *interfaceName = interface->getAttribute("name");
    InterfaceEntry *interfaceEntry = ift->getInterfaceByName(interfaceName);
    int interfaceId = interfaceEntry->getInterfaceId();

    RIPng::RoutingTableEntry *route;

    // process next interface
    cXMLElement *eIpv6address;
    std::string sIpv6address;
    IPv6Address ipv6address;
    int prefixLen;
        // get first ipv6 address
        eIpv6address = xmlParser::GetIPv6Address(NULL, interface);
        while (eIpv6address != NULL)
        {// process all ipv6 addresses on the interface
            sIpv6address = eIpv6address->getNodeValue();

            // check if it's a valid IPv6 address string with prefix and get prefix
            if (!ipv6address.tryParseAddrWithPrefix(sIpv6address.c_str(), prefixLen))
            {
               throw cRuntimeError("Unable to set IPv6 network address %s for RIPng route", sIpv6address.c_str());
            }

            if (!ipv6address.isLinkLocal() && !ipv6address.isMulticast() && !ipv6address.isUnspecified())
            {
                // make directly connected route
                route = new RIPng::RoutingTableEntry(ipv6address.getPrefix(prefixLen), prefixLen);
                route->setProcess(process);
                route->setInterfaceId(interfaceId);
                route->setNextHop(IPv6Address::UNSPECIFIED_ADDRESS);  // means directly connected network
                route->setMetric(process->getConnNetworkMetric());
                process->addRoutingTableEntry(route, false);

                // directly connected routes do not need a RIPng route timer
            }

            eIpv6address = xmlParser::GetIPv6Address(eIpv6address, NULL);
        }
}


void DeviceConfigurator::loadRIPConfig(RIPRouting *RIPModule)
{
    ASSERT(RIPModule != NULL);

    // get access to device node from XML
    const char *deviceType = par("deviceType");
    const char *deviceId = par("deviceId");
    const char *configFile = par("configFile");
    cXMLElement *device = xmlParser::GetDevice(deviceType, deviceId, configFile);

    if (device == NULL){
       ev << "No configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
            return;
    }

    // interfaces config
    cXMLElement *networkElement;
    const char *networkString;

    RIP::Interface *RIPInterface;

    int numInterfaces = ift->getNumInterfaces();
    InterfaceEntry *interface;
    int interfaceId;
    const char *interfaceName;

    //RIP NETWORK COMMAND
    networkElement = xmlParser::GetRIPNetwork(NULL, device);
    while (networkElement != NULL)
    {// process all RIP Network "command"

        networkString = networkElement->getNodeValue();
        if (networkString == NULL)
            continue;

        IPv4Address network = IPv4Address(networkString);

        for (int i = 0; i < numInterfaces; ++i)
        {
            interface = ift->getInterface(i);
            IPv4Address interfaceAddress = interface->ipv4Data()->getIPAddress();
            if (interfaceAddress.isUnspecified())
                continue;

            interfaceId = interface->getInterfaceId();
            //isUnspecified -> network = 0.0.0.0 -> all interfaces
            if (network.isUnspecified() || network.isNetwork(interfaceAddress))
            {// IP address of the interface match the network
                //check if the interface is not in the rip interfaces
                if (RIPModule->getEnabledInterfaceIndexById(interfaceId) == -1)
                {
                    RIPInterface = RIPModule->enableRIPOnInterface(interfaceId);
                    // add networks from the interface to the RIP database
                    loadNetworksFromInterfaceToRIPRT(RIPModule, interface);
                }
            }
        }

        //next Network element
        networkElement = xmlParser::GetRIPNetwork(networkElement, NULL);
    }

    //PASSIVE-INTERFACE
    cXMLElement *passiveInterfaceElem;
    passiveInterfaceElem = xmlParser::GetRIPPassiveInterface(NULL, device);
    while(passiveInterfaceElem != NULL)
    {
        RIPInterface = RIPModule->getEnabledInterfaceByName(passiveInterfaceElem->getNodeValue());
        RIPModule->setInterfacePassiveStatus(RIPInterface, true);

        // next passive-interface command
        passiveInterfaceElem = xmlParser::GetRIPPassiveInterface(passiveInterfaceElem, NULL);
    }

    //RIP PER INTERFACE CONFIGURATION
    std::string RIPInterfaceSplitHorizon;
    std::string RIPInterfacePoisonReverse;

    cXMLElement *interfaceElem;
    interfaceElem = xmlParser::GetInterface(NULL, device);
    while (interfaceElem != NULL)
    {
        interfaceName = interfaceElem->getAttribute("name");
        RIPInterfaceSplitHorizon = xmlParser::GetRIPInterfaceSplitHorizon(interfaceElem);
        RIPInterface = RIPModule->getEnabledInterfaceByName(interfaceName);

        if (RIPInterface != NULL)
        {
            if (RIPInterfaceSplitHorizon == "disable")
            {// disable Split Horizon on the interface (Split Horizon is enabled by default)
                RIPModule->setInterfaceSplitHorizon(RIPInterface, false);
            }

            RIPInterfacePoisonReverse = xmlParser::GetRIPInterfacePoisonReverse(interfaceElem);
            if (RIPInterfacePoisonReverse == "enable")
            {// enable Poison Reverse on the interface (Poison Reverse is disabled by default)
                RIPModule->setInterfacePoisonReverse(RIPInterface, true);
            }
        }

        //next interface
        interfaceElem = xmlParser::GetInterface(interfaceElem, NULL);
    }
}

void DeviceConfigurator::loadNetworksFromInterfaceToRIPRT(RIPRouting *RIPModule, InterfaceEntry *interface)
{
    // make directly connected route
    RIP::RoutingTableEntry *route = new RIP::RoutingTableEntry(interface->ipv4Data()->getIPAddress().doAnd(interface->ipv4Data()->getNetmask()),
                                                               interface->ipv4Data()->getNetmask());
    route->setInterface(interface);
    route->setGateway(IPv4Address::UNSPECIFIED_ADDRESS);  // means directly connected network
    route->setMetric(RIPModule->getConnNetworkMetric());
    RIPModule->addRoutingTableEntry(route, false);

    // directly connected routes do not need a RIPng route timer
}

void DeviceConfigurator::loadPimGlobalConfig(pimSM *pimSMModule)
{
    ASSERT(pimSMModule != NULL);

    // get access to device node from XML
    const char *deviceType = par("deviceType");
    const char *deviceId = par("deviceId");
    const char *configFile = par("configFile");
    cXMLElement *device = xmlParser::GetDevice(deviceType, deviceId, configFile);

    // get global pim element
    cXMLElement* pimGlobal = xmlParser::GetPimGlobal(device);
    if (pimGlobal != NULL)
    {
        //RP address
        cXMLElement* RP = pimGlobal->getElementByPath("RPAddress");
        if (RP != NULL)
        {
            cXMLElement * IPaddress = RP->getElementByPath("IPAddress");
            if (IPaddress != NULL)
            {
                std::string IPString = IPaddress->getNodeValue();
                pimSMModule->setRPAddress(IPString);
                EV << "IP address in config for RP found: " << IPString.c_str() << endl;
            }

        }
        //SPTthreshold
        cXMLElement* SPTthreshold = pimGlobal->getElementByPath("SPTthreshold");
        if (SPTthreshold != NULL)
        {
            std::string SPTthresholdString = SPTthreshold->getNodeValue();
            pimSMModule->setSPTthreshold(SPTthresholdString);
            EV << "SPTthreshold in config found: " << SPTthresholdString.c_str() << endl;
        }
    }
}

void DeviceConfigurator::loadPimInterfaceConfig(cXMLElement *iface)
{
    // for each interface node
    while (iface != NULL)
    {
        // get PIM node
        cXMLElement* pimNode = iface->getElementByPath("Pim");
        if (pimNode == NULL)
          break;                //FIXME it is break ok?

        // create new PIM interface
        PimInterface newentry;
        InterfaceEntry *interface = ift->getInterfaceByName(iface->getAttribute("name"));
        newentry.setInterfaceID(interface->getInterfaceId());
        newentry.setInterfacePtr(interface);

        // get PIM mode for interface
        cXMLElement* pimMode = pimNode->getElementByPath("Mode");
        if (pimMode == NULL)
          continue;

        const char *mode = pimMode->getNodeValue();
        //EV << "PimSplitter::PIM interface mode = "<< mode << endl;
        if (!strcmp(mode, "dense-mode"))
          newentry.setMode(Dense);
        else if (!strcmp(mode, "sparse-mode"))
          newentry.setMode(Sparse);
        else
          continue;

        // get PIM mode for interface
        cXMLElement* stateRefreshMode = pimNode->getElementByPath("StateRefresh");
        if (stateRefreshMode != NULL)
        {
            EV << "Nasel State Refresh" << endl;
            // will router send state refresh msgs?
            cXMLElement* origMode = stateRefreshMode->getElementByPath("OriginationInterval");
            if (origMode != NULL)
            {
                EV << "Nasel Orig" << endl;
                newentry.setSR(true);
            }
        }

        // register pim multicast address 224.0.0.13 (all PIM routers) on Pim interface
        std::vector<IPv4Address> intMulticastAddresses;

        //FIXME only for PIM-DM testing purposes
        cXMLElement* IPaddress = iface->getElementByPath("IPAddress");                  //Register 226.1.1.1 to R2 router
        std::string intfToRegister = IPaddress->getNodeValue();

        if (intfToRegister == "192.168.12.2" || intfToRegister == "192.168.22.2")
                interface->ipv4Data()->addMulticastListener(IPv4Address("226.1.1.1"));

        interface->ipv4Data()->addMulticastListener(IPv4Address("224.0.0.13"));
        intMulticastAddresses = interface->ipv4Data()->getReportedMulticastGroups();

        for(unsigned int i = 0; i < (intMulticastAddresses.size()); i++)
            EV << intMulticastAddresses[i] << ", ";
        EV << endl;

        newentry.setIntMulticastAddresses(newentry.deleteLocalIPs(intMulticastAddresses));

        // add new entry to pim interfaces table
        pimIft->addInterface(newentry);

        // get next interface
        iface = xmlParser::GetInterface(iface, NULL);
    }
}


/* IS-IS related methods */

void DeviceConfigurator::loadISISConfig(ISIS *isisModule, ISIS::ISIS_MODE isisMode){

    /* init module pointers based on isisMode */

    if(isisMode == ISIS::L2_ISIS_MODE){
        //TRILL
        isisModule->setTrill(TRILLAccess().get());


        //RBridgeSplitter


    }else if(isisMode == ISIS::L3_ISIS_MODE){
        //IPv4
        //TODO C2

        //IPv6
        //TODO C2

    }else{
        throw cRuntimeError("Unsupported IS-IS mode");
    }

    //CLNSTable must be present in both modes
    isisModule->setClnsTable(CLNSTableAccess().get());

    //InterfaceTable
    isisModule->setIft(InterfaceTableAccess().get());

    //NotificationBoard
    isisModule->setNb(NotificationBoardAccess().get());
    isisModule->subscribeNb();

    /* end of module's pointers init */



    if(device == NULL){
        if(isisMode == ISIS::L3_ISIS_MODE){
            /* In L3 mode we need configuration (at least NET) */
            throw cRuntimeError("No configuration found for this device");
        }else{
            /* For L2 mode we load defaults ...
             * ... repeat after me zero-configuration. */
            this->loadISISCoreDefaultConfig(isisModule);
            this->loadISISInterfacesConfig(isisModule);
        }
        return;
    }

    cXMLElement *isisRouting = xmlParser::GetIsisRouting(device);
    if (isisRouting == NULL)
    {
        if(isisMode == ISIS::L3_ISIS_MODE){
            throw cRuntimeError("Can't find ISISRouting in config file");
        }
    }


    if (isisRouting != NULL)
    {
        //NET
        //TODO: multiple NETs for migrating purposes (merging, splitting areas)
        const char *netAddr = this->getISISNETAddress(isisRouting);
        if (netAddr != NULL)
        {
            isisModule->setNetAddr(netAddr);
        }
        else if (isisMode == ISIS::L2_ISIS_MODE)
        {
            isisModule->generateNetAddr();
        }
        else
        {
            throw cRuntimeError("Net address wasn't specified in IS-IS routing.");
        }

        //IS type {L1(L2_ISIS_MODE) | L2 | L1L2 default for L3_ISIS_MODE}
        if (isisMode == ISIS::L2_ISIS_MODE)
        {
            isisModule->setIsType(L1_TYPE);
        }
        else
        {
            isisModule->setIsType(this->getISISISType(isisRouting));
        }

        //L1 Hello interval
        isisModule->setL1HelloInterval(this->getISISL1HelloInterval(isisRouting));

        //L1 Hello multiplier
        isisModule->setL1HelloMultiplier(this->getISISL1HelloMultiplier(isisRouting));

        //L2 Hello interval
        isisModule->setL2HelloInterval(this->getISISL2HelloInterval(isisRouting));

        //L2 Hello multiplier
        isisModule->setL2HelloMultiplier(this->getISISL2HelloMultiplier(isisRouting));

        //LSP interval
        isisModule->setLspInterval(this->getISISLSPInterval(isisRouting));

        //LSP refresh interval
        isisModule->setLspRefreshInterval(this->getISISLSPRefreshInterval(isisRouting));

        //LSP max lifetime
        isisModule->setLspMaxLifetime(this->getISISLSPMaxLifetime(isisRouting));

        //L1 LSP generating interval
        isisModule->setL1LspGenInterval(this->getISISL1LSPGenInterval(isisRouting));

        //L2 LSP generating interval
        isisModule->setL2LspGenInterval(this->getISISL2LSPGenInterval(isisRouting));

        //L1 LSP send interval
        isisModule->setL1LspSendInterval(this->getISISL1LSPSendInterval(isisRouting));

        //L2 LSP send interval
        isisModule->setL2LspSendInterval(this->getISISL2LSPSendInterval(isisRouting));

        //L1 LSP initial waiting period
        isisModule->setL1LspInitWait(this->getISISL1LSPInitWait(isisRouting));

        //L2 LSP initial waiting period
        isisModule->setL2LspInitWait(this->getISISL2LSPInitWait(isisRouting));

        //L1 CSNP interval
        isisModule->setL1CSNPInterval(this->getISISL1CSNPInterval(isisRouting));

        //L2 CSNP interval
        isisModule->setL2CSNPInterval(this->getISISL2CSNPInterval(isisRouting));

        //L1 PSNP interval
        isisModule->setL1PSNPInterval(this->getISISL1PSNPInterval(isisRouting));

        //L2 PSNP interval
        isisModule->setL2PSNPInterval(this->getISISL2PSNPInterval(isisRouting));

        //L1 SPF Full interval
        isisModule->setL1SpfFullInterval(this->getISISL1SPFFullInterval(isisRouting));

        //L2 SPF Full interval
        isisModule->setL2SpfFullInterval(this->getISISL2SPFFullInterval(isisRouting));

        /* End of core module properties */
    }
    else
    {
        this->loadISISCoreDefaultConfig(isisModule);

    }
    /* Load configuration for interfaces */

    this->loadISISInterfacesConfig(isisModule);
    /* End of load configuration for interfaces */

}
void DeviceConfigurator::loadISISInterfacesConfig(ISIS *isisModule){

    cXMLElement *interfaces = NULL;
    if (device != NULL)
    {
        interfaces = device->getFirstChildWithTag("Interfaces");
        if (interfaces == NULL)
        {
            EV
                    << "deviceId " << deviceId << ": <Interfaces></Interfaces> tag is missing in configuration file: \""
                            << configFile << "\"\n";
//        return;
        }
    }
    // add all interfaces to ISISIft vector containing additional information
//    InterfaceEntry *entryIFT = new InterfaceEntry(this); //TODO added "this" -> experimental
    for (int i = 0; i < ift->getNumInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (interfaces == NULL)
        {
            this->loadISISInterfaceDefaultConfig(isisModule, ie);
        }
        else
        {
            this->loadISISInterfaceConfig(isisModule, ie,
                    interfaces->getFirstChildWithAttribute("Interface", "name", ie->getName()));

        }
    }
}


void DeviceConfigurator::loadISISCoreDefaultConfig(ISIS *isisModule){
    //NET

          isisModule->generateNetAddr();


      //IS type {L1(L2_ISIS_MODE) | L2 | L1L2 default for L3_ISIS_MODE}

      isisModule->setIsType(L1_TYPE);

      //set Attach flag
      isisModule->setAtt(false);


      //L1 Hello interval
      isisModule->setL1HelloInterval(ISIS_HELLO_INTERVAL);

      //L1 Hello multiplier
      isisModule->setL1HelloMultiplier(ISIS_HELLO_MULTIPLIER);

      //L2 Hello interval
      isisModule->setL2HelloInterval(ISIS_HELLO_INTERVAL);

      //L2 Hello multiplier
      isisModule->setL2HelloMultiplier(ISIS_HELLO_MULTIPLIER);

      //LSP interval
      isisModule->setLspInterval(ISIS_LSP_INTERVAL);

      //LSP refresh interval
      isisModule->setLspRefreshInterval(ISIS_LSP_REFRESH_INTERVAL);

      //LSP max lifetime
      isisModule->setLspMaxLifetime(ISIS_LSP_MAX_LIFETIME);

      //L1 LSP generating interval
      isisModule->setL1LspGenInterval(ISIS_LSP_GEN_INTERVAL);

      //L2 LSP generating interval
      isisModule->setL2LspGenInterval(ISIS_LSP_GEN_INTERVAL);

      //L1 LSP send interval
      isisModule->setL1LspSendInterval(ISIS_LSP_SEND_INTERVAL);

      //L2 LSP send interval
      isisModule->setL2LspSendInterval(ISIS_LSP_SEND_INTERVAL);

      //L1 LSP initial waiting period
      isisModule->setL1LspInitWait(ISIS_LSP_INIT_WAIT);

      //L2 LSP initial waiting period
      isisModule->setL2LspInitWait(ISIS_LSP_INIT_WAIT);

      //L1 CSNP interval
      isisModule->setL1CSNPInterval(ISIS_CSNP_INTERVAL);

      //L2 CSNP interval
      isisModule->setL2CSNPInterval(ISIS_CSNP_INTERVAL);

      //L1 PSNP interval
      isisModule->setL1PSNPInterval(ISIS_PSNP_INTERVAL);

      //L2 PSNP interval
      isisModule->setL2PSNPInterval(ISIS_PSNP_INTERVAL);

      //L1 SPF Full interval
      isisModule->setL1SpfFullInterval(ISIS_SPF_FULL_INTERVAL);

      //L2 SPF Full interval
      isisModule->setL2SpfFullInterval(ISIS_SPF_FULL_INTERVAL);
}


void DeviceConfigurator::loadISISInterfaceDefaultConfig(ISIS *isisModule, InterfaceEntry *ie){

    ISISInterfaceData *d = new ISISInterfaceData();
        ISISinterface newIftEntry;
        newIftEntry.intID = ie->getInterfaceId();
        d->setIfaceId(ie->getInterfaceId());

        newIftEntry.gateIndex = ie->getNetworkLayerGateIndex();
        d->setGateIndex(ie->getNetworkLayerGateIndex());
        EV <<"deviceId: " << this->deviceId << "ISIS: adding interface, gateIndex: " <<newIftEntry.gateIndex <<endl;

        //set interface priority
        newIftEntry.priority = ISIS_DIS_PRIORITY;  //default value
        d->setPriority(ISIS_DIS_PRIORITY);

        /* Interface is NOT enabled by default. If ANY IS-IS related property is configured on interface then it's enabled. */
        newIftEntry.ISISenabled = false;
        d->setIsisEnabled(false);
        if(isisModule->getMode() == ISIS::L2_ISIS_MODE){
            newIftEntry.ISISenabled = true;
            d->setIsisEnabled(true);
        }

        //set network type (point-to-point vs. broadcast)
        newIftEntry.network = true; //default value means broadcast TODO check with TRILL default values
        d->setNetwork(ISIS_NETWORK_BROADCAST);

        //set interface metric

        newIftEntry.metric = ISIS_METRIC;    //default value
        d->setMetric(ISIS_METRIC);

        //set interface type according to global router configuration
        newIftEntry.circuitType = isisModule->getIsType();
        d->setCircuitType((ISISCircuitType)isisModule->getIsType()); //TODO B1


        //set L1 hello interval in seconds
        newIftEntry.L1HelloInterval = isisModule->getL1HelloInterval();
        d->setL1HelloInterval(isisModule->getL1HelloInterval());

        //set L1 hello multiplier
        newIftEntry.L1HelloMultiplier = isisModule->getL1HelloMultiplier();
        d->setL1HelloMultiplier(isisModule->getL1HelloMultiplier());


        //set L2 hello interval in seconds
        newIftEntry.L2HelloInterval = isisModule->getL2HelloInterval();
        d->setL2HelloInterval(isisModule->getL2HelloInterval());

        //set L2 hello multiplier
        newIftEntry.L2HelloMultiplier = isisModule->getL2HelloMultiplier();
        d->setL2HelloMultiplier(isisModule->getL2HelloMultiplier());

        //set lspInterval
        newIftEntry.lspInterval = isisModule->getLspInterval();
        d->setLspInterval(isisModule->getLspInterval());

        //set L1CsnpInterval
        newIftEntry.L1CsnpInterval = isisModule->getL1CsnpInterval();
        d->setL1CsnpInterval(isisModule->getL1CsnpInterval());

        //set L2CsnpInterval
        newIftEntry.L2CsnpInterval = isisModule->getL2CsnpInterval();
        d->setL2CsnpInterval(isisModule->getL2CsnpInterval());

        //set L1PsnpInterval
        newIftEntry.L1PsnpInterval = isisModule->getL1PsnpInterval();
        d->setL1PsnpInterval(isisModule->getL1PsnpInterval());

        //set L2PsnpInterval
        newIftEntry.L2PsnpInterval = isisModule->getL2PsnpInterval();
        d->setL2PsnpInterval(isisModule->getL2PsnpInterval());

        // priority is not needed for point-to-point, but it won't hurt
        // set priority of current DIS = me at start
        newIftEntry.L1DISpriority = newIftEntry.priority;
        d->setL1DisPriority(d->getPriority());
        newIftEntry.L2DISpriority = newIftEntry.priority;
        d->setL2DisPriority(d->getPriority());

        //set initial designated IS as himself

        memcpy(newIftEntry.L1DIS,isisModule->getSysId(), ISIS_SYSTEM_ID);
        //set LAN identifier; -99 is because, OMNeT starts numbering interfaces from 100 -> interfaceID 100 means LAN ID 0; and we want to start numbering from 1
        //newIftEntry.L1DIS[6] = ie->getInterfaceId() - 99;
        newIftEntry.L1DIS[ISIS_SYSTEM_ID] = isisModule->getISISIftSize() + 1;

        d->setL1Dis(newIftEntry.L1DIS);
        //do the same for L2 DIS

        memcpy(newIftEntry.L2DIS,isisModule->getSysId(), ISIS_SYSTEM_ID);
        //newIftEntry.L2DIS[6] = ie->getInterfaceId() - 99;
        newIftEntry.L2DIS[ISIS_SYSTEM_ID] = isisModule->getISISIftSize() + 1;
        d->setL2Dis(newIftEntry.L2DIS);

        /* By this time the trillData should be initialized.
         * So set the intial appointedForwaders to itself for configured VLAN(s).
         * TODO B5 add RFC reference and do some magic with vlanId, desiredVlanId, enabledVlans, ... */
        if(isisModule->getMode() == ISIS::L2_ISIS_MODE){
            ie->trillData()->addAppointedForwarder( ie->trillData()->getVlanId(), isisModule->getNickname());
        }
        newIftEntry.passive = false;
        d->setPassive(false);
        newIftEntry.entry = ie;

    //    this->ISISIft.push_back(newIftEntry);
        isisModule->appendISISInterface(newIftEntry);
        ie->setISISInterfaceData(d);
}


void DeviceConfigurator::loadISISInterfaceConfig(ISIS *isisModule, InterfaceEntry *entry, cXMLElement *intElement){


    if(intElement == NULL){

        this->loadISISInterfaceDefaultConfig(isisModule, entry);
        return;
    }
    ISISinterface newIftEntry;
    newIftEntry.intID = entry->getInterfaceId();

    newIftEntry.gateIndex = entry->getNetworkLayerGateIndex();
    EV <<"deviceId: " << this->deviceId << "ISIS: adding interface, gateIndex: " <<newIftEntry.gateIndex <<endl;

    //set interface priority
    newIftEntry.priority = ISIS_DIS_PRIORITY;  //default value

    /* Interface is NOT enabled by default. If ANY IS-IS related property is configured on interface then it's enabled. */
    newIftEntry.ISISenabled = false;
    if(isisModule->getMode() == ISIS::L2_ISIS_MODE){
        newIftEntry.ISISenabled = true;
    }

    cXMLElement *priority = intElement->getFirstChildWithTag("ISIS-Priority");
    if (priority != NULL && priority->getNodeValue() != NULL)
    {
        newIftEntry.priority = (unsigned char) atoi(priority->getNodeValue());
        newIftEntry.ISISenabled = true;
    }


    //set network type (point-to-point vs. broadcast)

    newIftEntry.network = true; //default value

    cXMLElement *network = intElement->getFirstChildWithTag("ISIS-Network");
    if (network != NULL && network->getNodeValue() != NULL)
    {
        if (!strcmp(network->getNodeValue(), "point-to-point"))
        {
            newIftEntry.network = false;
            EV << "Interface network type is point-to-point " << network->getNodeValue() << endl;
        }
        else if (!strcmp(network->getNodeValue(), "broadcast"))
        {
            EV << "Interface network type is broadcast " << network->getNodeValue() << endl;
        }
        else
        {
            EV << "ERORR: Unrecognized interface's network type: " << network->getNodeValue() << endl;

        }
        newIftEntry.ISISenabled = true;

    }



    //set interface metric

    newIftEntry.metric = ISIS_METRIC;    //default value

        cXMLElement *metric = intElement->getFirstChildWithTag("ISIS-Metric");
        if(metric != NULL && metric->getNodeValue() != NULL)
        {
            newIftEntry.metric = (unsigned char) atoi(metric->getNodeValue());
            newIftEntry.ISISenabled = true;
        }




    //set interface type according to global router configuration
    switch(isisModule->getIsType())
    {
        case(L1_TYPE):
                 newIftEntry.circuitType = L1_TYPE;
                 break;
        case(L2_TYPE):
                 newIftEntry.circuitType = L2_TYPE;
                 break;
        //if router is type is equal L1L2, then interface configuration sets the type
        default: {

            newIftEntry.circuitType = L1L2_TYPE;

            cXMLElement *circuitType = intElement->getFirstChildWithTag("ISIS-Circuit-Type");
            if (circuitType != NULL && circuitType->getNodeValue() != NULL)
            {
                if (strcmp(circuitType->getNodeValue(), "L2") == 0){
                    newIftEntry.circuitType = L2_TYPE;
                }
                else
                {
                    if (strcmp(circuitType->getNodeValue(), "L1") == 0)
                        newIftEntry.circuitType = L1_TYPE;
                }
                newIftEntry.ISISenabled = true;
            }
            else
            {
                newIftEntry.circuitType = L1L2_TYPE;
            }

            break;
        }
    }

    //set L1 hello interval in seconds
    cXMLElement *L1HelloInt = intElement->getFirstChildWithTag(
            "ISIS-L1-Hello-Interval");
    if (L1HelloInt == NULL || L1HelloInt->getNodeValue() == NULL) {
        newIftEntry.L1HelloInterval = isisModule->getL1HelloInterval();
    } else {
        newIftEntry.L1HelloInterval = atoi(L1HelloInt->getNodeValue());
    }

    //set L1 hello multiplier
    cXMLElement *L1HelloMult = intElement->getFirstChildWithTag(
            "ISIS-L1-Hello-Multiplier");
    if (L1HelloMult == NULL || L1HelloMult->getNodeValue() == NULL) {
        newIftEntry.L1HelloMultiplier = isisModule->getL1HelloMultiplier();
    } else {
        newIftEntry.L1HelloMultiplier = atoi(L1HelloMult->getNodeValue());
    }

    //set L2 hello interval in seconds
    cXMLElement *L2HelloInt = intElement->getFirstChildWithTag(
            "ISIS-L2-Hello-Interval");
    if (L2HelloInt == NULL || L2HelloInt->getNodeValue() == NULL) {
        newIftEntry.L2HelloInterval = isisModule->getL2HelloInterval();
    } else {
        newIftEntry.L2HelloInterval = atoi(L2HelloInt->getNodeValue());
    }

    //set L2 hello multiplier
    cXMLElement *L2HelloMult = intElement->getFirstChildWithTag(
            "ISIS-L2-Hello-Multiplier");
    if (L2HelloMult == NULL || L2HelloMult->getNodeValue() == NULL) {
        newIftEntry.L2HelloMultiplier = isisModule->getL2HelloMultiplier();
    } else {
        newIftEntry.L2HelloMultiplier = atoi(L2HelloMult->getNodeValue());
    }

    //set lspInterval
    cXMLElement *cxlspInt = intElement->getFirstChildWithTag("ISIS-LSP-Interval");
    if (cxlspInt == NULL || cxlspInt->getNodeValue() == NULL)
    {
//        newIftEntry.lspInterval = ISIS_LSP_INTERVAL;
        newIftEntry.lspInterval = isisModule->getLspInterval();
    }
    else
    {
        newIftEntry.lspInterval = atoi(cxlspInt->getNodeValue());
    }

    //set L1CsnpInterval
    cXMLElement *cxL1CsnpInt = intElement->getFirstChildWithTag("ISIS-L1-CSNP-Interval");
    if (cxL1CsnpInt == NULL || cxL1CsnpInt->getNodeValue() == NULL)
    {
//        newIftEntry.L1CsnpInterval = ISIS_CSNP_INTERVAL;
                newIftEntry.L1CsnpInterval = isisModule->getL1CsnpInterval();
    }
    else
    {
        newIftEntry.L1CsnpInterval = atoi(cxL1CsnpInt->getNodeValue());
    }

    //set L2CsnpInterval
    cXMLElement *cxL2CsnpInt = intElement->getFirstChildWithTag("ISIS-L2-CSNP-Interval");
    if (cxL2CsnpInt == NULL || cxL2CsnpInt->getNodeValue() == NULL)
    {
//        newIftEntry.L2CsnpInterval = ISIS_CSNP_INTERVAL;
        newIftEntry.L2CsnpInterval = isisModule->getL2CsnpInterval();
    }
    else
    {
        newIftEntry.L2CsnpInterval = atoi(cxL2CsnpInt->getNodeValue());
    }

    //set L1PsnpInterval
    cXMLElement *cxL1PsnpInt = intElement->getFirstChildWithTag("ISIS-L1-PSNP-Interval");
    if (cxL1PsnpInt == NULL || cxL1PsnpInt->getNodeValue() == NULL)
    {
//        newIftEntry.L1PsnpInterval = ISIS_PSNP_INTERVAL;
        newIftEntry.L1PsnpInterval = isisModule->getL1PsnpInterval();
    }
    else
    {
        newIftEntry.L1PsnpInterval = atoi(cxL1PsnpInt->getNodeValue());
    }

    //set L2PsnpInterval
    cXMLElement *cxL2PsnpInt = intElement->getFirstChildWithTag("ISIS-L2-PSNP-Interval");
    if (cxL2PsnpInt == NULL || cxL2PsnpInt->getNodeValue() == NULL)
    {
//        newIftEntry.L2PsnpInterval = ISIS_PSNP_INTERVAL;
        newIftEntry.L2PsnpInterval = isisModule->getL2PsnpInterval();
    }
    else
    {
        newIftEntry.L2PsnpInterval = atoi(cxL2PsnpInt->getNodeValue());
    }


    // priority is not needed for point-to-point, but it won't hurt
    // set priority of current DIS = me at start
    newIftEntry.L1DISpriority = newIftEntry.priority;
    newIftEntry.L2DISpriority = newIftEntry.priority;

    //set initial designated IS as himself
//    this->copyArrayContent((unsigned char*)this->sysId, newIftEntry.L1DIS, ISIS_SYSTEM_ID, 0, 0);
    memcpy(newIftEntry.L1DIS,isisModule->getSysId(), ISIS_SYSTEM_ID);
    //set LAN identifier; -99 is because, OMNeT starts numbering interfaces from 100 -> interfaceID 100 means LAN ID 0; and we want to start numbering from 1
    //newIftEntry.L1DIS[6] = entry->getInterfaceId() - 99;
    newIftEntry.L1DIS[ISIS_SYSTEM_ID] = newIftEntry.gateIndex + 1;
    //do the same for L2 DIS
//    memcpy((unsigned char*)isisModule->getSy, newIftEntry.L2DIS, ISIS_SYSTEM_ID);
    memcpy(newIftEntry.L2DIS,isisModule->getSysId(), ISIS_SYSTEM_ID);
    //newIftEntry.L2DIS[6] = entry->getInterfaceId() - 99;
    newIftEntry.L2DIS[ISIS_SYSTEM_ID] = newIftEntry.gateIndex + 1;

    newIftEntry.passive = false;
    newIftEntry.entry = entry;

//    this->ISISIft.push_back(newIftEntry);
    isisModule->appendISISInterface(newIftEntry);
}




const char *DeviceConfigurator::getISISNETAddress(cXMLElement *isisRouting)
{
    //TODO: multiple NETs for migrating purposes (merging, splitting areas)
    cXMLElement *net = isisRouting->getFirstChildWithTag("NET");
    if (net == NULL)
    {
//            EV << "deviceId " << deviceId << ": Net address wasn't specified in IS-IS routing\n";
//            throw cRuntimeError("Net address wasn't specified in IS-IS routing");
        return NULL;
    }
    return net->getNodeValue();
}

short int DeviceConfigurator::getISISISType(cXMLElement *isisRouting){
    //set router IS type {L1(L2_M | L2 | L1L2 (default)}
    cXMLElement *routertype = isisRouting->getFirstChildWithTag("IS-Type");
    if (routertype == NULL) {
        return L1L2_TYPE;
    } else {
        const char* routerTypeValue = routertype->getNodeValue();
        if (routerTypeValue == NULL) {
            return L1L2_TYPE;
        } else {
            if (strcmp(routerTypeValue, "level-1") == 0) {
                return L1_TYPE;
            } else {
                if (strcmp(routerTypeValue, "level-2") == 0) {
                    return L2_TYPE;
                } else {
                    return L1L2_TYPE;
                }
            }
        }
    }
}

int DeviceConfigurator::getISISL1HelloInterval(cXMLElement *isisRouting){
    //get L1 hello interval in seconds
    cXMLElement *L1HelloInt = isisRouting->getFirstChildWithTag(
            "L1-Hello-Interval");
    if (L1HelloInt == NULL || L1HelloInt->getNodeValue() == NULL) {
        return ISIS_HELLO_INTERVAL;
    } else {
        return atoi(L1HelloInt->getNodeValue());
    }
}

int DeviceConfigurator::getISISL1HelloMultiplier(cXMLElement *isisRouting){
    //get L1 hello multiplier
    cXMLElement *L1HelloMult = isisRouting->getFirstChildWithTag(
            "L1-Hello-Multiplier");
    if (L1HelloMult == NULL || L1HelloMult->getNodeValue() == NULL) {
        return ISIS_HELLO_MULTIPLIER;
    } else {
        return atoi(L1HelloMult->getNodeValue());
    }
}

int DeviceConfigurator::getISISL2HelloInterval(cXMLElement *isisRouting){
    //get L2 hello interval in seconds
    cXMLElement *L2HelloInt = isisRouting->getFirstChildWithTag(
            "L2-Hello-Interval");
    if (L2HelloInt == NULL || L2HelloInt->getNodeValue() == NULL) {
        return ISIS_HELLO_INTERVAL;
    } else {
        return atoi(L2HelloInt->getNodeValue());
    }
}

int DeviceConfigurator::getISISL2HelloMultiplier(cXMLElement *isisRouting){
    //get L2 hello multiplier
    cXMLElement *L2HelloMult = isisRouting->getFirstChildWithTag(
            "L2-Hello-Multiplier");
    if (L2HelloMult == NULL || L2HelloMult->getNodeValue() == NULL) {
        return ISIS_HELLO_MULTIPLIER;
    } else {
        return atoi(L2HelloMult->getNodeValue());
    }
}

int DeviceConfigurator::getISISLSPInterval(cXMLElement *isisRouting){
    //set lspInterval
    cXMLElement *cxlspInt = isisRouting->getFirstChildWithTag("LSP-Interval");
    if (cxlspInt == NULL || cxlspInt->getNodeValue() == NULL)
    {
        return ISIS_LSP_INTERVAL;
    }
    else
    {
        return atoi(cxlspInt->getNodeValue());
    }
}

int DeviceConfigurator::getISISLSPRefreshInterval(cXMLElement *isisRouting){
    //get lspRefreshInterval
    cXMLElement *cxlspRefInt = isisRouting->getFirstChildWithTag("LSP-Refresh-Interval");
    if (cxlspRefInt == NULL || cxlspRefInt->getNodeValue() == NULL)
    {
        return ISIS_LSP_REFRESH_INTERVAL;
    }
    else
    {
        return atoi(cxlspRefInt->getNodeValue());
    }
}

int DeviceConfigurator::getISISLSPMaxLifetime(cXMLElement *isisRouting)
{
    //get lspMaxLifetime
    cXMLElement *cxlspMaxLife = isisRouting->getFirstChildWithTag("LSP-Max-Lifetime");
    if (cxlspMaxLife == NULL || cxlspMaxLife->getNodeValue() == NULL)
    {
        return ISIS_LSP_MAX_LIFETIME;
    }
    else
    {
        return atoi(cxlspMaxLife->getNodeValue());
    }

}

int DeviceConfigurator::getISISL1LSPGenInterval(cXMLElement *isisRouting)
{
    //set L1LspGenInterval (CISCO's
    cXMLElement *cxL1lspGenInt = isisRouting->getFirstChildWithTag("L1-LSP-Gen-Interval");
    if (cxL1lspGenInt == NULL || cxL1lspGenInt->getNodeValue() == NULL)
    {
        return ISIS_LSP_GEN_INTERVAL;
    }
    else
    {
        return atoi(cxL1lspGenInt->getNodeValue());
    }
}

int DeviceConfigurator::getISISL2LSPGenInterval(cXMLElement *isisRouting)
{
    //get L2LspGenInterval (CISCO's
    cXMLElement *cxL2lspGenInt = isisRouting->getFirstChildWithTag("L2-LSP-Gen-Interval");
    if (cxL2lspGenInt == NULL || cxL2lspGenInt->getNodeValue() == NULL)
    {
        return ISIS_LSP_GEN_INTERVAL;
    }
    else
    {
        return atoi(cxL2lspGenInt->getNodeValue());
    }
}

int DeviceConfigurator::getISISL1LSPSendInterval(cXMLElement *isisRouting){
    //get L1LspSendInterval
    cXMLElement *cxL1lspSendInt = isisRouting->getFirstChildWithTag("L1-LSP-Send-Interval");
    if (cxL1lspSendInt == NULL || cxL1lspSendInt->getNodeValue() == NULL)
    {
        return ISIS_LSP_SEND_INTERVAL;
    }
    else
    {
        return atoi(cxL1lspSendInt->getNodeValue());
    }
}

int DeviceConfigurator::getISISL2LSPSendInterval(cXMLElement *isisRouting){
    //get L2LspSendInterval
    cXMLElement *cxL2lspSendInt = isisRouting->getFirstChildWithTag("L2-LSP-Send-Interval");
    if (cxL2lspSendInt == NULL || cxL2lspSendInt->getNodeValue() == NULL)
    {
        return ISIS_LSP_SEND_INTERVAL;
    }
    else
    {
        return atoi(cxL2lspSendInt->getNodeValue());
    }
}

int DeviceConfigurator::getISISL1LSPInitWait(cXMLElement *isisRouting)
{
    //get L1LspInitWait
    cXMLElement *cxL1lspInitWait = isisRouting->getFirstChildWithTag("L1-LSP-Init-Wait");
    if (cxL1lspInitWait == NULL || cxL1lspInitWait->getNodeValue() == NULL)
    {
        return ISIS_LSP_INIT_WAIT;
    }
    else
    {
        return atoi(cxL1lspInitWait->getNodeValue());
    }
}



int DeviceConfigurator::getISISL2LSPInitWait(cXMLElement *isisRouting)
{
    //get L2LspInitWait
    cXMLElement *cxL2lspInitWait = isisRouting->getFirstChildWithTag("L2-LSP-Init-Wait");
    if (cxL2lspInitWait == NULL || cxL2lspInitWait->getNodeValue() == NULL)
    {
        return ISIS_LSP_INIT_WAIT;
    }
    else
    {
        return atoi(cxL2lspInitWait->getNodeValue());
    }
}

int DeviceConfigurator::getISISL1CSNPInterval(cXMLElement *isisRouting){
    //get L1CsnpInterval
    cXMLElement *cxL1CsnpInt = isisRouting->getFirstChildWithTag("L1-CSNP-Interval");
    if (cxL1CsnpInt == NULL || cxL1CsnpInt->getNodeValue() == NULL)
    {
        return ISIS_CSNP_INTERVAL;
    }
    else
    {
        return atoi(cxL1CsnpInt->getNodeValue());
    }

}

int DeviceConfigurator::getISISL2CSNPInterval(cXMLElement *isisRouting){
    //get L2CsnpInterval
    cXMLElement *cxL2CsnpInt = isisRouting->getFirstChildWithTag("L2-CSNP-Interval");
    if (cxL2CsnpInt == NULL || cxL2CsnpInt->getNodeValue() == NULL)
    {
        return ISIS_CSNP_INTERVAL;
    }
    else
    {
        return atoi(cxL2CsnpInt->getNodeValue());
    }

}

int DeviceConfigurator::getISISL1PSNPInterval(cXMLElement *isisRouting){
    //get L1PSNPInterval
    cXMLElement *cxL1PSNPInt = isisRouting->getFirstChildWithTag("L1-PSNP-Interval");
    if (cxL1PSNPInt == NULL || cxL1PSNPInt->getNodeValue() == NULL)
    {
        return ISIS_PSNP_INTERVAL;
    }
    else
    {
        return atoi(cxL1PSNPInt->getNodeValue());
    }

}

int DeviceConfigurator::getISISL2PSNPInterval(cXMLElement *isisRouting){
    //get L2PSNPInterval
    cXMLElement *cxL2PSNPInt = isisRouting->getFirstChildWithTag("L2-PSNP-Interval");
    if (cxL2PSNPInt == NULL || cxL2PSNPInt->getNodeValue() == NULL)
    {
        return ISIS_PSNP_INTERVAL;
    }
    else
    {
        return atoi(cxL2PSNPInt->getNodeValue());
    }

}

int DeviceConfigurator::getISISL1SPFFullInterval(cXMLElement *isisRouting){
    //get L1SPFFullInterval
    cXMLElement *cxL1SPFFullInt = isisRouting->getFirstChildWithTag("L1-SPF-Full-Interval");
    if (cxL1SPFFullInt == NULL || cxL1SPFFullInt->getNodeValue() == NULL)
    {
        return ISIS_SPF_FULL_INTERVAL;
    }
    else
    {
        return atoi(cxL1SPFFullInt->getNodeValue());
    }

}

int DeviceConfigurator::getISISL2SPFFullInterval(cXMLElement *isisRouting){
    //get L2SPFFullInterval
    cXMLElement *cxL2SPFFullInt = isisRouting->getFirstChildWithTag("L2-SPF-Full-Interval");
    if (cxL2SPFFullInt == NULL || cxL2SPFFullInt->getNodeValue() == NULL)
    {
        return ISIS_SPF_FULL_INTERVAL;
    }
    else
    {
        return atoi(cxL2SPFFullInt->getNodeValue());
    }

}

void DeviceConfigurator::addIPv4MulticastGroups(cXMLElement *iface)
{
    while (iface != NULL)
    {
        // get MCastGroups Node
        cXMLElement* MCastGroupsNode = iface->getElementByPath("MCastGroups");
        if (MCastGroupsNode == NULL)
        {
            break;
        }



        //std::vector<IPv4Address> mcastGroupsList;
        InterfaceEntry *ie = ift->getInterfaceByName(iface->getAttribute("name"));
        bool empty = true;
        for (cXMLElement *mcastNode=MCastGroupsNode->getFirstChild(); mcastNode; mcastNode = mcastNode->getNextSibling())
        {
            const char *mcastAddress = mcastNode->getNodeValue();
            //mcastGroupsList.push_back((IPv4Address)mcastAddress);
            ie->ipv4Data()->joinMulticastGroup((IPv4Address)mcastAddress);
            empty = false;
        }
        if(empty)
            EV << "No Multicast Groups found for this interface " << iface->getAttribute("name") << " on this device: " <<  deviceType << " id=" << deviceId << endl;
        iface = xmlParser::GetInterface(iface, NULL);
    }

}

void DeviceConfigurator::addIPv6MulticastGroups(cXMLElement *iface)
{
    while (iface != NULL)
    {
        // get MCastGroups Node
        cXMLElement* MCastGroupsNode6 = iface->getElementByPath("MCastGroups6");
        if (MCastGroupsNode6 == NULL)
        {
            break;
        }



        //std::vector<IPv4Address> mcastGroupsList;
        InterfaceEntry *ie = ift->getInterfaceByName(iface->getAttribute("name"));
        bool empty = true;
        for (cXMLElement *mcastNode6=MCastGroupsNode6->getFirstChild(); mcastNode6; mcastNode6 = mcastNode6->getNextSibling())
        {
            const char *mcastAddress6 = mcastNode6->getNodeValue();
            //mcastGroupsList.push_back((IPv4Address)mcastAddress);
            ie->ipv6Data()->joinMulticastGroup(IPv6Address(mcastAddress6));      //todo
            empty = false;
        }
        if(empty)
            EV << "No Multicast6 Groups found for this interface " << iface->getAttribute("name") << " on this device: " <<  deviceType << " id=" << deviceId << endl;
        iface = xmlParser::GetInterface(iface, NULL);
    }

}

/* End of IS-IS related methods */

//
//- configuration for VRRPv2
//
void DeviceConfigurator::loadVRRPv2Config(VRRPv2* VRRPModule) {

    ASSERT(VRRPModule != NULL);

    // get access to device node from XML
    const char *deviceType = par("deviceType");
    const char *deviceId = par("deviceId");
    const char *configFile = par("configFile");
    cXMLElement *device = xmlParser::GetDevice(deviceType, deviceId, configFile);

    if (device == NULL){
       ev << "No configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
            return;
    }

    cXMLElement *interface;
    //get first router's interface
    interface = xmlParser::GetInterface(NULL, device);

    while (interface != NULL)
    {
        int interfaceId = ift->getInterfaceByName(interface->getAttribute("name"))->getInterfaceId();

        cXMLElement *group;
        group = xmlParser::GetVRRPGroup(NULL, interface);
        while (group != NULL)
        {
            int groupId = -1;
            if (xmlParser::HasVRPPGroup(group, &groupId))
                VRRPModule->addVirtualRouter(interfaceId, groupId);

            group = xmlParser::GetVRRPGroup(group, NULL);
        }
        interface = xmlParser::GetInterface(interface, NULL);
    }
}

void DeviceConfigurator::loadVRRPv2VirtualRouterConfig(VRRPv2VirtualRouter* VRRPModule) {
    ASSERT(VRRPModule != NULL);

    // get access to device node from XML
    const char *deviceType = par("deviceType");
    const char *deviceId = par("deviceId");
    const char *configFile = par("configFile");
    cXMLElement *device = xmlParser::GetDevice(deviceType, deviceId, configFile);

    if (device == NULL){
       EV << "No configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
            return;
    }

    std::ostringstream groupId;
    groupId <<  VRRPModule->getVrid();

    //stringstream ss;
    //ss << VRRPModule->getVrid();
    //string groupId = ss.str();

    //const char* aaa = VRRPModule->getInterface()->getFullName();

    cXMLElement *group = xmlParser::GetVRRPGroup(device, VRRPModule->getInterface()->getFullName(), groupId.str().c_str());
    if (group == NULL) {
        EV << "No configuration found for group " << groupId << endl;
        return;
    }

    cXMLElementList ifDetails = group->getChildren();
    for (cXMLElementList::iterator routeElemIt = ifDetails.begin(); routeElemIt != ifDetails.end(); routeElemIt++)
    {
        std::string nodeName = (*routeElemIt)->getTagName();

        if (nodeName == "IPAddress")
        {
            VRRPModule->setIPPrimary((*routeElemIt)->getNodeValue());
            VRRPModule->addIPSecondary((*routeElemIt)->getNodeValue());
        }
        else if (nodeName == "IPSecondary")
        {
            VRRPModule->addIPSecondary((*routeElemIt)->getNodeValue());
        }
        else if (nodeName == "Description")
        {
            VRRPModule->setDescription((*routeElemIt)->getNodeValue());
        }
        else if (nodeName == "Priority")
        {
            int value = 0;
            if (!xmlParser::Str2Int(&value, (*routeElemIt)->getNodeValue()))
                throw cRuntimeError("Invalid Priority value on interface %s", VRRPModule->getInterface()->getName());

            VRRPModule->setPriority(value);
        }
        else if (nodeName == "Preempt")
        {
            bool value = false;
            if (!xmlParser::Str2Bool(&value, (*routeElemIt)->getNodeValue())){
               throw cRuntimeError("Invalid Preempt value on interface %s", VRRPModule->getInterface()->getName());
            }
            VRRPModule->setPreemtion(value);

            if ((*routeElemIt)->hasAttributes()) {
                int value = 0;
                if (!xmlParser::Str2Int(&value, (*routeElemIt)->getAttribute("delay")))
                    throw cRuntimeError("Unable to parse valid Preemtion delay %s on interface %s", value, VRRPModule->getInterface()->getName());

                VRRPModule->setPreemtionDelay(value);
            }
        }
        else if (nodeName == "TimerAdvertise")
        {
            int value = 0;
            if (!xmlParser::Str2Int(&value, (*routeElemIt)->getNodeValue()))
                throw cRuntimeError("Unable to parse valid TimerAdvertise %s on interface %s", value, VRRPModule->getInterface()->getFullName());

            VRRPModule->setAdvertisement(value);
        }
        else if (nodeName == "TimerLearn")
        {
            bool value = false;
            if (!xmlParser::Str2Bool(&value, (*routeElemIt)->getNodeValue())){
               throw cRuntimeError("Invalid TimerLearn value on interface %s", VRRPModule->getInterface()->getName());
            }
            VRRPModule->setLearn(value);
        }
    }
}

//
//
//- configuration for EIGRP
//
//
bool DeviceConfigurator::wildcardToMask(const char *wildcard, IPv4Address& result)
{
    result.set(wildcard);
    uint32 wcNum = result.getInt();

    // convert wildcard to mask
    wcNum = ~wcNum;
    result.set(wcNum);

    if (!result.isValidNetmask())
        return false;

    return true;
}

EigrpNetwork *DeviceConfigurator::isEigrpInterface(std::vector<EigrpNetwork *>& networks, InterfaceEntry *interface)
{
    IPv4Address prefix, mask;
    IPv4Address ifAddress = interface->ipv4Data()->getIPAddress();
    IPv4Address ifmask = interface->ipv4Data()->getNetmask();
    vector<int> resultIfs;
    int maskLength, ifMaskLength;
    std::vector<EigrpNetwork *>::iterator it;

    if (ifAddress.isUnspecified())
                return false;

    for (it = networks.begin(); it != networks.end(); it++)
    {
        prefix = (*it)->getAddress();
        mask = (*it)->getMask();

        maskLength = (mask.isUnspecified()) ? prefix.getNetworkMask().getNetmaskLength() : mask.getNetmaskLength();
        ifMaskLength = ifmask.getNetmaskLength();

        // prefix isUnspecified -> network = 0.0.0.0 -> all interfaces, or
        // mask is unspecified -> classful match or
        // mask is specified -> classless match
        if (prefix.isUnspecified() ||
                (mask.isUnspecified() && prefix.isNetwork(ifAddress) && maskLength <= ifMaskLength) ||
                (prefix.maskedAddrAreEqual(prefix, ifAddress, mask) && maskLength <= ifMaskLength))
        {
            return *it;
        }
    }

    return NULL;
}

void DeviceConfigurator::loadEigrpIPv4Networks(cXMLElement *processElem, IEigrpModule *eigrpModule)
{
    const char *addressStr, *wildcardStr;
    IPv4Address address, mask;
    std::vector<EigrpNetwork *> networks;
    EigrpNetwork *net;
    InterfaceEntry *iface;

    cXMLElement *netoworkParentElem = processElem->getFirstChildWithTag("Networks");
    if (netoworkParentElem == NULL)
        return;
    cXMLElement *networkElem = xmlParser::GetEigrpIPv4Network(NULL, netoworkParentElem);

    while (networkElem != NULL)
    {
        // Get IP address
        if ((addressStr = xmlParser::GetNodeParamConfig(networkElem, "IPAddress", NULL)) == NULL)
        {// address is mandatory
            throw cRuntimeError("No IP address specified in the IPAddress node");
        }

        // Get wildcard
        wildcardStr = xmlParser::GetNodeParamConfig(networkElem, "Wildcard", NULL);

        // Create network address and mask
        address.set(addressStr);
        if (wildcardStr == NULL)
        {// wildcard is optional
            mask = IPv4Address::UNSPECIFIED_ADDRESS;
            // classful network
            address = address.getNetwork();
        }
        else
        {
            // Accepts incorrectly specified wildcard as normal mask (Cisco)
            mask = IPv4Address(wildcardStr);
            if (mask.isUnspecified() || !mask.isValidNetmask()) {
                if (!wildcardToMask(wildcardStr, mask))
                    throw cRuntimeError("Invalid wildcard in EIGRP network configuration");
            }
            address = address.doAnd(mask);
        }

        net = eigrpModule->addNetwork(address, mask);
        networks.push_back(net);

        networkElem = xmlParser::GetEigrpIPv4Network(networkElem, NULL);
    }

    // Find and store interfaces for networks
    for(int i = 0; i < ift->getNumInterfaces(); i++)
    {
        iface = ift->getInterface(i);
        if ((net = isEigrpInterface(networks, iface)) != NULL)
            eigrpModule->addInterface(iface->getInterfaceId(), net->getNetworkId(), true);
    }
}

void DeviceConfigurator::loadEigrpIPv4Config(IEigrpModule *eigrpModule)
{
    ASSERT(eigrpModule != NULL);

    // get access to device node from XML
    const char *deviceType = par("deviceType");
    const char *deviceId = par("deviceId");
    const char *configFile = par("configFile");
    cXMLElement *device = xmlParser::GetDevice(deviceType, deviceId, configFile);

    if (device == NULL)
    {
       EV << "No configuration found for this device (" << deviceType <<
               " id=" << deviceId << ")" << endl;
       return;
    }

    loadEigrpProcessesConfig(device, eigrpModule);

    loadEigrpInterfaceConfig(device, eigrpModule);
}

void DeviceConfigurator::loadEigrpProcessesConfig(cXMLElement *device, IEigrpModule *eigrpModule)
{
    // XML nodes for EIGRP
    cXMLElement *processElem = NULL;
    cXMLElement *tempNode = NULL;

    int asNum;    // converted AS number
    const char *asNumStr;   // string with AS number
    bool success;

    processElem = xmlParser::GetEigrpProcess(processElem, device);
    if (processElem == NULL)
    {
        EV << "No EIGRP configuration found." << endl;
        return;
    }

    while (processElem != NULL)
    {
        // AS number of process
        if ((asNumStr = processElem->getAttribute("asNumber")) == NULL)
        {// AS number is mandatory
            throw cRuntimeError("No EIGRP autonomous system number specified");
        }
        success = xmlParser::Str2Int(&asNum, asNumStr);
        if (!success || asNum < 1 || asNum > 65535)
        { // bad value, AS number must be in <1, 65535>
            throw cRuntimeError("Bad value for EIGRP autonomous system number");
        }
        eigrpModule->setASNum(asNum);

        // Loads networks and enables corresponding interfaces
        loadEigrpIPv4Networks(processElem, eigrpModule);

        // K-values for metric computation
        if ((tempNode = processElem->getFirstChildWithTag("MetricWeights")) != NULL)
        {
            EigrpKValues kval;
            kval.K1 = loadEigrpKValue(tempNode, "k1", "1");
            kval.K2 = loadEigrpKValue(tempNode, "k2", "0");
            kval.K3 = loadEigrpKValue(tempNode, "k3", "1");
            kval.K4 = loadEigrpKValue(tempNode, "k4", "0");
            kval.K5 = loadEigrpKValue(tempNode, "k5", "0");
            //kval.K6 = loadEigrpKValue(tempNode, "k6", "0");

            eigrpModule->setKValues(kval);
        }

        processElem = xmlParser::GetEigrpProcess(processElem, NULL);
    }
}

int DeviceConfigurator::loadEigrpKValue(cXMLElement *node, const char *attrName, const char *attrValue)
{
    int result;
    bool success;
    const char *kvalueStr = xmlParser::GetNodeAttrConfig(node, attrName, attrValue);

    success = xmlParser::Str2Int(&result, kvalueStr);
    if (!success || result < 0 || result > 255)
    { // bad value, K-value must be in <0, 255>
        throw cRuntimeError("Bad value for EIGRP metric weights");
    }
    return result;
}

void DeviceConfigurator::loadEigrpInterfaceConfig(cXMLElement *device, IEigrpModule *eigrpModule)
{
    // XML nodes for EIGRP
    cXMLElement *eigrpIfaceElem = NULL;
    cXMLElement *ifaceElem = NULL;

    bool success;
    int tempNumber;

    if ((ifaceElem = xmlParser::GetInterface(ifaceElem, device)) == NULL)
    {
        return;
    }

    while (ifaceElem != NULL)
    {
        // Get EIGRP configuration for interface
        eigrpIfaceElem = ifaceElem->getFirstChildWithTag("EIGRP-IPv4");

        if (eigrpIfaceElem != NULL)
        {
            // Get interface ID
            const char *ifaceName = ifaceElem->getAttribute("name");
            InterfaceEntry *iface = ift->getInterfaceByName(ifaceName);
            int ifaceId = iface->getInterfaceId();

            // Get EIGRP AS number
            const char *asNumStr;
            if ((asNumStr = eigrpIfaceElem->getAttribute("asNumber")) == NULL)
            {// AS number is mandatory
                throw cRuntimeError("No EIGRP autonomous system number specified in interface settings");
            }
            success = xmlParser::Str2Int(&tempNumber, asNumStr);
            if (!success || tempNumber < 1 || tempNumber > 65535)
            { // bad value, AS number must be in <1, 65535>
                throw cRuntimeError("Bad value for EIGRP autonomous system number");
            }
            // TODO vybrat podle AS spravny PDM a pro ten nastavovat nasledujici

            // Hello interval
            const char *helloStr = xmlParser::GetNodeParamConfig(eigrpIfaceElem, "HelloInterval", NULL);
            if (helloStr != NULL)
            {
                success = xmlParser::Str2Int(&tempNumber, helloStr);
                if (!success || tempNumber < 1 || tempNumber > 65535)
                { // bad value, Hello interval must be in <1, 65535>
                    throw cRuntimeError("Bad value for EIGRP Hello interval");
                }
                eigrpModule->setHelloInt(tempNumber, ifaceId);
            }

            // Hold interval
            const char *holdStr = xmlParser::GetNodeParamConfig(eigrpIfaceElem, "HoldInterval", NULL);
            if (holdStr != NULL)
            {
                success = xmlParser::Str2Int(&tempNumber, holdStr);
                if (!success || tempNumber < 1 || tempNumber > 65535)
                { // bad value, Hold interval must be in <1, 65535>
                    throw cRuntimeError("Bad value for EIGRP Hold interval");
                }
                eigrpModule->setHoldInt(tempNumber, ifaceId);
            }
        }

        ifaceElem = xmlParser::GetInterface(ifaceElem, NULL);
    }
}


void DeviceConfigurator::loadLISPConfig(LISPCore* LISPModule)
{
    ASSERT(LISPModule != NULL);

    // get access to device node from XML
    const char *deviceType = par("deviceType");
    const char *deviceId = par("deviceId");
    const char *configFile = par("configFile");
    cXMLElement *device = xmlParser::GetDevice(deviceType, deviceId, configFile);

    if (device == NULL){
       ev << "No configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
            return;
    }

    cXMLElement *mss = xmlParser::GetLISPMapServers(NULL, device);
    while (mss != NULL) {
        if (mss->hasAttributes()) {
            std::string mssv4 = mss->getAttribute("ipv4");
            std::string mssv6 = mss->getAttribute("ipv6");
            EV << "==== MapServer" << mssv4 << "   " << mssv6 << endl;
        }
        mss = xmlParser::GetLISPMapServers(mss, device);
    }
}
