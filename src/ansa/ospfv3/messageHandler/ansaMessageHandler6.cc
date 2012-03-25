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

#include "IPv6ControlInfo.h"

#include "ansaMessageHandler6.h"
#include "ansaOspfRouter6.h"

AnsaOspf6::MessageHandler::MessageHandler(AnsaOspf6::Router* containingRouter, cSimpleModule* containingModule) :
      AnsaOspf6::IMessageHandler(containingRouter),
      ospfModule(containingModule),
      helloHandler(containingRouter),
      ddHandler(containingRouter),
      lsRequestHandler(containingRouter),
      lsUpdateHandler(containingRouter),
      lsAckHandler(containingRouter){
}

void AnsaOspf6::MessageHandler::MessageReceived(cMessage* message) {
   if (message->isSelfMessage()){
      HandleTimer(check_and_cast<OspfTimer6*> (message));
   }else{
      OspfPacket6* packet = check_and_cast<OspfPacket6*> (message);
      EV << "Received packet: (" << packet->getClassName() << ")" << packet->getName() << "\n";
      if (packet->getRouterID() == router->GetRouterID()){
         EV << "This packet is from ourselves, discarding.\n";
         delete message;
      }else{
         ProcessPacket(packet);
      }
   }
}

void AnsaOspf6::MessageHandler::HandleTimer(OspfTimer6* timer){
   switch (timer->getTimerKind()){
      case InterfaceHelloTimer: {
         AnsaOspf6::Interface* intf;
         if (!(intf = reinterpret_cast<AnsaOspf6::Interface*> (timer->getContextPointer()))){
            // should not reach this point
            EV<< "Discarding invalid InterfaceHelloTimer.\n";
            delete timer;
         }else{
            PrintEvent("Hello Timer expired", intf);
            intf->ProcessEvent(AnsaOspf6::Interface::HelloTimer);
         }}
         break;
      case InterfaceWaitTimer: {
         AnsaOspf6::Interface* intf;
         if (! (intf = reinterpret_cast <AnsaOspf6::Interface*> (timer->getContextPointer()))){
            // should not reach this point
            EV << "Discarding invalid InterfaceWaitTimer.\n";
            delete timer;
         }else{
            PrintEvent("Wait Timer expired", intf);
            intf->ProcessEvent(AnsaOspf6::Interface::WaitTimer);
         }}
         break;
      case InterfaceAcknowledgementTimer: {
         AnsaOspf6::Interface* intf;
         if (! (intf = reinterpret_cast <AnsaOspf6::Interface*> (timer->getContextPointer()))){
            // should not reach this point
            EV << "Discarding invalid InterfaceAcknowledgementTimer.\n";
            delete timer;
         } else{
            PrintEvent("Acknowledgement Timer expired", intf);
            intf->ProcessEvent(AnsaOspf6::Interface::AcknowledgementTimer);
         }}
         break;
      case NeighborInactivityTimer: {
         AnsaOspf6::Neighbor* neighbor;
         if (! (neighbor = reinterpret_cast <AnsaOspf6::Neighbor*> (timer->getContextPointer()))){
            // should not reach this point
            EV << "Discarding invalid NeighborInactivityTimer.\n";
            delete timer;
         } else{
            PrintEvent("Inactivity Timer expired", neighbor->GetInterface(), neighbor);
            neighbor->ProcessEvent(AnsaOspf6::Neighbor::InactivityTimer);
         }}
         break;
      case NeighborPollTimer: {
         AnsaOspf6::Neighbor* neighbor;
         if (! (neighbor = reinterpret_cast <AnsaOspf6::Neighbor*> (timer->getContextPointer()))){
            // should not reach this point
            EV << "Discarding invalid NeighborInactivityTimer.\n";
            delete timer;
         } else{
            PrintEvent("Poll Timer expired", neighbor->GetInterface(), neighbor);
            //neighbor->ProcessEvent(AnsaOspf6::Neighbor::PollTimer);
         }}
         break;
      case NeighborDDRetransmissionTimer: {
         AnsaOspf6::Neighbor* neighbor;
         if (! (neighbor = reinterpret_cast <AnsaOspf6::Neighbor*> (timer->getContextPointer()))){
            // should not reach this point
            EV << "Discarding invalid NeighborDDRetransmissionTimer.\n";
            delete timer;
         } else{
            PrintEvent("Database Description Retransmission Timer expired", neighbor->GetInterface(), neighbor);
            //neighbor->ProcessEvent(AnsaOspf6::Neighbor::DDRetransmissionTimer);
         }}
         break;
      case NeighborUpdateRetransmissionTimer: {
         AnsaOspf6::Neighbor* neighbor;
         if (! (neighbor = reinterpret_cast <AnsaOspf6::Neighbor*> (timer->getContextPointer()))){
            // should not reach this point
            EV << "Discarding invalid NeighborUpdateRetransmissionTimer.\n";
            delete timer;
         } else{
            PrintEvent("Update Retransmission Timer expired", neighbor->GetInterface(), neighbor);
            //neighbor->ProcessEvent(AnsaOspf6::Neighbor::UpdateRetransmissionTimer);
         }}
         break;
      case NeighborRequestRetransmissionTimer: {
         AnsaOspf6::Neighbor* neighbor;
         if (! (neighbor = reinterpret_cast <AnsaOspf6::Neighbor*> (timer->getContextPointer()))){
            // should not reach this point
            EV << "Discarding invalid NeighborRequestRetransmissionTimer.\n";
            delete timer;
         } else{
            PrintEvent("Request Retransmission Timer expired", neighbor->GetInterface(), neighbor);
            //neighbor->ProcessEvent(AnsaOspf6::Neighbor::RequestRetransmissionTimer);
         }}
         break;
      case DatabaseAgeTimer: {
         PrintEvent("Ageing the database");
         router->AgeDatabase();
         }
         break;
      default:
         break;
   }
}

