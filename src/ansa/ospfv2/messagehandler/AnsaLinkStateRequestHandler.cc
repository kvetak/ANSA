#include "AnsaLinkStateRequestHandler.h"
#include "AnsaOSPFNeighbor.h"
#include "AnsaOSPFRouter.h"
#include <vector>

AnsaOSPF::LinkStateRequestHandler::LinkStateRequestHandler(AnsaOSPF::Router* containingRouter) :
    AnsaOSPF::IMessageHandler(containingRouter)
{
}

void AnsaOSPF::LinkStateRequestHandler::ProcessPacket(OSPFPacket* packet, AnsaOSPF::Interface* intf, AnsaOSPF::Neighbor* neighbor)
{
    router->GetMessageHandler()->PrintEvent("Link State Request packet received", intf, neighbor);

    AnsaOSPF::Neighbor::NeighborStateType neighborState = neighbor->GetState();

    if ((neighborState == AnsaOSPF::Neighbor::ExchangeState) ||
        (neighborState == AnsaOSPF::Neighbor::LoadingState) ||
        (neighborState == AnsaOSPF::Neighbor::FullState))
    {
        OSPFLinkStateRequestPacket* lsRequestPacket = check_and_cast<OSPFLinkStateRequestPacket*> (packet);

        unsigned long         requestCount = lsRequestPacket->getRequestsArraySize();
        bool                  error        = false;
        std::vector<OSPFLSA*> lsas;

        EV << "  Processing packet contents:\n";

        for (unsigned long i = 0; i < requestCount; i++) {
            LSARequest&      request = lsRequestPacket->getRequests(i);
            AnsaOSPF::LSAKeyType lsaKey;
            char             addressString[16];

            EV << "    LSARequest: type="
               << request.lsType
               << ", LSID="
               << AddressStringFromULong(addressString, sizeof(addressString), request.linkStateID)
               << ", advertisingRouter="
               << AddressStringFromULong(addressString, sizeof(addressString), request.advertisingRouter.getInt())
               << "\n";

            lsaKey.linkStateID = request.linkStateID;
            lsaKey.advertisingRouter = request.advertisingRouter.getInt();

            OSPFLSA* lsaInDatabase = router->FindLSA(static_cast<LSAType> (request.lsType), lsaKey, intf->GetArea()->GetAreaID());

            if (lsaInDatabase != NULL) {
                lsas.push_back(lsaInDatabase);
            } else {
                error = true;
                neighbor->ProcessEvent(AnsaOSPF::Neighbor::BadLinkStateRequest);
                break;
            }
        }

        if (!error) {
            int                   updatesCount   = lsas.size();
            int                   ttl            = (intf->GetType() == AnsaOSPF::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;
            AnsaOSPF::MessageHandler* messageHandler = router->GetMessageHandler();

            for (int j = 0; j < updatesCount; j++) {
                OSPFLinkStateUpdatePacket* updatePacket = intf->CreateUpdatePacket(lsas[j]);
                if (updatePacket != NULL) {
                    if (intf->GetType() == AnsaOSPF::Interface::Broadcast) {
                        if ((intf->GetState() == AnsaOSPF::Interface::DesignatedRouterState) ||
                            (intf->GetState() == AnsaOSPF::Interface::BackupState) ||
                            (intf->GetDesignatedRouter() == AnsaOSPF::NullDesignatedRouterID))
                        {
                            messageHandler->SendPacket(updatePacket, AnsaOSPF::AllSPFRouters, intf->GetIfIndex(), ttl);
                        } else {
                            messageHandler->SendPacket(updatePacket, AnsaOSPF::AllDRouters, intf->GetIfIndex(), ttl);
                        }
                    } else {
                        if (intf->GetType() == AnsaOSPF::Interface::PointToPoint) {
                            messageHandler->SendPacket(updatePacket, AnsaOSPF::AllSPFRouters, intf->GetIfIndex(), ttl);
                        } else {
                            messageHandler->SendPacket(updatePacket, neighbor->GetAddress(), intf->GetIfIndex(), ttl);
                        }
                    }

                }
            }
            // These update packets should not be placed on retransmission lists
        }
    }
}
