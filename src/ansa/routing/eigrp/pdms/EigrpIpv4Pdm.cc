//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 
/**
* @file EigrpIpv4Pdm.cc
* @author Jan Bloudicek (mailto:jbloudicek@gmail.com)
* @brief EIGRP IPv4 Protocol Dependent Module
* @detail Main module, it mediates control exchange between DUAL, routing table and
topology table.
*/

#include <algorithm>

//#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
//#include "AnsaIPv4Route.h"
#include "ansa/routing/eigrp/EigrpDeviceConfigurator.h"
#include "ansa/routing/eigrp/pdms/EigrpPrint.h"
#include "ansa/routing/eigrp/pdms/EigrpIpv4Pdm.h"

#define EIGRP_DEBUG
namespace inet {
Define_Module(EigrpIpv4Pdm);



EigrpIpv4Pdm::EigrpIpv4Pdm() : EIGRP_IPV4_MULT(IPv4Address(224, 0, 0, 10))
{
    SPLITTER_OUTGW = "splitterOut";
    RTP_OUTGW = "rtpOut";

    KVALUES_MAX.K1 = KVALUES_MAX.K2 = KVALUES_MAX.K3 = KVALUES_MAX.K4 = KVALUES_MAX.K5 = KVALUES_MAX.K6 = 255;

    asNum = -1;
    maximumPath = 4;
    variance = 1;
    adminDistInt = 90;
    useClassicMetric = true;
    ribScale = 128;

    kValues.K1 = kValues.K3 = 1;
    kValues.K2 = kValues.K4 = kValues.K5 = kValues.K6 = 0;

    eigrpStubEnabled = false;
}

EigrpIpv4Pdm::~EigrpIpv4Pdm()
{
    //delete this->routingForNetworks;
    //delete this->eigrpIftDisabled;
    //delete this->eigrpDual;
    //delete this->eigrpMetric;
}

void EigrpIpv4Pdm::initialize(int stage)
{
    // in stage 0 interfaces are registrated
    // in stage 2 interfaces and routing table
    // in stage 3
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_ROUTING_PROTOCOLS)
    {
        //this->eigrpIft = EigrpIfTableAccess().get();
        //this->eigrpNt = Eigrpv4NeighTableAccess().get();
        //this->eigrpTt = Eigrpv4TopolTableAccess().get();
        //this->rt = AnsaRoutingTableAccess().get();
        eigrpIft = check_and_cast<EigrpInterfaceTable*>(getModuleByPath("^.eigrpInterfaceTable"));
        eigrpNt = check_and_cast<EigrpIpv4NeighborTable*>(getModuleByPath("^.eigrpIpv4NeighborTable"));
        eigrpTt = check_and_cast<EigrpIpv4TopologyTable*>(getModuleByPath("^.eigrpIpv4TopologyTable"));
        rt = check_and_cast<IPv4RoutingTable*>(getModuleByPath(par("routingTableModule"))->getSubmodule("ipv4"));

        //this->eigrpDual = ModuleAccess<EigrpDual>("eigrpDual").get();
        //this->ift = InterfaceTableAccess().get();
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        //this->nb = NotificationBoardAccess().get();

        this->eigrpIftDisabled = new EigrpDisabledInterfaces();
        this->routingForNetworks = new EigrpNetworkTable<IPv4Address>();
        this->eigrpDual = new EigrpDual<IPv4Address>(this);
        this->eigrpMetric = new EigrpMetricHelper();

        // Set router ID
        IPv4Address rid = rt->getRouterId();
        this->eigrpTt->setRouterId(rid);

        // Load configuration of EIGRP
        //EigrpDeviceConfigurator *conf = ModuleAccess<EigrpDeviceConfigurator>("eigrpDeviceConfigurator").get();
        EigrpDeviceConfigurator *conf = new EigrpDeviceConfigurator(par("configData"), ift);
        conf->loadEigrpIPv4Config(this);

        IPSocket ipSocket(gate(SPLITTER_OUTGW));
        ipSocket.registerProtocol(IP_PROT_EIGRP);

        WATCH_PTRVECTOR(*routingForNetworks->getAllNetworks());
        WATCH(asNum);
        WATCH(kValues.K1);
        WATCH(kValues.K2);
        WATCH(kValues.K3);
        WATCH(kValues.K4);
        WATCH(kValues.K5);
        WATCH(kValues.K6);
        WATCH(maximumPath);
        WATCH(variance);
        WATCH(eigrpStub.connectedRt);
        WATCH(eigrpStub.leakMapRt);
        WATCH(eigrpStub.recvOnlyRt);
        WATCH(eigrpStub.redistributedRt);
        WATCH(eigrpStub.staticRt);
        WATCH(eigrpStub.summaryRt);

        // Subscribe for changes in the device
        //nb->subscribe(this, NF_INTERFACE_STATE_CHANGED);
        //nb->subscribe(this, NF_INTERFACE_CONFIG_CHANGED);
        //nb->subscribe(this, NF_IPv4_ROUTE_DELETED);
        cModule* host = getParentModule()->getParentModule();
        host->subscribe(NF_INTERFACE_STATE_CHANGED, this);
        host->subscribe(NF_INTERFACE_CONFIG_CHANGED, this);
        host->subscribe(NF_ROUTE_DELETED, this);

/*
        //ANSAIPv6Address testing
        ANSAIPv6Address ipv6("FF02::A");
        EV << "IPv6 address: " << ipv6.str() << endl;
        EV << "dptr: " << ipv6.dptr << ", .words(): " << ipv6.words() << endl;

        ANSAIPv6Address ipv62 = ANSAIPv6Address();
        EV << "IPv62 address: " << ipv62.str() << endl;
        EV << "dptr: " << ipv62.dptr << ", .words(): " << ipv62.words() << endl;
        EV << "netmask length: " << ipv62.getNetmaskLength() << endl;

        ANSAIPv6Address ipv63(ipv6);
        EV << "IPv63 address: " << ipv63.str() << endl;
        EV << "dptr: " << ipv63.dptr << ", .words(): " << ipv63.words() << endl;

        ANSAIPv6Address ipv64(1,0,5,8);
        EV << "IPv64 address: " << ipv64.str() << endl;
        EV << "dptr: " << ipv64.dptr << ", .words(): " << ipv64.words() << endl;

        ANSAIPv6Address ipv65(ipv64.words());
        EV << "IPv65 address: " << ipv65.str() << endl;
        EV << "dptr: " << ipv65.dptr << ", .words(): " << ipv65.words() << endl;

        ANSAIPv6Address ipv66 = ANSAIPv6Address(0xffffffff, 0xffffffff,0xffffffff,0xffffffff);
        EV << "IPv66 address: " << ipv66.str() << endl;
        EV << "dptr: " << ipv66.dptr << ", .words(): " << ipv66.words() << endl;
        EV << "netmask length: " << ipv66.getNetmaskLength() << endl;
        EV << "ANDed:" << ipv6.doAnd(ipv62).str() << endl;

        EV << "match? :" << ANSAIPv6Address::maskedAddrAreEqual(ipv6, ipv63, ipv62) << endl;
        EV << "match? :" << ANSAIPv6Address::maskedAddrAreEqual(ipv6, ipv62, ipv62) << endl;
*/
    }
}


//void EigrpIpv4Pdm::receiveChangeNotification(int category, const cObject *details)
void EigrpIpv4Pdm::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj DETAILS_ARG)
{
    // ignore notifications during initialization
    /*
    if (simulation.getContextType() == CTX_INITIALIZE)
        return;

    Enter_Method_Silent();
    printNotificationBanner(category, details);
    */
    Enter_Method_Silent("EigrpIPv4Pdm::receiveChangeNotification(%s)", notificationCategoryName(signalID));

    if (signalID == NF_INTERFACE_STATE_CHANGED)
    {
        ANSA_InterfaceEntry *iface = check_and_cast<ANSA_InterfaceEntry*>(details);
        processIfaceStateChange(iface);
    }
    else if (signalID == NF_INTERFACE_CONFIG_CHANGED)
    {
        ANSA_InterfaceEntry *iface = check_and_cast<ANSA_InterfaceEntry*>(details);
        EigrpInterface *eigrpIface = getInterfaceById(iface->getInterfaceId());
        double ifParam;

        if (eigrpIface == NULL)
            return;

        ifParam = iface->getBandwidth();
        if (ifParam != eigrpIface->getBandwidth())
        { // Bandwidth
            eigrpIface->setBandwidth(ifParam);
            if (eigrpIface->isEnabled())
                processIfaceConfigChange(eigrpIface);
        }
        ifParam = iface->getDelay();
        if (ifParam != eigrpIface->getDelay())
        { // Delay
            eigrpIface->setDelay(ifParam);
            if (eigrpIface->isEnabled())
                processIfaceConfigChange(eigrpIface);
        }

    }
    else if (signalID == NF_ROUTE_DELETED)
    { //
        processRTRouteDel(details);
    }
}

void EigrpIpv4Pdm::processIfaceStateChange(ANSA_InterfaceEntry *iface)
{
    EigrpInterface *eigrpIface;
    int ifaceId = iface->getInterfaceId();
    // Get address and mask of interface
    IPv4Address ifMask = iface->ipv4Data()->getNetmask();
    IPv4Address ifAddress = iface->ipv4Data()->getIPAddress().doAnd(ifMask);
    int networkId;

    if (iface->isUp())
    { // an interface goes up
        if (routingForNetworks->isInterfaceIncluded(ifAddress, ifMask, &networkId))
        { // Interface is included in EIGRP
            if ((eigrpIface = getInterfaceById(ifaceId)) == NULL)
            { // Create EIGRP interface
                eigrpIface = new EigrpInterface(iface, networkId, false);
            }
            if (!eigrpIface->isEnabled())
            {
                enableInterface(eigrpIface, ifAddress, ifMask, networkId);
                startHelloTimer(eigrpIface, simTime() + eigrpIface->getHelloInt() - 0.5);
            }
        }
    }
    else if (!iface->isUp() || !iface->hasCarrier())
    { // an interface goes down
        eigrpIface = this->eigrpIft->findInterfaceById(ifaceId);

        if (eigrpIface != NULL && eigrpIface->isEnabled())
        {
            disableInterface(iface, eigrpIface, ifAddress, ifMask);
        }
    }
}

