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

#include "ansaOspfInterfaceStateBackup6.h"
#include "ansaOspfInterfaceStateDesignatedRouter6.h"
#include "ansaOspfInterfaceStateDown6.h"
#include "ansaOspfInterfaceStateLoopback6.h"
#include "ansaOspfInterfaceStateNotDesignatedRouter6.h"

void AnsaOspf6::InterfaceStateNotDesignatedRouter::ProcessEvent(AnsaOspf6::Interface* intf, AnsaOspf6::Interface::InterfaceEventType event){

   if (event == AnsaOspf6::Interface::NeighborChange){
      CalculateDesignatedRouter(intf);
   }

   if (event == AnsaOspf6::Interface::InterfaceDown){
      intf->SetIsGoingDown(true);
      intf->Reset();
      ChangeState(intf, new AnsaOspf6::InterfaceStateDown, this);
      intf->SetIsGoingDown(false);
   }

   if (event == AnsaOspf6::Interface::LoopIndication){
      intf->Reset();
      ChangeState(intf, new AnsaOspf6::InterfaceStateLoopback, this);
   }

   if (event == AnsaOspf6::Interface::HelloTimer){
      if (intf->GetType() == AnsaOspf6::Interface::Broadcast){
         intf->SendHelloPacket(AnsaOspf6::AllSPFRouters);
      }else{    // AnsaOspf6::Interface::NBMA
         if (intf->GetRouterPriority() > 0){
            unsigned long neighborCount = intf->GetNeighborCount();
            for (unsigned long i = 0; i < neighborCount; i++) {
               const AnsaOspf6::Neighbor* neighbor = intf->GetNeighbor(i);
               if (neighbor->GetPriority() > 0) {
                  intf->SendHelloPacket(neighbor->GetAddress());
               }
            }
         }else{
           intf->SendHelloPacket(AnsaOspf6::AllDRouters);
           intf->SendHelloPacket(AnsaOspf6::AllDRouters);
         }
      }
      intf->GetArea()->GetRouter()->GetMessageHandler()->StartTimer(intf->GetHelloTimer(), intf->GetHelloInterval());
   }

   if (event == AnsaOspf6::Interface::AcknowledgementTimer) {
      intf->SendDelayedAcknowledgements();
   }
}
