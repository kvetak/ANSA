#include "ansa/routing/ospfv3/interface/OSPFv3Interface.h"
#include "ansa/routing/ospfv3/interface/OSPFv3InterfaceStateDown.h"
#include "ansa/routing/ospfv3/interface/OSPFv3InterfaceState.h"
#include "inet/networklayer/ipv6/IPv6Datagram_m.h"


namespace inet{

OSPFv3Interface::OSPFv3Interface(const char* name, cModule* routerModule, OSPFv3Process* processModule, OSPFv3InterfaceType interfaceType, bool passive) :
        helloInterval(DEFAULT_HELLO_INTERVAL),
        deadInterval(DEFAULT_DEAD_INTERVAL),
        pollInterval(4*DEFAULT_DEAD_INTERVAL),
        transmissionDelay(1),
        retransmissionInterval(5),
        ackDelay(1),
        routerPriority(DEFAULT_ROUTER_PRIORITY),
        DesignatedRouterIP(IPv6Address::UNSPECIFIED_ADDRESS),
        BackupRouterIP(IPv6Address::UNSPECIFIED_ADDRESS),
        DesignatedRouterID(IPv4Address::UNSPECIFIED_ADDRESS),
        BackupRouterID(IPv4Address::UNSPECIFIED_ADDRESS),
        DesignatedIntID(-1),
        interfaceCost(10)
{
    this->interfaceName=std::string(name);
    this->state = new OSPFv3InterfaceStateDown;
    this->containingModule = routerModule;
    this->containingProcess = processModule;
    this->ift = check_and_cast<IInterfaceTable *>(containingModule->getSubmodule("interfaceTable"));

    InterfaceEntry *ie = this->ift->getInterfaceByName(this->interfaceName.c_str());
    IPv6InterfaceData *ipv6int = ie->ipv6Data();
    this->interfaceId = ift->getInterfaceById(ie->getInterfaceId())->getInterfaceId();
    this->interfaceIP = ipv6int->getLinkLocalAddress();//TODO - check
    EV_DEBUG << "Interface IP: " << ipv6int->getLinkLocalAddress() << "\n";
    this->interfaceType = interfaceType;
    this->passiveInterface = passive;
    this->transitNetworkInterface = false; //false at first

    this->helloTimer = new cMessage();
    helloTimer->setKind(HELLO_TIMER);
    helloTimer->setContextPointer(this);
    helloTimer->setName("OSPFv3Interface::HelloTimer");

    this->waitTimer = new cMessage();
    waitTimer->setKind(WAIT_TIMER);
    waitTimer->setContextPointer(this);
    waitTimer->setName("OSPFv3Interface::WaitTimer");

    this->acknowledgementTimer = new cMessage();
    acknowledgementTimer->setKind(ACKNOWLEDGEMENT_TIMER);
    acknowledgementTimer->setContextPointer(this);
    acknowledgementTimer->setName("OSPFv3Interface::AcknowledgementTimer");
}//constructor

OSPFv3Interface::~OSPFv3Interface()
{

}//destructor

void OSPFv3Interface::processEvent(OSPFv3Interface::OSPFv3InterfaceEvent event)
{
    EV_DEBUG << "Passing event number " << event << " to state" << this->state->getInterfaceStateString() << "\n";
    this->state->processEvent(this, event);
}

OSPFv3HelloPacket* OSPFv3Interface::prepareHello()
{
    OSPFv3Options options;
    int length;
    OSPFv3HelloPacket* helloPacket = new OSPFv3HelloPacket();
    std::vector<IPv4Address> neighbors;

    //OSPF common packet header first
    helloPacket->setVersion(3);
    helloPacket->setType(HELLO_PACKET);
    //TODO - packet length
    helloPacket->setRouterID(this->getArea()->getInstance()->getProcess()->getRouterID());
    helloPacket->setAreaID(this->containingArea->getAreaID());
    helloPacket->setInstanceID(this->getArea()->getInstance()->getInstanceID());
    length = 16;

    //Hello content
    helloPacket->setInterfaceID(this->interfaceIndex);//TODO - check
    helloPacket->setRouterPriority(this->getRouterPriority());
    memset(&options, 0, sizeof(OSPFv3Options));

    options.rBit = true;
    options.v6Bit = true;
    if(this->getArea()->getExternalRoutingCapability())
        options.eBit = true;

    if(this->getArea()->getAreaType() == NSSA)
        options.nBit = true;

    helloPacket->setOptions(options);
    length+=8;
    //TODO - set options
    helloPacket->setHelloInterval(this->getHelloInterval());
    helloPacket->setDeadInterval(this->getDeadInterval());
    //TODO - set the DR correctly
    helloPacket->setDesignatedRouterID(this->getDesignatedID());
    helloPacket->setBackupDesignatedRouterID(this->BackupRouterID);
    //TODO - set the neighbor id correctly

    length += 12;

    int neighborCount = this->getNeighborCount();
    for(int i=0; i<neighborCount; i++){
        if(this->getNeighbor(i)->getState() >= OSPFv3Neighbor::INIT_STATE) {
            neighbors.push_back(this->getNeighbor(i)->getNeighborID());
            length+=4;
        }
    }

    unsigned int initedNeighborCount = neighbors.size();
    helloPacket->setNeighborIDArraySize(initedNeighborCount);
    for (unsigned int k = 0; k < initedNeighborCount; k++) {
        helloPacket->setNeighborID(k, neighbors.at(k));
    }

    helloPacket->setPacketLength(length);
    helloPacket->setByteLength(length);
    return helloPacket;
}

int OSPFv3Interface::getInterfaceMTU() const
{
    InterfaceEntry* ie = this->ift->getInterfaceByName(this->interfaceName.c_str());
    return ie->getMTU();
}

void OSPFv3Interface::changeState(OSPFv3InterfaceState* currentState, OSPFv3InterfaceState* newState)
{
    EV_DEBUG << "Interface state is changing from " << currentState->getInterfaceStateString() << " to " << newState->getInterfaceStateString() << "\n";

    if(this->previousState!=nullptr)
        delete this->previousState;//FIXME - create destructor

    this->previousState = currentState;
    this->state = newState;
    EV_DEBUG << "Changing state of interface \n";
}//changeState

OSPFv3Interface::OSPFv3InterfaceFAState OSPFv3Interface::getState() const
{
    return state->getState();
}//getState

void OSPFv3Interface::reset()
{
    EV_DEBUG << "Resetting interface " << this->getIntName() << "\n";
}//reset

//----------------------------------------------- Hello Packet ------------------------------------------------//

void OSPFv3Interface::processHelloPacket(OSPFv3Packet* packet)
{
    EV_DEBUG <<"$$$$$$$$$ Hello packet was received on interface " << this->getIntName() << "\n";
    OSPFv3HelloPacket* hello = check_and_cast<OSPFv3HelloPacket*>(packet);
    bool neighborChanged = false;
    bool backupSeen = false;
    bool neighborsDRStateChanged = false;
    bool drChanged = false;
    bool shouldRebuildRoutingTable=false;

    IPv6ControlInfo* ctlInfo = dynamic_cast<IPv6ControlInfo*>(packet->getControlInfo());

    //comparing hello and dead values
    if((hello->getHelloInterval()==this->getHelloInterval()) && (hello->getDeadInterval()==this->getDeadInterval())) {
        if(true) {//TODO - this will check the E-bit
            IPv4Address sourceId = hello->getRouterID();
            OSPFv3Neighbor* neighbor = this->getNeighborById(sourceId);

            if(neighbor != nullptr) {
                EV_DEBUG << "$$$$$$ This is not a new neighbor!!! I know him for a long time...\n";
                IPv4Address designatedRouterID = neighbor->getNeighborsDR();
                IPv4Address backupRouterID = neighbor->getNeighborsBackup();
                int newPriority = hello->getRouterPriority();
                IPv4Address newDesignatedRouterID = hello->getDesignatedRouterID();
                IPv4Address newBackupRouterID = hello->getBackupDesignatedRouterID();
                IPv4Address dRouterID;

                if ((this->interfaceType == OSPFv3InterfaceType::VIRTUAL_TYPE) &&
                        (neighbor->getState() == OSPFv3Neighbor::DOWN_STATE))
                {
                    neighbor->setNeighborPriority(hello->getRouterPriority());
                    neighbor->setNeighborDeadInterval(hello->getDeadInterval());
                }

                /* If a change in the neighbor's Router Priority field
                   was noted, the receiving interface's state machine is
                   scheduled with the event NEIGHBOR_CHANGE.
                 */
                if (neighbor->getNeighborPriority() != newPriority) {
                    neighborChanged = true;
                }

                /* If the neighbor is both declaring itself to be Designated
                   Router(Hello Packet's Designated Router field = Neighbor IP
                   address) and the Backup Designated Router field in the
                   packet is equal to 0.0.0.0 and the receiving interface is in
                   state Waiting, the receiving interface's state machine is
                   scheduled with the event BACKUP_SEEN.
                 */
                if ((newDesignatedRouterID == sourceId) &&
                        (newBackupRouterID == NULL_IPV4ADDRESS) &&
                        (this->getState() == OSPFv3InterfaceFAState::INTERFACE_STATE_WAITING))
                {
                    backupSeen = true;
                }
                else {
                    /* Otherwise, if the neighbor is declaring itself to be Designated Router and it
                       had not previously, or the neighbor is not declaring itself
                       Designated Router where it had previously, the receiving
                       interface's state machine is scheduled with the event
                       NEIGHBOR_CHANGE.
                     */
                    if (((newDesignatedRouterID == sourceId) &&
                            (newDesignatedRouterID != designatedRouterID)) ||
                            ((newDesignatedRouterID != sourceId) &&
                                    (sourceId == designatedRouterID)))
                    {
                        neighborChanged = true;
                        neighborsDRStateChanged = true;
                    }
                }

                /* If the neighbor is declaring itself to be Backup Designated
                   Router(Hello Packet's Backup Designated Router field =
                   Neighbor ID ) and the receiving interface is in state
                   Waiting, the receiving interface's state machine is
                   scheduled with the event BACKUP_SEEN.
                */
                if ((newBackupRouterID == sourceId) &&
                        (this->getState() == OSPFv3InterfaceFAState::INTERFACE_STATE_WAITING))
                {
                    backupSeen = true;
                }
                else {
                    /* Otherwise, if the neighbor is declaring itself to be Backup Designated Router
                    and it had not previously, or the neighbor is not declaring
                    itself Backup Designated Router where it had previously, the
                    receiving interface's state machine is scheduled with the
                    event NEIGHBOR_CHANGE.
                     */
                    if (((newBackupRouterID == sourceId) &&
                            (newBackupRouterID != backupRouterID)) ||
                            ((newBackupRouterID != sourceId) &&
                                    (sourceId == backupRouterID)))
                    {
                        neighborChanged = true;
                    }
                }

                neighbor->setNeighborID(hello->getRouterID());
                neighbor->setNeighborPriority(newPriority);
                neighbor->setNeighborAddress(ctlInfo->getSourceAddress().toIPv6());
                neighbor->setNeighborInterfaceID(hello->getInterfaceID());
                dRouterID = newDesignatedRouterID;
                if (newDesignatedRouterID != designatedRouterID) {
                    designatedRouterID = dRouterID;
                    drChanged = true;
                }

                neighbor->setDesignatedRouterID(dRouterID);
                dRouterID = newBackupRouterID;
                if (newBackupRouterID != backupRouterID) {
                    backupRouterID = dRouterID;
                    drChanged = true;
                }

                neighbor->setBackupDesignatedRouterID(dRouterID);
                if (drChanged) {
                    neighbor->setupDesignatedRouters(false);
                }

                /* If the neighbor router's Designated or Backup Designated Router
                   has changed it's necessary to look up the Router IDs belonging to the
                   new addresses.
                 */
                if (!neighbor->designatedRoutersAreSetUp()) {
                    OSPFv3Neighbor *designated = this->getNeighborById(designatedRouterID);
                    OSPFv3Neighbor *backup = this->getNeighborById(backupRouterID);

                    if (designated != nullptr) {
                        EV_DEBUG << "Setting new DR ID in hello processing\n";
                        dRouterID = designated->getNeighborID();
                        neighbor->setDesignatedRouterID(dRouterID);
                        EV_DEBUG << "New DR for neighbor " << dRouterID << " is " << neighbor->getNeighborsDR() << "\n";
                    }
                    if (backup != nullptr) {
                        dRouterID = backup->getNeighborID();
                        neighbor->setBackupDesignatedRouterID(dRouterID);
                    }
                    if ((designated != nullptr) && (backup != nullptr)) {
                        neighbor->setupDesignatedRouters(true);
                    }
                }

                neighbor->setLastHelloTime((int)simTime().dbl());
            }
            else {
                IPv4Address dRouterID;
                bool designatedSetUp = false;
                bool backupSetUp = false;

                neighbor = new OSPFv3Neighbor(sourceId, this);
                neighbor->setNeighborPriority(hello->getRouterPriority());
                neighbor->setNeighborAddress(ctlInfo->getSourceAddress().toIPv6());
                neighbor->setNeighborDeadInterval(hello->getDeadInterval());
                EV_DEBUG << "HELLO: Interface ID used in hello packet: " << hello->getInterfaceID() << "\n";
                neighbor->setNeighborInterfaceID(hello->getInterfaceID());

                dRouterID = hello->getDesignatedRouterID();
                OSPFv3Neighbor *designated = this->getNeighborById(dRouterID);

                // Get the Designated Router ID from the corresponding Neighbor Object.
                if (designated != nullptr) {
                    if (designated->getNeighborID() != dRouterID) {
                        dRouterID = designated->getNeighborID();
                    }
                    designatedSetUp = true;
                }
                neighbor->setDesignatedRouterID(dRouterID);

                dRouterID = hello->getBackupDesignatedRouterID();
                OSPFv3Neighbor* backup = this->getNeighborById(dRouterID);

                // Get the Backup Designated Router ID from the corresponding Neighbor Object.
                if (backup != nullptr) {
                    if (backup->getNeighborID() != dRouterID) {
                        dRouterID = backup->getNeighborID();
                    }
                    backupSetUp = true;
                }
                neighbor->setBackupDesignatedRouterID(dRouterID);
                if (designatedSetUp && backupSetUp) {
                    neighbor->setupDesignatedRouters(true);
                }
                this->addNeighbor(neighbor);
            }

            neighbor->processEvent(OSPFv3Neighbor::OSPFv3NeighborEventType::HELLO_RECEIVED);
            if ((this->interfaceType == OSPFv3InterfaceType::NBMA_TYPE) &&
                    (this->getRouterPriority() == 0) &&
                    (neighbor->getState() >= OSPFv3Neighbor::OSPFv3NeighborStateType::INIT_STATE))
            {
                OSPFv3HelloPacket* hello = this->prepareHello();
                this->getArea()->getInstance()->getProcess()->sendPacket(hello, neighbor->getNeighborIP(), this->interfaceName.c_str());
            }

//            IPv4Address interfaceAddress = intf->getAddressRange().address;
            unsigned int neighborsNeighborCount = hello->getNeighborIDArraySize();
            unsigned int i;

            for (i = 0; i < neighborsNeighborCount; i++) {
                if (hello->getNeighborID(i) == this->getArea()->getInstance()->getProcess()->getRouterID()) {
                    neighbor->processEvent(OSPFv3Neighbor::TWOWAY_RECEIVED);
                    break;
                }
            }

            /* Otherwise, the neighbor state machine should
               be executed with the event ONEWAY_RECEIVED, and the processing
               of the packet stops.
            */
            if (i == neighborsNeighborCount) {
                neighbor->processEvent(OSPFv3Neighbor::ONEWAY_RECEIVED);
            }

            if (neighborChanged) {
                EV_DEBUG << "Neighbor change noted in Hello packet processing in router " << this->getArea()->getInstance()->getProcess()->getRouterID() << "\n";
                this->processEvent(OSPFv3InterfaceEvent::NEIGHBOR_CHANGE_EVENT);
                /* In some cases neighbors get stuck in TwoWay state after a DR
                   or Backup change. (calculateDesignatedRouter runs before the
                   neighbors' signal of DR change + this router does not become
                   neither DR nor backup -> IS_ADJACENCY_OK does not get called.)
                   So to make it work(workaround) we'll call IS_ADJACENCY_OK for
                   all neighbors in TwoWay state from here. This shouldn't break
                   anything because if the neighbor state doesn't have to change
                   then needAdjacency returns false and nothing happnes in
                   IS_ADJACENCY_OK.
                 */
                unsigned int neighborCount = this->getNeighborCount();
                for (i = 0; i < neighborCount; i++) {
                    OSPFv3Neighbor *stuckNeighbor = this->neighbors.at(i);
                    if (stuckNeighbor->getState() == OSPFv3Neighbor::TWOWAY_STATE) {
                        stuckNeighbor->processEvent(OSPFv3Neighbor::IS_ADJACENCY_OK);
                    }
                }

                if (neighborsDRStateChanged) {
                    EV_DEBUG <<"Router DR has changed - need to add LSAs\n";
                    shouldRebuildRoutingTable = true;
//                    OSPFv3RouterLSA* routerLSA = this->getArea()->originateRouterLSA();
//                    RouterLSA *routerLSA = intf->getArea()->findRouterLSA(router->getRouterID());
//
//                    if (routerLSA != nullptr) {
//                        long sequenceNumber = routerLSA->getHeader().getLsSequenceNumber();
//                        if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
//                            routerLSA->getHeader().setLsAge(MAX_AGE);
//                            intf->getArea()->floodLSA(routerLSA);
//                            routerLSA->incrementInstallTime();
//                        }
//                        else {
//                            RouterLSA *newLSA = intf->getArea()->originateRouterLSA();
//
//                            newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
//                            shouldRebuildRoutingTable |= routerLSA->update(newLSA);
//                            delete newLSA;
//
//                            intf->getArea()->floodLSA(routerLSA);
//                        }
//                    }
                }
            }
        }
    }

    if (shouldRebuildRoutingTable) {
        this->getArea()->getInstance()->getProcess()->rebuildRoutingTable();
    }

    delete packet;
}//processHello

//--------------------------------------------Database Description Packets--------------------------------------------//


void OSPFv3Interface::processDDPacket(OSPFv3Packet* packet){
    OSPFv3DatabaseDescription* ddPacket = check_and_cast<OSPFv3DatabaseDescription* >(packet);
    EV_DEBUG << "Process " << this->getArea()->getInstance()->getProcess()->getProcessID() << " received a DD Packet from neighbor " << ddPacket->getRouterID() << " on interface " << this->interfaceName << "\n";

    OSPFv3Neighbor* neighbor = this->getNeighborById(ddPacket->getRouterID());
    if(neighbor == nullptr)
        EV_DEBUG << "DD Packet is originated by an unknown router - it is not listed as a neighbor\n";
    OSPFv3Neighbor::OSPFv3NeighborStateType neighborState = neighbor->getState();

    if ((ddPacket->getInterfaceMTU() <= this->getInterfaceMTU()) &&
            (neighborState > OSPFv3Neighbor::ATTEMPT_STATE))
    {
        switch (neighborState) {
        case OSPFv3Neighbor::TWOWAY_STATE:
            EV_DEBUG << "Parsing DD Packet - two way state - throwing away\n";
            delete(packet);
            //ignoring packet
            break;

        case OSPFv3Neighbor::INIT_STATE:
            EV_DEBUG << "Parsing DD Packet - init -> goint to 2way\n";
            neighbor->processEvent(OSPFv3Neighbor::TWOWAY_RECEIVED);
            neighbor->setLastReceivedDDPacket(ddPacket);
            delete(packet);
            break;

        case OSPFv3Neighbor::EXCHANGE_START_STATE: {
            EV_DEBUG << "Router " << this->getArea()->getInstance()->getProcess()->getRouterID() << " is processing DD packet in EXCHANGE_START STATE\n";
            OSPFv3DDOptions& ddOptions = ddPacket->getDdOptions();

            if (ddOptions.iBit && ddOptions.mBit && ddOptions.msBit &&
                    (ddPacket->getLsaHeadersArraySize() == 0))
            {
                if (neighbor->getNeighborID() > this->getArea()->getInstance()->getProcess()->getRouterID()) {
                    EV_DEBUG << "Router " << this->getArea()->getInstance()->getProcess()->getRouterID() << " is becoming the slave\n";
                    neighbor->setDatabaseExchangeRelationship(OSPFv3Neighbor::SLAVE);
                    neighbor->setLastReceivedDDPacket(ddPacket);

                    if (!preProcessDDPacket(ddPacket, neighbor, true)) {
                        break;
                    }

                    neighbor->processEvent(OSPFv3Neighbor::NEGOTIATION_DONE);
                    EV_DEBUG << "Router going to negotiation done state\n";
                    EV_DEBUG << "LinkStateRequestEmpty = " << neighbor->isLinkStateRequestListEmpty() << ", retransmission timer active = " << neighbor->isRequestRetransmissionTimerActive() << "\n";
                    if (!neighbor->isLinkStateRequestListEmpty() &&
                            !neighbor->isRequestRetransmissionTimerActive())
                    {
                        neighbor->sendLinkStateRequestPacket();
                        neighbor->clearRequestRetransmissionTimer();
                        neighbor->startRequestRetransmissionTimer();
                    }
                }
                else {
                    neighbor->sendDDPacket(true);
                }
            }

            if (!ddOptions.iBit && !ddOptions.msBit &&
                    (ddPacket->getSequenceNumber() == neighbor->getDDSequenceNumber()) &&
                    (neighbor->getNeighborID() < this->getArea()->getInstance()->getProcess()->getRouterID()))
            {
                neighbor->setDatabaseExchangeRelationship(OSPFv3Neighbor::MASTER);
                neighbor->setLastReceivedDDPacket(ddPacket);

                if (!preProcessDDPacket(ddPacket, neighbor, true)) {
                    EV_DEBUG << "???????????????????????????";
                    break;
                }

                neighbor->processEvent(OSPFv3Neighbor::NEGOTIATION_DONE);
                EV_DEBUG << "LinkStateRequestEmpty = " << neighbor->isLinkStateRequestListEmpty() << ", retransmission timer active = " << neighbor->isRequestRetransmissionTimerActive() << "\n";
                if (!neighbor->isLinkStateRequestListEmpty() &&
                        !neighbor->isRequestRetransmissionTimerActive())
                {
                    neighbor->sendLinkStateRequestPacket();
                    neighbor->clearRequestRetransmissionTimer();
                    neighbor->startRequestRetransmissionTimer();
                }
            }

            delete(packet);
        }
        break;

        case OSPFv3Neighbor::EXCHANGE_STATE: {
            EV_DEBUG << "Parsing DD Packet - EXCHANGE STATE\n";
            OSPFv3DDPacketID packetID;
            packetID.ddOptions = ddPacket->getDdOptions();
            packetID.options = ddPacket->getOptions();
            packetID.sequenceNumber = ddPacket->getSequenceNumber();

            if (packetID != neighbor->getLastReceivedDDPacket()) {
                if ((packetID.ddOptions.msBit &&
                        (neighbor->getDatabaseExchangeRelationship() != OSPFv3Neighbor::SLAVE)) ||
                        (!packetID.ddOptions.msBit &&
                                (neighbor->getDatabaseExchangeRelationship() != OSPFv3Neighbor::MASTER)) ||
                                packetID.ddOptions.iBit ||
                                (packetID.options != neighbor->getLastReceivedDDPacket().options))
                {
                    EV_DEBUG << "Last DD Sequence is : " << neighbor->getLastReceivedDDPacket().sequenceNumber << "\n";
                    neighbor->processEvent(OSPFv3Neighbor::SEQUENCE_NUMBER_MISMATCH);
                }
                else {
                    if (((neighbor->getDatabaseExchangeRelationship() == OSPFv3Neighbor::MASTER) &&
                            (packetID.sequenceNumber == neighbor->getDDSequenceNumber())) ||
                            ((neighbor->getDatabaseExchangeRelationship() == OSPFv3Neighbor::SLAVE) &&
                                    (packetID.sequenceNumber == (neighbor->getDDSequenceNumber() + 1))))
                    {
                        neighbor->setLastReceivedDDPacket(ddPacket);
                        if (!preProcessDDPacket(ddPacket, neighbor, false)) {
                            EV_DEBUG << "Parsing DD Packet - EXCHANGE - preprocessing was true \n";
                            break;
                        }
                        if (!neighbor->isLinkStateRequestListEmpty() &&
                                !neighbor->isRequestRetransmissionTimerActive())
                        {
                            EV_DEBUG << "Parsing DD Packet - sending LINKSTATEREQUEST\n";
                            neighbor->sendLinkStateRequestPacket();
                            neighbor->clearRequestRetransmissionTimer();
                            neighbor->startRequestRetransmissionTimer();
                        }
                    }
                    else {
                        neighbor->processEvent(OSPFv3Neighbor::SEQUENCE_NUMBER_MISMATCH);
                    }
                }
            }
            else {
                EV_DEBUG << "REceived DD was the same as the last one\n";
                if (neighbor->getDatabaseExchangeRelationship() == OSPFv3Neighbor::SLAVE) {
                    EV_DEBUG << "But I am a slave so I retransmit it\n";
                    neighbor->retransmitDatabaseDescriptionPacket();
                }
            }
        }
        break;

        case OSPFv3Neighbor::LOADING_STATE:
        case OSPFv3Neighbor::FULL_STATE: {
            OSPFv3DDPacketID packetID;
            packetID.ddOptions = ddPacket->getDdOptions();
            packetID.options = ddPacket->getOptions();
            packetID.sequenceNumber = ddPacket->getSequenceNumber();

            if ((packetID != neighbor->getLastReceivedDDPacket()) ||
                    (packetID.ddOptions.iBit))
            {
                EV_DEBUG << "  Processing packet contents(ddOptions="
                            << ((ddPacket->getDdOptions().iBit) ? "I " : "_ ")
                            << ((ddPacket->getDdOptions().mBit) ? "M " : "_ ")
                            << ((ddPacket->getDdOptions().msBit) ? "MS" : "__")
                            << "; seqNumber="
                            << ddPacket->getSequenceNumber()
                            << "):\n";

                neighbor->processEvent(OSPFv3Neighbor::SEQUENCE_NUMBER_MISMATCH);
            }
            else {
                if (neighbor->getDatabaseExchangeRelationship() == OSPFv3Neighbor::SLAVE) {
                    if (!neighbor->retransmitDatabaseDescriptionPacket()) {
                        neighbor->processEvent(OSPFv3Neighbor::SEQUENCE_NUMBER_MISMATCH);
                    }
                }
            }
        }
        break;

        default:
            break;
        }
    }
    else
        delete(packet); //simply reject the packet otherwise
}

bool OSPFv3Interface::preProcessDDPacket(OSPFv3DatabaseDescription *ddPacket, OSPFv3Neighbor* neighbor, bool inExchangeStart)
{
    EV_INFO << "  Processing packet contents(ddOptions="
            << ((ddPacket->getDdOptions().iBit) ? "I " : "_ ")
            << ((ddPacket->getDdOptions().mBit) ? "M " : "_ ")
            << ((ddPacket->getDdOptions().msBit) ? "MS" : "__")
            << "; seqNumber="
            << ddPacket->getSequenceNumber()
            << "):\n";

    unsigned int headerCount = ddPacket->getLsaHeadersArraySize();

    for (unsigned int i = 0; i < headerCount; i++) {
        OSPFv3LSAHeader& currentHeader = ddPacket->getLsaHeaders(i);
        uint16_t lsaType = currentHeader.getLsaType();
//        OSPFv3LSAType lsaType = static_cast<OSPFv3LSAType>(currentHeader.getLsaType());

//        EV_DETAIL << "    " << currentHeader;

        if (((lsaType != ROUTER_LSA) && (lsaType != AS_EXTERNAL_LSA) && (lsaType != LINK_LSA) &&
            (lsaType != NETWORK_LSA) && (lsaType != INTER_AREA_PREFIX_LSA) && (lsaType != INTER_AREA_ROUTER_LSA) &&
            (lsaType != NSSA_LSA) && (lsaType != INTRA_AREA_PREFIX_LSA)) ||
            ((lsaType == AS_EXTERNAL_LSA) && (!this->getArea()->getExternalRoutingCapability())))
        {
            EV_ERROR << " Error! LSA TYPE: " << lsaType << "\n";
            neighbor->processEvent(OSPFv3Neighbor::SEQUENCE_NUMBER_MISMATCH);
            return false;
        }
        else {
            EV_DEBUG << "Trying to locate LSA in database\n";
            LSAKeyType lsaKey;

            lsaKey.linkStateID = currentHeader.getLinkStateID();
            lsaKey.advertisingRouter = currentHeader.getAdvertisingRouter();
            lsaKey.LSType = currentHeader.getLsaType();

            OSPFv3LSAHeader *lsaInDatabase = this->getArea()->findLSA(lsaKey);

            // operator< and operator== on OSPFLSAHeaders determines which one is newer(less means older)
            if ((lsaInDatabase == nullptr) || (*lsaInDatabase < currentHeader)) {
                EV_DETAIL << " (newer)";
                EV_DEBUG << "Adding LSA from router "<< currentHeader.getAdvertisingRouter() << "on request list\n";
                neighbor->addToRequestList(&currentHeader);
            }
        }
        EV_DETAIL << "\n";
    }

    EV_DEBUG << "DatabaseSummaryListCount = " << neighbor->getDatabaseSummaryListCount() << endl;
    EV_DEBUG << "M_BIT " << ddPacket->getDdOptions().mBit << endl;
    if (neighbor->getDatabaseExchangeRelationship() == OSPFv3Neighbor::MASTER) {
        EV_DEBUG << "I am the master!\n";
        neighbor->incrementDDSequenceNumber();
        if ((neighbor->getDatabaseSummaryListCount() == 0) && !ddPacket->getDdOptions().mBit) {
            EV_DEBUG << "Passing EXCHANGE_DONE to neighbor\n";
            neighbor->processEvent(OSPFv3Neighbor::EXCHANGE_DONE);    // does nothing in ExchangeStart
        }
        else {
            if (!inExchangeStart) {
                EV_DEBUG <<"Sending packet\n";
                neighbor->sendDDPacket();
            }
        }
    }
    else {
        neighbor->setDDSequenceNumber(ddPacket->getSequenceNumber());
        if (!inExchangeStart) {
            neighbor->sendDDPacket();
        }
        if (!ddPacket->getDdOptions().mBit &&
            (neighbor->getDatabaseSummaryListCount() == 0))
        {
            neighbor->processEvent(OSPFv3Neighbor::EXCHANGE_DONE);    // does nothing in ExchangeStart
        }
    }
    return true;
}//preProcessDDPacket

//--------------------------------------------- Link State Requests --------------------------------------------//

void OSPFv3Interface::processLSR(OSPFv3Packet* packet, OSPFv3Neighbor* neighbor){
    OSPFv3LinkStateRequest* lsr = check_and_cast<OSPFv3LinkStateRequest* >(packet);
    EV_DEBUG << "Processing LSR Packet\n";

    bool error = false;
    std::vector<OSPFv3LSA *> lsas;

    //parse the packet only if the neighbor is in EXCHANGE, LOADING or FULL state
    if(neighbor->getState()==OSPFv3Neighbor::EXCHANGE_STATE || neighbor->getState() == OSPFv3Neighbor::LOADING_STATE
            || neighbor->getState() == OSPFv3Neighbor::FULL_STATE) {
        //getting lsa headers from request
        for(unsigned int i=0; i<lsr->getRequestsArraySize(); i++){
            OSPFv3LSRequest& request = lsr->getRequests(i);
            LSAKeyType lsaKey;

            EV_INFO << "    LSARequest: type=" << request.lsaType
                    << ", LSID=" << request.lsaID
                    << ", advertisingRouter=" << request.advertisingRouter
                    << "\n";

            lsaKey.LSType = request.lsaType;
            lsaKey.linkStateID = request.lsaID;
            lsaKey.advertisingRouter = request.advertisingRouter;

            OSPFv3LSA *lsaInDatabase = this->getArea()->getInstance()->getProcess()->findLSA(lsaKey, this->getArea()->getAreaID(), this->getArea()->getInstance()->getInstanceID());

            if (lsaInDatabase != nullptr) {
                lsas.push_back(lsaInDatabase);
            }
            else {
                error = true;
                EV_DEBUG << "Somehow I got here...\n ";
                neighbor->processEvent(OSPFv3Neighbor::BAD_LINK_STATE_REQUEST);
                break;
            }
        }


        if(!error) {
            int updateCount = lsas.size();
            int hopLimit = (this->getType() == OSPFv3Interface::VIRTUAL_TYPE) ? VIRTUAL_LINK_TTL : 1;

            //create a common LSU Header
            OSPFv3LSUpdate *updatePacket = this->prepareLSUHeader();

            //add LSAs to the packet
            for (int j = 0; j < updateCount; j++) {
                updatePacket = this->prepareUpdatePacket(lsas[j], updatePacket);
            }

            if (updatePacket != nullptr) {
                if (this->getType() == OSPFv3Interface::BROADCAST_TYPE) {
                    if ((this->getState() == OSPFv3Interface::INTERFACE_STATE_DESIGNATED) ||
                            //I don't think that this is ok(this->getState() == OSPFv3Interface::INTERFACE_STATE_BACKUP) ||
                            (this->getDesignatedID() == IPv4Address::UNSPECIFIED_ADDRESS))
                    {
                        this->getArea()->getInstance()->getProcess()->sendPacket(updatePacket, IPv6Address::ALL_OSPF_ROUTERS_MCAST, this->getIntName().c_str(), hopLimit);
                    }
                    else {
                        this->getArea()->getInstance()->getProcess()->sendPacket(updatePacket, IPv6Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST, this->getIntName().c_str(), hopLimit);
                    }
                }
                else {
                    if (this->getType() == OSPFv3Interface::POINTTOPOINT_TYPE) {
                        this->getArea()->getInstance()->getProcess()->sendPacket(updatePacket, IPv6Address::ALL_OSPF_ROUTERS_MCAST, this->getIntName().c_str(), hopLimit);
                    }
                    else {
                        this->getArea()->getInstance()->getProcess()->sendPacket(updatePacket, neighbor->getNeighborIP(), this->getIntName().c_str(), hopLimit);
                    }
                }
            }
        }
    }
    else//otherwise just ignore it
        delete(packet);
}


//-------------------------------------------- Link State Updates --------------------------------------------//
OSPFv3LSUpdate* OSPFv3Interface::prepareLSUHeader()
{
    EV_DEBUG << "Preparing LSU HEADER\n";
    OSPFv3LSUpdate* updatePacket = new OSPFv3LSUpdate();

    updatePacket->setType(LSU);
    updatePacket->setRouterID(this->getArea()->getInstance()->getProcess()->getRouterID());
    updatePacket->setAreaID(this->getArea()->getAreaID());
    updatePacket->setInstanceID(this->getArea()->getInstance()->getInstanceID());

    updatePacket->setPacketLength(OSPFV3_HEADER_LENGTH+4);//+4 to include the LSAcount
    updatePacket->setLsaCount(0);

    return updatePacket;
}

OSPFv3LSUpdate* OSPFv3Interface::prepareUpdatePacket(OSPFv3LSA *lsa, OSPFv3LSUpdate* updatePacket)
{
    EV_DEBUG << "Preparing LSU\n";
    int count = updatePacket->getLsaCount();
    uint16_t packetLength = updatePacket->getPacketLength();

    OSPFv3LSAHeader header = lsa->getHeader();
    uint16_t code = lsa->getHeader().getLsaType();

    switch(code){
        case ROUTER_LSA:
        {
            int pos = updatePacket->getRouterLSAsArraySize();
            OSPFv3RouterLSA* routerLSA = dynamic_cast<OSPFv3RouterLSA*>(lsa);
            updatePacket->setRouterLSAsArraySize(pos+1);
            updatePacket->setRouterLSAs(pos, *routerLSA);
            updatePacket->setLsaCount(count+1);
            packetLength += calculateLSASize(lsa);
            updatePacket->setPacketLength(packetLength);
            updatePacket->setByteLength(packetLength);
            //TODO - check max age
            break;
        }
        case NETWORK_LSA:
        {
            int pos = updatePacket->getNetworkLSAsArraySize();
            OSPFv3NetworkLSA* networkLSA = dynamic_cast<OSPFv3NetworkLSA*>(lsa);
            updatePacket->setNetworkLSAsArraySize(pos+1);
            updatePacket->setNetworkLSAs(pos, *networkLSA);
            updatePacket->setLsaCount(count+1);
            packetLength += calculateLSASize(lsa);
            updatePacket->setPacketLength(packetLength);
            updatePacket->setByteLength(packetLength);
            break;
        }

        case INTER_AREA_PREFIX_LSA:
        {
            int pos = updatePacket->getInterAreaPrefixLSAsArraySize();
            OSPFv3InterAreaPrefixLSA* prefixLSA = dynamic_cast<OSPFv3InterAreaPrefixLSA*>(lsa);
            updatePacket->setInterAreaPrefixLSAsArraySize(pos+1);
            updatePacket->setInterAreaPrefixLSAs(pos, *prefixLSA);
            updatePacket->setLsaCount(count+1);
            packetLength += calculateLSASize(lsa);
            updatePacket->setPacketLength(packetLength);
            updatePacket->setByteLength(packetLength);
            break;
        }

        case INTER_AREA_ROUTER_LSA:
            break;

        case AS_EXTERNAL_LSA:
            break;

        case DEPRECATED:
            break;

        case NSSA_LSA:
            break;

        case LINK_LSA:
        {
            int pos = updatePacket->getLinkLSAsArraySize();
            OSPFv3LinkLSA* linkLSA = dynamic_cast<OSPFv3LinkLSA*>(lsa);
            updatePacket->setLinkLSAsArraySize(pos+1);
            updatePacket->setLinkLSAs(pos, *linkLSA);
            updatePacket->setLsaCount(count+1);
            packetLength += calculateLSASize(lsa);
            updatePacket->setPacketLength(packetLength);
            updatePacket->setByteLength(packetLength);
            break;
        }

        case INTRA_AREA_PREFIX_LSA:
        {
            int pos = updatePacket->getIntraAreaPrefixLSAsArraySize();
            OSPFv3IntraAreaPrefixLSA* prefixLSA = dynamic_cast<OSPFv3IntraAreaPrefixLSA*>(lsa);
            updatePacket->setIntraAreaPrefixLSAsArraySize(pos+1);
            updatePacket->setIntraAreaPrefixLSAs(pos, *prefixLSA);
            updatePacket->setLsaCount(count+1);
            packetLength += calculateLSASize(lsa);
            updatePacket->setPacketLength(packetLength);
            updatePacket->setByteLength(packetLength);
            break;
        }
    }

    return updatePacket;
}//prepareUpdatePacekt

void OSPFv3Interface::processLSU(OSPFv3Packet* packet, OSPFv3Neighbor* neighbor){
    OSPFv3LSUpdate* lsUpdatePacket = dynamic_cast<OSPFv3LSUpdate*>(packet);
    bool rebuildRoutingTable = false;

    if(neighbor->getState()>=OSPFv3Neighbor::EXCHANGE_STATE) {
        EV_DEBUG << "Processing LSU\n";
        int currentType = ROUTER_LSA;
        IPv4Address areaID = lsUpdatePacket->getAreaID();
        OSPFv3Area *area = this->getArea();
        unsigned int currentLSAIndex = 0;

        //First I get count from one array
        while (currentType >= ROUTER_LSA && currentType <= INTRA_AREA_PREFIX_LSA){//AS_EXTERNAL_LSA) {TODO
            unsigned int lsaCount = 0;
            switch (currentType) {
            case ROUTER_LSA:
                lsaCount = lsUpdatePacket->getRouterLSAsArraySize();
                EV_DEBUG << "Parsing ROUTER_LSAs, lsaCount = " << lsaCount << "\n";
                break;

            case NETWORK_LSA:
                lsaCount = lsUpdatePacket->getNetworkLSAsArraySize();
                EV_DEBUG << "Parsing NETWORK_LSAs, lsaCount = " << lsaCount << "\n";
                break;

            case INTER_AREA_PREFIX_LSA:
                lsaCount = lsUpdatePacket->getInterAreaPrefixLSAsArraySize();
                EV_DEBUG << "Parsing InterAreaPrefixLSAs, lsaCount = " << lsaCount << endl;
                break;
            case INTER_AREA_ROUTER_LSA:
                currentType++;
                continue;
//                lsaCount = lsUpdatePacket->getSummaryLSAsArraySize();
                break;

            case AS_EXTERNAL_LSA:
                currentType+=2;
                continue;
//                lsaCount = lsUpdatePacket->getAsExternalLSAsArraySize();
                break;

            case NSSA_LSA:
                currentType++;
                continue;
                break;

            case LINK_LSA:
                lsaCount = lsUpdatePacket->getLinkLSAsArraySize();
                EV_DEBUG << "Parsing LINK_LSAs, lsaCount = " << lsaCount << "\n";
                break;

            case INTRA_AREA_PREFIX_LSA:
                lsaCount = lsUpdatePacket->getIntraAreaPrefixLSAsArraySize();
                EV_DEBUG << "Parsing INTRA_AREA_PREFIX_LSAs, lsaCount = " << lsaCount << "\n";
                break;
            default:
                throw cRuntimeError("Invalid currentType:%d", currentType);
            }

            for (unsigned int i = 0; i < lsaCount; i++) {
                OSPFv3LSA *currentLSA;

                switch (currentType) {
                case ROUTER_LSA:
                    currentLSA = (&(lsUpdatePacket->getRouterLSAs(i)));
                    break;

                case NETWORK_LSA:
                    EV_DEBUG << "Caught NETWORK_LSA in LSU\n";
                    currentLSA = (&(lsUpdatePacket->getNetworkLSAs(i)));
                    break;

                case INTER_AREA_PREFIX_LSA:
                    EV_DEBUG << "Caught INTER_AREA_PREFIX_LSA in Update\n";
                    currentLSA = (&(lsUpdatePacket->getInterAreaPrefixLSAs(i)));
                    break;//TODO - check this whether they are the same and what to do with them

                case INTER_AREA_ROUTER_LSA:

                    //                    currentLSA = (&(lsUpdatePacket->getSummaryLSAs(i)));
                    break;

                case AS_EXTERNAL_LSA:
                    //                    currentLSA = (&(lsUpdatePacket->getAsExternalLSAs(i)));
                    break;

                case NSSA_LSA:
                    break;

                case LINK_LSA:
                    currentLSA = (&(lsUpdatePacket->getLinkLSAs(i)));
                    break;

                case INTRA_AREA_PREFIX_LSA:
                    currentLSA = (&(lsUpdatePacket->getIntraAreaPrefixLSAs(i)));
                    break;

                default:
                    throw cRuntimeError("Invalid currentType:%d", currentType);
                }


                LSAKeyType lsaKey;

                lsaKey.linkStateID = currentLSA->getHeader().getLinkStateID();
                lsaKey.advertisingRouter = currentLSA->getHeader().getAdvertisingRouter();
                lsaKey.LSType = currentLSA->getHeader().getLsaType();

                OSPFv3LSA *lsaInDatabase = this->getArea()->getInstance()->getProcess()->findLSA(lsaKey, areaID, lsUpdatePacket->getInstanceID());

                unsigned short lsAge = currentLSA->getHeader().getLsaAge();
                AcknowledgementFlags ackFlags;

                if(lsaInDatabase!=nullptr)
                    EV_DEBUG << "LSA is already in database\n";
                else
                    EV_DEBUG << "LSA is new - adding to database\n";

                uint16_t lsaType = currentLSA->getHeader().getLsaType();
                ackFlags.floodedBackOut = false;
                ackFlags.lsaIsNewer = false;
                ackFlags.lsaIsDuplicate = false;
                ackFlags.impliedAcknowledgement = false;
                ackFlags.lsaReachedMaxAge = (lsAge == MAX_AGE);
                ackFlags.noLSAInstanceInDatabase = (lsaInDatabase == nullptr);

                //LSA has max_age, it is not in the database and no router is in exchange or loading state
                if ((ackFlags.lsaReachedMaxAge) && (ackFlags.noLSAInstanceInDatabase) && (!ackFlags.anyNeighborInExchangeOrLoadingState)) {
                    //a) send ACK
                    if (this->getType() == OSPFv3Interface::BROADCAST_TYPE) {
                        if ((this->getState() == OSPFv3Interface::INTERFACE_STATE_DESIGNATED) ||
                                (this->getState() == OSPFv3Interface::INTERFACE_STATE_BACKUP) ||
                                (this->getDesignatedID() == IPv4Address::UNSPECIFIED_ADDRESS))
                        {
                            EV_DEBUG << "Sending ACK to all\n";//TODO
                            this->sendLSAcknowledgement(&(currentLSA->getHeader()), IPv6Address::ALL_OSPF_ROUTERS_MCAST);
                        }
                        else {
                            EV_DEBUG << "Sending ACK to Designated mcast\n";//TODO
                            this->sendLSAcknowledgement(&(currentLSA->getHeader()), IPv6Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST);
                        }
                    }
                    else {
                        if (this->getType() == OSPFv3Interface::POINTTOPOINT_TYPE) {
                            EV_DEBUG << "Sending ACK to all\n";//TODO
                            this->sendLSAcknowledgement(&(currentLSA->getHeader()), IPv6Address::ALL_OSPF_ROUTERS_MCAST);
                        }
                        else {
                            EV_DEBUG << "Sending ACK only to neighbor\n";//TODO
                            this->sendLSAcknowledgement(&(currentLSA->getHeader()), neighbor->getNeighborIP());
                        }
                    }
                    //b)discard
                    continue;
                }

                //LSA is in database
                if (!ackFlags.noLSAInstanceInDatabase) {
                    // operator< and operator== on OSPFLSAHeaders determines which one is newer(less means older)
                    ackFlags.lsaIsNewer = (lsaInDatabase->getHeader() < currentLSA->getHeader());
                    ackFlags.lsaIsDuplicate = (operator==(lsaInDatabase->getHeader(), currentLSA->getHeader()));
                }

                if ((ackFlags.noLSAInstanceInDatabase) || (ackFlags.lsaIsNewer)) {
                    EV_DEBUG << "No lsa instance in database\n";
                    //a) LSA in database and it was installed less than MinLsArrival seconds ago
                    if ((!ackFlags.noLSAInstanceInDatabase) &&
                            (lsaInDatabase->getHeader().getAdvertisingRouter() != this->getArea()->getInstance()->getProcess()->getRouterID()))
                        //&&//TODO - install time - how?!?!
                        //(info->getInstallTime() < MIN_LS_ARRIVAL))
                    {//it should be discarded and no ack should be sent
                        EV_DEBUG << "Discarding\n";
                        continue;
                    }

                    //b)immediately flood the LSA
                    EV_DEBUG << "Flooding the LSA out\n";
                    ackFlags.floodedBackOut = this->getArea()->getInstance()->getProcess()->floodLSA(currentLSA, areaID, this, neighbor);
                    if (!ackFlags.noLSAInstanceInDatabase) {
                        LSAKeyType lsaKey;

                        lsaKey.linkStateID = lsaInDatabase->getHeader().getLinkStateID();
                        lsaKey.advertisingRouter = lsaInDatabase->getHeader().getAdvertisingRouter();
                        lsaKey.LSType = lsaInDatabase->getHeader().getLsaType();
                        //c) remove the copy from retransmission lists
                        //this->getArea()->getInstance()->removeFromAllRetransmissionLists(lsaKey);
                    }
                    //d) install LSA in the database

                    EV_DEBUG << "Installing the LSA\n";
                    if(currentType == LINK_LSA)
                        rebuildRoutingTable=this->getArea()->getInstance()->getProcess()->installLSA(currentLSA, this->getArea()->getInstance()->getInstanceID(), this->getArea()->getAreaID(), this);
                    else if(currentType == ROUTER_LSA || currentType == NETWORK_LSA || currentType == INTER_AREA_PREFIX_LSA || currentType == INTRA_AREA_PREFIX_LSA)
                        rebuildRoutingTable=this->getArea()->getInstance()->getProcess()->installLSA(currentLSA, this->getArea()->getInstance()->getInstanceID(), this->getArea()->getAreaID());

                    if(currentType == INTER_AREA_PREFIX_LSA) {
                        this->getArea()->originateInterAreaPrefixLSA(currentLSA, this->getArea());
                        /*int areaCnt = this->getArea()->getInstance()->getAreaCount();
                        for(int i=0; i<areaCnt; i++){
                            OSPFv3Area* area = this->getArea()->getInstance()->getArea(i);
                            if(area->getAreaID() == this->getArea()->getAreaID())
                                continue;

                            OSPFv3LSUpdate* updatePacket = this->prepareLSUHeader();
                            updatePacket->setAreaID(area->getAreaID());
                            updatePacket = this->prepareUpdatePacket(currentLSA, updatePacket);

                            int intfCnt = area->getInterfaceCount();
                            for(int j=0; j<intfCnt; j++) {
                                OSPFv3Interface* intf = area->getInterface(j);
                                area->getInstance()->getProcess()->sendPacket(updatePacket, IPv6Address::ALL_OSPF_ROUTERS_MCAST, intf->getIntName().c_str(), 1);
                            }
                        }*/
                    }

                    EV_INFO << "    (update installed)\n";

                    this->addDelayedAcknowledgement(currentLSA->getHeader());
//                    this->acknowledgeLSA(&(currentLSA->getHeader()), ackFlags, lsUpdatePacket->getRouterID());
                    if ((currentLSA->getHeader().getAdvertisingRouter() == this->getArea()->getInstance()->getProcess()->getRouterID()) ||
                            ((lsaType == NETWORK_LSA)))// &&//TODO
                        //(router->isLocalAddress(currentLSA->getHeader().getLinkStateID()))))
                    {
                        if (ackFlags.noLSAInstanceInDatabase) {
                            currentLSA->getHeader().setLsaAge(MAX_AGE);
                            if(currentLSA->getHeader().getLsaType()!=LINK_LSA)
                                this->getArea()->getInstance()->getProcess()->floodLSA(currentLSA, areaID, this);
                            else {
                                LSAKeyType lsaKey;

                                lsaKey.linkStateID = currentLSA->getHeader().getLinkStateID();
                                lsaKey.advertisingRouter = currentLSA->getHeader().getAdvertisingRouter();
                                lsaKey.LSType = currentLSA->getHeader().getLsaType();
                                neighbor->removeFromRequestList(lsaKey);
                            }
                        }
                        else {
                            if (ackFlags.lsaIsNewer) {
                                long sequenceNumber = currentLSA->getHeader().getLsaSequenceNumber();
                                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                                    lsaInDatabase->getHeader().setLsaAge(MAX_AGE);
                                    this->getArea()->getInstance()->getProcess()->floodLSA(lsaInDatabase, areaID);
                                }
                                else {
                                    lsaInDatabase->getHeader().setLsaSequenceNumber(sequenceNumber + 1);
                                    this->getArea()->getInstance()->getProcess()->floodLSA(lsaInDatabase, areaID);
                                }
                            }
                        }
                    }
                    continue;
                }
                //TODO - LSA is on request list
                //TODO - lsa is a duplicate
                //TODO - database copy is more recent
                //TODO - Otherwise as long as the database copy has not been sent in link state update send it to the neighbor directly


            }//for (unsigned int i = 0; i < lsaCount; i++)

            currentType++;
        } //while (currentType >= ROUTER_LSA && currentType <= LINK_LSA){



    } //if(neighbor->getState()>=OSPFv3Neighbor::EXCHANGE_STATE)
    else {
        EV_DEBUG << "Neighbor in lesser than EXCHANGE_STATE -> Drop the packet\n";
    }

