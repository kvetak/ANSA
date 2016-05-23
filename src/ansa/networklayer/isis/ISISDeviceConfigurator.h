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

#ifndef __INET_ISISDEVICECONFIGURATOR_H_
#define __INET_ISISDEVICECONFIGURATOR_H_

#include <omnetpp.h>

/*#include "RoutingTable6Access.h"
#include "InterfaceTableAccess.h"
#include "AnsaRoutingTableAccess.h"
#include "PimInterfaceTable.h"
#include "IPv4InterfaceData.h"*/

#include "ISISAccess.h"
#include "TRILLAccess.h"

class ISISDeviceConfigurator
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


      /////////////////////////
      //    ISIS related     //
      /////////////////////////
      void loadISISCoreDefaultConfig(ISIS *isisModule);
      void loadISISInterfaceDefaultConfig(ISIS *isisModule, InterfaceEntry *entry);
      void loadISISInterfacesConfig(ISIS *isisModule);
      void loadISISInterfaceConfig(ISIS *isisModule, InterfaceEntry *entry, cXMLElement *intElement);
      const char *getISISNETAddress(cXMLElement *isisRouting);
      short int getISISISType(cXMLElement *isisRouting);
      int getISISL1HelloInterval(cXMLElement *isisRouting);
      int getISISL1HelloMultiplier(cXMLElement *isisRouting);
      int getISISL2HelloInterval(cXMLElement *isisRouting);
      int getISISL2HelloMultiplier(cXMLElement *isisRouting);
      int getISISLSPInterval(cXMLElement *isisRouting);
      int getISISLSPRefreshInterval(cXMLElement *isisRouting);
      int getISISLSPMaxLifetime(cXMLElement *isisRouting);
      int getISISL1LSPGenInterval(cXMLElement *isisRouting);
      int getISISL2LSPGenInterval(cXMLElement *isisRouting);
      int getISISL1LSPSendInterval(cXMLElement *isisRouting);
      int getISISL2LSPSendInterval(cXMLElement *isisRouting);
      int getISISL1LSPInitWait(cXMLElement *isisRouting);
      int getISISL2LSPInitWait(cXMLElement *isisRouting);
      int getISISL1CSNPInterval(cXMLElement *isisRouting);
      int getISISL2CSNPInterval(cXMLElement *isisRouting);
      int getISISL1PSNPInterval(cXMLElement *isisRouting);
      int getISISL2PSNPInterval(cXMLElement *isisRouting);
      int getISISL1SPFFullInterval(cXMLElement *isisRouting);
      int getISISL2SPFFullInterval(cXMLElement *isisRouting);

   public:
      ISISDeviceConfigurator();
      ISISDeviceConfigurator(const char* devId, const char* devType, const char* confFile, IInterfaceTable* intf);
      virtual ~ISISDeviceConfigurator();

      //static bool Str2Int(int *retValue, const char *str);
      //static bool Str2Bool(bool *ret, const char *str);

      static cXMLElement * GetDevice(const char *deviceType, const char *deviceId, const char *configFile);
      static cXMLElement * GetInterface(cXMLElement *iface, cXMLElement *device);
      //static cXMLElement * GetStaticRoute(cXMLElement *route, cXMLElement *device);
      //static cXMLElement * GetStaticRoute6(cXMLElement *route, cXMLElement *device);

      //static cXMLElement *GetAdvPrefix(cXMLElement *prefix, cXMLElement *iface);
      //static cXMLElement *GetIPv6Address(cXMLElement *addr, cXMLElement *iface);

      //static bool isMulticastEnabled(cXMLElement *device);


      ////////////////////////
      //    IGMP Related    //
      ////////////////////////
      //void addIPv4MulticastGroups(cXMLElement *iface);
      //void addIPv6MulticastGroups(cXMLElement *iface);


      /////////////////////////
      //    ISIS related     //
      /////////////////////////
      /*
       * Loads configuraion for IS-IS module.
       * @param isisModule [in]
       * @param isisMode [in] L2_ISIS_MODE or L3_ISIS_MODE
       */
      void loadISISConfig(ISIS *isisModule, ISIS::ISIS_MODE isisMode);
      static cXMLElement * GetIsisRouting(cXMLElement * device);

};



#endif