void EigrpIpv4Pdm::processIfaceConfigChange(EigrpInterface *eigrpIface)
{
    EigrpRouteSource<IPv4Address> *source;
    int routeCount = eigrpTt->getNumRoutes();
    int ifaceId = eigrpIface->getInterfaceId();
    EigrpWideMetricPar ifParam = eigrpMetric->getParam(eigrpIface);
    EigrpWideMetricPar newParam;
    uint64_t metric;

    // Update routes through the interface
    for (int i = 0; i < routeCount; i++)
    {
        source = eigrpTt->getRoute(i);
        if (source->getIfaceId() == ifaceId && source->isValid())
        {
            // Update metric of source
            if (source->getNexthopId() == EigrpNeighbor<IPv4Address>::UNSPEC_ID)
            { // connected route
                source->setMetricParams(ifParam);
                metric = eigrpMetric->computeClassicMetric(ifParam, this->kValues);
                source->setMetric(metric);
            }
            else
            {
                newParam = eigrpMetric->adjustParam(ifParam, source->getRdParams());
                source->setMetricParams(newParam);
                metric = eigrpMetric->computeClassicMetric(newParam, this->kValues);
                source->setMetric(metric);
            }

            // Notify DUAL about event
            eigrpDual->processEvent(EigrpDual<IPv4Address>::RECV_UPDATE, source, source->getNexthopId(), false);
        }
    }

    flushMsgRequests();
    eigrpTt->purgeTable();
}

void EigrpIpv4Pdm::processRTRouteDel(const cObject *details)
{
    const IPv4Route *changedRt = check_and_cast<const IPv4Route *>(details);
    //ANSAIPv4Route *changedAnsaRt = dynamic_cast<ANSAIPv4Route *>(changedRt);
    const IPv4Route *changedAnsaRt = changedRt;
    unsigned adminDist;
    EigrpRouteSource<IPv4Address> *source = NULL;

    if (changedAnsaRt != NULL)
        adminDist = changedAnsaRt->getAdminDist();
    else
        adminDist = changedRt->getAdminDist();

#ifdef EIGRP_DEBUG
    //EV << "EIGRP: received notification about deletion of route in RT with AD=" << adminDist << endl;
#endif

    if (adminDist == this->adminDistInt)
    { // Deletion of EIGRP internal route
        source = eigrpTt->findRoute(changedRt->getDestination(), changedRt->getNetmask(), changedRt->getGateway());
        if (source == NULL)
        {
            ASSERT(false);
            EV << "EIGRP: removed EIGRP route from RT, not found corresponding route source in TT" << endl;
            return;
        }

        if (source->isSuccessor())
        {
            if (!source->getRouteInfo()->isActive())
            { // Process route in DUAL (no change of metric)
                this->eigrpDual->processEvent(EigrpDual<IPv4Address>::LOST_ROUTE, source, IEigrpPdm::UNSPEC_SENDER, false);
            }
        }
        // Else do nothing - EIGRP itself removed route from RT
    }
    else
    { // Deletion of non EIGRP route

    }
}

void EigrpIpv4Pdm::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    { // Timer
        this->processTimer(msg);
    }
    else
    { // EIGRP message
        if (dynamic_cast<EigrpMessage *>(msg))
        {
            processMsgFromNetwork(msg);
            delete msg->removeControlInfo();
            delete msg;
            msg = NULL;
        }
        else if (dynamic_cast<EigrpMsgReq *>(msg))
        {
            processMsgFromRtp(msg);
            delete msg;
            msg = NULL;
        }
    }
}

void EigrpIpv4Pdm::processMsgFromNetwork(cMessage *msg)
{
    // Get source IP address and interface ID
    IPv4ControlInfo *ctrlInfo =check_and_cast<IPv4ControlInfo *>(msg->getControlInfo());
    IPv4Address srcAddr = ctrlInfo->getSrcAddr();
    int ifaceId = ctrlInfo->getInterfaceId();
    cMessage *msgDup = NULL;

#ifdef EIGRP_DEBUG
    EigrpInterface *eigrpIface = eigrpIft->findInterfaceById(ifaceId);
    ASSERT(eigrpIface != NULL);
    ASSERT(!eigrpIface->isPassive());
#endif

    // Find neighbor if exists
    EigrpNeighbor<IPv4Address> *neigh;
    if ((neigh = eigrpNt->findNeighbor(srcAddr)) != NULL)
    { // Reset hold timer
        resetTimer(neigh->getHoldTimer(), neigh->getHoldInt());
    }

    // Send message to RTP (message must be duplicated)
    msgDup = msg->dup();
    msgDup->setControlInfo(ctrlInfo->dup());
    send(msgDup, RTP_OUTGW);

    if (dynamic_cast<EigrpIpv4Ack*>(msg) && neigh != NULL)
    {
        processAckMsg(msg, srcAddr, ifaceId, neigh);
    }
    else if (dynamic_cast<EigrpIpv4Hello*>(msg))
    {
        processHelloMsg(msg, srcAddr, ifaceId, neigh);
    }
    else if (dynamic_cast<EigrpIpv4Update*>(msg) && neigh != NULL)
    {
        processUpdateMsg(msg, srcAddr, ifaceId, neigh);
    }
    else if (dynamic_cast<EigrpIpv4Query*>(msg) && neigh != NULL)
    {
        processQueryMsg(msg, srcAddr, ifaceId, neigh);
    }
    else if (dynamic_cast<EigrpIpv4Reply*>(msg) && neigh != NULL)
    {
        processReplyMsg(msg, srcAddr, ifaceId, neigh);
    }
    else if (neigh != NULL)
    {
        EV << "EIGRP: Received message of unknown type, skipped" << endl;
    }
    else
    {
        EV << "EIGRP: Received message from "<< srcAddr <<" that is not neighbor, skipped" << endl;
    }
}

void EigrpIpv4Pdm::processMsgFromRtp(cMessage *msg)
{
    EigrpMsgReq *msgReq = check_and_cast<EigrpMsgReq *>(msg);
    EigrpMessage *eigrpMsg = NULL;
    EigrpIpv4Message *eigrpMsgRt = NULL;
    int routeCnt = msgReq->getRoutesArraySize();
    IPv4Address destAddress;
    int destIface = msgReq->getDestInterface();
    EigrpInterface *eigrpIface = eigrpIft->findInterfaceById(destIface);
    int64_t sizeOfMsg = 20;

#ifdef EIGRP_DEBUG
    ASSERT(eigrpIface != NULL && !eigrpIface->isPassive());
#endif

    if (!getDestIpAddress(msgReq->getDestNeighbor(), &destAddress) || eigrpIface == NULL)
    { // Discard message request
        return;
    }

    // Print message information
    printSentMsg(routeCnt, destAddress, msgReq);

    // Create EIGRP message
    switch (msgReq->getOpcode())
    {
    case EIGRP_HELLO_MSG:
        if (msgReq->getGoodbyeMsg() == true) {       // Goodbye message
            eigrpMsg = createHelloMsg(eigrpIface->getHoldInt(), this->KVALUES_MAX, destAddress, msgReq);
            sizeOfMsg += 12;
        }
        else if (msgReq->getAckNumber() > 0)    // Ack message
        {
            eigrpMsg = createAckMsg(destAddress, msgReq);
        }
        else // Hello message
        {
            eigrpMsg = createHelloMsg(eigrpIface->getHoldInt(), this->kValues, destAddress, msgReq);
            sizeOfMsg += 12;
        }
        break;

    case EIGRP_UPDATE_MSG:
        eigrpMsgRt = createUpdateMsg(destAddress, msgReq);
        // Add route TLV
        if (routeCnt > 0) addRoutesToMsg(eigrpMsgRt, msgReq);

        sizeOfMsg += routeCnt * 44;

        eigrpMsg = eigrpMsgRt;
        break;

    case EIGRP_QUERY_MSG:
        eigrpMsgRt = createQueryMsg(destAddress, msgReq);
        // Add route TLV
        if (routeCnt > 0) addRoutesToMsg(eigrpMsgRt, msgReq);

        sizeOfMsg += routeCnt * 44;

        eigrpMsg = eigrpMsgRt;
        break;

    case EIGRP_REPLY_MSG:
        eigrpMsgRt = createReplyMsg(destAddress, msgReq);
        // Add route TLV
        if (routeCnt > 0) addRoutesToMsg(eigrpMsgRt, msgReq);

        sizeOfMsg += routeCnt * 44;

        eigrpMsg = eigrpMsgRt;
        break;

    default:
        ASSERT(false);
        return;
        break;
    }

    eigrpMsg->setByteLength(sizeOfMsg);
    // Send message to network
    send(eigrpMsg, SPLITTER_OUTGW);
}

bool EigrpIpv4Pdm::getDestIpAddress(int destNeigh, IPv4Address *resultAddress)
{
    EigrpNeighbor<IPv4Address> *neigh = NULL;

    if (destNeigh == EigrpNeighbor<IPv4Address>::UNSPEC_ID)
        resultAddress->set(EIGRP_IPV4_MULT.getInt());
    else
    {
        if ((neigh = eigrpNt->findNeighborById(destNeigh)) == NULL)
            return false;
        resultAddress->set(neigh->getIPAddress().getInt());
    }

    return true;
}

void EigrpIpv4Pdm::printSentMsg(int routeCnt, IPv4Address& destAddress, EigrpMsgReq *msgReq)
{
    int type = msgReq->getOpcode();

    EV << "EIGRP: send " << eigrp::UserMsgs[type];
    if (type == EIGRP_HELLO_MSG)
    {
        if (msgReq->getAckNumber() > 0)
            EV << " (ack) ";
        else if (msgReq->getGoodbyeMsg() == true)
            EV << " (goodbye) ";
    }

    EV << " message to " << destAddress << " on IF " << msgReq->getDestInterface();

    // Print flags
    EV << ", flags: ";
    if (msgReq->getInit()) EV << "init";
    else if (msgReq->getEot()) EV << "eot";
    else if (msgReq->getCr()) EV << "cr";
    else if (msgReq->getRs()) EV << "rs";

    if (type != EIGRP_HELLO_MSG) EV << ", route count: " << routeCnt;
    EV << ", seq num:" << msgReq->getSeqNumber();
    EV << ", ack num:" << msgReq->getAckNumber();
    EV << endl;
}

