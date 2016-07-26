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

#ifndef __INET_VRRPV2DEVICECONFIGURATOR_H_
#define __INET_VRRPV2DEVICECONFIGURATOR_H_

#include <omnetpp.h>

#include "ansa/routing/vrrp/VRRPv2.h"
#include "ansa/routing/vrrp/VRRPv2VirtualRouter.h"

namespace inet {

class VRRPv2DeviceConfigurator
{
   private:
      cXMLElement* configFile = nullptr;

   protected:
      IInterfaceTable *ift = nullptr;

   public:
      VRRPv2DeviceConfigurator();
      VRRPv2DeviceConfigurator(cXMLElement* confFile, IInterfaceTable* intf);
      virtual ~VRRPv2DeviceConfigurator();

      static bool Str2Int(int *retValue, const char *str);
      static bool Str2Bool(bool *ret, const char *str);

      static cXMLElement * GetInterface(cXMLElement *iface, cXMLElement *device);

      //////////////////////////////
      // Configuration for VRRPv2 //
      //////////////////////////////
      /**
       * Loads configuration for VRRPv2
       * @param VRRPModule [in]
       */
      void loadVRRPv2Config(VRRPv2* VRRPModule);
      void loadVRRPv2VirtualRouterConfig(VRRPv2VirtualRouter* VRRPModule);

      static cXMLElement *GetVRRPGroup(cXMLElement *group, cXMLElement *iface);
      static cXMLElement *GetVRRPGroup(cXMLElement *device, const char* name, const char* groupId);
      static bool HasVRPPGroup(cXMLElement* group, int *groupId);

};

}
#endif
