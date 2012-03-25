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

#include "ansaOspfInterface6.h"
#include "ansaOspfInterfaceState6.h"
#include "ansaOspfInterfaceStateBackup6.h"
#include "ansaOspfInterfaceStateDesignatedRouter6.h"
#include "ansaOspfInterfaceStateNotDesignatedRouter6.h"

void AnsaOspf6::InterfaceState::ChangeState(AnsaOspf6::Interface* intf, AnsaOspf6::InterfaceState* newState, AnsaOspf6::InterfaceState* currentState){

   ev << "Interface " << intf->interfaceAddress << " is changing state to " << newState->GetState() << endl;

   AnsaOspf6::Interface::InterfaceStateType oldState  = currentState->GetState();
   AnsaOspf6::Interface::InterfaceStateType nextState = newState->GetState();
   AnsaOspf6::Interface::OspfInterfaceType  intfType  = intf->GetType();
   bool rebuildRoutingTable = false;

   intf->ChangeState(newState, currentState);

   /* TODO:
   if (  (oldState == AnsaOspf6::Interface::DownState) ||
         (nextState == AnsaOspf6::Interface::DownState) ||
         (oldState == AnsaOspf6::Interface::LoopbackState) ||
         (nextState == AnsaOspf6::Interface::LoopbackState) ||
         (oldState == AnsaOspf6::Interface::DesignatedRouterState) ||
         (nextState == AnsaOspf6::Interface::DesignatedRouterState)
      || (  (intfType == AnsaOspf6::Interface::PointToPoint)
         && (  (oldState == AnsaOspf6::Interface::PointToPointState) ||
               (nextState == AnsaOspf6::Interface::PointToPointState)))
      || (  (  (intfType == AnsaOspf6::Interface::Broadcast) ||
               (intfType == AnsaOspf6::Interface::NBMA))
         && (  (oldState == AnsaOspf6::Interface::WaitingState) ||
               (nextState == AnsaOspf6::Interface::WaitingState)))){

      AnsaOspf6::RouterLsa* routerLSA = intf->GetArea()->FindRouterLSA(intf->GetArea()->GetRouter()->GetRouterID());

      if (routerLSA != NULL){
         long sequenceNumber = routerLSA->getHeader().getLsSequenceNumber();
         if (sequenceNumber == MAX_SEQUENCE_NUMBER){
            routerLSA->getHeader().setLsAge(MAX_AGE);
            intf->GetArea()->FloodLSA(routerLSA);
            routerLSA->IncrementInstallTime();
         }else{
            AnsaOspf6::RouterLsa* newLSA = intf->GetArea()->OriginateRouterLSA();

            newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
            rebuildRoutingTable |= routerLSA->Update(newLSA);
            delete newLSA;

            intf->GetArea()->FloodLSA(routerLSA);
         }

      }else{  // (lsa == NULL) -> This must be the first time any interface is up...
         AnsaOspf6::RouterLsa* newLSA = intf->GetArea()->OriginateRouterLSA();

         rebuildRoutingTable |= intf->GetArea()->InstallRouterLSA(newLSA);

         routerLSA = intf->GetArea()->FindRouterLSA(intf->GetArea()->GetRouter()->GetRouterID());

         intf->GetArea()->SetSPFTreeRoot(routerLSA);
         intf->GetArea()->FloodLSA(newLSA);
         delete newLSA;
     }
   }


   if (nextState == AnsaOspf6::Interface::DesignatedRouterState) {
      AnsaOspf6::NetworkLsa* newLSA = intf->GetArea()->OriginateNetworkLSA(intf);
      if (newLSA != NULL){
         rebuildRoutingTable |= intf->GetArea()->InstallNetworkLSA(newLSA);

         intf->GetArea()->FloodLSA(newLSA);
         delete newLSA;

      }else{    // no neighbors on the network -> old NetworkLSA must be flushed
         AnsaOspf6::NetworkLSA* oldLSA = intf->GetArea()->FindNetworkLSA(ULongFromIPv4Address(intf->GetAddressRange().address));

         if (oldLSA != NULL){
            oldLSA->getHeader().setLsAge(MAX_AGE);
            intf->GetArea()->FloodLSA(oldLSA);
            oldLSA->IncrementInstallTime();
         }
      }
   }

   if (oldState == AnsaOspf6::Interface::DesignatedRouterState) {
      AnsaOspf6::NetworkLSA* networkLSA = intf->GetArea()->FindNetworkLSA(ULongFromIPv4Address(intf->GetAddressRange().address));

      if (networkLSA != NULL){
         networkLSA->getHeader().setLsAge(MAX_AGE);
         intf->GetArea()->FloodLSA(networkLSA);
         networkLSA->IncrementInstallTime();
      }
   }

   if (rebuildRoutingTable) {
      intf->GetArea()->GetRouter()->RebuildRoutingTable();
   }
   */
}