void AnsaOspf6::MessageHandler::ProcessPacket(OspfPacket6* packet, AnsaOspf6::Interface* unused1, AnsaOspf6::Neighbor* unused2){

   // there is no loop
   do {

      // The version number field MUST specify protocol version 3.
      if (packet->getVersion() != 3){
         break;
      }

      IPv6ControlInfo* controlInfo = check_and_cast<IPv6ControlInfo *> (packet->getControlInfo());
      int interfaceId = controlInfo->getInterfaceId();
      AnsaOspf6::AreaID areaID = packet->getAreaID();
      AnsaOspf6::InstanceID instanceID = packet->getInstanceID();
      AnsaOspf6::Area* area = router->GetArea(areaID);

      // The Area ID and Instance ID found in the OSPF header must be verified.
      if (area == NULL){
         break;
      }

      // TODO: check instance ID
      // if (instanceId != )

      AnsaOspf6::Interface* intf = area->GetInterface(interfaceId);

      if (intf != NULL && instanceID != intf->GetInstanceID()){
         break;
      }


      /* Match the backbone area and other criteria for a configured
       virtual link.  The receiving router must be an ABR (Area
       Border Router) and the Router ID specified in the packet (the
       source router) must be the other end of a configured virtual
       link.  Additionally, the receiving link must have an OSPFv3
       interface that attaches to the virtual link's configured
       transit area and the Instance ID must match the virtual link's
       Instance ID.
       */
      if (intf == NULL){
         if (areaID != BackboneAreaID ){
            break;
         }

         intf = area->FindVirtualLink(packet->getRouterID());
         if (intf == NULL){
            break;
         }

         AnsaOspf6::Area* virtualLinkTransitArea = router->GetArea(intf->GetTransitAreaID());
         if (virtualLinkTransitArea == NULL){
            intf = NULL;
         }else{
            // the receiving interface must attach to the virtual link's configured transit area
            AnsaOspf6::Interface* virtualLinkInterface = virtualLinkTransitArea->GetInterface(interfaceId);

            if (virtualLinkInterface == NULL) {
               intf = NULL;
            }
         }

      // intf != NULL
      }else{

         IPv6Address destinationAddress = controlInfo->getDestAddr();
         AnsaOspf6::Interface::InterfaceStateType interfaceState = intf->GetState();

         // if destination address is AllDRouters the receiving interface must be in DesignatedRouter or Backup state
         if (destinationAddress == AnsaOspf6::AllDRouters){
            if (  (interfaceState != AnsaOspf6::Interface::DesignatedRouterState)
               && (interfaceState != AnsaOspf6::Interface::BackupState)){
               break;
            }
         }


         OspfPacketType6 packetType = static_cast<OspfPacketType6> (packet->getType());
         AnsaOspf6::Neighbor* neighbor = NULL;

         // all packets except HelloPackets are sent only along adjacencies, so a Neighbor must exist
         if (packetType != HelloPacket){

            // On all link types (e.g., broadcast, NBMA, point-to-point, etc.),
            // neighbors are identified solely by their OSPF Router ID.
            neighbor = intf->GetNeighborByID(packet->getRouterID());
         }

         switch (packetType){
            case HelloPacket:
               helloHandler.ProcessPacket(packet, intf);
               router->stat.AddHelloPacketReceived();
               router->stat.AddOspfPacketReceived();
               break;
            case DatabaseDescriptionPacket:
               if (neighbor != NULL){
                  ddHandler.ProcessPacket(packet, intf, neighbor);
                  router->stat.AddOspfPacketReceived();
               }
               break;
            case LinkStateRequestPacket:
               if (neighbor != NULL){
                  lsRequestHandler.ProcessPacket(packet, intf, neighbor);
                  router->stat.AddOspfPacketReceived();
               }
               break;
            case LinkStateUpdatePacket:
               if (neighbor != NULL){
                  lsUpdateHandler.ProcessPacket(packet, intf, neighbor);
                  router->stat.AddOspfPacketReceived();
               }
               break;
            case LinkStateAcknowledgementPacket:
               if (neighbor != NULL){
                  lsAckHandler.ProcessPacket(packet, intf, neighbor);
                  router->stat.AddOspfPacketReceived();
               }
               break;
            default:
               break;
         }
      }

   // there is no loop
   } while(false);

   delete packet;
}

