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

#include "ansaMessageHandler6.h"
#include "ansaOspfArea6.h"
#include "ansaOspfInterface6.h"
#include "ansaOspfInterfaceStateDown6.h"
#include "ansaOspfRouter6.h"

#include "InterfaceTableAccess.h"
#include "IPv6InterfaceData.h"

AnsaOspf6::Interface::Interface(AnsaOspf6::Interface::OspfInterfaceType ifType) :
      interfaceType(ifType),
      ifIndex(0),
      instanceID(0),
      mtu(0),
      areaID(AnsaOspf6::BackboneAreaID),
      transitAreaID(AnsaOspf6::BackboneAreaID),
      helloInterval(10),
      pollInterval(120),
      routerDeadInterval(40),
      interfaceTransmissionDelay(1),
      routerPriority(1),
      designatedRouter(AnsaOspf6::NullDesignatedRouterID),
      backupDesignatedRouter(AnsaOspf6::NullDesignatedRouterID),
      interfaceOutputCost(10),
      retransmissionInterval(5),
      acknowledgementDelay(1),
      parentArea(NULL),
      isGoingDown(false){

   state = new AnsaOspf6::InterfaceStateDown;
   previousState = NULL;
   helloTimer = new OspfTimer6;
   helloTimer->setTimerKind(InterfaceHelloTimer);
   helloTimer->setContextPointer(this);
   helloTimer->setName("AnsaOspf6::Interface::InterfaceHelloTimer");
   waitTimer = new OspfTimer6;
   waitTimer->setTimerKind(InterfaceWaitTimer);
   waitTimer->setContextPointer(this);
   waitTimer->setName("AnsaOspf6::Interface::InterfaceWaitTimer");
   /*
   acknowledgementTimer = new OspfTimer6;
   acknowledgementTimer->setTimerKind(InterfaceAcknowledgementTimer);
   acknowledgementTimer->setContextPointer(this);
   acknowledgementTimer->setName("AnsaOspf6::Interface::InterfaceAcknowledgementTimer");
   */
}


AnsaOspf6::Interface::~Interface(void){
   MessageHandler* messageHandler = parentArea->GetRouter()->GetMessageHandler();

   if (helloTimer != NULL){
      messageHandler->ClearTimer(helloTimer);
      delete helloTimer;
   }

   if (waitTimer != NULL){
      messageHandler->ClearTimer(waitTimer);
      delete waitTimer;
   }
   /* TODO
   messageHandler->ClearTimer(acknowledgementTimer);
   delete acknowledgementTimer;
   */
   if (previousState != NULL){
      delete previousState;
   }
   delete state;

   long neighborCount = neighboringRouters.size();
   for (long i = 0; i < neighborCount; i++){
      delete neighboringRouters[i];
   }
}


void AnsaOspf6::Interface::SetIfIndex(IfaceID id){
   ifIndex = id;
   if (interfaceType == AnsaOspf6::Interface::UnknownType) {
      InterfaceEntry* routingInterface = InterfaceTableAccess().get()->getInterfaceById(ifIndex);
      interfaceAddress = routingInterface->ipv6Data()->getLinkLocalAddress();
      mtu = routingInterface->getMTU();
   }
}


void AnsaOspf6::Interface::ChangeState(AnsaOspf6::InterfaceState* newState, AnsaOspf6::InterfaceState* currentState){
   if (previousState != NULL) {
      delete previousState;
   }
   state = newState;
   previousState = currentState;
}


void AnsaOspf6::Interface::ProcessEvent(AnsaOspf6::Interface::InterfaceEventType event){
   state->ProcessEvent(this, event);
}


void AnsaOspf6::Interface::Reset(void){
   MessageHandler* messageHandler = parentArea->GetRouter()->GetMessageHandler();

   messageHandler->ClearTimer(helloTimer);
   messageHandler->ClearTimer(waitTimer);
   messageHandler->ClearTimer(acknowledgementTimer);

   designatedRouter = NullDesignatedRouterID;
   backupDesignatedRouter = NullDesignatedRouterID;
   long neighborCount = neighboringRouters.size();
   for (long i = 0; i < neighborCount; i++) {
      neighboringRouters[i]->ProcessEvent(AnsaOspf6::Neighbor::KillNeighbor);
   }
}

