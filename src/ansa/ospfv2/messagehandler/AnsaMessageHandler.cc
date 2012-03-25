#include "AnsaMessageHandler.h"
#include "AnsaOSPFRouter.h"



AnsaOSPF::MessageHandler::MessageHandler(AnsaOSPF::Router* containingRouter, cSimpleModule* containingModule) :
    AnsaOSPF::IMessageHandler(containingRouter),
    ospfModule(containingModule),
    helloHandler(containingRouter),
    ddHandler(containingRouter),
    lsRequestHandler(containingRouter),
    lsUpdateHandler(containingRouter),
    lsAckHandler(containingRouter)
{
}

void AnsaOSPF::MessageHandler::MessageReceived(cMessage* message)
{
    if (message->isSelfMessage()) {
        HandleTimer(check_and_cast<OSPFTimer*> (message));
    } else {
        OSPFPacket* packet = check_and_cast<OSPFPacket*> (message);
        EV << "Received packet: (" << packet->getClassName() << ")" << packet->getName() << "\n";
        if (packet->getRouterID() == router->GetRouterID()) {
            EV << "This packet is from ourselves, discarding.\n";
            delete message;
        } else {
            ProcessPacket(packet);
        }
    }
}

void AnsaOSPF::MessageHandler::HandleTimer(OSPFTimer* timer)
{
    switch (timer->getTimerKind()) {
        case InterfaceHelloTimer:
            {
                AnsaOSPF::Interface* intf;
                if (! (intf = reinterpret_cast <AnsaOSPF::Interface*> (timer->getContextPointer()))) {
                    // should not reach this point
                    EV << "Discarding invalid InterfaceHelloTimer.\n";
                    delete timer;
                } else {
                    PrintEvent("Hello Timer expired", intf);
                    intf->ProcessEvent(AnsaOSPF::Interface::HelloTimer);
                }
            }
            break;
        case InterfaceWaitTimer:
            {
                AnsaOSPF::Interface* intf;
                if (! (intf = reinterpret_cast <AnsaOSPF::Interface*> (timer->getContextPointer()))) {
                    // should not reach this point
                    EV << "Discarding invalid InterfaceWaitTimer.\n";
                    delete timer;
                } else {
                    PrintEvent("Wait Timer expired", intf);
                    intf->ProcessEvent(AnsaOSPF::Interface::WaitTimer);
                }
            }
            break;
        case InterfaceAcknowledgementTimer:
            {
                AnsaOSPF::Interface* intf;
                if (! (intf = reinterpret_cast <AnsaOSPF::Interface*> (timer->getContextPointer()))) {
                    // should not reach this point
                    EV << "Discarding invalid InterfaceAcknowledgementTimer.\n";
                    delete timer;
                } else {
                    PrintEvent("Acknowledgement Timer expired", intf);
                    intf->ProcessEvent(AnsaOSPF::Interface::AcknowledgementTimer);
                }
            }
            break;
        case NeighborInactivityTimer:
            {
                AnsaOSPF::Neighbor* neighbor;
                if (! (neighbor = reinterpret_cast <AnsaOSPF::Neighbor*> (timer->getContextPointer()))) {
                    // should not reach this point
                    EV << "Discarding invalid NeighborInactivityTimer.\n";
                    delete timer;
                } else {
                    PrintEvent("Inactivity Timer expired", neighbor->GetInterface(), neighbor);
                    neighbor->ProcessEvent(AnsaOSPF::Neighbor::InactivityTimer);
                }
            }
            break;
        case NeighborPollTimer:
            {
                AnsaOSPF::Neighbor* neighbor;
                if (! (neighbor = reinterpret_cast <AnsaOSPF::Neighbor*> (timer->getContextPointer()))) {
                    // should not reach this point
                    EV << "Discarding invalid NeighborInactivityTimer.\n";
                    delete timer;
                } else {
                    PrintEvent("Poll Timer expired", neighbor->GetInterface(), neighbor);
                    neighbor->ProcessEvent(AnsaOSPF::Neighbor::PollTimer);
                }
            }
            break;
        case NeighborDDRetransmissionTimer:
            {
                AnsaOSPF::Neighbor* neighbor;
                if (! (neighbor = reinterpret_cast <AnsaOSPF::Neighbor*> (timer->getContextPointer()))) {
                    // should not reach this point
                    EV << "Discarding invalid NeighborDDRetransmissionTimer.\n";
                    delete timer;
                } else {
                    PrintEvent("Database Description Retransmission Timer expired", neighbor->GetInterface(), neighbor);
                    neighbor->ProcessEvent(AnsaOSPF::Neighbor::DDRetransmissionTimer);
                }
            }
            break;
        case NeighborUpdateRetransmissionTimer:
            {
                AnsaOSPF::Neighbor* neighbor;
                if (! (neighbor = reinterpret_cast <AnsaOSPF::Neighbor*> (timer->getContextPointer()))) {
                    // should not reach this point
                    EV << "Discarding invalid NeighborUpdateRetransmissionTimer.\n";
                    delete timer;
                } else {
                    PrintEvent("Update Retransmission Timer expired", neighbor->GetInterface(), neighbor);
                    neighbor->ProcessEvent(AnsaOSPF::Neighbor::UpdateRetransmissionTimer);
                }
            }
            break;
        case NeighborRequestRetransmissionTimer:
            {
                AnsaOSPF::Neighbor* neighbor;
                if (! (neighbor = reinterpret_cast <AnsaOSPF::Neighbor*> (timer->getContextPointer()))) {
                    // should not reach this point
                    EV << "Discarding invalid NeighborRequestRetransmissionTimer.\n";
                    delete timer;
                } else {
                    PrintEvent("Request Retransmission Timer expired", neighbor->GetInterface(), neighbor);
                    neighbor->ProcessEvent(AnsaOSPF::Neighbor::RequestRetransmissionTimer);
                }
            }
            break;
        case DatabaseAgeTimer:
            {
                PrintEvent("Ageing the database");
                router->AgeDatabase();
            }
            break;
        default: break;
    }
}

