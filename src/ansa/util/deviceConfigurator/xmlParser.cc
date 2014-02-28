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

#include "xmlParser.h"
#include <errno.h>

cXMLElement * xmlParser::GetDevice(const char *deviceType, const char *deviceId, const char *configFile){

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


cXMLElement * xmlParser::GetInterface(cXMLElement *iface, cXMLElement *device){

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

cXMLElement * xmlParser::GetIPv6Address(cXMLElement *addr, cXMLElement *iface){

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

cXMLElement * xmlParser::GetAdvPrefix(cXMLElement *prefix, cXMLElement *iface){

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
}

cXMLElement * xmlParser::GetStaticRoute6(cXMLElement *route, cXMLElement *device){

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
}

cXMLElement * xmlParser::GetStaticRoute(cXMLElement *route, cXMLElement *device){

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
}

cXMLElement * xmlParser::GetOspfProcess6(cXMLElement *process, cXMLElement *device){

   // initial call of the method - get <Routing6> -> <Ospf>
   // and then first "Process" child node
   if (device != NULL){

      cXMLElement *routing = device->getFirstChildWithTag("Routing6");
      if (routing == NULL)
         return NULL;

      cXMLElement *ospf = routing->getFirstChildWithTag("Ospf");
      if (ospf == NULL)
         return NULL;

      process = ospf->getFirstChildWithTag("Process");

   // repeated call - get another "Process" sibling node
   }else if (process != NULL){
      process = process->getNextSiblingWithTag("Process");
   }else{
      process = NULL;
   }

   return process;
}

/*
 * A utility method for proper str -> int conversion with error checking.
 */
bool xmlParser::Str2Int(int *retValue, const char *str){

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

bool xmlParser::Str2Bool(bool *ret, const char *str){

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


/* Check if IS-IS is enabled in XML config.
 * Return NULL if not presented, otherwise return main IS-IS element
 */
cXMLElement * xmlParser::GetIsisRouting(cXMLElement * device)
{
    if(device == NULL)
        return NULL;

    cXMLElement *routing = device->getFirstChildWithTag("Routing");
    if(routing == NULL)
        return  NULL;

    cXMLElement * isis = routing->getFirstChildWithTag("ISIS");
    return isis;
}

const char *xmlParser::GetNodeParamConfig(cXMLElement *node, const char *paramName, const char *defaultValue)
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

const char *xmlParser::GetNodeAttrConfig(cXMLElement *node, const char *attrName, const char *defaultValue)
{
    ASSERT(node != NULL);

    const char *attrValue = node->getAttribute(attrName);
    if (attrValue == NULL)
        return defaultValue;

    return attrValue;
}

//
//
//- configuration for RIPng
//
//
cXMLElement *xmlParser::GetInterfaceRIPngProcess(cXMLElement *ripng, cXMLElement *iface)
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

const char *xmlParser::GetInterfaceRIPngPassiveStatus(cXMLElement *ripng)
{
    return GetNodeParamConfig(ripng, "PassiveInterface", "disable");
}

const char *xmlParser::GetInterfaceRIPngSplitHorizon(cXMLElement *ripng)
{
    return GetNodeParamConfig(ripng, "SplitHorizon", "enable");
}

const char *xmlParser::GetInterfaceRIPngPoisonReverse(cXMLElement *ripng)
{
    return GetNodeParamConfig(ripng, "PoisonReverse", "disable");
}

cXMLElement *xmlParser::GetInterfaceRIPngDefaultInformation(cXMLElement *ripng)
{
    return ripng->getFirstChildWithTag("DefaultInformation");
}

const char  *xmlParser::GetInterfaceRIPngMetricOffset(cXMLElement *ripng)
{
    return GetNodeParamConfig(ripng, "MetricOffset", "0");
}

cXMLElement *xmlParser::GetRIPngProcess(cXMLElement *process, cXMLElement *device)
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

cXMLElement *xmlParser::GetRIPngProcessTimers(cXMLElement *process)
{
    return process->getFirstChildWithTag("Timers");
}

//
//
//- configuration for RIP
//
//

cXMLElement *xmlParser::GetRIPNetwork(cXMLElement *network, cXMLElement *device)
{
    // initial call of the method - find <Routing> -> <RIP>
    // and then get first "Network" child node
    if (device != NULL){

       cXMLElement *routing = device->getFirstChildWithTag("Routing");
       if (routing == NULL)
          return NULL;

       cXMLElement *RIP = routing->getFirstChildWithTag("RIP");
       if (routing == NULL)
          return NULL;

       network = RIP->getFirstChildWithTag("Network");

    // repeated call - get another "Network" sibling node
    }else if (network != NULL){
        network = network->getNextSiblingWithTag("Network");
    }else{
        network = NULL;
    }

    return network;
}

cXMLElement *xmlParser::GetRIPPassiveInterface(cXMLElement *passiveInterface, cXMLElement *device)
{
    // initial call of the method - find <Routing> -> <RIP>
    // and then get first "Passive-interface" child node
    if (device != NULL){

      cXMLElement *routing = device->getFirstChildWithTag("Routing");
      if (routing == NULL)
         return NULL;

      cXMLElement *RIP = routing->getFirstChildWithTag("RIP");
      if (routing == NULL)
         return NULL;

      passiveInterface = RIP->getFirstChildWithTag("Passive-interface");

    // repeated call - get another "Passive-interface" sibling node
    }else if (passiveInterface != NULL){
        passiveInterface = passiveInterface->getNextSiblingWithTag("Passive-interface");
    }else{
        passiveInterface = NULL;
    }

    return passiveInterface;
}

const char *xmlParser::GetRIPInterfaceSplitHorizon(cXMLElement *iface)
{
    return GetNodeParamConfig(iface, "RIPSplitHorizon", "enable");
}

const char *xmlParser::GetRIPInterfacePoisonReverse(cXMLElement *iface)
{
    return GetNodeParamConfig(iface, "RIPPoisonReverse", "disable");
}

bool xmlParser::isMulticastEnabled(cXMLElement *device)
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
}

cXMLElement * xmlParser::GetPimGlobal(cXMLElement * device)
{
    if(device == NULL)
        return NULL;

    cXMLElement *routing = device->getFirstChildWithTag("Routing");
    if(routing == NULL)
        return  NULL;

    cXMLElement * multicast = routing->getFirstChildWithTag("Multicast");
    if(multicast == NULL)
        return  NULL;

    cXMLElement * pimGlobal = multicast->getFirstChildWithTag("Pim");

    return  pimGlobal;
}

cXMLElement * xmlParser::GetVRRPGroup(cXMLElement *group, cXMLElement *iface) {

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

cXMLElement * xmlParser::GetVRRPGroup(cXMLElement *device, const char* name, const char* groupId)
{
    string path = "./Interfaces/Interface[\@name='" ;
    path += name;
    path += "']/VRRP/Group[\@id='";
    path += groupId;
    path += "']";

    return device->getElementByPath(path.c_str());
}

bool xmlParser::HasVRPPGroup(cXMLElement* group, int *groupId)
{
    cXMLElement *address = group->getFirstChildWithTag("IPAddress");
    if (address == NULL)
        return false;

    if (!xmlParser::Str2Int(groupId, group->getAttribute("id")))
        return false;

    if ((* groupId) < 0 || (* groupId) > 255)
        return false;

    return true;
}

//
//
//- configuration for EIGRP
//
//
cXMLElement *xmlParser::GetEigrpProcess(cXMLElement *process, cXMLElement *device)
{
    // initial call of the method - get first "AS" child node in "EIGRP"
    if (device != NULL)
    {
        cXMLElement *routing = device->getFirstChildWithTag("Routing");
        if (routing == NULL)
            return NULL;

        cXMLElement *eigrp = routing->getFirstChildWithTag("EIGRP");
        if (eigrp == NULL)
            return NULL;

        process = eigrp->getFirstChildWithTag("ProcessIPv4");

    // repeated call - get another "AS" sibling node
    }
    else if (process != NULL)
    {
        process = process->getNextSiblingWithTag("ProcessIPv4");
    }
    else
    {
        process = NULL;
    }

    return process;
}

cXMLElement *xmlParser::GetEigrpIPv4Network(cXMLElement *network, cXMLElement *process)
{
    // initial call of the method - find first "Network" node in process
    if (process != NULL){

       network = process->getFirstChildWithTag("Network");

    // repeated call - get another "Network" sibling node
    }else if (network != NULL){
        network = network->getNextSiblingWithTag("Network");
    }else{
        network = NULL;
    }

    return network;
}
