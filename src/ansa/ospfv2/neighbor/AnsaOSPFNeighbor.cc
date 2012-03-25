#include "AnsaOSPFNeighbor.h"
#include "AnsaOSPFNeighborState.h"
#include "AnsaOSPFNeighborStateDown.h"
#include "AnsaMessageHandler.h"
#include "AnsaOSPFArea.h"
#include "AnsaOSPFRouter.h"
#include <memory.h>

// FIXME!!! Should come from a global unique number generator module.
unsigned long AnsaOSPF::Neighbor::ddSequenceNumberInitSeed = 0;

AnsaOSPF::Neighbor::Neighbor(RouterID neighbor) :
    updateRetransmissionTimerActive(false),
    requestRetransmissionTimerActive(false),
    firstAdjacencyInited(false),
    ddSequenceNumber(0),
    neighborID(neighbor),
    neighborPriority(0),
    neighborIPAddress(AnsaOSPF::NullIPv4Address),
    neighborsDesignatedRouter(AnsaOSPF::NullDesignatedRouterID),
    neighborsBackupDesignatedRouter(AnsaOSPF::NullDesignatedRouterID),
    designatedRoutersSetUp(false),
    neighborsRouterDeadInterval(40),
    lastTransmittedDDPacket(NULL)
{
    memset(&lastReceivedDDPacket, 0, sizeof(AnsaOSPF::Neighbor::DDPacketID));
    // setting only I and M bits is invalid -> good initializer
    lastReceivedDDPacket.ddOptions.I_Init = true;
    lastReceivedDDPacket.ddOptions.M_More = true;
    inactivityTimer = new OSPFTimer;
    inactivityTimer->setTimerKind(NeighborInactivityTimer);
    inactivityTimer->setContextPointer(this);
    inactivityTimer->setName("AnsaOSPF::Neighbor::NeighborInactivityTimer");
    pollTimer = new OSPFTimer;
    pollTimer->setTimerKind(NeighborPollTimer);
    pollTimer->setContextPointer(this);
    pollTimer->setName("AnsaOSPF::Neighbor::NeighborPollTimer");
    ddRetransmissionTimer = new OSPFTimer;
    ddRetransmissionTimer->setTimerKind(NeighborDDRetransmissionTimer);
    ddRetransmissionTimer->setContextPointer(this);
    ddRetransmissionTimer->setName("AnsaOSPF::Neighbor::NeighborDDRetransmissionTimer");
    updateRetransmissionTimer = new OSPFTimer;
    updateRetransmissionTimer->setTimerKind(NeighborUpdateRetransmissionTimer);
    updateRetransmissionTimer->setContextPointer(this);
    updateRetransmissionTimer->setName("AnsaOSPF::Neighbor::Neighbor::NeighborUpdateRetransmissionTimer");
    requestRetransmissionTimer = new OSPFTimer;
    requestRetransmissionTimer->setTimerKind(NeighborRequestRetransmissionTimer);
    requestRetransmissionTimer->setContextPointer(this);
    requestRetransmissionTimer->setName("AnsaOSPF::Neighbor::NeighborRequestRetransmissionTimer");
    state = new AnsaOSPF::NeighborStateDown;
    previousState = NULL;
}

AnsaOSPF::Neighbor::~Neighbor(void)
{
    Reset();
    MessageHandler* messageHandler = parentInterface->GetArea()->GetRouter()->GetMessageHandler();
    messageHandler->ClearTimer(inactivityTimer);
    messageHandler->ClearTimer(pollTimer);
    delete inactivityTimer;
    delete pollTimer;
    delete ddRetransmissionTimer;
    delete updateRetransmissionTimer;
    delete requestRetransmissionTimer;
    if (previousState != NULL) {
        delete previousState;
    }
    delete state;
}

void AnsaOSPF::Neighbor::ChangeState(NeighborState* newState, NeighborState* currentState)
{
    if (previousState != NULL) {
        delete previousState;
    }
    state = newState;
    previousState = currentState;
}

void AnsaOSPF::Neighbor::ProcessEvent(AnsaOSPF::Neighbor::NeighborEventType event)
{
    state->ProcessEvent(this, event);
}