    if(rebuildRoutingTable)
        this->getArea()->getInstance()->getProcess()->rebuildRoutingTable();
}//processLSU

void OSPFv3Interface::processLSAck(OSPFv3Packet* packet){

}//processLSAck

void OSPFv3Interface::acknowledgeLSA(OSPFv3LSAHeader* lsaHeader,
        AcknowledgementFlags acknowledgementFlags,
        IPv4Address lsaSource)
{
    EV_DEBUG << "AcknowledgeLSA\n";
    bool sendDirectAcknowledgment = false;

    if (!acknowledgementFlags.floodedBackOut) {
        EV_DEBUG << "\tFloodedBackOut is false\n";
        if (this->getState() == OSPFv3Interface::INTERFACE_STATE_BACKUP) {
            if ((acknowledgementFlags.lsaIsNewer && (lsaSource == this->getDesignatedID())) ||
                (acknowledgementFlags.lsaIsDuplicate && acknowledgementFlags.impliedAcknowledgement))
            {
                EV_DEBUG << "Adding delayed acknowledgement\n";
                this->addDelayedAcknowledgement(*lsaHeader);
            }
            else {
                if ((acknowledgementFlags.lsaIsDuplicate && !acknowledgementFlags.impliedAcknowledgement) ||
                    (acknowledgementFlags.lsaReachedMaxAge &&
                     acknowledgementFlags.noLSAInstanceInDatabase &&
                     acknowledgementFlags.anyNeighborInExchangeOrLoadingState))
                {
                    sendDirectAcknowledgment = true;
                }
            }
        }
        else {
            if (acknowledgementFlags.lsaIsNewer) {
                EV_DEBUG << "Adding delayed acknowledgement\n";
                this->addDelayedAcknowledgement(*lsaHeader);
            }
            else {
                if ((acknowledgementFlags.lsaIsDuplicate && !acknowledgementFlags.impliedAcknowledgement) ||
                    (acknowledgementFlags.lsaReachedMaxAge &&
                     acknowledgementFlags.noLSAInstanceInDatabase &&
                     acknowledgementFlags.anyNeighborInExchangeOrLoadingState))
                {
                    sendDirectAcknowledgment = true;
                }
            }
        }
    }

    if (sendDirectAcknowledgment) {
        OSPFv3LSAck *ackPacket = new OSPFv3LSAck();

        ackPacket->setType(LS_ACK);
        ackPacket->setRouterID(this->getArea()->getInstance()->getProcess()->getRouterID());
        ackPacket->setAreaID(this->getArea()->getAreaID());

        ackPacket->setLsaHeadersArraySize(1);
        ackPacket->setLsaHeaders(0, *lsaHeader);

        ackPacket->setByteLength(OSPFV3_HEADER_LENGTH + OSPFV3_LSA_HEADER_LENGTH);

        int hopLimit = (this->getType() == OSPFv3Interface::VIRTUAL_TYPE) ? VIRTUAL_LINK_TTL : 1;

        if (this->getType() == OSPFv3Interface::BROADCAST_TYPE) {
            if ((this->getState() == OSPFv3Interface::INTERFACE_STATE_DESIGNATED) ||
                (this->getState() == OSPFv3Interface::INTERFACE_STATE_BACKUP) ||
                (this->getDesignatedID() == IPv4Address::UNSPECIFIED_ADDRESS))
            {
                this->getArea()->getInstance()->getProcess()->sendPacket(ackPacket, IPv6Address::ALL_OSPF_ROUTERS_MCAST, this->getIntName().c_str(), hopLimit);
            }
            else {
                this->getArea()->getInstance()->getProcess()->sendPacket(ackPacket, IPv6Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST, this->getIntName().c_str(), hopLimit);
            }
        }
        else {
            if (this->getType() == OSPFv3Interface::POINTTOPOINT_TYPE) {
                this->getArea()->getInstance()->getProcess()->sendPacket(ackPacket, IPv6Address::ALL_OSPF_ROUTERS_MCAST, this->getIntName().c_str(), hopLimit);
            }
            else {
                OSPFv3Neighbor *neighbor = this->getNeighborById(lsaSource);
                ASSERT(neighbor);
                this->getArea()->getInstance()->getProcess()->sendPacket(ackPacket, neighbor->getNeighborIP(), this->getIntName().c_str(), hopLimit);
            }
        }
    }
}//acknowledgeLSA


