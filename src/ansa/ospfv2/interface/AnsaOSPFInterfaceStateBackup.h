#ifndef __INET_OSPFINTERFACESTATEBACKUP_H
#define __INET_OSPFINTERFACESTATEBACKUP_H

#include "AnsaOSPFInterfaceState.h"

namespace AnsaOSPF {

class InterfaceStateBackup : public InterfaceState
{
public:
    virtual void ProcessEvent(Interface* intf, Interface::InterfaceEventType event);
    virtual Interface::InterfaceStateType GetState(void) const { return Interface::BackupState; }
};

} // namespace AnsaOSPF

#endif // __INET_OSPFINTERFACESTATEBACKUP_H

