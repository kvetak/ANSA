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

#include "ansaOspfInterfaceStateDown6.h"
#include "ansaOspfInterfaceStateLoopback6.h"
#include "ansaOspfInterfaceStateNotDesignatedRouter6.h"
#include "ansaOspfInterfaceStatePointToPoint6.h"
#include "ansaOspfInterfaceStateWaiting6.h"

void AnsaOspf6::InterfaceStateDown::ProcessEvent(AnsaOspf6::Interface* intf, AnsaOspf6::Interface::InterfaceEventType event){

   if (event == AnsaOspf6::Interface::InterfaceUp){

      AnsaOspf6::MessageHandler* messageHandler = intf->GetArea()->GetRouter()->GetMessageHandler();

      // add some deviation to avoid startup collisions
      messageHandler->StartTimer(intf->GetHelloTimer(), truncnormal(0.1, 0.01));
      /* TODO
      messageHandler->StartTimer(intf->GetAcknowledgementTimer(), intf->GetAcknowledgementDelay());
      */

      switch (intf->GetType()){
         case AnsaOspf6::Interface::PointToPoint:
         case AnsaOspf6::Interface::PointToMultiPoint:
         case AnsaOspf6::Interface::Virtual:
            ChangeState(intf, new AnsaOspf6::InterfaceStatePointToPoint, this);
            break;

         case AnsaOspf6::Interface::NBMA:
            if (intf->GetRouterPriority() == 0){
               ChangeState(intf, new AnsaOspf6::InterfaceStateNotDesignatedRouter, this);
            }else{
               ChangeState(intf, new AnsaOspf6::InterfaceStateWaiting, this);
               messageHandler->StartTimer(intf->GetWaitTimer(), intf->GetRouterDeadInterval());

               long neighborCount = intf->GetNeighborCount();
               for (long i = 0; i < neighborCount; i++) {
                  AnsaOspf6::Neighbor* neighbor = intf->GetNeighbor(i);
                  if (neighbor->GetPriority() > 0) {
                     neighbor->ProcessEvent(AnsaOspf6::Neighbor::Start);
                  }
               }
            }
            break;

         case AnsaOspf6::Interface::Broadcast:
            if (intf->GetRouterPriority() == 0){
               ChangeState(intf, new AnsaOspf6::InterfaceStateNotDesignatedRouter, this);
            }else{
               ChangeState(intf, new AnsaOspf6::InterfaceStateWaiting, this);
               messageHandler->StartTimer(intf->GetWaitTimer(), intf->GetRouterDeadInterval());
            }
            break;

         default:
            break;
      }
   }

   if (event == AnsaOspf6::Interface::LoopIndication) {
      intf->Reset();
      ChangeState(intf, new AnsaOspf6::InterfaceStateLoopback, this);
   }
}