// http://tools.ietf.org/html/rfc5340#section-4.2.1.1
void AnsaOspf6::Interface::SendHelloPacket(IPv6Address destination, short ttl){

   OspfOptions6 options;
   OspfHelloPacket6* helloPacket = new OspfHelloPacket6;
   std::vector<RouterID> neighbors;

   helloPacket->setType(HelloPacket);
   helloPacket->setRouterID(parentArea->GetRouter()->GetRouterID());
   helloPacket->setAreaID(parentArea->GetAreaID());

   helloPacket->setInterfaceID(ifIndex);
   helloPacket->setInstanceID(instanceID);
   helloPacket->setRtrPriority(routerPriority);

   memset(&options, 0, sizeof(OspfOptions6));
   options.E_ExternalRoutingCapability = parentArea->GetExternalRoutingCapability();
   helloPacket->setOptions(options);

   helloPacket->setHelloInterval(helloInterval);
   helloPacket->setRouterDeadInterval(routerDeadInterval);
   helloPacket->setDesignatedRouterID(designatedRouter);
   helloPacket->setBackupDesignatedRouterID(backupDesignatedRouter);

   long neighborCount = neighboringRouters.size();
   for (long j = 0; j < neighborCount; j++) {
      if (neighboringRouters[j]->GetState() >= AnsaOspf6::Neighbor::InitState) {
         neighbors.push_back(neighboringRouters[j]->GetNeighborID());
      }
   }

   unsigned int initedNeighborCount = neighbors.size();
   helloPacket->setNeighborIDArraySize(initedNeighborCount);
   for (unsigned int k = 0; k < initedNeighborCount; k++){
      helloPacket->setNeighborID(k, neighbors[k]);
   }

   parentArea->GetRouter()->GetMessageHandler()->SendPacket(helloPacket, destination, ifIndex, ttl);
}