void AnsaOSPF::MessageHandler::ProcessPacket(OSPFPacket* packet, AnsaOSPF::Interface* unused1, AnsaOSPF::Neighbor* unused2)
{
    // packet version must be OSPF version 2
    if (packet->getVersion() == 2) {
        IPControlInfo*  controlInfo = check_and_cast<IPControlInfo *> (packet->getControlInfo());
        int             interfaceId = controlInfo->getInterfaceId();
        AnsaOSPF::AreaID    areaID      = packet->getAreaID().getInt();
        AnsaOSPF::Area*     area        = router->GetArea(areaID);

        if (area != NULL) {
            // packet Area ID must either match the Area ID of the receiving interface or...
            AnsaOSPF::Interface* intf = area->GetInterface(interfaceId);

            if (intf == NULL) {
                // it must be the backbone area and...
                if (areaID == BackboneAreaID) {
                    if (router->GetAreaCount() > 1) {
                        // it must be a virtual link and the source router's router ID must be the endpoint of this virtual link and...
                        intf = area->FindVirtualLink(packet->getRouterID().getInt());

                        if (intf != NULL) {
                            AnsaOSPF::Area* virtualLinkTransitArea = router->GetArea(intf->GetTransitAreaID());

                            if (virtualLinkTransitArea != NULL) {
                                // the receiving interface must attach to the virtual link's configured transit area
                                AnsaOSPF::Interface* virtualLinkInterface = virtualLinkTransitArea->GetInterface(interfaceId);

                                if (virtualLinkInterface == NULL) {
                                    intf = NULL;
                                }
                            } else {
                                intf = NULL;
                            }
                        }
                    }
                }
            }
            if (intf != NULL) {
                unsigned long                       destinationAddress = controlInfo->getDestAddr().getInt();
                unsigned long                       allDRouters        = ULongFromIPv4Address(AnsaOSPF::AllDRouters);
                AnsaOSPF::Interface::InterfaceStateType interfaceState     = intf->GetState();

                // if destination address is AllDRouters the receiving interface must be in DesignatedRouter or Backup state
                if (
                    ((destinationAddress == allDRouters) &&
                     (
                      (interfaceState == AnsaOSPF::Interface::DesignatedRouterState) ||
                      (interfaceState == AnsaOSPF::Interface::BackupState)
                     )
                    ) ||
                    (destinationAddress != allDRouters)
                   )
                {
                    // packet authentication
                    if (AuthenticatePacket(packet)) {
                        OSPFPacketType  packetType = static_cast<OSPFPacketType> (packet->getType());
                        AnsaOSPF::Neighbor* neighbor   = NULL;

                        // all packets except HelloPackets are sent only along adjacencies, so a Neighbor must exist
                        if (packetType != HelloPacket) {
                            switch (intf->GetType()) {
                                case AnsaOSPF::Interface::Broadcast:
                                case AnsaOSPF::Interface::NBMA:
                                case AnsaOSPF::Interface::PointToMultiPoint:
                                    neighbor = intf->GetNeighborByAddress(IPv4AddressFromULong(controlInfo->getSrcAddr().getInt()));
                                    break;
                                case AnsaOSPF::Interface::PointToPoint:
                                case AnsaOSPF::Interface::Virtual:
                                    neighbor = intf->GetNeighborByID(packet->getRouterID().getInt());
                                    break;
                                default: break;
                            }
                        }
                        switch (packetType) {
                            case HelloPacket:
                                helloHandler.ProcessPacket(packet, intf);
                                router->stat.AddHelloPacketReceived();
                                router->stat.AddOspfPacketReceived();
                                break;
                            case DatabaseDescriptionPacket:
                                if (neighbor != NULL) {
                                    ddHandler.ProcessPacket(packet, intf, neighbor);
                                    router->stat.AddOspfPacketReceived();
                                }
                                break;
                            case LinkStateRequestPacket:
                                if (neighbor != NULL) {
                                    lsRequestHandler.ProcessPacket(packet, intf, neighbor);
                                    router->stat.AddOspfPacketReceived();
                                }
                                break;
                            case LinkStateUpdatePacket:
                                if (neighbor != NULL) {
                                    lsUpdateHandler.ProcessPacket(packet, intf, neighbor);
                                    router->stat.AddOspfPacketReceived();
                                }
                                break;
                            case LinkStateAcknowledgementPacket:
                                if (neighbor != NULL) {
                                    lsAckHandler.ProcessPacket(packet, intf, neighbor);
                                    router->stat.AddOspfPacketReceived();
                                }
                                break;
                            default: break;
                        }
                    }
                }
            }
        }
    }
    delete packet;
}

