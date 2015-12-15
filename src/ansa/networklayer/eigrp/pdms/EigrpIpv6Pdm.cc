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
* @file EigrpIpv6Pdm.cc
* @author Vit Rek, xrekvi00@stud.fit.vutbr.cz
* @date 6. 11. 2014
* @brief EIGRP IPv6 Protocol Dependent Module
* @detail Main module, it mediates control exchange between DUAL, routing table and
topology table.
*/

#include <algorithm>

#include "IPv6ControlInfo.h"
#include "EigrpDeviceConfigurator.h"
#include "EigrpPrint.h"
#include "EigrpIpv6Pdm.h"
#include "IPv6Address.h"

#include "ANSARoutingTable6.h"
#include "ANSARoutingTable6Access.h"




#define EIGRP_DEBUG

Define_Module(EigrpIpv6Pdm);


EigrpIpv6Pdm::EigrpIpv6Pdm() : EIGRP_IPV6_MULT(IPv6Address("FF02::A"))
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

EigrpIpv6Pdm::~EigrpIpv6Pdm()
{
    delete this->routingForNetworks;
    delete this->eigrpIftDisabled;
    delete this->eigrpDual;
    delete this->eigrpMetric;
}

void EigrpIpv6Pdm::initialize(int stage)
{
    // in stage 0 interfaces are registrated
    // in stage 2 interfaces and routing table
    // in stage 3
    if (stage == 3)
    {
        this->eigrpIft = EigrpIfTable6Access().get();
        this->eigrpNt = Eigrpv6NeighTableAccess().get();
        this->eigrpTt = Eigrpv6TopolTableAccess().get();
        this->rt = ANSARoutingTable6Access().get();
        //this->eigrpDual = ModuleAccess<EigrpDual>("eigrpDual").get();

        this->ift = InterfaceTableAccess().get();
        this->nb = NotificationBoardAccess().get();

        this->eigrpIftDisabled = new EigrpDisabledInterfaces();
        this->routingForNetworks = new EigrpNetworkTable<IPv6Address>();
        this->eigrpDual = new EigrpDual<IPv6Address>(this);
        this->eigrpMetric = new EigrpMetricHelper();

        // Load configuration of EIGRP
        EigrpDeviceConfigurator *conf = ModuleAccess<EigrpDeviceConfigurator>("eigrpDeviceConfigurator").get();
        conf->loadEigrpIPv6Config(this);

        WATCH_PTRVECTOR(*routingForNetworks->getAllNetworks());
        WATCH(asNum);
        WATCH(kValues);
        WATCH(maximumPath);
        WATCH(variance);
        WATCH(eigrpStub);

        // Subscribe for changes in the device
        nb->subscribe(this, NF_INTERFACE_STATE_CHANGED);
        nb->subscribe(this, NF_INTERFACE_CONFIG_CHANGED);
        nb->subscribe(this, NF_IPv6_ROUTE_DELETED);

    }
}


void EigrpIpv6Pdm::receiveChangeNotification(int category, const cObject *details)
{
    // ignore notifications during initialization
    if (simulation.getContextType() == CTX_INITIALIZE)
        return;

    Enter_Method_Silent();
    printNotificationBanner(category, details);

    if (category == NF_INTERFACE_STATE_CHANGED)
    {
        InterfaceEntry *iface = check_and_cast<InterfaceEntry*>(details);
        processIfaceStateChange(iface);
    }
    else if (category == NF_INTERFACE_CONFIG_CHANGED)
    {
        InterfaceEntry *iface = check_and_cast<InterfaceEntry*>(details);
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
    else if (category == NF_IPv6_ROUTE_DELETED)
    { //
        processRTRouteDel(details);
    }
}

void EigrpIpv6Pdm::processIfaceStateChange(InterfaceEntry *iface)
{
    EigrpInterface *eigrpIface;
    int ifaceId = iface->getInterfaceId();

    if (iface->isUp())
    { // an interface goes up

        //add directly-connected routes to RT
        PrefixVector::iterator it;

        for (it = netPrefixes.begin(); it != netPrefixes.end(); ++it)
        {// through all known prefixes search prefixes belong to this interface
            if(it->ifaceId == ifaceId)
            {// belonging to same interface -> add
                if(rt->findRoute(it->network, it->prefixLength, IPv6Address::UNSPECIFIED_ADDRESS) == NULL)
                {// route is not in RT -> add
                    rt->addStaticRoute(it->network, it->prefixLength, ifaceId, IPv6Address::UNSPECIFIED_ADDRESS, 0);
                }
            }
        }

        if((eigrpIface = getInterfaceById(ifaceId)) != NULL)
        {// interface is included in EIGRP process
            if (!eigrpIface->isEnabled())
            {// interface disabled -> enable
                enableInterface(eigrpIface);
                startHelloTimer(eigrpIface, simTime() + eigrpIface->getHelloInt() - 0.5);
            }
        }
    }
    else if (iface->isDown() || !iface->hasCarrier())
    { // an interface goes down

        //delete all directly-connected routes from RT
        IPv6Route *route = NULL;
        for(int i = 0; i < rt->getNumRoutes(); ++i)
        {
            route = rt->getRoute(i);

            if(route->getInterfaceId() == ifaceId && route->getNextHop() == IPv6Address::UNSPECIFIED_ADDRESS && route->getDestPrefix() != IPv6Address::LINKLOCAL_PREFIX)
            {// Found Directly-connected (no link-local) route on interface -> remove
                rt->removeRoute(route);
            }

        }


        eigrpIface = this->eigrpIft->findInterfaceById(ifaceId);
        if (eigrpIface != NULL && eigrpIface->isEnabled())
        {
            disableInterface(iface, eigrpIface);
        }
    }
}

void EigrpIpv6Pdm::processIfaceConfigChange(EigrpInterface *eigrpIface)
{
    EigrpRouteSource<IPv6Address> *source;
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
            if (source->getNexthopId() == EigrpNeighbor<IPv6Address>::UNSPEC_ID)
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
            eigrpDual->processEvent(EigrpDual<IPv6Address>::RECV_UPDATE, source, source->getNexthopId(), false);
        }
    }

    flushMsgRequests();
    eigrpTt->purgeTable();
}

