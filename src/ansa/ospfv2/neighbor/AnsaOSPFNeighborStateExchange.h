#ifndef __INET_OSPFNEIGHBORSTATEEXCHANGE_H
#define __INET_OSPFNEIGHBORSTATEEXCHANGE_H

#include "AnsaOSPFNeighborState.h"

namespace AnsaOSPF {

class NeighborStateExchange : public NeighborState
{
public:
    virtual void ProcessEvent(Neighbor* neighbor, Neighbor::NeighborEventType event);
    virtual Neighbor::NeighborStateType GetState(void) const { return Neighbor::ExchangeState; }
};

} // namespace AnsaOSPF

#endif // __INET_OSPFNEIGHBORSTATEEXCHANGE_H