void AnsaOspf6::InterfaceState::CalculateDesignatedRouter(AnsaOspf6::Interface* intf){

   AnsaOspf6::RouterID           routerID                = intf->parentArea->GetRouter()->GetRouterID();
   AnsaOspf6::DesignatedRouterID currentDesignatedRouter = intf->designatedRouter;
   AnsaOspf6::DesignatedRouterID currentBackupRouter     = intf->backupDesignatedRouter;

   unsigned int             neighborCount           = intf->neighboringRouters.size();
   unsigned char            repeatCount             = 0;
   unsigned int             i;

   AnsaOspf6::DesignatedRouterID declaredBackup;
   unsigned char                 declaredBackupPriority;
   AnsaOspf6::RouterID           declaredBackupID;
   bool                          backupDeclared;

   AnsaOspf6::DesignatedRouterID declaredDesignatedRouter;
   unsigned char                 declaredDesignatedRouterPriority;
   AnsaOspf6::RouterID           declaredDesignatedRouterID;
   bool                          designatedRouterDeclared;

   do {
      // calculating backup designated router
      declaredBackup = AnsaOspf6::NullDesignatedRouterID;
      declaredBackupPriority = 0;
      declaredBackupID = AnsaOspf6::NullRouterID;
      backupDeclared = false;

      AnsaOspf6::DesignatedRouterID highestRouter     = AnsaOspf6::NullDesignatedRouterID;
      unsigned char                 highestPriority   = 0;
      AnsaOspf6::RouterID           highestID         = AnsaOspf6::NullRouterID;

      for (i = 0; i < neighborCount; i++){
         AnsaOspf6::Neighbor* neighbor          = intf->neighboringRouters[i];
         unsigned char        neighborPriority  = neighbor->GetPriority();

         if (neighbor->GetState() < AnsaOspf6::Neighbor::TwoWayState){
            continue;
         }

         if (neighborPriority == 0) {
            continue;
         }

         AnsaOspf6::RouterID           neighborID                      = neighbor->GetNeighborID();
         AnsaOspf6::DesignatedRouterID neighborsDesignatedRouter       = neighbor->GetDesignatedRouter();
         AnsaOspf6::DesignatedRouterID neighborsBackupDesignatedRouter = neighbor->GetBackupDesignatedRouter();

         if (neighborsDesignatedRouter != neighborID){
            if (neighborsBackupDesignatedRouter == neighborID) {
               if (  (neighborPriority > declaredBackupPriority) ||
                     ((neighborPriority == declaredBackupPriority) &&
                           (neighborID > declaredBackupID))){

                  declaredBackup = neighborsBackupDesignatedRouter;
                  declaredBackupPriority = neighborPriority;
                  declaredBackupID = neighborID;
                  backupDeclared = true;
               }
            }

            if (!backupDeclared){
               if (  (neighborPriority > highestPriority) ||
                     ((neighborPriority == highestPriority) &&
                           (neighborID > highestID))){

                  highestRouter = neighborID;
                  highestPriority = neighborPriority;
                  highestID = neighborID;
               }
            }
         }
      }

      // also include the router itself in the calculations
      if (intf->routerPriority > 0) {
         if (currentDesignatedRouter != routerID){
            if (currentBackupRouter == routerID){
               if (  (intf->routerPriority > declaredBackupPriority) ||
                     ((intf->routerPriority == declaredBackupPriority) &&
                           (routerID > declaredBackupID))){

                  declaredBackup = routerID;
                  declaredBackupPriority = intf->routerPriority;
                  declaredBackupID = routerID;
                  backupDeclared = true;
               }

            }

            if (!backupDeclared){
               if ((intf->routerPriority > highestPriority) ||
                     ((intf->routerPriority == highestPriority) &&
                           (routerID > highestID))){

                  declaredBackup = routerID;
                  declaredBackupPriority = intf->routerPriority;
                  declaredBackupID = routerID;
                  backupDeclared = true;

               }else{
                  declaredBackup = highestRouter;
                  declaredBackupPriority = highestPriority;
                  declaredBackupID = highestID;
                  backupDeclared = true;
               }
            }
         }
      }

      // calculating designated router
      declaredDesignatedRouter = AnsaOspf6::NullDesignatedRouterID;
      declaredDesignatedRouterPriority = 0;
      declaredDesignatedRouterID = AnsaOspf6::NullRouterID;
      designatedRouterDeclared = false;

      for (i = 0; i < neighborCount; i++){
         AnsaOspf6::Neighbor* neighbor         = intf->neighboringRouters[i];
         unsigned char        neighborPriority = neighbor->GetPriority();

         if (neighbor->GetState() < AnsaOspf6::Neighbor::TwoWayState){
            continue;
         }

         if (neighborPriority == 0){
            continue;
         }

         AnsaOspf6::RouterID           neighborID                      = neighbor->GetNeighborID();
         AnsaOspf6::DesignatedRouterID neighborsDesignatedRouter       = neighbor->GetDesignatedRouter();
         AnsaOspf6::DesignatedRouterID neighborsBackupDesignatedRouter = neighbor->GetBackupDesignatedRouter();

         if (neighborsDesignatedRouter == neighborID) {
            if (  (neighborPriority > declaredDesignatedRouterPriority) ||
                  ((neighborPriority == declaredDesignatedRouterPriority) &&
                        (neighborID > declaredDesignatedRouterID))){

               declaredDesignatedRouter = neighborsDesignatedRouter;
               declaredDesignatedRouterPriority = neighborPriority;
               declaredDesignatedRouterID = neighborID;
               designatedRouterDeclared = true;
            }
         }
      }


      // also include the router itself in the calculations
      if (intf->routerPriority > 0){
         if (currentDesignatedRouter == routerID){
            if (  (intf->routerPriority > declaredDesignatedRouterPriority) ||
                  ((intf->routerPriority == declaredDesignatedRouterPriority) &&
                        (routerID > declaredDesignatedRouterID))){

               declaredDesignatedRouter = routerID;
               declaredDesignatedRouterPriority = intf->routerPriority;
               declaredDesignatedRouterID = routerID;
               designatedRouterDeclared = true;
            }
         }
      }

      if (!designatedRouterDeclared){
         declaredDesignatedRouter = declaredBackup;
         declaredDesignatedRouterPriority = declaredBackupPriority;
         declaredDesignatedRouterID = declaredBackupID;
         designatedRouterDeclared = true;
      }

      // if the router is any kind of DR or is no longer one of them, then repeat
      if ( ((declaredDesignatedRouter != AnsaOspf6::NullRouterID) &&
               ((currentDesignatedRouter == routerID) && (declaredDesignatedRouter != routerID))
            || ((currentDesignatedRouter != routerID) && (declaredDesignatedRouter == routerID)))
        || ((declaredBackup != AnsaOspf6::NullRouterID) &&
              ((currentBackupRouter == routerID) && (declaredBackup != routerID))
           || ((currentBackupRouter != routerID) && (declaredBackup == routerID)))){

         currentDesignatedRouter = declaredDesignatedRouter;
         currentBackupRouter = declaredBackup;
         repeatCount++;

      } else {
         repeatCount += 2;
      }

   } while (repeatCount < 2);



   AnsaOspf6::RouterID routersOldDesignatedRouterID = intf->designatedRouter;
   AnsaOspf6::RouterID routersOldBackupID           = intf->backupDesignatedRouter;

   intf->designatedRouter = declaredDesignatedRouter;
   intf->backupDesignatedRouter = declaredBackup;

   bool wasBackupDesignatedRouter = (routersOldBackupID == routerID);
   bool wasDesignatedRouter       = (routersOldDesignatedRouterID == routerID);
   bool wasOther                  = (intf->GetState() == AnsaOspf6::Interface::NotDesignatedRouterState);
   bool wasWaiting                = (!wasBackupDesignatedRouter && !wasDesignatedRouter && !wasOther);
   bool isBackupDesignatedRouter  = (declaredBackup == routerID);
   bool isDesignatedRouter        = (declaredDesignatedRouter == routerID);
   bool isOther                   = (!isBackupDesignatedRouter && !isDesignatedRouter);

   if (wasBackupDesignatedRouter) {
      if (isDesignatedRouter){
         ChangeState(intf, new AnsaOspf6::InterfaceStateDesignatedRouter, this);
      }

      if (isOther){
         ChangeState(intf, new AnsaOspf6::InterfaceStateNotDesignatedRouter, this);
      }
   }

   if (wasDesignatedRouter){
      if (isBackupDesignatedRouter){
         ChangeState(intf, new AnsaOspf6::InterfaceStateBackup, this);
      }

      if (isOther){
         ChangeState(intf, new AnsaOspf6::InterfaceStateNotDesignatedRouter, this);
      }
   }

   if (wasOther){
      if (isDesignatedRouter){
         ChangeState(intf, new AnsaOspf6::InterfaceStateDesignatedRouter, this);
      }

      if (isBackupDesignatedRouter){
         ChangeState(intf, new AnsaOspf6::InterfaceStateBackup, this);
      }
   }

   if (wasWaiting){
      if (isDesignatedRouter){
         ChangeState(intf, new AnsaOspf6::InterfaceStateDesignatedRouter, this);
      }

      if (isBackupDesignatedRouter){
         ChangeState(intf, new AnsaOspf6::InterfaceStateBackup, this);
      }

      if (isOther){
         ChangeState(intf, new AnsaOspf6::InterfaceStateNotDesignatedRouter, this);
      }
   }

   for (i = 0; i < neighborCount; i++) {
      if (  (intf->interfaceType == AnsaOspf6::Interface::NBMA) &&
            (
                  (!wasBackupDesignatedRouter && isBackupDesignatedRouter) ||
                  (!wasDesignatedRouter && isDesignatedRouter)
            )
         ){

         if (intf->neighboringRouters[i]->GetPriority() == 0){
            intf->neighboringRouters[i]->ProcessEvent(AnsaOspf6::Neighbor::Start);
         }
      }

      if (  (declaredDesignatedRouter != routersOldDesignatedRouterID) ||
            (declaredBackup != routersOldBackupID)){

         if (intf->neighboringRouters[i]->GetState() >= AnsaOspf6::Neighbor::TwoWayState){
            intf->neighboringRouters[i]->ProcessEvent(AnsaOspf6::Neighbor::IsAdjacencyOK);
         }
      }
   }
}
