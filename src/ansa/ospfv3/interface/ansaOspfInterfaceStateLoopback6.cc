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

#include "ansaOspfInterfaceStateDown6.h"
#include "ansaOspfInterfaceStateLoopback6.h"

void AnsaOspf6::InterfaceStateLoopback::ProcessEvent(AnsaOspf6::Interface* intf, AnsaOspf6::Interface::InterfaceEventType event){

   if (event == AnsaOspf6::Interface::InterfaceDown) {
      intf->Reset();
      ChangeState(intf, new AnsaOspf6::InterfaceStateDown, this);
   }

   if (event == AnsaOspf6::Interface::UnloopIndication) {
      ChangeState(intf, new AnsaOspf6::InterfaceStateDown, this);
   }
}
