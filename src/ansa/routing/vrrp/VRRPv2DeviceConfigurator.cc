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

#include "ansa/routing/vrrp/VRRPv2DeviceConfigurator.h"
#include <errno.h>

namespace inet {

using namespace std;

VRRPv2DeviceConfigurator::VRRPv2DeviceConfigurator() {
    configFile = nullptr;
    ift = nullptr;
}

VRRPv2DeviceConfigurator::VRRPv2DeviceConfigurator(cXMLElement* confFile, IInterfaceTable* intf)
: configFile(confFile), ift(intf)
{
}

VRRPv2DeviceConfigurator::~VRRPv2DeviceConfigurator() {
    configFile = nullptr;
    ift = nullptr;
}

cXMLElement * VRRPv2DeviceConfigurator::GetInterface(cXMLElement *iface, cXMLElement *device){

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

/*
 * A utility method for proper str -> int conversion with error checking.
 */
bool VRRPv2DeviceConfigurator::Str2Int(int *retValue, const char *str){

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

bool VRRPv2DeviceConfigurator::Str2Bool(bool *ret, const char *str){

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

void VRRPv2DeviceConfigurator::loadVRRPv2Config(VRRPv2* VRRPModule) {

    ASSERT(VRRPModule != NULL);

    // get access to device node from XML
    //const char *deviceType = par("deviceType");
    //const char *deviceId = par("deviceId");
    //const char *configFile = par("configFile");
    cXMLElement *device = configFile;

    if (device == NULL){
       EV << "No VRRP configuration found for this device!" << endl;
            return;
    }

    cXMLElement *interface;
    //get first router's interface
    interface = GetInterface(NULL, device);

    while (interface != NULL)
    {

        EV << ift->getInterfaceByName(interface->getAttribute("name")) << endl;
        InterfaceEntry* ie = ift->getInterfaceByName(interface->getAttribute("name"));
        if (!ie) {
            EV << "Interface " << interface->getAttribute("name") << " does not exist!" << endl;
            continue;
        }
        int interfaceId = ie->getInterfaceId();

        cXMLElement *group;
        group = GetVRRPGroup(NULL, interface);
        while (group != NULL)
        {
            int groupId = -1;
            if (HasVRPPGroup(group, &groupId)) {
                const char* ifnam = ift->getInterfaceById(interfaceId)->getName();
                VRRPModule->addVirtualRouter(interfaceId, groupId, ifnam);
            }

            group = GetVRRPGroup(group, NULL);
        }
        interface = GetInterface(interface, NULL);
    }
}

void VRRPv2DeviceConfigurator::loadVRRPv2VirtualRouterConfig(VRRPv2VirtualRouter* VRRPModule) {
    ASSERT(VRRPModule != NULL);

    // get access to device node from XML
    //const char *deviceType = par("deviceType");
    //const char *deviceId = par("deviceId");
    //const char *configFile = par("configFile");
    cXMLElement *device = configFile;

    if (device == NULL){
       EV << "No VRRP configuration found for this device" << endl;
            return;
    }

    std::ostringstream groupId;
    groupId <<  VRRPModule->getVrid();

    //stringstream ss;
    //ss << VRRPModule->getVrid();
    //string groupId = ss.str();

    //const char* aaa = VRRPModule->getInterface()->getFullName();
    //const char* bbb = groupId.str().c_str();

    cXMLElement *group = GetVRRPGroup(device, VRRPModule->getInterface()->getFullName(), groupId.str().c_str());
    if (group == NULL) {
        EV << "No configuration found for VRRP group " << groupId.str() << endl;
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
            if (!Str2Int(&value, (*routeElemIt)->getNodeValue()))
                throw cRuntimeError("Invalid Priority value on interface %s", VRRPModule->getInterface()->getName());

            VRRPModule->setPriority(value);
        }
        else if (nodeName == "Preempt")
        {
            bool value = false;
            if (!Str2Bool(&value, (*routeElemIt)->getNodeValue())){
               throw cRuntimeError("Invalid Preempt value on interface %s", VRRPModule->getInterface()->getName());
            }
            VRRPModule->setPreemtion(value);

            if ((*routeElemIt)->hasAttributes()) {
                int value = 0;
                if (!Str2Int(&value, (*routeElemIt)->getAttribute("delay")))
                    throw cRuntimeError("Unable to parse valid Preemtion delay %s on interface %s", value, VRRPModule->getInterface()->getName());

                VRRPModule->setPreemtionDelay(value);
            }
        }
        else if (nodeName == "TimerAdvertise")
        {
            int value = 0;
            if (!Str2Int(&value, (*routeElemIt)->getNodeValue()))
                throw cRuntimeError("Unable to parse valid TimerAdvertise %s on interface %s", value, VRRPModule->getInterface()->getFullName());

            VRRPModule->setAdvertisement(value);
        }
        else if (nodeName == "TimerLearn")
        {
            bool value = false;
            if (!Str2Bool(&value, (*routeElemIt)->getNodeValue())){
               throw cRuntimeError("Invalid TimerLearn value on interface %s", VRRPModule->getInterface()->getName());
            }
            VRRPModule->setLearn(value);
        }
    }
}

cXMLElement * VRRPv2DeviceConfigurator::GetVRRPGroup(cXMLElement *group, cXMLElement *iface) {

   // initial call of the method - find <Interfaces> and get first "Interface" node
   if (iface != NULL){

      cXMLElement *groups = iface->getFirstChildWithTag("VRRP");
      if (groups == NULL)
         return NULL;

      group = groups->getFirstChildWithTag("Group");

   // repeated call - get another "Interface" sibling node
   }else if (group != NULL){
       group = group->getNextSiblingWithTag("Group");
   }else{
       group = NULL;
   }

   return group;
}

cXMLElement * VRRPv2DeviceConfigurator::GetVRRPGroup(cXMLElement *device, const char* name, const char* groupId)
{
    string path = "./Interfaces/Interface[\@name='" ;
    path += name;
    path += "']/VRRP/Group[\@id='";
    path += groupId;
    path += "']";

    return device->getElementByPath(path.c_str());
}

bool VRRPv2DeviceConfigurator::HasVRPPGroup(cXMLElement* group, int *groupId)
{
    cXMLElement *address = group->getFirstChildWithTag("IPAddress");
    if (address == NULL)
        return false;

    if (!Str2Int(groupId, group->getAttribute("id")))
        return false;

    if ((* groupId) < 0 || (* groupId) > 255)
        return false;

    return true;
}

}
