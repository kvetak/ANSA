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
* @file CDPDeviceConfigurator.h
*/

#ifndef CDPDEVICECONFIGURATOR_H_
#define CDPDEVICECONFIGURATOR_H_

#include <omnetpp.h>

#include "ansa/linklayer/cdp/CDPMain.h"
#include "ansa/linklayer/cdp/tables/CDPInterfaceTable.h"

namespace inet {

class CDPDeviceConfigurator
{
   private:
      const char *deviceId = nullptr;
      const char *deviceType = nullptr;
      cXMLElement* configFile = nullptr;
      cXMLElement *device = nullptr;

   protected:
      IInterfaceTable *ift = nullptr;

   public:
      CDPDeviceConfigurator();
      CDPDeviceConfigurator(const char* devId, const char* devType, cXMLElement* confFile, IInterfaceTable* intf);
      virtual ~CDPDeviceConfigurator();

      //xmlParser
      static bool Str2Int(int *retValue, const char *str);
      bool Str2Bool(bool *ret, const char *str);

      static cXMLElement * GetDevice(const char *deviceType, const char *deviceId, cXMLElement* configFile);
      static cXMLElement * GetInterface(cXMLElement *iface, cXMLElement *device);

      static cXMLElement *GetODRProcess(cXMLElement *device);
      void loadODRProcessConfig(cXMLElement *device, CDPMain *cMain);

      /////////////////////////////
      // configuration for CDP   //
      /////////////////////////////
      void loadCDPConfig(CDPMain *cMain);
      void loadCDPInterfacesConfig(cXMLElement *device, CDPMain *lMain);
      void loadCDPInterface(cXMLElement *ifaceElem, CDPMain *lMain, CDPInterface *lIface);
};

} /* namespace inet */

#endif /* CDPDEVICECONFIGURATOR_H_ */