void AnsaOSPF::MessageHandler::SendPacket(OSPFPacket* packet, IPv4Address destination, int outputIfIndex, short ttl)
{
    IPControlInfo *ipControlInfo = new IPControlInfo();
    ipControlInfo->setProtocol(IP_PROT_OSPF);
    ipControlInfo->setDestAddr(ULongFromIPv4Address(destination));
    ipControlInfo->setTimeToLive(ttl);
    ipControlInfo->setInterfaceId(outputIfIndex);

    packet->setControlInfo(ipControlInfo);
    switch (packet->getType()) {
        case HelloPacket:
            {
                packet->setKind(HelloPacket);
                packet->setName("OSPF_HelloPacket");

                OSPFHelloPacket* helloPacket = check_and_cast<OSPFHelloPacket*> (packet);
                PrintHelloPacket(helloPacket, destination, outputIfIndex);
                
                router->stat.AddHelloPacketSend();
            }
            break;
        case DatabaseDescriptionPacket:
            {
                packet->setKind(DatabaseDescriptionPacket);
                packet->setName("OSPF_DDPacket");

                OSPFDatabaseDescriptionPacket* ddPacket = check_and_cast<OSPFDatabaseDescriptionPacket*> (packet);
                PrintDatabaseDescriptionPacket(ddPacket, destination, outputIfIndex);
            }
            break;
        case LinkStateRequestPacket:
            {
                packet->setKind(LinkStateRequestPacket);
                packet->setName("OSPF_LSReqPacket");

                OSPFLinkStateRequestPacket* requestPacket = check_and_cast<OSPFLinkStateRequestPacket*> (packet);
                PrintLinkStateRequestPacket(requestPacket, destination, outputIfIndex);
            }
            break;
        case LinkStateUpdatePacket:
            {
                packet->setKind(LinkStateUpdatePacket);
                packet->setName("OSPF_LSUpdPacket");

                OSPFLinkStateUpdatePacket* updatePacket = check_and_cast<OSPFLinkStateUpdatePacket*> (packet);
                PrintLinkStateUpdatePacket(updatePacket, destination, outputIfIndex);
            }
            break;
        case LinkStateAcknowledgementPacket:
            {
                packet->setKind(LinkStateAcknowledgementPacket);
                packet->setName("OSPF_LSAckPacket");

                OSPFLinkStateAcknowledgementPacket* ackPacket = check_and_cast<OSPFLinkStateAcknowledgementPacket*> (packet);
                PrintLinkStateAcknowledgementPacket(ackPacket, destination, outputIfIndex);
            }
            break;
        default: break;
    }

    ospfModule->send(packet,"ipOut");
    router->stat.AddOspfPacketSend();
}