//--------------------------------------- Link State Advertisements -----------------------------------------//
void OSPFv3Interface::sendLSAcknowledgement(OSPFv3LSAHeader *lsaHeader, IPv6Address destination)
{
    OSPFv3Options options;
    OSPFv3LSAck *lsAckPacket = new OSPFv3LSAck();

    lsAckPacket->setType(LS_ACK);
    lsAckPacket->setRouterID(this->getArea()->getInstance()->getProcess()->getRouterID());
    lsAckPacket->setAreaID(this->getArea()->getAreaID());

    lsAckPacket->setLsaHeadersArraySize(1);
    lsAckPacket->setLsaHeaders(0, *lsaHeader);

    lsAckPacket->setByteLength(OSPFV3_HEADER_LENGTH + OSPFV3_LSA_HEADER_LENGTH);

    int hopLimit = (interfaceType == OSPFv3Interface::VIRTUAL_TYPE) ? VIRTUAL_LINK_TTL : 1;
    this->getArea()->getInstance()->getProcess()->sendPacket(lsAckPacket, destination, this->getIntName().c_str(), hopLimit);
}//sendLSAcknowledgement

void OSPFv3Interface::addDelayedAcknowledgement(OSPFv3LSAHeader& lsaHeader)
{
    if (interfaceType == OSPFv3Interface::BROADCAST_TYPE) {
        if ((getState() == OSPFv3Interface::INTERFACE_STATE_DESIGNATED) ||
                (getState() == OSPFv3Interface::INTERFACE_STATE_BACKUP) ||
                (this->DesignatedRouterID == IPv4Address::UNSPECIFIED_ADDRESS))
        {
            delayedAcknowledgements[IPv6Address::ALL_OSPF_ROUTERS_MCAST].push_back(lsaHeader);
        }
        else {
            delayedAcknowledgements[IPv6Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST].push_back(lsaHeader);
        }
    }
    else {
        long neighborCount = this->neighbors.size();
        for (long i = 0; i < neighborCount; i++) {
            if (this->neighbors.at(i)->getState() >= OSPFv3Neighbor::EXCHANGE_STATE) {
                delayedAcknowledgements[this->neighbors.at(i)->getNeighborIP()].push_back(lsaHeader);
            }
        }
    }

    if(!this->acknowledgementTimer->isScheduled())
        this->getArea()->getInstance()->getProcess()->setTimer(this->acknowledgementTimer, 1);
}//addDelayedAcknowledgement