void AnsaOSPF::Neighbor::Reset(void)
{
    for (std::list<OSPFLSA*>::iterator retIt = linkStateRetransmissionList.begin();
         retIt != linkStateRetransmissionList.end();
         retIt++)
    {
        delete(*retIt);
    }
    linkStateRetransmissionList.clear();

    std::list<OSPFLSAHeader*>::iterator it;
    for (it = databaseSummaryList.begin(); it != databaseSummaryList.end(); it++) {
        delete(*it);
    }
    databaseSummaryList.clear();
    for (it = linkStateRequestList.begin(); it != linkStateRequestList.end(); it++)
    {
        delete(*it);
    }
    linkStateRequestList.clear();

    parentInterface->GetArea()->GetRouter()->GetMessageHandler()->ClearTimer(ddRetransmissionTimer);
    ClearUpdateRetransmissionTimer();
    ClearRequestRetransmissionTimer();

    if (lastTransmittedDDPacket != NULL) {
        delete lastTransmittedDDPacket;
        lastTransmittedDDPacket = NULL;
    }
}

void AnsaOSPF::Neighbor::InitFirstAdjacency(void)
{
    ddSequenceNumber = GetUniqueULong();
    firstAdjacencyInited = true;
}

unsigned long AnsaOSPF::Neighbor::GetUniqueULong(void)
{
    // FIXME!!! Should come from a global unique number generator module.
    return (ddSequenceNumberInitSeed++);
}

AnsaOSPF::Neighbor::NeighborStateType AnsaOSPF::Neighbor::GetState(void) const
{
    return state->GetState();
}

const char* AnsaOSPF::Neighbor::GetStateString(AnsaOSPF::Neighbor::NeighborStateType stateType)
{
    switch (stateType) {
        case DownState:             return "Down";
        case AttemptState:          return "Attempt";
        case InitState:             return "Init";
        case TwoWayState:           return "TwoWay";
        case ExchangeStartState:    return "ExchangeStart";
        case ExchangeState:         return "Exchange";
        case LoadingState:          return "Loading";
        case FullState:             return "Full";
        default:                    ASSERT(false);
    }
    return "";
}