void AnsaOSPF::MessageHandler::ClearTimer(OSPFTimer* timer)
{
    ospfModule->cancelEvent(timer);
}

void AnsaOSPF::MessageHandler::StartTimer(OSPFTimer* timer, simtime_t delay)
{
    ospfModule->scheduleAt(simTime() + delay, timer);
}

void AnsaOSPF::MessageHandler::PrintEvent(const char* eventString, const AnsaOSPF::Interface* onInterface, const AnsaOSPF::Neighbor* forNeighbor /*= NULL*/) const
{
    EV << eventString;
    if ((onInterface != NULL) || (forNeighbor != NULL)) {
        EV << ": ";
    }
    if (forNeighbor != NULL) {
        char addressString[16];
        EV << "neighbor["
           << AddressStringFromULong(addressString, sizeof(addressString), forNeighbor->GetNeighborID())
           << "] (state: "
           << forNeighbor->GetStateString(forNeighbor->GetState())
           << "); ";
    }
    if (onInterface != NULL) {
        EV << "interface["
           << static_cast <short> (onInterface->GetIfIndex())
           << "] ";
        switch (onInterface->GetType()) {
            case AnsaOSPF::Interface::PointToPoint:      EV << "(PointToPoint)";
                                                     break;
            case AnsaOSPF::Interface::Broadcast:         EV << "(Broadcast)";
                                                     break;
            case AnsaOSPF::Interface::NBMA:              EV << "(NBMA).\n";
                                                     break;
            case AnsaOSPF::Interface::PointToMultiPoint: EV << "(PointToMultiPoint)";
                                                     break;
            case AnsaOSPF::Interface::Virtual:           EV << "(Virtual)";
                                                     break;
            default:                                 EV << "(Unknown)";
        }
        EV << " (state: "
           << onInterface->GetStateString(onInterface->GetState())
           << ")";
    }
    EV << ".\n";
}

