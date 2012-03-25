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
#include "ansaOspfRouter6.h"
#include "ansaMessageHandler6.h"

#include "ansaOspfNeighbor6.h"
#include "ansaOspfNeighborState6.h"
#include "ansaOspfNeighborStateDown6.h"

// FIXME!!! Should come from a global unique number generator module.
unsigned long AnsaOspf6::Neighbor::ddSequenceNumberInitSeed = 0;

AnsaOspf6::Neighbor::Neighbor(RouterID neighbor) :
      updateRetransmissionTimerActive(false),
      requestRetransmissionTimerActive(false),
      firstAdjacencyInited(false),
      ddSequenceNumber(0),
      neighborID(neighbor),
      neighborPriority(0),
      neighborIPAddress(IPv6Address::UNSPECIFIED_ADDRESS),
      neighborsDesignatedRouter(AnsaOspf6::NullDesignatedRouterID),
      neighborsBackupDesignatedRouter(AnsaOspf6::NullDesignatedRouterID),
      designatedRoutersSetUp(false),
      neighborsRouterDeadInterval(40),
      lastTransmittedDDPacket(NULL){

   inactivityTimer = new OspfTimer6;
   inactivityTimer->setTimerKind(NeighborInactivityTimer);
   inactivityTimer->setContextPointer(this);
   inactivityTimer->setName("AnsaOspf6::Neighbor::NeighborInactivityTimer");
   pollTimer = new OspfTimer6;
   pollTimer->setTimerKind(NeighborPollTimer);
   pollTimer->setContextPointer(this);
   pollTimer->setName("AnsaOspf6::Neighbor::NeighborPollTimer");

   /* TODO
   ddRetransmissionTimer = new OspfTimer6;
   ddRetransmissionTimer->setTimerKind(NeighborDDRetransmissionTimer);
   ddRetransmissionTimer->setContextPointer(this);
   ddRetransmissionTimer->setName("AnsaOspf6::Neighbor::NeighborDDRetransmissionTimer");
   updateRetransmissionTimer = new OspfTimer6;
   updateRetransmissionTimer->setTimerKind(NeighborUpdateRetransmissionTimer);
   updateRetransmissionTimer->setContextPointer(this);
   updateRetransmissionTimer->setName("AnsaOspf6::Neighbor::Neighbor::NeighborUpdateRetransmissionTimer");
   requestRetransmissionTimer = new OspfTimer6;
   requestRetransmissionTimer->setTimerKind(NeighborRequestRetransmissionTimer);
   requestRetransmissionTimer->setContextPointer(this);
   requestRetransmissionTimer->setName("AnsaOspf6::Neighbor::NeighborRequestRetransmissionTimer");

   memset(&lastReceivedDDPacket, 0, sizeof(AnsaOspf6::Neighbor::DDPacketID));
   // setting only I and M bits is invalid -> good initializer
   lastReceivedDDPacket.ddOptions.I_Init = true;
   lastReceivedDDPacket.ddOptions.M_More = true;
   */

   state = new AnsaOspf6::NeighborStateDown;
   previousState = NULL;
}

AnsaOspf6::Neighbor::~Neighbor(void){
   Reset();
   MessageHandler* messageHandler = parentInterface->GetArea()->GetRouter()->GetMessageHandler();

   if (inactivityTimer != NULL){
      messageHandler->ClearTimer(inactivityTimer);
      delete inactivityTimer;
   }

   if (pollTimer != NULL){
      messageHandler->ClearTimer(pollTimer);
      delete pollTimer;
   }

   /*
   delete ddRetransmissionTimer;
   delete updateRetransmissionTimer;
   delete requestRetransmissionTimer;
   */

   if (previousState != NULL) {
      delete previousState;
   }
   delete state;
}

void AnsaOspf6::Neighbor::ChangeState(NeighborState* newState, NeighborState* currentState) {
   if (previousState != NULL){
      delete previousState;
   }
   state = newState;
   previousState = currentState;
}

void AnsaOspf6::Neighbor::ProcessEvent(AnsaOspf6::Neighbor::NeighborEventType event) {
   state->ProcessEvent(this, event);
}

