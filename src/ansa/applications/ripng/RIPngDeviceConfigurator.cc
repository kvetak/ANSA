/*
 * RIPngDeviceConfigurator.cc
 *
 *  Created on: Oct 22, 2015
 *      Author: root
 */

#include "RIPngDeviceConfigurator.h"
/*#include "IPv6Address.h"
#include "IPv6InterfaceData.h"
#include "IPv4Address.h"*/
#include <errno.h>


//Define_Module(RIPngDeviceConfigurator);

using namespace std;

/*void RIPngDeviceConfigurator::initialize(int stage){

    if (stage == 0)
    {

        // these variables needs to be set only once
        deviceType = par("deviceType");
        deviceId = par("deviceId");
        configFile = par("configFile");

        device = GetDevice(deviceType, deviceId, configFile);
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
                cXMLElement *iface = GetInterface(NULL, device);
                if (iface == NULL)
                    ev << "No IPv4 interface configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
                else
                    loadInterfaceConfig(iface);

                // configure static routing
                cXMLElement *route = GetStaticRoute(NULL, device);
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
            cXMLElement *iface = GetInterface(NULL, device);
            if (iface == NULL)
                ev << "No IPv6 interface configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
            else
                loadInterfaceConfig6(iface);

            // configure static routing
            cXMLElement *route = GetStaticRoute6(NULL, device);
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

        if (isMulticastEnabled(device))
        {
            // get PIM interface table of this device
            pimIft = PimInterfaceTableAccess().get();
            if (pimIft == NULL)
                throw cRuntimeError("PimInterfaces not found");

            // fill pim interfaces table from config file
            cXMLElement *iface = GetInterface(NULL, device);
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
        else if (gateway != NULL ) {
            loadDefaultRouter(gateway);
            gateway = gateway->getNextSiblingWithTag("DefaultRouter");
            if (gateway)
                loadDefaultRouter(gateway);
        }


    }
    else if(stage == 10)
    {
        if(device == NULL)
            return;

        cXMLElement *iface = GetInterface(NULL, device);
        addIPv4MulticastGroups(iface);
        addIPv6MulticastGroups(iface);
    }
}*/


/*void RIPngDeviceConfigurator::handleMessage(cMessage *msg){
   throw cRuntimeError("This module does not receive messages");
   delete msg;
}*/

/*void RIPngDeviceConfigurator::loadStaticRouting6(cXMLElement *route){

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
         if (!Str2Int(&adminDistance, distance->getNodeValue())){
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
      route = GetStaticRoute6(route, NULL);
   }
}*/

/*void RIPngDeviceConfigurator::loadStaticRouting(cXMLElement* route)
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

        route = GetStaticRoute(route, NULL);
    }
}*/


RIPngDeviceConfigurator::RIPngDeviceConfigurator() {
    deviceId = NULL;
    deviceType = NULL;
    configFile = NULL;
}

RIPngDeviceConfigurator::RIPngDeviceConfigurator(const char* devId,
        const char* devType, const char* confFile, IInterfaceTable* intf)
: deviceType(devType), deviceId(devId), configFile(confFile), ift(intf)
{
}

RIPngDeviceConfigurator::~RIPngDeviceConfigurator() {
    deviceId = NULL;
    deviceType = NULL;
    configFile = NULL;
}

