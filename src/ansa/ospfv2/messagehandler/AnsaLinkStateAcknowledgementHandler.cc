#include "AnsaLinkStateAcknowledgementHandler.h"
#include "AnsaOSPFRouter.h"

AnsaOSPF::LinkStateAcknowledgementHandler::LinkStateAcknowledgementHandler(AnsaOSPF::Router* containingRouter) :
    AnsaOSPF::IMessageHandler(containingRouter)
{
}

void AnsaOSPF::LinkStateAcknowledgementHandler::ProcessPacket(OSPFPacket* packet, AnsaOSPF::Interface* intf, AnsaOSPF::Neighbor* neighbor)
{
    router->GetMessageHandler()->PrintEvent("Link State Acknowledgement packet received", intf, neighbor);

    if (neighbor->GetState() >= AnsaOSPF::Neighbor::ExchangeState) {
        OSPFLinkStateAcknowledgementPacket* lsAckPacket = check_and_cast<OSPFLinkStateAcknowledgementPacket*> (packet);

        int  lsaCount = lsAckPacket->getLsaHeadersArraySize();

        EV << "  Processing packet contents:\n";

        for (int i  = 0; i < lsaCount; i++) {
            OSPFLSAHeader&   lsaHeader = lsAckPacket->getLsaHeaders(i);
            OSPFLSA*         lsaOnRetransmissionList;
            AnsaOSPF::LSAKeyType lsaKey;

            EV << "    ";
            PrintLSAHeader(lsaHeader, ev.getOStream());
            EV << "\n";

            lsaKey.linkStateID = lsaHeader.getLinkStateID();
            lsaKey.advertisingRouter = lsaHeader.getAdvertisingRouter().getInt();

            if ((lsaOnRetransmissionList = neighbor->FindOnRetransmissionList(lsaKey)) != NULL) {
                if (operator== (lsaHeader, lsaOnRetransmissionList->getHeader())) {
                    neighbor->RemoveFromRetransmissionList(lsaKey);
                } else {
                    EV << "Got an Acknowledgement packet for an unsent Update packet.\n";
                }
            }
        }
        if (neighbor->IsLinkStateRetransmissionListEmpty()) {
            neighbor->ClearUpdateRetransmissionTimer();
        }
    }
}
