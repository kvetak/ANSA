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

#ifndef ANSAOSPFNEIGHBORSTATEEXCHANGE6_H_
#define ANSAOSPFNEIGHBORSTATEEXCHANGE6_H_

#include "ansaOspfNeighborState6.h"

namespace AnsaOspf6 {

class NeighborStateExchange : public NeighborState {
   virtual void ProcessEvent(Neighbor* neighbor, Neighbor::NeighborEventType event);
   virtual Neighbor::NeighborStateType GetState(void) const { return Neighbor::ExchangeState; }
};

}

#endif /* ANSAOSPFNEIGHBORSTATEEXCHANGE6_H_ */