void RIPngDeviceConfigurator::loadRIPngConfig(RIPngRouting *RIPngModule)
{
    ASSERT(RIPngModule != NULL);

    // get access to device node from XML
    //const char *deviceType = par("deviceType");
    //const char *deviceId = par("deviceId");
    //const char *configFile = par("configFile");
    cXMLElement *device = GetDevice(deviceType, deviceId, configFile);

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

      cXMLElement *RIPngProcessElement = GetRIPngProcess(NULL, device);
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

              RIPngParamString = GetNodeParamConfig(RIPngProcessElement, "Port", "");
              if (strcmp(RIPngParamString, "") != 0)
              {// set port and address
                  int port;
                  if (Str2Int(&port, RIPngParamString))
                  {
                      RIPngParamString = GetNodeParamConfig(RIPngProcessElement, "Address", "");
                      if (strcmp(RIPngParamString, "") != 0)
                      {
                          IPv6Address RIPngAddress = IPv6Address(RIPngParamString);
                          RIPngModule->setPortAndAddress(RIPngProcess, port, RIPngAddress);
                      }
                  }
              }

              RIPngParamString = GetNodeParamConfig(RIPngProcessElement, "Distance", "");
              if (strcmp(RIPngParamString, "") != 0)
              {// set distance (AD)
                  int distance;
                  if (Str2Int(&distance, RIPngParamString))
                  {
                      RIPngModule->setDistance(RIPngProcess, distance);
                  }
              }

              RIPngProcessTimersElement = GetRIPngProcessTimers(RIPngProcessElement);
              if (RIPngProcessTimersElement != NULL)
              {// set timers
                  int update, route, garbage;
                  RIPngParamString = GetNodeParamConfig(RIPngProcessTimersElement, "Update", "-1");
                  if (!Str2Int(&update, RIPngParamString))
                      update = -1;
                  RIPngParamString = GetNodeParamConfig(RIPngProcessTimersElement, "Route", "-1");
                  if (!Str2Int(&route, RIPngParamString))
                      route = -1;
                  RIPngParamString = GetNodeParamConfig(RIPngProcessTimersElement, "Garbage", "-1");
                  if (!Str2Int(&garbage, RIPngParamString))
                      garbage = -1;

                  RIPngModule->setTimers(RIPngProcess, update, route, garbage);
              }

              //Must be set before any INTERFACE PoisonReverse command
              RIPngParamString = GetNodeParamConfig(RIPngProcessElement, "PoisonReverse", "");
              if (strcmp(RIPngParamString, "enable") == 0)
              {// enable Poison Reverse (Poison Reverse is disabled by default)
                  RIPngModule->setPoisonReverse(RIPngProcess, true);
              }

              //Must be set before any INTERFACE SplitHorizon command
              RIPngParamString = GetNodeParamConfig(RIPngProcessElement, "SplitHorizon", "");
              if (strcmp(RIPngParamString, "disable") == 0)
              {// // disable Split Horizon (Split Horizon is enabled by default)
                  RIPngModule->setSplitHorizon(RIPngProcess, false);
              }
          }

          RIPngProcessElement = GetRIPngProcess(RIPngProcessElement, NULL);
      }

    // interfaces config
    cXMLElement *interfaceElement;
    cXMLElement *interfaceRIPngProcessElement;
    cXMLElement *interfaceRIPngDefaultInfElement;
    const char *interfaceRIPngProcessName;
    const char *interfaceRIPngParamString;
    RIPng::Interface *ripngInterface;

      //get first router's interface
      interfaceElement = GetInterface(NULL, device);
      while (interfaceElement != NULL)
      {// process all interfaces in config file
          const char *interfaceName = interfaceElement->getAttribute("name");
          interfaceRIPngProcessElement = GetInterfaceRIPngProcess(NULL, interfaceElement);
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

                  interfaceRIPngParamString = GetInterfaceRIPngPassiveStatus(interfaceRIPngProcessElement);
                  if (strcmp(interfaceRIPngParamString, "enable") == 0)
                  {// set the interface as passive (interface is "active" by default)
                      RIPngProcess->setInterfacePassiveStatus(ripngInterface, true);
                  }

                  interfaceRIPngParamString = GetInterfaceRIPngSplitHorizon(interfaceRIPngProcessElement);
                  if (strcmp(interfaceRIPngParamString, "disable") == 0)
                  {// disable Split Horizon on the interface (Split Horizon is enabled by default)
                      RIPngProcess->setInterfaceSplitHorizon(ripngInterface, false);
                  }

                  interfaceRIPngParamString = GetInterfaceRIPngPoisonReverse(interfaceRIPngProcessElement);
                  if (strcmp(interfaceRIPngParamString, "enable") == 0)
                  {// enable Poison Reverse on the interface (Poison Reverse is disabled by default)
                      RIPngProcess->setInterfacePoisonReverse(ripngInterface, true);
                  }

                  int metricOffset;
                  interfaceRIPngParamString = GetInterfaceRIPngMetricOffset(interfaceRIPngProcessElement);
                  if (Str2Int(&metricOffset, interfaceRIPngParamString) && metricOffset != 0)
                  {// metric-offset
                      RIPngProcess->setInterfaceMetricOffset(ripngInterface, metricOffset);
                  }

                  interfaceRIPngDefaultInfElement = GetInterfaceRIPngDefaultInformation(interfaceRIPngProcessElement);
                  if (interfaceRIPngDefaultInfElement != NULL)
                  {// default-information
                      bool defRouteOnly = false;
                      interfaceRIPngParamString = GetNodeParamConfig(interfaceRIPngDefaultInfElement, "DefaultOnly", "false");
                      Str2Bool(&defRouteOnly, interfaceRIPngParamString);

                      int metric = 0;
                      interfaceRIPngParamString = GetNodeParamConfig(interfaceRIPngDefaultInfElement, "Metric", "0");
                      Str2Int(&metric, interfaceRIPngParamString);

                      RIPngProcess->setInterfaceDefaultInformation(ripngInterface, true, defRouteOnly, metric);
                  }

              }

              // process next RIPng on the interface
              interfaceRIPngProcessElement = GetInterfaceRIPngProcess(interfaceRIPngProcessElement, NULL);

          }

          // process next interface
          interfaceElement = GetInterface(interfaceElement, NULL);
      }
}