void AnsaOspf6::Interface::SendLSAcknowledgement(OspfLsaHeader6* lsaHeader, IPv6Address destination) {
   OspfOptions6 options;
   OspfLinkStateAcknowledgementPacket6* lsAckPacket = new OspfLinkStateAcknowledgementPacket6;

   lsAckPacket->setType(LinkStateAcknowledgementPacket);
   lsAckPacket->setRouterID(parentArea->GetRouter()->GetRouterID());
   lsAckPacket->setAreaID(parentArea->GetAreaID());
   lsAckPacket->setInstanceID(instanceID);

   lsAckPacket->setLsaHeadersArraySize(1);
   lsAckPacket->setLsaHeaders(0, *lsaHeader);

   int ttl = (interfaceType == AnsaOspf6::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;
   parentArea->GetRouter()->GetMessageHandler()->SendPacket(lsAckPacket, destination, ifIndex, ttl);
}

AnsaOspf6::Neighbor* AnsaOspf6::Interface::GetNeighborByID(AnsaOspf6::RouterID neighborID) {
   std::map<AnsaOspf6::RouterID, AnsaOspf6::Neighbor*>::iterator neighborIt =
         neighboringRoutersByID.find(neighborID);
   if (neighborIt != neighboringRoutersByID.end()){
      return (neighborIt->second);
   }else{
      return NULL;
   }
}

AnsaOspf6::Neighbor* AnsaOspf6::Interface::GetNeighborByAddress(IPv6Address address) {
   std::map<IPv6Address, AnsaOspf6::Neighbor*, AnsaOspf6::IPv6Address_Less>::iterator neighborIt = neighboringRoutersByAddress.find(address);
   if (neighborIt != neighboringRoutersByAddress.end()){
      return (neighborIt->second);
   }else{
      return NULL;
   }
}

void AnsaOspf6::Interface::AddNeighbor(AnsaOspf6::Neighbor* neighbor) {
   neighboringRoutersByID[neighbor->GetNeighborID()] = neighbor;
   neighboringRoutersByAddress[neighbor->GetAddress()] = neighbor;
   neighbor->SetInterface(this);
   neighboringRouters.push_back(neighbor);
}

AnsaOspf6::Interface::InterfaceStateType AnsaOspf6::Interface::GetState(void) const {
   return state->GetState();
}

const char* AnsaOspf6::Interface::GetStateString(AnsaOspf6::Interface::InterfaceStateType stateType) {
   switch (stateType){
      case DownState:
         return "Down";
      case LoopbackState:
         return "Loopback";
      case WaitingState:
         return "Waiting";
      case PointToPointState:
         return "PointToPoint";
      case NotDesignatedRouterState:
         return "NotDesignatedRouter";
      case BackupState:
         return "Backup";
      case DesignatedRouterState:
         return "DesignatedRouter";
      default:
         ASSERT(false);
   }
   return "";
}

const char* AnsaOspf6::Interface::GetTypeString (OspfInterfaceType type){
   switch (type){
      case UnknownType:
         return "Unknown";
      case PointToPoint:
         return "Point-to-Point";
      case Broadcast:
         return "Broadcast";
      case NBMA:
         return "NBMA";
      case PointToMultiPoint:
         return "Point-to-Multipoint";
      case Virtual:
         return "Virtual";
      default:
         ASSERT(false);
   }
   return "";
}

bool AnsaOspf6::Interface::HasAnyNeighborInStates(int states) const {
   long neighborCount = neighboringRouters.size();
   for (long i = 0; i < neighborCount; i++){
      AnsaOspf6::Neighbor::NeighborStateType neighborState = neighboringRouters[i]->GetState();
      if (neighborState & states){
         return true;
      }
   }
   return false;
}

void AnsaOspf6::Interface::RemoveFromAllRetransmissionLists(AnsaOspf6::LsaKeyType6 lsaKey) {
   long neighborCount = neighboringRouters.size();
   for (long i = 0; i < neighborCount; i++){
      neighboringRouters[i]->RemoveFromRetransmissionList(lsaKey);
   }
}

bool AnsaOspf6::Interface::IsOnAnyRetransmissionList(AnsaOspf6::LsaKeyType6 lsaKey) const {
   long neighborCount = neighboringRouters.size();
   for (long i = 0; i < neighborCount; i++){
      if (neighboringRouters[i]->IsLSAOnRetransmissionList(lsaKey)){
         return true;
      }
   }
   return false;
}

bool AnsaOspf6::Interface::FloodLSA(OspfLsa6* lsa, AnsaOspf6::Interface* intf, AnsaOspf6::Neighbor* neighbor){
   bool floodedBackOut = false;

   if (  (  (lsa->getHeader().getLsType() == AsExternalLsaType)
         && (interfaceType != AnsaOspf6::Interface::Virtual)
         && (parentArea->GetExternalRoutingCapability()))
      || (  (lsa->getHeader().getLsType() != AsExternalLsaType)
         && (  (  (areaID != AnsaOspf6::BackboneAreaID)
               && (interfaceType != AnsaOspf6::Interface::Virtual))
         || (areaID == AnsaOspf6::BackboneAreaID)))){

      long neighborCount = neighboringRouters.size();
      bool lsaAddedToRetransmissionList = false;
      AnsaOspf6::LinkStateID linkStateID = lsa->getHeader().getLinkStateID();
      AnsaOspf6::LsaKeyType6 lsaKey;

      lsaKey.linkStateID = linkStateID;
      lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

      for (long i = 0; i < neighborCount; i++){ // (1)

         if (neighboringRouters[i]->GetState() < AnsaOspf6::Neighbor::ExchangeState){ // (1) (a)
            continue;
         }

         if (neighboringRouters[i]->GetState() < AnsaOspf6::Neighbor::FullState){ // (1) (b)
            OspfLsaHeader6* requestLSAHeader = neighboringRouters[i]->FindOnRequestList(lsaKey);
            if (requestLSAHeader != NULL){
               // operator< and operator== on OSPFLSAHeaders determines which one is newer(less means older)
               if (lsa->getHeader() < (*requestLSAHeader)){
                  continue;
               }
               if (operator==(lsa->getHeader(), (*requestLSAHeader))){
                  neighboringRouters[i]->RemoveFromRequestList(lsaKey);
                  continue;
               }
               neighboringRouters[i]->RemoveFromRequestList(lsaKey);
            }
         }

         if (neighbor == neighboringRouters[i]){ // (1) (c)
            continue;
         }

         neighboringRouters[i]->AddToRetransmissionList(lsa); // (1) (d)
         lsaAddedToRetransmissionList = true;
      }


      if (lsaAddedToRetransmissionList){ // (2)
         if (  (intf != this)
            || (  (neighbor != NULL)
               && (neighbor->GetNeighborID() != designatedRouter)
               && (neighbor->GetNeighborID() != backupDesignatedRouter))){ // (3)

            if ((intf != this) || (GetState() != AnsaOspf6::Interface::BackupState)){ // (4)
               OspfLinkStateUpdatePacket6* updatePacket = CreateUpdatePacket(lsa); // (5)

               if (updatePacket != NULL){
                  int ttl = (interfaceType == AnsaOspf6::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;
                  AnsaOspf6::MessageHandler* messageHandler = parentArea->GetRouter()->GetMessageHandler();

                  if (interfaceType == AnsaOspf6::Interface::Broadcast){
                     if (  (GetState() == AnsaOspf6::Interface::DesignatedRouterState)
                        || (GetState() == AnsaOspf6::Interface::BackupState)
                        || (designatedRouter == AnsaOspf6::NullDesignatedRouterID)){
                        messageHandler->SendPacket(updatePacket, AnsaOspf6::AllSPFRouters, ifIndex, ttl);
                        for (long k = 0; k < neighborCount; k++){
                           neighboringRouters[k]->AddToTransmittedLSAList(lsaKey);
                           if (!neighboringRouters[k]->IsUpdateRetransmissionTimerActive()){
                              neighboringRouters[k]->StartUpdateRetransmissionTimer();
                           }
                        }
                     }else{
                        messageHandler->SendPacket(updatePacket, AnsaOspf6::AllDRouters, ifIndex, ttl);
                        AnsaOspf6::Neighbor* dRouter = GetNeighborByID(designatedRouter);
                        AnsaOspf6::Neighbor* backupDRouter = GetNeighborByID(backupDesignatedRouter);
                        if (dRouter != NULL){
                           dRouter->AddToTransmittedLSAList(lsaKey);
                           if (!dRouter->IsUpdateRetransmissionTimerActive()){
                              dRouter->StartUpdateRetransmissionTimer();
                           }
                        }
                        if (backupDRouter != NULL){
                           backupDRouter->AddToTransmittedLSAList(lsaKey);
                           if (!backupDRouter->IsUpdateRetransmissionTimerActive()){
                              backupDRouter->StartUpdateRetransmissionTimer();
                           }
                        }
                     }

                  }else{
                     if (interfaceType == AnsaOspf6::Interface::PointToPoint){
                        messageHandler->SendPacket(updatePacket, AnsaOspf6::AllSPFRouters, ifIndex, ttl);
                        if (neighborCount > 0){
                           neighboringRouters[0]->AddToTransmittedLSAList(lsaKey);
                           if (!neighboringRouters[0]->IsUpdateRetransmissionTimerActive()){
                              neighboringRouters[0]->StartUpdateRetransmissionTimer();
                           }
                        }
                     }else{
                        for (long m = 0; m < neighborCount; m++){
                           if (neighboringRouters[m]->GetState() >= AnsaOspf6::Neighbor::ExchangeState){
                              messageHandler->SendPacket(updatePacket, neighboringRouters[m]->GetAddress(), ifIndex, ttl);
                              neighboringRouters[m]->AddToTransmittedLSAList(lsaKey);
                              if (!neighboringRouters[m]->IsUpdateRetransmissionTimerActive()){
                                 neighboringRouters[m]->StartUpdateRetransmissionTimer();
                              }
                           }
                        }
                     }
                  }

                  if (intf == this){
                     floodedBackOut = true;
                  }
               }
            }
         }
      }
   }

   return floodedBackOut;
}

OspfLinkStateUpdatePacket6* AnsaOspf6::Interface::CreateUpdatePacket(OspfLsa6* lsa){
   LsaType6 lsaType = static_cast<LsaType6> (lsa->getHeader().getLsType());
   OspfRouterLsa6* routerLSA = (lsaType == RouterLsaType) ? dynamic_cast<OspfRouterLsa6*> (lsa) : NULL;
   OspfNetworkLsa6* networkLSA = (lsaType == NetworkLsaType) ? dynamic_cast<OspfNetworkLsa6*> (lsa) : NULL;
   OspfInterAreaPrefixLsa6* interAreaPrefixLSA = (lsaType = InterAreaPrefixLsaType) ? dynamic_cast<OspfInterAreaPrefixLsa6*> (lsa) : NULL;
   OspfInterAreaRouterLsa6* interAreaRouterLSA = (lsaType = InterAreaRouterLsaType) ? dynamic_cast<OspfInterAreaRouterLsa6*> (lsa) : NULL;
   OspfAsExternalLsa6* asExternalLSA = (lsaType == AsExternalLsaType) ? dynamic_cast<OspfAsExternalLsa6*> (lsa) : NULL;
   OspfLinkLsa6* linkLSA = (lsaType == LinkLsaType) ?  dynamic_cast<OspfLinkLsa6*> (lsa) : NULL;
   OspfIntraAreaPrefixLsa6* intraAreaPrefixLSA = (lsaType = IntraAreaPrefixLsaType) ?  dynamic_cast<OspfIntraAreaPrefixLsa6*> (lsa) : NULL;

   if (  ((lsaType == RouterLsaType) && (routerLSA != NULL))
      || ((lsaType == NetworkLsaType) && (networkLSA != NULL))
      || ((lsaType == InterAreaPrefixLsaType) && (interAreaPrefixLSA != NULL))
      || ((lsaType == InterAreaRouterLsaType) && (interAreaRouterLSA != NULL))
      || ((lsaType == AsExternalLsaType) && (asExternalLSA != NULL))
      || ((lsaType == LinkLsaType) && (linkLSA != NULL))
      || ((lsaType == IntraAreaPrefixLsaType) && (intraAreaPrefixLSA != NULL))){

      OspfLinkStateUpdatePacket6* updatePacket = new OspfLinkStateUpdatePacket6;

      updatePacket->setType(LinkStateUpdatePacket);
      updatePacket->setRouterID(parentArea->GetRouter()->GetRouterID());
      updatePacket->setAreaID(areaID);
      updatePacket->setNumberOfLsas(1);

      switch (lsaType){
         case RouterLsaType: {
            updatePacket->setRouterLsasArraySize(1);
            updatePacket->setRouterLsas(0, *routerLSA);
            unsigned short lsAge = updatePacket->getRouterLsas(0).getHeader().getLsAge();
            if (lsAge < MAX_AGE - interfaceTransmissionDelay){
               updatePacket->getRouterLsas(0).getHeader().setLsAge(lsAge + interfaceTransmissionDelay);
            }else{
               updatePacket->getRouterLsas(0).getHeader().setLsAge(MAX_AGE);
            }}
            break;
         case NetworkLsaType: {
            updatePacket->setNetworkLsasArraySize(1);
            updatePacket->setNetworkLsas(0, *networkLSA);
            unsigned short lsAge = updatePacket->getNetworkLsas(0).getHeader().getLsAge();
            if (lsAge < MAX_AGE - interfaceTransmissionDelay){
               updatePacket->getNetworkLsas(0).getHeader().setLsAge(lsAge + interfaceTransmissionDelay);
            }else{
               updatePacket->getNetworkLsas(0).getHeader().setLsAge(MAX_AGE);
            }}
            break;
         case InterAreaPrefixLsaType: {
            updatePacket->setInterAreaPrefixLsasArraySize(1);
            updatePacket->setInterAreaPrefixLsas(0, *interAreaPrefixLSA);
            unsigned short lsAge = updatePacket->getInterAreaPrefixLsas(0).getHeader().getLsAge();
            if (lsAge < MAX_AGE - interfaceTransmissionDelay){
               updatePacket->getInterAreaPrefixLsas(0).getHeader().setLsAge(lsAge + interfaceTransmissionDelay);
            }else{
               updatePacket->getInterAreaPrefixLsas(0).getHeader().setLsAge(MAX_AGE);
            }}
            break;
         case InterAreaRouterLsaType: {
            updatePacket->setInterAreaRouterLsasArraySize(1);
            updatePacket->setInterAreaRouterLsas(0, *interAreaRouterLSA);
            unsigned short lsAge = updatePacket->getInterAreaRouterLsas(0).getHeader().getLsAge();
            if (lsAge < MAX_AGE - interfaceTransmissionDelay){
               updatePacket->getInterAreaRouterLsas(0).getHeader().setLsAge(lsAge + interfaceTransmissionDelay);
            }else{
               updatePacket->getInterAreaRouterLsas(0).getHeader().setLsAge(MAX_AGE);
            }}
            break;
         case AsExternalLsaType: {
            updatePacket->setAsExternalLsasArraySize(1);
            updatePacket->setAsExternalLsas(0, *asExternalLSA);
            unsigned short lsAge = updatePacket->getAsExternalLsas(0).getHeader().getLsAge();
            if (lsAge < MAX_AGE - interfaceTransmissionDelay){
               updatePacket->getAsExternalLsas(0).getHeader().setLsAge(lsAge + interfaceTransmissionDelay);
            }else{
               updatePacket->getAsExternalLsas(0).getHeader().setLsAge(MAX_AGE);
            }}
            break;
         case LinkLsaType: {
            updatePacket->setLinkLsasArraySize(1);
            updatePacket->setLinkLsas(0, *linkLSA);
            unsigned short lsAge = updatePacket->getLinkLsas(0).getHeader().getLsAge();
            if (lsAge < MAX_AGE - interfaceTransmissionDelay){
               updatePacket->getLinkLsas(0).getHeader().setLsAge(lsAge + interfaceTransmissionDelay);
            }else{
               updatePacket->getLinkLsas(0).getHeader().setLsAge(MAX_AGE);
            }}
            break;
         case IntraAreaPrefixLsaType: {
            updatePacket->setIntraAreaPrefixLsasArraySize(1);
            updatePacket->setIntraAreaPrefixLsas(0, *intraAreaPrefixLSA);
            unsigned short lsAge = updatePacket->getIntraAreaPrefixLsas(0).getHeader().getLsAge();
            if (lsAge < MAX_AGE - interfaceTransmissionDelay){
               updatePacket->getIntraAreaPrefixLsas(0).getHeader().setLsAge(lsAge + interfaceTransmissionDelay);
            }else{
               updatePacket->getIntraAreaPrefixLsas(0).getHeader().setLsAge(MAX_AGE);
            }}
            break;
         default:
            break;
      }
      return updatePacket;
   }
   return NULL;
}

void AnsaOspf6::Interface::AddDelayedAcknowledgement(OspfLsaHeader6& lsaHeader){
   if (interfaceType == AnsaOspf6::Interface::Broadcast){
      if (  (GetState() == AnsaOspf6::Interface::DesignatedRouterState)
         || (GetState() == AnsaOspf6::Interface::BackupState)
         || (designatedRouter == AnsaOspf6::NullDesignatedRouterID)){
         delayedAcknowledgements[AnsaOspf6::AllSPFRouters].push_back(lsaHeader);
      }else{
         delayedAcknowledgements[AnsaOspf6::AllDRouters].push_back(lsaHeader);
      }
   }else{
      long neighborCount = neighboringRouters.size();
      for (long i = 0; i < neighborCount; i++){
         if (neighboringRouters[i]->GetState() >= AnsaOspf6::Neighbor::ExchangeState){
            delayedAcknowledgements[neighboringRouters[i]->GetAddress()].push_back(lsaHeader);
         }
      }
   }
}

void AnsaOspf6::Interface::SendDelayedAcknowledgements(void){
   AnsaOspf6::MessageHandler* messageHandler = parentArea->GetRouter()->GetMessageHandler();
   std::map<IPv6Address, std::list<OspfLsaHeader6>, AnsaOspf6::IPv6Address_Less>::iterator delayIt;
   for (delayIt = delayedAcknowledgements.begin(); delayIt != delayedAcknowledgements.end(); delayIt++){
      int ackCount = delayIt->second.size();
      if (ackCount > 0){
         while (!(delayIt->second.empty())){
            OspfLinkStateAcknowledgementPacket6* ackPacket = new OspfLinkStateAcknowledgementPacket6;

            ackPacket->setType(LinkStateAcknowledgementPacket);
            ackPacket->setRouterID(parentArea->GetRouter()->GetRouterID());
            ackPacket->setAreaID(areaID);

            while (!(delayIt->second.empty())){
               unsigned long headerCount = ackPacket->getLsaHeadersArraySize();
               ackPacket->setLsaHeadersArraySize(headerCount + 1);
               ackPacket->setLsaHeaders(headerCount, *(delayIt->second.begin()));
               delayIt->second.pop_front();
            }

            int ttl = (interfaceType == AnsaOspf6::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;

            if (interfaceType == AnsaOspf6::Interface::Broadcast){
               if (  (GetState() == AnsaOspf6::Interface::DesignatedRouterState)
                  || (GetState() == AnsaOspf6::Interface::BackupState)
                  || (designatedRouter == AnsaOspf6::NullDesignatedRouterID)){
                  messageHandler->SendPacket(ackPacket, AnsaOspf6::AllSPFRouters, ifIndex, ttl);
               }else{
                  messageHandler->SendPacket(ackPacket, AnsaOspf6::AllDRouters, ifIndex, ttl);
               }
            }else{
               if (interfaceType == AnsaOspf6::Interface::PointToPoint){
                  messageHandler->SendPacket(ackPacket, AnsaOspf6::AllSPFRouters, ifIndex, ttl);
               }else{
                  messageHandler->SendPacket(ackPacket, delayIt->first, ifIndex, ttl);
               }
            }
         }
      }
   }
   messageHandler->StartTimer(acknowledgementTimer, acknowledgementDelay);
}

void AnsaOspf6::Interface::AgeTransmittedLSALists(void) {
   long neighborCount = neighboringRouters.size();
   for (long i = 0; i < neighborCount; i++){
      neighboringRouters[i]->AgeTransmittedLSAList();
   }
}