void AnsaOspf6::MessageHandler::SendPacket(OspfPacket6* packet, IPv6Address destination, int outputIfIndex, short hopLimit){

   IPv6ControlInfo *ipControlInfo = new IPv6ControlInfo();
   ipControlInfo->setProtocol(IP_PROT_OSPF);
   ipControlInfo->setDestAddr(destination);
   ipControlInfo->setHopLimit(hopLimit);
   ipControlInfo->setInterfaceId(outputIfIndex);
   packet->setControlInfo(ipControlInfo);

   switch (packet->getType()){
      case HelloPacket: {
         packet->setKind(HelloPacket);
         packet->setName("OSPFv3_HelloPacket");

         OspfHelloPacket6* helloPacket = check_and_cast<OspfHelloPacket6*> (packet);
         PrintHelloPacket(helloPacket, destination, outputIfIndex);

         router->stat.AddHelloPacketSend();
         }
         break;
      case DatabaseDescriptionPacket: {
         packet->setKind(DatabaseDescriptionPacket);
         packet->setName("OSPFv3_DDPacket");

         OspfDatabaseDescriptionPacket6* ddPacket = check_and_cast<OspfDatabaseDescriptionPacket6*> (packet);
         PrintDatabaseDescriptionPacket(ddPacket, destination, outputIfIndex);
         }
         break;
      case LinkStateRequestPacket: {
         packet->setKind(LinkStateRequestPacket);
         packet->setName("OSPF_LSReqPacket");

         OspfLinkStateRequestPacket6* requestPacket = check_and_cast<OspfLinkStateRequestPacket6*> (packet);
         PrintLinkStateRequestPacket(requestPacket, destination, outputIfIndex);
         }
         break;
      case LinkStateUpdatePacket: {
         packet->setKind(LinkStateUpdatePacket);
         packet->setName("OSPFv3_LSUpdPacket");

         OspfLinkStateUpdatePacket6* updatePacket = check_and_cast<OspfLinkStateUpdatePacket6*> (packet);
         PrintLinkStateUpdatePacket(updatePacket, destination, outputIfIndex);
         }
         break;
      case LinkStateAcknowledgementPacket: {
         packet->setKind(LinkStateAcknowledgementPacket);
         packet->setName("OSPFv3_LSAckPacket");

         OspfLinkStateAcknowledgementPacket6* ackPacket = check_and_cast<OspfLinkStateAcknowledgementPacket6*> (packet);
         PrintLinkStateAcknowledgementPacket(ackPacket, destination, outputIfIndex);
         }
         break;
      default:
         break;
   }

   ospfModule->send(packet, "ipv6Out");
   router->stat.AddOspfPacketSend();
}

void AnsaOspf6::MessageHandler::ClearTimer(OspfTimer6* timer) {
   if (ospfModule != NULL && timer != NULL){
      ospfModule->cancelEvent(timer);
   }
}

void AnsaOspf6::MessageHandler::StartTimer(OspfTimer6* timer, simtime_t delay) {
   ospfModule->scheduleAt(simTime() + delay, timer);
}

