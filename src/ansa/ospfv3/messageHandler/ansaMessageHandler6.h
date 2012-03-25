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

#ifndef ANSAMESSAGEHANDLER6_H_
#define ANSAMESSAGEHANDLER6_H_

#include "ansaIMessageHandler6.h"
#include "ansaHelloHandler6.h"
#include "ansaDatabaseDescriptionHandler6.h"
#include "ansaLinkStateRequestHandler6.h"
#include "ansaLinkStateUpdateHandler6.h"
#include "ansaLinkStateAcknowledgementHandler6.h"

#include "ansaOspfInterface6.h"
#include "ansaOspfTimer6_m.h"


namespace AnsaOspf6 {

class MessageHandler : public IMessageHandler {
private:
   cSimpleModule*                  ospfModule;

   HelloHandler                    helloHandler;
   DatabaseDescriptionHandler      ddHandler;
   LinkStateRequestHandler         lsRequestHandler;
   LinkStateUpdateHandler          lsUpdateHandler;
   LinkStateAcknowledgementHandler lsAckHandler;

public:
   MessageHandler (Router* containingRouter, cSimpleModule* containingModule);

   void    MessageReceived (cMessage* message);
   void    HandleTimer     (OspfTimer6* timer);

   void    ProcessPacket   (OspfPacket6* packet, Interface* unused1 = NULL, Neighbor* unused2 = NULL);

   void    SendPacket      (OspfPacket6* packet, IPv6Address destination, int outputIfIndex, short hopLimit = 1);
   void    ClearTimer      (OspfTimer6* timer);
   void    StartTimer      (OspfTimer6* timer, simtime_t delay);

   void    PrintEvent                           (const char* eventString, const Interface* onInterface = NULL, const Neighbor* forNeighbor = NULL) const;
   void    PrintHelloPacket                     (const OspfHelloPacket6* helloPacket, IPv6Address destination, int outputIfIndex) const;
   void    PrintDatabaseDescriptionPacket       (const OspfDatabaseDescriptionPacket6* ddPacket, IPv6Address destination, int outputIfIndex) const;
   void    PrintLinkStateRequestPacket          (const OspfLinkStateRequestPacket6* requestPacket, IPv6Address destination, int outputIfIndex) const;
   void    PrintLinkStateUpdatePacket           (const OspfLinkStateUpdatePacket6* updatePacket, IPv6Address destination, int outputIfIndex) const;
   void    PrintLinkStateAcknowledgementPacket  (const OspfLinkStateAcknowledgementPacket6* ackPacket, IPv6Address destination, int outputIfIndex) const;
};

}

#endif /* ANSAMESSAGEHANDLER6_H_ */