void EigrpIpv4Pdm::printRecvMsg(EigrpMessage *msg, IPv4Address& addr, int ifaceId)
{
    EV << "EIGRP: received " << eigrp::UserMsgs[msg->getOpcode()];
    if (msg->getOpcode() == EIGRP_HELLO_MSG && msg->getAckNum() > 0)
        EV << " (ack) ";
    EV << " message from " << addr << " on IF " << ifaceId;

    EV << ", flags: ";
    if (msg->getInit()) EV << "init";
    else if (msg->getEot()) EV << "eot";
    else if (msg->getCr()) EV << "cr";
    else if (msg->getRs()) EV << "rs";

    EV << ", seq num:" << msg->getSeqNum();
    EV << ", ack num:" << msg->getAckNum();
    EV << endl;
}

void EigrpIpv4Pdm::processTimer(cMessage *msg)
{
    EigrpTimer *timer = check_and_cast<EigrpTimer*>(msg);
    EigrpInterface *eigrpIface = NULL;
    EigrpNeighbor<IPv4Address> *neigh = NULL;
    cObject *contextBasePtr = NULL;
    EigrpMsgReq *msgReq = NULL;
    int ifaceId = -1;

    switch (timer->getTimerKind())
    {
    case EIGRP_HELLO_TIMER:
        // get interface that belongs to timer
        contextBasePtr = (cObject*)timer->getContextPointer();
        eigrpIface = check_and_cast<EigrpInterface *>(contextBasePtr);

        // schedule Hello timer
        scheduleAt(simTime() + eigrpIface->getHelloInt() - 0.5, timer);

        // send Hello message
        msgReq = createMsgReq(EIGRP_HELLO_MSG, EigrpNeighbor<IPv4Address>::UNSPEC_ID, eigrpIface->getInterfaceId());
        send(msgReq, RTP_OUTGW);
        break;

    case EIGRP_HOLD_TIMER:
        // get neighbor from context
        contextBasePtr = (cObject*)timer->getContextPointer();
        neigh = check_and_cast<EigrpNeighbor<IPv4Address> *>(contextBasePtr);
        ifaceId = neigh->getIfaceId();

        // remove neighbor
        EV << "Neighbor " << neigh->getIPAddress() <<" is down, holding time expired" << endl;
        removeNeighbor(neigh);
        neigh = NULL;
        flushMsgRequests();
        eigrpTt->purgeTable();

        // Send goodbye and restart Hello timer
        eigrpIface = eigrpIft->findInterfaceById(ifaceId);
        resetTimer(eigrpIface->getHelloTimer(), eigrpIface->getHelloInt() - 0.5);
        msgReq = createMsgReq(EIGRP_HELLO_MSG, EigrpNeighbor<IPv4Address>::UNSPEC_ID, ifaceId);
        msgReq->setGoodbyeMsg(true);
        send(msgReq, RTP_OUTGW);
        break;

    default:
        EV << "Timer with unknown kind was skipped" << endl;
        delete timer;
        timer = NULL;
        break;
    }
}

void EigrpIpv4Pdm::processAckMsg(cMessage *msg, IPv4Address& srcAddress, int ifaceId, EigrpNeighbor<IPv4Address> *neigh)
{
    printRecvMsg(check_and_cast<EigrpIpv4Ack*>(msg), srcAddress, ifaceId);
    if (neigh->isStateUp() == false)
    {
        // If neighbor is "pending", then change its state to "up"
        neigh->setStateUp(true);
        // Send all EIGRP paths from routing table to sender
        sendAllEigrpPaths(eigrpIft->findInterfaceById(ifaceId), neigh);
    }

    if (neigh->getRoutesForDeletion())
    { // Remove unreachable routes waiting for Ack from neighbor
        eigrpTt->delayedRemove(neigh->getNeighborId());
        neigh->setRoutesForDeletion(false);
    }
}

void EigrpIpv4Pdm::processHelloMsg(cMessage *msg, IPv4Address& srcAddress, int ifaceId, EigrpNeighbor<IPv4Address> *neigh)
{
    EigrpIpv4Hello *hello = check_and_cast<EigrpIpv4Hello *>(msg);
    EigrpTlvParameter tlvParam = hello->getParameterTlv();

    printRecvMsg(hello, srcAddress, ifaceId);

    if (tlvParam.kValues == KVALUES_MAX)
    { // Received Goodbye message, remove neighbor
        if (neigh != NULL)
        {
            EV << "     interface goodbye received" << endl;
            EV << "EIGRP neighbor " << srcAddress << " is down, interface goodbye received" << endl;
            removeNeighbor(neigh);
            flushMsgRequests();
            eigrpTt->purgeTable();
        }
    }
    else if (neigh == NULL)
    { // New neighbor
        processNewNeighbor(ifaceId, srcAddress, hello);
    }
    else
    { // neighbor exists, its state is "up" or "pending"
        // Check K-values
        if (!(tlvParam.kValues == this->kValues))
        { // Not satisfied
            EV << "EIGRP neighbor " << srcAddress << " is down, " << eigrp::UserMsgs[eigrp::M_NEIGH_BAD_KVALUES] << endl;
            removeNeighbor(neigh);
            flushMsgRequests();
            eigrpTt->purgeTable();

            // send Goodbye message and reset Hello timer
            EigrpInterface *iface = this->eigrpIft->findInterfaceById(ifaceId);
            resetTimer(iface->getHelloTimer(), iface->getHelloInt() - 0.5);
            EigrpMsgReq *msgReq = createMsgReq(EIGRP_HELLO_MSG, EigrpNeighbor<IPv4Address>::UNSPEC_ID, ifaceId);
            msgReq->setGoodbyeMsg(true);
            send(msgReq, RTP_OUTGW);
        }
        else if (tlvParam.holdTimer != neigh->getHoldInt()) // Save Hold interval
            neigh->setHoldInt(tlvParam.holdTimer);
    }
}

void EigrpIpv4Pdm::processUpdateMsg(cMessage *msg, IPv4Address& srcAddress, int ifaceId, EigrpNeighbor<IPv4Address> *neigh)
{
    EigrpIpv4Update *update = check_and_cast<EigrpIpv4Update *>(msg);
    EigrpInterface *eigrpIface = eigrpIft->findInterfaceById(ifaceId);
    EigrpRouteSource<IPv4Address> *src;
    bool skipRoute, notifyDual, isSourceNew;

    printRecvMsg(update, srcAddress, ifaceId);

    if (neigh->isStateUp() == false && update->getAckNum() != 0)
    { // First ack from neighbor
        // If neighbor is "pending", then change its state to "up"
        neigh->setStateUp(true);
        // Send all EIGRP paths from routing table to sender
        sendAllEigrpPaths(eigrpIface, neigh);
    }

    if (update->getInit())
    { // Request to send all paths from routing table
        if (neigh->isStateUp() == true)
        {
            sendAllEigrpPaths(eigrpIface, neigh);
        }
    }
    else if (neigh->isStateUp())
    { // Neighbor is "up", forward message to DUAL
        int cnt = update->getInterRoutesArraySize();
#ifdef EIGRP_DEBUG
        EV << "     Route count:" << cnt << endl;
#endif

        for (int i = 0; i < cnt; i++)
        {
            skipRoute = false;
            notifyDual = false;
            EigrpMpIpv4Internal tlv = update->getInterRoutes(i);

            if (tlv.routerID == eigrpTt->getRouterId() || eigrpMetric->isParamMaximal(tlv.metric))
            { // Route with RID is equal to RID of router or tlv is unreachable route
                IPv4Address nextHop = getNextHopAddr(tlv.nextHop, srcAddress);
                if (eigrpTt->findRoute(tlv.destAddress, tlv.destMask, nextHop) == NULL)
                    skipRoute = true;    // Route is not found in TT -> discard route
            }

            if (skipRoute)
            { // Discard route
                EV << "EIGRP: discard route " << tlv.destAddress << endl;
            }
            else
            { // process route
                src = processInterRoute(tlv, srcAddress, neigh->getNeighborId(), eigrpIface, &notifyDual, &isSourceNew);
                if (notifyDual)
                    eigrpDual->processEvent(EigrpDual<IPv4Address>::RECV_UPDATE, src, neigh->getNeighborId(), isSourceNew);
                else
                    EV << "EIGRP: route " << tlv.destAddress << " is not processed by DUAL, no change of metric" << endl;
            }
        }
        flushMsgRequests();
        eigrpTt->purgeTable();
    }
    // else ignore message
}

void EigrpIpv4Pdm::processQueryMsg(cMessage *msg, IPv4Address& srcAddress, int ifaceId, EigrpNeighbor<IPv4Address> *neigh)
{
    EigrpIpv4Query *query = check_and_cast<EigrpIpv4Query *>(msg);
    EigrpInterface *eigrpIface = eigrpIft->findInterfaceById(ifaceId);
    EigrpRouteSource<IPv4Address> *src = NULL;
    bool notifyDual, isSourceNew;

    printRecvMsg(query, srcAddress, ifaceId);

    int cnt = query->getInterRoutesArraySize();
#ifdef EIGRP_DEBUG
    EV << "     Route count:" << cnt << endl;
#endif

    for (int i = 0; i < cnt; i++)
    {
        src = processInterRoute(query->getInterRoutes(i), srcAddress, neigh->getNeighborId(), eigrpIface, &notifyDual, &isSourceNew);
        // Always notify DUAL
        eigrpDual->processEvent(EigrpDual<IPv4Address>::RECV_QUERY, src, neigh->getNeighborId(), isSourceNew);
    }
    flushMsgRequests();
    eigrpTt->purgeTable();
}

