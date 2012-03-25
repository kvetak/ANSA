#ifndef __INET_OSPFINTERFACESTATEPOINTTOPOINT_H
#define __INET_OSPFINTERFACESTATEPOINTTOPOINT_H

#include "AnsaOSPFInterfaceState.h"

namespace AnsaOSPF {

class InterfaceStatePointToPoint : public InterfaceState
{
public:
    virtual void ProcessEvent(Interface* intf, Interface::InterfaceEventType event);
    virtual Interface::InterfaceStateType GetState(void) const { return Interface::PointToPointState; }
};

} // namespace AnsaOSPF

#endif // __INET_OSPFINTERFACESTATEPOINTTOPOINT_H