void OSPFv3Interface::sendDelayedAcknowledgements()
{
    for (auto & elem : delayedAcknowledgements)
    {
        int ackCount = elem.second.size();
        if (ackCount > 0) {
            while (!(elem.second.empty())) {
                OSPFv3LSAck *ackPacket = new OSPFv3LSAck();
                long packetSize = OSPFV3_HEADER_LENGTH;

                ackPacket->setType(LS_ACK);
                ackPacket->setRouterID(this->getArea()->getInstance()->getProcess()->getRouterID());
                ackPacket->setAreaID(this->getArea()->getAreaID());
                ackPacket->setChecksum(0);
                ackPacket->setInstanceID(this->getArea()->getInstance()->getInstanceID());
                ackPacket->setLsas(ackCount);

                while (!elem.second.empty()) {
                    unsigned long headerCount = ackPacket->getLsaHeadersArraySize();
                    ackPacket->setLsaHeadersArraySize(headerCount + 1);
                    ackPacket->setLsaHeaders(headerCount, *(elem.second.begin()));
                    elem.second.pop_front();
                    packetSize += OSPFV3_LSA_HEADER_LENGTH;
                }

                ackPacket->setByteLength(packetSize);
                ackPacket->setPacketLength(packetSize);

                int ttl = (interfaceType == OSPFv3Interface::VIRTUAL_TYPE) ? VIRTUAL_LINK_TTL : 1;

                if (interfaceType == OSPFv3Interface::BROADCAST_TYPE) {
                    if ((getState() == OSPFv3Interface::INTERFACE_STATE_DESIGNATED) ||
                            (getState() == OSPFv3Interface::INTERFACE_STATE_BACKUP) ||
                            (this->DesignatedRouterID == IPv4Address::UNSPECIFIED_ADDRESS))
                    {
                        this->getArea()->getInstance()->getProcess()->sendPacket(ackPacket, IPv6Address::ALL_OSPF_ROUTERS_MCAST, this->getIntName().c_str(), ttl);
                    }
                    else {
                        this->getArea()->getInstance()->getProcess()->sendPacket(ackPacket, IPv6Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST, this->getIntName().c_str(), ttl);
                    }
                }
                else {
                    if (interfaceType == OSPFv3Interface::POINTTOPOINT_TYPE) {
                        this->getArea()->getInstance()->getProcess()->sendPacket(ackPacket, IPv6Address::ALL_OSPF_ROUTERS_MCAST, this->getIntName().c_str(), ttl);
                    }
                    else {
                        this->getArea()->getInstance()->getProcess()->sendPacket(ackPacket, elem.first, this->getIntName().c_str(), ttl);
                    }
                }
            }
        }
    }
    this->getArea()->getInstance()->getProcess()->clearTimer(this->acknowledgementTimer);
}//sendDelayedAcknowledgements


