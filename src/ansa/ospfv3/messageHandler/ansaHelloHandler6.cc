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

#include "ansaOspfArea6.h"
#include "ansaOspfInterface6.h"
#include "ansaOspfNeighbor6.h"
#include "ansaOspfRouter6.h"

#include "ansaHelloHandler6.h"

AnsaOspf6::HelloHandler::HelloHandler(AnsaOspf6::Router* containingRouter) :
      AnsaOspf6::IMessageHandler(containingRouter) {
}

void AnsaOspf6::HelloHandler::ProcessPacket(OspfPacket6* packet, AnsaOspf6::Interface* intf, AnsaOspf6::Neighbor* unused) {

   OspfHelloPacket6* helloPacket = check_and_cast<OspfHelloPacket6*> (packet);
   bool rebuildRoutingTable = false;

   /* The values of the HelloInterval and RouterDeadInterval fields in the received
    * Hello packet must be checked against the values configured for the receiving
      interface. Any mismatch causes processing to stop and the packet to be dropped.
    */
   if (intf->GetHelloInterval() != helloPacket->getHelloInterval()){
      ev << "Hello interval mismatch" << endl;
      return;
   }

   if (intf->GetRouterDeadInterval() != helloPacket->getRouterDeadInterval()){
      ev << "Router dead interval mismatch" << endl;
      return;
   }

   AnsaOspf6::Interface::OspfInterfaceType interfaceType = intf->GetType();

   /* The setting of the E-bit found in the Hello Packet's Options field
    * must match this area's ExternalRoutingCapability.
    */
   if (intf->GetArea()->GetExternalRoutingCapability() != helloPacket->getOptions().E_ExternalRoutingCapability){
      ev << "External routing capability mismatch" << endl;
      return;
   }

   IPv6ControlInfo *controlInfo = check_and_cast<IPv6ControlInfo *> (helloPacket->getControlInfo());
   IPv6Address srcAddress = controlInfo->getSrcAddr();
   bool neighborChanged = false;
   bool neighborsDRStateChanged = false;
   bool backupSeen = false;
   AnsaOspf6::Neighbor* neighbor;

   /* On all link types (e.g., broadcast, NBMA, point-to-point, etc.),
      neighbors are identified solely by their OSPF Router ID.
   */
   neighbor = intf->GetNeighborByID(helloPacket->getRouterID());

   // neighbor is known
   if (neighbor != NULL){

      router->GetMessageHandler()->PrintEvent("Hello packet received", intf, neighbor);

      char newPriority = helloPacket->getRtrPriority();
      DesignatedRouterID newDesignatedRouter = helloPacket->getDesignatedRouterID();
      DesignatedRouterID newBackupRouter = helloPacket->getBackupDesignatedRouterID();

      if (  (interfaceType == AnsaOspf6::Interface::Virtual)
         && (neighbor->GetState() == AnsaOspf6::Neighbor::DownState)){
         neighbor->SetPriority(helloPacket->getRtrPriority());
         neighbor->SetRouterDeadInterval(helloPacket->getRouterDeadInterval());
      }

      /* If a change in the neighbor's Router Priority field
       was noted, the receiving interface's state machine is
       scheduled with the event NeighborChange.
       */
      if (neighbor->GetPriority() != newPriority){
         neighborChanged = true;
      }

      /* If the neighbor is both declaring itself to be Designated
       Router(Hello Packet's Designated Router field = Neighbor IP
       address) and the Backup Designated Router field in the
       packet is equal to 0.0.0.0 and the receiving interface is in
       state Waiting, the receiving interface's state machine is
       scheduled with the event BackupSeen.
       */
      if (  (newDesignatedRouter == neighbor->GetNeighborID())
         && (newBackupRouter == 0)
         && (intf->GetState() == AnsaOspf6::Interface::WaitingState)){
         backupSeen = true;

      /* Otherwise, if the neighbor is declaring itself to be Designated Router and it
       had not previously, or the neighbor is not declaring itself
       Designated Router where it had previously, the receiving
       interface's state machine is scheduled with the event
       NeighborChange.
       */
      }else if (  (  (newDesignatedRouter == neighbor->GetNeighborID())
                  && (newDesignatedRouter != neighbor->GetDesignatedRouter()))
               || (  (newDesignatedRouter != neighbor->GetNeighborID())
                  && (neighbor->GetNeighborID() == neighbor->GetDesignatedRouter()))){
         neighborChanged = true;
         neighborsDRStateChanged = true;
      }


      /* If the neighbor is declaring itself to be Backup Designated
       Router(Hello Packet's Backup Designated Router field =
       Neighbor IP address) and the receiving interface is in state
       Waiting, the receiving interface's state machine is
       scheduled with the event BackupSeen.
       */
      if (  (newBackupRouter == neighbor->GetNeighborID())
         && (intf->GetState() == AnsaOspf6::Interface::WaitingState)){
         backupSeen = true;

      /* Otherwise, if the neighbor is declaring itself to be Backup Designated Router
       and it had not previously, or the neighbor is not declaring
       itself Backup Designated Router where it had previously, the
       receiving interface's state machine is scheduled with the
       event NeighborChange.
       */
      }else if (  (  (newBackupRouter == neighbor->GetNeighborID())
                  && (newBackupRouter != neighbor->GetBackupDesignatedRouter()))
               || (  (newBackupRouter != neighbor->GetNeighborID())
                  && (neighbor->GetNeighborID() == neighbor->GetBackupDesignatedRouter()))){
         neighborChanged = true;
      }

      neighbor->SetPriority(newPriority);
      neighbor->SetAddress(srcAddress);
      if (newDesignatedRouter != neighbor->GetDesignatedRouter()){
         neighbor->SetDesignatedRouter(newDesignatedRouter);
      }

      if (newBackupRouter != neighbor->GetBackupDesignatedRouter()){
         neighbor->SetBackupDesignatedRouter(newBackupRouter);
      }


   // neighbor is unknown
   }else{
      AnsaOspf6::DesignatedRouterID dRouterID;

      neighbor = new AnsaOspf6::Neighbor(helloPacket->getRouterID());
      neighbor->SetPriority(helloPacket->getRtrPriority());
      neighbor->SetAddress(srcAddress);
      neighbor->SetRouterDeadInterval(helloPacket->getRouterDeadInterval());

      router->GetMessageHandler()->PrintEvent("Hello packet received", intf, neighbor);

      DesignatedRouterID newDesignatedRouter = helloPacket->getDesignatedRouterID();
      DesignatedRouterID newBackupRouter = helloPacket->getBackupDesignatedRouterID();

      neighbor->SetDesignatedRouter(newDesignatedRouter);
      neighbor->SetBackupDesignatedRouter(newBackupRouter);

      intf->AddNeighbor(neighbor);
   }

   neighbor->ProcessEvent(AnsaOspf6::Neighbor::HelloReceived);
   if (  (interfaceType == AnsaOspf6::Interface::NBMA)
      && (intf->GetRouterPriority() == 0)
      && (neighbor->GetState() >= AnsaOspf6::Neighbor::InitState)){
      intf->SendHelloPacket(neighbor->GetAddress());
   }

   unsigned int neighborsNeighborCount = helloPacket->getNeighborIDArraySize();
   unsigned int i;
   /* The list of neighbors contained in the Hello Packet is
    examined.  If the router itself appears in this list, the
    neighbor state machine should be executed with the event TwoWayReceived.
    */
   for (i = 0; i < neighborsNeighborCount; i++){
      if (helloPacket->getNeighborID(i) == router->GetRouterID()){
         neighbor->ProcessEvent(AnsaOspf6::Neighbor::TwoWayReceived);
         break;
      }
   }
   /* Otherwise, the neighbor state machine should
    be executed with the event OneWayReceived, and the processing
    of the packet stops.
    */
   if (i == neighborsNeighborCount){
      neighbor->ProcessEvent(AnsaOspf6::Neighbor::OneWayReceived);
   }


   /* In some cases neighbors get stuck in TwoWay state after a DR
    or Backup change. (CalculateDesignatedRouter runs before the
    neighbors' signal of DR change + this router does not become
    neither DR nor backup -> IsAdjacencyOK does not get called.)
    So to make it work(workaround) we'll call IsAdjacencyOK for
    all neighbors in TwoWay state from here. This shouldn't break
    anything because if the neighbor state doesn't have to change
    then NeedAdjacency returns false and nothing happnes in
    IsAdjacencyOK.
    */
   if (neighborChanged){
      intf->ProcessEvent(AnsaOspf6::Interface::NeighborChange);

      unsigned int neighborCount = intf->GetNeighborCount();
      for (i = 0; i < neighborCount; i++){
         AnsaOspf6::Neighbor* stuckNeighbor = intf->GetNeighbor(i);
         if (stuckNeighbor->GetState() == AnsaOspf6::Neighbor::TwoWayState){
            stuckNeighbor->ProcessEvent(AnsaOspf6::Neighbor::IsAdjacencyOK);
         }
      }

      /* TODO:
      if (neighborsDRStateChanged){
         AnsaOspf6::RouterLsa* routerLSA = intf->GetArea()->FindRouterLSA(router->GetRouterID());

         if (routerLSA != NULL){
            long sequenceNumber = routerLSA->getHeader().getLsSequenceNumber();
            if (sequenceNumber == MAX_SEQUENCE_NUMBER){
               routerLSA->getHeader().setLsAge(MAX_AGE);
               intf->GetArea()->FloodLSA(routerLSA);
               routerLSA->IncrementInstallTime();
            }else{
               AnsaOspf6::RouterLsa *newLSA = intf->GetArea()->OriginateRouterLSA();

               newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
               rebuildRoutingTable |= routerLSA->Update(newLSA);
               delete newLSA;

               intf->GetArea()->FloodLSA(routerLSA);
            }
         }
      }
      */
   }

   if (backupSeen){
      intf->ProcessEvent(AnsaOspf6::Interface::BackupSeen);
   }

   if (rebuildRoutingTable){
      router->RebuildRoutingTable();
   }
}
