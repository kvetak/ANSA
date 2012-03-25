#include "AnsaOSPFInterfaceStateDown.h"
#include "AnsaMessageHandler.h"
#include "AnsaOSPFArea.h"
#include "AnsaOSPFRouter.h"
#include "AnsaOSPFInterfaceStatePointToPoint.h"
#include "AnsaOSPFInterfaceStateNotDesignatedRouter.h"
#include "AnsaOSPFInterfaceStateWaiting.h"
#include "AnsaOSPFInterfaceStateLoopback.h"

void AnsaOSPF::InterfaceStateDown::ProcessEvent(AnsaOSPF::Interface* intf, AnsaOSPF::Interface::InterfaceEventType event)
{
    if (event == AnsaOSPF::Interface::InterfaceUp) {
        AnsaOSPF::MessageHandler* messageHandler = intf->GetArea()->GetRouter()->GetMessageHandler();
        messageHandler->StartTimer(intf->GetHelloTimer(), truncnormal(0.1, 0.01)); // add some deviation to avoid startup collisions
        messageHandler->StartTimer(intf->GetAcknowledgementTimer(), intf->GetAcknowledgementDelay());
        switch (intf->GetType()) {
            case AnsaOSPF::Interface::PointToPoint:
            case AnsaOSPF::Interface::PointToMultiPoint:
            case AnsaOSPF::Interface::Virtual:
                ChangeState(intf, new AnsaOSPF::InterfaceStatePointToPoint, this);
                break;

            case AnsaOSPF::Interface::NBMA:
                if (intf->GetRouterPriority() == 0) {
                    ChangeState(intf, new AnsaOSPF::InterfaceStateNotDesignatedRouter, this);
                } else {
                    ChangeState(intf, new AnsaOSPF::InterfaceStateWaiting, this);
                    messageHandler->StartTimer(intf->GetWaitTimer(), intf->GetRouterDeadInterval());

                    long neighborCount = intf->GetNeighborCount();
                    for (long i = 0; i < neighborCount; i++) {
                        AnsaOSPF::Neighbor* neighbor = intf->GetNeighbor(i);
                        if (neighbor->GetPriority() > 0) {
                            neighbor->ProcessEvent(AnsaOSPF::Neighbor::Start);
                        }
                    }
                }
                break;

            case AnsaOSPF::Interface::Broadcast:
                if (intf->GetRouterPriority() == 0) {
                    ChangeState(intf, new AnsaOSPF::InterfaceStateNotDesignatedRouter, this);
                } else {
                    ChangeState(intf, new AnsaOSPF::InterfaceStateWaiting, this);
                    messageHandler->StartTimer(intf->GetWaitTimer(), intf->GetRouterDeadInterval());
                }
                break;

            default:
                break;
        }
    }
    if (event == AnsaOSPF::Interface::LoopIndication) {
        intf->Reset();
        ChangeState(intf, new AnsaOSPF::InterfaceStateLoopback, this);
    }
}