void AnsaOSPF::MessageHandler::PrintHelloPacket(const OSPFHelloPacket* helloPacket, IPv4Address destination, int outputIfIndex) const
{
    char addressString[16];
    EV << "Sending Hello packet to "
       << AddressStringFromIPv4Address(addressString, sizeof(addressString), destination)
       << " on interface["
       << outputIfIndex
       << "] with contents:\n";
    EV << "  netMask="
       << AddressStringFromULong(addressString, sizeof(addressString), helloPacket->getNetworkMask().getInt())
       << "\n";
    EV << "  DR="
       << AddressStringFromULong(addressString, sizeof(addressString), helloPacket->getDesignatedRouter().getInt())
       << "\n";
    EV << "  BDR="
       << AddressStringFromULong(addressString, sizeof(addressString), helloPacket->getBackupDesignatedRouter().getInt())
       << "\n";
    EV << "  neighbors:\n";

    unsigned int neighborCount = helloPacket->getNeighborArraySize();
    for (unsigned int i = 0; i < neighborCount; i++) {
        EV << "    "
           << AddressStringFromULong(addressString, sizeof(addressString), helloPacket->getNeighbor(i).getInt())
           << "\n";
    }
}

void AnsaOSPF::MessageHandler::PrintDatabaseDescriptionPacket(const OSPFDatabaseDescriptionPacket* ddPacket, IPv4Address destination, int outputIfIndex) const
{
    char addressString[16];
    EV << "Sending Database Description packet to "
       << AddressStringFromIPv4Address(addressString, sizeof(addressString), destination)
       << " on interface["
       << outputIfIndex
       << "] with contents:\n";

    const OSPFDDOptions& ddOptions = ddPacket->getDdOptions();
    EV << "  ddOptions="
       << ((ddOptions.I_Init) ? "I " : "_ ")
       << ((ddOptions.M_More) ? "M " : "_ ")
       << ((ddOptions.MS_MasterSlave) ? "MS" : "__")
       << "\n";
    EV << "  seqNumber="
       << ddPacket->getDdSequenceNumber()
       << "\n";
    EV << "  LSA headers:\n";

    unsigned int lsaCount = ddPacket->getLsaHeadersArraySize();
    for (unsigned int i = 0; i < lsaCount; i++) {
        EV << "    ";
        PrintLSAHeader(ddPacket->getLsaHeaders(i), ev.getOStream());
        EV << "\n";
    }
}

void AnsaOSPF::MessageHandler::PrintLinkStateRequestPacket(const OSPFLinkStateRequestPacket* requestPacket, IPv4Address destination, int outputIfIndex) const
{
    char addressString[16];
    EV << "Sending Link State Request packet to "
       << AddressStringFromIPv4Address(addressString, sizeof(addressString), destination)
       << " on interface["
       << outputIfIndex
       << "] with requests:\n";

    unsigned int requestCount = requestPacket->getRequestsArraySize();
    for (unsigned int i = 0; i < requestCount; i++) {
        const LSARequest& request = requestPacket->getRequests(i);
        EV << "  type="
           << request.lsType
           << ", LSID="
           << AddressStringFromULong(addressString, sizeof(addressString), request.linkStateID);
        EV << ", advertisingRouter="
           << AddressStringFromULong(addressString, sizeof(addressString), request.advertisingRouter.getInt())
           << "\n";
    }
}

