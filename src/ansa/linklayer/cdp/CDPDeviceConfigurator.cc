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
* @file CDPDeviceConfigurator.cc
* @authors Tomas Rajca, Vladimir Vesely (ivesely@fit.vutbr.cz)
* @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
*/

#include "ansa/linklayer/cdp/CDPDeviceConfigurator.h"
#include <errno.h>

namespace inet {
using namespace std;

//
// - CDP configuration
//

void CDPDeviceConfigurator::loadCDPConfig(CDPMain *cMain)
{
    ASSERT(cMain != nullptr);

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
    loadODRProcessConfig(device, cMain);

    loadCDPInterfacesConfig(device, cMain);
}

void CDPDeviceConfigurator::loadODRProcessConfig(cXMLElement *device, CDPMain *cMain)
{
    ASSERT(cMain != nullptr);
    ASSERT(device != nullptr);

    cXMLElement *processElem = nullptr;

    //Check if odr routing is enabled
    processElem = GetODRProcess(device);
    if (processElem == nullptr)
    {// ODR is not enabled
        return;
    }

    int tempNumber;
    bool success;

    cXMLElementList ifDetails = processElem->getChildren();
    for (cXMLElementList::iterator ifElemIt = ifDetails.begin(); ifElemIt != ifDetails.end(); ifElemIt++)
    {
        std::string nodeName = (*ifElemIt)->getTagName();

        if (nodeName == "invalid")
        {
            success = Str2Int(&tempNumber, (*ifElemIt)->getNodeValue());
            if (!success || tempNumber < 0 || tempNumber > 2147483)
                throw cRuntimeError("Bad value for ODR invalid time (<0, 2147483>)");
            cMain->setRouteInvalidTime(tempNumber);
        }
        else if (nodeName == "holddown")
        {
            success = Str2Int(&tempNumber, (*ifElemIt)->getNodeValue());
            if (!success || tempNumber < 0 || tempNumber > 2147483)
                throw cRuntimeError("Bad value for ODR holddown time (<0, 2147483>");
            cMain->setRouteHolddownTime(tempNumber);
        }
        else if (nodeName == "flush")
        {
            success = Str2Int(&tempNumber, (*ifElemIt)->getNodeValue());
            if (!success || tempNumber < 0 || tempNumber > 2147483)
                throw cRuntimeError("Bad value for ODR flush time (<0, 2147483>)");
            cMain->setRouteFlushTime(tempNumber);
        }
    }
}

cXMLElement *CDPDeviceConfigurator::GetODRProcess(cXMLElement *device)
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

    return routing->getFirstChildWithTag("ODR");
}

void CDPDeviceConfigurator::loadCDPInterfacesConfig(cXMLElement *device, CDPMain *cMain)
{
    cXMLElement *ifaceElem = nullptr;
    cXMLElement *CDPIfaceElem = nullptr;
    CDPInterface *cdpIface;

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


        CDPIfaceElem = ifaceElem->getFirstChildWithTag("CDP");
        cdpIface = cMain->getCit()->findInterfaceById(iface->getInterfaceId());

        if (CDPIfaceElem != nullptr && cdpIface != nullptr)
        {// interface contains CDP configuration
            loadCDPInterface(CDPIfaceElem, cMain, cdpIface);
        }

        ifaceElem = GetInterface(ifaceElem, nullptr);
    }
}

CDPDeviceConfigurator::CDPDeviceConfigurator() {
    deviceId = nullptr;
    deviceType = nullptr;
    configFile = nullptr;
}

CDPDeviceConfigurator::CDPDeviceConfigurator(const char* devId,
        const char* devType, cXMLElement* confFile, IInterfaceTable* intf)
: deviceId(devId), deviceType(devType), configFile(confFile), ift(intf)
{
}

CDPDeviceConfigurator::~CDPDeviceConfigurator() {
    deviceId = nullptr;
    deviceType = nullptr;
    configFile = nullptr;
}

void CDPDeviceConfigurator::loadCDPInterface(cXMLElement *ifaceElem, CDPMain *cMain, CDPInterface *cIface)
{
    bool value;

    cXMLElementList ifDetails = ifaceElem->getChildren();
    for (cXMLElementList::iterator ifElemIt = ifDetails.begin(); ifElemIt != ifDetails.end(); ifElemIt++)
    {
        std::string nodeName = (*ifElemIt)->getTagName();

        if (nodeName == "status")
        {
            if (!Str2Bool(&value, (*ifElemIt)->getNodeValue()))
                throw cRuntimeError("Invalid CDP status value on interface %s", cIface->getInterface()->getName());
            cIface->setCDPEnabled(value);
        }
    }
}
// - End of CDP configuration


/**********************XML_PARSER************************************/



cXMLElement * CDPDeviceConfigurator::GetDevice(const char *deviceType, const char *deviceId, cXMLElement* configFile){

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


cXMLElement * CDPDeviceConfigurator::GetInterface(cXMLElement *iface, cXMLElement *device){

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
bool CDPDeviceConfigurator::Str2Int(int *retValue, const char *str){

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

/*
 * A utility method for proper str -> bool conversion with error checking.
 */
bool CDPDeviceConfigurator::Str2Bool(bool *ret, const char *str)
{
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
} /* namespace inet */
