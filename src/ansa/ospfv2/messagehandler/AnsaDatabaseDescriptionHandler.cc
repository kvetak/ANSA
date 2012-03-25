#include "AnsaDatabaseDescriptionHandler.h"
#include "AnsaOSPFNeighbor.h"
#include "AnsaOSPFInterface.h"
#include "AnsaOSPFRouter.h"
#include "AnsaOSPFArea.h"

AnsaOSPF::DatabaseDescriptionHandler::DatabaseDescriptionHandler(AnsaOSPF::Router* containingRouter) :
    AnsaOSPF::IMessageHandler(containingRouter)
{
}

void AnsaOSPF::DatabaseDescriptionHandler::ProcessPacket(OSPFPacket* packet, AnsaOSPF::Interface* intf, AnsaOSPF::Neighbor* neighbor)
{
    router->GetMessageHandler()->PrintEvent("Database Description packet received", intf, neighbor);

    OSPFDatabaseDescriptionPacket* ddPacket = check_and_cast<OSPFDatabaseDescriptionPacket*> (packet);

    AnsaOSPF::Neighbor::NeighborStateType neighborState = neighbor->GetState();

    if ((ddPacket->getInterfaceMTU() <= intf->GetMTU()) &&
        (neighborState > AnsaOSPF::Neighbor::AttemptState))
    {
        switch (neighborState) {
            case AnsaOSPF::Neighbor::TwoWayState:
                break;
            case AnsaOSPF::Neighbor::InitState:
                neighbor->ProcessEvent(AnsaOSPF::Neighbor::TwoWayReceived);
                break;
            case AnsaOSPF::Neighbor::ExchangeStartState:
                {
                    OSPFDDOptions& ddOptions = ddPacket->getDdOptions();

                    if (ddOptions.I_Init && ddOptions.M_More && ddOptions.MS_MasterSlave &&
                        (ddPacket->getLsaHeadersArraySize() == 0))
                    {
                        if (neighbor->GetNeighborID() > router->GetRouterID()) {
                            AnsaOSPF::Neighbor::DDPacketID packetID;
                            packetID.ddOptions      = ddOptions;
                            packetID.options        = ddPacket->getOptions();
                            packetID.sequenceNumber = ddPacket->getDdSequenceNumber();

                            neighbor->SetOptions(packetID.options);
                            neighbor->SetDatabaseExchangeRelationship(AnsaOSPF::Neighbor::Slave);
                            neighbor->SetDDSequenceNumber(packetID.sequenceNumber);
                            neighbor->SetLastReceivedDDPacket(packetID);

                            if (!ProcessDDPacket(ddPacket, intf, neighbor, true)) {
                                break;
                            }

                            neighbor->ProcessEvent(AnsaOSPF::Neighbor::NegotiationDone);
                            if (!neighbor->IsLinkStateRequestListEmpty() &&
                                !neighbor->IsRequestRetransmissionTimerActive())
                            {
                                neighbor->SendLinkStateRequestPacket();
                                neighbor->ClearRequestRetransmissionTimer();
                                neighbor->StartRequestRetransmissionTimer();
                            }
                        } else {
                            neighbor->SendDatabaseDescriptionPacket(true);
                        }
                    }
                    if (!ddOptions.I_Init && !ddOptions.MS_MasterSlave &&
                        (ddPacket->getDdSequenceNumber() == neighbor->GetDDSequenceNumber()) &&
                        (neighbor->GetNeighborID() < router->GetRouterID()))
                    {
                        AnsaOSPF::Neighbor::DDPacketID packetID;
                        packetID.ddOptions      = ddOptions;
                        packetID.options        = ddPacket->getOptions();
                        packetID.sequenceNumber = ddPacket->getDdSequenceNumber();

                        neighbor->SetOptions(packetID.options);
                        neighbor->SetDatabaseExchangeRelationship(AnsaOSPF::Neighbor::Master);
                        neighbor->SetLastReceivedDDPacket(packetID);

                        if (!ProcessDDPacket(ddPacket, intf, neighbor, true)) {
                            break;
                        }

                        neighbor->ProcessEvent(AnsaOSPF::Neighbor::NegotiationDone);
                        if (!neighbor->IsLinkStateRequestListEmpty() &&
                            !neighbor->IsRequestRetransmissionTimerActive())
                        {
                            neighbor->SendLinkStateRequestPacket();
                            neighbor->ClearRequestRetransmissionTimer();
                            neighbor->StartRequestRetransmissionTimer();
                        }
                    }
                }
                break;
            case AnsaOSPF::Neighbor::ExchangeState:
                {
                    AnsaOSPF::Neighbor::DDPacketID packetID;
                    packetID.ddOptions      = ddPacket->getDdOptions();
                    packetID.options        = ddPacket->getOptions();
                    packetID.sequenceNumber = ddPacket->getDdSequenceNumber();

                    if (packetID != neighbor->GetLastReceivedDDPacket()) {
                        if ((packetID.ddOptions.MS_MasterSlave &&
                             (neighbor->GetDatabaseExchangeRelationship() != AnsaOSPF::Neighbor::Slave)) ||
                            (!packetID.ddOptions.MS_MasterSlave &&
                             (neighbor->GetDatabaseExchangeRelationship() != AnsaOSPF::Neighbor::Master)) ||
                            packetID.ddOptions.I_Init ||
                            (packetID.options != neighbor->GetLastReceivedDDPacket().options))
                        {
                            neighbor->ProcessEvent(AnsaOSPF::Neighbor::SequenceNumberMismatch);
                        } else {
                            if (((neighbor->GetDatabaseExchangeRelationship() == AnsaOSPF::Neighbor::Master) &&
                                 (packetID.sequenceNumber == neighbor->GetDDSequenceNumber())) ||
                                ((neighbor->GetDatabaseExchangeRelationship() == AnsaOSPF::Neighbor::Slave) &&
                                 (packetID.sequenceNumber == (neighbor->GetDDSequenceNumber() + 1))))
                            {
                                neighbor->SetLastReceivedDDPacket(packetID);
                                if (!ProcessDDPacket(ddPacket, intf, neighbor, false)) {
                                    break;
                                }
                                if (!neighbor->IsLinkStateRequestListEmpty() &&
                                    !neighbor->IsRequestRetransmissionTimerActive())
                                {
                                    neighbor->SendLinkStateRequestPacket();
                                    neighbor->ClearRequestRetransmissionTimer();
                                    neighbor->StartRequestRetransmissionTimer();
                                }
                            } else {
                                neighbor->ProcessEvent(AnsaOSPF::Neighbor::SequenceNumberMismatch);
                            }
                        }
                    } else {
                        if (neighbor->GetDatabaseExchangeRelationship() == AnsaOSPF::Neighbor::Slave) {
                            neighbor->RetransmitDatabaseDescriptionPacket();
                        }
                    }
                }
                break;
            case AnsaOSPF::Neighbor::LoadingState:
            case AnsaOSPF::Neighbor::FullState:
                {
                    AnsaOSPF::Neighbor::DDPacketID packetID;
                    packetID.ddOptions      = ddPacket->getDdOptions();
                    packetID.options        = ddPacket->getOptions();
                    packetID.sequenceNumber = ddPacket->getDdSequenceNumber();

                    if ((packetID != neighbor->GetLastReceivedDDPacket()) ||
                        (packetID.ddOptions.I_Init))
                    {
                        neighbor->ProcessEvent(AnsaOSPF::Neighbor::SequenceNumberMismatch);
                    } else {
                        if (neighbor->GetDatabaseExchangeRelationship() == AnsaOSPF::Neighbor::Slave) {
                            if (!neighbor->RetransmitDatabaseDescriptionPacket()) {
                                neighbor->ProcessEvent(AnsaOSPF::Neighbor::SequenceNumberMismatch);
                            }
                        }
                    }
                }
                break;
            default: break;
        }
    }
}