void AnsaOSPF::MessageHandler::PrintLinkStateUpdatePacket(const OSPFLinkStateUpdatePacket* updatePacket, IPv4Address destination, int outputIfIndex) const
{
    char addressString[16];
    EV << "Sending Link State Update packet to "
       << AddressStringFromIPv4Address(addressString, sizeof(addressString), destination)
       << " on interface["
       << outputIfIndex
       << "] with updates:\n";

    unsigned int i           = 0;
    unsigned int updateCount = updatePacket->getRouterLSAsArraySize();

    for (i = 0; i < updateCount; i++) {
        const OSPFRouterLSA& lsa = updatePacket->getRouterLSAs(i);
        EV << "  ";
        PrintLSAHeader(lsa.getHeader(), ev.getOStream());
        EV << "\n";

        EV << "  bits="
           << ((lsa.getV_VirtualLinkEndpoint()) ? "V " : "_ ")
           << ((lsa.getE_ASBoundaryRouter()) ? "E " : "_ ")
           << ((lsa.getB_AreaBorderRouter()) ? "B" : "_")
           << "\n";
        EV << "  links:\n";

        unsigned int linkCount = lsa.getLinksArraySize();
        for (unsigned int j = 0; j < linkCount; j++) {
            const Link& link = lsa.getLinks(j);
            EV << "    ID="
               << AddressStringFromULong(addressString, sizeof(addressString), link.getLinkID().getInt())
               << ",";
            EV << " data="
               << AddressStringFromULong(addressString, sizeof(addressString), link.getLinkData())
               << ", type=";
            switch (link.getType()) {
                case PointToPointLink:  EV << "PointToPoint";   break;
                case TransitLink:       EV << "Transit";        break;
                case StubLink:          EV << "Stub";           break;
                case VirtualLink:       EV << "Virtual";        break;
                default:                EV << "Unknown";        break;
            }
            EV << ", cost="
               << link.getLinkCost()
               << "\n";
        }
    }

    updateCount = updatePacket->getNetworkLSAsArraySize();
    for (i = 0; i < updateCount; i++) {
        const OSPFNetworkLSA& lsa = updatePacket->getNetworkLSAs(i);
        EV << "  ";
        PrintLSAHeader(lsa.getHeader(), ev.getOStream());
        EV << "\n";

        EV << "  netMask="
           << AddressStringFromULong(addressString, sizeof(addressString), lsa.getNetworkMask().getInt())
           << "\n";
        EV << "  attachedRouters:\n";

        unsigned int routerCount = lsa.getAttachedRoutersArraySize();
        for (unsigned int j = 0; j < routerCount; j++) {
            EV << "    "
               << AddressStringFromULong(addressString, sizeof(addressString), lsa.getAttachedRouters(j).getInt())
               << "\n";
        }
    }

    updateCount = updatePacket->getSummaryLSAsArraySize();
    for (i = 0; i < updateCount; i++) {
        const OSPFSummaryLSA& lsa = updatePacket->getSummaryLSAs(i);
        EV << "  ";
        PrintLSAHeader(lsa.getHeader(), ev.getOStream());
        EV << "\n";

        EV << "  netMask="
           << AddressStringFromULong(addressString, sizeof(addressString), lsa.getNetworkMask().getInt())
           << "\n";
        EV << "  cost="
           << lsa.getRouteCost()
           << "\n";
    }

    updateCount = updatePacket->getAsExternalLSAsArraySize();
    for (i = 0; i < updateCount; i++) {
        const OSPFASExternalLSA& lsa = updatePacket->getAsExternalLSAs(i);
        EV << "  ";
        PrintLSAHeader(lsa.getHeader(), ev.getOStream());
        EV << "\n";

        const OSPFASExternalLSAContents& contents = lsa.getContents();
        EV << "  netMask="
           << AddressStringFromULong(addressString, sizeof(addressString), contents.getNetworkMask().getInt())
           << "\n";
        EV << "  bits="
           << ((contents.getE_ExternalMetricType()) ? "E\n" : "_\n");
        EV << "  cost="
           << contents.getRouteCost()
           << "\n";
        EV << "  forward="
           << AddressStringFromULong(addressString, sizeof(addressString), contents.getForwardingAddress().getInt())
           << "\n";
    }
}

void AnsaOSPF::MessageHandler::PrintLinkStateAcknowledgementPacket(const OSPFLinkStateAcknowledgementPacket* ackPacket, IPv4Address destination, int outputIfIndex) const
{
    char addressString[16];
    EV << "Sending Link State Acknowledgement packet to "
       << AddressStringFromIPv4Address(addressString, sizeof(addressString), destination)
       << " on interface["
       << outputIfIndex
       << "] with acknowledgements:\n";

    unsigned int lsaCount = ackPacket->getLsaHeadersArraySize();
    for (unsigned int i = 0; i < lsaCount; i++) {
        EV << "    ";
        PrintLSAHeader(ackPacket->getLsaHeaders(i), ev.getOStream());
        EV << "\n";
    }
}