void AnsaOspf6::MessageHandler::PrintEvent(const char* eventString,
      const AnsaOspf6::Interface* onInterface, const AnsaOspf6::Neighbor* forNeighbor /*= NULL*/) const {

   EV<< eventString;
   if ((onInterface != NULL) || (forNeighbor != NULL)){
      EV << ": ";
   }
   if (forNeighbor != NULL){
      char addressString[16];
      EV << "neighbor["
      << IPAddress(forNeighbor->GetNeighborID())
      << "] (state: "
      << forNeighbor->GetStateString(forNeighbor->GetState())
      << "); ";
   }
   if (onInterface != NULL){
      EV << "interface["
      << static_cast <short> (onInterface->GetIfIndex())
      << "] ";
      switch (onInterface->GetType()){
         case AnsaOspf6::Interface::PointToPoint: EV << "(PointToPoint)";
         break;
         case AnsaOspf6::Interface::Broadcast: EV << "(Broadcast)";
         break;
         case AnsaOspf6::Interface::NBMA: EV << "(NBMA).\n";
         break;
         case AnsaOspf6::Interface::PointToMultiPoint: EV << "(PointToMultiPoint)";
         break;
         case AnsaOspf6::Interface::Virtual: EV << "(Virtual)";
         break;
         default: EV << "(Unknown)";
      }
      EV << " (state: "
      << onInterface->GetStateString(onInterface->GetState())
      << ")";
   }
   EV << ".\n";
}

void AnsaOspf6::MessageHandler::PrintHelloPacket(
      const OspfHelloPacket6* helloPacket,
      IPv6Address destination,
      int outputIfIndex) const {

   EV << "Sending Hello packet to " << destination
      << " via interface[" << outputIfIndex << "] with contents:" << endl;
   EV << "  interfaceID=" << helloPacket->getInterfaceID() << endl;
   EV << "  DR=" << IPAddress(helloPacket->getDesignatedRouterID()) << endl;
   EV << "  BDR=" << IPAddress(helloPacket->getBackupDesignatedRouterID()) << endl;
   EV << "  neighbors:" << endl;

   unsigned int neighborCount = helloPacket->getNeighborIDArraySize();
   for (unsigned int i = 0; i < neighborCount; i++) {
      EV << "    " << IPAddress(helloPacket->getNeighborID(i)) << endl;
   }
}

void AnsaOspf6::MessageHandler::PrintDatabaseDescriptionPacket(
      const OspfDatabaseDescriptionPacket6* ddPacket,
      IPv6Address destination,
      int outputIfIndex) const {

   EV << "Sending Database Description packet to " << destination
      << " via interface[" << outputIfIndex << "] with contents:" << endl;

   const OspfDdOptions6& ddOptions = ddPacket->getDdOptions();
   EV << "  ddOptions="
      << ((ddOptions.I_Init) ? "I " : "_ ")
      << ((ddOptions.M_More) ? "M " : "_ ")
      << ((ddOptions.MS_MasterSlave) ? "MS" : "__") << endl;

   EV << "  seqNumber="
      << ddPacket->getDdSequenceNumber() << endl;

   EV << "  LSA headers:" << endl;

   unsigned int lsaCount = ddPacket->getLsaHeadersArraySize();
   for (unsigned int i = 0; i < lsaCount; i++) {
      EV << "    ";
      PrintLsaHeader6(ddPacket->getLsaHeaders(i), ev.getOStream());
      EV << endl;
   }
}

void AnsaOspf6::MessageHandler::PrintLinkStateRequestPacket(
      const OspfLinkStateRequestPacket6* requestPacket,
      IPv6Address destination,
      int outputIfIndex) const {


   EV << "Sending Link State Request packet to " << destination
      << " via interface[" << outputIfIndex << "] with contents:" << endl;

   unsigned int requestCount = requestPacket->getRequestsArraySize();
   for (unsigned int i = 0; i < requestCount; i++){
      const OspfLsaRequest6& request = requestPacket->getRequests(i);
      EV << "  type=" << request.lsType << ", LSID=" << request.linkStateID;
      EV << ", advertisingRouter=" << IPAddress(request.advertisingRouter) << endl;
   }
}

