#ifndef __ANSA_OSPFV3NEIGHBORSTATEDOWN_H_
#define __ANSA_OSPFV3NEIGHBORSTATEDOWN_H_

#include "inet/common/INETDefs.h"
#include "ansa/routing/ospfv3/neighbor/OSPFv3Neighbor.h"
#include "ansa/routing/ospfv3/neighbor/OSPFv3NeighborState.h"

namespace inet{

class INET_API OSPFv3NeighborStateDown : public OSPFv3NeighborState
{
    /*
     * Indicates that no Hello has been received. On NBMA networks, hello packets may still
     * be sent to Down neighbors but at a reduced rate.
     */
  public:
    void processEvent(OSPFv3Neighbor* neighbor, OSPFv3Neighbor::OSPFv3NeighborEventType event) override;
    virtual OSPFv3Neighbor::OSPFv3NeighborStateType getState() const override { return OSPFv3Neighbor::DOWN_STATE; }
    std::string getNeighborStateString(){return std::string("OSPFv3NeighborStateDown");};
    ~OSPFv3NeighborStateDown(){};

};

}//namespace inet
#endif
