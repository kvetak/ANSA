#ifndef __INET_OSPFNEIGHBORSTATELOADING_H
#define __INET_OSPFNEIGHBORSTATELOADING_H

#include "AnsaOSPFNeighborState.h"

namespace AnsaOSPF {

class NeighborStateLoading : public NeighborState
{
public:
    virtual void ProcessEvent(Neighbor* neighbor, Neighbor::NeighborEventType event);
    virtual Neighbor::NeighborStateType GetState(void) const { return Neighbor::LoadingState; }
};

} // namespace AnsaOSPF

#endif // __INET_OSPFNEIGHBORSTATELOADING_H