void EigrpIpv4Pdm::processReplyMsg(cMessage *msg, IPv4Address& srcAddress, int ifaceId, EigrpNeighbor<IPv4Address> *neigh)
{
    EigrpIpv4Reply *reply = check_and_cast<EigrpIpv4Reply *>(msg);
    EigrpInterface *eigrpIface = eigrpIft->findInterfaceById(ifaceId);
    EigrpRouteSource<IPv4Address> *src;
    bool notifyDual, isSourceNew;

    printRecvMsg(reply, srcAddress, ifaceId);

    int cnt = reply->getInterRoutesArraySize();
#ifdef EIGRP_DEBUG
    EV << "     Route count:" << cnt << endl;
#endif

    for (int i = 0; i < cnt; i++)
    {
        src = processInterRoute(reply->getInterRoutes(i), srcAddress, neigh->getNeighborId(), eigrpIface, &notifyDual, &isSourceNew);
        // Always notify DUAL
        eigrpDual->processEvent(EigrpDual<IPv4Address>::RECV_REPLY, src, neigh->getNeighborId(), isSourceNew);
    }
    flushMsgRequests();
    eigrpTt->purgeTable();
}

/**
 * @param neigh Neighbor which is next hop for a route in TLV.
 */
EigrpRouteSource<IPv4Address> *EigrpIpv4Pdm::processInterRoute(EigrpMpIpv4Internal& tlv, IPv4Address& srcAddr,
        int sourceNeighId, EigrpInterface *eigrpIface, bool *notifyDual, bool *isSourceNew)
{
    IPv4Address nextHop = getNextHopAddr(tlv.nextHop, srcAddr);
    EigrpNeighbor<IPv4Address> *nextHopNeigh = eigrpNt->findNeighbor(nextHop);
    EigrpRouteSource<IPv4Address> *src;
    EigrpWideMetricPar newParam, oldParam, oldNeighParam;
    EigrpWideMetricPar ifParam = eigrpMetric->getParam(eigrpIface);

    // Find route or create one (route source is identified by ID of the next hop - not by ID of sender)
    src = eigrpTt->findOrCreateRoute(tlv.destAddress, tlv.destMask, tlv.routerID, eigrpIface, nextHopNeigh->getNeighborId(), isSourceNew);

    // Compare old and new neighbor's parameters
    oldNeighParam = src->getRdParams();
    if (*isSourceNew || !eigrpMetric->compareParameters(tlv.metric, oldNeighParam, this->kValues))
    {
        // Compute reported distance (must be there)
        uint64_t metric = eigrpMetric->computeClassicMetric(tlv.metric, this->kValues);
        src->setRdParams(tlv.metric);
        src->setRd(metric);

        // Get new metric parameters
        newParam = eigrpMetric->adjustParam(ifParam, tlv.metric);
        // Get old metric parameters for comparison
        oldParam = src->getMetricParams();

        if (!eigrpMetric->compareParameters(newParam, oldParam, this->kValues))
        {
            // Set source of route
            if (*isSourceNew)
                src->setNextHop(nextHop);
            src->setMetricParams(newParam);
            // Compute metric
            metric = eigrpMetric->computeClassicMetric(newParam, this->kValues);
            src->setMetric(metric);
            *notifyDual = true;
        }
    }

    return src;
}

EigrpTimer *EigrpIpv4Pdm::createTimer(char timerKind, void *context)
{
    EigrpTimer *timer = new EigrpTimer();
    timer->setTimerKind(timerKind);
    timer->setContextPointer(context);

    return timer;
}

EigrpIpv4Hello *EigrpIpv4Pdm::createHelloMsg(int holdInt, EigrpKValues kValues, IPv4Address& destAddress, EigrpMsgReq *msgReq)
{
    EigrpIpv4Hello *msg = new EigrpIpv4Hello("EIGRPHello");
    EigrpTlvParameter paramTlv;

    addMessageHeader(msg, EIGRP_HELLO_MSG, msgReq);
    addCtrInfo(msg, msgReq->getDestInterface(), destAddress);

    // Add parameter type TLV
    paramTlv.holdTimer = holdInt;
    paramTlv.kValues = kValues;
    msg->setParameterTlv(paramTlv);

    // Add stub TLV
    if (this->eigrpStubEnabled)
    {
        EigrpTlvStub stubTlv;
        stubTlv.stub = this->eigrpStub;
        msg->setStubTlv(stubTlv);
    }

    return msg;
}

EigrpIpv4Ack *EigrpIpv4Pdm::createAckMsg(IPv4Address& destAddress, EigrpMsgReq *msgReq)
{
    EigrpIpv4Ack *msg = new EigrpIpv4Ack("EIGRPAck");
    addMessageHeader(msg, EIGRP_HELLO_MSG, msgReq);
    msg->setAckNum(msgReq->getAckNumber());
    addCtrInfo(msg, msgReq->getDestInterface(), destAddress);
    return msg;
}

EigrpIpv4Update *EigrpIpv4Pdm::createUpdateMsg(const IPv4Address& destAddress, EigrpMsgReq *msgReq)
{
    EigrpIpv4Update *msg = new EigrpIpv4Update("EIGRPUpdate");
    addMessageHeader(msg, EIGRP_UPDATE_MSG, msgReq);
    msg->setAckNum(msgReq->getAckNumber());
    msg->setSeqNum(msgReq->getSeqNumber());
    addCtrInfo(msg, msgReq->getDestInterface(), destAddress);

    return msg;
}

EigrpIpv4Query *EigrpIpv4Pdm::createQueryMsg(IPv4Address& destAddress, EigrpMsgReq *msgReq)
{
    EigrpIpv4Query *msg = new EigrpIpv4Query("EIGRPQuery");
    addMessageHeader(msg, EIGRP_QUERY_MSG, msgReq);
    msg->setAckNum(msgReq->getAckNumber());
    msg->setSeqNum(msgReq->getSeqNumber());
    addCtrInfo(msg, msgReq->getDestInterface(), destAddress);
    return msg;
}

EigrpIpv4Reply *EigrpIpv4Pdm::createReplyMsg(IPv4Address& destAddress, EigrpMsgReq *msgReq)
{
    EigrpIpv4Reply *msg = new EigrpIpv4Reply("EIGRPReply");
    addMessageHeader(msg, EIGRP_REPLY_MSG, msgReq);
    msg->setAckNum(msgReq->getAckNumber());
    msg->setSeqNum(msgReq->getSeqNumber());
    addCtrInfo(msg, msgReq->getDestInterface(), destAddress);
    return msg;
}

void EigrpIpv4Pdm::addMessageHeader(EigrpMessage *msg, int opcode, EigrpMsgReq *msgReq)
{
    msg->setOpcode(opcode);
    msg->setAsNum(this->asNum);
    msg->setInit(msgReq->getInit());
    msg->setEot(msgReq->getEot());
    msg->setRs(msgReq->getRs());
    msg->setCr(msgReq->getCr());
}

void EigrpIpv4Pdm::addCtrInfo(EigrpMessage *msg, int ifaceId,
        const IPv4Address &destAddress)
{
    IPv4ControlInfo *ctrl = new IPv4ControlInfo();
    ctrl->setDestAddr(destAddress);
    ctrl->setProtocol(88);
    ctrl->setTimeToLive(1);
    ctrl->setInterfaceId(ifaceId);

    msg->setControlInfo(ctrl);
}

void EigrpIpv4Pdm::createRouteTlv(EigrpMpIpv4Internal *routeTlv, EigrpRoute<IPv4Address> *route, bool unreachable)
{
    EigrpWideMetricPar rtMetric = route->getRdPar();
    routeTlv->destAddress = route->getRouteAddress();
    routeTlv->destMask = route->getRouteMask();
    routeTlv->nextHop = IPv4Address::UNSPECIFIED_ADDRESS;
    setRouteTlvMetric(&routeTlv->metric, &rtMetric);
    if (unreachable)
    {
        routeTlv->metric.delay = EigrpMetricHelper::DELAY_INF;
        routeTlv->metric.bandwidth = EigrpMetricHelper::BANDWIDTH_INF;
    }
}

void EigrpIpv4Pdm::setRouteTlvMetric(EigrpWideMetricPar *msgMetric, EigrpWideMetricPar *rtMetric)
{
    msgMetric->bandwidth = rtMetric->bandwidth;
    msgMetric->delay = rtMetric->delay;
    msgMetric->hopCount = rtMetric->hopCount;
    msgMetric->load = rtMetric->load;
    msgMetric->mtu = rtMetric->mtu;
    msgMetric->offset = 0;      // TODO
    msgMetric->priority = 0;    // TODO
    msgMetric->reliability = rtMetric->reliability;
}

void EigrpIpv4Pdm::addRoutesToMsg(EigrpIpv4Message *msg, const EigrpMsgReq *msgReq)
{
    // Add routes to the message
    int reqCnt = msgReq->getRoutesArraySize();
    EigrpRouteSource<IPv4Address> *source = NULL;
    EigrpRoute<IPv4Address> *route = NULL;

    msg->setInterRoutesArraySize(reqCnt);
    for (int i = 0; i < reqCnt; i++)
    {
        EigrpMpIpv4Internal routeTlv;
        EigrpMsgRoute req = msgReq->getRoutes(i);

        if ((source = eigrpTt->findRouteById(req.sourceId)) == NULL)
        { // Route was removed (only for sent Update messages to stub routers)
            route = eigrpTt->findRouteInfoById(req.routeId);
            ASSERT(route != NULL);
        }
        else
        {
            route = source->getRouteInfo();
        }
        routeTlv.routerID = req.originator;
        createRouteTlv(&routeTlv, route, req.unreachable);

        msg->setInterRoutes(i, routeTlv);

#ifdef EIGRP_DEBUG
        EV << "     route: " << routeTlv.destAddress << "/" << routeTlv.destMask.getNetmaskLength();
        EV << " originator: " << routeTlv.routerID;
        if (eigrpMetric->isParamMaximal(routeTlv.metric)) EV << " (unreachable) ";
        EV << ", bw: " << routeTlv.metric.bandwidth;
        EV << ", dly: " << routeTlv.metric.delay;
        EV << ", hopcnt: " << (int)routeTlv.metric.hopCount;
        EV << endl;
    }
#endif
}

EigrpMsgReq *EigrpIpv4Pdm::createMsgReq(HeaderOpcode msgType, int destNeighbor, int destIface)
{
    EigrpMsgReq *msgReq = new EigrpMsgReq(eigrp::UserMsgs[msgType]);

    msgReq->setOpcode(msgType);
    msgReq->setDestNeighbor(destNeighbor);
    msgReq->setDestInterface(destIface);

    return msgReq;
}