void RIPngDeviceConfigurator::loadPrefixesFromInterfaceToRIPngRT(RIPngProcess *process, cXMLElement *interface)
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
        eIpv6address = GetIPv6Address(NULL, interface);
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

            eIpv6address = GetIPv6Address(eIpv6address, NULL);
        }
}



/*void RIPngDeviceConfigurator::loadInterfaceConfig6(cXMLElement *iface){

   // for each interface node
   while (iface != NULL){

      // get interface name and find matching interface in interface table
      const char *ifaceName = iface->getAttribute("name");
      InterfaceEntry *ie = ift->getInterfaceByName(ifaceName);
      if (ie == NULL){
         //throw cRuntimeError("No interface called %s on this device", ifaceName);
         EV << "No interface called %s on this device" << ifaceName << endl;
         // get next interface
         iface = GetInterface(iface, NULL);
         continue;
      }

      // for each IPv6 address
      cXMLElement *addr = GetIPv6Address(NULL, iface);
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
         addr = GetIPv6Address(addr, NULL);
      }

      setInterfaceParamters(ie); //TODO - verify

      // for each parameter
      for (cXMLElement *param = iface->getFirstChild(); param; param = param->getNextSibling()){

         if(strcmp(param->getTagName(), "NdpAdvSendAdvertisements") == 0){
            bool value = false;
            if (!Str2Bool(&value, param->getNodeValue())){
               throw cRuntimeError("Invalid NdpAdvSendAdvertisements value on interface %s", ie->getName());
            }
            ie->ipv6Data()->setAdvSendAdvertisements(value);
         }

         if(strcmp(param->getTagName(), "NdpMaxRtrAdvInterval") == 0){
            int value = 0;
            if (!Str2Int(&value, param->getNodeValue())){
               throw cRuntimeError("Unable to parse valid NdpMaxRtrAdvInterval %s on interface %s", value, ifaceName);
            }
            if (value < 4 || value > 1800){
               value = 600;
            }
            ie->ipv6Data()->setMaxRtrAdvInterval(value);
         }

         if(strcmp(param->getTagName(), "NdpMinRtrAdvInterval") == 0){
            int value = 0;
            if (!Str2Int(&value, param->getNodeValue())){
               throw cRuntimeError("Unable to parse valid NdpMinRtrAdvInterval %s on interface %s", value, ifaceName);
            }
            if (value < 3 || value > 1350){
               value = 200;
            }
            ie->ipv6Data()->setMinRtrAdvInterval(value);
         }
      }

      // for each IPv6 prefix
      cXMLElement *prefix = GetAdvPrefix(NULL, iface);
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
            if (!Str2Int(&value, validLifeTime)){
               throw cRuntimeError("Unable to parse valid lifetime %s on IPv6 prefix %s on interface %s", validLifeTime, addrFull.c_str(), ifaceName);
            }
            advPrefix.advValidLifetime = value;
         }

         value = 604800;
         if (preferredLifeTime != NULL){
            if (!Str2Int(&value, preferredLifeTime)){
               throw cRuntimeError("Unable to parse preferred lifetime %s on IPv6 prefix %s on interface %s", preferredLifeTime, addrFull.c_str(), ifaceName);
            }
            advPrefix.advPreferredLifetime = value;
         }

         advPrefix.advOnLinkFlag = true;
         advPrefix.advAutonomousFlag = true;

         // adding prefix
         ie->ipv6Data()->addAdvPrefix(advPrefix);

         // get next IPv6 address
         prefix = GetAdvPrefix(prefix, NULL);
      }



      // get next interface
      iface = GetInterface(iface, NULL);
   }
}*/


