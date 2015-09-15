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

#include "networklayer/ipv6/RoutingTable6Access.h"
#include "networklayer/common/InterfaceTableAccess.h"
#include "ansa/networklayer/ipv4/AnsaRoutingTableAccess.h"
#include "ansa/networklayer/pim/tables/PimInterfaceTable.h"
#include "networklayer/ipv4/IPv4InterfaceData.h"

#include "ansa/networklayer/ipv4/AnsaRoutingTable.h"

#include "ansa/applications/ripng/RIPngRouting.h"
#include "ansa/applications/rip/RIPRouting.h"
#include "ansa/networklayer/pim/modes/pimSM.h"
#include "ansa/networklayer/vrrpv2/VRRPv2.h"
#include "ansa/networklayer/vrrpv2/VRRPv2VirtualRouter.h"
#include "ansa/networklayer/eigrp/pdms/IEigrpModule.h"
#include "ansa/networklayer/eigrp/tables/EigrpNetworkTable.h"

/* TRILL */
#include "ansa/linklayer/rbridge/TRILLAccess.h"
/* IS-IS */
#include "ansa/networklayer/isis/ISISAccess.h"
/* END IS-IS */
/* END TRILL */


/* Babel */
#include "ansa/applications/babel/BabelMain.h"
#include "ansa/applications/babel/BabelInterfaceTable.h"
#include "ansa/applications/babel/cost/IBabelCostComputation.h"
/* End Babel */






class DeviceConfigurator : public cSimpleModule {

   private:
      const char *deviceType;
      const char *deviceId;
      const char *configFile;
      cXMLElement *device;

   protected:
      inet::IInterfaceTable *ift;
      inet::IPv6RoutingTable *rt6;
      inet::IRoutingTable *rt;
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

      /**< Sets default bandwidth and delay */
      void setInterfaceParamters(inet::InterfaceEntry *interface);
      /**< Returns default delay of interface by link type */
      double getDefaultDelay(const char *linkType);

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
      void loadISISInterfaceDefaultConfig(ISIS *isisModule, inet::InterfaceEntry *entry);
      void loadISISInterfacesConfig(ISIS *isisModule);
      void loadISISInterfaceConfig(ISIS *isisModule, inet::InterfaceEntry *entry, cXMLElement *intElement);
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
      EigrpNetwork<inet::IPv4Address> *isEigrpInterface(std::vector<EigrpNetwork<inet::IPv4Address> *>& networks, inet::InterfaceEntry *interface);
      /**< Converts wildcard to netmask and check validity */
      bool wildcardToMask(const char *wildcard, inet::IPv4Address& result);
      /**< Loads configuration of EIGRP process */
      void loadEigrpProcessesConfig(cXMLElement *device, IEigrpModule<inet::IPv4Address> *eigrpModule);
      /**< Loads configuration of interfaces for EIGRP */
      void loadEigrpInterfacesConfig(cXMLElement *device, IEigrpModule<inet::IPv4Address> *eigrpModule);
      void loadEigrpInterface(cXMLElement *eigrpIface, IEigrpModule<inet::IPv4Address> *eigrpModule, int ifaceId, const char *ifaceName);
      /**< Loads networks added to EIGRP */
      void loadEigrpIPv4Networks(cXMLElement *processElem, IEigrpModule<inet::IPv4Address> *eigrpModule);
      /**< Loads K-value and converts it to number */
      int loadEigrpKValue(cXMLElement *node, const char *attrName, const char *attrValue);
      /**< Loads stub configuration */
      bool loadEigrpStubConf(cXMLElement *node, const char *attrName);

      /**< Loads configuration of EIGRP IPv6 process */
      void loadEigrpProcesses6Config(cXMLElement *device, IEigrpModule<inet::IPv6Address> *eigrpModule);
      /**< Loads configuration of interfaces for EIGRP IPv6 */
      void loadEigrpInterfaces6Config(cXMLElement *device, IEigrpModule<inet::IPv6Address> *eigrpModule);
      /**< Loads interfaces for EIGRP IPv6 */
      void loadEigrpInterface6(cXMLElement *eigrpIface, IEigrpModule<inet::IPv6Address> *eigrpModule, int ifaceId, const char *ifaceName);


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
      void loadNetworksFromInterfaceToRIPRT(RIPRouting *RIPModule, inet::InterfaceEntry *interface);

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
      void loadEigrpIPv4Config(IEigrpModule<inet::IPv4Address> *eigrpModule);


      /**
       * Loads configuration for EIGRP IPv6
       * @param eigrpModule [in]
       */
      void loadEigrpIPv6Config(IEigrpModule<inet::IPv6Address> *eigrpModule);



      /////////////////////////////
      // configuration for Babel //
      /////////////////////////////
      void loadBabelConfig(BabelMain *bMain);
      void loadBabelProcessConfig(cXMLElement *device, BabelMain *bMain);
      void loadBabelInterfacesConfig(cXMLElement *device, BabelMain *bMain);
      void loadBabelInterface(cXMLElement *ifaceElem, BabelMain *bMain, BabelInterface *bIface);
};

#endif
