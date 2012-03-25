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

#include "ansaOspfRouter6.h"

#include "ansaLinkStateAcknowledgementHandler6.h"

AnsaOspf6::LinkStateAcknowledgementHandler::LinkStateAcknowledgementHandler(AnsaOspf6::Router* containingRouter) :
      AnsaOspf6::IMessageHandler(containingRouter) {
}

void AnsaOspf6::LinkStateAcknowledgementHandler::ProcessPacket(OspfPacket6* packet, AnsaOspf6::Interface* intf, AnsaOspf6::Neighbor* neighbor){
   router->GetMessageHandler()->PrintEvent("Link State Acknowledgement packet received", intf, neighbor);

   if (neighbor->GetState() >= AnsaOspf6::Neighbor::ExchangeState){

      OspfLinkStateAcknowledgementPacket6* lsAckPacket = check_and_cast<OspfLinkStateAcknowledgementPacket6*> (packet);
      int lsaCount = lsAckPacket->getLsaHeadersArraySize();

      EV<< "  Processing packet contents:\n";

      for (int i = 0; i < lsaCount; i++){
         OspfLsaHeader6& lsaHeader = lsAckPacket->getLsaHeaders(i);
         OspfLsa6* lsaOnRetransmissionList;
         AnsaOspf6::LsaKeyType6 lsaKey;

         EV << "    ";
         PrintLsaHeader6(lsaHeader, ev.getOStream());
         EV << "\n";

         lsaKey.linkStateID = lsaHeader.getLinkStateID();
         lsaKey.advertisingRouter = lsaHeader.getAdvertisingRouter();

         if ((lsaOnRetransmissionList = neighbor->FindOnRetransmissionList(lsaKey)) != NULL){
               if (operator== (lsaHeader, lsaOnRetransmissionList->getHeader())) {
                   neighbor->RemoveFromRetransmissionList(lsaKey);
               } else {
                   EV << "Got an Acknowledgement packet for an unsent Update packet.\n";
               }
           }
       }
       if (neighbor->IsLinkStateRetransmissionListEmpty()) {
           neighbor->ClearUpdateRetransmissionTimer();
       }
   }
}

