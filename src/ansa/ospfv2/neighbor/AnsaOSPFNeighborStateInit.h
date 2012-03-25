#ifndef __INET_OSPFNEIGHBORSTATEINIT_H
#define __INET_OSPFNEIGHBORSTATEINIT_H

#include "AnsaOSPFNeighborState.h"

namespace AnsaOSPF {

class NeighborStateInit : public NeighborState
{
public:
    virtual void ProcessEvent(Neighbor* neighbor, Neighbor::NeighborEventType event);
    virtual Neighbor::NeighborStateType GetState(void) const { return Neighbor::InitState; }
};

} // namespace AnsaOSPF

#endif // __INET_OSPFNEIGHBORSTATEINIT_H