void EigrpIpv4Pdm::sendAllEigrpPaths(EigrpInterface *eigrpIface, EigrpNeighbor<IPv4Address> *neigh)
{
    int routeCnt = eigrpTt->getNumRouteInfo();
    int addedRoutes = 0;    // Number of routes in message
    EigrpRoute<IPv4Address> *route;
    EigrpRouteSource<IPv4Address> *source;
    EigrpMsgReq *msgReq = createMsgReq(EIGRP_UPDATE_MSG, neigh->getNeighborId(), neigh->getIfaceId());

    msgReq->setRoutesArraySize(routeCnt);

    for (int i = 0; i < routeCnt; i++)
    {
        route = eigrpTt->getRouteInfo(i);
        if (route->isActive())
            continue;

        if ((source = eigrpTt->getBestSuccessor(route)) != NULL)
        {
            if (this->eigrpStubEnabled && applyStubToUpdate(source))
                continue;   // Apply stub settings to the route
            if (eigrpIface->isSplitHorizonEn() && applySplitHorizon(eigrpIface, source, route))
                continue;   // Apply Split Horizon rule

            EigrpMsgRoute routeReq;
            routeReq.sourceId = source->getSourceId();
            routeReq.routeId = source->getRouteId();
            routeReq.originator = source->getOriginator();
            routeReq.invalid = false;
            msgReq->setRoutes(addedRoutes /* not variable i */, routeReq);
            addedRoutes++;
        }
    }

    if (addedRoutes < routeCnt) // reduce size of array
        msgReq->setRoutesArraySize(addedRoutes);

    msgReq->setEot(true);

    send(msgReq, RTP_OUTGW);
}

void EigrpIpv4Pdm::processNewNeighbor(int ifaceId, IPv4Address &srcAddress, EigrpIpv4Hello *rcvMsg)
{
    EigrpMsgReq *msgReqUpdate = NULL, *msgReqHello = NULL;
    EigrpNeighbor<IPv4Address> *neigh;
    EigrpInterface *iface = eigrpIft->findInterfaceById(ifaceId);
    EigrpTlvParameter paramTlv = rcvMsg->getParameterTlv();
    EigrpStub stubConf = rcvMsg->getStubTlv().stub;
    int ecode;      // Code of user message

    // Check rules for establishing neighborship
    if ((ecode = checkNeighborshipRules(ifaceId, rcvMsg->getAsNum(), srcAddress,
            paramTlv.kValues)) != eigrp::M_OK)
    {
        EV << "EIGRP can't create neighborship with " << srcAddress << ", " << eigrp::UserMsgs[ecode] << endl;

        if (ecode == eigrp::M_NEIGH_BAD_KVALUES)
        { // Send Goodbye message and reset Hello timer
            resetTimer(iface->getHelloTimer(), iface->getHelloInt() - 0.5);
            msgReqHello = createMsgReq(EIGRP_HELLO_MSG, EigrpNeighbor<IPv4Address>::UNSPEC_ID, ifaceId);
            msgReqHello->setGoodbyeMsg(true);
            send(msgReqHello, RTP_OUTGW);
        }
        return;
    }

    EV << "Neighbor " << srcAddress << " is up, new adjacency" << endl;

    neigh = createNeighbor(iface, srcAddress, paramTlv.holdTimer);

    if (stubConf.connectedRt || stubConf.leakMapRt || stubConf.recvOnlyRt || stubConf.redistributedRt || stubConf.staticRt || stubConf.summaryRt)
    { // Process stub configuration
        neigh->setStubEnable(true);
        neigh->setStubConf(stubConf);
        iface->incNumOfStubs();
        eigrpNt->incStubCount();
    }

    // Reply with Hello message and reset Hello timer
    resetTimer(iface->getHelloTimer(), iface->getHelloInt() - 0.5);
    msgReqHello = createMsgReq(EIGRP_HELLO_MSG, EigrpNeighbor<IPv4Address>::UNSPEC_ID, ifaceId);
    send(msgReqHello, RTP_OUTGW);

    // Send Update with INIT flag
    msgReqUpdate = createMsgReq(EIGRP_UPDATE_MSG, neigh->getNeighborId(), neigh->getIfaceId());
    msgReqUpdate->setInit(true);
    send(msgReqUpdate, RTP_OUTGW);
}

int EigrpIpv4Pdm::checkNeighborshipRules(int ifaceId, int neighAsNum,
        IPv4Address &neighAddr, const EigrpKValues &neighKValues)
{
    IPv4Address ifaceAddr, ifaceMask;

    if (this->eigrpIft->findInterfaceById(ifaceId) == NULL)
    { // EIGRP must be enabled on interface
        return eigrp::M_DISABLED_ON_IF;
    }
    else if (this->asNum != neighAsNum)
    { // AS numbers must be equal
        return eigrp::M_NEIGH_BAD_AS;
    }
    else if (!(this->kValues == neighKValues))
    { // K-values must be same
        return eigrp::M_NEIGH_BAD_KVALUES;
    }
    else
    {
        // get IP address of interface and mask
        ANSA_InterfaceEntry *iface = check_and_cast<ANSA_InterfaceEntry *>(ift->getInterfaceById(ifaceId));
        ifaceAddr.set(iface->ipv4Data()->getIPAddress().getInt());
        ifaceMask.set(iface->ipv4Data()->getNetmask().getInt());

        // check, if the neighbor is on same subnet
        if (!ifaceAddr.maskedAddrAreEqual(ifaceAddr, neighAddr, ifaceMask))
        {
            return eigrp::M_NEIGH_BAD_SUBNET;
        }
    }

    return eigrp::M_OK;
}

EigrpNeighbor<IPv4Address> *EigrpIpv4Pdm::createNeighbor(EigrpInterface *eigrpIface, IPv4Address& address, uint16_t holdInt)
{
    // Create record in the neighbor table
    EigrpNeighbor<IPv4Address> *neigh = new EigrpNeighbor<IPv4Address>(eigrpIface->getInterfaceId(), eigrpIface->getInterfaceName(), address);
    EigrpTimer *holdt = createTimer(EIGRP_HOLD_TIMER, neigh);
    neigh->setHoldTimer(holdt);
    neigh->setHoldInt(holdInt);
    eigrpNt->addNeighbor(neigh);
    // Start Hold timer
    scheduleAt(simTime() + holdInt, holdt);

    eigrpIface->incNumOfNeighbors();

    return neigh;
}

void EigrpIpv4Pdm::removeNeighbor(EigrpNeighbor<IPv4Address> *neigh)
{
    EigrpRouteSource<IPv4Address> *source = NULL;
    EigrpRoute<IPv4Address> *route = NULL;
    // Find interface (enabled/disabled)
    EigrpInterface *eigrpIface = getInterfaceById(neigh->getIfaceId());

    int nextHopId = neigh->getNeighborId();
    int ifaceId = neigh->getIfaceId();
    const char *ifaceName = neigh->getIfaceName();
    int routeId;

    // Remove neighbor from NT
    this->eigrpNt->removeNeighbor(neigh);
    eigrpIface->decNumOfNeighbors();
    if (neigh->isStubEnabled())
    {
        eigrpIface->decNumOfStubs();
        eigrpNt->decStubCount();
    }
    delete neigh;
    neigh = NULL;

    for (int i = 0; i < eigrpTt->getNumRouteInfo(); i++)
    { // Process routes that go via this neighbor
        route = eigrpTt->getRouteInfo(i);
        routeId = route->getRouteId();
        // Note: TT can not contain two or more records with the same network address and next hop address
        source = eigrpTt->findRouteByNextHop(routeId, nextHopId);

#ifdef EIGRP_DEBUG
        EV << "EIGRP: Destination: " << route->getRouteAddress() << "/" << route->getRouteMask().getNetmaskLength() << ", active " << route->isActive() << endl;
#endif

        if (route->isActive())
        {
            if (source == NULL)
            { // Create dummy source for active route (instead of Reply)
#ifdef EIGRP_DEBUG
                EV << "     Create dummy route " << route->getRouteAddress() << " via <unspecified> for deletion of reply status handle" << endl;
#endif
                source = new EigrpRouteSource<IPv4Address>(ifaceId, ifaceName, nextHopId, routeId, route);
                eigrpTt->addRoute(source);
                eigrpDual->processEvent(EigrpDual<IPv4Address>::NEIGHBOR_DOWN, source, nextHopId, true);
            }
            else
                eigrpDual->processEvent(EigrpDual<IPv4Address>::NEIGHBOR_DOWN, source, nextHopId, false);
        }
        else
        { // Notify DUAL about event
            if (source != NULL)
                eigrpDual->processEvent(EigrpDual<IPv4Address>::NEIGHBOR_DOWN, source, nextHopId, false);
        }
    }

    // Do not flush messages in transmit queue
}

IPv4Route *EigrpIpv4Pdm::createRTRoute(EigrpRouteSource<IPv4Address> *successor)
{
    //int ifaceId = successor->getIfaceId();
    EigrpRoute<IPv4Address> *route = successor->getRouteInfo();
    //ANSAIPv4Route *rtEntry = new ANSAIPv4Route();
    IPv4Route* rtEntry = new IPv4Route();
    rtEntry->setDestination(route->getRouteAddress());
    rtEntry->setNetmask(route->getRouteMask());
    rtEntry->setSourceType(IRoute::EIGRP);
    rtEntry->setInterface(ift->getInterfaceById(successor->getIfaceId()));
    rtEntry->setGateway(successor->getNextHop());
    setRTRouteMetric(rtEntry, successor->getMetric());

    if (successor->isInternal())
    {
        // Set any source except IFACENETMASK and MANUAL
        //rtEntry->setSource(IPv4Route::ZEBRA);
        // Set right protocol source
        //rtEntry->setRoutingProtocolSource(ANSAIPv4Route::pEIGRP);
        //rtEntry->setAdminDist(ANSAIPv4Route::dEIGRPInternal);
        rtEntry->setAdminDist(IPv4Route::dEIGRPInternal);
    }
    else {
        rtEntry->setAdminDist(IPv4Route::dEIGRPExternal);
    }

    return rtEntry;
}

