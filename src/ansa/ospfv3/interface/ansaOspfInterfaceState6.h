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

#ifndef ANSAOSPFINTERFACESTATE6_H_
#define ANSAOSPFINTERFACESTATE6_H_

#include "ansaOspfInterface6.h"

namespace AnsaOspf6 {

class InterfaceState {
   protected:
      void ChangeState(Interface* intf, InterfaceState* newState, InterfaceState* currentState);
      void CalculateDesignatedRouter(Interface* intf);

   public:
      virtual ~InterfaceState() {}

      virtual void ProcessEvent(Interface* intf, Interface::InterfaceEventType event) = 0;
      virtual Interface::InterfaceStateType GetState(void) const = 0;
};

}

#endif /* ANSAOSPFINTERFACESTATE6_H_ */