void AnsaOspf6::MessageHandler::PrintLinkStateUpdatePacket(
      const OspfLinkStateUpdatePacket6* updatePacket,
      IPv6Address destination,
      int outputIfIndex) const {

   EV << "Sending Link State Update packet to " << destination
      << " via interface[" << outputIfIndex << "] with contents:" << endl;

   // RouterLSA
   unsigned int lsaCount = updatePacket->getRouterLsasArraySize();
   if (lsaCount > 0){
      ev << "RouterLsa (" << lsaCount << "):" << endl;
   }
   for (int i = 0; i < lsaCount; i++){
      const OspfRouterLsa6& lsa = updatePacket->getRouterLsas(i);
      ev << "  ";
      PrintLsaHeader6(lsa.getHeader(), ev.getOStream());
      ev << endl;
      PrintRouterLsa(lsa, ev.getOStream());
   }

   // NetworkLSA
   lsaCount = updatePacket->getNetworkLsasArraySize();
   if (lsaCount > 0){
      ev << "NetworkLsa (" << lsaCount << "):" << endl;
   }
   for (int i = 0; i < lsaCount; i++){
      const OspfNetworkLsa6& lsa = updatePacket->getNetworkLsas(i);
      ev << "  ";
      PrintLsaHeader6(lsa.getHeader(), ev.getOStream());
      ev << endl;
      PrintNetworkLsa(lsa, ev.getOStream());
   }

   // InterAreaPrefixLSA
   lsaCount = updatePacket->getInterAreaPrefixLsasArraySize();
   if (lsaCount > 0){
      ev << "InterAreaPrefixLsa (" << lsaCount << "):" << endl;
   }
   for (int i = 0; i < lsaCount; i++){
      const OspfInterAreaPrefixLsa6& lsa = updatePacket->getInterAreaPrefixLsas(i);
      ev << "  ";
      PrintLsaHeader6(lsa.getHeader(), ev.getOStream());
      ev << endl;
      PrintInterAreaPrefixLsa(lsa, ev.getOStream());
   }

   // InterAreaRouterLSA
   lsaCount = updatePacket->getInterAreaRouterLsasArraySize();
   if (lsaCount > 0){
      ev << "InterAreaRouterLsa (" << lsaCount << "):" << endl;
   }
   for (int i = 0; i < lsaCount; i++){
      const OspfInterAreaRouterLsa6& lsa = updatePacket->getInterAreaRouterLsas(i);
      ev << "  ";
      PrintLsaHeader6(lsa.getHeader(), ev.getOStream());
      ev << endl;
      PrintInterAreaRouterLsa(lsa, ev.getOStream());
   }

   // AsExternalLSA
   lsaCount = updatePacket->getAsExternalLsasArraySize();
   if (lsaCount > 0){
      ev << "AsExternalLsa (" << lsaCount << "):" << endl;
   }
   for (int i = 0; i < lsaCount; i++){
      const OspfAsExternalLsa6& lsa = updatePacket->getAsExternalLsas(i);
      ev << "  ";
      PrintLsaHeader6(lsa.getHeader(), ev.getOStream());
      ev << endl;
      PrintAsExternalLsa(lsa, ev.getOStream());
   }

   // LinkLSA
   lsaCount = updatePacket->getLinkLsasArraySize();
   if (lsaCount > 0){
      ev << "LinkLsa (" << lsaCount << "):" << endl;
   }
   for (int i = 0; i < lsaCount; i++){
      const OspfLinkLsa6& lsa = updatePacket->getLinkLsas(i);
      ev << "  ";
      PrintLsaHeader6(lsa.getHeader(), ev.getOStream());
      ev << endl;
      PrintLinkLsa(lsa, ev.getOStream());
   }

   // IntraAreaPrefix LSA
   lsaCount = updatePacket->getIntraAreaPrefixLsasArraySize();
   if (lsaCount > 0){
      ev << "IntraAreaPrefixLsa (" << lsaCount << "):" << endl;
   }
   for (int i = 0; i < lsaCount; i++){
      const OspfIntraAreaPrefixLsa6& lsa = updatePacket->getIntraAreaPrefixLsas(i);
      ev << "  ";
      PrintLsaHeader6(lsa.getHeader(), ev.getOStream());
      ev << endl;
      PrintIntraAreaPrefixLsa(lsa, ev.getOStream());
   }
}

void AnsaOspf6::MessageHandler::PrintLinkStateAcknowledgementPacket(
      const OspfLinkStateAcknowledgementPacket6* ackPacket,
      IPv6Address destination,
      int outputIfIndex) const {

   EV << "Sending Link State Acknowledgement packet to " << destination
      << " via interface[" << outputIfIndex << "] with contents:" << endl;

   unsigned int lsaCount = ackPacket->getLsaHeadersArraySize();
   for (unsigned int i = 0; i < lsaCount; i++) {
      EV << "    ";
      PrintLsaHeader6(ackPacket->getLsaHeaders(i), ev.getOStream());
      EV << "\n";
   }
}
