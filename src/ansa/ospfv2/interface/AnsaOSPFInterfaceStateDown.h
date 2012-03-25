#ifndef __INET_OSPFINTERFACESTATEDOWN_H
#define __INET_OSPFINTERFACESTATEDOWN_H

#include "AnsaOSPFInterfaceState.h"

namespace AnsaOSPF {

class InterfaceStateDown : public InterfaceState
{
public:
    virtual void ProcessEvent(Interface* intf, Interface::InterfaceEventType event);
    virtual Interface::InterfaceStateType GetState(void) const { return Interface::DownState; }
};

} // namespace AnsaOSPF

#endif // __INET_OSPFINTERFACESTATEDOWN_H