void EigrpIpv6Pdm::processRTRouteDel(const cObject *details)
{
    IPv6Route *changedRt = check_and_cast<IPv6Route *>(details);
    ANSAIPv6Route *changedAnsaRt = dynamic_cast<ANSAIPv6Route *>(changedRt);
    unsigned adminDist;
    EigrpRouteSource<IPv6Address> *source = NULL;

    if (changedAnsaRt != NULL)
        adminDist = changedAnsaRt->getAdminDist();
    else
        adminDist = changedRt->getAdminDist();

#ifdef EIGRP_DEBUG
    //ev << "EIGRP: received notification about deletion of route in RT with AD=" << adminDist << endl;
#endif

    if (adminDist == this->adminDistInt)
    { // Deletion of EIGRP internal route
        source = eigrpTt->findRoute(changedRt->getDestPrefix(), makeNetmask(changedRt->getPrefixLength()), changedRt->getNextHop());
        if (source == NULL)
        {
            ASSERT(false);
            ev << "EIGRP: removed EIGRP route from RT, not found corresponding route source in TT" << endl;
            return;
        }

        if (source->isSuccessor())
        {
            if (!source->getRouteInfo()->isActive())
            { // Process route in DUAL (no change of metric)
                this->eigrpDual->processEvent(EigrpDual<IPv6Address>::LOST_ROUTE, source, IEigrpPdm::UNSPEC_SENDER, false);
            }
        }
        // Else do nothing - EIGRP itself removed route from RT
    }
    else
    { // Deletion of non EIGRP route

    }
}

void EigrpIpv6Pdm::handleMessage(cMessage *msg)
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

