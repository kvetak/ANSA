#ifndef __INET_OSPFNEIGHBORSTATEATTEMPT_H
#define __INET_OSPFNEIGHBORSTATEATTEMPT_H

#include "AnsaOSPFNeighborState.h"

namespace AnsaOSPF {

class NeighborStateAttempt : public NeighborState
{
public:
    virtual void ProcessEvent(Neighbor* neighbor, Neighbor::NeighborEventType event);
    virtual Neighbor::NeighborStateType GetState(void) const { return Neighbor::AttemptState; }
};

} // namespace AnsaOSPF

#endif // __INET_OSPFNEIGHBORSTATEATTEMPT_H