void AnsaOspf6::Neighbor::Reset(void) {

   /* TODO:
   for (std::list<OspfLsa6*>::iterator retIt = linkStateRetransmissionList.begin(); retIt
         != linkStateRetransmissionList.end(); retIt++){
      delete (*retIt);
   }
   linkStateRetransmissionList.clear();

   std::list<OspfLsaHeader6*>::iterator it;
   for (it = databaseSummaryList.begin(); it != databaseSummaryList.end(); it++){
      delete (*it);
   }
   databaseSummaryList.clear();
   for (it = linkStateRequestList.begin(); it != linkStateRequestList.end(); it++){
      delete (*it);
   }
   linkStateRequestList.clear();

   parentInterface->GetArea()->GetRouter()->GetMessageHandler()->ClearTimer(ddRetransmissionTimer);
   ClearUpdateRetransmissionTimer();
   ClearRequestRetransmissionTimer();

   if (lastTransmittedDDPacket != NULL){
      delete lastTransmittedDDPacket;
      lastTransmittedDDPacket = NULL;
   }
   */
}

void AnsaOspf6::Neighbor::InitFirstAdjacency(void) {
   ddSequenceNumber = GetUniqueULong();
   firstAdjacencyInited = true;
}

unsigned long AnsaOspf6::Neighbor::GetUniqueULong(void) {
   return (ddSequenceNumberInitSeed++);
}

AnsaOspf6::Neighbor::NeighborStateType AnsaOspf6::Neighbor::GetState(void) const {
   return state->GetState();
}

const char* AnsaOspf6::Neighbor::GetStateString(AnsaOspf6::Neighbor::NeighborStateType stateType) {
   switch (stateType){
      case DownState:
         return "Down";
      case AttemptState:
         return "Attempt";
      case InitState:
         return "Init";
      case TwoWayState:
         return "TwoWay";
      case ExchangeStartState:
         return "ExchangeStart";
      case ExchangeState:
         return "Exchange";
      case LoadingState:
         return "Loading";
      case FullState:
         return "Full";
      default:
         ASSERT(false);
   }
   return "";
}

