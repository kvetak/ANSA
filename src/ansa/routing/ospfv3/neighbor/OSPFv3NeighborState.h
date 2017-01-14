#ifndef __ANSA_OSPFV3NEIGHBORSTATE_H_
#define __ANSA_OSPFV3NEIGHBORSTATE_H_

#include "inet/common/INETDefs.h"
#include "ansa/routing/ospfv3/neighbor/OSPFv3Neighbor.h"
#include "ansa/routing/ospfv3/interface/OSPFv3Interface.h"


namespace inet{

class INET_API OSPFv3NeighborState
{
  protected:
    void changeState(OSPFv3Neighbor *neighbor, OSPFv3NeighborState *newState, OSPFv3NeighborState *currentState);

  public:
    virtual void processEvent(OSPFv3Neighbor* neighbor, OSPFv3Neighbor::OSPFv3NeighborEventType event) = 0;
    virtual OSPFv3Neighbor::OSPFv3NeighborStateType getState() const = 0;
    virtual std::string getNeighborStateString() = 0;
    virtual ~OSPFv3NeighborState(){};
};

}//namespace inet
#endif
