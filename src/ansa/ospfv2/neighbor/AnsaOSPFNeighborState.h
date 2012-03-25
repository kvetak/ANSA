#ifndef __INET_OSPFNEIGHBORSTATE_H
#define __INET_OSPFNEIGHBORSTATE_H

#include "AnsaOSPFNeighbor.h"

namespace AnsaOSPF {

class NeighborState {
protected:
    void ChangeState(Neighbor* neighbor, NeighborState* newState, NeighborState* currentState);

public:
    virtual ~NeighborState() {}

    virtual void ProcessEvent(Neighbor* neighbor, Neighbor::NeighborEventType event) = 0;
    virtual Neighbor::NeighborStateType GetState(void) const = 0;
};

} // namespace AnsaOSPF

#endif // __INET_OSPFNEIGHBORSTATE_H

