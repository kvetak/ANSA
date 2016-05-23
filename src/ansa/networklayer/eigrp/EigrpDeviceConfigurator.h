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

#ifndef __INET_EIGRPDEVICECONFIGURATOR_H_
#define __INET_EIGRPDEVICECONFIGURATOR_H_

#include <omnetpp.h>
/*
#include "RoutingTable6Access.h"
#include "InterfaceTableAccess.h"
#include "RoutingTableAccess.h"
#include "PimInterfaceTable.h"*/
#include "IPv4InterfaceData.h"
#include "AnsaRoutingTableAccess.h"

#include "IEigrpModule.h"
#include "EigrpNetworkTable.h"
#include "IInterfaceTable.h"

class EigrpDeviceConfigurator
{
   private:
      const char *deviceType = NULL;
      const char *deviceId = NULL;
      const char *configFile = NULL;
      cXMLElement *device = NULL;

   protected:
      IInterfaceTable *ift = NULL;
      //RoutingTable6 *rt6;
      //IRoutingTable *rt;
      //PimInterfaceTable *pimIft;        /**< Link to table of PIM interfaces. */

      //virtual int numInitStages() const {return 11;}
      //virtual void initialize(int stage);
      //virtual void handleMessage(cMessage *msg);

      //////////////////////////
      /// IPv4 Configuration ///
      //////////////////////////
      //void loadDefaultRouter(cXMLElement *gateway);
      //void loadInterfaceConfig(cXMLElement* iface);
      //void loadStaticRouting(cXMLElement* route);

      //////////////////////////
      /// IPv6 Configuration ///
      //////////////////////////
      //void loadDefaultRouter6(cXMLElement *gateway);
      //void loadInterfaceConfig6(cXMLElement *iface);
      //void loadStaticRouting6(cXMLElement *route);

      /**< Sets default bandwidth and delay */
      //void setInterfaceParamters(InterfaceEntry *interface);
      /**< Returns default delay of interface by link type */
      //double getDefaultDelay(const char *linkType);


      ///////////////////////////
      // configuration for PIM //
      ///////////////////////////
      //void loadPimInterfaceConfig(cXMLElement *iface);

      /////////////////////////////
      // configuration for EIGRP //
      /////////////////////////////
      /**< Gets interfaces that correspond to the IP address and mask */
      EigrpNetwork<IPv4Address> *isEigrpInterface(std::vector<EigrpNetwork<IPv4Address> *>& networks, InterfaceEntry *interface);
      /**< Converts wildcard to netmask and check validity */
      bool wildcardToMask(const char *wildcard, IPv4Address& result);
      /**< Loads configuration of EIGRP process */
      void loadEigrpProcessesConfig(cXMLElement *device, IEigrpModule<IPv4Address> *eigrpModule);
      /**< Loads configuration of interfaces for EIGRP */
      void loadEigrpInterfacesConfig(cXMLElement *device, IEigrpModule<IPv4Address> *eigrpModule);
      void loadEigrpInterface(cXMLElement *eigrpIface, IEigrpModule<IPv4Address> *eigrpModule, int ifaceId, const char *ifaceName);
      /**< Loads networks added to EIGRP */
      void loadEigrpIPv4Networks(cXMLElement *processElem, IEigrpModule<IPv4Address> *eigrpModule);
      /**< Loads K-value and converts it to number */
      int loadEigrpKValue(cXMLElement *node, const char *attrName, const char *attrValue);
      /**< Loads stub configuration */
      bool loadEigrpStubConf(cXMLElement *node, const char *attrName);

      /**< Loads configuration of EIGRP IPv6 process */
      void loadEigrpProcesses6Config(cXMLElement *device, IEigrpModule<IPv6Address> *eigrpModule);
      /**< Loads configuration of interfaces for EIGRP IPv6 */
      void loadEigrpInterfaces6Config(cXMLElement *device, IEigrpModule<IPv6Address> *eigrpModule);
      /**< Loads interfaces for EIGRP IPv6 */
      void loadEigrpInterface6(cXMLElement *eigrpIface, IEigrpModule<IPv6Address> *eigrpModule, int ifaceId, const char *ifaceName);

   public:
      EigrpDeviceConfigurator();
      EigrpDeviceConfigurator(const char* devId, const char* devType, const char* confFile, IInterfaceTable* intf);
      virtual ~EigrpDeviceConfigurator();

      static bool Str2Int(int *retValue, const char *str);
      static bool Str2Bool(bool *ret, const char *str);

      static cXMLElement * GetDevice(const char *deviceType, const char *deviceId, const char *configFile);
      static cXMLElement * GetInterface(cXMLElement *iface, cXMLElement *device);
      static cXMLElement * GetStaticRoute(cXMLElement *route, cXMLElement *device);
      static cXMLElement * GetStaticRoute6(cXMLElement *route, cXMLElement *device);

      //static cXMLElement *GetAdvPrefix(cXMLElement *prefix, cXMLElement *iface);
      static cXMLElement *GetIPv6Address(cXMLElement *addr, cXMLElement *iface);

      //static bool isMulticastEnabled(cXMLElement *device);

      static const char *GetNodeParamConfig(cXMLElement *node, const char *paramName, const char *defaultValue);
      static const char *GetNodeAttrConfig(cXMLElement *node, const char *attrName, const char *defaultValue);


      ////////////////////////
      //    IGMP Related    //
      ////////////////////////

      //void addIPv4MulticastGroups(cXMLElement *iface);
      //void addIPv6MulticastGroups(cXMLElement *iface);

      /////////////////////////////
      // configuration for EIGRP //
      /////////////////////////////
      /**
       * Loads configuration for EIGRP
       * @param eigrpModule [in]
       */
      void loadEigrpIPv4Config(IEigrpModule<IPv4Address> *eigrpModule);


      /**
       * Loads configuration for EIGRP IPv6
       * @param eigrpModule [in]
       */
      void loadEigrpIPv6Config(IEigrpModule<IPv6Address> *eigrpModule);

      static cXMLElement *GetEigrpProcess(cXMLElement *process, cXMLElement *device);
      static cXMLElement *GetEigrpIPv4Network(cXMLElement *network, cXMLElement *process);
      static cXMLElement *GetEigrpProcess6(cXMLElement *process, cXMLElement *device);
};



#endif
