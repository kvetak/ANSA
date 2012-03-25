#include "AnsaOSPFInterfaceStatePointToPoint.h"
#include "AnsaOSPFInterfaceStateDown.h"
#include "AnsaOSPFInterfaceStateLoopback.h"
#include "AnsaOSPFArea.h"
#include "AnsaOSPFRouter.h"
#include "AnsaMessageHandler.h"

void AnsaOSPF::InterfaceStatePointToPoint::ProcessEvent(AnsaOSPF::Interface* intf, AnsaOSPF::Interface::InterfaceEventType event)
{
    if (event == AnsaOSPF::Interface::InterfaceDown) {
        intf->SetIsGoingDown(true);
        intf->Reset();
        ChangeState(intf, new AnsaOSPF::InterfaceStateDown, this);
        intf->SetIsGoingDown(false);
    }
    if (event == AnsaOSPF::Interface::LoopIndication) {
        intf->Reset();
        ChangeState(intf, new AnsaOSPF::InterfaceStateLoopback, this);
    }
    if (event == AnsaOSPF::Interface::HelloTimer) {
        if (intf->GetType() == AnsaOSPF::Interface::Virtual) {
            if (intf->GetNeighborCount() > 0) {
                intf->SendHelloPacket(intf->GetNeighbor(0)->GetAddress(), VIRTUAL_LINK_TTL);
            }
        } else {
            intf->SendHelloPacket(AnsaOSPF::AllSPFRouters);
        }
        intf->GetArea()->GetRouter()->GetMessageHandler()->StartTimer(intf->GetHelloTimer(), intf->GetHelloInterval());
    }
    if (event == AnsaOSPF::Interface::AcknowledgementTimer) {
        intf->SendDelayedAcknowledgements();
    }
}