void EigrpIpv4Pdm::msgToAllIfaces(int destination, HeaderOpcode msgType, EigrpRouteSource<IPv4Address> *source, bool forcePoisonRev, bool forceUnreachable)
{
    EigrpInterface *eigrpIface;
    int ifCount = eigrpIft->getNumInterfaces();
    int numOfNeigh;

    for (int i = 0; i < ifCount; i++)
    {
        eigrpIface = eigrpIft->getInterface(i);

        // Send message only to interface with stub neighbor
        if (destination == IEigrpPdm::STUB_RECEIVER && eigrpIface->getNumOfStubs() == 0)
            continue;

        if ((numOfNeigh = eigrpIface->getNumOfNeighbors()) > 0)
        {
            if (msgType == EIGRP_QUERY_MSG)
            {
                if (!applyStubToQuery(eigrpIface, numOfNeigh))
                    msgToIface(msgType, source, eigrpIface, forcePoisonRev);
                // Else do not send Query to stub router (skip the route on interface)
            }
            else
                msgToIface(msgType, source, eigrpIface, forcePoisonRev, forceUnreachable);
        }
    }
}

bool EigrpIpv4Pdm::applyStubToQuery(EigrpInterface *eigrpIface, int numOfNeigh)
{
    if (this->eigrpStubEnabled)
        return false;   // Send Query to all neighbors
    if (numOfNeigh == eigrpIface->getNumOfStubs())
        return true;    // Do not send Query to stub router
    return false;
}

void EigrpIpv4Pdm::msgToIface(HeaderOpcode msgType, EigrpRouteSource<IPv4Address> *source, EigrpInterface *eigrpIface, bool forcePoisonRev, bool forceUnreachable)
{
    EigrpNeighbor<IPv4Address> *neigh = NULL;
    EigrpMsgRoute msgRt;
    int ifaceId, destNeigh = 0;

    if (eigrpIface->isSplitHorizonEn() && applySplitHorizon(eigrpIface, source, source->getRouteInfo()))
    {
        if (forcePoisonRev)
        { // Apply Poison Reverse instead of Split Horizon
            msgRt.unreachable = true;
        }
        else // Apply Split Horizon rule - do not send route to the interface
            return;
    }

    msgRt.sourceId = source->getSourceId();
    msgRt.routeId = source->getRouteId();
    msgRt.originator = source->getOriginator();
    msgRt.invalid = false;

    // Get destination interface ID and destination neighbor ID
    ifaceId = eigrpIface->getInterfaceId();
    if (ift->getInterfaceById(ifaceId)->isMulticast())
    { // Multicast
        destNeigh = IEigrpPdm::UNSPEC_RECEIVER;
    }
    else
    { // Unicast
        if (neigh == NULL)
            neigh = eigrpNt->getFirstNeighborOnIf(ifaceId);
        destNeigh = neigh->getNeighborId();
    }

    pushMsgRouteToQueue(msgType, ifaceId, destNeigh, msgRt);
}

EigrpMsgReq *EigrpIpv4Pdm::pushMsgRouteToQueue(HeaderOpcode msgType, int ifaceId, int neighId, const EigrpMsgRoute& msgRt)
{
    EigrpMsgReq *request = NULL;
    RequestVector::iterator it;

    // Find or create message
    for (it = reqQueue.begin(); it != reqQueue.end(); it++)
    {
        if ((*it)->getDestInterface() == ifaceId && (*it)->getOpcode() == msgType)
        {
            request = *it;
            break;
        }
    }
    if (request == NULL)
    { // Create new request
        request = createMsgReq(msgType, neighId, ifaceId);
        request->setRoutesArraySize(1);
        request->setRoutes(0, msgRt);
        reqQueue.push_back(request);
    }
    else
    { // Use existing request
        int rtSize = request->getRoutesArraySize();
        int index;
        if ((index = request->findMsgRoute(msgRt.routeId)) >= 0)
        { // Use existing route
            if (msgRt.unreachable)
                request->getRoutes(index).unreachable = true;
        }
        else
        { // Add route to request
            request->setRoutesArraySize(rtSize + 1);
            request->setRoutes(rtSize, msgRt);
        }
    }

    return request;
}

bool EigrpIpv4Pdm::applySplitHorizon(EigrpInterface *destInterface, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route)
{
    if (route->getNumSucc() <= 1)   // Only 1 successor, check its interface ID (source is always successor)
        return destInterface->getInterfaceId() == source->getIfaceId();
    else    // There is more than 1 successor. Is any of them on the interface?
        return eigrpTt->getBestSuccessorByIf(route, destInterface->getInterfaceId()) != NULL;
}

bool EigrpIpv4Pdm::applyStubToUpdate(EigrpRouteSource<IPv4Address> *src)
{
    if (this->eigrpStub.recvOnlyRt)
        return true;    // Do not send any route

    // Send only specified type of route
    else if (this->eigrpStub.connectedRt && src->getNexthopId() == EigrpNeighbor<IPv4Address>::UNSPEC_ID)
        return false;
    else if (this->eigrpStub.redistributedRt && src->isRedistributed())
        return false;
    else if (this->eigrpStub.summaryRt && src->isSummary())
        return false;
    else if (this->eigrpStub.staticRt && src->isRedistributed())
        return false;

    // TODO: leakMapRt

    return true;
}

void EigrpIpv4Pdm::flushMsgRequests()
{
    RequestVector::iterator it;
    IPv4Address destAddress;

    // Send Query
    for (it = reqQueue.begin(); it != reqQueue.end(); it++)
    {
        if ((*it)->getOpcode() == EIGRP_QUERY_MSG)
        {
            // Check if interface exists
            if (eigrpIft->findInterfaceById((*it)->getDestInterface()) == NULL)
                continue;
            else
                send(*it, RTP_OUTGW);
        }
    }

    // Send other messages
    for (it = reqQueue.begin(); it != reqQueue.end(); it++)
    {
        // Check if interface exists
        if (eigrpIft->findInterfaceById((*it)->getDestInterface()) == NULL)
        {
            delete *it; // Discard request
            continue;
        }

        if ((*it)->getOpcode() != EIGRP_QUERY_MSG)
            send(*it, RTP_OUTGW);
    }

    reqQueue.clear();
}

/*bool EigrpIpv4Pdm::getRoutesFromRequests(RequestVector *msgReqs)
{
    bool emptyQueue = true;
    EigrpMsgRequest *request = NULL;
    EigrpMsgRequest *reqToSend = NULL;
    RequestVector::iterator it;

    for (it = transmitionQueue.begin(); it != transmitionQueue.end(); it++)
    {
        if ((*it)->invalid)
            continue;

        request = *it;
        if (reqToSend == NULL)
        {
            emptyQueue = false;
            reqToSend = request;
        }

        if (reqToSend != NULL)
        {
            if (reqToSend->type == request->type && reqToSend->destInterface == request->destInterface &&
                    reqToSend->destNeighbor == request->destNeighbor)
            {
                request->invalid = true;
                msgReqs->push_back(request);
            }
        }
    }

    return emptyQueue;
}*/

EigrpInterface *EigrpIpv4Pdm::getInterfaceById(int ifaceId)
{
    EigrpInterface *iface;

    if ((iface = eigrpIft->findInterfaceById(ifaceId)) != NULL)
        return iface;
    else
        return eigrpIftDisabled->findInterface(ifaceId);
}

void EigrpIpv4Pdm::disableInterface(ANSA_InterfaceEntry *iface, EigrpInterface *eigrpIface, IPv4Address& ifAddress, IPv4Address& ifMask)
{
    EigrpTimer* hellot;
    EigrpNeighbor<IPv4Address> *neigh;
    EigrpRouteSource<IPv4Address> *source;
    int neighCount;
    int ifaceId = eigrpIface->getInterfaceId();

    EV << "EIGRP disabled on interface " << eigrpIface->getName() << "(" << ifaceId << ")" << endl;

    iface->ipv4Data()->leaveMulticastGroup(EIGRP_IPV4_MULT);

    // stop hello timer
    if ((hellot = eigrpIface->getHelloTimer()) != NULL)
        cancelEvent(hellot);

    // First process route on the interface
    IPv4Address ifNetwork = ifAddress.doAnd(ifMask);
    source = eigrpTt->findRoute(ifNetwork, ifMask, EigrpNeighbor<IPv4Address>::UNSPEC_ID);
    ASSERT(source != NULL);
    // Notify DUAL about event
    eigrpDual->processEvent(EigrpDual<IPv4Address>::INTERFACE_DOWN, source, EigrpNeighbor<IPv4Address>::UNSPEC_ID, false);

    // Remove interface from EIGRP interface table (must be there)
    if (eigrpIface->isEnabled())
    {
        eigrpIft->removeInterface(eigrpIface);
        eigrpIftDisabled->addInterface(eigrpIface);
        eigrpIface->setEnabling(false);
    }

    // Delete all neighbors on the interface
    neighCount = eigrpNt->getNumNeighbors();
    for (int i = 0; i < neighCount; i++)
    {
        neigh = eigrpNt->getNeighbor(i);
        if (neigh->getIfaceId() == ifaceId)
        {
            removeNeighbor(neigh);
        }
    }

    flushMsgRequests();
    eigrpTt->purgeTable();
}

