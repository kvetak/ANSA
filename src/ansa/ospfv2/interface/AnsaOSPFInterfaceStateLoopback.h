#ifndef __INET_OSPFINTERFACESTATELOOPBACK_H
#define __INET_OSPFINTERFACESTATELOOPBACK_H

#include "AnsaOSPFInterfaceState.h"

namespace AnsaOSPF {

class InterfaceStateLoopback : public InterfaceState
{
public:
    virtual void ProcessEvent(Interface* intf, Interface::InterfaceEventType event);
    virtual Interface::InterfaceStateType GetState(void) const { return Interface::LoopbackState; }
};

} // namespace AnsaOSPF

#endif // __INET_OSPFINTERFACESTATELOOPBACK_H

