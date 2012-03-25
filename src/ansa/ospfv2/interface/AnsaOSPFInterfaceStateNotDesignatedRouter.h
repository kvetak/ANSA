#ifndef __INET_OSPFINTERFACESTATENOTDESIGNATEDROUTER_H
#define __INET_OSPFINTERFACESTATENOTDESIGNATEDROUTER_H

#include "AnsaOSPFInterfaceState.h"

namespace AnsaOSPF {

class InterfaceStateNotDesignatedRouter : public InterfaceState
{
public:
    virtual void ProcessEvent(Interface* intf, Interface::InterfaceEventType event);
    virtual Interface::InterfaceStateType GetState(void) const { return Interface::NotDesignatedRouterState; }
};

} // namespace AnsaOSPF

#endif // __INET_OSPFINTERFACESTATENOTDESIGNATEDROUTER_H