//-------------------------------------------- Flooding ---------------------------------------------//
bool OSPFv3Interface::floodLSA(OSPFv3LSA* lsa, OSPFv3Interface* interface, OSPFv3Neighbor* neighbor)
{
    bool floodedBackOut = false;

    if (
            (
                    (lsa->getHeader().getLsaType() == AS_EXTERNAL_LSA) &&
                    (interfaceType != OSPFv3Interface::VIRTUAL_TYPE) &&
                    (this->getArea()->getExternalRoutingCapability())
            ) ||
            (
                    (lsa->getHeader().getLsaType() != AS_EXTERNAL_LSA) &&
                    (
                            (
                                    (this->getArea()->getAreaID() != IPv4Address::UNSPECIFIED_ADDRESS) &&
                                    (interfaceType != OSPFv3Interface::VIRTUAL_TYPE)
                            ) ||
                            (this->getArea()->getAreaID() == IPv4Address::UNSPECIFIED_ADDRESS)
                    )
            )
    )
    {
        EV_DEBUG << "Checking if this is backbone\n";
        long neighborCount = this->getNeighborCount();
        bool lsaAddedToRetransmissionList = false;
        IPv4Address linkStateID = lsa->getHeader().getLinkStateID();
        LSAKeyType lsaKey;

        lsaKey.linkStateID = linkStateID;
        lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();
        lsaKey.LSType = lsa->getHeader().getLsaType();

        for (long i = 0; i < neighborCount; i++) {    // (1)
            if (this->neighbors.at(i)->getState() < OSPFv3Neighbor::EXCHANGE_STATE) {    // (1) (a)
                EV_DEBUG << "Skipping neighbor " << this->neighbors.at(i)->getNeighborID() << "\n";
                continue;
            }
            if (this->neighbors.at(i)->getState() < OSPFv3Neighbor::FULL_STATE) {    // (1) (b)
                EV_DEBUG << "Adjacency not yet full\n";
                OSPFv3LSAHeader *requestLSAHeader = this->neighbors.at(i)->findOnRequestList(lsaKey);
                if (requestLSAHeader != nullptr) {
                    EV_DEBUG << "Instance of new lsa already on the list\n";
                    // operator< and operator== on OSPFLSAHeaders determines which one is newer(less means older)
                    if (lsa->getHeader() < (*requestLSAHeader)) {
                        EV_DEBUG << "Instance is less recent\n";
                        continue;
                    }
                    if (operator==(lsa->getHeader(), (*requestLSAHeader))) {
                        EV_DEBUG << "Two instances are the same, removing from request list\n";
                        this->neighbors.at(i)->removeFromRequestList(lsaKey);
                        continue;
                    }
                    this->neighbors.at(i)->removeFromRequestList(lsaKey);
                }
            }
            if (neighbor == this->neighbors.at(i)) {    // (1) (c)
                EV_DEBUG << "1c - next neighbor\n";
                continue;
            }
            this->neighbors.at(i)->addToRetransmissionList(lsa);    // (1) (d)
            lsaAddedToRetransmissionList = true;
        }
        if (lsaAddedToRetransmissionList) {    // (2)
            EV_DEBUG << "lsaAddedToRetransmissionList true\n";
            if ((interface != this) ||
                    ((neighbor != nullptr) &&
                            (neighbor->getNeighborID() != this->getDesignatedID()) &&
                            (neighbor->getNeighborID() != this->getBackupID())))    // (3)
            {
                EV_DEBUG << "step 3 passed\n";
                if ((interface != this) || (getState() != OSPFv3Interface::INTERFACE_STATE_BACKUP)) {    // (4)
                    EV_DEBUG << "step 4 passed\n";
                    OSPFv3LSUpdate* updatePacket = this->prepareLSUHeader();   // (5)
                    updatePacket = this->prepareUpdatePacket(lsa, updatePacket);

                    if (updatePacket != nullptr) {
                        EV_DEBUG << "Prepared LSUpdate packet is ready\n";
                        int hopLimit = (interfaceType == OSPFv3Interface::VIRTUAL_TYPE) ? VIRTUAL_LINK_TTL : 1;

                        if (interfaceType == OSPFv3Interface::BROADCAST_TYPE) {
                            if ((getState() == OSPFv3Interface::INTERFACE_STATE_DESIGNATED) ||
                                    (getState() == OSPFv3Interface::INTERFACE_STATE_BACKUP) ||
                                    (this->DesignatedRouterID == IPv4Address::UNSPECIFIED_ADDRESS))
                            {
                                EV_DEBUG << "Sending LSUpdate packet\n";
                                this->getArea()->getInstance()->getProcess()->sendPacket(updatePacket, IPv6Address::ALL_OSPF_ROUTERS_MCAST, this->getIntName().c_str(), hopLimit);
                                for (long k = 0; k < neighborCount; k++) {
                                    this->neighbors.at(k)->addToTransmittedLSAList(lsaKey);
                                    if (!this->neighbors.at(k)->isUpdateRetransmissionTimerActive()) {
                                        EV_DEBUG << "The timer is not active\n";
                                        this->neighbors.at(k)->startUpdateRetransmissionTimer();
                                        EV_DEBUG << "Takze ho nastavim\n";
                                    }
                                }
                            }
                            else {
                                EV_DEBUG << "Sending packet from floodLSA\n";
                                this->getArea()->getInstance()->getProcess()->sendPacket(updatePacket, IPv6Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST, this->getIntName().c_str(), hopLimit);
                                OSPFv3Neighbor *dRouter = this->getNeighborById(this->DesignatedRouterID);
                                OSPFv3Neighbor *backupDRouter = this->getNeighborById(this->BackupRouterID);
                                if (dRouter != nullptr) {
                                    dRouter->addToTransmittedLSAList(lsaKey);
                                    if (!dRouter->isUpdateRetransmissionTimerActive()) {
                                        dRouter->startUpdateRetransmissionTimer();
                                    }
                                }
                                if (backupDRouter != nullptr) {
                                    backupDRouter->addToTransmittedLSAList(lsaKey);
                                    if (!backupDRouter->isUpdateRetransmissionTimerActive()) {
                                        backupDRouter->startUpdateRetransmissionTimer();
                                    }
                                }
                            }
                        }
                        else {
                            if (interfaceType == OSPFv3Interface::POINTTOPOINT_TYPE) {
                                this->getArea()->getInstance()->getProcess()->sendPacket(updatePacket, IPv6Address::ALL_OSPF_ROUTERS_MCAST, this->getIntName().c_str(), hopLimit);
                                if (neighborCount > 0) {
                                    this->neighbors.at(0)->addToTransmittedLSAList(lsaKey);
                                    if (!this->neighbors.at(0)->isUpdateRetransmissionTimerActive()) {
                                        this->neighbors.at(0)->startUpdateRetransmissionTimer();
                                    }
                                }
                            }
                            else {
                                for (long m = 0; m < neighborCount; m++) {
                                    if (this->neighbors.at(m)->getState() >= OSPFv3Neighbor::EXCHANGE_STATE) {
                                        this->getArea()->getInstance()->getProcess()->sendPacket(updatePacket, this->neighbors.at(m)->getNeighborIP(), this->getIntName().c_str(), hopLimit);
                                        this->neighbors.at(m)->addToTransmittedLSAList(lsaKey);
                                        if (!this->neighbors.at(m)->isUpdateRetransmissionTimerActive()) {
                                            this->neighbors.at(m)->startUpdateRetransmissionTimer();
                                        }
                                    }
                                }
                            }
                        }

                        if (interface == this) {
                            floodedBackOut = true;
                        }
                    }
                }
            }
        }
    }

    return floodedBackOut;
}//floodLSA

