#ifndef __ANSA_OSPFV3NEIGHBORSTATEEXCHANGE_H_
#define __ANSA_OSPFV3NEIGHBORSTATEEXCHANGE_H_

#include "inet/common/INETDefs.h"
#include "ansa/routing/ospfv3/neighbor/OSPFv3Neighbor.h"
#include "ansa/routing/ospfv3/neighbor/OSPFv3NeighborState.h"

namespace inet{

class INET_API OSPFv3NeighborStateExchange : public OSPFv3NeighborState
{
    /*
     * DDs are sent to the neighbor. Each DD has DD sequence number and is explicitly acknowledged.
     * Link state requests may be sent in this state. Flooding procedure works on all adjacencies.
     */
  public:
    void processEvent(OSPFv3Neighbor* neighbor, OSPFv3Neighbor::OSPFv3NeighborEventType event) override;
    virtual OSPFv3Neighbor::OSPFv3NeighborStateType getState() const override { return OSPFv3Neighbor::EXCHANGE_STATE; }
    std::string getNeighborStateString(){return std::string("OSPFv3NeighborStateExchange");};
    ~OSPFv3NeighborStateExchange(){};
};

}//namespace inet
#endif
