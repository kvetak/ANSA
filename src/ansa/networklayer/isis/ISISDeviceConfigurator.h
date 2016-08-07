// Copyright (C) 2012 - 2016 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
 * @file ISISDeviceConfigurator.h
 * @author Marcel Marek (mailto:imarek@fit.vutbr.cz)
 * @date 5.8.2016
 */


#ifndef ANSA_NETWORKLAYER_ISIS_ISISDEVICECONFIGURATOR_H_
#define ANSA_NETWORKLAYER_ISIS_ISISDEVICECONFIGURATOR_H_


#include <omnetpp.h>



#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "ansa/networklayer/isis/ISIStypes.h"
#include "ansa/networklayer/isis/ISIS.h"

namespace inet {
class ISIS;

class ISISDeviceConfigurator {
private:


    const char *deviceId = nullptr;
    const char *deviceType = nullptr;
    cXMLElement* configFile = nullptr;
    cXMLElement *device = nullptr;

//   const char *deviceType;
//   const char *deviceId;
//   const char *configFile;
//   cXMLElement *device;
   IInterfaceTable *ift = nullptr;



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

         static cXMLElement * getIsisRouting(cXMLElement * device);

public:
    ISISDeviceConfigurator();
    ISISDeviceConfigurator(const char* devId, const char* devType, cXMLElement* confFile, IInterfaceTable* iftable);
    virtual ~ISISDeviceConfigurator();



    /////////////////////////
    //    ISIS related     //
    /////////////////////////
    /*
     * Loads configuraion for IS-IS module.
     * @param isisModule [in]
     * @param isisMode [in] L2_ISIS_MODE or L3_ISIS_MODE
     */
    void loadISISConfig(ISIS *isisModule, ISIS::ISIS_MODE isisMode);



};

} /* namespace inet */

#endif /* ANSA_NETWORKLAYER_ISIS_ISISDEVICECONFIGURATOR_H_ */
