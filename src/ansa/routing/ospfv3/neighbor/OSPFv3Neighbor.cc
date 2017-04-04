#include "ansa/routing/ospfv3/neighbor/OSPFv3Neighbor.h"
#include "ansa/routing/ospfv3/neighbor/OSPFv3NeighborStateDown.h"

namespace inet{

// FIXME!!! Should come from a global unique number generator module.
unsigned long OSPFv3Neighbor::ddSequenceNumberInitSeed = 0;

OSPFv3Neighbor::OSPFv3Neighbor(IPv4Address newId, OSPFv3Interface* parent)
{
    EV_DEBUG << "$$$$$$ New OSPFv3Neighbor has been created\n";
    this->neighborId = newId;
    this->state = new OSPFv3NeighborStateDown;
    this->containingInterface = parent;
    this->neighborIPAddress = IPv6Address::UNSPECIFIED_ADDRESS;
    this->neighborsDesignatedRouter = NULL_IPV4ADDRESS;
    this->neighborsBackupDesignatedRouter = NULL_IPV4ADDRESS;
    this->neighborsDesignatedIP = IPv6Address::UNSPECIFIED_ADDRESS;
    this->neighborsBackupIP = IPv6Address::UNSPECIFIED_ADDRESS;
    this->neighborsRouterDeadInterval = DEFAULT_DEAD_INTERVAL;

    inactivityTimer = new cMessage();
    inactivityTimer->setKind(NEIGHBOR_INACTIVITY_TIMER);
    inactivityTimer->setContextPointer(this);
    inactivityTimer->setName("OSPFv3Neighbor::NeighborInactivityTimer");
    pollTimer = new cMessage();
    pollTimer->setKind(NEIGHBOR_POLL_TIMER);
    pollTimer->setContextPointer(this);
    pollTimer->setName("OSPFv3Neighbor::NeighborPollTimer");
    ddRetransmissionTimer = new cMessage();
    ddRetransmissionTimer->setKind(NEIGHBOR_DD_RETRANSMISSION_TIMER);
    ddRetransmissionTimer->setContextPointer(this);
    ddRetransmissionTimer->setName("OSPFv3Neighbor::NeighborDDRetransmissionTimer");
    updateRetransmissionTimer = new cMessage();
    updateRetransmissionTimer->setKind(NEIGHBOR_UPDATE_RETRANSMISSION_TIMER);
    updateRetransmissionTimer->setContextPointer(this);
    updateRetransmissionTimer->setName("OSPFv3Neighbor::Neighbor::NeighborUpdateRetransmissionTimer");
    requestRetransmissionTimer = new cMessage();
    requestRetransmissionTimer->setKind(NEIGHBOR_REQUEST_RETRANSMISSION_TIMER);
    requestRetransmissionTimer->setContextPointer(this);
    requestRetransmissionTimer->setName("OSPFv3sNeighbor::NeighborRequestRetransmissionTimer");
}//constructor

OSPFv3Neighbor::~OSPFv3Neighbor()
{

}//destructor
OSPFv3Neighbor::OSPFv3NeighborStateType OSPFv3Neighbor::getState() const
{
    return state->getState();
}

void OSPFv3Neighbor::processEvent(OSPFv3Neighbor::OSPFv3NeighborEventType event)
{
    EV_DEBUG << "Passing event number " << event << " to state\n";
    this->state->processEvent(this, event);
}

void OSPFv3Neighbor::changeState(OSPFv3NeighborState *newState, OSPFv3NeighborState *currentState)
{
    if (this->previousState != nullptr) {
        EV_DEBUG << "Changing neighbor from state" << currentState->getNeighborStateString() << " to " << newState->getNeighborStateString() << "\n";
        delete this->previousState;
    }
    this->state = newState;
    this->previousState = currentState;
}

void OSPFv3Neighbor::reset()
{
    EV_DEBUG << "Reseting the neighbor " << this->getNeighborID() << "\n";
    //TODO
    for (auto retIt = linkStateRetransmissionList.begin();
         retIt != linkStateRetransmissionList.end();
         retIt++)
    {
        delete (*retIt);
    }
    linkStateRetransmissionList.clear();

    for (auto it = databaseSummaryList.begin(); it != databaseSummaryList.end(); it++) {
        delete (*it);
    }
    databaseSummaryList.clear();
    for (auto it = linkStateRequestList.begin(); it != linkStateRequestList.end(); it++) {
        delete (*it);
    }
    linkStateRequestList.clear();

    this->getInterface()->getArea()->getInstance()->getProcess()->clearTimer(ddRetransmissionTimer);
    clearUpdateRetransmissionTimer();
    clearRequestRetransmissionTimer();

    if (lastTransmittedDDPacket != nullptr) {
        delete lastTransmittedDDPacket;
        lastTransmittedDDPacket = nullptr;
    }
}

bool OSPFv3Neighbor::needAdjacency()
{
    OSPFv3Interface::OSPFv3InterfaceType interfaceType = this->getInterface()->getType();
    IPv4Address routerID = this->getInterface()->getArea()->getInstance()->getProcess()->getRouterID();
    IPv4Address dRouter = this->getInterface()->getDesignatedID();
    IPv4Address backupDRouter = this->getInterface()->getBackupID();

    EV_DEBUG <<"My ID: " << routerID <<"\n";
    EV_DEBUG << "Inteface DR: " << dRouter << ", interface Backup: " << backupDRouter << ", neighborsDR: " << neighborsDesignatedRouter << ", neighborsBackup: "<<neighborsBackupDesignatedRouter << "\n";
    EV_DEBUG << "designatedSet " << designatedRoutersSetUp << "\n";

    if ((interfaceType == OSPFv3Interface::POINTTOPOINT_TYPE) ||
        (interfaceType == OSPFv3Interface::POINTTOMULTIPOINT_TYPE) ||
        (interfaceType == OSPFv3Interface::VIRTUAL_TYPE) ||
        (dRouter == routerID) ||
        (backupDRouter == routerID) ||
        (this->getNeighborID() == dRouter) ||
        (this->getNeighborID() == backupDRouter))
    {
        EV_DEBUG << "I need an adjacency with router " << this->getNeighborID()<<"\n";
        return true;
    }
    else {
        return false;
    }
}

void OSPFv3Neighbor::initFirstAdjacency()
{
    ddSequenceNumber = getUniqueULong();
    firstAdjacencyInited = true;
}

unsigned long OSPFv3Neighbor::getUniqueULong()
{
    // FIXME!!! Should come from a global unique number generator module.
    return ddSequenceNumberInitSeed++;
}

void OSPFv3Neighbor::startUpdateRetransmissionTimer()
{
    this->getInterface()->getArea()->getInstance()->getProcess()->setTimer(this->getUpdateRetransmissionTimer(), this->getInterface()->getRetransmissionInterval());
    this->updateRetransmissionTimerActive = true;
    EV_DEBUG << "Starting UPDATE TIMER\n";
}

void OSPFv3Neighbor::clearUpdateRetransmissionTimer()
{
    this->getInterface()->getArea()->getInstance()->getProcess()->clearTimer(this->getUpdateRetransmissionTimer());
    this->updateRetransmissionTimerActive = false;
}

void OSPFv3Neighbor::startRequestRetransmissionTimer()
{
    this->getInterface()->getArea()->getInstance()->getProcess()->setTimer(this->requestRetransmissionTimer, this->getInterface()->getRetransmissionInterval());
    this->requestRetransmissionTimerActive = true;
}

void OSPFv3Neighbor::clearRequestRetransmissionTimer()
{
    this->getInterface()->getArea()->getInstance()->getProcess()->clearTimer(this->getRequestRetransmissionTimer());
    this->requestRetransmissionTimerActive = false;
}

void OSPFv3Neighbor::setLastReceivedDDPacket(OSPFv3DatabaseDescription* ddPacket){
    OSPFv3DDOptions& ddOptions = ddPacket->getDdOptions();
    OSPFv3DDPacketID packetID;
    packetID.ddOptions = ddOptions;
    packetID.options = ddPacket->getOptions();
    packetID.sequenceNumber = ddPacket->getSequenceNumber();

    setOptions(packetID.options);
    setDDSequenceNumber(packetID.sequenceNumber);
    this->lastReceivedDDPacket = packetID;
}//setLastReceivedDDPacket

void OSPFv3Neighbor::sendDDPacket(bool init)
{
    OSPFv3DatabaseDescription* ddPacket = new OSPFv3DatabaseDescription();

    //common header first
    ddPacket->setType(DATABASE_DESCRIPTION);
    ddPacket->setRouterID(this->getInterface()->getArea()->getInstance()->getProcess()->getRouterID());
    ddPacket->setAreaID(this->getInterface()->getArea()->getAreaID());
    ddPacket->setInstanceID(this->getInterface()->getArea()->getInstance()->getInstanceID());

    //DD packet next
    OSPFv3Options options;
    memset(&options, 0, sizeof(OSPFv3Options));
    options.eBit = this->getInterface()->getArea()->getExternalRoutingCapability();
    ddPacket->setOptions(options);
    ddPacket->setInterfaceMTU(this->getInterface()->getInterfaceMTU());
    OSPFv3DDOptions ddOptions;
    ddPacket->setSequenceNumber(this->ddSequenceNumber);

    if(init){
        ddOptions.iBit = true;
        ddOptions.mBit = true;
        ddOptions.msBit = true;
    }
    else{
        ddOptions.iBit = false;
        ddOptions.mBit = (databaseSummaryList.empty()) ? false : true;
        ddOptions.msBit = (databaseExchangeRelationship == OSPFv3Neighbor::MASTER) ? true : false;
    }

    ddPacket->setDdOptions(ddOptions);
    int packetSize = OSPFV3_HEADER_LENGTH + OSPFV3_DD_HEADER_LENGTH;

    if (init || databaseSummaryList.empty()) {
        ddPacket->setLsaHeadersArraySize(0);
    }
    else{
        while (!this->databaseSummaryList.empty()){/// && (packetSize <= (maxPacketSize - OSPF_LSA_HEADER_LENGTH))) {
            unsigned long headerCount = ddPacket->getLsaHeadersArraySize();
            OSPFv3LSAHeader *lsaHeader = *(databaseSummaryList.begin());
            ddPacket->setLsaHeadersArraySize(headerCount + 1);
            ddPacket->setLsaHeaders(headerCount, *lsaHeader);
            delete lsaHeader;
            databaseSummaryList.pop_front();
            packetSize += OSPFV3_LSA_HEADER_LENGTH;
        }
    }

    ddPacket->setPacketLength(packetSize);

    //if(this->lastTransmittedDDPacket != nullptr)
        //delete this->lastTransmittedDDPacket;

    //this->lastTransmittedDDPacket = ddPacket;
    //TODO - virtual link hopLimit
    //TODO - checksum

    if(this->getInterface()->getType() == OSPFv3Interface::POINTTOPOINT_TYPE)
        this->getInterface()->getArea()->getInstance()->getProcess()->sendPacket(ddPacket,IPv6Address::ALL_OSPF_ROUTERS_MCAST, this->getInterface()->getIntName().c_str());
    else
        this->getInterface()->getArea()->getInstance()->getProcess()->sendPacket(ddPacket,this->getNeighborIP(), this->getInterface()->getIntName().c_str());

}

void OSPFv3Neighbor::sendLinkStateRequestPacket()
{
    OSPFv3LinkStateRequest *requestPacket = new OSPFv3LinkStateRequest();

    requestPacket->setType(LSR);
    requestPacket->setRouterID(this->getInterface()->getArea()->getInstance()->getProcess()->getRouterID());
    requestPacket->setAreaID(this->getInterface()->getArea()->getAreaID());
    requestPacket->setInstanceID(this->getInterface()->getArea()->getInstance()->getInstanceID());

    //long maxPacketSize = (((IP_MAX_HEADER_BYTES + OSPF_HEADER_LENGTH + OSPF_REQUEST_LENGTH) > parentInterface->getMTU()) ?
    //                      IPV4_DATAGRAM_LENGTH :
    //                      parentInterface->getMTU()) - IP_MAX_HEADER_BYTES;
    long packetSize = OSPFV3_HEADER_LENGTH;

    if (linkStateRequestList.empty()) {
        requestPacket->setRequestsArraySize(0);
    }
    else {
        auto it = linkStateRequestList.begin();

        while (it != linkStateRequestList.end()) { //TODO - maxpacketsize
            unsigned long requestCount = requestPacket->getRequestsArraySize();
            OSPFv3LSAHeader *requestHeader = (*it);
            OSPFv3LSRequest request;

            request.lsaType = requestHeader->getLsaType();
            request.lsaID = requestHeader->getLinkStateID();
            request.advertisingRouter = requestHeader->getAdvertisingRouter();

            requestPacket->setRequestsArraySize(requestCount + 1);
            requestPacket->setRequests(requestCount, request);

            packetSize += OSPFV3_LSR_LENGTH;
            it++;
        }
    }

    requestPacket->setByteLength(packetSize);

    //TODO - ttl and checksum

    if(this->getInterface()->getType() == OSPFv3Interface::POINTTOPOINT_TYPE)
        this->getInterface()->getArea()->getInstance()->getProcess()->sendPacket(requestPacket,IPv6Address::ALL_OSPF_ROUTERS_MCAST, this->getInterface()->getIntName().c_str());
    else
        this->getInterface()->getArea()->getInstance()->getProcess()->sendPacket(requestPacket,this->getNeighborIP(), this->getInterface()->getIntName().c_str());
}

void OSPFv3Neighbor::createDatabaseSummary()
{
    OSPFv3Area* area = this->getInterface()->getArea();
    int routerLSACount = area->getRouterLSACount();

    for(int i=0; i<routerLSACount; i++) {
        OSPFv3LSAHeader* lsaHeader = new OSPFv3LSAHeader(area->getRouterLSA(i)->getHeader());
        this->databaseSummaryList.push_back(lsaHeader);
    }

    int networkLSACount = area->getNetworkLSACount();

    for(int i=0; i<networkLSACount; i++) {
        OSPFv3LSAHeader* lsaHeader = new OSPFv3LSAHeader(area->getNetworkLSA(i)->getHeader());
        this->databaseSummaryList.push_back(lsaHeader);
    }

    int linkLsaCount = this->getInterface()->getLinkLSACount();
    for(int i=0; i<linkLsaCount; i++) {
        OSPFv3LSAHeader* lsaHeader = new OSPFv3LSAHeader(this->getInterface()->getLinkLSA(i)->getHeader());
        this->databaseSummaryList.push_back(lsaHeader);
    }

    int intraAreaPrefixCnt = area->getIntraAreaPrefixLSACount();
    for(int i=0; i<intraAreaPrefixCnt; i++) {
        OSPFv3LSAHeader* lsaHeader = new OSPFv3LSAHeader(area->getIntraAreaPrefixLSA(i)->getHeader());
        this->databaseSummaryList.push_back(lsaHeader);
    }
}

void OSPFv3Neighbor::retransmitUpdatePacket()
{
    EV_DEBUG << "Retransmitting update packet\n";
    OSPFv3LSUpdate *updatePacket = new OSPFv3LSUpdate();

    updatePacket->setType(LSU);
    updatePacket->setRouterID(this->getInterface()->getArea()->getInstance()->getProcess()->getRouterID());
    updatePacket->setAreaID(this->getInterface()->getArea()->getAreaID());
    updatePacket->setInstanceID(this->getInterface()->getArea()->getInstance()->getInstanceID());

    bool packetFull = false;
    unsigned short lsaCount = 0;
    unsigned long packetLength = OSPFV3_HEADER_LENGTH + OSPFV3_LSA_HEADER_LENGTH;
    auto it = linkStateRetransmissionList.begin();

    EV_DEBUG << "Retransmit packet - before while\n";
    while (!packetFull && (it != linkStateRetransmissionList.end())) {
        uint16_t lsaType = (*it)->getHeader().getLsaType();
        OSPFv3RouterLSA *routerLSA = (lsaType == ROUTER_LSA) ? dynamic_cast<OSPFv3RouterLSA *>(*it) : nullptr;
        OSPFv3NetworkLSA *networkLSA = (lsaType == NETWORK_LSA) ? dynamic_cast<OSPFv3NetworkLSA *>(*it) : nullptr;
        OSPFv3LinkLSA *linkLSA = (lsaType == LINK_LSA) ? dynamic_cast<OSPFv3LinkLSA *>(*it) : nullptr;
//        OSPFSummaryLSA *summaryLSA = ((lsaType == SUMMARYLSA_NETWORKS_TYPE) ||
//                                      (lsaType == SUMMARYLSA_ASBOUNDARYROUTERS_TYPE)) ? dynamic_cast<OSPFSummaryLSA *>(*it) : nullptr;
//        OSPFASExternalLSA *asExternalLSA = (lsaType == AS_EXTERNAL_LSA_TYPE) ? dynamic_cast<OSPFASExternalLSA *>(*it) : nullptr;
        long lsaSize = 0;
        bool includeLSA = false;

        switch (lsaType) {
            case ROUTER_LSA:
                if (routerLSA != nullptr) {
                    lsaSize = calculateLSASize(routerLSA);
                }
                break;

            case NETWORK_LSA:
                if (networkLSA != nullptr) {
                    lsaSize = calculateLSASize(networkLSA);
                }
                break;
//
//            case SUMMARYLSA_NETWORKS_TYPE:
//            case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE:
//                if (summaryLSA != nullptr) {
//                    lsaSize = calculateLSASize(summaryLSA);
//                }
//                break;
//
//            case AS_EXTERNAL_LSA_TYPE:
//                if (asExternalLSA != nullptr) {
//                    lsaSize = calculateLSASize(asExternalLSA);
//                }
//                break;
//
//            default:
//                break;
            case LINK_LSA:
                if(linkLSA != nullptr)
                    lsaSize = calculateLSASize(linkLSA);
                break;
        }

        if (packetLength + lsaSize < this->getInterface()->getInterfaceMTU()) {
            includeLSA = true;
            lsaCount++;
        }
        else {
            if ((lsaCount == 0) && (packetLength + lsaSize < IPV6_DATAGRAM_LENGTH)) {
                includeLSA = true;
                lsaCount++;
                packetFull = true;
            }
        }

        if (includeLSA) {
            packetLength += lsaSize;
            switch (lsaType) {
                case ROUTER_LSA:
                    if (routerLSA != nullptr) {
                        unsigned int routerLSACount = updatePacket->getRouterLSAsArraySize();

                        updatePacket->setRouterLSAsArraySize(routerLSACount + 1);
                        updatePacket->setRouterLSAs(routerLSACount, *routerLSA);

                        unsigned short lsAge = updatePacket->getRouterLSAs(routerLSACount).getHeader().getLsaAge();
                        if (lsAge < MAX_AGE - this->getInterface()->getTransDelayInterval()) {
                            updatePacket->getRouterLSAs(routerLSACount).getHeader().setLsaAge(lsAge + this->getInterface()->getTransDelayInterval());
                        }
                        else {
                            updatePacket->getRouterLSAs(routerLSACount).getHeader().setLsaAge(MAX_AGE);
                        }
                    }
                    break;

                case NETWORK_LSA:
                    if (networkLSA != nullptr) {
                        unsigned int networkLSACount = updatePacket->getNetworkLSAsArraySize();

                        updatePacket->setNetworkLSAsArraySize(networkLSACount + 1);
                        updatePacket->setNetworkLSAs(networkLSACount, *networkLSA);

                        unsigned short lsAge = updatePacket->getNetworkLSAs(networkLSACount).getHeader().getLsaAge();
                        if (lsAge < MAX_AGE - this->getInterface()->getTransDelayInterval()) {
                            updatePacket->getNetworkLSAs(networkLSACount).getHeader().setLsaAge(lsAge + this->getInterface()->getTransDelayInterval());
                        }
                        else {
                            updatePacket->getNetworkLSAs(networkLSACount).getHeader().setLsaAge(MAX_AGE);
                        }
                    }
                    break;
//
//                case SUMMARYLSA_NETWORKS_TYPE:
//                case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE:
//                    if (summaryLSA != nullptr) {
//                        unsigned int summaryLSACount = updatePacket->getSummaryLSAsArraySize();
//
//                        updatePacket->setSummaryLSAsArraySize(summaryLSACount + 1);
//                        updatePacket->setSummaryLSAs(summaryLSACount, *summaryLSA);
//
//                        unsigned short lsAge = updatePacket->getSummaryLSAs(summaryLSACount).getHeader().getLsAge();
//                        if (lsAge < MAX_AGE - parentInterface->getTransmissionDelay()) {
//                            updatePacket->getSummaryLSAs(summaryLSACount).getHeader().setLsAge(lsAge + parentInterface->getTransmissionDelay());
//                        }
//                        else {
//                            updatePacket->getSummaryLSAs(summaryLSACount).getHeader().setLsAge(MAX_AGE);
//                        }
//                    }
//                    break;
//
//                case AS_EXTERNAL_LSA_TYPE:
//                    if (asExternalLSA != nullptr) {
//                        unsigned int asExternalLSACount = updatePacket->getAsExternalLSAsArraySize();
//
//                        updatePacket->setAsExternalLSAsArraySize(asExternalLSACount + 1);
//                        updatePacket->setAsExternalLSAs(asExternalLSACount, *asExternalLSA);
//
//                        unsigned short lsAge = updatePacket->getAsExternalLSAs(asExternalLSACount).getHeader().getLsAge();
//                        if (lsAge < MAX_AGE - parentInterface->getTransmissionDelay()) {
//                            updatePacket->getAsExternalLSAs(asExternalLSACount).getHeader().setLsAge(lsAge + parentInterface->getTransmissionDelay());
//                        }
//                        else {
//                            updatePacket->getAsExternalLSAs(asExternalLSACount).getHeader().setLsAge(MAX_AGE);
//                        }
//                    }
//                    break;
                case LINK_LSA:
                    if (linkLSA != nullptr) {
                        unsigned int linkLSACount = updatePacket->getLinkLSAsArraySize();

                        updatePacket->setLinkLSAsArraySize(linkLSACount + 1);
                        updatePacket->setLinkLSAs(linkLSACount, *linkLSA);

                        unsigned short lsAge = updatePacket->getLinkLSAs(linkLSACount).getHeader().getLsaAge();
                        if (lsAge < MAX_AGE - this->getInterface()->getTransDelayInterval()) {
                            updatePacket->getLinkLSAs(linkLSACount).getHeader().setLsaAge(lsAge + this->getInterface()->getTransDelayInterval());
                        }
                        else {
                            updatePacket->getLinkLSAs(linkLSACount).getHeader().setLsaAge(MAX_AGE);
                        }
                    }
                    break;
//
                default:
                    break;
            }
        }

        it++;
    }

    EV_DEBUG << "Retransmit - packet length: "<<packetLength<<"\n";
    updatePacket->setByteLength(packetLength); //IPV6 HEADER BYTES

    this->getInterface()->getArea()->getInstance()->getProcess()->sendPacket(updatePacket, this->getNeighborIP(), this->getInterface()->getIntName().c_str());
    //TODO
//    int ttl = (parentInterface->getType() == Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;
//    messageHandler->sendPacket(updatePacket, neighborIPAddress, parentInterface->getIfIndex(), ttl);
}

void OSPFv3Neighbor::addToRetransmissionList(OSPFv3LSA *lsa)
{
    auto it = linkStateRetransmissionList.begin();
    for ( ; it != linkStateRetransmissionList.end(); it++) {
        if (((*it)->getHeader().getLinkStateID() == lsa->getHeader().getLinkStateID()) &&
            ((*it)->getHeader().getAdvertisingRouter().getInt() == lsa->getHeader().getAdvertisingRouter().getInt()))
        {
            break;
        }
    }

    OSPFv3LSA *lsaCopy = nullptr;
    switch (lsa->getHeader().getLsaType()) {
        case ROUTER_LSA:
            lsaCopy = new OSPFv3RouterLSA(*(check_and_cast<OSPFv3RouterLSA *>(lsa)));
            break;

        case NETWORK_LSA:
            lsaCopy = new OSPFv3NetworkLSA(*(check_and_cast<OSPFv3NetworkLSA *>(lsa)));
            break;

//        case SUMMARYLSA_NETWORKS_TYPE:
//        case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE:
//            lsaCopy = new OSPFSummaryLSA(*(check_and_cast<OSPFSummaryLSA *>(lsa)));
//            break;
//
//        case AS_EXTERNAL_LSA_TYPE:
//            lsaCopy = new OSPFASExternalLSA(*(check_and_cast<OSPFASExternalLSA *>(lsa)));
//            break;

        case LINK_LSA:
            lsaCopy = new OSPFv3LinkLSA(*(check_and_cast<OSPFv3LinkLSA *>(lsa)));
            break;
        default:
            ASSERT(false);    // error
            break;
    }

    if (it != linkStateRetransmissionList.end()) {
        delete (*it);
        *it = static_cast<OSPFv3LSA *>(lsaCopy);
    }
    else {
        linkStateRetransmissionList.push_back(static_cast<OSPFv3LSA *>(lsaCopy));
    }
}

bool OSPFv3Neighbor::retransmitDatabaseDescriptionPacket()
{
    EV_DEBUG << "Retransmitting DD Packet\n";
    if (lastTransmittedDDPacket != nullptr) {
        OSPFv3DatabaseDescription *ddPacket = new OSPFv3DatabaseDescription(*lastTransmittedDDPacket);
//        int ttl = (parentInterface->getType() == Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;

//        if (parentInterface->getType() == Interface::POINTTOPOINT) {
//            messageHandler->sendPacket(ddPacket, IPv4Address::ALL_OSPF_ROUTERS_MCAST, parentInterface->getIfIndex(), ttl);
//        }
//        else {
        this->getInterface()->getArea()->getInstance()->getProcess()->sendPacket(ddPacket, this->getNeighborIP(), this->getInterface()->getIntName().c_str());
//        }

        return true;
    }
    else {
        EV_DEBUG << "But no packet is in the line\n";
        return false;
    }
}

void OSPFv3Neighbor::addToRequestList(OSPFv3LSAHeader *lsaHeader)
{
    linkStateRequestList.push_back(new OSPFv3LSAHeader(*lsaHeader));
    EV_DEBUG << "Currently on request list:\n";
    for(auto it = linkStateRequestList.begin(); it != linkStateRequestList.end(); it++) {
        EV_DEBUG << "\tType: "<<(*it)->getLsaType() << ", ID: " << (*it)->getLinkStateID() << ", Adv: " << (*it)->getAdvertisingRouter() << "\n";
    }
}

bool OSPFv3Neighbor::isLSAOnRequestList(LSAKeyType lsaKey)
{
    if(findOnRequestList(lsaKey)==nullptr)
        return false;

    return true;
}

OSPFv3LSAHeader* OSPFv3Neighbor::findOnRequestList(LSAKeyType lsaKey)
{
    for (auto it = linkStateRequestList.begin(); it != linkStateRequestList.end(); it++) {
        if (((*it)->getLinkStateID() == lsaKey.linkStateID) &&
                ((*it)->getAdvertisingRouter() == lsaKey.advertisingRouter))
        {
            EV_DEBUG << "Found on request list. Currently on request list:\n";
                for(auto it = linkStateRequestList.begin(); it != linkStateRequestList.end(); it++) {
                    EV_DEBUG << "\tType: "<<(*it)->getLsaType() << ", ID: " << (*it)->getLinkStateID() << ", Adv: " << (*it)->getAdvertisingRouter() << "\n";
                }
            return *it;
        }
    }
    return nullptr;
}

void OSPFv3Neighbor::removeFromRequestList(LSAKeyType lsaKey)
{
    auto it = linkStateRequestList.begin();
    while (it != linkStateRequestList.end()) {
        if (((*it)->getLinkStateID() == lsaKey.linkStateID) &&
            ((*it)->getAdvertisingRouter() == lsaKey.advertisingRouter))
        {
            delete (*it);
            it = linkStateRequestList.erase(it);
        }
        else {
            it++;
        }
    }
    if ((getState() == OSPFv3Neighbor::LOADING_STATE) && (linkStateRequestList.empty())) {
        clearRequestRetransmissionTimer();
        processEvent(OSPFv3Neighbor::LOADING_DONE);
    }
}//removeFromRequestList

void OSPFv3Neighbor::addToTransmittedLSAList(LSAKeyType lsaKey)
{
    TransmittedLSA transmit;

    transmit.lsaKey = lsaKey;
    transmit.age = 0;

    transmittedLSAs.push_back(transmit);
}//addToTransmittedLSAList

bool OSPFv3Neighbor::isOnTransmittedLSAList(LSAKeyType lsaKey) const
{
    for (std::list<TransmittedLSA>::const_iterator it = transmittedLSAs.begin(); it != transmittedLSAs.end(); it++) {
        if ((it->lsaKey.linkStateID == lsaKey.linkStateID) &&
            (it->lsaKey.advertisingRouter == lsaKey.advertisingRouter))
        {
            return true;
        }
    }
    return false;
}//isOnTransmittedLSAList

void OSPFv3Neighbor::deleteLastSentDDPacket()
{
    if (lastTransmittedDDPacket != nullptr) {
        delete lastTransmittedDDPacket;
        lastTransmittedDDPacket = nullptr;
    }
}//deleteLastSentDDPacket

void OSPFv3Neighbor::removeFromRetransmissionList(LSAKeyType lsaKey)
{
    auto it = linkStateRetransmissionList.begin();
    while (it != linkStateRetransmissionList.end()) {
        if (((*it)->getHeader().getLinkStateID() == lsaKey.linkStateID) &&
            ((*it)->getHeader().getAdvertisingRouter() == lsaKey.advertisingRouter))
        {
            delete (*it);
            it = linkStateRetransmissionList.erase(it);
        }
        else {
            it++;
        }
    }
}//removeFromRetransmissionList
}