void OSPFv3Interface::removeFromAllRetransmissionLists(LSAKeyType lsaKey)
{

}

OSPFv3Neighbor* OSPFv3Interface::getNeighborById(IPv4Address neighborId)
{
    std::map<IPv4Address, OSPFv3Neighbor*>::iterator neighborIt = this->neighborsById.find(neighborId);
    if(neighborIt == this->neighborsById.end())
        return nullptr;

    return neighborIt->second;
}//getNeighborById

void OSPFv3Interface::removeNeighborByID(IPv4Address neighborId)
{
    std::map<IPv4Address, OSPFv3Neighbor*>::iterator neighborIt = this->neighborsById.find(neighborId);
    if(neighborIt != this->neighborsById.end()) {
        this->neighborsById.erase(neighborIt);
    }

    int neighborCnt = this->getNeighborCount();
    for(int i=0; i<neighborCnt; i++){
        OSPFv3Neighbor* current = this->getNeighbor(i);
        if(current->getNeighborID() == neighborId) {
            this->neighbors.erase(this->neighbors.begin()+i);
            break;
        }
    }
}//getNeighborById

void OSPFv3Interface::addNeighbor(OSPFv3Neighbor* newNeighbor)
{
    OSPFv3Neighbor* check = this->getNeighborById(newNeighbor->getNeighborID());
    if(check==nullptr){
        this->neighbors.push_back(newNeighbor);
        this->neighborsById[newNeighbor->getNeighborID()]=newNeighbor;
    }
}//addNeighbor