bool AnsaOSPF::DatabaseDescriptionHandler::ProcessDDPacket(OSPFDatabaseDescriptionPacket* ddPacket, AnsaOSPF::Interface* intf, AnsaOSPF::Neighbor* neighbor, bool inExchangeStart)
{
    EV << "  Processing packet contents(ddOptions="
       << ((ddPacket->getDdOptions().I_Init) ? "I " : "_ ")
       << ((ddPacket->getDdOptions().M_More) ? "M " : "_ ")
       << ((ddPacket->getDdOptions().MS_MasterSlave) ? "MS" : "__")
       << "; seqNumber="
       << ddPacket->getDdSequenceNumber()
       << "):\n";

    unsigned int headerCount = ddPacket->getLsaHeadersArraySize();

    for (unsigned int i = 0; i < headerCount; i++) {
        OSPFLSAHeader& currentHeader = ddPacket->getLsaHeaders(i);
        LSAType        lsaType       = static_cast<LSAType> (currentHeader.getLsType());

        EV << "    ";
        PrintLSAHeader(currentHeader, ev.getOStream());

        if ((lsaType < RouterLSAType) || (lsaType > ASExternalLSAType) ||
            ((lsaType == ASExternalLSAType) && (!intf->GetArea()->GetExternalRoutingCapability())))
        {
            EV << " Error!\n";
            neighbor->ProcessEvent(AnsaOSPF::Neighbor::SequenceNumberMismatch);
            return false;
        } else {
            AnsaOSPF::LSAKeyType lsaKey;

            lsaKey.linkStateID = currentHeader.getLinkStateID();
            lsaKey.advertisingRouter = currentHeader.getAdvertisingRouter().getInt();

            OSPFLSA* lsaInDatabase = router->FindLSA(lsaType, lsaKey, intf->GetArea()->GetAreaID());

            // operator< and operator== on OSPFLSAHeaders determines which one is newer(less means older)
            if ((lsaInDatabase == NULL) || (lsaInDatabase->getHeader() < currentHeader)) {
                EV << " (newer)";
                neighbor->AddToRequestList(&currentHeader);
            }
        }
        EV << "\n";
    }

    if (neighbor->GetDatabaseExchangeRelationship() == AnsaOSPF::Neighbor::Master) {
        neighbor->IncrementDDSequenceNumber();
        if ((neighbor->GetDatabaseSummaryListCount() == 0) && !ddPacket->getDdOptions().M_More) {
            neighbor->ProcessEvent(AnsaOSPF::Neighbor::ExchangeDone);  // does nothing in ExchangeStart
        } else {
            if (!inExchangeStart) {
                neighbor->SendDatabaseDescriptionPacket();
            }
        }
    } else {
        neighbor->SetDDSequenceNumber(ddPacket->getDdSequenceNumber());
        if (!inExchangeStart) {
            neighbor->SendDatabaseDescriptionPacket();
        }
        if (!ddPacket->getDdOptions().M_More &&
            (neighbor->GetDatabaseSummaryListCount() == 0))
        {
            neighbor->ProcessEvent(AnsaOSPF::Neighbor::ExchangeDone);  // does nothing in ExchangeStart
        }
    }
    return true;
}