void EigrpIpv6Pdm::processMsgFromNetwork(cMessage *msg)
{
    // Get source IP address and interface ID
    IPv6ControlInfo *ctrlInfo =check_and_cast<IPv6ControlInfo *>(msg->getControlInfo());
    IPv6Address srcAddr = ctrlInfo->getSrcAddr();
    int ifaceId = ctrlInfo->getInterfaceId();
    cMessage *msgDup = NULL;

    InterfaceEntry::State status = ift->getInterfaceById(ifaceId)->getState();
    if(status == InterfaceEntry::DOWN || status == InterfaceEntry::GOING_UP )
    {// message received on DOWN or GOING_UP iface -> ignore
        EV << "Received message on DOWN interface - message ignored" << endl;
        return;
    }

#ifdef EIGRP_DEBUG
    EigrpInterface *eigrpIface = eigrpIft->findInterfaceById(ifaceId);
    ASSERT(eigrpIface != NULL);
    ASSERT(!eigrpIface->isPassive());
#endif

    // Find neighbor if exists
    EigrpNeighbor<IPv6Address> *neigh;
    if ((neigh = eigrpNt->findNeighbor(srcAddr)) != NULL)
    { // Reset hold timer
        resetTimer(neigh->getHoldTimer(), neigh->getHoldInt());
    }

    // Send message to RTP (message must be duplicated)
    msgDup = msg->dup();
    msgDup->setControlInfo(ctrlInfo->dup());
    send(msgDup, RTP_OUTGW);

    if (dynamic_cast<EigrpIpv6Ack*>(msg) && neigh != NULL)
    {
        processAckMsg(msg, srcAddr, ifaceId, neigh);
    }
    else if (dynamic_cast<EigrpIpv6Hello*>(msg))
    {
        processHelloMsg(msg, srcAddr, ifaceId, neigh);
    }
    else if (dynamic_cast<EigrpIpv6Update*>(msg) && neigh != NULL)
    {
        processUpdateMsg(msg, srcAddr, ifaceId, neigh);
    }
    else if (dynamic_cast<EigrpIpv6Query*>(msg) && neigh != NULL)
    {
        processQueryMsg(msg, srcAddr, ifaceId, neigh);
    }
    else if (dynamic_cast<EigrpIpv6Reply*>(msg) && neigh != NULL)
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

void EigrpIpv6Pdm::processMsgFromRtp(cMessage *msg)
{
    EigrpMsgReq *msgReq = check_and_cast<EigrpMsgReq *>(msg);
    EigrpMessage *eigrpMsg = NULL;
    EigrpIpv6Message *eigrpMsgRt = NULL;
    int routeCnt = msgReq->getRoutesArraySize();
    IPv6Address destAddress;
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

        sizeOfMsg += routeCnt * 68; //magic size

        eigrpMsg = eigrpMsgRt;
        break;

    case EIGRP_QUERY_MSG:
        eigrpMsgRt = createQueryMsg(destAddress, msgReq);
        // Add route TLV
        if (routeCnt > 0) addRoutesToMsg(eigrpMsgRt, msgReq);

        sizeOfMsg += routeCnt * 68; //magic size;

        eigrpMsg = eigrpMsgRt;
        break;

    case EIGRP_REPLY_MSG:
        eigrpMsgRt = createReplyMsg(destAddress, msgReq);
        // Add route TLV
        if (routeCnt > 0) addRoutesToMsg(eigrpMsgRt, msgReq);

        sizeOfMsg += routeCnt * 68; //magic size;

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


bool EigrpIpv6Pdm::getDestIpAddress(int destNeigh, IPv6Address *resultAddress)
{
    EigrpNeighbor<IPv6Address> *neigh = NULL;
    const uint32 *addr = NULL;

    if (destNeigh == EigrpNeighbor<IPv6Address>::UNSPEC_ID)
    {// destination neighbor unset -> use multicast
        addr = EIGRP_IPV6_MULT.words();
        resultAddress->set(addr[0],addr[1],addr[2],addr[3]);
    }
    else
    {// destination neighbor set -> use unicast
        if ((neigh = eigrpNt->findNeighborById(destNeigh)) == NULL)
            return false;
        addr = neigh->getIPAddress().words();
        resultAddress->set(addr[0],addr[1],addr[2],addr[3]);
    }

    return true;
}

void EigrpIpv6Pdm::printSentMsg(int routeCnt, IPv6Address& destAddress, EigrpMsgReq *msgReq)
{
    int type = msgReq->getOpcode();

    ev << "EIGRP: send " << eigrp::UserMsgs[type];
    if (type == EIGRP_HELLO_MSG)
    {
        if (msgReq->getAckNumber() > 0)
            ev << " (ack) ";
        else if (msgReq->getGoodbyeMsg() == true)
            ev << " (goodbye) ";
    }

    ev << " message to " << destAddress << " on IF " << msgReq->getDestInterface();

    // Print flags
    ev << ", flags: ";
    if (msgReq->getInit()) ev << "init";
    else if (msgReq->getEot()) ev << "eot";
    else if (msgReq->getCr()) ev << "cr";
    else if (msgReq->getRs()) ev << "rs";

    if (type != EIGRP_HELLO_MSG) ev << ", route count: " << routeCnt;
    ev << ", seq num:" << msgReq->getSeqNumber();
    ev << ", ack num:" << msgReq->getAckNumber();
    ev << endl;
}

void EigrpIpv6Pdm::printRecvMsg(EigrpMessage *msg, IPv6Address& addr, int ifaceId)
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

void EigrpIpv6Pdm::processTimer(cMessage *msg)
{
    EigrpTimer *timer = check_and_cast<EigrpTimer*>(msg);
    EigrpInterface *eigrpIface = NULL;
    EigrpNeighbor<IPv6Address> *neigh = NULL;
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
        msgReq = createMsgReq(EIGRP_HELLO_MSG, EigrpNeighbor<IPv6Address>::UNSPEC_ID, eigrpIface->getInterfaceId());
        send(msgReq, RTP_OUTGW);
        break;

    case EIGRP_HOLD_TIMER:
        // get neighbor from context
        contextBasePtr = (cObject*)timer->getContextPointer();
        neigh = check_and_cast<EigrpNeighbor<IPv6Address> *>(contextBasePtr);
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
        msgReq = createMsgReq(EIGRP_HELLO_MSG, EigrpNeighbor<IPv6Address>::UNSPEC_ID, ifaceId);
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

void EigrpIpv6Pdm::processAckMsg(cMessage *msg, IPv6Address& srcAddress, int ifaceId, EigrpNeighbor<IPv6Address> *neigh)
{
    printRecvMsg(check_and_cast<EigrpIpv6Ack*>(msg), srcAddress, ifaceId);
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

void EigrpIpv6Pdm::processHelloMsg(cMessage *msg, IPv6Address& srcAddress, int ifaceId, EigrpNeighbor<IPv6Address> *neigh)
{
    EigrpIpv6Hello *hello = check_and_cast<EigrpIpv6Hello *>(msg);
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
            EigrpMsgReq *msgReq = createMsgReq(EIGRP_HELLO_MSG, EigrpNeighbor<IPv6Address>::UNSPEC_ID, ifaceId);
            msgReq->setGoodbyeMsg(true);
            send(msgReq, RTP_OUTGW);
        }
        else if (tlvParam.holdTimer != neigh->getHoldInt()) // Save Hold interval
            neigh->setHoldInt(tlvParam.holdTimer);
    }
}

void EigrpIpv6Pdm::processUpdateMsg(cMessage *msg, IPv6Address& srcAddress, int ifaceId, EigrpNeighbor<IPv6Address> *neigh)
{
    EigrpIpv6Update *update = check_and_cast<EigrpIpv6Update *>(msg);
    EigrpInterface *eigrpIface = eigrpIft->findInterfaceById(ifaceId);
    EigrpRouteSource<IPv6Address> *src;
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
        ev << "     Route count:" << cnt << endl;
#endif

        for (int i = 0; i < cnt; i++)
        {
            skipRoute = false;
            notifyDual = false;
            EigrpMpIpv6Internal tlv = update->getInterRoutes(i);

            if (tlv.routerID == eigrpTt->getRouterId() || eigrpMetric->isParamMaximal(tlv.metric))
            { // Route with RID is equal to RID of router or tlv is unreachable route
                IPv6Address nextHop = getNextHopAddr(tlv.nextHop, srcAddress);
                if (eigrpTt->findRoute(tlv.destAddress, tlv.destMask, nextHop) == NULL)
                    skipRoute = true;    // Route is not found in TT -> discard route
            }

            if (skipRoute)
            { // Discard route
                ev << "EIGRP: discard route " << tlv.destAddress << endl;
            }
            else
            { // process route
                src = processInterRoute(tlv, srcAddress, neigh->getNeighborId(), eigrpIface, &notifyDual, &isSourceNew);
                if (notifyDual)
                    eigrpDual->processEvent(EigrpDual<IPv6Address>::RECV_UPDATE, src, neigh->getNeighborId(), isSourceNew);
                else
                    ev << "EIGRP: route " << tlv.destAddress << " is not processed by DUAL, no change of metric" << endl;
            }
        }
        flushMsgRequests();
        eigrpTt->purgeTable();
    }
    // else ignore message
}

void EigrpIpv6Pdm::processQueryMsg(cMessage *msg, IPv6Address& srcAddress, int ifaceId, EigrpNeighbor<IPv6Address> *neigh)
{
    EigrpIpv6Query *query = check_and_cast<EigrpIpv6Query *>(msg);
    EigrpInterface *eigrpIface = eigrpIft->findInterfaceById(ifaceId);
    EigrpRouteSource<IPv6Address> *src = NULL;
    bool notifyDual, isSourceNew;

    printRecvMsg(query, srcAddress, ifaceId);

    int cnt = query->getInterRoutesArraySize();
#ifdef EIGRP_DEBUG
    ev << "     Route count:" << cnt << endl;
#endif

    for (int i = 0; i < cnt; i++)
    {
        src = processInterRoute(query->getInterRoutes(i), srcAddress, neigh->getNeighborId(), eigrpIface, &notifyDual, &isSourceNew);
        // Always notify DUAL
        eigrpDual->processEvent(EigrpDual<IPv6Address>::RECV_QUERY, src, neigh->getNeighborId(), isSourceNew);
    }
    flushMsgRequests();
    eigrpTt->purgeTable();
}

void EigrpIpv6Pdm::processReplyMsg(cMessage *msg, IPv6Address& srcAddress, int ifaceId, EigrpNeighbor<IPv6Address> *neigh)
{
    EigrpIpv6Reply *reply = check_and_cast<EigrpIpv6Reply *>(msg);
    EigrpInterface *eigrpIface = eigrpIft->findInterfaceById(ifaceId);
    EigrpRouteSource<IPv6Address> *src;
    bool notifyDual, isSourceNew;

    printRecvMsg(reply, srcAddress, ifaceId);

    int cnt = reply->getInterRoutesArraySize();
#ifdef EIGRP_DEBUG
    ev << "     Route count:" << cnt << endl;
#endif

    for (int i = 0; i < cnt; i++)
    {
        src = processInterRoute(reply->getInterRoutes(i), srcAddress, neigh->getNeighborId(), eigrpIface, &notifyDual, &isSourceNew);
        // Always notify DUAL
        eigrpDual->processEvent(EigrpDual<IPv6Address>::RECV_REPLY, src, neigh->getNeighborId(), isSourceNew);
    }
    flushMsgRequests();
    eigrpTt->purgeTable();
}

/**
 * @param neigh Neighbor which is next hop for a route in TLV.
 */
EigrpRouteSource<IPv6Address> *EigrpIpv6Pdm::processInterRoute(EigrpMpIpv6Internal& tlv, IPv6Address& srcAddr,
        int sourceNeighId, EigrpInterface *eigrpIface, bool *notifyDual, bool *isSourceNew)
{
    IPv6Address nextHop = getNextHopAddr(tlv.nextHop, srcAddr);
    EigrpNeighbor<IPv6Address> *nextHopNeigh = eigrpNt->findNeighbor(nextHop);
    EigrpRouteSource<IPv6Address> *src;
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

EigrpTimer *EigrpIpv6Pdm::createTimer(char timerKind, void *context)
{
    EigrpTimer *timer = new EigrpTimer();
    timer->setTimerKind(timerKind);
    timer->setContextPointer(context);

    return timer;
}

EigrpIpv6Hello *EigrpIpv6Pdm::createHelloMsg(int holdInt, EigrpKValues kValues, IPv6Address& destAddress, EigrpMsgReq *msgReq)
{
    EigrpIpv6Hello *msg = new EigrpIpv6Hello("EIGRPHello");
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

EigrpIpv6Ack *EigrpIpv6Pdm::createAckMsg(IPv6Address& destAddress, EigrpMsgReq *msgReq)
{
    EigrpIpv6Ack *msg = new EigrpIpv6Ack("EIGRPAck");
    addMessageHeader(msg, EIGRP_HELLO_MSG, msgReq);
    msg->setAckNum(msgReq->getAckNumber());
    addCtrInfo(msg, msgReq->getDestInterface(), destAddress);
    return msg;
}

EigrpIpv6Update *EigrpIpv6Pdm::createUpdateMsg(const IPv6Address& destAddress, EigrpMsgReq *msgReq)
{
    EigrpIpv6Update *msg = new EigrpIpv6Update("EIGRPUpdate");
    addMessageHeader(msg, EIGRP_UPDATE_MSG, msgReq);
    msg->setAckNum(msgReq->getAckNumber());
    msg->setSeqNum(msgReq->getSeqNumber());
    addCtrInfo(msg, msgReq->getDestInterface(), destAddress);

    return msg;
}

EigrpIpv6Query *EigrpIpv6Pdm::createQueryMsg(IPv6Address& destAddress, EigrpMsgReq *msgReq)
{
    EigrpIpv6Query *msg = new EigrpIpv6Query("EIGRPQuery");
    addMessageHeader(msg, EIGRP_QUERY_MSG, msgReq);
    msg->setAckNum(msgReq->getAckNumber());
    msg->setSeqNum(msgReq->getSeqNumber());
    addCtrInfo(msg, msgReq->getDestInterface(), destAddress);
    return msg;
}

EigrpIpv6Reply *EigrpIpv6Pdm::createReplyMsg(IPv6Address& destAddress, EigrpMsgReq *msgReq)
{
    EigrpIpv6Reply *msg = new EigrpIpv6Reply("EIGRPReply");
    addMessageHeader(msg, EIGRP_REPLY_MSG, msgReq);
    msg->setAckNum(msgReq->getAckNumber());
    msg->setSeqNum(msgReq->getSeqNumber());
    addCtrInfo(msg, msgReq->getDestInterface(), destAddress);
    return msg;
}

void EigrpIpv6Pdm::addMessageHeader(EigrpMessage *msg, int opcode, EigrpMsgReq *msgReq)
{
    msg->setOpcode(opcode);
    msg->setAsNum(this->asNum);
    msg->setInit(msgReq->getInit());
    msg->setEot(msgReq->getEot());
    msg->setRs(msgReq->getRs());
    msg->setCr(msgReq->getCr());
}

void EigrpIpv6Pdm::addCtrInfo(EigrpMessage *msg, int ifaceId,
        const IPv6Address &destAddress)
{
    IPv6ControlInfo *ctrl = new IPv6ControlInfo();
    ctrl->setSrcAddr(ift->getInterfaceById(ifaceId)->ipv6Data()->getLinkLocalAddress());   //use link-local address as source address
    ctrl->setDestAddr(destAddress);
    ctrl->setProtocol(88);
    ctrl->setHopLimit(1);
    ctrl->setInterfaceId(ifaceId);

    msg->setControlInfo(ctrl);
}

void EigrpIpv6Pdm::createRouteTlv(EigrpMpIpv6Internal *routeTlv, EigrpRoute<IPv6Address> *route, bool unreachable)
{
    EigrpWideMetricPar rtMetric = route->getRdPar();
    routeTlv->destAddress = route->getRouteAddress();
    routeTlv->destMask = route->getRouteMask();
    routeTlv->nextHop = IPv6Address::UNSPECIFIED_ADDRESS;
    setRouteTlvMetric(&routeTlv->metric, &rtMetric);
    if (unreachable)
    {
        routeTlv->metric.delay = EigrpMetricHelper::DELAY_INF;
        routeTlv->metric.bandwidth = EigrpMetricHelper::BANDWIDTH_INF;
    }
}

void EigrpIpv6Pdm::setRouteTlvMetric(EigrpWideMetricPar *msgMetric, EigrpWideMetricPar *rtMetric)
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

void EigrpIpv6Pdm::addRoutesToMsg(EigrpIpv6Message *msg, const EigrpMsgReq *msgReq)
{
    // Add routes to the message
    int reqCnt = msgReq->getRoutesArraySize();
    EigrpRouteSource<IPv6Address> *source = NULL;
    EigrpRoute<IPv6Address> *route = NULL;

    msg->setInterRoutesArraySize(reqCnt);
    for (int i = 0; i < reqCnt; i++)
    {
        EigrpMpIpv6Internal routeTlv;
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
        ev << "     route: " << routeTlv.destAddress << "/" << getNetmaskLength(routeTlv.destMask);
        ev << " originator: " << routeTlv.routerID;
        if (eigrpMetric->isParamMaximal(routeTlv.metric)) ev << " (unreachable) ";
        ev << ", bw: " << routeTlv.metric.bandwidth;
        ev << ", dly: " << routeTlv.metric.delay;
        ev << ", hopcnt: " << (int)routeTlv.metric.hopCount;
        ev << endl;
    }
#endif
}

EigrpMsgReq *EigrpIpv6Pdm::createMsgReq(HeaderOpcode msgType, int destNeighbor, int destIface)
{
    EigrpMsgReq *msgReq = new EigrpMsgReq(eigrp::UserMsgs[msgType]);

    msgReq->setOpcode(msgType);
    msgReq->setDestNeighbor(destNeighbor);
    msgReq->setDestInterface(destIface);

    return msgReq;
}

void EigrpIpv6Pdm::sendAllEigrpPaths(EigrpInterface *eigrpIface, EigrpNeighbor<IPv6Address> *neigh)
{
    int routeCnt = eigrpTt->getNumRouteInfo();
    int addedRoutes = 0;    // Number of routes in message
    EigrpRoute<IPv6Address> *route;
    EigrpRouteSource<IPv6Address> *source;
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

void EigrpIpv6Pdm::processNewNeighbor(int ifaceId, IPv6Address &srcAddress, EigrpIpv6Hello *rcvMsg)
{
    EigrpMsgReq *msgReqUpdate = NULL, *msgReqHello = NULL;
    EigrpNeighbor<IPv6Address> *neigh;
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
            msgReqHello = createMsgReq(EIGRP_HELLO_MSG, EigrpNeighbor<IPv6Address>::UNSPEC_ID, ifaceId);
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
    msgReqHello = createMsgReq(EIGRP_HELLO_MSG, EigrpNeighbor<IPv6Address>::UNSPEC_ID, ifaceId);
    send(msgReqHello, RTP_OUTGW);

    // Send Update with INIT flag
    msgReqUpdate = createMsgReq(EIGRP_UPDATE_MSG, neigh->getNeighborId(), neigh->getIfaceId());
    msgReqUpdate->setInit(true);
    send(msgReqUpdate, RTP_OUTGW);
}

int EigrpIpv6Pdm::checkNeighborshipRules(int ifaceId, int neighAsNum,
        IPv6Address &neighAddr, const EigrpKValues &neighKValues)
{
    IPv6Address ifaceAddr, ifaceMask;

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
        // check, if the neighbor uses as source address Link-local address
        if (neighAddr.getScope() != IPv6Address::LINK)
        {//source address is not Link-local address -> bad
            return eigrp::M_NEIGH_BAD_SUBNET;
        }
    }

    return eigrp::M_OK;
}

EigrpNeighbor<IPv6Address> *EigrpIpv6Pdm::createNeighbor(EigrpInterface *eigrpIface, IPv6Address& address, uint16_t holdInt)
{
    // Create record in the neighbor table
    EigrpNeighbor<IPv6Address> *neigh = new EigrpNeighbor<IPv6Address>(eigrpIface->getInterfaceId(), eigrpIface->getInterfaceName(), address);
    EigrpTimer *holdt = createTimer(EIGRP_HOLD_TIMER, neigh);
    neigh->setHoldTimer(holdt);
    neigh->setHoldInt(holdInt);
    eigrpNt->addNeighbor(neigh);
    // Start Hold timer
    scheduleAt(simTime() + holdInt, holdt);

    eigrpIface->incNumOfNeighbors();

    return neigh;
}

void EigrpIpv6Pdm::removeNeighbor(EigrpNeighbor<IPv6Address> *neigh)
{
    EigrpRouteSource<IPv6Address> *source = NULL;
    EigrpRoute<IPv6Address> *route = NULL;
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
        ev << "EIGRP: Destination: " << route->getRouteAddress() << "/" << getNetmaskLength(route->getRouteMask()) << ", active " << route->isActive() << endl;
#endif

        if (route->isActive())
        {
            if (source == NULL)
            { // Create dummy source for active route (instead of Reply)
#ifdef EIGRP_DEBUG
                ev << "     Create dummy route " << route->getRouteAddress() << " via <unspecified> for deletion of reply status handle" << endl;
#endif
                source = new EigrpRouteSource<IPv6Address>(ifaceId, ifaceName, nextHopId, routeId, route);
                eigrpTt->addRoute(source);
                eigrpDual->processEvent(EigrpDual<IPv6Address>::NEIGHBOR_DOWN, source, nextHopId, true);
            }
            else
                eigrpDual->processEvent(EigrpDual<IPv6Address>::NEIGHBOR_DOWN, source, nextHopId, false);
        }
        else
        { // Notify DUAL about event
            if (source != NULL)
                eigrpDual->processEvent(EigrpDual<IPv6Address>::NEIGHBOR_DOWN, source, nextHopId, false);
        }
    }

    // Do not flush messages in transmit queue
}

ANSAIPv6Route *EigrpIpv6Pdm::createRTRoute(EigrpRouteSource<IPv6Address> *successor)
{
    EigrpRoute<IPv6Address> *route = successor->getRouteInfo();
    ANSAIPv6Route *rtEntry = new ANSAIPv6Route(route->getRouteAddress(), getNetmaskLength(route->getRouteMask()), IPv6Route::ROUTING_PROT);
    rtEntry->setInterfaceId(successor->getIfaceId());
    rtEntry->setNextHop(successor->getNextHop());
    setRTRouteMetric(rtEntry, successor->getMetric());

    // Set protocol source and AD
    if (successor->isInternal())
    {
        rtEntry->setRoutingProtocolSource(ANSAIPv6Route::pEIGRP);
        rtEntry->setAdminDist(ANSAIPv6Route::dEIGRPInternal);
    }
    else
    {
        rtEntry->setRoutingProtocolSource(ANSAIPv6Route::pEIGRPext);
        rtEntry->setAdminDist(ANSAIPv6Route::dEIGRPExternal);
    }
    return rtEntry;
}

void EigrpIpv6Pdm::msgToAllIfaces(int destination, HeaderOpcode msgType, EigrpRouteSource<IPv6Address> *source, bool forcePoisonRev, bool forceUnreachable)
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

bool EigrpIpv6Pdm::applyStubToQuery(EigrpInterface *eigrpIface, int numOfNeigh)
{
    if (this->eigrpStubEnabled)
        return false;   // Send Query to all neighbors
    if (numOfNeigh == eigrpIface->getNumOfStubs())
        return true;    // Do not send Query to stub router
    return false;
}

void EigrpIpv6Pdm::msgToIface(HeaderOpcode msgType, EigrpRouteSource<IPv6Address> *source, EigrpInterface *eigrpIface, bool forcePoisonRev, bool forceUnreachable)
{
    EigrpNeighbor<IPv6Address> *neigh = NULL;
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

EigrpMsgReq *EigrpIpv6Pdm::pushMsgRouteToQueue(HeaderOpcode msgType, int ifaceId, int neighId, const EigrpMsgRoute& msgRt)
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

bool EigrpIpv6Pdm::applySplitHorizon(EigrpInterface *destInterface, EigrpRouteSource<IPv6Address> *source, EigrpRoute<IPv6Address> *route)
{
    if (route->getNumSucc() <= 1)   // Only 1 successor, check its interface ID (source is always successor)
        return destInterface->getInterfaceId() == source->getIfaceId();
    else    // There is more than 1 successor. Is any of them on the interface?
        return eigrpTt->getBestSuccessorByIf(route, destInterface->getInterfaceId()) != NULL;
}

bool EigrpIpv6Pdm::applyStubToUpdate(EigrpRouteSource<IPv6Address> *src)
{
    if (this->eigrpStub.recvOnlyRt)
        return true;    // Do not send any route

    // Send only specified type of route
    else if (this->eigrpStub.connectedRt && src->getNexthopId() == EigrpNeighbor<IPv6Address>::UNSPEC_ID)
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

void EigrpIpv6Pdm::flushMsgRequests()
{
    RequestVector::iterator it;
    IPv6Address destAddress;

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

EigrpInterface *EigrpIpv6Pdm::getInterfaceById(int ifaceId)
{
    EigrpInterface *iface;

    if ((iface = eigrpIft->findInterfaceById(ifaceId)) != NULL)
        return iface;
    else
        return eigrpIftDisabled->findInterface(ifaceId);
}

void EigrpIpv6Pdm::disableInterface(InterfaceEntry *iface, EigrpInterface *eigrpIface)
{
    EigrpTimer* hellot;
    EigrpNeighbor<IPv6Address> *neigh;
    EigrpRouteSource<IPv6Address> *source;
    int neighCount;
    int ifaceId = eigrpIface->getInterfaceId();

    EV << "EIGRP disabled on interface " << eigrpIface->getName() << "(" << ifaceId << ")" << endl;

    // Unregister multicast address
    iface->ipv6Data()->leaveMulticastGroup(EIGRP_IPV6_MULT);
    iface->ipv6Data()->removeAddress(EIGRP_IPV6_MULT);

    // stop hello timer
    if ((hellot = eigrpIface->getHelloTimer()) != NULL)
        cancelEvent(hellot);

    std::set<int>::iterator it;
    EigrpNetwork<IPv6Address> *eigrpnet = NULL;

    for (it = eigrpIface->getNetworksIdsBegin(); it != eigrpIface->getNetworksIdsEnd(); ++it)
    {
        eigrpnet = routingForNetworks->findNetworkById(*it);
        source = eigrpTt->findRoute(eigrpnet->getAddress(), eigrpnet->getMask(), EigrpNeighbor<IPv6Address>::UNSPEC_ID);
        ASSERT(source != NULL);
        // Notify DUAL about event
        eigrpDual->processEvent(EigrpDual<IPv6Address>::INTERFACE_DOWN, source, EigrpNeighbor<IPv6Address>::UNSPEC_ID, false);
    }

    eigrpIface->clearNetworkIds();

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

EigrpInterface *EigrpIpv6Pdm::addInterfaceToEigrp(int ifaceId, bool enabled)
{
    InterfaceEntry *iface = ift->getInterfaceById(ifaceId);
    // create EIGRP interface
    EigrpInterface *eigrpIface = NULL;

    eigrpIface = getInterfaceById(ifaceId); //search for existing iface

    if(eigrpIface == NULL)
    {// iface not found -> create new
        eigrpIface = new EigrpInterface(iface, EigrpNetworkTable<IPv6Address>::UNSPEC_NETID, false);
    }

    if (enabled)
    {
        enableInterface(eigrpIface);
        startHelloTimer(eigrpIface, simTime() + uniform(0,1));
    }
    else
    {
        eigrpIftDisabled->addInterface(eigrpIface);
    }

    return eigrpIface;
}


void EigrpIpv6Pdm::enableInterface(EigrpInterface *eigrpIface)
{
    int ifaceId = eigrpIface->getInterfaceId();
    EigrpRouteSource<IPv6Address> *src;
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

    // Register multicast address on interface      //TODO - should be passive interface joined in multicast group?
    IPv6InterfaceData *ifaceIpv6 = ift->getInterfaceById(ifaceId)->ipv6Data();
    ifaceIpv6->joinMulticastGroup(EIGRP_IPV6_MULT); //join to group FF02::A, optionally
    ifaceIpv6->assignAddress(EIGRP_IPV6_MULT, false, 0, 0); //add group address to interface, mandatory


    PrefixVector::iterator it;
    EigrpNetwork<IPv6Address> *eigrpnet = NULL;
    IPv6Address network;
    IPv6Address mask;

    for (it = netPrefixes.begin(); it != netPrefixes.end(); ++it)
    {// through all known prefixes search prefixes belonging to enabling interface
        if (it->ifaceId == ifaceId)
        {// found prefix belonging to interface
            network = it->network;
            mask = IPv6Address(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff).getPrefix(it->prefixLength);

            eigrpnet = addNetwork(network, mask);

            if(eigrpnet)
            {//successfully added to EigrpNetworkTable -> tie with interface
                eigrpIface->insertToNetworksIds(eigrpnet->getNetworkId()); //pak predelat na protected a pridat metody
            }

            // Create route
            src = eigrpTt->findOrCreateRoute(network, mask, eigrpTt->getRouterId(), eigrpIface, EigrpNeighbor<IPv6Address>::UNSPEC_ID, &isSourceNew);
            ASSERT(isSourceNew == true);
            // Compute metric
            metricPar = eigrpMetric->getParam(eigrpIface);
            src->setMetricParams(metricPar);
            uint64_t metric = eigrpMetric->computeClassicMetric(metricPar, this->kValues);
            src->setMetric(metric);

            // Notify DUAL about event
            eigrpDual->processEvent(EigrpDual<IPv6Address>::INTERFACE_UP, src, EigrpNeighbor<IPv6Address>::UNSPEC_ID, isSourceNew);
        }
    }


    flushMsgRequests();
    eigrpTt->purgeTable();
}


void EigrpIpv6Pdm::startHelloTimer(EigrpInterface *eigrpIface, simtime_t interval)
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

EigrpNetwork<IPv6Address> *EigrpIpv6Pdm::addNetwork(IPv6Address address, IPv6Address mask)
{
    return routingForNetworks->addNetwork(address, mask);
}

void EigrpIpv6Pdm::setHelloInt(int interval, int ifaceId)
{
    EigrpInterface *iface = getInterfaceById(ifaceId);
    if (iface == NULL)
        iface = addInterfaceToEigrp(ifaceId, false);
    iface->setHelloInt(interval);
}

void EigrpIpv6Pdm::setHoldInt(int interval, int ifaceId)
{
    EigrpInterface *iface = getInterfaceById(ifaceId);
    if (iface == NULL)
        iface = addInterfaceToEigrp(ifaceId, false);
    iface->setHoldInt(interval);
}

void EigrpIpv6Pdm::setSplitHorizon(bool shenabled, int ifaceId)
{
    EigrpInterface *iface = getInterfaceById(ifaceId);
    if (iface == NULL)
        iface = addInterfaceToEigrp(ifaceId, false);
    iface->setSplitHorizon(shenabled);
}

void EigrpIpv6Pdm::setPassive(bool passive, int ifaceId)
{
    EigrpInterface *eigrpIface = getInterfaceById(ifaceId);

    if (eigrpIface == NULL)
        eigrpIface = addInterfaceToEigrp(ifaceId, false);
    else if (eigrpIface->isEnabled())
    { // Disable sending and receiving of messages
        InterfaceEntry *iface = ift->getInterfaceById(ifaceId);
        iface->ipv6Data()->leaveMulticastGroup(EIGRP_IPV6_MULT);

        if(iface->ipv6Data()->hasAddress(EIGRP_IPV6_MULT))
        {
            iface->ipv6Data()->removeAddress(EIGRP_IPV6_MULT);
        }

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

void EigrpIpv6Pdm::sendUpdate(int destNeighbor, EigrpRoute<IPv6Address> *route, EigrpRouteSource<IPv6Address> *source, bool forcePoisonRev, const char *reason)
{
    ev << "DUAL: send Update message about " << route->getRouteAddress() << " to all neighbors, " << reason << endl;
    if (this->eigrpStubEnabled && applyStubToUpdate(source))
    {
        ev << "     Stub routing applied, message will not be sent" << endl;
        return;
    }
    msgToAllIfaces(destNeighbor, EIGRP_UPDATE_MSG, source, forcePoisonRev, false);
}

void EigrpIpv6Pdm::sendQuery(int destNeighbor, EigrpRoute<IPv6Address> *route, EigrpRouteSource<IPv6Address> *source, bool forcePoisonRev)
{
    bool forceUnreachable = false;

    ev << "DUAL: send Query message about " << route->getRouteAddress() << " to all neighbors" << endl;

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

void EigrpIpv6Pdm::sendReply(EigrpRoute<IPv6Address> *route, int destNeighbor, EigrpRouteSource<IPv6Address> *source, bool forcePoisonRev, bool isUnreachable)
{
    EigrpMsgRoute msgRt;
    EigrpNeighbor<IPv6Address> *neigh = eigrpNt->findNeighborById(destNeighbor);

    ev << "DUAL: send Reply message about " << route->getRouteAddress() << endl;
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
bool EigrpIpv6Pdm::removeRouteFromRT(EigrpRouteSource<IPv6Address> *source, ANSAIPv6Route::RoutingProtocolSource *removedRtSrc)
{
    EigrpRoute<IPv6Address> *route = source->getRouteInfo();
    IPv6Route *rtEntry =rt->findRoute(route->getRouteAddress(), getNetmaskLength(route->getRouteMask()), source->getNextHop());
    ANSAIPv6Route *ansaRtEntry = dynamic_cast<ANSAIPv6Route *>(rtEntry);

    if (rtEntry != NULL && ansaRtEntry != NULL)
    {
        *removedRtSrc = ansaRtEntry->getRoutingProtocolSource();
        if (*removedRtSrc == ANSAIPv6Route::pEIGRP)
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
    return rtEntry != NULL;
}

bool EigrpIpv6Pdm::isRTSafeForAdd(EigrpRoute<IPv6Address> *route, unsigned int eigrpAd)
{
    IPv6Route *routeInTable = rt->findRoute(route->getRouteAddress(), getNetmaskLength(route->getRouteMask()));
    ANSAIPv6Route *ansaRoute = NULL;

    if (routeInTable == NULL)
        return true; // Route not found

    ansaRoute = dynamic_cast<ANSAIPv6Route*>(routeInTable);
    if (ansaRoute != NULL)
    { // AnsaIPv4Route use own AD attribute
        if (ansaRoute->getAdminDist() < eigrpAd)
            return false;
        return true;
    }
    if (ansaRoute == NULL && routeInTable->getAdminDist() == IPv6Route::dUnknown)
        return false;   // Connected route has AD = 255 (dUnknown) in IPv4Route
    if (routeInTable != NULL && routeInTable->getAdminDist() < eigrpAd)
        return false;   // Other IPv4Route with right AD
    return true;
}

EigrpRouteSource<IPv6Address> *EigrpIpv6Pdm::updateRoute(EigrpRoute<IPv6Address> *route, uint64_t dmin, bool *rtableChanged, bool removeUnreach)
{
    EigrpRouteSource<IPv6Address> *source = NULL, *bestSuccessor = NULL;
    IPv6Route *rtEntry = NULL;
    int routeNum = eigrpTt->getNumRoutes();
    int routeId = route->getRouteId();
    int pathsInRT = 0;      // Number of paths in RT (equal to number of successors)
    int sourceCounter = 0;  // Number of route sources
    uint64_t routeFd = route->getFd();

    ev << "EIGRP: Search successor for route " << route->getRouteAddress() << ", FD is " << route->getFd() << endl;

    for (int i = 0; i < routeNum; i++)
    {
        source = eigrpTt->getRoute(i);
        if (source->getRouteId() != routeId || !source->isValid())
            continue;

        sourceCounter++;

        if (source->getRd() < routeFd /* FC, use this FD (not dmin) */ &&
                source->getMetric() <= dmin*this->variance && pathsInRT < this->maximumPath) /* Load Balancing */
        {
            ev << "     successor " << source->getNextHop() << " (" << source->getMetric() << "/" << source->getRd() << ")" << endl;

            if ((rtEntry = rt->findRoute(route->getRouteAddress(), getNetmaskLength(route->getRouteMask()), source->getNextHop())) == NULL)
                if (!isRTSafeForAdd(route, adminDistInt))
                { // In RT exists route with smaller AD, do not mark the route source as successor
                    source->setSuccessor(false);
                    ev << "           route can not be added into RT, there is route with smaller AD" << endl;
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
            ev << "     invalidate route via " << source->getNextHop() << " in TT" << endl;
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

bool EigrpIpv6Pdm::removeOldSuccessor(EigrpRouteSource<IPv6Address> *source, EigrpRoute<IPv6Address> *route)
{
    ANSAIPv6Route::RoutingProtocolSource srcProto = ANSAIPv6Route::pUnknown;
    bool rtFound, rtableChanged = false;

    // To distinguish the route deleted by EIGRP in ReceiveChangeNotification method
    source->setSuccessor(false);

    rtFound = removeRouteFromRT(source, &srcProto);
#ifdef EIGRP_DEBUG
    //ev << "EIGRP: removing old successor: rt found: " << rtFound << " src: " << srcProto << endl;
#endif

    if (!rtFound || (rtFound && (srcProto == ANSAIPv6Route::pEIGRP || srcProto == ANSAIPv6Route::pEIGRPext)))
    {
        if (route->getSuccessor() == source)
            route->setSuccessor(NULL);
        rtableChanged = true;
    }
    else // Route from other sources can not be removed, do not change Successor's record in TT
        source->setSuccessor(true);

    return rtableChanged;
}

bool EigrpIpv6Pdm::installRouteToRT(EigrpRoute<IPv6Address> *route, EigrpRouteSource<IPv6Address> *source, uint64_t dmin, IPv6Route *rtEntry)
{
    ANSAIPv6Route *ansaRtEntry = NULL;
    bool rtableChanged = false;

    if (rtEntry != NULL && (ansaRtEntry = dynamic_cast<ANSAIPv6Route *>(rtEntry)) != NULL)
    {
        if (ansaRtEntry->getRoutingProtocolSource() != ANSAIPv6Route::pEIGRP)
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
        rt->addRoutingProtocolRoute(ansaRtEntry);

        rtableChanged = true;
    }

    return rtableChanged;
}

bool EigrpIpv6Pdm::setReplyStatusTable(EigrpRoute<IPv6Address> *route, EigrpRouteSource<IPv6Address> *source, bool forcePoisonRev, int *neighCount, int *stubCount)
{
    int neighTotalCount = eigrpNt->getNumNeighbors();
    EigrpNeighbor<IPv6Address> *neigh;
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

bool EigrpIpv6Pdm::hasNeighborForUpdate(EigrpRouteSource<IPv6Address> *source)
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

void EigrpIpv6Pdm::setDelayedRemove(int neighId, EigrpRouteSource<IPv6Address> *src)
{
    EigrpNeighbor<IPv6Address> *neigh = eigrpNt->findNeighborById(neighId);


    ASSERT(neigh != NULL);
    neigh->setRoutesForDeletion(true);
    src->setDelayedRemove(neighId);
    src->setValid(true);    // Can not be invalid

#ifdef EIGRP_DEBUG
        ev << "DUAL: route via " << src->getNextHop() << " will be removed from TT after receiving Ack from neighbor" << endl;
#endif
}

void EigrpIpv6Pdm::sendUpdateToStubs(EigrpRouteSource<IPv6Address> *succ ,EigrpRouteSource<IPv6Address> *oldSucc, EigrpRoute<IPv6Address> *route)
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

bool EigrpIpv6Pdm::addNetPrefix(const IPv6Address &network, const short int prefixLen, const int ifaceId)
{
    PrefixVector::iterator it;

    for (it = netPrefixes.begin(); it != netPrefixes.end(); ++it)
    {// through all known prefixes search same prefix
        if(it->network == network && it->prefixLength == prefixLen)
        {// found same prefix
            if(it->ifaceId == ifaceId)
            {// belonging to same interface = more than one IPv6 addresses from same prefix on interface = ok -> already added
                return true;
            }
            else
            {// same prefix on different interfaces = bad -> do not add
                return false;
            }
        }
    }

    // Add new prefix
    IPv6netPrefix newprefix;
    newprefix.network = network;
    newprefix.prefixLength = prefixLen;
    newprefix.ifaceId = ifaceId;
    this->netPrefixes.push_back(newprefix);

    //EV << "Added prefix: " << this->netPrefixes.back().network << "/" << this->netPrefixes.back().prefixLength << " on iface " << this->netPrefixes.back().ifaceId << endl;

    return true;
}