OSPFv3LinkLSA* OSPFv3Interface::originateLinkLSA()
{
    OSPFv3LinkLSA* linkLSA = new OSPFv3LinkLSA();
    OSPFv3LSAHeader& lsaHeader = linkLSA->getHeader();

    //First the LSA Header
    lsaHeader.setLsaAge((int)simTime().dbl());
    lsaHeader.setLsaType(LINK_LSA);
    lsaHeader.setLinkStateID(IPv4Address(this->getInterfaceIndex()));
    lsaHeader.setAdvertisingRouter(this->getArea()->getInstance()->getProcess()->getRouterID());
    lsaHeader.setLsaSequenceNumber(this->linkLSASequenceNumber++);
//    lsaHeader.setLsaChecksum(); TODO

    uint16_t packetLength=OSPFV3_LSA_HEADER_LENGTH + OSPFV3_LINK_LSA_BODY_LENGTH;

    //Then the LSA Body
    linkLSA->setRouterPriority(this->getRouterPriority());
    //TODO - options
    OSPFv3Options lsOptions;
    memset(&lsOptions, 0, sizeof(OSPFv3Options));
    linkLSA->setOspfOptions(lsOptions);

    InterfaceEntry* ie = this->ift->getInterfaceByName(this->interfaceName.c_str());
    IPv6InterfaceData* ipv6Data = ie->ipv6Data();
    linkLSA->setLinkLocalInterfaceAdd(ipv6Data->getLinkLocalAddress());

    int numPrefixes;
    if(this->getArea()->getInstance()->getAddressFamily() == IPV4INSTANCE)
        numPrefixes = 1;
    else
        numPrefixes = ipv6Data->getNumAddresses();

    for(int i=0; i<numPrefixes; i++) {
        if(this->getArea()->getInstance()->getAddressFamily() == IPV4INSTANCE) {
            IPv4InterfaceData* ipv4Data = ie->ipv4Data();
            IPv4Address ipAdd = ipv4Data->getIPAddress();

            OSPFv3LinkLSAPrefix prefix;
            prefix.dnBit = false;
            prefix.laBit = false;
            prefix.nuBit = false;
            prefix.pBit = false;
            prefix.xBit = false;
            prefix.prefixLen = ipAdd.getNetmaskLength();
            prefix.addressPrefix = L3Address(ipAdd);//TODO - this is smaller than 16B

            linkLSA->setPrefixesArraySize(linkLSA->getPrefixesArraySize()+1);
            linkLSA->setPrefixes(i, prefix);
            packetLength+=20;
        }
        else {
            IPv6Address ipAdd = ipv6Data->getLinkLocalAddress();

//            //for some reason the ff02::5 is there as well
//            if(ipAdd.isMulticast()) {
//                numPrefixes--;
//                continue;
//            }
            EV_DEBUG << "Creating Link LSA for address: " << ipv6Data->getLinkLocalAddress() << "\n";
            OSPFv3LinkLSAPrefix prefix;
            prefix.dnBit = false;
            prefix.laBit = false;
            prefix.nuBit = false;
            prefix.pBit = false;
            prefix.xBit = false;
            prefix.prefixLen = 128;
            prefix.addressPrefix = L3Address(ipAdd);

            linkLSA->setPrefixesArraySize(linkLSA->getPrefixesArraySize()+1);
            linkLSA->setPrefixes(i, prefix);
            packetLength+=20;
        }
    }

    linkLSA->setNumPrefixes(numPrefixes);
    lsaHeader.setLsaLength(packetLength);
    this->linkLSAList.push_back(linkLSA);
    return linkLSA;
}

