#include "AnsaOSPFNeighborStateInit.h"
#include "AnsaOSPFNeighborStateDown.h"
#include "AnsaOSPFNeighborStateExchangeStart.h"
#include "AnsaOSPFNeighborStateTwoWay.h"
#include "AnsaMessageHandler.h"
#include "AnsaOSPFInterface.h"
#include "AnsaOSPFArea.h"
#include "AnsaOSPFRouter.h"

void AnsaOSPF::NeighborStateInit::ProcessEvent(AnsaOSPF::Neighbor* neighbor, AnsaOSPF::Neighbor::NeighborEventType event)
{
    if ((event == AnsaOSPF::Neighbor::KillNeighbor) || (event == AnsaOSPF::Neighbor::LinkDown)) {
        MessageHandler* messageHandler = neighbor->GetInterface()->GetArea()->GetRouter()->GetMessageHandler();
        neighbor->Reset();
        messageHandler->ClearTimer(neighbor->GetInactivityTimer());
        ChangeState(neighbor, new AnsaOSPF::NeighborStateDown, this);
    }
    if (event == AnsaOSPF::Neighbor::InactivityTimer) {
        neighbor->Reset();
        if (neighbor->GetInterface()->GetType() == AnsaOSPF::Interface::NBMA) {
            MessageHandler* messageHandler = neighbor->GetInterface()->GetArea()->GetRouter()->GetMessageHandler();
            messageHandler->StartTimer(neighbor->GetPollTimer(), neighbor->GetInterface()->GetPollInterval());
        }
        ChangeState(neighbor, new AnsaOSPF::NeighborStateDown, this);
    }
    if (event == AnsaOSPF::Neighbor::HelloReceived) {
        MessageHandler* messageHandler = neighbor->GetInterface()->GetArea()->GetRouter()->GetMessageHandler();
        messageHandler->ClearTimer(neighbor->GetInactivityTimer());
        messageHandler->StartTimer(neighbor->GetInactivityTimer(), neighbor->GetRouterDeadInterval());
    }
    if (event == AnsaOSPF::Neighbor::TwoWayReceived) {
        if (neighbor->NeedAdjacency()) {
            MessageHandler* messageHandler = neighbor->GetInterface()->GetArea()->GetRouter()->GetMessageHandler();
            if (!(neighbor->IsFirstAdjacencyInited())) {
                neighbor->InitFirstAdjacency();
            } else {
                neighbor->IncrementDDSequenceNumber();
            }
            neighbor->SendDatabaseDescriptionPacket(true);
            messageHandler->StartTimer(neighbor->GetDDRetransmissionTimer(), neighbor->GetInterface()->GetRetransmissionInterval());
            ChangeState(neighbor, new AnsaOSPF::NeighborStateExchangeStart, this);
        } else {
            ChangeState(neighbor, new AnsaOSPF::NeighborStateTwoWay, this);
        }
    }
}
