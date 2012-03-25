#include "AnsaOSPFNeighborStateExchangeStart.h"
#include "AnsaOSPFNeighborStateDown.h"
#include "AnsaOSPFNeighborStateInit.h"
#include "AnsaOSPFNeighborStateTwoWay.h"
#include "AnsaOSPFNeighborStateExchange.h"
#include "AnsaMessageHandler.h"
#include "AnsaOSPFInterface.h"
#include "AnsaOSPFArea.h"
#include "AnsaOSPFRouter.h"

void AnsaOSPF::NeighborStateExchangeStart::ProcessEvent(AnsaOSPF::Neighbor* neighbor, AnsaOSPF::Neighbor::NeighborEventType event)
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
    if (event == AnsaOSPF::Neighbor::OneWayReceived) {
        neighbor->Reset();
        ChangeState(neighbor, new AnsaOSPF::NeighborStateInit, this);
    }
    if (event == AnsaOSPF::Neighbor::HelloReceived) {
        MessageHandler* messageHandler = neighbor->GetInterface()->GetArea()->GetRouter()->GetMessageHandler();
        messageHandler->ClearTimer(neighbor->GetInactivityTimer());
        messageHandler->StartTimer(neighbor->GetInactivityTimer(), neighbor->GetRouterDeadInterval());
    }
    if (event == AnsaOSPF::Neighbor::IsAdjacencyOK) {
        if (!neighbor->NeedAdjacency()) {
            neighbor->Reset();
            ChangeState(neighbor, new AnsaOSPF::NeighborStateTwoWay, this);
        }
    }
    if (event == AnsaOSPF::Neighbor::DDRetransmissionTimer) {
        MessageHandler* messageHandler = neighbor->GetInterface()->GetArea()->GetRouter()->GetMessageHandler();
        neighbor->RetransmitDatabaseDescriptionPacket();
        messageHandler->StartTimer(neighbor->GetDDRetransmissionTimer(), neighbor->GetInterface()->GetRetransmissionInterval());
    }
    if (event == AnsaOSPF::Neighbor::NegotiationDone) {
        neighbor->CreateDatabaseSummary();
        neighbor->SendDatabaseDescriptionPacket();
        MessageHandler* messageHandler = neighbor->GetInterface()->GetArea()->GetRouter()->GetMessageHandler();
        messageHandler->ClearTimer(neighbor->GetDDRetransmissionTimer());
        ChangeState(neighbor, new AnsaOSPF::NeighborStateExchange, this);
    }
}
