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

#include "ansaOspfArea6.h"
#include "ansaOspfInterface6.h"
#include "ansaLsa6.h"
#include "ansaOspfNeighbor6.h"
#include "ansaOspfRouter6.h"

#include "ansaDatabaseDescriptionHandler6.h"

AnsaOspf6::DatabaseDescriptionHandler::DatabaseDescriptionHandler(AnsaOspf6::Router* containingRouter) :
      AnsaOspf6::IMessageHandler(containingRouter) {
}

void AnsaOspf6::DatabaseDescriptionHandler::ProcessPacket(OspfPacket6* packet, AnsaOspf6::Interface* intf, AnsaOspf6::Neighbor* neighbor) {

   router->GetMessageHandler()->PrintEvent("Database Description packet received", intf, neighbor);

   OspfDatabaseDescriptionPacket6* ddPacket = check_and_cast<OspfDatabaseDescriptionPacket6*> (packet);

   AnsaOspf6::Neighbor::NeighborStateType neighborState = neighbor->GetState();

   if ((ddPacket->getInterfaceMtu() <= intf->GetMtu()) && (neighborState > AnsaOspf6::Neighbor::AttemptState)){

      switch (neighborState){

         case AnsaOspf6::Neighbor::TwoWayState:
            break;

         case AnsaOspf6::Neighbor::InitState:
            neighbor->ProcessEvent(AnsaOspf6::Neighbor::TwoWayReceived);
            break;

         case AnsaOspf6::Neighbor::ExchangeStartState: {
            OspfDdOptions6& ddOptions = ddPacket->getDdOptions();

            if (  ddOptions.I_Init
               && ddOptions.M_More
               && ddOptions.MS_MasterSlave
               && (ddPacket->getLsaHeadersArraySize() == 0)){

               if (neighbor->GetNeighborID() > router->GetRouterID()){
                  AnsaOspf6::Neighbor::DDPacketID packetID;
                  packetID.ddOptions = ddOptions;
                  packetID.options = ddPacket->getOptions();
                  packetID.sequenceNumber = ddPacket->getDdSequenceNumber();

                  neighbor->SetOptions(packetID.options);
                  neighbor->SetDatabaseExchangeRelationship(AnsaOspf6::Neighbor::Slave);
                  neighbor->SetDDSequenceNumber(packetID.sequenceNumber);
                  neighbor->SetLastReceivedDDPacket(packetID);

                  if (!ProcessDDPacket(ddPacket, intf, neighbor, true)){
                     break;
                  }

                  neighbor->ProcessEvent(AnsaOspf6::Neighbor::NegotiationDone);
                  if (!neighbor->IsLinkStateRequestListEmpty()
                        && !neighbor->IsRequestRetransmissionTimerActive()){
                     neighbor->SendLinkStateRequestPacket();
                     neighbor->ClearRequestRetransmissionTimer();
                     neighbor->StartRequestRetransmissionTimer();
                  }
               }else{
                  neighbor->SendDatabaseDescriptionPacket(true);
               }
            }
            if (  !ddOptions.I_Init
               && !ddOptions.MS_MasterSlave
               && (ddPacket->getDdSequenceNumber() == neighbor->GetDDSequenceNumber())
               && (neighbor->GetNeighborID() < router->GetRouterID())){

               AnsaOspf6::Neighbor::DDPacketID packetID;
               packetID.ddOptions = ddOptions;
               packetID.options = ddPacket->getOptions();
               packetID.sequenceNumber = ddPacket->getDdSequenceNumber();

               neighbor->SetOptions(packetID.options);
               neighbor->SetDatabaseExchangeRelationship(AnsaOspf6::Neighbor::Master);
               neighbor->SetLastReceivedDDPacket(packetID);

               if (!ProcessDDPacket(ddPacket, intf, neighbor, true)){
                  break;
               }

               neighbor->ProcessEvent(AnsaOspf6::Neighbor::NegotiationDone);
               if (  !neighbor->IsLinkStateRequestListEmpty()
                  && !neighbor->IsRequestRetransmissionTimerActive()){

                  neighbor->SendLinkStateRequestPacket();
                  neighbor->ClearRequestRetransmissionTimer();
                  neighbor->StartRequestRetransmissionTimer();
               }
            }}
            break;

         case AnsaOspf6::Neighbor::ExchangeState: {
            AnsaOspf6::Neighbor::DDPacketID packetID;
            packetID.ddOptions = ddPacket->getDdOptions();
            packetID.options = ddPacket->getOptions();
            packetID.sequenceNumber = ddPacket->getDdSequenceNumber();

            if (packetID != neighbor->GetLastReceivedDDPacket()){
               if (  (packetID.ddOptions.MS_MasterSlave
                        && (neighbor->GetDatabaseExchangeRelationship() != AnsaOspf6::Neighbor::Slave))
                     || (!packetID.ddOptions.MS_MasterSlave
                        && (neighbor->GetDatabaseExchangeRelationship() != AnsaOspf6::Neighbor::Master))
                     || packetID.ddOptions.I_Init
                     || (packetID.options != neighbor->GetLastReceivedDDPacket().options)){

                  neighbor->ProcessEvent(AnsaOspf6::Neighbor::SequenceNumberMismatch);

               }else{
                  if (  ((neighbor->GetDatabaseExchangeRelationship() == AnsaOspf6::Neighbor::Master)
                           && (packetID.sequenceNumber == neighbor->GetDDSequenceNumber()))
                        || ((neighbor->GetDatabaseExchangeRelationship() == AnsaOspf6::Neighbor::Slave)
                              && (packetID.sequenceNumber == (neighbor->GetDDSequenceNumber() + 1)))){

                     neighbor->SetLastReceivedDDPacket(packetID);

                     if (!ProcessDDPacket(ddPacket, intf, neighbor, false)){
                        break;
                     }

                     if (  !neighbor->IsLinkStateRequestListEmpty()
                           && !neighbor->IsRequestRetransmissionTimerActive()){

                        neighbor->SendLinkStateRequestPacket();
                        neighbor->ClearRequestRetransmissionTimer();
                        neighbor->StartRequestRetransmissionTimer();
                     }

                  }else{
                     neighbor->ProcessEvent(AnsaOspf6::Neighbor::SequenceNumberMismatch);
                  }
               }
            }else{
               if (neighbor->GetDatabaseExchangeRelationship() == AnsaOspf6::Neighbor::Slave){
                  neighbor->RetransmitDatabaseDescriptionPacket();
               }
            }}
            break;


         case AnsaOspf6::Neighbor::LoadingState:
         case AnsaOspf6::Neighbor::FullState: {
            AnsaOspf6::Neighbor::DDPacketID packetID;
            packetID.ddOptions = ddPacket->getDdOptions();
            packetID.options = ddPacket->getOptions();
            packetID.sequenceNumber = ddPacket->getDdSequenceNumber();

            if (  (packetID != neighbor->GetLastReceivedDDPacket())
                  || (packetID.ddOptions.I_Init)){
               neighbor->ProcessEvent(AnsaOspf6::Neighbor::SequenceNumberMismatch);
            }else{
               if (neighbor->GetDatabaseExchangeRelationship() == AnsaOspf6::Neighbor::Slave){
                  if (!neighbor->RetransmitDatabaseDescriptionPacket()){
                     neighbor->ProcessEvent(AnsaOspf6::Neighbor::SequenceNumberMismatch);
                  }
               }
            }}
            break;
         default:
            break;
      }
   }
}

