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
 * @file TRILL.cc
 * @author Marek Cerny, Jiri Trhlik, Tomas Prochazka (mailto:xproch21@stud.fit.vutbr.cz), .. DOPLNTE Marcel Marek (mailto:xscrew02@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 2011
 * @brief
 * @detail
 * @todo
 */

#include "deviceConfigurator.h"

#include "IPv6Address.h"
#include "IPv6InterfaceData.h"
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
        /* doesn't need to be performed anywhere else,
         * if it's NULL then behaviour depends on device type
         */
        device = xmlParser::GetDevice(deviceType, deviceId, configFile);

    }



   // interfaces and routing table are not ready before stage 2
    if (stage == 2){

        // get table of interfaces of this device
        ift = InterfaceTableAccess().get();
        if (ift == NULL){
           throw cRuntimeError("InterfaceTable not found");
        }
        // get routing table of this device
        rt = AnsaRoutingTableAccess().getIfExists();
        if (rt != NULL){
           //throw cRuntimeError("AnsaRoutingTable not found");

            for (int i=0; i<ift->getNumInterfaces(); ++i)
                rt->configureInterfaceForIPv4(ift->getInterface(i));

            const char *routerIdStr = par("deviceId").stringValue();
            this->readRoutingTableFromXml(configFile, routerIdStr);
        }

      // get routing table of this device
      rt6 = RoutingTable6Access().getIfExists();
      if (rt6 != NULL){
          //throw cRuntimeError("RoutingTable6 not found");

          // RFC 4861 specifies that sending RAs should be disabled by default
          for (int i = 0; i < ift->getNumInterfaces(); i++){
             ift->getInterface(i)->ipv6Data()->setAdvSendAdvertisements(false);
          }

          device = xmlParser::GetDevice(deviceType, deviceId, configFile);
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
   else if(stage == 3)
     {
         device = xmlParser::GetDevice(deviceType, deviceId, configFile);
         if (device == NULL){
            ev << "No configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
            return;
         }

         if (xmlParser::isMulticastEnabled(device))
         {
             // get PIM interface table of this device
             pimIft = PimInterfaceTableAccess().get();
             if (pimIft == NULL){
                 throw cRuntimeError("PimInterfaces not found");
             }

             // is multicast routing enabled on the device?
             if (!xmlParser::isMulticastEnabled(device))
             {
                 EV<< "Multicast routing is not enable for this device (" << deviceType << " id=" << deviceId << ")" << endl;
             }
             else
             {
                 // fill pim interfaces table from config file
                 cXMLElement *iface = xmlParser::GetInterface(NULL, device);
                 if (iface != NULL)
                     loadPimInterfaceConfig(iface);
                 else
                     EV<< "No PIM interface is configured for this device (" << deviceType << " id=" << deviceId << ")" << endl;
             }
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

         ipv6 = IPv6Address(addrFull.substr(0, addrFull.find_last_of('/')).c_str());

         // IPv6NeighbourDiscovery doesn't implement DAD for non-link-local addresses
         // -> we have to set the address as non-tentative
         ie->ipv6Data()->assignAddress(ipv6, false, 0, 0);

         // adding directly connected route to the routing table
         IPv6Route *route = new IPv6Route(ipv6.getPrefix(prefixLen), prefixLen, IPv6Route::STATIC);
         route->setInterfaceId(ie->getInterfaceId());
         route->setNextHop(IPv6Address::UNSPECIFIED_ADDRESS);
         route->setMetric(0);

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


void DeviceConfigurator::loadDefaultRouter(cXMLElement *gateway){

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

bool DeviceConfigurator::readRoutingTableFromXml(const char *filename, const char *RouterId)
{
    cXMLElement* routerConfig = ev.getXMLDocument(filename);
    if (routerConfig == NULL) {
        return false;
    }

    // load information on this router
    std::string routerXPath("Router[@id='");
    routerXPath += RouterId;
    routerXPath += "']";

    cXMLElement* routerNode = routerConfig->getElementByPath(routerXPath.c_str());
    if (routerNode == NULL)
        opp_error("No configuration for Router ID: %s", RouterId);

    cXMLElement* IntNode = routerNode->getFirstChildWithTag("Interfaces");
    if (IntNode)
        readInterfaceFromXml(IntNode);

    cXMLElement* routingNode = routerNode->getFirstChildWithTag("Routing");
    if (routingNode){
       cXMLElement* staticNode = routingNode->getFirstChildWithTag("Static");
       if (staticNode)
          readStaticRouteFromXml(staticNode);
    }
    return true;
}

void DeviceConfigurator::readInterfaceFromXml(cXMLElement* Node)
{
    InterfaceEntry* ie;

    cXMLElementList intConfig = Node->getChildren();
    for (cXMLElementList::iterator intConfigIt = intConfig.begin(); intConfigIt != intConfig.end(); intConfigIt++)
    {
      std::string nodeName = (*intConfigIt)->getTagName();
      if (nodeName == "Interface" && (*intConfigIt)->getAttribute("name"))
      {
        std::string intName=(*intConfigIt)->getAttribute("name");
        std::string typeName=intName.substr(0,3);

        ie=ift->getInterfaceByName(intName.c_str());

        if (!ie)
          opp_error("Error in routing file: interface name `%s' not registered by any L2 module", intName.c_str());

        //implicitne nastavenia
        if (typeName=="eth")
              ie->setBroadcast(true);
        if (typeName=="ppp")
              ie->setPointToPoint(true);

        //register multicast groups
        ie->ipv4Data()->addMulticastListener(IPv4Address("224.0.0.1"));
        ie->ipv4Data()->addMulticastListener(IPv4Address("224.0.0.2"));

        ie->ipv4Data()->setMetric(1);
        ie->setMtu(1500);

        cXMLElementList ifDetails = (*intConfigIt)->getChildren();
        for (cXMLElementList::iterator ifElemIt = ifDetails.begin(); ifElemIt != ifDetails.end(); ifElemIt++)
        {
          std::string nodeName = (*ifElemIt)->getTagName();

          if (nodeName=="IPAddress")
          {
            ie->ipv4Data()->setIPAddress(IPv4Address((*ifElemIt)->getNodeValue()));
          }

          if (nodeName=="Mask")
          {
            ie->ipv4Data()->setNetmask(IPv4Address((*ifElemIt)->getNodeValue()));
          }

          if (nodeName=="MTU")
          {
            ie->setMtu(atoi((*ifElemIt)->getNodeValue()));
          }

        }

      }
    }
}

void DeviceConfigurator::readStaticRouteFromXml(cXMLElement* Node)
{
  cXMLElementList intConfig = Node->getChildren();
  for (cXMLElementList::iterator intConfigIt = intConfig.begin(); intConfigIt != intConfig.end(); intConfigIt++)
  {
    std::string nodeName = (*intConfigIt)->getTagName();
    if (nodeName == "Route")
    {
        IPv4Route *e = new IPv4Route();
        cXMLElementList ifDetails = (*intConfigIt)->getChildren();
        for (cXMLElementList::iterator ifElemIt = ifDetails.begin(); ifElemIt != ifDetails.end(); ifElemIt++)
        {
          std::string nodeName = (*ifElemIt)->getTagName();

          if (nodeName=="NetworkAddress")
          {
            e->setDestination(IPv4Address((*ifElemIt)->getNodeValue()));
            EV << "Address = " << e->getDestination() << endl;
          }

          if (nodeName=="NetworkMask")
          {
            e->setNetmask(IPv4Address((*ifElemIt)->getNodeValue()));
            EV << "NetworkMask = " << e->getNetmask() << endl;
          }

          if (nodeName=="NextHopAddress")
          {
            e->setGateway(IPv4Address((*ifElemIt)->getNodeValue()));
            InterfaceEntry *intf=NULL;
            for (int i=0; i<ift->getNumInterfaces(); i++)
            {
              intf = ift->getInterface(i);
              if (((intf->ipv4Data()->getIPAddress()).doAnd(intf->ipv4Data()->getNetmask()))==((e->getGateway()).doAnd(intf->ipv4Data()->getNetmask())))
                  break;

            }
            if (intf)
              e->setInterface(intf);
            else
              opp_error("Error.");
            e->setMetric(1);
          }
          if (nodeName=="ExitInterface")
          {
            InterfaceEntry *ie=ift->getInterfaceByName((*ifElemIt)->getNodeValue());
            if (!ie)
                opp_error("Interface does not exists");

            e->setInterface(ie);
            e->setGateway(IPv4Address::UNSPECIFIED_ADDRESS);
            e->setMetric(0);
          }
          if (nodeName=="StaticRouteMetric")
          {
            e->setMetric(atoi((*ifElemIt)->getNodeValue()));
          }
        }
        rt->addRoute(e);
    }

  }
}


//
//
//- configuration for RIPng
//
//
void DeviceConfigurator::loadRIPngConfig(RIPngRouting *RIPngModule){

    ASSERT(RIPngModule != NULL);

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
    cXMLElement *interface;
    RIPng::Interface *ripngInterface;
    std::string RIPngInterfaceStatus;
    std::string RIPngInterfacePassiveStatus;
    std::string RIPngInterfaceSplitHorizon;
    std::string RIPngInterfacePoisonReverse;

      //get first router's interface
      interface = xmlParser::GetInterface(NULL, device);
      while (interface != NULL)
      {// process all interfaces in config file
          const char *interfaceName = interface->getAttribute("name");
          RIPngInterfaceStatus = xmlParser::getInterfaceRIPngStatus(interface);
          if (RIPngInterfaceStatus == "enable")
          {
              ripngInterface = RIPngModule->enableRIPngOnInterface(interfaceName);
              // add prefixes from int to the RIPng routing table
              loadPrefixesFromInterfaceToRIPngRT(RIPngModule, interface);

              RIPngInterfacePassiveStatus = xmlParser::getRIPngInterfacePassiveStatus(interface);
              if (RIPngInterfacePassiveStatus == "enable")
              {// set the interface as passive (interface is "active" by default)
                  RIPngModule->setInterfacePassiveStatus(ripngInterface, true);
              }

              RIPngInterfaceSplitHorizon = xmlParser::getRIPngInterfaceSplitHorizon(interface);
              if (RIPngInterfaceSplitHorizon == "disable")
              {// disable Split Horizon on the interface (Split Horizon is enabled by default)
                  RIPngModule->setInterfaceSplitHorizon(ripngInterface, false);
              }

              RIPngInterfacePoisonReverse = xmlParser::getRIPngInterfacePoisonReverse(interface);
              if (RIPngInterfacePoisonReverse == "enable")
              {// enable Poison Reverse on the interface (Poison Reverse is disabled by default)
                  RIPngModule->setInterfacePoisonReverse(ripngInterface, true);
              }
          }

          // process next interface
          interface = xmlParser::GetInterface(interface, NULL);
      }
}

void DeviceConfigurator::loadPrefixesFromInterfaceToRIPngRT(RIPngRouting *RIPngModule, cXMLElement *interface)
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
               throw cRuntimeError("Unable to set IPv6 network address %s for static route", sIpv6address.c_str());
            }

            if (!ipv6address.isLinkLocal() && !ipv6address.isMulticast())
            {
                // make directly connected route
                route = new RIPng::RoutingTableEntry(ipv6address.getPrefix(prefixLen), prefixLen);
                route->setInterfaceId(interfaceId);
                route->setNextHop(IPv6Address::UNSPECIFIED_ADDRESS);  // means directly connected network
                route->setMetric(RIPngModule->getConnNetworkMetric());
                RIPngModule->addRoutingTableEntry(route, false);

                // directly connected routes do not need a RIPng route timer
            }

            eIpv6address = xmlParser::GetIPv6Address(eIpv6address, NULL);
        }
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
          continue;

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

        for(int i = 0; i < (intMulticastAddresses.size()); i++)
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
        //TODO

        //IPv6
        //TODO

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
        InterfaceEntry *entryIFT = ift->getInterface(i);
        if (interfaces == NULL)
        {
            this->loadISISInterfaceDefaultConfig(isisModule, entryIFT);
        }
        else
        {
            this->loadISISInterfaceConfig(isisModule, entryIFT,
                    interfaces->getFirstChildWithAttribute("Interface", "name", entryIFT->getName()));

        }
    }
}


void DeviceConfigurator::loadISISCoreDefaultConfig(ISIS *isisModule){
    //NET

          isisModule->generateNetAddr();


      //IS type {L1(L2_ISIS_MODE) | L2 | L1L2 default for L3_ISIS_MODE}

      isisModule->setIsType(L1_TYPE);


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


void DeviceConfigurator::loadISISInterfaceDefaultConfig(ISIS *isisModule, InterfaceEntry *entry){

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

        //set network type (point-to-point vs. broadcast)

        newIftEntry.network = true; //default value means broadcast TODO check with TRILL default values

        //set interface metric

        newIftEntry.metric = ISIS_METRIC;    //default value

        //set interface type according to global router configuration
        newIftEntry.circuitType = isisModule->getIsType();


        //set L1 hello interval in seconds
        newIftEntry.L1HelloInterval = isisModule->getL1HelloInterval();


        //set L1 hello multiplier
        newIftEntry.L1HelloMultiplier = isisModule->getL1HelloMultiplier();


        //set L2 hello interval in seconds
        newIftEntry.L2HelloInterval = isisModule->getL2HelloInterval();


        //set L2 hello multiplier
        newIftEntry.L2HelloMultiplier = isisModule->getL2HelloMultiplier();

        //set lspInterval
        newIftEntry.lspInterval = isisModule->getLspInterval();

        //set L1CsnpInterval
        newIftEntry.L1CsnpInterval = isisModule->getL1CsnpInterval();

        //set L2CsnpInterval
        newIftEntry.L2CsnpInterval = isisModule->getL2CsnpInterval();

        //set L1PsnpInterval
        newIftEntry.L1PsnpInterval = isisModule->getL1PsnpInterval();

        //set L2PsnpInterval
        newIftEntry.L2PsnpInterval = isisModule->getL2PsnpInterval();

        // priority is not needed for point-to-point, but it won't hurt
        // set priority of current DIS = me at start
        newIftEntry.L1DISpriority = newIftEntry.priority;
        newIftEntry.L2DISpriority = newIftEntry.priority;

        //set initial designated IS as himself

        memcpy(newIftEntry.L1DIS,isisModule->getSysId(), ISIS_SYSTEM_ID);
        //set LAN identifier; -99 is because, OMNeT starts numbering interfaces from 100 -> interfaceID 100 means LAN ID 0; and we want to start numbering from 1
        //newIftEntry.L1DIS[6] = entry->getInterfaceId() - 99;
        newIftEntry.L1DIS[ISIS_SYSTEM_ID] = isisModule->getISISIftSize() + 1;
        //do the same for L2 DIS

        memcpy(newIftEntry.L2DIS,isisModule->getSysId(), ISIS_SYSTEM_ID);
        //newIftEntry.L2DIS[6] = entry->getInterfaceId() - 99;
        newIftEntry.L2DIS[ISIS_SYSTEM_ID] = isisModule->getISISIftSize() + 1;

        newIftEntry.passive = false;
        newIftEntry.entry = entry;

    //    this->ISISIft.push_back(newIftEntry);
        isisModule->appendISISInterface(newIftEntry);
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

/* End of IS-IS related methods */