/*void RIPngDeviceConfigurator::loadDefaultRouter6(cXMLElement *gateway){

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
}*/


/*void RIPngDeviceConfigurator::loadPimInterfaceConfig(cXMLElement *iface)
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
        iface = GetInterface(iface, NULL);
    }
}*/


/*void RIPngDeviceConfigurator::loadDefaultRouter(cXMLElement *gateway)
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
}*/

/*void RIPngDeviceConfigurator::addIPv4MulticastGroups(cXMLElement *iface)
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
        iface = GetInterface(iface, NULL);
    }

}*/

/*void RIPngDeviceConfigurator::addIPv6MulticastGroups(cXMLElement *iface)
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
        iface = GetInterface(iface, NULL);
    }

}*/


/*void RIPngDeviceConfigurator::loadInterfaceConfig(cXMLElement* iface)
{
    AnsaRoutingTable *ANSArt = dynamic_cast<AnsaRoutingTable *>(rt);

    while (iface != NULL)
    {
        // get interface name and find matching interface in interface table
        const char *ifaceName = iface->getAttribute("name");
        InterfaceEntry *ie = ift->getInterfaceByName(ifaceName);

        if (ie == NULL) {
           //throw cRuntimeError("No interface called %s on this device", ifaceName);
            EV << "No interface " << ifaceName << "called on this device" << endl;
            iface = GetInterface(iface, NULL);
            continue;
        }

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
                if (!Str2Int(&tempNumber, (*ifElemIt)->getNodeValue()))
                    throw cRuntimeError("Bad value for bandwidth on interface %s", ifaceName);
                ie->setBandwidth(tempNumber);
            }

            if (nodeName=="Delay")
            { // Delay in tens of microseconds
                if (!Str2Int(&tempNumber, (*ifElemIt)->getNodeValue()))
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
            if (!(ie->ipv4Data()->getIPAddress().isUnspecified()
                    && ie->ipv4Data()->getNetmask() == IPv4Address::ALLONES_ADDRESS ) )
                ANSArt->addRoute(route);
        }

        iface = GetInterface(iface, NULL);
    }
}*/


/*void RIPngDeviceConfigurator::setInterfaceParamters(InterfaceEntry *interface)
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
}*/


/*double RIPngDeviceConfigurator::getDefaultDelay(const char *linkType)
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
}*/


cXMLElement * RIPngDeviceConfigurator::GetDevice(const char *deviceType, const char *deviceId, const char *configFile){

   // get access to the XML file (if exists)
   cXMLElement *config = ev.getXMLDocument(configFile);
   if (config == NULL)
      return NULL;


   string type = deviceType;
   string id = deviceId;
   if (type.empty() || id.empty())
      return NULL;

   // create string that describes device node, such as <Router id="10.0.0.1">
   string deviceNodePath = type;
   deviceNodePath += "[@id='";
   deviceNodePath += id;
   deviceNodePath += "']";

   // get access to the device node
   cXMLElement *device = config->getElementByPath(deviceNodePath.c_str());

   return device;
}