void AnsaOspf6::Neighbor::SendDatabaseDescriptionPacket(bool init) {
   OspfDatabaseDescriptionPacket6* ddPacket = new OspfDatabaseDescriptionPacket6;

   ddPacket->setType(DatabaseDescriptionPacket);
   ddPacket->setRouterID(parentInterface->GetArea()->GetRouter()->GetRouterID());
   ddPacket->setAreaID(parentInterface->GetArea()->GetAreaID());

   if (parentInterface->GetType() != AnsaOspf6::Interface::Virtual){
      ddPacket->setInterfaceMtu(parentInterface->GetMtu());
   }else{
      ddPacket->setInterfaceMtu(0);
   }

   OspfOptions6 options;
   memset(&options, 0, sizeof(OspfOptions6));
   options.E_ExternalRoutingCapability = parentInterface->GetArea()->GetExternalRoutingCapability();
   ddPacket->setOptions(options);

   ddPacket->setDdSequenceNumber(ddSequenceNumber);

   if (init || databaseSummaryList.empty()){
      ddPacket->setLsaHeadersArraySize(0);
   }else{
      // delete included LSAs from summary list
      // (they are still in lastTransmittedDDPacket)
      while (!databaseSummaryList.empty()){
         unsigned long headerCount = ddPacket->getLsaHeadersArraySize();
         OspfLsaHeader6* lsaHeader = *(databaseSummaryList.begin());
         ddPacket->setLsaHeadersArraySize(headerCount + 1);
         ddPacket->setLsaHeaders(headerCount, *lsaHeader);
         delete lsaHeader;
         databaseSummaryList.pop_front();
      }
   }

   OspfDdOptions6 ddOptions;
   memset(&ddOptions, 0, sizeof(OspfDdOptions6));
   if (init){
      ddOptions.I_Init = true;
      ddOptions.M_More = true;
      ddOptions.MS_MasterSlave = true;
   }else{
      ddOptions.I_Init = false;
      ddOptions.M_More = (databaseSummaryList.empty()) ? false : true;
      ddOptions.MS_MasterSlave = (databaseExchangeRelationship == AnsaOspf6::Neighbor::Master) ? true : false;
   }
   ddPacket->setDdOptions(ddOptions);

   AnsaOspf6::MessageHandler* messageHandler = parentInterface->GetArea()->GetRouter()->GetMessageHandler();
   int ttl = (parentInterface->GetType() == AnsaOspf6::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;
   if (parentInterface->GetType() == AnsaOspf6::Interface::PointToPoint){
      messageHandler->SendPacket(ddPacket, AnsaOspf6::AllSPFRouters, parentInterface->GetIfIndex(), ttl);
   }else{
      messageHandler->SendPacket(ddPacket, neighborIPAddress, parentInterface->GetIfIndex(), ttl);
   }

   if (lastTransmittedDDPacket != NULL){
      delete lastTransmittedDDPacket;
   }
   lastTransmittedDDPacket = new OspfDatabaseDescriptionPacket6(*ddPacket);
}

bool AnsaOspf6::Neighbor::RetransmitDatabaseDescriptionPacket(void) {
   if (lastTransmittedDDPacket != NULL){
      OspfDatabaseDescriptionPacket6* ddPacket = new OspfDatabaseDescriptionPacket6(
            *lastTransmittedDDPacket);
      AnsaOspf6::MessageHandler* messageHandler =
            parentInterface->GetArea()->GetRouter()->GetMessageHandler();
      int ttl = (parentInterface->GetType() == AnsaOspf6::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;

      if (parentInterface->GetType() == AnsaOspf6::Interface::PointToPoint){
         messageHandler->SendPacket(ddPacket, AnsaOspf6::AllSPFRouters,
               parentInterface->GetIfIndex(), ttl);
      }else{
         messageHandler->SendPacket(ddPacket, neighborIPAddress, parentInterface->GetIfIndex(), ttl);
      }

      return true;
   }else{
      return false;
   }
}

void AnsaOspf6::Neighbor::CreateDatabaseSummary(void) {

   AnsaOspf6::Area* area = parentInterface->GetArea();
   unsigned long routerLSACount = area->GetRouterLSACount();

   for (unsigned long i = 0; i < routerLSACount; i++){
      if (area->GetRouterLSA(i)->getHeader().getLsAge() < MAX_AGE){
         OspfLsaHeader6* routerLSA = new OspfLsaHeader6(area->GetRouterLSA(i)->getHeader());
         databaseSummaryList.push_back(routerLSA);
      }
   }

   unsigned long networkLSACount = area->GetNetworkLSACount();
   for (unsigned long j = 0; j < networkLSACount; j++){
      if (area->GetNetworkLSA(j)->getHeader().getLsAge() < MAX_AGE){
         OspfLsaHeader6* networkLSA = new OspfLsaHeader6(area->GetNetworkLSA(j)->getHeader());
         databaseSummaryList.push_back(networkLSA);
      }
   }

   unsigned long interAreaPrefixLSACount = area->GetInterAreaPrefixLSACount();
   for (unsigned long j = 0; j < interAreaPrefixLSACount; j++){
      if (area->GetInterAreaPrefixLSA(j)->getHeader().getLsAge() < MAX_AGE){
         OspfLsaHeader6* interAreaPrefixLSA = new OspfLsaHeader6(area->GetInterAreaPrefixLSA(j)->getHeader());
         databaseSummaryList.push_back(interAreaPrefixLSA);
      }
   }

   unsigned long interAreaRouterLSACount = area->GetInterAreaRouterLSACount();
   for (unsigned long j = 0; j < interAreaRouterLSACount; j++){
      if (area->GetInterAreaRouterLSA(j)->getHeader().getLsAge() < MAX_AGE){
         OspfLsaHeader6* interAreaRouterLSA = new OspfLsaHeader6(area->GetInterAreaRouterLSA(j)->getHeader());
         databaseSummaryList.push_back(interAreaRouterLSA);
      }
   }

   /* TODO: link lsa?
   unsigned long networkLSACount = area->GetNetworkLSACount();
   for (unsigned long j = 0; j < networkLSACount; j++){
      if (area->GetNetworkLSA(j)->getHeader().getLsAge() < MAX_AGE){
         OspfLsaHeader6* networkLSA = new OspfLsaHeader6(area->GetNetworkLSA(j)->getHeader());
         databaseSummaryList.push_back(networkLSA);
      }
   }
   */

   if (  (parentInterface->GetType() != AnsaOspf6::Interface::Virtual)
      && (area->GetExternalRoutingCapability())){
      AnsaOspf6::Router* router = area->GetRouter();
      unsigned long asExternalLSACount = router->GetASExternalLSACount();

      for (unsigned long m = 0; m < asExternalLSACount; m++){
         if (router->GetASExternalLSA(m)->getHeader().getLsAge() < MAX_AGE){
            OspfLsaHeader6* asExternalLSA = new OspfLsaHeader6(router->GetASExternalLSA(m)->getHeader());
            databaseSummaryList.push_back(asExternalLSA);
         }
      }
   }

   unsigned long intraAreaPrefixLSACount = area->GetIntraAreaPrefixLSACount();
   for (unsigned long j = 0; j < intraAreaPrefixLSACount; j++){
      if (area->GetIntraAreaPrefixLSA(j)->getHeader().getLsAge() < MAX_AGE){
         OspfLsaHeader6* intraAreaPrefixLSA = new OspfLsaHeader6(area->GetIntraAreaPrefixLSA(j)->getHeader());
         databaseSummaryList.push_back(intraAreaPrefixLSA);
      }
   }
}

void AnsaOspf6::Neighbor::SendLinkStateRequestPacket(void) {
   OspfLinkStateRequestPacket6* requestPacket = new OspfLinkStateRequestPacket6;

   requestPacket->setType(LinkStateRequestPacket);
   requestPacket->setRouterID(parentInterface->GetArea()->GetRouter()->GetRouterID());
   requestPacket->setAreaID(parentInterface->GetArea()->GetAreaID());

   if (linkStateRequestList.empty()){
      requestPacket->setRequestsArraySize(0);
   }else{
      std::list<OspfLsaHeader6*>::iterator it = linkStateRequestList.begin();

      while (it != linkStateRequestList.end()){
         unsigned long requestCount = requestPacket->getRequestsArraySize();
         OspfLsaHeader6* requestHeader = (*it);
         OspfLsaRequest6 request;

         request.lsType = requestHeader->getLsType();
         request.linkStateID = requestHeader->getLinkStateID();
         request.advertisingRouter = requestHeader->getAdvertisingRouter();

         requestPacket->setRequestsArraySize(requestCount + 1);
         requestPacket->setRequests(requestCount, request);

         it++;
      }
   }

   AnsaOspf6::MessageHandler* messageHandler = parentInterface->GetArea()->GetRouter()->GetMessageHandler();
   int ttl = (parentInterface->GetType() == AnsaOspf6::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;
   if (parentInterface->GetType() == AnsaOspf6::Interface::PointToPoint){
      messageHandler->SendPacket(requestPacket, AnsaOspf6::AllSPFRouters, parentInterface->GetIfIndex(), ttl);
   }else{
      messageHandler->SendPacket(requestPacket, neighborIPAddress, parentInterface->GetIfIndex(), ttl);
   }
}

bool AnsaOspf6::Neighbor::NeedAdjacency(void) {

   /* TODO:
   AnsaOspf6::Interface::OspfInterfaceType interfaceType = parentInterface->GetType();
   AnsaOspf6::RouterID routerID = parentInterface->GetArea()->GetRouter()->GetRouterID();
   AnsaOspf6::DesignatedRouterID dRouter = parentInterface->GetDesignatedRouter();
   AnsaOspf6::DesignatedRouterID backupDRouter = parentInterface->GetBackupDesignatedRouter();

   if (  (interfaceType == AnsaOspf6::Interface::PointToPoint)
      || (interfaceType == AnsaOspf6::Interface::PointToMultiPoint)
      || (interfaceType == AnsaOspf6::Interface::Virtual)
      || (dRouter == routerID)
      || (backupDRouter == routerID)
      || ((neighborsDesignatedRouter == dRouter) || (!designatedRoutersSetUp))
      || (neighborsBackupDesignatedRouter == backupDRouter)
      || (!designatedRoutersSetUp)){
      return true;
   }else{
      return false;
   }
   */

   return false;
}

/**
 * If the LSA is already on the retransmission list then it is replaced, else
 * a copy of the LSA is added to the end of the retransmission list.
 * @param lsa [in] The LSA to be added.
 */
void AnsaOspf6::Neighbor::AddToRetransmissionList(OspfLsa6* lsa) {
   std::list<OspfLsa6*>::iterator it;
   for (it = linkStateRetransmissionList.begin(); it != linkStateRetransmissionList.end(); it++){
      if (  ((*it)->getHeader().getLinkStateID() == lsa->getHeader().getLinkStateID())
         && ((*it)->getHeader().getAdvertisingRouter() == lsa->getHeader().getAdvertisingRouter())){
         break;
      }
   }

   OspfLsa6* lsaCopy = NULL;
   switch (lsa->getHeader().getLsType()){
      case RouterLsaType:
         lsaCopy = new OspfRouterLsa6(*(check_and_cast<OspfRouterLsa6*> (lsa)));
         break;
      case NetworkLsaType:
         lsaCopy = new OspfNetworkLsa6(*(check_and_cast<OspfNetworkLsa6*> (lsa)));
         break;
      case InterAreaPrefixLsaType:
         lsaCopy = new OspfInterAreaPrefixLsa6(*(check_and_cast<OspfInterAreaPrefixLsa6*> (lsa)));
         break;
      case InterAreaRouterLsaType:
         lsaCopy = new OspfInterAreaRouterLsa6(*(check_and_cast<OspfInterAreaRouterLsa6*> (lsa)));
         break;
      case AsExternalLsaType:
         lsaCopy = new OspfAsExternalLsa6(*(check_and_cast<OspfAsExternalLsa6*> (lsa)));
         break;
      // TODO ?? patøí to sem
      case LinkLsaType:
         lsaCopy = new OspfLinkLsa6(*(check_and_cast<OspfLinkLsa6*> (lsa)));
         break;
      case IntraAreaPrefixLsaType:
         lsaCopy = new OspfIntraAreaPrefixLsa6(*(check_and_cast<OspfIntraAreaPrefixLsa6*> (lsa)));
         break;
      default:
         ASSERT(false); // error
         break;
   }

   if (it != linkStateRetransmissionList.end()){
      delete (*it);
      *it = static_cast<OspfLsa6*> (lsaCopy);
   }else{
      linkStateRetransmissionList.push_back(static_cast<OspfLsa6*> (lsaCopy));
   }
}

void AnsaOspf6::Neighbor::RemoveFromRetransmissionList(AnsaOspf6::LsaKeyType6 lsaKey) {
   std::list<OspfLsa6*>::iterator it = linkStateRetransmissionList.begin();
   while (it != linkStateRetransmissionList.end()){
      if (((*it)->getHeader().getLinkStateID() == lsaKey.linkStateID)
            && ((*it)->getHeader().getAdvertisingRouter() == lsaKey.advertisingRouter)){
         delete (*it);
         it = linkStateRetransmissionList.erase(it);
      }else{
         it++;
      }
   }
}

bool AnsaOspf6::Neighbor::IsLSAOnRetransmissionList(AnsaOspf6::LsaKeyType6 lsaKey) const {
   for (std::list<OspfLsa6*>::const_iterator it = linkStateRetransmissionList.begin(); it
         != linkStateRetransmissionList.end(); it++){
      const OspfLsa6* lsa = *it;
      if ((lsa->getHeader().getLinkStateID() == lsaKey.linkStateID)
            && (lsa->getHeader().getAdvertisingRouter() == lsaKey.advertisingRouter)){
         return true;
      }
   }
   return false;
}

OspfLsa6* AnsaOspf6::Neighbor::FindOnRetransmissionList(AnsaOspf6::LsaKeyType6 lsaKey) {
   for (std::list<OspfLsa6*>::iterator it = linkStateRetransmissionList.begin(); it
         != linkStateRetransmissionList.end(); it++){
      if (((*it)->getHeader().getLinkStateID() == lsaKey.linkStateID)
            && ((*it)->getHeader().getAdvertisingRouter() == lsaKey.advertisingRouter)){
         return (*it);
      }
   }
   return NULL;
}

void AnsaOspf6::Neighbor::StartUpdateRetransmissionTimer(void) {
   MessageHandler* messageHandler = parentInterface->GetArea()->GetRouter()->GetMessageHandler();
   messageHandler->StartTimer(updateRetransmissionTimer,
         parentInterface->GetRetransmissionInterval());
   updateRetransmissionTimerActive = true;
}

void AnsaOspf6::Neighbor::ClearUpdateRetransmissionTimer(void) {
   MessageHandler* messageHandler = parentInterface->GetArea()->GetRouter()->GetMessageHandler();
   messageHandler->ClearTimer(updateRetransmissionTimer);
   updateRetransmissionTimerActive = false;
}

void AnsaOspf6::Neighbor::AddToRequestList(OspfLsaHeader6* lsaHeader) {
   linkStateRequestList.push_back(new OspfLsaHeader6 (*lsaHeader));
}

void AnsaOspf6::Neighbor::RemoveFromRequestList(AnsaOspf6::LsaKeyType6 lsaKey) {
   std::list<OspfLsaHeader6*>::iterator it = linkStateRequestList.begin();
   while (it != linkStateRequestList.end()){
      if (((*it)->getLinkStateID() == lsaKey.linkStateID)
            && ((*it)->getAdvertisingRouter() == lsaKey.advertisingRouter)){
         delete (*it);
         it = linkStateRequestList.erase(it);
      }else{
         it++;
      }
   }

   if ((GetState() == AnsaOspf6::Neighbor::LoadingState) && (linkStateRequestList.empty())){
      ClearRequestRetransmissionTimer();
      ProcessEvent(AnsaOspf6::Neighbor::LoadingDone);
   }
}

bool AnsaOspf6::Neighbor::IsLSAOnRequestList(AnsaOspf6::LsaKeyType6 lsaKey) const {
   for (std::list<OspfLsaHeader6*>::const_iterator it = linkStateRequestList.begin(); it
         != linkStateRequestList.end(); it++){
      const OspfLsaHeader6* lsaHeader = *it;
      if ((lsaHeader->getLinkStateID() == lsaKey.linkStateID)
            && (lsaHeader->getAdvertisingRouter() == lsaKey.advertisingRouter)){
         return true;
      }
   }
   return false;
}

OspfLsaHeader6* AnsaOspf6::Neighbor::FindOnRequestList(AnsaOspf6::LsaKeyType6 lsaKey) {
   for (std::list<OspfLsaHeader6*>::iterator it = linkStateRequestList.begin(); it
         != linkStateRequestList.end(); it++){
      if (((*it)->getLinkStateID() == lsaKey.linkStateID)
            && ((*it)->getAdvertisingRouter() == lsaKey.advertisingRouter)){
         return (*it);
      }
   }
   return NULL;
}

void AnsaOspf6::Neighbor::StartRequestRetransmissionTimer(void) {
   MessageHandler* messageHandler = parentInterface->GetArea()->GetRouter()->GetMessageHandler();
   messageHandler->StartTimer(requestRetransmissionTimer,
         parentInterface->GetRetransmissionInterval());
   requestRetransmissionTimerActive = true;
}

void AnsaOspf6::Neighbor::ClearRequestRetransmissionTimer(void) {
   MessageHandler* messageHandler = parentInterface->GetArea()->GetRouter()->GetMessageHandler();
   messageHandler->ClearTimer(requestRetransmissionTimer);
   requestRetransmissionTimerActive = false;
}

void AnsaOspf6::Neighbor::AddToTransmittedLSAList(AnsaOspf6::LsaKeyType6 lsaKey) {
   TransmittedLSA transmit;

   transmit.lsaKey = lsaKey;
   transmit.age = 0;

   transmittedLSAs.push_back(transmit);
}

bool AnsaOspf6::Neighbor::IsOnTransmittedLSAList(AnsaOspf6::LsaKeyType6 lsaKey) const {
   for (std::list<TransmittedLSA>::const_iterator it = transmittedLSAs.begin(); it
         != transmittedLSAs.end(); it++){
      if ((it->lsaKey.linkStateID == lsaKey.linkStateID) && (it->lsaKey.advertisingRouter
            == lsaKey.advertisingRouter)){
         return true;
      }
   }
   return false;
}

void AnsaOspf6::Neighbor::AgeTransmittedLSAList(void) {
   std::list<TransmittedLSA>::iterator it = transmittedLSAs.begin();
   while ((it != transmittedLSAs.end()) && (it->age == MIN_LS_ARRIVAL)){
      transmittedLSAs.pop_front();
      it = transmittedLSAs.begin();
   }
   for (it = transmittedLSAs.begin(); it != transmittedLSAs.end(); it++){
      it->age++;
   }
}

void AnsaOspf6::Neighbor::RetransmitUpdatePacket(void) {
   OspfLinkStateUpdatePacket6* updatePacket = new OspfLinkStateUpdatePacket6;

   updatePacket->setType(LinkStateUpdatePacket);
   updatePacket->setRouterID(parentInterface->GetArea()->GetRouter()->GetRouterID());
   updatePacket->setAreaID(parentInterface->GetArea()->GetAreaID());

   bool packetFull = false;
   unsigned short lsaCount = 0;
   std::list<OspfLsa6*>::iterator it = linkStateRetransmissionList.begin();

   while (!packetFull && (it != linkStateRetransmissionList.end())){
      LsaType6 lsaType = static_cast<LsaType6> ((*it)->getHeader().getLsType());
      OspfRouterLsa6* routerLSA = (lsaType == RouterLsaType) ? dynamic_cast<OspfRouterLsa6*> (*it) : NULL;
      OspfNetworkLsa6* networkLSA = (lsaType == NetworkLsaType) ? dynamic_cast<OspfNetworkLsa6*> (*it) : NULL;
      OspfInterAreaPrefixLsa6* interAreaPrefixLSA = (lsaType = InterAreaPrefixLsaType) ? dynamic_cast<OspfInterAreaPrefixLsa6*> (*it) : NULL;
      OspfInterAreaRouterLsa6* interAreaRouterLSA = (lsaType = InterAreaRouterLsaType) ? dynamic_cast<OspfInterAreaRouterLsa6*> (*it) : NULL;
      OspfAsExternalLsa6* asExternalLSA = (lsaType == AsExternalLsaType) ? dynamic_cast<OspfAsExternalLsa6*> (*it) : NULL;
      OspfLinkLsa6* linkLSA = (lsaType == LinkLsaType) ?  dynamic_cast<OspfLinkLsa6*> (*it) : NULL;
      OspfIntraAreaPrefixLsa6* intraAreaPrefixLSA = (lsaType = IntraAreaPrefixLsaType) ?  dynamic_cast<OspfIntraAreaPrefixLsa6*> (*it) : NULL;

      long lsaSize = 0;
      bool includeLSA = true;
      lsaCount++;

      if (includeLSA){
         switch (lsaType){
            case RouterLsaType:
               if (routerLSA != NULL){
                  unsigned int routerLSACount = updatePacket->getRouterLsasArraySize();

                  updatePacket->setRouterLsasArraySize(routerLSACount + 1);
                  updatePacket->setRouterLsas(routerLSACount, *routerLSA);

                  unsigned short lsAge = updatePacket->getRouterLsas(routerLSACount).getHeader().getLsAge();
                  if (lsAge < MAX_AGE - parentInterface->GetTransmissionDelay()){
                     updatePacket->getRouterLsas(routerLSACount).getHeader().setLsAge(lsAge + parentInterface->GetTransmissionDelay());
                  }else{
                     updatePacket->getRouterLsas(routerLSACount).getHeader().setLsAge(MAX_AGE);
                  }
               }
               break;
            case NetworkLsaType:
               if (networkLSA != NULL){
                  unsigned int networkLSACount = updatePacket->getNetworkLsasArraySize();

                  updatePacket->setNetworkLsasArraySize(networkLSACount + 1);
                  updatePacket->setNetworkLsas(networkLSACount, *networkLSA);

                  unsigned short lsAge = updatePacket->getNetworkLsas(networkLSACount).getHeader().getLsAge();
                  if (lsAge < MAX_AGE - parentInterface->GetTransmissionDelay()){
                     updatePacket->getNetworkLsas(networkLSACount).getHeader().setLsAge(lsAge + parentInterface->GetTransmissionDelay());
                  }else{
                     updatePacket->getNetworkLsas(networkLSACount).getHeader().setLsAge(MAX_AGE);
                  }
               }
               break;
            case InterAreaPrefixLsaType:
               if (interAreaPrefixLSA != NULL){
                  unsigned int interAreaPrefixLSACount = updatePacket->getInterAreaPrefixLsasArraySize();

                  updatePacket->setInterAreaPrefixLsasArraySize(interAreaPrefixLSACount + 1);
                  updatePacket->setInterAreaPrefixLsas(interAreaPrefixLSACount, *interAreaPrefixLSA);

                  unsigned short lsAge = updatePacket->getInterAreaPrefixLsas(interAreaPrefixLSACount).getHeader().getLsAge();
                  if (lsAge < MAX_AGE - parentInterface->GetTransmissionDelay()){
                     updatePacket->getInterAreaPrefixLsas(interAreaPrefixLSACount).getHeader().setLsAge(lsAge + parentInterface->GetTransmissionDelay());
                  }else{
                     updatePacket->getInterAreaPrefixLsas(interAreaPrefixLSACount).getHeader().setLsAge(MAX_AGE);
                  }
               }
               break;
            case InterAreaRouterLsaType:
               if (interAreaRouterLSA != NULL){
                  unsigned int interAreaRouterLSACount = updatePacket->getInterAreaRouterLsasArraySize();

                  updatePacket->setInterAreaRouterLsasArraySize(interAreaRouterLSACount + 1);
                  updatePacket->setInterAreaRouterLsas(interAreaRouterLSACount, *interAreaRouterLSA);

                  unsigned short lsAge = updatePacket->getInterAreaRouterLsas(interAreaRouterLSACount).getHeader().getLsAge();
                  if (lsAge < MAX_AGE - parentInterface->GetTransmissionDelay()){
                     updatePacket->getInterAreaRouterLsas(interAreaRouterLSACount).getHeader().setLsAge(lsAge + parentInterface->GetTransmissionDelay());
                  }else{
                     updatePacket->getInterAreaRouterLsas(interAreaRouterLSACount).getHeader().setLsAge(MAX_AGE);
                  }
               }
               break;
            case AsExternalLsaType:
               if (asExternalLSA != NULL){
                  unsigned int asExternalLSACount = updatePacket->getAsExternalLsasArraySize();

                  updatePacket->setAsExternalLsasArraySize(asExternalLSACount + 1);
                  updatePacket->setAsExternalLsas(asExternalLSACount, *asExternalLSA);

                  unsigned short lsAge =
                        updatePacket->getAsExternalLsas(asExternalLSACount).getHeader().getLsAge();
                  if (lsAge < MAX_AGE - parentInterface->GetTransmissionDelay()){
                     updatePacket->getAsExternalLsas(asExternalLSACount).getHeader().setLsAge(lsAge
                           + parentInterface->GetTransmissionDelay());
                  }else{
                     updatePacket->getAsExternalLsas(asExternalLSACount).getHeader().setLsAge(
                           MAX_AGE);
                  }
               }
               break;
            case LinkLsaType:
               if (linkLSA != NULL){
                  unsigned int linkLSACount = updatePacket->getLinkLsasArraySize();

                  updatePacket->setLinkLsasArraySize(linkLSACount + 1);
                  updatePacket->setLinkLsas(linkLSACount, *linkLSA);

                  unsigned short lsAge = updatePacket->getLinkLsas(linkLSACount).getHeader().getLsAge();
                  if (lsAge < MAX_AGE - parentInterface->GetTransmissionDelay()){
                     updatePacket->getLinkLsas(linkLSACount).getHeader().setLsAge(lsAge + parentInterface->GetTransmissionDelay());
                  }else{
                     updatePacket->getLinkLsas(linkLSACount).getHeader().setLsAge(MAX_AGE);
                  }
               }
               break;
            case IntraAreaPrefixLsaType:
               if (intraAreaPrefixLSA != NULL){
                  unsigned int intraAreaPrefixLSACount = updatePacket->getIntraAreaPrefixLsasArraySize();

                  updatePacket->setIntraAreaPrefixLsasArraySize(intraAreaPrefixLSACount + 1);
                  updatePacket->setIntraAreaPrefixLsas(intraAreaPrefixLSACount, *intraAreaPrefixLSA);

                  unsigned short lsAge = updatePacket->getIntraAreaPrefixLsas(intraAreaPrefixLSACount).getHeader().getLsAge();
                  if (lsAge < MAX_AGE - parentInterface->GetTransmissionDelay()){
                     updatePacket->getIntraAreaPrefixLsas(intraAreaPrefixLSACount).getHeader().setLsAge(lsAge + parentInterface->GetTransmissionDelay());
                  }else{
                     updatePacket->getIntraAreaPrefixLsas(intraAreaPrefixLSACount).getHeader().setLsAge(MAX_AGE);
                  }
               }
               break;
            default:
               break;
         }
      }

      it++;
   }

   AnsaOspf6::MessageHandler* messageHandler = parentInterface->GetArea()->GetRouter()->GetMessageHandler();
   int ttl = (parentInterface->GetType() == AnsaOspf6::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;
   messageHandler->SendPacket(updatePacket, neighborIPAddress, parentInterface->GetIfIndex(), ttl);
}

void AnsaOspf6::Neighbor::DeleteLastSentDDPacket(void) {
   if (lastTransmittedDDPacket != NULL){
      delete lastTransmittedDDPacket;
      lastTransmittedDDPacket = NULL;
   }
}
