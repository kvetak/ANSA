#include "AnsaOSPFInterfaceStateLoopback.h"
#include "AnsaOSPFInterfaceStateDown.h"

void AnsaOSPF::InterfaceStateLoopback::ProcessEvent(AnsaOSPF::Interface* intf, AnsaOSPF::Interface::InterfaceEventType event)
{
    if (event == AnsaOSPF::Interface::InterfaceDown) {
        intf->Reset();
        ChangeState(intf, new AnsaOSPF::InterfaceStateDown, this);
    }
    if (event == AnsaOSPF::Interface::UnloopIndication) {
        ChangeState(intf, new AnsaOSPF::InterfaceStateDown, this);
    }
}

