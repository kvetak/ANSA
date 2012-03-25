#ifndef __INET_OSPFINTERFACESTATEDESIGNATEDROUTER_H
#define __INET_OSPFINTERFACESTATEDESIGNATEDROUTER_H

#include "AnsaOSPFInterfaceState.h"

namespace AnsaOSPF {

class InterfaceStateDesignatedRouter : public InterfaceState
{
public:
    virtual void ProcessEvent(Interface* intf, Interface::InterfaceEventType event);
    virtual Interface::InterfaceStateType GetState(void) const { return Interface::DesignatedRouterState; }
};

} // namespace AnsaOSPF

#endif // __INET_OSPFINTERFACESTATEDESIGNATEDROUTER_H

