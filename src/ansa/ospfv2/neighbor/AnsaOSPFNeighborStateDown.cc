#include "AnsaOSPFNeighborStateDown.h"
#include "AnsaOSPFNeighborStateAttempt.h"
#include "AnsaOSPFNeighborStateInit.h"
#include "AnsaMessageHandler.h"
#include "AnsaOSPFInterface.h"
#include "AnsaOSPFArea.h"
#include "AnsaOSPFRouter.h"

void AnsaOSPF::NeighborStateDown::ProcessEvent(AnsaOSPF::Neighbor* neighbor, AnsaOSPF::Neighbor::NeighborEventType event)
{
    if (event == AnsaOSPF::Neighbor::Start) {
        MessageHandler* messageHandler = neighbor->GetInterface()->GetArea()->GetRouter()->GetMessageHandler();
        int             ttl            = (neighbor->GetInterface()->GetType() == AnsaOSPF::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;

        messageHandler->ClearTimer(neighbor->GetPollTimer());
        neighbor->GetInterface()->SendHelloPacket(neighbor->GetAddress(), ttl);
        messageHandler->StartTimer(neighbor->GetInactivityTimer(), neighbor->GetRouterDeadInterval());
        ChangeState(neighbor, new AnsaOSPF::NeighborStateAttempt, this);
    }
    if (event == AnsaOSPF::Neighbor::HelloReceived) {
        MessageHandler* messageHandler = neighbor->GetInterface()->GetArea()->GetRouter()->GetMessageHandler();
        messageHandler->ClearTimer(neighbor->GetPollTimer());
        messageHandler->StartTimer(neighbor->GetInactivityTimer(), neighbor->GetRouterDeadInterval());
        ChangeState(neighbor, new AnsaOSPF::NeighborStateInit, this);
    }
    if (event == AnsaOSPF::Neighbor::PollTimer) {
        int ttl = (neighbor->GetInterface()->GetType() == AnsaOSPF::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;
        neighbor->GetInterface()->SendHelloPacket(neighbor->GetAddress(), ttl);
        MessageHandler* messageHandler = neighbor->GetInterface()->GetArea()->GetRouter()->GetMessageHandler();
        messageHandler->StartTimer(neighbor->GetPollTimer(), neighbor->GetInterface()->GetPollInterval());
    }
}
