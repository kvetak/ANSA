#include "AnsaOSPFInterfaceStateNotDesignatedRouter.h"
#include "AnsaOSPFInterfaceStateDown.h"
#include "AnsaOSPFInterfaceStateLoopback.h"
#include "AnsaOSPFArea.h"
#include "AnsaOSPFRouter.h"
#include "AnsaMessageHandler.h"

void AnsaOSPF::InterfaceStateNotDesignatedRouter::ProcessEvent(AnsaOSPF::Interface* intf, AnsaOSPF::Interface::InterfaceEventType event)
{
    if (event == AnsaOSPF::Interface::NeighborChange) {
        CalculateDesignatedRouter(intf);
    }
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
        if (intf->GetType() == AnsaOSPF::Interface::Broadcast) {
            intf->SendHelloPacket(AnsaOSPF::AllSPFRouters);
        } else {    // AnsaOSPF::Interface::NBMA
            if (intf->GetRouterPriority() > 0) {
                unsigned long neighborCount = intf->GetNeighborCount();
                for (unsigned long i = 0; i < neighborCount; i++) {
                    const AnsaOSPF::Neighbor* neighbor = intf->GetNeighbor(i);
                    if (neighbor->GetPriority() > 0) {
                        intf->SendHelloPacket(neighbor->GetAddress());
                    }
                }
            } else {
                intf->SendHelloPacket(intf->GetDesignatedRouter().ipInterfaceAddress);
                intf->SendHelloPacket(intf->GetBackupDesignatedRouter().ipInterfaceAddress);
            }
        }
        intf->GetArea()->GetRouter()->GetMessageHandler()->StartTimer(intf->GetHelloTimer(), intf->GetHelloInterval());
    }
    if (event == AnsaOSPF::Interface::AcknowledgementTimer) {
        intf->SendDelayedAcknowledgements();
    }
}

