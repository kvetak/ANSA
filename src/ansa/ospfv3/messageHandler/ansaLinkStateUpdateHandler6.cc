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
#include "ansaOspfCommon6.h"
#include "ansaOspfNeighbor6.h"
#include "ansaOspfRouter6.h"

#include "ansaLinkStateUpdateHandler6.h"

class LsaProcessingMarker {
   private:
      unsigned int index;

   public:
      LsaProcessingMarker(unsigned int counter) : index(counter) { EV<< "    --> Processing LSA(" << index << ")\n"; }
      ~LsaProcessingMarker() { EV << "    <-- LSA(" << index << ") processed.\n"; }
   };

AnsaOspf6::LinkStateUpdateHandler::LinkStateUpdateHandler(AnsaOspf6::Router* containingRouter) :
   AnsaOspf6::IMessageHandler(containingRouter) {
}

void AnsaOspf6::LinkStateUpdateHandler::ProcessPacket(OspfPacket6* packet,
      AnsaOspf6::Interface* intf, AnsaOspf6::Neighbor* neighbor) {

   router->GetMessageHandler()->PrintEvent("Link State Update packet received", intf, neighbor);

   OspfLinkStateUpdatePacket6* lsUpdatePacket = check_and_cast<OspfLinkStateUpdatePacket6*> (packet);
   bool rebuildRoutingTable = false;

   if (neighbor->GetState() >= AnsaOspf6::Neighbor::ExchangeState){
      AnsaOspf6::AreaID areaID = lsUpdatePacket->getAreaID();
      AnsaOspf6::Area* area = router->GetArea(areaID);
      LsaType6 currentType = RouterLsaType;
      unsigned int currentLSAIndex = 0;

      EV<< "  Processing packet contents:\n";

      while (currentType <= IntraAreaPrefixLsaType){
         unsigned int lsaCount = 0;

         switch (currentType){
            case RouterLsaType:
               lsaCount = lsUpdatePacket->getRouterLsasArraySize();
               break;
            case NetworkLsaType:
               lsaCount = lsUpdatePacket->getNetworkLsasArraySize();
               break;
            case InterAreaPrefixLsaType:
               lsaCount = lsUpdatePacket->getInterAreaPrefixLsasArraySize();
               break;
            case InterAreaRouterLsaType:
               lsaCount = lsUpdatePacket->getInterAreaRouterLsasArraySize();
               break;
            case AsExternalLsaType:
               lsaCount = lsUpdatePacket->getAsExternalLsasArraySize();
               break;
            case LinkLsaType:
               lsaCount = lsUpdatePacket->getLinkLsasArraySize();
               break;
            case IntraAreaPrefixLsaType:
               lsaCount = lsUpdatePacket->getIntraAreaPrefixLsasArraySize();
               break;
            default:
               break;
         }

         for (unsigned int i = 0; i < lsaCount; i++){
            OspfLsa6* currentLSA;

            switch (currentType){
               case RouterLsaType:
                  currentLSA = (&(lsUpdatePacket->getRouterLsas(i)));
                  break;
               case NetworkLsaType:
                  currentLSA = (&(lsUpdatePacket->getNetworkLsas(i)));
                  break;
               case InterAreaPrefixLsaType:
                  currentLSA = (&(lsUpdatePacket->getInterAreaPrefixLsas(i)));
                  break;
               case InterAreaRouterLsaType:
                  currentLSA = (&(lsUpdatePacket->getInterAreaRouterLsas(i)));
                  break;
               case AsExternalLsaType:
                  currentLSA = (&(lsUpdatePacket->getAsExternalLsas(i)));
                  break;
               case LinkLsaType:
                  currentLSA = (&(lsUpdatePacket->getLinkLsas(i)));
                  break;
               case IntraAreaPrefixLsaType:
                  currentLSA = (&(lsUpdatePacket->getIntraAreaPrefixLsas(i)));
                  break;
               default:
                  break;
            }

            LsaType6 lsaType = static_cast<LsaType6> (currentLSA->getHeader().getLsType());
            if (  (lsaType != RouterLsaType) &&
                  (lsaType != NetworkLsaType) &&
                  (lsaType != InterAreaPrefixLsaType) &&
                  (lsaType != InterAreaRouterLsaType) &&
                  (lsaType != AsExternalLsaType) &&
                  (lsaType != LinkLsaType) &&
                  (lsaType != IntraAreaPrefixLsaType)){
               continue;
            }

            LsaProcessingMarker marker(currentLSAIndex++);
            EV << "    ";
            PrintLsaHeader6(currentLSA->getHeader(), ev.getOStream());
            EV << "\n";

            if ((lsaType == AsExternalLsaType) && (!area->GetExternalRoutingCapability())){
               continue;
            }
            AnsaOspf6::LsaKeyType6 lsaKey;

            lsaKey.linkStateID = currentLSA->getHeader().getLinkStateID();
            lsaKey.advertisingRouter = currentLSA->getHeader().getAdvertisingRouter();

            OspfLsa6* lsaInDatabase = router->FindLSA(lsaType, lsaKey, areaID);
            unsigned short lsAge = currentLSA->getHeader().getLsAge();
            AcknowledgementFlags ackFlags;

            ackFlags.floodedBackOut = false;
            ackFlags.lsaIsNewer = false;
            ackFlags.lsaIsDuplicate = false;
            ackFlags.impliedAcknowledgement = false;
            ackFlags.lsaReachedMaxAge = (lsAge == MAX_AGE);
            ackFlags.noLSAInstanceInDatabase = (lsaInDatabase == NULL);
            ackFlags.anyNeighborInExchangeOrLoadingState = router->HasAnyNeighborInStates(AnsaOspf6::Neighbor::ExchangeState | AnsaOspf6::Neighbor::LoadingState);


            if ((ackFlags.lsaReachedMaxAge) && (ackFlags.noLSAInstanceInDatabase) && (!ackFlags.anyNeighborInExchangeOrLoadingState)){
               if (intf->GetType() == AnsaOspf6::Interface::Broadcast){
                  if (  (intf->GetState() == AnsaOspf6::Interface::DesignatedRouterState) ||
                        (intf->GetState() == AnsaOspf6::Interface::BackupState) ||
                        (intf->GetDesignatedRouter() == AnsaOspf6::NullDesignatedRouterID)){
                           intf->SendLSAcknowledgement(&(currentLSA->getHeader()), AnsaOspf6::AllSPFRouters);
                        }else{
                           intf->SendLSAcknowledgement(&(currentLSA->getHeader()), AnsaOspf6::AllDRouters);
                        }
               }else{
                  if (intf->GetType() == AnsaOspf6::Interface::PointToPoint){
                     intf->SendLSAcknowledgement(&(currentLSA->getHeader()), AnsaOspf6::AllSPFRouters);
                  }else{
                     intf->SendLSAcknowledgement(&(currentLSA->getHeader()), neighbor->GetAddress());
                  }
               }
               continue;
            }

            if (!ackFlags.noLSAInstanceInDatabase){
               // operator< and operator== on OSPFLSAHeaders determines which one is newer(less means older)
               ackFlags.lsaIsNewer = (lsaInDatabase->getHeader() < currentLSA->getHeader());
               ackFlags.lsaIsDuplicate = (operator== (lsaInDatabase->getHeader(), currentLSA->getHeader()));
            }
            if ((ackFlags.noLSAInstanceInDatabase) || (ackFlags.lsaIsNewer)){
               LsaTrackingInfo* info = (!ackFlags.noLSAInstanceInDatabase) ? dynamic_cast<LsaTrackingInfo*> (lsaInDatabase) : NULL;
               if (  (!ackFlags.noLSAInstanceInDatabase) &&
                     (info != NULL) &&
                     (info->GetSource() == LsaTrackingInfo::Flooded) &&
                     (info->GetInstallTime() < MIN_LS_ARRIVAL)){
                  //continue; TODO
               }
               ackFlags.floodedBackOut = router->FloodLSA(currentLSA, areaID, intf, neighbor);
               if (!ackFlags.noLSAInstanceInDatabase){
                  AnsaOspf6::LsaKeyType6 lsaKey;

                  lsaKey.linkStateID = lsaInDatabase->getHeader().getLinkStateID();
                  lsaKey.advertisingRouter = lsaInDatabase->getHeader().getAdvertisingRouter();

                  router->RemoveFromAllRetransmissionLists(lsaKey);
               }
               rebuildRoutingTable |= router->InstallLSA(currentLSA, areaID);

               EV << "    (update installed)\n";

               AcknowledgeLSA(currentLSA->getHeader(), intf, ackFlags, lsUpdatePacket->getRouterID());

               // TODO: verify
               if (currentLSA->getHeader().getAdvertisingRouter() == router->GetRouterID()){

                  if (ackFlags.noLSAInstanceInDatabase){
                     currentLSA->getHeader().setLsAge(MAX_AGE);
                     router->FloodLSA(currentLSA, areaID);
                  }else{
                     if (ackFlags.lsaIsNewer){
                        long sequenceNumber = currentLSA->getHeader().getLsSequenceNumber();
                        if (sequenceNumber == MAX_SEQUENCE_NUMBER){
                           lsaInDatabase->getHeader().setLsAge(MAX_AGE);
                           router->FloodLSA(lsaInDatabase, areaID);
                        }else{
                           lsaInDatabase->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                           router->FloodLSA(lsaInDatabase, areaID);
                        }
                     }
                  }
               }
               continue;
            }


            if (neighbor->IsLSAOnRequestList(lsaKey)){
               neighbor->ProcessEvent(AnsaOspf6::Neighbor::BadLinkStateRequest);
               break;
            }

            if (ackFlags.lsaIsDuplicate){
               if (neighbor->IsLSAOnRetransmissionList(lsaKey)){
                  neighbor->RemoveFromRetransmissionList(lsaKey);
                  ackFlags.impliedAcknowledgement = true;
               }
               AcknowledgeLSA(currentLSA->getHeader(), intf, ackFlags, lsUpdatePacket->getRouterID());
               continue;
            }

            if (  (lsaInDatabase->getHeader().getLsAge() == MAX_AGE) &&
                  (lsaInDatabase->getHeader().getLsSequenceNumber() == MAX_SEQUENCE_NUMBER)){
               continue;
            }

            if (!neighbor->IsOnTransmittedLSAList(lsaKey)){
               OspfLinkStateUpdatePacket6* updatePacket = intf->CreateUpdatePacket(lsaInDatabase);
               if (updatePacket != NULL){
                  int ttl = (intf->GetType() == AnsaOspf6::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;

                  if (intf->GetType() == AnsaOspf6::Interface::Broadcast){
                     if (  (intf->GetState() == AnsaOspf6::Interface::DesignatedRouterState) ||
                           (intf->GetState() == AnsaOspf6::Interface::BackupState) ||
                           (intf->GetDesignatedRouter() == AnsaOspf6::NullDesignatedRouterID)){
                        router->GetMessageHandler()->SendPacket(updatePacket, AnsaOspf6::AllSPFRouters, intf->GetIfIndex(), ttl);
                     }else{
                        router->GetMessageHandler()->SendPacket(updatePacket, AnsaOspf6::AllDRouters, intf->GetIfIndex(), ttl);
                     }
                  }else{
                     if (intf->GetType() == AnsaOspf6::Interface::PointToPoint){
                        router->GetMessageHandler()->SendPacket(updatePacket, AnsaOspf6::AllSPFRouters, intf->GetIfIndex(), ttl);
                     }else{
                        router->GetMessageHandler()->SendPacket(updatePacket, neighbor->GetAddress(), intf->GetIfIndex(), ttl);
                     }
                  }
               }
            }

         } // for each LSA of a type

         currentType = static_cast<LsaType6> (currentType + 1);

      } // for each LSA type
   }

   if (rebuildRoutingTable){
      router->RebuildRoutingTable();
   }
}

void AnsaOspf6::LinkStateUpdateHandler::AcknowledgeLSA(OspfLsaHeader6& lsaHeader,
      AnsaOspf6::Interface* intf,
      AnsaOspf6::LinkStateUpdateHandler::AcknowledgementFlags acknowledgementFlags,
      AnsaOspf6::RouterID lsaSource){

   bool sendDirectAcknowledgment = false;

   if (!acknowledgementFlags.floodedBackOut){
      if (intf->GetState() == AnsaOspf6::Interface::BackupState){
         if ((acknowledgementFlags.lsaIsNewer
               && (lsaSource == intf->GetDesignatedRouter()))
               || (acknowledgementFlags.lsaIsDuplicate
                     && acknowledgementFlags.impliedAcknowledgement)){
            intf->AddDelayedAcknowledgement(lsaHeader);
         }else{
            if ((acknowledgementFlags.lsaIsDuplicate
                  && !acknowledgementFlags.impliedAcknowledgement)
                  || (acknowledgementFlags.lsaReachedMaxAge
                        && acknowledgementFlags.noLSAInstanceInDatabase
                        && acknowledgementFlags.anyNeighborInExchangeOrLoadingState)){
               sendDirectAcknowledgment = true;
            }
         }
      }else{
         if (acknowledgementFlags.lsaIsNewer){
            intf->AddDelayedAcknowledgement(lsaHeader);
         }else{
            if ((acknowledgementFlags.lsaIsDuplicate
                  && !acknowledgementFlags.impliedAcknowledgement)
                  || (acknowledgementFlags.lsaReachedMaxAge
                        && acknowledgementFlags.noLSAInstanceInDatabase
                        && acknowledgementFlags.anyNeighborInExchangeOrLoadingState)){
               sendDirectAcknowledgment = true;
            }
         }
      }
   }

   if (sendDirectAcknowledgment){
      OspfLinkStateAcknowledgementPacket6* ackPacket = new OspfLinkStateAcknowledgementPacket6;

      ackPacket->setType(LinkStateAcknowledgementPacket);
      ackPacket->setRouterID(router->GetRouterID());
      ackPacket->setAreaID(intf->GetArea()->GetAreaID());

      ackPacket->setLsaHeadersArraySize(1);
      ackPacket->setLsaHeaders(0, lsaHeader);

      int ttl = (intf->GetType() == AnsaOspf6::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;

      if (intf->GetType() == AnsaOspf6::Interface::Broadcast){
         if ((intf->GetState() == AnsaOspf6::Interface::DesignatedRouterState) || (intf->GetState()
               == AnsaOspf6::Interface::BackupState) || (intf->GetDesignatedRouter()
               == AnsaOspf6::NullDesignatedRouterID)){
            router->GetMessageHandler()->SendPacket(ackPacket, AnsaOspf6::AllSPFRouters, intf->GetIfIndex(), ttl);
         }else{
            router->GetMessageHandler()->SendPacket(ackPacket, AnsaOspf6::AllDRouters, intf->GetIfIndex(), ttl);
         }
      }else{
         if (intf->GetType() == AnsaOspf6::Interface::PointToPoint){
            router->GetMessageHandler()->SendPacket(ackPacket, AnsaOspf6::AllSPFRouters, intf->GetIfIndex(), ttl);
         }else{
            router->GetMessageHandler()->SendPacket(ackPacket, intf->GetNeighborByID(lsaSource)->GetAddress(), intf->GetIfIndex(), ttl);
         }
      }
   }
}