cXMLElement * RIPngDeviceConfigurator::GetInterface(cXMLElement *iface, cXMLElement *device){

   // initial call of the method - find <Interfaces> and get first "Interface" node
   if (device != NULL){

      cXMLElement *ifaces = device->getFirstChildWithTag("Interfaces");
      if (ifaces == NULL)
         return NULL;

      iface = ifaces->getFirstChildWithTag("Interface");

   // repeated call - get another "Interface" sibling node
   }else if (iface != NULL){
      iface = iface->getNextSiblingWithTag("Interface");
   }else{
      iface = NULL;
   }

   return iface;
}

cXMLElement * RIPngDeviceConfigurator::GetIPv6Address(cXMLElement *addr, cXMLElement *iface){

   // initial call of the method - get first "IPv6Address" child node
   if (iface != NULL){
      addr = iface->getFirstChildWithTag("IPv6Address");

   // repeated call - get another "IPv6Address" sibling node
   }else if (addr != NULL){
      addr = addr->getNextSiblingWithTag("IPv6Address");
   }else{
      addr = NULL;
   }

   return addr;
}

/*cXMLElement * RIPngDeviceConfigurator::GetAdvPrefix(cXMLElement *prefix, cXMLElement *iface){

   // initial call of the method - get first "NdpAdvPrefix" child node
   if (iface != NULL){
      prefix = iface->getFirstChildWithTag("NdpAdvPrefix");

   // repeated call - get another "NdpAdvPrefix" sibling node
   }else if (prefix != NULL){
      prefix = prefix->getNextSiblingWithTag("NdpAdvPrefix");
   }else{
      prefix = NULL;
   }

   return prefix;
}*/

/*cXMLElement * RIPngDeviceConfigurator::GetStaticRoute6(cXMLElement *route, cXMLElement *device){

   // initial call of the method - find <Routing> -> <Static>
   // and then get first "Route" child node
   if (device != NULL){

      cXMLElement *routing = device->getFirstChildWithTag("Routing6");
      if (routing == NULL)
         return NULL;

      cXMLElement *staticRouting = routing->getFirstChildWithTag("Static");
      if (staticRouting == NULL)
         return NULL;

      route = staticRouting->getFirstChildWithTag("Route");

   // repeated call - get another "Route" sibling node
   }else if (route != NULL){
      route = route->getNextSiblingWithTag("Route");
   }else{
      route = NULL;
   }

   return route;
}*/

/*cXMLElement * RIPngDeviceConfigurator::GetStaticRoute(cXMLElement *route, cXMLElement *device){

   // initial call of the method - find <Routing6> -> <Static>
   // and then get first "Route" child node
   if (device != NULL){

      cXMLElement *routing = device->getFirstChildWithTag("Routing");
      if (routing == NULL)
         return NULL;

      cXMLElement *staticRouting = routing->getFirstChildWithTag("Static");
      if (staticRouting == NULL)
         return NULL;

      route = staticRouting->getFirstChildWithTag("Route");

   // repeated call - get another "Route" sibling node
   }else if (route != NULL){
      route = route->getNextSiblingWithTag("Route");
   }else{
      route = NULL;
   }

   return route;
}*/


const char *RIPngDeviceConfigurator::GetNodeParamConfig(cXMLElement *node, const char *paramName, const char *defaultValue)
{
    ASSERT(node != NULL);

    cXMLElement* paramElem = node->getElementByPath(paramName);
    if (paramElem == NULL)
        return defaultValue;

    const char *paramValue = paramElem->getNodeValue();
    if (paramValue == NULL)
        return defaultValue;

    return paramValue;
}


/*
 * A utility method for proper str -> int conversion with error checking.
 */
