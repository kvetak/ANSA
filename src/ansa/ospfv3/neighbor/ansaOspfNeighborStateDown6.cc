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
#include "ansaMessageHandler6.h"

#include "ansaOspfNeighborStateAttempt6.h"
#include "ansaOspfNeighborStateDown6.h"
#include "ansaOspfNeighborStateInit6.h"

void AnsaOspf6::NeighborStateDown::ProcessEvent(AnsaOspf6::Neighbor* neighbor,
      AnsaOspf6::Neighbor::NeighborEventType event) {

   if (event == AnsaOspf6::Neighbor::Start){
      MessageHandler* messageHandler =
            neighbor->GetInterface()->GetArea()->GetRouter()->GetMessageHandler();
      int ttl = (neighbor->GetInterface()->GetType() == AnsaOspf6::Interface::Virtual)
            ? VIRTUAL_LINK_TTL : 1;

      messageHandler->ClearTimer(neighbor->GetPollTimer());
      neighbor->GetInterface()->SendHelloPacket(neighbor->GetAddress(), ttl);
      messageHandler->StartTimer(neighbor->GetInactivityTimer(), neighbor->GetRouterDeadInterval());
      ChangeState(neighbor, new AnsaOspf6::NeighborStateAttempt, this);
   }
   if (event == AnsaOspf6::Neighbor::HelloReceived){
      MessageHandler* messageHandler = neighbor->GetInterface()->GetArea()->GetRouter()->GetMessageHandler();
      messageHandler->ClearTimer(neighbor->GetPollTimer());
      messageHandler->StartTimer(neighbor->GetInactivityTimer(), neighbor->GetRouterDeadInterval());
      ChangeState(neighbor, new AnsaOspf6::NeighborStateInit, this);
   }
   if (event == AnsaOspf6::Neighbor::PollTimer){
      int ttl = (neighbor->GetInterface()->GetType() == AnsaOspf6::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;
      neighbor->GetInterface()->SendHelloPacket(neighbor->GetAddress(), ttl);
      MessageHandler* messageHandler = neighbor->GetInterface()->GetArea()->GetRouter()->GetMessageHandler();
      messageHandler->StartTimer(neighbor->GetPollTimer(), neighbor->GetInterface()->GetPollInterval());
   }
}
