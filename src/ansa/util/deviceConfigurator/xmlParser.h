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

#ifndef XMLPARSER_H_
#define XMLPARSER_H_

#include <omnetpp.h>

using namespace std;

/*
 * Set of static methods that can be used by multiple modules for unified
 * access to various pars of the configuration XML file.
 *
 * The idea is to provide tools that will take care about tag parsing while
 * the calling module can focus on extracting actual aplication-specific data.
 */
class xmlParser {

   public:
      static cXMLElement * GetDevice(const char *deviceType, const char *deviceId, const char *configFile);
      static cXMLElement * GetInterface(cXMLElement *iface, cXMLElement *device);
      static cXMLElement * GetStaticRoute(cXMLElement *route, cXMLElement *device);
      static cXMLElement * GetStaticRoute6(cXMLElement *route, cXMLElement *device);
      static cXMLElement * GetOspfProcess6(cXMLElement *process, cXMLElement *device);
      static cXMLElement * GetIPv6Address(cXMLElement *addr, cXMLElement *iface);
      static cXMLElement * GetAdvPrefix(cXMLElement *prefix, cXMLElement *iface);
      static cXMLElement * GetIsisRouting(cXMLElement * device);
      static cXMLElement * GetPimGlobal(cXMLElement * device);
      static bool isMulticastEnabled(cXMLElement *device);
      static bool Str2Int(int *retValue, const char *str);
      static bool Str2Bool(bool *ret, const char *str);

      static const char *GetNodeParamConfig(cXMLElement *node, const char *paramName, const char *defaultValue);
      static const char *GetNodeAttrConfig(cXMLElement *node, const char *attrName, const char *defaultValue);

      // configuration for RIPng
      static cXMLElement *GetRIPngProcess(cXMLElement *process, cXMLElement *device);
      static cXMLElement *GetRIPngProcessTimers(cXMLElement *process);
      static cXMLElement *GetInterfaceRIPngProcess(cXMLElement *ripng, cXMLElement *iface);
      static const char  *GetInterfaceRIPngPassiveStatus(cXMLElement *ripng);
      static const char  *GetInterfaceRIPngSplitHorizon(cXMLElement *ripng);
      static const char  *GetInterfaceRIPngPoisonReverse(cXMLElement *ripng);
      static cXMLElement *GetInterfaceRIPngDefaultInformation(cXMLElement *ripng);
      static const char  *GetInterfaceRIPngMetricOffset(cXMLElement *ripng);

      // configuration for RIP
      static cXMLElement *GetRIPNetwork(cXMLElement *network, cXMLElement *device);
      static cXMLElement *GetRIPPassiveInterface(cXMLElement *passiveInterface, cXMLElement *device);
      static const char  *GetRIPInterfaceSplitHorizon(cXMLElement *iface);
      static const char  *GetRIPInterfacePoisonReverse(cXMLElement *iface);

      // configuration for VRRP
      static cXMLElement *GetVRRPGroup(cXMLElement *group, cXMLElement *iface);
      static cXMLElement *GetVRRPGroup(cXMLElement *device, const char* name, const char* groupId);
      static bool HasVRPPGroup(cXMLElement* group, int *groupId);

      // configuration for EIGRP
      static cXMLElement *GetEigrpProcess(cXMLElement *process, cXMLElement *device);
      static cXMLElement *GetEigrpIPv4Network(cXMLElement *network, cXMLElement *process);

      // configuration for LISP
      static cXMLElement* GetLISPRouting(cXMLElement* device);
      static cXMLElement* GetLISPMapServerAddr(cXMLElement* ele, cXMLElement* lisp);
      static cXMLElement* GetLISPMapResolverAddr(cXMLElement* ele, cXMLElement* lisp);
      static cXMLElement* GetLISPSite(cXMLElement* ele, cXMLElement* ms);
      static cXMLElement* GetLISPSiteKey(cXMLElement* ele, cXMLElement* site);
      static cXMLElement* GetLISPSiteEid(cXMLElement* ele, cXMLElement* site);
      static cXMLElement* GetLISPMapServer(cXMLElement *ele, cXMLElement* lisp);
      static cXMLElement* GetLISPMapResolver(cXMLElement* ele, cXMLElement* lisp);
      static cXMLElement* GetLISPMapping(cXMLElement* ele, cXMLElement* lisp);
      static cXMLElement* GetLISPMappingEid(cXMLElement* ele, cXMLElement* mapping);
      static cXMLElement* GetLISPMappingRloc(cXMLElement* ele, cXMLElement* mapping);
      static cXMLElement* GetLISPMappingPriority(cXMLElement* ele, cXMLElement* mapping);
      static cXMLElement* GetLISPMappingWeight(cXMLElement* ele, cXMLElement* mapping);
      static cXMLElement* GetLISPElement(cXMLElement* element, const char* tag, cXMLElement* parent);
};

#endif /* XMLPARSER_H_ */
