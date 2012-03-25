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
#include "ansaOspfRouter6.h"

#include "ansaOspfNeighborState6.h"


void AnsaOspf6::NeighborState::ChangeState(AnsaOspf6::Neighbor* neighbor,
      AnsaOspf6::NeighborState* newState, AnsaOspf6::NeighborState* currentState) {

   AnsaOspf6::Neighbor::NeighborStateType oldState = currentState->GetState();
   AnsaOspf6::Neighbor::NeighborStateType nextState = newState->GetState();
   bool rebuildRoutingTable = false;

   neighbor->ChangeState(newState, currentState);

   /* TODO:
   if (((oldState == AnsaOspf6::Neighbor::FullState) || (nextState == AnsaOspf6::Neighbor::FullState))
         && !(neighbor->GetInterface()->IsGoingDown())){
      AnsaOspf6::RouterID routerID = neighbor->GetInterface()->GetArea()->GetRouter()->GetRouterID();
      AnsaOspf6::RouterLsa* routerLSA = neighbor->GetInterface()->GetArea()->FindRouterLSA(routerID);

      if (routerLSA != NULL){
         long sequenceNumber = routerLSA->getHeader().getLsSequenceNumber();
         if (sequenceNumber == MAX_SEQUENCE_NUMBER){
            routerLSA->getHeader().setLsAge(MAX_AGE);
            neighbor->GetInterface()->GetArea()->FloodLSA(routerLSA);
            routerLSA->IncrementInstallTime();
         }else{
            AnsaOspf6::RouterLsa* newLSA = neighbor->GetInterface()->GetArea()->OriginateRouterLSA();

            newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
            rebuildRoutingTable |= routerLSA->Update(newLSA);
            delete newLSA;

            neighbor->GetInterface()->GetArea()->FloodLSA(routerLSA);
         }
      }

      if (neighbor->GetInterface()->GetState() == AnsaOspf6::Interface::DesignatedRouterState){
         AnsaOspf6::NetworkLSA* networkLSA = neighbor->GetInterface()->GetArea()->FindNetworkLSA(
               ULongFromIPv4Address(neighbor->GetInterface()->GetAddressRange().address));

         if (networkLSA != NULL){
            long sequenceNumber = networkLSA->getHeader().getLsSequenceNumber();
            if (sequenceNumber == MAX_SEQUENCE_NUMBER){
               networkLSA->getHeader().setLsAge(MAX_AGE);
               neighbor->GetInterface()->GetArea()->FloodLSA(networkLSA);
               networkLSA->IncrementInstallTime();
            }else{
               AnsaOspf6::NetworkLsa* newLSA =
                     neighbor->GetInterface()->GetArea()->OriginateNetworkLSA(
                           neighbor->GetInterface());

               if (newLSA != NULL){
                  newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                  rebuildRoutingTable |= networkLSA->Update(newLSA);
                  delete newLSA;
               }else{ // no neighbors on the network -> old NetworkLSA must be flushed
                  networkLSA->getHeader().setLsAge(MAX_AGE);
                  networkLSA->IncrementInstallTime();
               }

               neighbor->GetInterface()->GetArea()->FloodLSA(networkLSA);
            }
         }
      }
   }

   if (rebuildRoutingTable){
      neighbor->GetInterface()->GetArea()->GetRouter()->RebuildRoutingTable();
   }
   */
}
