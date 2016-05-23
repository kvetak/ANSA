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

#ifndef __INET_RIPDEVICECONFIGURATOR_H_
#define __INET_RIPDEVICECONFIGURATOR_H_


#include <omnetpp.h>

/*
#include "RoutingTable6Access.h"
#include "InterfaceTableAccess.h"
#include "AnsaRoutingTableAccess.h"
#include "PimInterfaceTable.h"
#include "IPv4InterfaceData.h"
#include "RoutingTableAccess.h"
*/

#include "RIPRouting.h"

class RIPDeviceConfigurator
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

   public:
      RIPDeviceConfigurator();
      RIPDeviceConfigurator(const char* devId, const char* devType, const char* confFile, IInterfaceTable* intf);
      virtual ~RIPDeviceConfigurator();
   //xmlParser
      //static bool Str2Int(int *retValue, const char *str);
      //static bool Str2Bool(bool *ret, const char *str);

      static cXMLElement * GetDevice(const char *deviceType, const char *deviceId, const char *configFile);
      static cXMLElement * GetInterface(cXMLElement *iface, cXMLElement *device);
      //static cXMLElement * GetStaticRoute(cXMLElement *route, cXMLElement *device);
      //static cXMLElement * GetStaticRoute6(cXMLElement *route, cXMLElement *device);

      static const char *GetNodeParamConfig(cXMLElement *node, const char *paramName, const char *defaultValue);


      //static cXMLElement *GetAdvPrefix(cXMLElement *prefix, cXMLElement *iface);
      //static cXMLElement *GetIPv6Address(cXMLElement *addr, cXMLElement *iface);

      //static bool isMulticastEnabled(cXMLElement *device);


      // configuration for RIP
      static cXMLElement *GetRIPNetwork(cXMLElement *network, cXMLElement *device);
      static cXMLElement *GetRIPPassiveInterface(cXMLElement *passiveInterface, cXMLElement *device);
      static const char  *GetRIPInterfaceSplitHorizon(cXMLElement *iface);
      static const char  *GetRIPInterfacePoisonReverse(cXMLElement *iface);


       ///////////////////////////
       // configuration for RIP //
       ///////////////////////////
       /**
        * Loads configuration for RIPModule
        * @param RIPModule [in]
        */
       void loadRIPConfig(RIPRouting *RIPModule);

       /**
        * Adds networks obtained from the interface configuration to the RIPRouting table
        * @param RIPModule [in]
        * @param interface [in] interface, from which should be added networks
        */
       void loadNetworksFromInterfaceToRIPRT(RIPRouting *RIPModule, InterfaceEntry *interface);

      ////////////////////////
      //    IGMP Related    //
      ////////////////////////

      //void addIPv4MulticastGroups(cXMLElement *iface);
      //void addIPv6MulticastGroups(cXMLElement *iface);

};

#endif
