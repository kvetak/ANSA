/*
 * BabelDeviceConfigurator.cc
 *
 *  Created on: Oct 22, 2015
 *      Author: root
 */

#include "BabelDeviceConfigurator.h"
//#include "IPv6Address.h"
//#include "IPv6InterfaceData.h"
//#include "IPv4Address.h"
#include <errno.h>

namespace inet {
//Define_Module(BabelDeviceConfigurator);

using namespace std;
/*
void BabelDeviceConfigurator::initialize(int stage){

    if (stage == 0)
    {

        // these variables needs to be set only once
        deviceType = par("deviceType");
        deviceId = par("deviceId");
        configFile = par("configFile");

        device = GetDevice(deviceType, deviceId, configFile);
        if (device == nullptr)
        {
            ev << "No configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
            return;
        }
    }
    else if (stage == 2)
    {// interfaces and routing table are not ready before stage 2
        // get table of interfaces of this device

        ift = InterfaceTableAccess().get();
        if (ift == nullptr)
            throw cRuntimeError("InterfaceTable not found");

        //////////////////////////
        /// IPv4 Configuration ///
        //////////////////////////
        // get routing table of this device
        rt = RoutingTableAccess().getIfExists();
        if (rt != nullptr)
        {
            for (int i=0; i<ift->getNumInterfaces(); ++i)
            {
                //ift->getInterface(i)->setMulticast(true);
                rt->configureInterfaceForIPv4(ift->getInterface(i));
            }

            if (device != nullptr)
            {
                cXMLElement *iface = GetInterface(nullptr, device);
                if (iface == nullptr)
                    ev << "No IPv4 interface configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
                else
                    loadInterfaceConfig(iface);

                // configure static routing
                cXMLElement *route = GetStaticRoute(nullptr, device);
                if (route == nullptr && strcmp(deviceType, "Router") == 0)
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
        if (rt6 != nullptr)
        {
            // RFC 4861 specifies that sending RAs should be disabled by default
            for (int i = 0; i < ift->getNumInterfaces(); i++)
                ift->getInterface(i)->ipv6Data()->setAdvSendAdvertisements(false);

            if (device == nullptr)
                return;

            // configure interfaces - addressing
            cXMLElement *iface = GetInterface(nullptr, device);
            if (iface == nullptr)
                ev << "No IPv6 interface configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
            else
                loadInterfaceConfig6(iface);

            // configure static routing
            cXMLElement *route = GetStaticRoute6(nullptr, device);
            if (route == nullptr && strcmp(deviceType, "Router") == 0)
                ev << "No IPv6 static routing configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
            else
                loadStaticRouting6(route);

            // Adding default route requires routing table lookup to pick the right output
            // interface. This needs to be performed when all IPv6 addresses are already assigned
            // and there are matching records in the routing table.
            cXMLElement *gateway = device->getFirstChildWithTag("DefaultRouter6");
            if (gateway == nullptr && strcmp(deviceType, "Host") == 0)
                ev << "No IPv6 default-router configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
            else
                loadDefaultRouter6(gateway);
        }
    }
    else if(stage == 3)
    {
        if (device == nullptr)
            return;

        if (isMulticastEnabled(device))
        {
            // get PIM interface table of this device
            pimIft = PimInterfaceTableAccess().get();
            if (pimIft == nullptr)
                throw cRuntimeError("PimInterfaces not found");

            // fill pim interfaces table from config file
            cXMLElement *iface = GetInterface(nullptr, device);
            if (iface != nullptr)
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
        if (device == nullptr)
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
        if (gateway == nullptr && strcmp(deviceType, "Host") == 0)
            ev << "No IPv4 default-router configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
        else if (gateway != nullptr ) {
            loadDefaultRouter(gateway);
            gateway = gateway->getNextSiblingWithTag("DefaultRouter");
            if (gateway)
                loadDefaultRouter(gateway);
        }


    }
    else if(stage == 10)
    {
        if(device == nullptr)
            return;

        cXMLElement *iface = GetInterface(nullptr, device);
        addIPv4MulticastGroups(iface);
        addIPv6MulticastGroups(iface);
    }
}
*/
/*
void BabelDeviceConfigurator::handleMessage(cMessage *msg){
   throw cRuntimeError("This module does not receive messages");
   delete msg;
}

void BabelDeviceConfigurator::loadStaticRouting6(cXMLElement *route){

   // for each static route
   while (route != nullptr){

      // get network address string with prefix
      cXMLElement *network = route->getFirstChildWithTag("NetworkAddress");
      if (network == nullptr){
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
      if (nextHop == nullptr){
         throw cRuntimeError("IPv6 next hop address for static route not set");
      }

      IPv6Address addrNextHop = IPv6Address(nextHop->getNodeValue());


      // optinal argument - administrative distance is set to 1 if not set
      cXMLElement *distance = route->getFirstChildWithTag("AdministrativeDistance");
      int adminDistance = 1;
      if (distance != nullptr){
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
      if (record == nullptr){
         ev << "No directly connected route for IPv6 next hop address " << addrNextHop << " found" << endl;
      }else{
         // add static route
         rt6->addStaticRoute(addrNetwork, prefixLen, record->getInterfaceId(), addrNextHop, adminDistance);
      }


      // get next static route
      route = GetStaticRoute6(route, nullptr);
   }
}

void BabelDeviceConfigurator::loadStaticRouting(cXMLElement* route)
{
    AnsaRoutingTable *ANSArt = dynamic_cast<AnsaRoutingTable *>(rt);

    while (route != nullptr)
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
                InterfaceEntry *intf=nullptr;
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
        if (ANSArt != nullptr)
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

        route = GetStaticRoute(route, nullptr);
    }
}
*/
//
// - Babel configuration
//

void BabelDeviceConfigurator::loadBabelConfig(BabelMain *bMain)
{
    ASSERT(bMain != nullptr);

    device = GetDevice(deviceType, deviceId, configFile);
    if (device == nullptr)
    {
        ev << "No configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
        return;
    }

    if (device == nullptr)
    {
       EV << "No configuration found for this device (" << deviceType <<
               " id=" << deviceId << ")" << endl;
       return;
    }

    loadBabelProcessConfig(device, bMain);

    loadBabelInterfacesConfig(device, bMain);
}

void BabelDeviceConfigurator::loadBabelProcessConfig(cXMLElement *device, BabelMain *bMain)
{
    ASSERT(bMain != nullptr);
    ASSERT(device != nullptr);

    cXMLElement *processElem = nullptr;
    cXMLElement *routerIdElem = nullptr;
    cXMLElement *portElem = nullptr;

    //Check if Babel routing is enabled
    processElem = GetBabelProcess(device);
    if (processElem == nullptr)
    {// Babel is not enabled
        EV << "No Babel configuration found." << endl;
        return;
    }

    //Find explicitly specified routerId, or generate EUI64
    routerIdElem = processElem->getFirstChildWithTag("RouterId");
    if(routerIdElem == nullptr)
    {// not found -> generate
        bMain->generateRouterId();

    }
    else
    {// found -> try parse
        string ridstr = routerIdElem->getNodeValue();

        //trim string
        ridstr.erase(ridstr.find_last_not_of(" \t\n\r\f\v") + 1);
        ridstr.erase(0, ridstr.find_first_not_of(" \t\n\r\f\v"));


        //count colons
        if(std::count(ridstr.begin(), ridstr.end(), ':') == 3)
        {
            //locate colons
            size_t colon1 = ridstr.find(':', 0);
            size_t colon2 = ridstr.find(':', colon1 + 1);
            size_t colon3 = ridstr.find(':', colon2 + 1);

            //convert strings to uint
            uint16_t ridpart0 = strtoul(ridstr.substr(0, colon1).c_str(), nullptr, 16);
            uint16_t ridpart1 = strtoul(ridstr.substr(colon1 + 1, colon2 - (colon1 + 1)).c_str(), nullptr, 16);
            uint16_t ridpart2 = strtoul(ridstr.substr(colon2 + 1, colon3 - (colon2 + 1)).c_str(), nullptr, 16);
            uint16_t ridpart3 = strtoul(ridstr.substr(colon3 + 1).c_str(), nullptr, 16);

            //convert to uint32_t
            bMain->setRouterId((ridpart0<<16) | ridpart1, (ridpart2<<16) | ridpart3);

            EV << "RouterId loaded from config file: " << bMain->getRouterId() << endl;
        }
        else
        {// RouterId in bad format -> generate
            EV << "Bad format of RouterId in config file - IGNORED" << endl;
            bMain->generateRouterId();
        }
    }

    //Find explicitly specified port
    portElem = processElem->getFirstChildWithTag("Port");
    if(portElem != nullptr)
    {
        int porttmp;
        bool success = Str2Int(&porttmp, portElem->getNodeValue());
        if (!success || porttmp < 1 || porttmp > UINT16_MAX)
        {
            EV << "Bad value for UDP port (<1, 65535>) in config file - IGNORED (Used default: "
               << Babel::defval::PORT << ")" << endl;
        }
        else
        {
            bMain->setPort(porttmp);
            EV << "Using non default UDP port: " << porttmp << endl;
        }
    }
}

void BabelDeviceConfigurator::loadBabelInterfacesConfig(cXMLElement *device, BabelMain *bMain)
{
    cXMLElement *ifaceElem = nullptr;
    cXMLElement *babelIfaceElem = nullptr;

    if ((ifaceElem = GetInterface(ifaceElem, device)) == nullptr)
        return;

    while (ifaceElem != nullptr)
    {
        // Get interface ID
        const char *ifaceName = ifaceElem->getAttribute("name");
        InterfaceEntry *iface = ift->getInterfaceByName(ifaceName);
        if (iface == nullptr){
            throw cRuntimeError("No interface called %s on this device", ifaceName);
        }


        babelIfaceElem = ifaceElem->getFirstChildWithTag("Babel");

        if (babelIfaceElem != nullptr)
        {// interface contains babel configuration

            BabelInterface *bIface = new BabelInterface(iface);

            //remember local prefixes
            cXMLElement *ipv4AddrElem = ifaceElem->getFirstChildWithTag("IPAddress");
            cXMLElement *ipv4MaskElem = ifaceElem->getFirstChildWithTag("Mask");
            if(ipv4AddrElem != nullptr && ipv4MaskElem != nullptr)
            {
                bIface->addDirectlyConn(Babel::netPrefix<L3Address>(IPv4Address(ipv4AddrElem->getNodeValue()), IPv4Address(ipv4MaskElem->getNodeValue()).getNetmaskLength()));
            }


            // for each IPv6 address - save info about network prefix
            cXMLElement *ipv6AddrElem = GetIPv6Address(nullptr, ifaceElem);
            while (ipv6AddrElem != nullptr)
            {
                // get address string
                string addrFull = ipv6AddrElem->getNodeValue();
                IPv6Address ipv6;
                int prefixLen;

                // check if it's a valid IPv6 address string with prefix and get prefix
                if (!ipv6.tryParseAddrWithPrefix(addrFull.c_str(), prefixLen))
                {
                    throw cRuntimeError("Unable to set IPv6 address %s on interface %s", addrFull.c_str(), ifaceName);
                }

                ipv6 = IPv6Address(addrFull.substr(0, addrFull.find_last_of('/')).c_str());

                //EV << "IPv6 address: " << ipv6 << "/" << prefixLen << " on iface " << ifaceName << endl;

                if(ipv6.getScope() != IPv6Address::LINK)
                {// is not link-local -> add
                    bIface->addDirectlyConn(Babel::netPrefix<L3Address>(ipv6, prefixLen));
                }
                // get next IPv6 address
                ipv6AddrElem = GetIPv6Address(ipv6AddrElem, nullptr);
            }



            string ifacename = string(iface->getName()).erase(3);
            //TODO
            if(ifacename == "eth")
            {
                bIface->setWired(true);
                bIface->setNominalRxcost(Babel::defval::NOM_RXCOST_WIRED);
                bIface->setSplitHorizon(true);
            }
            else if(ifacename == "ppp")
            {
                bIface->setWired(true);
                bIface->setNominalRxcost(Babel::defval::NOM_RXCOST_WIRED);
                bIface->setSplitHorizon(true);
            }
            else if(ifacename == "wla")
            {
                bIface->setWired(false);
                bIface->setNominalRxcost(Babel::defval::NOM_RXCOST_WIRELESS);
                bIface->setSplitHorizon(false);
            }

            loadBabelInterface(babelIfaceElem, bMain, bIface);

            if(bMain->bit.addInterface(bIface))
            {
                bMain->activateInterface(bIface);
            }
        }

        ifaceElem = GetInterface(ifaceElem, nullptr);
    }
}

BabelDeviceConfigurator::BabelDeviceConfigurator() {
    deviceId = nullptr;
    deviceType = nullptr;
    configFile = nullptr;
}

BabelDeviceConfigurator::BabelDeviceConfigurator(const char* devId,
        const char* devType, const char* confFile, IInterfaceTable* intf)
: deviceId(devId), deviceType(devType), configFile(confFile), ift(intf)
{
}

BabelDeviceConfigurator::~BabelDeviceConfigurator() {
    deviceId = nullptr;
    deviceType = nullptr;
    configFile = nullptr;
}

void BabelDeviceConfigurator::loadBabelInterface(cXMLElement *ifaceElem, BabelMain *bMain, BabelInterface *bIface)
{
    int tempNumber;
    bool tempBool, success;

    uint16_t hInterval = 0;
    uint16_t uInterval = 0;
    string ccmstr = "";

    cXMLElementList ifDetails = ifaceElem->getChildren();
    for (cXMLElementList::iterator ifElemIt = ifDetails.begin(); ifElemIt != ifDetails.end(); ifElemIt++)
    {
        std::string nodeName = (*ifElemIt)->getTagName();

        if (nodeName == "HelloInterval")
        {
            success = Str2Int(&tempNumber, (*ifElemIt)->getNodeValue());
            if (!success || tempNumber < 1 || tempNumber > UINT16_MAX)
                throw cRuntimeError("Bad value for Babel Hello interval (<1, 65535>) on interface %s", bIface->getIfaceName());
            hInterval = tempNumber;
        }
        else if (nodeName == "UpdateInterval")
        {
            success = Str2Int(&tempNumber, (*ifElemIt)->getNodeValue());
            if (!success || tempNumber < 10 || tempNumber > UINT16_MAX)
                throw cRuntimeError("Bad value for Babel Update interval (<10, 65535>) on interface %s", bIface->getIfaceName());
            uInterval = tempNumber;
        }
        else if (nodeName == "SplitHorizon")
        {
            if (!Str2Bool(&tempBool, (*ifElemIt)->getNodeValue()))
                throw cRuntimeError("Bad value for Babel Split Horizon on interface %s", bIface->getIfaceName());
            bIface->setSplitHorizon(tempBool);
        }
        else if (nodeName == "Wired")
        {
            if (!Str2Bool(&tempBool, (*ifElemIt)->getNodeValue()))
                throw cRuntimeError("Bad value for Babel Wired on interface %s", bIface->getIfaceName());
            bIface->setWired(tempBool);
        }
        else if (nodeName == "Rxcost")
        {
            success = Str2Int(&tempNumber, (*ifElemIt)->getNodeValue());
            if (!success || tempNumber < 1 || tempNumber > UINT16_MAX)
                throw cRuntimeError("Bad value for Babel Rxcost (<1, 65535>) on interface %s", bIface->getIfaceName());
            bIface->setNominalRxcost(tempNumber);
        }
        else if (nodeName == "AFSend")
        {
            string afstr = (*ifElemIt)->getNodeValue();
            //trim string
            afstr.erase(afstr.find_last_not_of(" \t\n\r\f\v") + 1);
            afstr.erase(0, afstr.find_first_not_of(" \t\n\r\f\v"));
            //convert to lowercase
            std::transform(afstr.begin(), afstr.end(), afstr.begin(), ::tolower);

            if(afstr.compare("ipv6") == 0)
            {// IPv6 only
                 bIface->setAfSend(Babel::AF::IPv6);
            }
            else if(afstr.compare("ipv4") == 0)
            {// IPv4 only
                bIface->setAfSend(Babel::AF::IPv4);
            }
            else if(afstr.compare("ipvx") == 0 || afstr.compare("ipv46") == 0 || afstr.compare("ipv4andipv6") == 0 || afstr.compare("both") == 0)
            {// IPv4 and IPv6
                bIface->setAfSend(Babel::AF::IPvX);
            }
            else if(afstr.compare("none") == 0 || afstr.compare("passive") == 0)
            {// passive
                bIface->setAfSend(Babel::AF::NONE);
            }
            else
            {
                throw cRuntimeError("Bad value for Babel AddressFamily (possible values: IPv6, IPv4 or IPvX) on interface %s", bIface->getIfaceName());
            }
        }
        else if (nodeName == "AFDistribute")
        {
            string afstr = (*ifElemIt)->getNodeValue();
            //trim string
            afstr.erase(afstr.find_last_not_of(" \t\n\r\f\v") + 1);
            afstr.erase(0, afstr.find_first_not_of(" \t\n\r\f\v"));
            //convert to lowercase
            std::transform(afstr.begin(), afstr.end(), afstr.begin(), ::tolower);

            if(afstr.compare("ipv6") == 0)
            {// IPv6 only
                 bIface->setAfDist(Babel::AF::IPv6);
            }
            else if(afstr.compare("ipv4") == 0)
            {// IPv4 only
                bIface->setAfDist(Babel::AF::IPv4);
            }
            else if(afstr.compare("ipvx") == 0 || afstr.compare("ipv46") == 0 || afstr.compare("ipv4andipv6") == 0 || afstr.compare("both") == 0)
            {// IPv4 and IPv6
                bIface->setAfDist(Babel::AF::IPvX);
            }
            else if(afstr.compare("none") == 0)
            {// passive
                bIface->setAfDist(Babel::AF::NONE);
            }
            else
            {
                throw cRuntimeError("Bad value for Babel AddressFamily (possible values: IPv6, IPv4 or IPvX) on interface %s", bIface->getIfaceName());
            }
        }
        else if (nodeName == "CostCompModule")
        {
            ccmstr = (*ifElemIt)->getNodeValue();
            //trim string
            ccmstr.erase(ccmstr.find_last_not_of(" \t\n\r\f\v") + 1);
            ccmstr.erase(0, ccmstr.find_first_not_of(" \t\n\r\f\v"));
        }
    }



    if(ccmstr.size() == 0)
    {
        if(bIface->getWired())
        {
            ccmstr = "cost2outof3";
        }
        else
        {
            ccmstr = "costetx";
        }
    }

    cModule *submodule = bMain->getParentModule()->getSubmodule(ccmstr.c_str());

    if(submodule)
    {
        IBabelCostComputation *cc = dynamic_cast<IBabelCostComputation *>(submodule);

        if(cc)
        {
            bIface->setCostComputationModule(cc);
        }
        else
        {
            throw cRuntimeError("CostComputation module %s is not IBabelCostComputation class", ccmstr.c_str());
        }
    }
    else
    {
        throw cRuntimeError("Can not find CostComputation module with name %s", ccmstr.c_str());
    }

    if(hInterval != 0)
    {// Hello interval specified -> set it
        bIface->setHInterval(hInterval);
    }
    else
    {// not specified -> set it to default

        if(bIface->getWired())
        {
            bIface->setHInterval(Babel::defval::HELLO_INTERVAL_WIRE_CS);
        }
        else
        {
            bIface->setHInterval(Babel::defval::HELLO_INTERVAL_CS);
        }

    }


    if(uInterval != 0)
    {// Update interval specified -> set it
        bIface->setUInterval(uInterval);
    }
    else
    {// not specified -> set it to some multiple of Hello interval
        bIface->setUInterval(Babel::defval::UPDATE_INTERVAL_MULT * bIface->getHInterval());
    }
}
// - End of Babel configuration


/*
void BabelDeviceConfigurator::loadInterfaceConfig6(cXMLElement *iface){

   // for each interface node
   while (iface != nullptr){

      // get interface name and find matching interface in interface table
      const char *ifaceName = iface->getAttribute("name");
      InterfaceEntry *ie = ift->getInterfaceByName(ifaceName);
      if (ie == nullptr){
         //throw cRuntimeError("No interface called %s on this device", ifaceName);
         EV << "No interface called %s on this device" << ifaceName << endl;
         // get next interface
         iface = GetInterface(iface, nullptr);
         continue;
      }

      // for each IPv6 address
      cXMLElement *addr = GetIPv6Address(nullptr, iface);
      while (addr != nullptr){

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
         addr = GetIPv6Address(addr, nullptr);
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
      cXMLElement *prefix = GetAdvPrefix(nullptr, iface);
      while (prefix != nullptr){

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
         if (validLifeTime != nullptr){
            if (!Str2Int(&value, validLifeTime)){
               throw cRuntimeError("Unable to parse valid lifetime %s on IPv6 prefix %s on interface %s", validLifeTime, addrFull.c_str(), ifaceName);
            }
            advPrefix.advValidLifetime = value;
         }

         value = 604800;
         if (preferredLifeTime != nullptr){
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
         prefix = GetAdvPrefix(prefix, nullptr);
      }



      // get next interface
      iface = GetInterface(iface, nullptr);
   }
}


void BabelDeviceConfigurator::loadDefaultRouter6(cXMLElement *gateway){

   if (gateway == nullptr)
      return;

   // get default-router address string (without prefix)
   IPv6Address nextHop;
   nextHop = IPv6Address(gateway->getNodeValue());

   // browse routing table to find the best route to default-router
   const IPv6Route *route = rt6->doLongestPrefixMatch(nextHop);
   if (route == nullptr){
      return;
   }

   // add default static route
   rt6->addStaticRoute(IPv6Address::UNSPECIFIED_ADDRESS, 0, route->getInterfaceId(), nextHop, 1);
}


void BabelDeviceConfigurator::loadPimInterfaceConfig(cXMLElement *iface)
{
    // for each interface node
    while (iface != nullptr)
    {
        // get PIM node
        cXMLElement* pimNode = iface->getElementByPath("Pim");
        if (pimNode == nullptr)
          break;                //FIXME it is break ok?

        // create new PIM interface
        PimInterface newentry;
        InterfaceEntry *interface = ift->getInterfaceByName(iface->getAttribute("name"));
        newentry.setInterfaceID(interface->getInterfaceId());
        newentry.setInterfacePtr(interface);

        // get PIM mode for interface
        cXMLElement* pimMode = pimNode->getElementByPath("Mode");
        if (pimMode == nullptr)
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
        if (stateRefreshMode != nullptr)
        {
            EV << "Nasel State Refresh" << endl;
            // will router send state refresh msgs?
            cXMLElement* origMode = stateRefreshMode->getElementByPath("OriginationInterval");
            if (origMode != nullptr)
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
        iface = GetInterface(iface, nullptr);
    }
}


void BabelDeviceConfigurator::loadDefaultRouter(cXMLElement *gateway)
{
    if (gateway == nullptr)
      return;

    // get default-router address string (without prefix)
    IPv4Address nextHop;
    nextHop = IPv4Address(gateway->getNodeValue());

    // browse routing table to find the best route to default-router
    const IPv4Route *route = rt->findBestMatchingRoute(nextHop);
    if (route == nullptr)
      return;

    AnsaRoutingTable *ANSArt = dynamic_cast<AnsaRoutingTable *>(rt);

    //To the ANSA RoutingTable add ANSAIPv4Route, to the inet RoutingTable add IPv4Route
    if (ANSArt != nullptr)
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

void BabelDeviceConfigurator::addIPv4MulticastGroups(cXMLElement *iface)
{
    while (iface != nullptr)
    {
        // get MCastGroups Node
        cXMLElement* MCastGroupsNode = iface->getElementByPath("MCastGroups");
        if (MCastGroupsNode == nullptr)
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
        iface = GetInterface(iface, nullptr);
    }

}

void BabelDeviceConfigurator::addIPv6MulticastGroups(cXMLElement *iface)
{
    while (iface != nullptr)
    {
        // get MCastGroups Node
        cXMLElement* MCastGroupsNode6 = iface->getElementByPath("MCastGroups6");
        if (MCastGroupsNode6 == nullptr)
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
        iface = GetInterface(iface, nullptr);
    }

}


void BabelDeviceConfigurator::loadInterfaceConfig(cXMLElement* iface)
{
    AnsaRoutingTable *ANSArt = dynamic_cast<AnsaRoutingTable *>(rt);

    while (iface != nullptr)
    {
        // get interface name and find matching interface in interface table
        const char *ifaceName = iface->getAttribute("name");
        InterfaceEntry *ie = ift->getInterfaceByName(ifaceName);

        if (ie == nullptr) {
           //throw cRuntimeError("No interface called %s on this device", ifaceName);
            EV << "No interface " << ifaceName << "called on this device" << endl;
            iface = GetInterface(iface, nullptr);
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
        if (ANSArt != nullptr)
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

        iface = GetInterface(iface, nullptr);
    }
}


void BabelDeviceConfigurator::setInterfaceParamters(InterfaceEntry *interface)
{
    int gateId = interface->getNodeOutputGateId();
    cModule *host = findContainingNode(interface->getInterfaceModule());
    cGate *outGw;
    cChannel *link;

    if (host != nullptr && gateId != -1)
    { // Get link type
        outGw = host->gate(gateId);
        if ((link = outGw->getChannel()) == nullptr)
            return;

        const char *linkType = link->getChannelType()->getName();
        cPar bwPar = link->par("datarate");

        // Bandwidth in kbps
        interface->setBandwidth(bwPar.doubleValue() / 1000);
        interface->setDelay(getDefaultDelay(linkType));
    }
}


double BabelDeviceConfigurator::getDefaultDelay(const char *linkType)
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
*/

/**********************XML_PARSER************************************/



cXMLElement * BabelDeviceConfigurator::GetDevice(const char *deviceType, const char *deviceId, const char *configFile){

   // get access to the XML file (if exists)
   cXMLElement *config = ev.getXMLDocument(configFile);
   if (config == nullptr)
      return nullptr;


   string type = deviceType;
   string id = deviceId;
   if (type.empty() || id.empty())
      return nullptr;

   // create string that describes device node, such as <Router id="10.0.0.1">
   string deviceNodePath = type;
   deviceNodePath += "[@id='";
   deviceNodePath += id;
   deviceNodePath += "']";

   // get access to the device node
   cXMLElement *device = config->getElementByPath(deviceNodePath.c_str());

   return device;
}


cXMLElement * BabelDeviceConfigurator::GetInterface(cXMLElement *iface, cXMLElement *device){

   // initial call of the method - find <Interfaces> and get first "Interface" node
   if (device != nullptr){

      cXMLElement *ifaces = device->getFirstChildWithTag("Interfaces");
      if (ifaces == nullptr)
         return nullptr;

      iface = ifaces->getFirstChildWithTag("Interface");

   // repeated call - get another "Interface" sibling node
   }else if (iface != nullptr){
      iface = iface->getNextSiblingWithTag("Interface");
   }else{
      iface = nullptr;
   }

   return iface;
}

cXMLElement * BabelDeviceConfigurator::GetIPv6Address(cXMLElement *addr, cXMLElement *iface){

   // initial call of the method - get first "IPv6Address" child node
   if (iface != nullptr){
      addr = iface->getFirstChildWithTag("IPv6Address");

   // repeated call - get another "IPv6Address" sibling node
   }else if (addr != nullptr){
      addr = addr->getNextSiblingWithTag("IPv6Address");
   }else{
      addr = nullptr;
   }

   return addr;
}

cXMLElement * BabelDeviceConfigurator::GetAdvPrefix(cXMLElement *prefix, cXMLElement *iface){

   // initial call of the method - get first "NdpAdvPrefix" child node
   if (iface != nullptr){
      prefix = iface->getFirstChildWithTag("NdpAdvPrefix");

   // repeated call - get another "NdpAdvPrefix" sibling node
   }else if (prefix != nullptr){
      prefix = prefix->getNextSiblingWithTag("NdpAdvPrefix");
   }else{
      prefix = nullptr;
   }

   return prefix;
}
/*
cXMLElement * BabelDeviceConfigurator::GetStaticRoute6(cXMLElement *route, cXMLElement *device){

   // initial call of the method - find <Routing> -> <Static>
   // and then get first "Route" child node
   if (device != nullptr){

      cXMLElement *routing = device->getFirstChildWithTag("Routing6");
      if (routing == nullptr)
         return nullptr;

      cXMLElement *staticRouting = routing->getFirstChildWithTag("Static");
      if (staticRouting == nullptr)
         return nullptr;

      route = staticRouting->getFirstChildWithTag("Route");

   // repeated call - get another "Route" sibling node
   }else if (route != nullptr){
      route = route->getNextSiblingWithTag("Route");
   }else{
      route = nullptr;
   }

   return route;
}

cXMLElement * BabelDeviceConfigurator::GetStaticRoute(cXMLElement *route, cXMLElement *device){

   // initial call of the method - find <Routing6> -> <Static>
   // and then get first "Route" child node
   if (device != nullptr){

      cXMLElement *routing = device->getFirstChildWithTag("Routing");
      if (routing == nullptr)
         return nullptr;

      cXMLElement *staticRouting = routing->getFirstChildWithTag("Static");
      if (staticRouting == nullptr)
         return nullptr;

      route = staticRouting->getFirstChildWithTag("Route");

   // repeated call - get another "Route" sibling node
   }else if (route != nullptr){
      route = route->getNextSiblingWithTag("Route");
   }else{
      route = nullptr;
   }

   return route;
}


const char *BabelDeviceConfigurator::GetNodeParamConfig(cXMLElement *node, const char *paramName, const char *defaultValue)
{
    ASSERT(node != nullptr);

    cXMLElement* paramElem = node->getElementByPath(paramName);
    if (paramElem == nullptr)
        return defaultValue;

    const char *paramValue = paramElem->getNodeValue();
    if (paramValue == nullptr)
        return defaultValue;

    return paramValue;
}

const char *BabelDeviceConfigurator::GetNodeAttrConfig(cXMLElement *node, const char *attrName, const char *defaultValue)
{
    ASSERT(node != nullptr);

    const char *attrValue = node->getAttribute(attrName);
    if (attrValue == nullptr)
        return defaultValue;

    return attrValue;
}
*/
//
// - Babel configuration
//

cXMLElement *BabelDeviceConfigurator::GetBabelProcess(cXMLElement *device)
{

    if (device == nullptr)
    {
        return nullptr;
    }

    cXMLElement *routing = device->getFirstChildWithTag("Routing");
    if (routing == nullptr)
    {
        return nullptr;
    }

    return routing->getFirstChildWithTag("Babel");
}


/*
 * A utility method for proper str -> int conversion with error checking.
 */
bool BabelDeviceConfigurator::Str2Int(int *retValue, const char *str){

   if (retValue == nullptr || str == nullptr){
      return false;
   }

   char *tail = nullptr;
   long value = 0;
   errno = 0;

   value = strtol(str, &tail, 0);

   if (*tail != '\0' || errno == ERANGE || errno == EINVAL || value < INT_MIN || value > INT_MAX){
      return false;
   }

   *retValue = (int) value;
   return true;
}

bool BabelDeviceConfigurator::Str2Bool(bool *ret, const char *str){

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

/*
bool BabelDeviceConfigurator::isMulticastEnabled(cXMLElement *device)
{
    // Routing element
    cXMLElement* routingNode = device->getElementByPath("Routing");
    if (routingNode == nullptr)
         return false;

    // Multicast element
    cXMLElement* multicastNode = routingNode->getElementByPath("Multicast");
    if (multicastNode == nullptr)
       return false;


    // Multicast has to be enabled
    const char* enableAtt = multicastNode->getAttribute("enable");
    if (strcmp(enableAtt, "1"))
        return false;

    return true;
}
*/


}