void AnsaOSPF::Neighbor::SendDatabaseDescriptionPacket(bool init)
{
    OSPFDatabaseDescriptionPacket* ddPacket = new OSPFDatabaseDescriptionPacket;

    ddPacket->setType(DatabaseDescriptionPacket);
    ddPacket->setRouterID(parentInterface->GetArea()->GetRouter()->GetRouterID());
    ddPacket->setAreaID(parentInterface->GetArea()->GetAreaID());
    ddPacket->setAuthenticationType(parentInterface->GetAuthenticationType());
    AnsaOSPF::AuthenticationKeyType authKey = parentInterface->GetAuthenticationKey();
    for (int i = 0; i < 8; i++) {
        ddPacket->setAuthentication(i, authKey.bytes[i]);
    }

    if (parentInterface->GetType() != AnsaOSPF::Interface::Virtual) {
        ddPacket->setInterfaceMTU(parentInterface->GetMTU());
    } else {
        ddPacket->setInterfaceMTU(0);
    }

    OSPFOptions options;
    memset(&options, 0, sizeof(OSPFOptions));
    options.E_ExternalRoutingCapability = parentInterface->GetArea()->GetExternalRoutingCapability();
    ddPacket->setOptions(options);

    ddPacket->setDdSequenceNumber(ddSequenceNumber);

    long maxPacketSize = ((IPV4_HEADER_LENGTH + OSPF_HEADER_LENGTH + OSPF_DD_HEADER_LENGTH + OSPF_LSA_HEADER_LENGTH) > parentInterface->GetMTU()) ?
                          IPV4_DATAGRAM_LENGTH :
                          parentInterface->GetMTU();

    if (init || databaseSummaryList.empty()) {
        ddPacket->setLsaHeadersArraySize(0);
    } else {
        // delete included LSAs from summary list
        // (they are still in lastTransmittedDDPacket)
        long packetSize = IPV4_HEADER_LENGTH + OSPF_HEADER_LENGTH + OSPF_DD_HEADER_LENGTH;
        while ((!databaseSummaryList.empty()) && (packetSize <= (maxPacketSize - OSPF_LSA_HEADER_LENGTH))) {
            unsigned long   headerCount = ddPacket->getLsaHeadersArraySize();
            OSPFLSAHeader*  lsaHeader   = *(databaseSummaryList.begin());
            ddPacket->setLsaHeadersArraySize(headerCount + 1);
            ddPacket->setLsaHeaders(headerCount, *lsaHeader);
            delete lsaHeader;
            databaseSummaryList.pop_front();
            packetSize += OSPF_LSA_HEADER_LENGTH;
        }
    }

    OSPFDDOptions ddOptions;
    memset(&ddOptions, 0, sizeof(OSPFDDOptions));
    if (init) {
        ddOptions.I_Init = true;
        ddOptions.M_More = true;
        ddOptions.MS_MasterSlave = true;
    } else {
        ddOptions.I_Init = false;
        ddOptions.M_More = (databaseSummaryList.empty()) ? false : true;
        ddOptions.MS_MasterSlave = (databaseExchangeRelationship == AnsaOSPF::Neighbor::Master) ? true : false;
    }
    ddPacket->setDdOptions(ddOptions);

    ddPacket->setPacketLength(0); // TODO: Calculate correct length
    ddPacket->setChecksum(0); // TODO: Calculate correct cheksum(16-bit one's complement of the entire packet)

    AnsaOSPF::MessageHandler* messageHandler = parentInterface->GetArea()->GetRouter()->GetMessageHandler();
    int ttl = (parentInterface->GetType() == AnsaOSPF::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;
    if (parentInterface->GetType() == AnsaOSPF::Interface::PointToPoint) {
        messageHandler->SendPacket(ddPacket, AnsaOSPF::AllSPFRouters, parentInterface->GetIfIndex(), ttl);
    } else {
        messageHandler->SendPacket(ddPacket, neighborIPAddress, parentInterface->GetIfIndex(), ttl);
    }

    if (lastTransmittedDDPacket != NULL) {
        delete lastTransmittedDDPacket;
    }
    lastTransmittedDDPacket = new OSPFDatabaseDescriptionPacket(*ddPacket);
}

bool AnsaOSPF::Neighbor::RetransmitDatabaseDescriptionPacket(void)
{
    if (lastTransmittedDDPacket != NULL) {
        OSPFDatabaseDescriptionPacket* ddPacket       = new OSPFDatabaseDescriptionPacket(*lastTransmittedDDPacket);
        AnsaOSPF::MessageHandler*          messageHandler = parentInterface->GetArea()->GetRouter()->GetMessageHandler();
        int                            ttl            = (parentInterface->GetType() == AnsaOSPF::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;

        if (parentInterface->GetType() == AnsaOSPF::Interface::PointToPoint) {
            messageHandler->SendPacket(ddPacket, AnsaOSPF::AllSPFRouters, parentInterface->GetIfIndex(), ttl);
        } else {
            messageHandler->SendPacket(ddPacket, neighborIPAddress, parentInterface->GetIfIndex(), ttl);
        }

        return true;
    } else {
        return false;
    }
}

void AnsaOSPF::Neighbor::CreateDatabaseSummary(void)
{
    AnsaOSPF::Area*   area           = parentInterface->GetArea();
    unsigned long routerLSACount = area->GetRouterLSACount();

    /* Note: OSPF specification says:
     * "LSAs whose age is equal to MaxAge are instead added to the neighbor's
     *  Link state retransmission list."
     * But this task has been already done during the aging of the database. (???)
     * So we'll skip this.
     */
    for (unsigned long i = 0; i < routerLSACount; i++) {
        if (area->GetRouterLSA(i)->getHeader().getLsAge() < MAX_AGE) {
            OSPFLSAHeader* routerLSA = new OSPFLSAHeader(area->GetRouterLSA(i)->getHeader());
            databaseSummaryList.push_back(routerLSA);
        }
    }

    unsigned long networkLSACount = area->GetNetworkLSACount();
    for (unsigned long j = 0; j < networkLSACount; j++) {
        if (area->GetNetworkLSA(j)->getHeader().getLsAge() < MAX_AGE) {
            OSPFLSAHeader* networkLSA = new OSPFLSAHeader(area->GetNetworkLSA(j)->getHeader());
            databaseSummaryList.push_back(networkLSA);
        }
    }

    unsigned long summaryLSACount = area->GetSummaryLSACount();
    for (unsigned long k = 0; k < summaryLSACount; k++) {
        if (area->GetSummaryLSA(k)->getHeader().getLsAge() < MAX_AGE) {
            OSPFLSAHeader* summaryLSA = new OSPFLSAHeader(area->GetSummaryLSA(k)->getHeader());
            databaseSummaryList.push_back(summaryLSA);
        }
    }

    if ((parentInterface->GetType() != AnsaOSPF::Interface::Virtual) &&
        (area->GetExternalRoutingCapability()))
    {
        AnsaOSPF::Router* router             = area->GetRouter();
        unsigned long asExternalLSACount = router->GetASExternalLSACount();

        for (unsigned long m = 0; m < asExternalLSACount; m++) {
            if (router->GetASExternalLSA(m)->getHeader().getLsAge() < MAX_AGE) {
                OSPFLSAHeader* asExternalLSA = new OSPFLSAHeader(router->GetASExternalLSA(m)->getHeader());
                databaseSummaryList.push_back(asExternalLSA);
            }
        }
    }
}

void AnsaOSPF::Neighbor::SendLinkStateRequestPacket(void)
{
    OSPFLinkStateRequestPacket* requestPacket = new OSPFLinkStateRequestPacket;

    requestPacket->setType(LinkStateRequestPacket);
    requestPacket->setRouterID(parentInterface->GetArea()->GetRouter()->GetRouterID());
    requestPacket->setAreaID(parentInterface->GetArea()->GetAreaID());
    requestPacket->setAuthenticationType(parentInterface->GetAuthenticationType());
    AnsaOSPF::AuthenticationKeyType authKey = parentInterface->GetAuthenticationKey();
    for (int i = 0; i < 8; i++) {
        requestPacket->setAuthentication(i, authKey.bytes[i]);
    }

    long maxPacketSize = ((IPV4_HEADER_LENGTH + OSPF_HEADER_LENGTH + OSPF_REQUEST_LENGTH) > parentInterface->GetMTU()) ?
                          IPV4_DATAGRAM_LENGTH :
                          parentInterface->GetMTU();

    if (linkStateRequestList.empty()) {
        requestPacket->setRequestsArraySize(0);
    } else {
        long packetSize = IPV4_HEADER_LENGTH + OSPF_HEADER_LENGTH;
        std::list<OSPFLSAHeader*>::iterator it = linkStateRequestList.begin();

        while ((it != linkStateRequestList.end()) && (packetSize <= (maxPacketSize - OSPF_REQUEST_LENGTH))) {
            unsigned long  requestCount  = requestPacket->getRequestsArraySize();
            OSPFLSAHeader* requestHeader = (*it);
            LSARequest     request;

            request.lsType = requestHeader->getLsType();
            request.linkStateID = requestHeader->getLinkStateID();
            request.advertisingRouter = requestHeader->getAdvertisingRouter();

            requestPacket->setRequestsArraySize(requestCount + 1);
            requestPacket->setRequests(requestCount, request);

            packetSize += OSPF_REQUEST_LENGTH;
            it++;
        }
    }

    requestPacket->setPacketLength(0); // TODO: Calculate correct length
    requestPacket->setChecksum(0); // TODO: Calculate correct cheksum(16-bit one's complement of the entire packet)

    AnsaOSPF::MessageHandler* messageHandler = parentInterface->GetArea()->GetRouter()->GetMessageHandler();
    int ttl = (parentInterface->GetType() == AnsaOSPF::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;
    if (parentInterface->GetType() == AnsaOSPF::Interface::PointToPoint) {
        messageHandler->SendPacket(requestPacket, AnsaOSPF::AllSPFRouters, parentInterface->GetIfIndex(), ttl);
    } else {
        messageHandler->SendPacket(requestPacket, neighborIPAddress, parentInterface->GetIfIndex(), ttl);
    }
}

bool AnsaOSPF::Neighbor::NeedAdjacency(void)
{
    AnsaOSPF::Interface::OSPFInterfaceType interfaceType = parentInterface->GetType();
    AnsaOSPF::RouterID                     routerID      = parentInterface->GetArea()->GetRouter()->GetRouterID();
    AnsaOSPF::DesignatedRouterID           dRouter       = parentInterface->GetDesignatedRouter();
    AnsaOSPF::DesignatedRouterID           backupDRouter = parentInterface->GetBackupDesignatedRouter();

    if ((interfaceType == AnsaOSPF::Interface::PointToPoint) ||
        (interfaceType == AnsaOSPF::Interface::PointToMultiPoint) ||
        (interfaceType == AnsaOSPF::Interface::Virtual) ||
        (dRouter.routerID == routerID) ||
        (backupDRouter.routerID == routerID) ||
        ((neighborsDesignatedRouter.routerID == dRouter.routerID) ||
         (!designatedRoutersSetUp &&
          (neighborsDesignatedRouter.ipInterfaceAddress == dRouter.ipInterfaceAddress))) ||
        ((neighborsBackupDesignatedRouter.routerID == backupDRouter.routerID) ||
         (!designatedRoutersSetUp &&
          (neighborsBackupDesignatedRouter.ipInterfaceAddress == backupDRouter.ipInterfaceAddress))))
    {
        return true;
    } else {
        return false;
    }
}

/**
 * If the LSA is already on the retransmission list then it is replaced, else
 * a copy of the LSA is added to the end of the retransmission list.
 * @param lsa [in] The LSA to be added.
 */
void AnsaOSPF::Neighbor::AddToRetransmissionList(OSPFLSA* lsa)
{
    std::list<OSPFLSA*>::iterator it;
    for (it = linkStateRetransmissionList.begin(); it != linkStateRetransmissionList.end(); it++) {
        if (((*it)->getHeader().getLinkStateID() == lsa->getHeader().getLinkStateID()) &&
            ((*it)->getHeader().getAdvertisingRouter().getInt() == lsa->getHeader().getAdvertisingRouter().getInt()))
        {
            break;
        }
    }

    OSPFLSA* lsaCopy = NULL;
    switch (lsa->getHeader().getLsType()) {
        case RouterLSAType:
            lsaCopy = new OSPFRouterLSA(*(check_and_cast<OSPFRouterLSA*> (lsa)));
            break;
        case NetworkLSAType:
            lsaCopy = new OSPFNetworkLSA(*(check_and_cast<OSPFNetworkLSA*> (lsa)));
            break;
        case SummaryLSA_NetworksType:
        case SummaryLSA_ASBoundaryRoutersType:
            lsaCopy = new OSPFSummaryLSA(*(check_and_cast<OSPFSummaryLSA*> (lsa)));
            break;
        case ASExternalLSAType:
            lsaCopy = new OSPFASExternalLSA(*(check_and_cast<OSPFASExternalLSA*> (lsa)));
            break;
        default:
            ASSERT(false); // error
            break;
    }

    if (it != linkStateRetransmissionList.end()) {
        delete(*it);
        *it = static_cast<OSPFLSA*> (lsaCopy);
    } else {
        linkStateRetransmissionList.push_back(static_cast<OSPFLSA*> (lsaCopy));
    }
}

void AnsaOSPF::Neighbor::RemoveFromRetransmissionList(AnsaOSPF::LSAKeyType lsaKey)
{
    std::list<OSPFLSA*>::iterator it = linkStateRetransmissionList.begin();
    while (it != linkStateRetransmissionList.end()) {
        if (((*it)->getHeader().getLinkStateID() == lsaKey.linkStateID) &&
            ((*it)->getHeader().getAdvertisingRouter().getInt() == lsaKey.advertisingRouter))
        {
            delete(*it);
            it = linkStateRetransmissionList.erase(it);
        } else {
            it++;
        }
    }
}

bool AnsaOSPF::Neighbor::IsLSAOnRetransmissionList(AnsaOSPF::LSAKeyType lsaKey) const
{
    for (std::list<OSPFLSA*>::const_iterator it = linkStateRetransmissionList.begin(); it != linkStateRetransmissionList.end(); it++) {
        const OSPFLSA* lsa = *it;
        if ((lsa->getHeader().getLinkStateID() == lsaKey.linkStateID) &&
            (lsa->getHeader().getAdvertisingRouter().getInt() == lsaKey.advertisingRouter))
        {
            return true;
        }
    }
    return false;
}

OSPFLSA* AnsaOSPF::Neighbor::FindOnRetransmissionList(AnsaOSPF::LSAKeyType lsaKey)
{
    for (std::list<OSPFLSA*>::iterator it = linkStateRetransmissionList.begin(); it != linkStateRetransmissionList.end(); it++) {
        if (((*it)->getHeader().getLinkStateID() == lsaKey.linkStateID) &&
            ((*it)->getHeader().getAdvertisingRouter().getInt() == lsaKey.advertisingRouter))
        {
            return (*it);
        }
    }
    return NULL;
}

void AnsaOSPF::Neighbor::StartUpdateRetransmissionTimer(void)
{
    MessageHandler* messageHandler = parentInterface->GetArea()->GetRouter()->GetMessageHandler();
    messageHandler->StartTimer(updateRetransmissionTimer, parentInterface->GetRetransmissionInterval());
    updateRetransmissionTimerActive = true;
}

void AnsaOSPF::Neighbor::ClearUpdateRetransmissionTimer(void)
{
    MessageHandler* messageHandler = parentInterface->GetArea()->GetRouter()->GetMessageHandler();
    messageHandler->ClearTimer(updateRetransmissionTimer);
    updateRetransmissionTimerActive = false;
}

void AnsaOSPF::Neighbor::AddToRequestList(OSPFLSAHeader* lsaHeader)
{
    linkStateRequestList.push_back(new OSPFLSAHeader(*lsaHeader));
}

void AnsaOSPF::Neighbor::RemoveFromRequestList(AnsaOSPF::LSAKeyType lsaKey)
{
    std::list<OSPFLSAHeader*>::iterator it = linkStateRequestList.begin();
    while (it != linkStateRequestList.end()) {
        if (((*it)->getLinkStateID() == lsaKey.linkStateID) &&
            ((*it)->getAdvertisingRouter().getInt() == lsaKey.advertisingRouter))
        {
            delete(*it);
            it = linkStateRequestList.erase(it);
        } else {
            it++;
        }
    }

    if ((GetState() == AnsaOSPF::Neighbor::LoadingState) && (linkStateRequestList.empty())) {
        ClearRequestRetransmissionTimer();
        ProcessEvent(AnsaOSPF::Neighbor::LoadingDone);
    }
}

bool AnsaOSPF::Neighbor::IsLSAOnRequestList(AnsaOSPF::LSAKeyType lsaKey) const
{
    for (std::list<OSPFLSAHeader*>::const_iterator it = linkStateRequestList.begin(); it != linkStateRequestList.end(); it++) {
        const OSPFLSAHeader* lsaHeader = *it;
        if ((lsaHeader->getLinkStateID() == lsaKey.linkStateID) &&
            (lsaHeader->getAdvertisingRouter().getInt() == lsaKey.advertisingRouter))
        {
            return true;
        }
    }
    return false;
}

OSPFLSAHeader* AnsaOSPF::Neighbor::FindOnRequestList(AnsaOSPF::LSAKeyType lsaKey)
{
    for (std::list<OSPFLSAHeader*>::iterator it = linkStateRequestList.begin(); it != linkStateRequestList.end(); it++) {
        if (((*it)->getLinkStateID() == lsaKey.linkStateID) &&
            ((*it)->getAdvertisingRouter().getInt() == lsaKey.advertisingRouter))
        {
            return (*it);
        }
    }
    return NULL;
}

void AnsaOSPF::Neighbor::StartRequestRetransmissionTimer(void)
{
    MessageHandler* messageHandler = parentInterface->GetArea()->GetRouter()->GetMessageHandler();
    messageHandler->StartTimer(requestRetransmissionTimer, parentInterface->GetRetransmissionInterval());
    requestRetransmissionTimerActive = true;
}

void AnsaOSPF::Neighbor::ClearRequestRetransmissionTimer(void)
{
    MessageHandler* messageHandler = parentInterface->GetArea()->GetRouter()->GetMessageHandler();
    messageHandler->ClearTimer(requestRetransmissionTimer);
    requestRetransmissionTimerActive = false;
}

void AnsaOSPF::Neighbor::AddToTransmittedLSAList(AnsaOSPF::LSAKeyType lsaKey)
{
    TransmittedLSA transmit;

    transmit.lsaKey = lsaKey;
    transmit.age = 0;

    transmittedLSAs.push_back(transmit);
}

bool AnsaOSPF::Neighbor::IsOnTransmittedLSAList(AnsaOSPF::LSAKeyType lsaKey) const
{
    for (std::list<TransmittedLSA>::const_iterator it = transmittedLSAs.begin(); it != transmittedLSAs.end(); it++) {
        if ((it->lsaKey.linkStateID == lsaKey.linkStateID) &&
            (it->lsaKey.advertisingRouter == lsaKey.advertisingRouter))
        {
            return true;
        }
    }
    return false;
}

void AnsaOSPF::Neighbor::AgeTransmittedLSAList(void)
{
    std::list<TransmittedLSA>::iterator it = transmittedLSAs.begin();
    while ((it != transmittedLSAs.end()) && (it->age == MIN_LS_ARRIVAL)) {
        transmittedLSAs.pop_front();
        it = transmittedLSAs.begin();
    }
    for (it = transmittedLSAs.begin(); it != transmittedLSAs.end(); it++) {
        it->age++;
    }
}

void AnsaOSPF::Neighbor::RetransmitUpdatePacket(void)
{
    OSPFLinkStateUpdatePacket* updatePacket = new OSPFLinkStateUpdatePacket;

    updatePacket->setType(LinkStateUpdatePacket);
    updatePacket->setRouterID(parentInterface->GetArea()->GetRouter()->GetRouterID());
    updatePacket->setAreaID(parentInterface->GetArea()->GetAreaID());
    updatePacket->setAuthenticationType(parentInterface->GetAuthenticationType());
    AnsaOSPF::AuthenticationKeyType authKey = parentInterface->GetAuthenticationKey();
    for (int i = 0; i < 8; i++) {
        updatePacket->setAuthentication(i, authKey.bytes[i]);
    }

    bool                          packetFull   = false;
    unsigned short                lsaCount     = 0;
    unsigned long                 packetLength = IPV4_HEADER_LENGTH + OSPF_LSA_HEADER_LENGTH;
    std::list<OSPFLSA*>::iterator it           = linkStateRetransmissionList.begin();

    while (!packetFull && (it != linkStateRetransmissionList.end())) {
        LSAType            lsaType       = static_cast<LSAType> ((*it)->getHeader().getLsType());
        OSPFRouterLSA*     routerLSA     = (lsaType == RouterLSAType) ? dynamic_cast<OSPFRouterLSA*> (*it) : NULL;
        OSPFNetworkLSA*    networkLSA    = (lsaType == NetworkLSAType) ? dynamic_cast<OSPFNetworkLSA*> (*it) : NULL;
        OSPFSummaryLSA*    summaryLSA    = ((lsaType == SummaryLSA_NetworksType) ||
                                            (lsaType == SummaryLSA_ASBoundaryRoutersType)) ? dynamic_cast<OSPFSummaryLSA*> (*it) : NULL;
        OSPFASExternalLSA* asExternalLSA = (lsaType == ASExternalLSAType) ? dynamic_cast<OSPFASExternalLSA*> (*it) : NULL;
        long               lsaSize       = 0;
        bool               includeLSA    = false;

        switch (lsaType) {
            case RouterLSAType:
                if (routerLSA != NULL) {
                    lsaSize = CalculateLSASize(routerLSA);
                }
                break;
            case NetworkLSAType:
                if (networkLSA != NULL) {
                    lsaSize = CalculateLSASize(networkLSA);
                }
                break;
            case SummaryLSA_NetworksType:
            case SummaryLSA_ASBoundaryRoutersType:
                if (summaryLSA != NULL) {
                    lsaSize = CalculateLSASize(summaryLSA);
                }
                break;
            case ASExternalLSAType:
                if (asExternalLSA != NULL) {
                    lsaSize = CalculateLSASize(asExternalLSA);
                }
                break;
            default: break;
        }

        if (packetLength + lsaSize < parentInterface->GetMTU()) {
            includeLSA = true;
            lsaCount++;
        } else {
            if ((lsaCount == 0) && (packetLength + lsaSize < IPV4_DATAGRAM_LENGTH)) {
                includeLSA = true;
                lsaCount++;
                packetFull = true;
            }
        }

        if (includeLSA) {
            switch (lsaType) {
                case RouterLSAType:
                    if (routerLSA != NULL) {
                        unsigned int routerLSACount = updatePacket->getRouterLSAsArraySize();

                        updatePacket->setRouterLSAsArraySize(routerLSACount + 1);
                        updatePacket->setRouterLSAs(routerLSACount, *routerLSA);

                        unsigned short lsAge = updatePacket->getRouterLSAs(routerLSACount).getHeader().getLsAge();
                        if (lsAge < MAX_AGE - parentInterface->GetTransmissionDelay()) {
                            updatePacket->getRouterLSAs(routerLSACount).getHeader().setLsAge(lsAge + parentInterface->GetTransmissionDelay());
                        } else {
                            updatePacket->getRouterLSAs(routerLSACount).getHeader().setLsAge(MAX_AGE);
                        }
                    }
                    break;
                case NetworkLSAType:
                    if (networkLSA != NULL) {
                        unsigned int networkLSACount = updatePacket->getNetworkLSAsArraySize();

                        updatePacket->setNetworkLSAsArraySize(networkLSACount + 1);
                        updatePacket->setNetworkLSAs(networkLSACount, *networkLSA);

                        unsigned short lsAge = updatePacket->getNetworkLSAs(networkLSACount).getHeader().getLsAge();
                        if (lsAge < MAX_AGE - parentInterface->GetTransmissionDelay()) {
                            updatePacket->getNetworkLSAs(networkLSACount).getHeader().setLsAge(lsAge + parentInterface->GetTransmissionDelay());
                        } else {
                            updatePacket->getNetworkLSAs(networkLSACount).getHeader().setLsAge(MAX_AGE);
                        }
                    }
                    break;
                case SummaryLSA_NetworksType:
                case SummaryLSA_ASBoundaryRoutersType:
                    if (summaryLSA != NULL) {
                        unsigned int summaryLSACount = updatePacket->getSummaryLSAsArraySize();

                        updatePacket->setSummaryLSAsArraySize(summaryLSACount + 1);
                        updatePacket->setSummaryLSAs(summaryLSACount, *summaryLSA);

                        unsigned short lsAge = updatePacket->getSummaryLSAs(summaryLSACount).getHeader().getLsAge();
                        if (lsAge < MAX_AGE - parentInterface->GetTransmissionDelay()) {
                            updatePacket->getSummaryLSAs(summaryLSACount).getHeader().setLsAge(lsAge + parentInterface->GetTransmissionDelay());
                        } else {
                            updatePacket->getSummaryLSAs(summaryLSACount).getHeader().setLsAge(MAX_AGE);
                        }
                    }
                    break;
                case ASExternalLSAType:
                    if (asExternalLSA != NULL) {
                        unsigned int asExternalLSACount = updatePacket->getAsExternalLSAsArraySize();

                        updatePacket->setAsExternalLSAsArraySize(asExternalLSACount + 1);
                        updatePacket->setAsExternalLSAs(asExternalLSACount, *asExternalLSA);

                        unsigned short lsAge = updatePacket->getAsExternalLSAs(asExternalLSACount).getHeader().getLsAge();
                        if (lsAge < MAX_AGE - parentInterface->GetTransmissionDelay()) {
                            updatePacket->getAsExternalLSAs(asExternalLSACount).getHeader().setLsAge(lsAge + parentInterface->GetTransmissionDelay());
                        } else {
                            updatePacket->getAsExternalLSAs(asExternalLSACount).getHeader().setLsAge(MAX_AGE);
                        }
                    }
                    break;
                default: break;
            }
        }

        it++;
    }

    updatePacket->setPacketLength(0); // TODO: Calculate correct length
    updatePacket->setChecksum(0); // TODO: Calculate correct cheksum(16-bit one's complement of the entire packet)

    AnsaOSPF::MessageHandler* messageHandler = parentInterface->GetArea()->GetRouter()->GetMessageHandler();
    int ttl = (parentInterface->GetType() == AnsaOSPF::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;
    messageHandler->SendPacket(updatePacket, neighborIPAddress, parentInterface->GetIfIndex(), ttl);
}

void AnsaOSPF::Neighbor::DeleteLastSentDDPacket(void)
{
    if (lastTransmittedDDPacket != NULL) {
        delete lastTransmittedDDPacket;
        lastTransmittedDDPacket = NULL;
    }
}
