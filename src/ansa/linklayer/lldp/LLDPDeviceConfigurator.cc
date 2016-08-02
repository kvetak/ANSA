//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
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
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 *
 */

#include "ansa/linklayer/lldp/LLDPDeviceConfigurator.h"
#include <errno.h>

namespace inet {
using namespace std;

//
// - LLDP configuration
//

void LLDPDeviceConfigurator::loadLLDPConfig(LLDPMain *lMain)
{
    ASSERT(lMain != nullptr);

    device = GetDevice(deviceType, deviceId, configFile);
    if (device == nullptr)
    {
        EV << "No configuration found for this device (" << deviceType << " id=" << deviceId << ")" << endl;
        return;
    }

    if (device == nullptr)
    {
       EV << "No configuration found for this device (" << deviceType <<
               " id=" << deviceId << ")" << endl;
       return;
    }

    loadLLDPInterfacesConfig(device, lMain);
}

void LLDPDeviceConfigurator::loadLLDPInterfacesConfig(cXMLElement *device, LLDPMain *lMain)
{
    cXMLElement *ifaceElem = nullptr;
    cXMLElement *LLDPIfaceElem = nullptr;
    LLDPAgent *lAgent;
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


        LLDPIfaceElem = ifaceElem->getFirstChildWithTag("LLDP");
        lAgent = lMain->getLat()->findAgentById(iface->getInterfaceId());

        if (LLDPIfaceElem != nullptr && lAgent != nullptr)
        {// interface contains LLDP configuration
            loadLLDPInterface(LLDPIfaceElem, lMain, lAgent);
        }

        ifaceElem = GetInterface(ifaceElem, nullptr);
    }
}

LLDPDeviceConfigurator::LLDPDeviceConfigurator() {
    deviceId = nullptr;
    deviceType = nullptr;
    configFile = nullptr;
}

LLDPDeviceConfigurator::LLDPDeviceConfigurator(const char* devId,
        const char* devType, cXMLElement* confFile, IInterfaceTable* intf)
: deviceId(devId), deviceType(devType), configFile(confFile), ift(intf)
{
}

LLDPDeviceConfigurator::~LLDPDeviceConfigurator() {
    deviceId = nullptr;
    deviceType = nullptr;
    configFile = nullptr;
}

void LLDPDeviceConfigurator::loadLLDPInterface(cXMLElement *ifaceElem, LLDPMain *lMain, LLDPAgent *lIface)
{
    int tempNumber;
    bool success;

    cXMLElementList ifDetails = ifaceElem->getChildren();
    for (cXMLElementList::iterator ifElemIt = ifDetails.begin(); ifElemIt != ifDetails.end(); ifElemIt++)
    {
        std::string nodeName = (*ifElemIt)->getTagName();

        if (nodeName == "msgFastTx")
        {
            success = Str2Int(&tempNumber, (*ifElemIt)->getNodeValue());
            if (!success || tempNumber < 1 || tempNumber > 3600)
                throw cRuntimeError("Bad value for LLDP msgFastTx (<1, 3600>) on interface %s", lIface->getIfaceName());
            lIface->setMsgFastTx(tempNumber);
        }
        else if (nodeName == "msgTxHold")
        {
            success = Str2Int(&tempNumber, (*ifElemIt)->getNodeValue());
            if (!success || tempNumber < 1 || tempNumber > 100)
                throw cRuntimeError("Bad value for LLDP msgTxHold (<1, 100>) on interface %s", lIface->getIfaceName());
            lIface->setMsgTxHold(tempNumber);
        }
        else if (nodeName == "msgTxInterval")
        {
            success = Str2Int(&tempNumber, (*ifElemIt)->getNodeValue());
            if (!success || tempNumber < 1 || tempNumber > 3600)
                throw cRuntimeError("Bad value for LLDP msgTxInterval (<1, 3600>) on interface %s", lIface->getIfaceName());
            lIface->setMsgTxInterval(tempNumber);
        }
        else if (nodeName == "reinitDelay")
        {
            success = Str2Int(&tempNumber, (*ifElemIt)->getNodeValue());
            if (!success || tempNumber < 1 || tempNumber > UINT16_MAX)
                throw cRuntimeError("Bad value for LLDP reinitDelay (<1, 3600>) on interface %s", lIface->getIfaceName());
            lIface->setReinitDelay(tempNumber);
        }
        else if (nodeName == "txCreditMax")
        {
            success = Str2Int(&tempNumber, (*ifElemIt)->getNodeValue());
            if (!success || tempNumber < 1 || tempNumber > 10)
                throw cRuntimeError("Bad value for LLDP txCreditMax (<1, 10>) on interface %s", lIface->getIfaceName());
            lIface->setTxCreditMax(tempNumber);
        }
        else if (nodeName == "txFastInit")
        {
            success = Str2Int(&tempNumber, (*ifElemIt)->getNodeValue());
            if (!success || tempNumber < 1 || tempNumber > 8)
                throw cRuntimeError("Bad value for LLDP txCreditMax (<1, 8>) on interface %s", lIface->getIfaceName());
            lIface->setTxFastInit(tempNumber);
        }
        else if (nodeName == "AdminStatus")
        {
            string afstr = (*ifElemIt)->getNodeValue();
            //trim string
            afstr.erase(afstr.find_last_not_of(" \t\n\r\f\v") + 1);
            afstr.erase(0, afstr.find_first_not_of(" \t\n\r\f\v"));
            //convert to lowercase
            std::transform(afstr.begin(), afstr.end(), afstr.begin(), ::tolower);

            if(afstr.compare("enabledrxtx") == 0)
            {// reception and transmission of LLDPDUs
                 lIface->setAdminStatus(enabledRxTx);
            }
            else if(afstr.compare("enabledtxonly") == 0)
            {// transmission of LLDPDUs
                 lIface->setAdminStatus(enabledTxOnly);
            }
            else if(afstr.compare("enabledrxonly") == 0)
            {// reception of LLDPDUs
                 lIface->setAdminStatus(enabledRxOnly);
            }
            else if(afstr.compare("disabled") == 0)
            {// disabled for both reception and transmission
                 lIface->setAdminStatus(disabled);
            }
            else
            {
                throw cRuntimeError("Bad value for LLDP AdminStatus (possible values: enabledRxTx, enabledTxOnly, enabledRxOnly or disabled) on interface %s", lIface->getIfaceName());
            }
        }
    }
}
// - End of LLDP configuration


/**********************XML_PARSER************************************/



cXMLElement * LLDPDeviceConfigurator::GetDevice(const char *deviceType, const char *deviceId, cXMLElement* configFile){

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


cXMLElement * LLDPDeviceConfigurator::GetInterface(cXMLElement *iface, cXMLElement *device){

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


/*
 * A utility method for proper str -> int conversion with error checking.
 */
bool LLDPDeviceConfigurator::Str2Int(int *retValue, const char *str){

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

}
