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

// Copyright (C) 2011 - 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
 * @file deviceConfigurator.h
 * @author Marek Cerny, Jiri Trhlik (mailto:jiritm@gmail.com), Tomas Prochazka, etc ... DOPLNTE Marcel Marek (mailto:xscrew02@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 2011
 * @brief
 * @detail
 * @todo Z9
 */

#ifndef __ANSAINET_CONFIGLOADER_H_
#define __ANSAINET_CONFIGLOADER_H_

#include <omnetpp.h>

#include "RoutingTable6Access.h"
#include "InterfaceTableAccess.h"
#include "AnsaRoutingTableAccess.h"
#include "PimInterfaceTable.h"
#include "IPv4InterfaceData.h"

#include "AnsaRoutingTable.h"

#include "RIPngRouting.h"
#include "RIPRouting.h"
#include "pimSM.h"
#include "VRRPv2.h"
#include "VRRPv2VirtualRouter.h"
#include "IEigrpModule.h"

/* TRILL */
#include "TRILLAccess.h"
/* IS-IS */
#include "ISISAccess.h"
/* END IS-IS */
/* END TRILL */


class DeviceConfigurator : public cSimpleModule {

   private:
      const char *deviceType;
      const char *deviceId;
      const char *configFile;
      cXMLElement *device;

   protected:
      IInterfaceTable *ift;
      RoutingTable6 *rt6;
      IRoutingTable *rt;
      PimInterfaceTable *pimIft;        /**< Link to table of PIM interfaces. */

      virtual int numInitStages() const {return 11;}
      virtual void initialize(int stage);
      virtual void handleMessage(cMessage *msg);

      //////////////////////////
      /// IPv4 Configuration ///
      //////////////////////////
      void loadDefaultRouter(cXMLElement *gateway);
      void loadInterfaceConfig(cXMLElement* iface);
      void loadStaticRouting(cXMLElement* route);

      //////////////////////////
      /// IPv6 Configuration ///
      //////////////////////////
      void loadDefaultRouter6(cXMLElement *gateway);
      void loadInterfaceConfig6(cXMLElement *iface);
      void loadStaticRouting6(cXMLElement *route);

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
      /* END of ISIS related */

      ///////////////////////////
      // configuration for PIM //
      ///////////////////////////
      void loadPimInterfaceConfig(cXMLElement *iface);

      /////////////////////////////
      // configuration for EIGRP //
      /////////////////////////////
      /**< Gets interfaces that correspond to the IP address and mask */
      std::vector<int> getInterfacesByIpv4Prefix(const IPv4Address& prefix, const IPv4Address& mask);
      /**< Converts wildcard to netmask and check validity */
      bool wildcardToMask(const char *wildcard, IPv4Address& result);
      /**< Loads configuration of EIGRP process */
      void loadEigrpProcessesConfig(cXMLElement *device, IEigrpModule *eigrpModule);
      /**< Loads configuration of interface for EIGRP */
      void loadEigrpInterfaceConfig(cXMLElement *device, IEigrpModule *eigrpModule);
      /**< Loads networks added to EIGRP */
      void loadEigrpIPv4Networks(cXMLElement *processElem, IEigrpModule *eigrpModule);
      /**< Loads K-value and converts it to number */
      int loadEigrpKValue(cXMLElement *node, const char *attrName, const char *attrValue);

   public:
      // global configuration for PIM
      void loadPimGlobalConfig(pimSM *pimSMModule);

      /////////////////////////////
      // configuration for RIPng //
      /////////////////////////////
      /**
       * Loads configuration for RIPngModule
       * @param RIPngModule [in]
       */
      void loadRIPngConfig(RIPngRouting *RIPngModule);

      /**
       * Adds unicast prefixes obtained from the interface configuration to the RIPngRouting table
       * @param RIPngModule [in]
       * @param interface [in] interface, from which should be added prefixes
       * @see InterfaceTable
       */
      void loadPrefixesFromInterfaceToRIPngRT(RIPngProcess *process, cXMLElement *interface);

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

      /////////////////////////
      //    ISIS related     //
      /////////////////////////
      /*
       * Loads configuraion for IS-IS module.
       * @param isisModule [in]
       * @param isisMode [in] L2_ISIS_MODE or L3_ISIS_MODE
       */
      void loadISISConfig(ISIS *isisModule, ISIS::ISIS_MODE isisMode);


      ////////////////////////
      //    IGMP Related    //
      ////////////////////////

      void addIPv4MulticastGroups(cXMLElement *iface);
      void addIPv6MulticastGroups(cXMLElement *iface);

      //////////////////////////////
      // Configuration for VRRPv2 //
      //////////////////////////////
      /**
       * Loads configuration for VRRPv2
       * @param VRRPModule [in]
       */
      void loadVRRPv2Config(VRRPv2* VRRPModule);
      void loadVRRPv2VirtualRouterConfig(VRRPv2VirtualRouter* VRRPModule);

      /////////////////////////////
      // configuration for EIGRP //
      /////////////////////////////
      /**
       * Loads configuration for EIGRP
       * @param eigrpModule [in]
       */
      void loadEigrpIPv4Config(IEigrpModule *eigrpModule);



};

#endif