bool AnsaOspf6::DatabaseDescriptionHandler::ProcessDDPacket(
      OspfDatabaseDescriptionPacket6* ddPacket,
      AnsaOspf6::Interface* intf,
      AnsaOspf6::Neighbor* neighbor,
      bool inExchangeStart) {

   EV << "  Processing packet contents(ddOptions="
   << ((ddPacket->getDdOptions().I_Init) ? "I " : "_ ")
   << ((ddPacket->getDdOptions().M_More) ? "M " : "_ ")
   << ((ddPacket->getDdOptions().MS_MasterSlave) ? "MS" : "__")
   << "; seqNumber="
   << ddPacket->getDdSequenceNumber()
   << "):\n";

   unsigned int headerCount = ddPacket->getLsaHeadersArraySize();

   for (unsigned int i = 0; i < headerCount; i++){
      OspfLsaHeader6& currentHeader = ddPacket->getLsaHeaders(i);
      LsaType6 lsaType = static_cast<LsaType6> (currentHeader.getLsType());

      EV << "    ";
      PrintLsaHeader6(currentHeader, ev.getOStream());

      if ((lsaType < RouterLsaType) || (lsaType > AsExternalLsaType)
            || ((lsaType == AsExternalLsaType) && (!intf->GetArea()->GetExternalRoutingCapability()))){

         EV << " Error!\n";
         neighbor->ProcessEvent(AnsaOspf6::Neighbor::SequenceNumberMismatch);
         return false;
      }else{
         AnsaOspf6::LsaKeyType6 lsaKey;

         lsaKey.linkStateID = currentHeader.getLinkStateID();
         lsaKey.advertisingRouter = currentHeader.getAdvertisingRouter();

         OspfLsa6* lsaInDatabase = router->FindLSA(lsaType, lsaKey, intf->GetArea()->GetAreaID());

         // operator< and operator== on OSPFLSAHeaders determines which one is newer(less means older)
         if ((lsaInDatabase == NULL) || (lsaInDatabase->getHeader() < currentHeader)){
            EV << " (newer)";
            neighbor->AddToRequestList(&currentHeader);
         }
      }
      EV << "\n";
   }

   if (neighbor->GetDatabaseExchangeRelationship() == AnsaOspf6::Neighbor::Master){
      neighbor->IncrementDDSequenceNumber();
      if ((neighbor->GetDatabaseSummaryListCount() == 0) && !ddPacket->getDdOptions().M_More){
         neighbor->ProcessEvent(AnsaOspf6::Neighbor::ExchangeDone);  // does nothing in ExchangeStart
      }else{
         if (!inExchangeStart){
            neighbor->SendDatabaseDescriptionPacket();
         }
      }
   }else{
      neighbor->SetDDSequenceNumber(ddPacket->getDdSequenceNumber());
      if (!inExchangeStart) {
         neighbor->SendDatabaseDescriptionPacket();
      }
      if (!ddPacket->getDdOptions().M_More && (neighbor->GetDatabaseSummaryListCount() == 0)){
         neighbor->ProcessEvent(AnsaOspf6::Neighbor::ExchangeDone);  // does nothing in ExchangeStart
      }
   }
   return true;
}
