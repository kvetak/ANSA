// The MIT License (MIT)
//
// Copyright (c) 2016 Brno University of Technology
//
//@author Vladimir Vesely (iveselyATfitDOTvutbrDOTcz)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
#ifndef __INET_BABELDEVICECONFIGURATOR_H_
#define __INET_BABELDEVICECONFIGURATOR_H_


#include <omnetpp.h>
#include <algorithm>

/*
#include "RoutingTable6Access.h"
#include "InterfaceTableAccess.h"
#include "AnsaRoutingTableAccess.h"
#include "PimInterfaceTable.h"
#include "IPv4InterfaceData.h"
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
      const char *deviceId = nullptr;
      const char *deviceType = nullptr;
      cXMLElement* configFile = nullptr;
      cXMLElement *device = nullptr;

   protected:
      IInterfaceTable *ift = nullptr;
      //IRoutingTable *rt6;
      //IRoutingTable *rt;
      //PimInterfaceTable *pimIft;        /**< Link to table of PIM interfaces. */

      //virtual int numInitStages() const {return 11;}
      //virtual void initialize(int stage);
      //virtual void handleMessage(cMessage *msg);
/*
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
*/
      /**< Sets default bandwidth and delay */
      //void setInterfaceParamters(InterfaceEntry *interface);
      /**< Returns default delay of interface by link type */
      //double getDefaultDelay(const char *linkType);


      ///////////////////////////
      // configuration for PIM //
      ///////////////////////////
//      void loadPimInterfaceConfig(cXMLElement *iface);

   public:
      BabelDeviceConfigurator();
      BabelDeviceConfigurator(const char* devId, const char* devType, cXMLElement* confFile, IInterfaceTable* intf);
      virtual ~BabelDeviceConfigurator();
      //xmlParser
      static bool Str2Int(int *retValue, const char *str);
      static bool Str2Bool(bool *ret, const char *str);

      static cXMLElement * GetDevice(const char *deviceType, const char *deviceId, cXMLElement* configFile);
      static cXMLElement * GetInterface(cXMLElement *iface, cXMLElement *device);
      //static cXMLElement * GetStaticRoute(cXMLElement *route, cXMLElement *device);
      //static cXMLElement * GetStaticRoute6(cXMLElement *route, cXMLElement *device);

      //static const char *GetNodeParamConfig(cXMLElement *node, const char *paramName, const char *defaultValue);
      //static const char *GetNodeAttrConfig(cXMLElement *node, const char *attrName, const char *defaultValue);
      // configuration for Babel
      static cXMLElement *GetBabelProcess(cXMLElement *device);
      static cXMLElement *GetAdvPrefix(cXMLElement *prefix, cXMLElement *iface);
      static cXMLElement *GetIPv6Address(cXMLElement *addr, cXMLElement *iface);

      //static bool isMulticastEnabled(cXMLElement *device);
      //end-xmlParser


      ////////////////////////
      //    IGMP Related    //
      ////////////////////////

      //void addIPv4MulticastGroups(cXMLElement *iface);
      //void addIPv6MulticastGroups(cXMLElement *iface);


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