void EigrpIpv4Pdm::enableInterface(EigrpInterface *eigrpIface, IPv4Address& ifAddress, IPv4Address& ifMask, int networkId)
{
    int ifaceId = eigrpIface->getInterfaceId();
    EigrpRouteSource<IPv4Address> *src;
    EigrpWideMetricPar metricPar;
    bool isSourceNew;

    EV << "EIGRP enabled on interface " << eigrpIface->getName() << "(" << ifaceId << ")" << endl;

    // Move interface to EIGRP interface table
    if (!eigrpIface->isEnabled())
    {
        eigrpIftDisabled->removeInterface(eigrpIface);
        eigrpIft->addInterface(eigrpIface);
        eigrpIface->setEnabling(true);
    }

    // Register multicast address on interface
    IPv4InterfaceData *ifaceIpv4 = ift->getInterfaceById(ifaceId)->ipv4Data();
    ifaceIpv4->joinMulticastGroup(EIGRP_IPV4_MULT);

    eigrpIface->setNetworkId(networkId);

    // Create route
    src = eigrpTt->findOrCreateRoute(ifAddress, ifMask, eigrpTt->getRouterId(), eigrpIface, EigrpNeighbor<IPv4Address>::UNSPEC_ID, &isSourceNew);
    ASSERT(isSourceNew == true);
    // Compute metric
    metricPar = eigrpMetric->getParam(eigrpIface);
    src->setMetricParams(metricPar);
    uint64_t metric = eigrpMetric->computeClassicMetric(metricPar, this->kValues);
    src->setMetric(metric);

    // Notify DUAL about event
    eigrpDual->processEvent(EigrpDual<IPv4Address>::INTERFACE_UP, src, EigrpNeighbor<IPv4Address>::UNSPEC_ID, isSourceNew);
    flushMsgRequests();
    eigrpTt->purgeTable();
}

EigrpInterface *EigrpIpv4Pdm::addInterfaceToEigrp(int ifaceId, int networkId, bool enabled)
{
    ANSA_InterfaceEntry *iface = check_and_cast<ANSA_InterfaceEntry*>(ift->getInterfaceById(ifaceId));
    // create EIGRP interface
    EigrpInterface *eigrpIface = new EigrpInterface(iface, networkId, false);
    IPv4Address ifAddress, ifMask;

    if (enabled)
    {
        // Get address and mask of interface
        ifMask = iface->ipv4Data()->getNetmask();
        ifAddress = iface->ipv4Data()->getIPAddress().doAnd(ifMask);

        enableInterface(eigrpIface, ifAddress, ifMask, networkId);
        startHelloTimer(eigrpIface, simTime() + uniform(0,1));
    }
    else
    {
        eigrpIftDisabled->addInterface(eigrpIface);
    }

    return eigrpIface;
}

void EigrpIpv4Pdm::startHelloTimer(EigrpInterface *eigrpIface, simtime_t interval)
{
    EigrpTimer *hellot;

    // Start Hello timer on interface
    if (!eigrpIface->isPassive())
    {
        if ((hellot = eigrpIface->getHelloTimer()) == NULL)
        {
            hellot = createTimer(EIGRP_HELLO_TIMER, eigrpIface);
            eigrpIface->setHelloTimerPtr(hellot);
        }
        scheduleAt(interval, hellot);
    }
}

//
// interface IEigrpModule
//

EigrpNetwork<IPv4Address> *EigrpIpv4Pdm::addNetwork(IPv4Address address, IPv4Address mask)
{
    return routingForNetworks->addNetwork(address, mask);
}

void EigrpIpv4Pdm::setHelloInt(int interval, int ifaceId)
{
    EigrpInterface *iface = getInterfaceById(ifaceId);
    if (iface == NULL)
        iface = addInterfaceToEigrp(ifaceId, EigrpNetworkTable<IPv4Address>::UNSPEC_NETID, false);
    iface->setHelloInt(interval);
}

void EigrpIpv4Pdm::setHoldInt(int interval, int ifaceId)
{
    EigrpInterface *iface = getInterfaceById(ifaceId);
    if (iface == NULL)
        iface = addInterfaceToEigrp(ifaceId, EigrpNetworkTable<IPv4Address>::UNSPEC_NETID, false);
    iface->setHoldInt(interval);
}

void EigrpIpv4Pdm::setSplitHorizon(bool shenabled, int ifaceId)
{
    EigrpInterface *iface = getInterfaceById(ifaceId);
    if (iface == NULL)
        iface = addInterfaceToEigrp(ifaceId, EigrpNetworkTable<IPv4Address>::UNSPEC_NETID, false);
    iface->setSplitHorizon(shenabled);
}

void EigrpIpv4Pdm::setPassive(bool passive, int ifaceId)
{
    EigrpInterface *eigrpIface = getInterfaceById(ifaceId);

    if (eigrpIface == NULL)
        eigrpIface = addInterfaceToEigrp(ifaceId, EigrpNetworkTable<IPv4Address>::UNSPEC_NETID, false);
    else if (eigrpIface->isEnabled())
    { // Disable sending and receiving of messages
        ANSA_InterfaceEntry *iface = check_and_cast<ANSA_InterfaceEntry*>(ift->getInterfaceById(ifaceId));
        iface->ipv4Data()->leaveMulticastGroup(EIGRP_IPV4_MULT);

        // Stop and delete hello timer
        EigrpTimer *hellot = eigrpIface->getHelloTimer();
        if (hellot != NULL)
        {
            cancelEvent(hellot);
            delete hellot;
            eigrpIface->setHelloTimerPtr(NULL);
        }
    }
    // Else do nothing (interface is not part of EIGRP)

    eigrpIface->setPassive(passive);
}

//
// interface IEigrpPdm
//

void EigrpIpv4Pdm::sendUpdate(int destNeighbor, EigrpRoute<IPv4Address> *route, EigrpRouteSource<IPv4Address> *source, bool forcePoisonRev, const char *reason)
{
    EV << "DUAL: send Update message about " << route->getRouteAddress() << " to all neighbors, " << reason << endl;
    if (this->eigrpStubEnabled && applyStubToUpdate(source))
    {
        EV << "     Stub routing applied, message will not be sent" << endl;
        return;
    }
    msgToAllIfaces(destNeighbor, EIGRP_UPDATE_MSG, source, forcePoisonRev, false);
}

void EigrpIpv4Pdm::sendQuery(int destNeighbor, EigrpRoute<IPv4Address> *route, EigrpRouteSource<IPv4Address> *source, bool forcePoisonRev)
{
    bool forceUnreachable = false;

    EV << "DUAL: send Query message about " << route->getRouteAddress() << " to all neighbors" << endl;

    if (this->eigrpStubEnabled)
        forceUnreachable = true;    // Send Query with infinite metric to all neighbors
    else
    {
        if (this->eigrpNt->getStubCount() > 0)
        { // Apply Poison Reverse instead of Split Horizon rule
            forcePoisonRev = true;
        }
    }

    msgToAllIfaces(destNeighbor, EIGRP_QUERY_MSG, source, forcePoisonRev, forceUnreachable);
}

void EigrpIpv4Pdm::sendReply(EigrpRoute<IPv4Address> *route, int destNeighbor, EigrpRouteSource<IPv4Address> *source, bool forcePoisonRev, bool isUnreachable)
{
    EigrpMsgRoute msgRt;
    EigrpNeighbor<IPv4Address> *neigh = eigrpNt->findNeighborById(destNeighbor);

    EV << "DUAL: send Reply message about " << route->getRouteAddress() << endl;
    if (neigh == NULL)
        return;

    msgRt.invalid = false;
    msgRt.sourceId = source->getSourceId();
    msgRt.routeId = source->getRouteId();
    msgRt.originator = source->getOriginator();
    msgRt.unreachable = isUnreachable;

    if (this->eigrpStubEnabled)
        msgRt.unreachable = true;   // Stub router always reply with infinite metric

    // Apply Poison Reverse (instead of Split Horizon)
    if (!isUnreachable && eigrpIft->findInterfaceById(neigh->getIfaceId())->isSplitHorizonEn())
    { // Poison Reverse is enabled when Split Horizon is enabled
        // Note: destNeighbor is also neighbor that sent Query
        if (forcePoisonRev)
            msgRt.unreachable = true;
    }

    pushMsgRouteToQueue(EIGRP_REPLY_MSG, neigh->getIfaceId(), neigh->getNeighborId(), msgRt);
}

/**
 * @return if route is found in routing table then returns true.
 */
bool EigrpIpv4Pdm::removeRouteFromRT(EigrpRouteSource<IPv4Address> *source, IRoute::SourceType *removedRtSrc)
{
    EigrpRoute<IPv4Address> *route = source->getRouteInfo();
    IPv4Route *rtEntry = findRoute(route->getRouteAddress(), route->getRouteMask(), source->getNextHop());

    //ANSAIPv4Route *ansaRtEntry = dynamic_cast<ANSAIPv4Route *>(rtEntry);
    IPv4Route *ansaRtEntry = rtEntry;
    if (ansaRtEntry != NULL)
    {
        *removedRtSrc = ansaRtEntry->getSourceType();
        //if (*removedRtSrc == ANSAIPv4Route::pEIGRP)
        if (ansaRtEntry->getSourceType() == IRoute::EIGRP)
        {
            EV << "EIGRP: delete route " << route->getRouteAddress() << " via " << source->getNextHop() << " from RT" << endl;
            rt->removeRoute(rtEntry);
        }
    }
    else
    {
#ifdef EIGRP_DEBUG
        EV << "EIGRP: EIGRP route "<< route->getRouteAddress() << " via " << source->getNextHop() << " can not be removed, not found in RT" << endl;
#endif
    }
    return rtEntry != nullptr;
}

/*void EigrpIpv4Pdm::removeSourceFromTT(EigrpRouteSource<IPv4Address> *source)
{
    EV << "EIGRP remove route " << source->getRouteInfo()->getRouteAddress();
    EV << " via " << source->getNextHop() << " from TT" << endl;

    if (eigrpTt->removeRoute(source) != NULL)
    {
        // Remove also route if there is no source (only if the route is not active)
        if (source->getRouteInfo()->getRefCnt() == 1 && !source->getRouteInfo()->isActive())
            eigrpTt->removeRouteInfo(source->getRouteInfo());
        delete source;
    }
}*/

bool EigrpIpv4Pdm::isRTSafeForAdd(EigrpRoute<IPv4Address> *route, unsigned int eigrpAd)
{
    IPv4Route *routeInTable = findRoute(route->getRouteAddress(), route->getRouteMask());
    IPv4Route *ansaRoute = nullptr;

    if (routeInTable == nullptr)
        return true; // Route not found

    ansaRoute = routeInTable;
    if (ansaRoute != nullptr)
    { // AnsaIPv4Route use own AD attribute
        if (ansaRoute->getAdminDist() < eigrpAd)
            return false;
        return true;
    }
    if (ansaRoute == NULL && routeInTable->getAdminDist() == IPv4Route::dUnknown)
        return false;   // Connected route has AD = 255 (dUnknown) in IPv4Route
    if (routeInTable != NULL && routeInTable->getAdminDist() < eigrpAd)
        return false;   // Other IPv4Route with right AD
    return true;
}

