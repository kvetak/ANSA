//
// Marek Cerny, 2MSK
// FIT VUT 2011
//
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

#ifndef __ANSAINET_ANSAOSPFROUTING6_H_
#define __ANSAINET_ANSAOSPFROUTING6_H_

#include <omnetpp.h>

#include "AnsaInterfaceTable.h"
#include "AnsaInterfaceTableAccess.h"

#include "NotificationBoard.h"
#include "IPv6ControlInfo.h"
#include "IPAddress.h"

#include "xmlParser.h"

#include "ansaOspfCommon6.h"
#include "ansaOspfRouter6.h"
#include "ansaOspfPacket6_m.h"


class AnsaOspfRouting6 : public cSimpleModule, protected INotifiable {

   private:
      AnsaInterfaceTable*  ift;
      bool                 ospfEnabled;

      std::map<AnsaOspf6::ProcessID, AnsaOspf6::Router *>   routers;
      std::vector<AnsaOspf6::Router *>                      iface2Routers;
      std::map<std::string, std::string>                    interfaces;

   protected:
      NotificationBoard *nb;

   public:
      AnsaOspfRouting6();
      virtual ~AnsaOspfRouting6(void);

   private:
      void loadOspfRouting(cXMLElement *process);
      void loadInterfaceConfig(cXMLElement *iface);
      void addWatches(void);
      AnsaOspf6::Router * getOspfRouter(int interfaceId);

   protected:
      virtual int numInitStages() const {return 5;}
      virtual void initialize(int stage);
      virtual void handleMessage(cMessage *msg);
      virtual void receiveChangeNotification(int category, const cPolymorphic *details);
};

#endif
