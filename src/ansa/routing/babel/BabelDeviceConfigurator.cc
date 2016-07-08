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

void BabelDeviceConfigurator::loadBabelConfig(BabelMain *bMain)
{
    ASSERT(bMain != nullptr);

    device = configFile;
    if (configFile == nullptr) {
        EV << "No Babel configuration found for this device!" << endl;
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

            BabelInterface *bIface = new BabelInterface(iface,bMain->getIntuniform(0,UINT16_MAX));

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

BabelDeviceConfigurator::BabelDeviceConfigurator(cXMLElement* confFile, IInterfaceTable* intf)
: configFile(confFile), ift(intf)
{
}

BabelDeviceConfigurator::~BabelDeviceConfigurator() {
    ift = nullptr;
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

cXMLElement * BabelDeviceConfigurator::GetDevice(const char *deviceType, const char *deviceId, cXMLElement* configFile){

   // get access to the XML file (if exists)
   cXMLElement *config = configFile;
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

}