EigrpRouteSource<IPv4Address> *EigrpIpv4Pdm::updateRoute(EigrpRoute<IPv4Address> *route, uint64_t dmin, bool *rtableChanged, bool removeUnreach)
{
    EigrpRouteSource<IPv4Address> *source = NULL, *bestSuccessor = NULL;
    IPv4Route *rtEntry = NULL;
    int routeNum = eigrpTt->getNumRoutes();
    int routeId = route->getRouteId();
    int pathsInRT = 0;      // Number of paths in RT (equal to number of successors)
    int sourceCounter = 0;  // Number of route sources
    uint64_t routeFd = route->getFd();

    EV << "EIGRP: Search successor for route " << route->getRouteAddress() << ", FD is " << route->getFd() << endl;

    for (int i = 0; i < routeNum; i++)
    {
        source = eigrpTt->getRoute(i);
        if (source->getRouteId() != routeId || !source->isValid())
            continue;

        sourceCounter++;

        if (source->getRd() < routeFd /* FC, use this FD (not dmin) */ &&
                source->getMetric() <= dmin*this->variance && pathsInRT < this->maximumPath) /* Load Balancing */
        {
            EV << "     successor " << source->getNextHop() << " (" << source->getMetric() << "/" << source->getRd() << ")" << endl;

            if ((rtEntry = findRoute(route->getRouteAddress(), route->getRouteMask(), source->getNextHop())) == NULL)
                if (!isRTSafeForAdd(route, adminDistInt))
                { // In RT exists route with smaller AD, do not mark the route source as successor
                    source->setSuccessor(false);
                    EV << "           route can not be added into RT, there is route with smaller AD" << endl;
                    continue;   // Skip the route
                }

            if (installRouteToRT(route, source, dmin, rtEntry))
                *rtableChanged = true;

            pathsInRT++;
            source->setSuccessor(true);
        }
        else if (source->isSuccessor())
        { // Remove old successor from RT
            if (removeOldSuccessor(source, route))
                *rtableChanged = true;
        }

        if (removeUnreach && source->isUnreachable() && source->getDelayedRemove() == 0 && source->isValid())
        { // Invalidate unreachable routes in TT
            source->setValid(false);
            EV << "     invalidate route via " << source->getNextHop() << " in TT" << endl;
        }
    }

    route->setNumSucc(pathsInRT);

    if ((bestSuccessor = eigrpTt->getBestSuccessor(route)) != NULL)
    { // Update route with best Successor
        if (dmin < routeFd) // Set only if there is a successor
            route->setFd(dmin);
        route->setDij(dmin);
        route->setRdPar(bestSuccessor->getMetricParams());
        route->setSuccessor(bestSuccessor);

        if (sourceCounter == 1)
        { // Set FD of route to Successor metric (according to Cisco EIGRP implementation)
            route->setFd(bestSuccessor->getMetric());
        }
    }
    else
        route->setSuccessor(NULL);

    return bestSuccessor;
}

bool EigrpIpv4Pdm::removeOldSuccessor(EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route)
{
    //ANSAIPv4Route::RoutingProtocolSource srcProto = ANSAIPv4Route::pUnknown;
    IRoute::SourceType srcProto = IRoute::UNKNOWN;
    bool rtFound, rtableChanged = false;

    // To distinguish the route deleted by EIGRP in ReceiveChangeNotification method
    source->setSuccessor(false);

    rtFound = removeRouteFromRT(source, &srcProto);
#ifdef EIGRP_DEBUG
    //EV << "EIGRP: removing old successor: rt found: " << rtFound << " src: " << srcProto << endl;
#endif
    //if (!rtFound || (rtFound && (srcProto == ANSAIPv6Route::pEIGRP || srcProto == ANSAIPv6Route::pEIGRPext)))
    if (!rtFound || (rtFound && (srcProto == IRoute::EIGRP)))
    {
        if (route->getSuccessor() == source)
            route->setSuccessor(NULL);
        rtableChanged = true;
    }
    else // Route from other sources can not be removed, do not change Successor's record in TT
        source->setSuccessor(true);

    return rtableChanged;
}

bool EigrpIpv4Pdm::installRouteToRT(EigrpRoute<IPv4Address> *route, EigrpRouteSource<IPv4Address> *source, uint64_t dmin, IPv4Route *rtEntry)
{
    IPv4Route *ansaRtEntry = rtEntry;
    bool rtableChanged = false;

    //if (rtEntry != NULL && (ansaRtEntry = dynamic_cast<ANSAIPv6Route *>(rtEntry)) != NULL)
    if (ansaRtEntry != nullptr)
    {
        //if (ansaRtEntry->getRoutingProtocolSource() != ANSAIPv4Route::pEIGRP)
        if (ansaRtEntry->getSourceType() != IRoute::EIGRP)
            return rtableChanged;   // Do not add route to RT
        else if ((unsigned int)ansaRtEntry->getMetric() != source->getMetric())
        { // Update EIGRP route in RT
            EV << "EIGRP: Update EIGRP route " << route->getRouteAddress() << " via " << source->getNextHop() << " in RT" << endl;
            setRTRouteMetric(ansaRtEntry, source->getMetric());
        }
    }
    else
    { // Insert new route to RT
        EV << "EIGRP: add EIGRP route " << route->getRouteAddress() << " via " << source->getNextHop() << " to RT" << endl;
        ansaRtEntry = createRTRoute(source);
        // rt->prepareForAddRoute(ansaRtEntry);    // Do not check safety (already checked)
        rt->addRoute(ansaRtEntry);

        rtableChanged = true;
    }

    return rtableChanged;
}

bool EigrpIpv4Pdm::setReplyStatusTable(EigrpRoute<IPv4Address> *route, EigrpRouteSource<IPv4Address> *source, bool forcePoisonRev, int *neighCount, int *stubCount)
{
    int neighTotalCount = eigrpNt->getNumNeighbors();
    EigrpNeighbor<IPv4Address> *neigh;
    EigrpInterface *eigrpIface = NULL;

    for (int i = 0; i < neighTotalCount; i++)
    {
        neigh = eigrpNt->getNeighbor(i);
        if ((eigrpIface = eigrpIft->findInterfaceById(neigh->getIfaceId())) == NULL)
            continue;   // The interface has been removed

        // Apply stub routing
        if (!this->eigrpStubEnabled)
        {
            if (neigh->isStubEnabled())
            {
                (*stubCount)++;
                (*neighCount)++;
                continue;   // Non stub router can not send Query to stub router
            }

            if (this->eigrpNt->getStubCount() > 0)
            { // Apply Poison Reverse instead of Split Horizon rule
                forcePoisonRev = true;
            }
        }

        // Apply Split Horizon and Poison Reverse
        if (eigrpIface->isSplitHorizonEn() && applySplitHorizon(eigrpIface, source, route))
        {
            if (forcePoisonRev)
            { // Apply Poison Reverse instead of Split Horizon
                route->setReplyStatus(neigh->getNeighborId());
            }
            else
                continue;   // Do not send route
        } else
            route->setReplyStatus(neigh->getNeighborId());


        (*neighCount)++;
    }

    return route->getReplyStatusSum() > 0;
}

bool EigrpIpv4Pdm::hasNeighborForUpdate(EigrpRouteSource<IPv4Address> *source)
{
    int ifaceCnt = this->eigrpIft->getNumInterfaces();
    EigrpInterface *eigrpIface;

    for (int i = 0; i < ifaceCnt; i++)
    {
        eigrpIface = eigrpIft->getInterface(i);

        // Do not apply Split Horizon rule
        if (eigrpIface != NULL && eigrpIface->getNumOfNeighbors() > 0)
        {
            return true;
        }
    }

    return false;
}

void EigrpIpv4Pdm::setDelayedRemove(int neighId, EigrpRouteSource<IPv4Address> *src)
{
    EigrpNeighbor<IPv4Address> *neigh = eigrpNt->findNeighborById(neighId);


    ASSERT(neigh != NULL);
    neigh->setRoutesForDeletion(true);
    src->setDelayedRemove(neighId);
    src->setValid(true);    // Can not be invalid

#ifdef EIGRP_DEBUG
        EV << "DUAL: route via " << src->getNextHop() << " will be removed from TT after receiving Ack from neighbor" << endl;
#endif
}

IPv4Route* EigrpIpv4Pdm::findRoute(const IPv4Address& network,
        const IPv4Address& netmask) {
    IPv4Route *route = nullptr;
    for (int i = 0; i < rt->getNumRoutes(); i++) {
        auto it = rt->getRoute(i);
        if (it->getDestination() == network && it->getNetmask() == netmask) // match
        {
            route = it;
            break;
        }
    }

    return route;
}

IPv4Route* EigrpIpv4Pdm::findRoute(const IPv4Address& network,
        const IPv4Address& netmask, const IPv4Address& nexthop) {
    IPv4Route *route = nullptr;
    for (int i = 0; i < rt->getNumRoutes(); i++) {
        auto it = rt->getRoute(i);
        if (it->getDestination() == network && it->getNetmask() == netmask && it->getGateway() == nexthop) {
            route = it;
            break;
        }
    }
    return route;
}

void EigrpIpv4Pdm::sendUpdateToStubs(EigrpRouteSource<IPv4Address> *succ ,EigrpRouteSource<IPv4Address> *oldSucc, EigrpRoute<IPv4Address> *route)
{
    if (!this->eigrpStubEnabled && eigrpNt->getStubCount() > 0)
    {
        if (succ == NULL)
        { // Send old successor
            // Route will be removed after router receives Ack from neighbor
            if (oldSucc->isUnreachable())
            {
                route->setUpdateSent(true);
                route->setNumSentMsgs(route->getNumSentMsgs() + 1);
            }
            sendUpdate(IEigrpPdm::STUB_RECEIVER, route, oldSucc, true, "notify stubs about change");
        }
        else // Send successor
            sendUpdate(IEigrpPdm::STUB_RECEIVER, route, succ, true, "notify stubs about change");
    }
}


}
