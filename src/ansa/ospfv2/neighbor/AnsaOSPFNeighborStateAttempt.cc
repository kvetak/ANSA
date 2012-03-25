#include "AnsaOSPFNeighborStateAttempt.h"
#include "AnsaOSPFNeighborStateDown.h"
#include "AnsaOSPFNeighborStateInit.h"
#include "AnsaMessageHandler.h"
#include "AnsaOSPFInterface.h"
#include "AnsaOSPFArea.h"
#include "AnsaOSPFRouter.h"

void AnsaOSPF::NeighborStateAttempt::ProcessEvent(AnsaOSPF::Neighbor* neighbor, AnsaOSPF::Neighbor::NeighborEventType event)
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
        ChangeState(neighbor, new AnsaOSPF::NeighborStateInit, this);
    }
}
