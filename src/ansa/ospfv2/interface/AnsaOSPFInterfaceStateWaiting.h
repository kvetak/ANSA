#ifndef __INET_OSPFINTERFACESTATEWAITING_H
#define __INET_OSPFINTERFACESTATEWAITING_H

#include "AnsaOSPFInterfaceState.h"

namespace AnsaOSPF {

class InterfaceStateWaiting : public InterfaceState
{
public:
    virtual void ProcessEvent(Interface* intf, Interface::InterfaceEventType event);
    virtual Interface::InterfaceStateType GetState(void) const { return Interface::WaitingState; }
};

} // namespace AnsaOSPF

#endif // __INET_OSPFINTERFACESTATEWAITING_H