bool RIPngDeviceConfigurator::Str2Int(int *retValue, const char *str){

   if (retValue == NULL || str == NULL){
      return false;
   }

   char *tail = NULL;
   long value = 0;
   errno = 0;

   value = strtol(str, &tail, 0);

   if (*tail != '\0' || errno == ERANGE || errno == EINVAL || value < INT_MIN || value > INT_MAX){
      return false;
   }

   *retValue = (int) value;
   return true;
}


bool RIPngDeviceConfigurator::Str2Bool(bool *ret, const char *str){

   if (  (strcmp(str, "yes") == 0)
      || (strcmp(str, "enabled") == 0)
      || (strcmp(str, "on") == 0)
      || (strcmp(str, "true") == 0)){

      *ret = true;
      return true;
   }

   if (  (strcmp(str, "no") == 0)
      || (strcmp(str, "disabled") == 0)
      || (strcmp(str, "off") == 0)
      || (strcmp(str, "false") == 0)){

      *ret = false;
      return true;
   }

   int value;
   if (Str2Int(&value, str)){
      if (value == 1){
         *ret = true;
         return true;
      }

      if (value == 0){
         *ret = false;
         return true;
      }
   }

   return false;
}


/*bool RIPngDeviceConfigurator::isMulticastEnabled(cXMLElement *device)
{
    // Routing element
    cXMLElement* routingNode = device->getElementByPath("Routing");
    if (routingNode == NULL)
         return false;

    // Multicast element
    cXMLElement* multicastNode = routingNode->getElementByPath("Multicast");
    if (multicastNode == NULL)
       return false;


    // Multicast has to be enabled
    const char* enableAtt = multicastNode->getAttribute("enable");
    if (strcmp(enableAtt, "1"))
        return false;

    return true;
}*/


cXMLElement *RIPngDeviceConfigurator::GetInterfaceRIPngProcess(cXMLElement *ripng, cXMLElement *iface)
{
    // initial call of the method - get first "RIPng" child node
    if (iface != NULL)
    {
        ripng = iface->getFirstChildWithTag("RIPng");
    // repeated call - get another "RIPng" sibling node
    }
    else if (ripng != NULL)
    {
        ripng = ripng->getNextSiblingWithTag("RIPng");
    }
    else
    {
        ripng = NULL;
    }

    return ripng;
}

const char *RIPngDeviceConfigurator::GetInterfaceRIPngPassiveStatus(cXMLElement *ripng)
{
    return GetNodeParamConfig(ripng, "PassiveInterface", "disable");
}

const char *RIPngDeviceConfigurator::GetInterfaceRIPngSplitHorizon(cXMLElement *ripng)
{
    return GetNodeParamConfig(ripng, "SplitHorizon", "enable");
}

const char *RIPngDeviceConfigurator::GetInterfaceRIPngPoisonReverse(cXMLElement *ripng)
{
    return GetNodeParamConfig(ripng, "PoisonReverse", "disable");
}

cXMLElement *RIPngDeviceConfigurator::GetInterfaceRIPngDefaultInformation(cXMLElement *ripng)
{
    return ripng->getFirstChildWithTag("DefaultInformation");
}

const char  *RIPngDeviceConfigurator::GetInterfaceRIPngMetricOffset(cXMLElement *ripng)
{
    return GetNodeParamConfig(ripng, "MetricOffset", "0");
}

cXMLElement *RIPngDeviceConfigurator::GetRIPngProcess(cXMLElement *process, cXMLElement *device)
{
    // initial call of the method - find <Routing6> -> <RIPng>
    // and then get first "RIPng" child node
    if (device != NULL)
    {
        cXMLElement *routing = device->getFirstChildWithTag("Routing6");
        if (routing == NULL)
            return NULL;

        process = routing->getFirstChildWithTag("RIPng");

    // repeated call - get another "RIPng" sibling node
    }
    else if (process != NULL)
    {
        process = process->getNextSiblingWithTag("RIPng");
    }
    else
    {
        process = NULL;
    }

    return process;
}

cXMLElement *RIPngDeviceConfigurator::GetRIPngProcessTimers(cXMLElement *process)
{
    return process->getFirstChildWithTag("Timers");
}

