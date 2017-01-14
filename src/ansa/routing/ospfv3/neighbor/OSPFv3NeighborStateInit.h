#ifndef __ANSA_OSPFV3NEIGHBORSTATEINIT_H_
#define __ANSA_OSPFV3NEIGHBORSTATEINIT_H_

#include "inet/common/INETDefs.h"
#include "ansa/routing/ospfv3/neighbor/OSPFv3Neighbor.h"
#include "ansa/routing/ospfv3/neighbor/OSPFv3NeighborState.h"

namespace inet{

class INET_API OSPFv3NeighborStateInit : public OSPFv3NeighborState
{
    /*
     * Hello has recently been seen from the neighbor, but bidirectional comm has not been established.
     * This means that the router has not seen itself in the Hello from neighbor. All neighbors
     * in this or higher states are listed in hello.
     */
  public:
    void processEvent(OSPFv3Neighbor* neighbor, OSPFv3Neighbor::OSPFv3NeighborEventType event) override;
    virtual OSPFv3Neighbor::OSPFv3NeighborStateType getState() const override { return OSPFv3Neighbor::INIT_STATE; }
    std::string getNeighborStateString(){return std::string("OSPFv3NeighborStateInit");};
    ~OSPFv3NeighborStateInit(){};
};

}//namespace inet
#endif
