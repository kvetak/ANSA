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
* @author Vit Rek (rek@kn.vutbr.cz)
* @author Vladimir Vesely (ivesely@fit.vutbr.cz)
* @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
*/

#ifndef __INET_BABELDEVICECONFIGURATOR_H_
#define __INET_BABELDEVICECONFIGURATOR_H_


#include <omnetpp.h>
#include <algorithm>

/*
#include "RoutingTable6Access.h"
#include "InterfaceTableAccess.h"
#include "AnsaRoutingTableAccess.h"
#include "PimInterfaceTable.h"
#include "Ipv4InterfaceData.h"
#include "ANSARoutingTable6Access.h"
#include "RoutingTableAccess.h"
*/

#include "ansa/routing/babel/BabelMain.h"
#include "ansa/routing/babel/BabelInterfaceTable.h"
#include "ansa/routing/babel/cost/IBabelCostComputation.h"
namespace inet {
class INET_API BabelDeviceConfigurator
        //: public cSimpleModule
{

   private:

      cXMLElement* configFile = nullptr;
      cXMLElement* device = nullptr;

   protected:
      IInterfaceTable *ift = nullptr;

   public:
      BabelDeviceConfigurator(cXMLElement* confFile, IInterfaceTable* intf);
      virtual ~BabelDeviceConfigurator();
      //xmlParser
      static bool Str2Int(int *retValue, const char *str);
      static bool Str2Bool(bool *ret, const char *str);

      static cXMLElement * GetDevice(const char *deviceType, const char *deviceId, cXMLElement* configFile);
      static cXMLElement * GetInterface(cXMLElement *iface, cXMLElement *device);
      // configuration for Babel
      static cXMLElement *GetBabelProcess(cXMLElement *device);
      static cXMLElement *GetAdvPrefix(cXMLElement *prefix, cXMLElement *iface);
      static cXMLElement *GetIPv6Address(cXMLElement *addr, cXMLElement *iface);

      /////////////////////////////
      // configuration for Babel //
      /////////////////////////////
      void loadBabelConfig(BabelMain *bMain);
      void loadBabelProcessConfig(cXMLElement *device, BabelMain *bMain);
      void loadBabelInterfacesConfig(cXMLElement *device, BabelMain *bMain);
      void loadBabelInterface(cXMLElement *ifaceElem, BabelMain *bMain, BabelInterface *bIface);
};


}
#endif