OSPFv3LinkLSA* OSPFv3Interface::getLinkLSAbyKey(LSAKeyType lsaKey)
{
    for (auto it=this->linkLSAList.begin(); it!=this->linkLSAList.end(); it++)
    {
        if(((*it)->getHeader().getAdvertisingRouter() == lsaKey.advertisingRouter) && (*it)->getHeader().getLinkStateID() == lsaKey.linkStateID) {
            return (*it);
        }
    }

    return nullptr;
}

bool OSPFv3Interface::installLinkLSA(OSPFv3LinkLSA *lsa)
{
    EV_DEBUG << "Link LSA is being installed in database \n";
    LSAKeyType lsaKey;
    lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
    lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();
    lsaKey.LSType = lsa->getHeader().getLsaType();

    OSPFv3LinkLSA* lsaInDatabase = this->getLinkLSAbyKey(lsaKey);
    if (lsaInDatabase != nullptr) {
        EV_DEBUG << "Link LSA is being removed from retransmission lists\n";
        this->getArea()->removeFromAllRetransmissionLists(lsaKey);
        return this->updateLinkLSA(lsaInDatabase, lsa);
    }
    else {
        OSPFv3LinkLSA* lsaCopy = new OSPFv3LinkLSA(*lsa);
        this->linkLSAList.push_back(lsaCopy);
        return true;
    }
}//installLinkLSA

bool OSPFv3Interface::updateLinkLSA(OSPFv3LinkLSA* currentLsa, OSPFv3LinkLSA* newLsa)
{
    bool different = linkLSADiffersFrom(currentLsa, newLsa);
    (*currentLsa) = (*newLsa);
    currentLsa->getHeader().setLsaAge(0);//reset the age
    if (different) {
//        clearNextHops();//TODO
        return true;
    }
    else {
        return false;
    }
}

bool OSPFv3Interface::linkLSADiffersFrom(OSPFv3LinkLSA* currentLsa, OSPFv3LinkLSA* newLsa)
{
    const OSPFv3LSAHeader& thisHeader = currentLsa->getHeader();
    const OSPFv3LSAHeader& lsaHeader = newLsa->getHeader();
    bool differentHeader = (((thisHeader.getLsaAge() == MAX_AGE) && (lsaHeader.getLsaAge() != MAX_AGE)) ||
                            ((thisHeader.getLsaAge() != MAX_AGE) && (lsaHeader.getLsaAge() == MAX_AGE)) ||
                            (thisHeader.getLsaLength() != lsaHeader.getLsaLength()));
    bool differentBody = false;

    if (!differentHeader) {
        differentBody = ((currentLsa->getRouterPriority() != newLsa->getRouterPriority()) ||
                         (currentLsa->getOspfOptions() != newLsa->getOspfOptions()) ||
                         (currentLsa->getLinkLocalInterfaceAdd() != newLsa->getLinkLocalInterfaceAdd()) ||
                         (currentLsa->getPrefixesArraySize() != newLsa->getPrefixesArraySize()));

        if (!differentBody) {
            unsigned int prefixCount = currentLsa->getPrefixesArraySize();
            for (unsigned int i = 0; i < prefixCount; i++) {
                auto thisRouter = currentLsa->getPrefixes(i);
                auto lsaRouter = newLsa->getPrefixes(i);
                bool differentLink = ((thisRouter.prefixLen != lsaRouter.prefixLen) ||
                                      (thisRouter.dnBit != lsaRouter.dnBit) ||
                                      (thisRouter.laBit != lsaRouter.laBit) ||
                                      (thisRouter.nuBit != lsaRouter.nuBit) ||
                                      (thisRouter.xBit != lsaRouter.xBit) ||
                                      (thisRouter.addressPrefix != lsaRouter.addressPrefix) ||
                                      (thisRouter.pBit != lsaRouter.pBit));

                if (differentLink) {
                    differentBody = true;
                    break;
                }
            }
        }
    }

    return differentHeader || differentBody;
}

std::string OSPFv3Interface::info() const
{
    std::stringstream out;

    out << "Interface " << this->getIntName() << " Info:\n";
    return out.str();
}//info

std::string OSPFv3Interface::detailedInfo() const
{
    std::stringstream out;
    IPv4Address neighbor;
    IPv6Address designatedIP;
    IPv6Address backupIP;

    int adjCount = 0;
    for(auto it=this->neighbors.begin(); it!=this->neighbors.end(); it++) {
        if((*it)->getState()==OSPFv3Neighbor::FULL_STATE)
            adjCount++;

        if((*it)->getNeighborID() == this->DesignatedRouterID)
            designatedIP = (*it)->getNeighborIP();
        if((*it)->getNeighborID() == this->BackupRouterID)
            backupIP = (*it)->getNeighborIP();
    }

    out << "Interface " << this->getIntName() << "\n"; //TODO - isUP?
    out << "Link Local Address ";//TODO - for over all addresses
    InterfaceEntry* ie = this->ift->getInterfaceByName(this->getIntName().c_str());
    IPv6InterfaceData *ipv6int = ie->ipv6Data();
    out << ipv6int->getLinkLocalAddress() << ", Interface ID " << this->interfaceId << "\n";

    if(this->getArea()->getInstance()->getAddressFamily() == IPV4INSTANCE) {
        IPv4InterfaceData* ipv4int = ie->ipv4Data();
        out << "Internet Address " << ipv4int->getIPAddress() << endl;
    }

    out << "Area " << this->getArea()->getAreaID().getInt();
    out << ", Process ID " << this->getArea()->getInstance()->getProcess()->getProcessID();
    out << ", Instance ID " << this->getArea()->getInstance()->getInstanceID() << ", ";
    out << "Router ID " << this->getArea()->getInstance()->getProcess()->getRouterID() << endl;

    out << "Network Type " << this->OSPFv3IntTypeOutput[this->getType()];
    out << ", Cost: " << this->getInterfaceCost() << endl;//TODO - type needs to be a string

    out << "Transmit Delay is " << this->getTransDelayInterval() << " sec, ";
    out << "State " << this->OSPFv3IntStateOutput[this->getState()];
    out << ", Priority " << this->getRouterPriority() << endl;

    out << "Designated Router (ID) " << this->getDesignatedID().str(false);
    out << ", local address " << this->getDesignatedIP() << endl;

    out << "Backup Designated router (ID) " << this->getBackupID().str(false);
    out << ", local address " << this->getBackupIP() << endl;

    out << "Timer intervals configured, Hello " << this->getHelloInterval();
    out << ", Dead " << this->getDeadInterval();
    out << ", Wait " << this->getDeadInterval();
    out << ", Retransmit " << this->getRetransmissionInterval() << endl;

    out << "\tHello due in " << (int)simTime().dbl()%this->helloInterval << endl;

    out << "Neighbor Count is " << this->getNeighborCount();
    out << ", Adjacent neighbor count is " << adjCount << endl;

    for(auto it=this->neighbors.begin(); it!=this->neighbors.end(); it++) {
        if((*it)->getNeighborID() == this->DesignatedRouterID)
            out << "Adjacent with neighbor "<< this->DesignatedRouterID << "(Designated Router)\n";
        else if((*it)->getNeighborID() == this->BackupRouterID)
            out << "Adjacent with neighbor "<< this->BackupRouterID << "(Backup Designated Router)\n";
        else if((*it)->getState() == OSPFv3Neighbor::FULL_STATE)
            out << "Adjacent with neighbor " << (*it)->getNeighborID() << endl;
    }

    out << "Suppress Hello for 0 neighbor(s)\n";

    return out.str();
}//detailedInfo
}//namespace inet
