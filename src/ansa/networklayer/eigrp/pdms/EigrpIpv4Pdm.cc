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

#include "IPv4ControlInfo.h"
#include "deviceConfigurator.h"
#include "AnsaIPv4Route.h"

#include "EigrpIpv4Pdm.h"
#include "EigrpInterfaceTable.h"
#include "EigrpIpv4NeighborTable.h"
#include "EigrpIpv4TopologyTable.h"
#include "EigrpTimer_m.h"
#include "EigrpMessage_m.h"
#include "EigrpDual.h"

#define DEBUG

Define_Module(EigrpIpv4Pdm);

namespace eigrp
{

// User message codes
enum UserMsgCodes
{
  M_OK = 0,             // no message
  M_DISABLED_ON_IF,     // EIGRP is disabled on interface
  M_NEIGH_BAD_AS,       // neighbor has bad AS number
  M_NEIGH_BAD_KVALUES,  // neighbor has bad K-values
  M_NEIGH_BAD_SUBNET,   // neighbor isn't on common subnet
  M_HELLO_SEND,         // send Hello message
  M_UPDATE_SEND,        // send Update message
  M_QUERY_SEND,        // send Query message
  M_REPLY_SEND,        // send Query message
};

// User messages
const char *UserMsgs[] =
{
  // M_OK
  "OK",
  // M_DISABLED_ON_IF
  "EIGRP process isn't enabled on interface",
  // M_NEIGH_BAD_AS
  "AS number is different",
  // M_NEIGH_BAD_KVALUES
  "K-value mismatch",
  // M_NEIGH_BAD_SUBNET
  "not on the common subnet",
  // M_HELLO_SEND
  "send Hello message",
  // M_UPDATE_SEND
  "send Update message",
  // M_QUERY_SEND
  "send Query message",
  // M_REPLY_SEND
  "send Reply message",
};

}; // end of namespace eigrp

/**
 * Output operator for IPv4Network class.
 */
std::ostream& operator<<(std::ostream& os, const EigrpNetwork& network)
{
    os << "Address = " << network.getAddress() << ", Mask = " << network.getMask();
    return os;
}

bool operator==(const EigrpKValues& k1, const EigrpKValues& k2)
{
    return k1.K1 == k2.K1 && k1.K2 == k2.K2 &&
            k1.K3 == k2.K3 && k1.K4 == k2.K4 &&
            k1.K5 == k2.K5 && k1.K6 == k2.K6;
}


EigrpIpv4Pdm::EigrpIpv4Pdm() : EIGRP_IPV4_MULT(IPv4Address(224, 0, 0, 10))
{
    //EIGRP_IPV4_MULT.set("224.0.0.10");
    PDM_OUTPUT_GW = "rtpOut";

    KVALUES_MAX.K1 = KVALUES_MAX.K2 = KVALUES_MAX.K3 = KVALUES_MAX.K4 = KVALUES_MAX.K5 = 255;
    KVALUES_MAX.K6 = 0;

    asNum = -1;

    // K1 = K3 = 1; other K-values are default 0
    kValues.K1 = kValues.K3 = 1;
    kValues.K2 = kValues.K4 = kValues.K5 = kValues.K6 = 0;
}

EigrpIpv4Pdm::~EigrpIpv4Pdm()
{
    delete this->routingForNetworks;
    delete this->eigrpIftDisabled;
    delete this->eigrpDual;
}

void EigrpIpv4Pdm::initialize(int stage)
{
    // in stage 0 interfaces are registrated
    // in stage 2 interfaces and routing table
    // in stage 3
    if (stage == 3)
    {
        this->eigrpIft = ModuleAccess<EigrpInterfaceTable>("eigrpInterfaceTable").get();
        this->eigrpNt = ModuleAccess<EigrpIpv4NeighborTable>("eigrpIpv4NeighborTable").get();
        this->eigrpTt = ModuleAccess<EigrpIpv4TopologyTable>("eigrpIpv4TopologyTable").get();
        this->rt = AnsaRoutingTableAccess().get();
        //this->eigrpDual = ModuleAccess<EigrpDual>("eigrpDual").get();

        this->ift = InterfaceTableAccess().get();
        this->nb = NotificationBoardAccess().get();

        // subscribe for changes in the device
        nb->subscribe(this, NF_INTERFACE_STATE_CHANGED);
        nb->subscribe(this, NF_INTERFACE_CONFIG_CHANGED);
        nb->subscribe(this, NF_IPv4_ROUTE_DELETED);
        // TODO in future NF_INTERFACE_STATE_CHANGED for BW and DLY change

        this->eigrpIftDisabled = new EigrpDisabledInterfaces();
        this->routingForNetworks = new EigrpNetworkTable();
        this->eigrpDual = new EigrpDual(this);
        this->eigrpMetric = new EigrpMetricHelper();

        // load configuration
        DeviceConfigurator *conf = ModuleAccess<DeviceConfigurator>("deviceConfigurator").get();
        conf->loadEigrpIPv4Config(this);

        WATCH_PTRVECTOR(*routingForNetworks->getAllNetworks());
        WATCH(asNum);
        WATCH(kValues.K1);
        WATCH(kValues.K2);
        WATCH(kValues.K3);
        WATCH(kValues.K4);
        WATCH(kValues.K5);
        WATCH(kValues.K6);
    }
}


void EigrpIpv4Pdm::receiveChangeNotification(int category, const cObject *details)
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
                processIfaceConfigChange(iface, eigrpIface);
        }
        ifParam = iface->getDelay();
        if (ifParam != eigrpIface->getDelay())
        { // Delay
            eigrpIface->setDelay(ifParam);
            if (eigrpIface->isEnabled())
                processIfaceConfigChange(iface, eigrpIface);
        }
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

        // Get source IP address and interface ID
        IPv4ControlInfo *ctrlInfo =check_and_cast<IPv4ControlInfo *>(msg->getControlInfo());
        IPv4Address srcAddr = ctrlInfo->getSrcAddr();
        int ifaceId = ctrlInfo->getInterfaceId();

        // Find neighbor if exists
        EigrpNeighbor<IPv4Address> *neigh;
        if ((neigh = eigrpNt->findNeighbor(srcAddr)) != NULL)
        { // Reset hold timer
            resetTimer(neigh->getHoldTimer(), neigh->getHoldInt());
        }

        if (dynamic_cast<EigrpHello*>(msg))
        {
            EV << "Received Hello message from " << srcAddr << endl;
            processHelloMsg(msg, srcAddr, ifaceId, neigh);
        }
        else if (dynamic_cast<EigrpUpdate*>(msg) && neigh != NULL)
        {
            EV << "Received Update message from " << srcAddr << endl;
            processUpdateMsg(msg, srcAddr, ifaceId, neigh);
        }
        else if (dynamic_cast<EigrpQuery*>(msg) && neigh != NULL)
        {
            EV << "Received Query message from " << srcAddr << endl;
            processQueryMsg(msg, srcAddr, ifaceId, neigh);
        }
        else if (dynamic_cast<EigrpReply*>(msg) && neigh != NULL)
        {
            EV << "Received Reply message from " << srcAddr << endl;
            processReplyMsg(msg, srcAddr, ifaceId, neigh);
        }
        else if (neigh != NULL)
        {
            EV << "Received message of unknown type, skipped" << endl;
        }
        else
        {
            EV << "Received message from "<< srcAddr <<" that is not neighbor" << endl;
        }

        delete msg;
        msg = NULL;
    }
}

void EigrpIpv4Pdm::processTimer(cMessage *msg)
{
    EigrpTimer *timer = check_and_cast<EigrpTimer*>(msg);
    char timerKind = timer->getTimerKind();

    EigrpInterface *eigrpIface;
    InterfaceEntry *iface;
    IPv4Address ifMask;
    EigrpNeighbor<IPv4Address> *neigh;
    cObject *contextBasePtr;

    int ifaceId;

    switch (timerKind)
    {
    case EIGRP_HELLO_TIMER:
        // get interface that belongs to timer
        contextBasePtr = (cObject*)timer->getContextPointer();
        eigrpIface = check_and_cast<EigrpInterface *>(contextBasePtr);

        // schedule Hello timer
        scheduleAt(simTime() + eigrpIface->getHelloInt() /*- uniform(0, 0.4)*/, timer);

        // send Hello message
        sendHelloMsg(eigrpIface->getInterfaceId(), eigrpIface->getHoldInt(), this->kValues);
        break;

    case EIGRP_HOLD_TIMER:
        // get neighbor from context
        contextBasePtr = (cObject*)timer->getContextPointer();
        neigh = check_and_cast<EigrpNeighbor<IPv4Address> *>(contextBasePtr);
        ifaceId = neigh->getIfaceId();

        // remove neighbor
        EV << "Neighbor " << neigh->getIPAddress() <<" is down, holding time expired" << endl;
        iface = ift->getInterfaceById(ifaceId);
        ifMask = iface->ipv4Data()->getNetmask();
        removeNeighbor(neigh, ifMask);
        neigh = NULL;

        // Send goodbye and restart Hello timer
        eigrpIface = eigrpIft->findInterfaceById(ifaceId);
        resetTimer(eigrpIface->getHelloTimer(), eigrpIface->getHelloInt());
        sendGoodbyeMsg(ifaceId, eigrpIface->getHoldInt());
        break;

    default:
        EV << "Timer with unknown kind was skipped" << endl;
        delete timer;
        timer = NULL;
        break;
    }
}

void EigrpIpv4Pdm::processHelloMsg(cMessage *msg, IPv4Address& srcAddress, int ifaceId, EigrpNeighbor<IPv4Address> *neigh)
{
    InterfaceEntry *iface = NULL;
    IPv4Address ifMask;
    EigrpHello *hello = check_and_cast<EigrpHello *>(msg);
    EigrpTlvParameter tlvParam = hello->getParameterTlv();

    if (tlvParam.kValues == KVALUES_MAX)
    { // Received Goodbye message, remove neighbor
        EV << "Interface goodbye received" << endl;
        if (neigh != NULL)
        {
            EV << "EIGRP neighbor " << srcAddress << " is down, interface goodbye received" << endl;
            iface = ift->getInterfaceById(ifaceId);
            ifMask = iface->ipv4Data()->getNetmask();
            removeNeighbor(neigh, ifMask);
        }
        return;
    }

    if (neigh == NULL)
    { // New neighbor
        processNewNeighbor(ifaceId, srcAddress, hello);
    }
    else
    { // neighbor exists, its state is "up" or "pending"
        // Check K-values
        if (!(tlvParam.kValues == this->kValues))
        { // Not satisfied
            EV << "EIGRP neighbor " << srcAddress << " is down, " << eigrp::UserMsgs[eigrp::M_NEIGH_BAD_KVALUES] << endl;
            iface = ift->getInterfaceById(ifaceId);
            ifMask = iface->ipv4Data()->getNetmask();
            removeNeighbor(neigh, ifMask);

            // send Goodbye message and reset Hello timer
            EigrpInterface *iface = this->eigrpIft->findInterfaceById(ifaceId);
            resetTimer(iface->getHelloTimer(), iface->getHelloInt());
            sendGoodbyeMsg(ifaceId, iface->getHoldInt());

            return;
        }

        // Save Hold interval
        if (tlvParam.holdTimer != neigh->getHoldInt())
            neigh->setHoldInt(tlvParam.holdTimer);
    }
}

void EigrpIpv4Pdm::processUpdateMsg(cMessage *msg, IPv4Address& srcAddress, int ifaceId, EigrpNeighbor<IPv4Address> *neigh)
{
    EigrpIpv4Update *update = check_and_cast<EigrpIpv4Update *>(msg);
    EigrpInterface *eigrpIface = eigrpIft->findInterfaceById(ifaceId);
    EigrpRouteSource<IPv4Address> *src;

    if (update->getInit())
    { // Request to send all paths from routing table
        // If neighbor is "pending", then change its state to "up"
        if (!neigh->getState())
        {
            neigh->setState(true);
        }

        // Send all EIGRP paths from routing table to sender
        sendAllEigrpPaths(ifaceId, srcAddress);
    }
    else
    {
        if (neigh->getState())
        { // Neighbor is "up", forward message to DUAL
            int cnt = update->getInterRoutesArraySize();
            bool sourceNew;


            for (int i = 0; i < cnt; i++)
            {
                src = processInterRoute(update->getInterRoutes(i), srcAddress, neigh->getNeighborId(), eigrpIface, sourceNew);
                // Notify DUAL about event
                eigrpDual->processEvent(EigrpDual::RECV_UPDATE, src, sourceNew);
            }
        }
        // else ignore message
    }
}

void EigrpIpv4Pdm::processQueryMsg(cMessage *msg, IPv4Address& srcAddress, int ifaceId, EigrpNeighbor<IPv4Address> *neigh)
{
    EigrpIpv4Query *query = check_and_cast<EigrpIpv4Query *>(msg);
    EigrpInterface *eigrpIface = eigrpIft->findInterfaceById(ifaceId);
    EigrpRouteSource<IPv4Address> *src;
    bool sourceNew;

    int cnt = query->getInterRoutesArraySize();
    for (int i = 0; i < cnt; i++)
    {
        src = processInterRoute(query->getInterRoutes(i), srcAddress, neigh->getNeighborId(), eigrpIface, sourceNew);
        eigrpDual->processEvent(EigrpDual::RECV_QUERY, src, sourceNew);
    }
}

void EigrpIpv4Pdm::processReplyMsg(cMessage *msg, IPv4Address& srcAddress, int ifaceId, EigrpNeighbor<IPv4Address> *neigh)
{
    EigrpIpv4Reply *reply = check_and_cast<EigrpIpv4Reply *>(msg);
    EigrpInterface *eigrpIface = eigrpIft->findInterfaceById(ifaceId);
    EigrpRouteSource<IPv4Address> *src;
    bool sourceNew;

    int cnt = reply->getInterRoutesArraySize();
    for (int i = 0; i < cnt; i++)
    {
        src = processInterRoute(reply->getInterRoutes(i), srcAddress, neigh->getNeighborId(), eigrpIface, sourceNew);
        eigrpDual->processEvent(EigrpDual::RECV_REPLY, src, sourceNew);
    }
}

/**
 * @param neigh Neighbor which is next hop for a route in TLV.
 */
EigrpRouteSource<IPv4Address> *EigrpIpv4Pdm::processInterRoute(EigrpIpv4Internal& tlv, IPv4Address& srcAddr,
        int sourceNeighId, EigrpInterface *eigrpIface, bool &sourceNewResult)
{
    int ifaceId = eigrpIface->getInterfaceId();
    IPv4Address nextHop = getNextHopAddr(tlv.nextHop, srcAddr);
    EigrpNeighbor<IPv4Address> *nextHopNeigh = eigrpNt->findNeighbor(nextHop);
    InterfaceEntry *iface = ift->getInterfaceById(ifaceId);
    EigrpRouteSource<IPv4Address> *src;
    EigrpMetricPar newParam, oldParam;
    EigrpMetricPar ifParam = eigrpMetric->getParam(eigrpIface, iface);

    // Find route or create one (route source is identified by ID of the next hop - not by ID of sender)
    src = eigrpTt->findOrCreateRoute(tlv.destAddress, tlv.destMask, ifaceId, nextHopNeigh->getNeighborId(), sourceNewResult);

    // Get new metric parameters
    newParam = eigrpMetric->adjustParam(ifParam, tlv.metric);
    // Get old metric parameters for comparison
    oldParam = src->getMetricParams();

    // Compute reported distance (must be there)
    uint32_t metric = eigrpMetric->computeMetric(tlv.metric, this->kValues);
    src->setRdParams(tlv.metric);
    src->setRd(metric);

    if (sourceNewResult || !eigrpMetric->compareParamters(newParam, oldParam, this->kValues))
    {
        // Set source of route
        if (sourceNewResult)
            src->setNextHop(nextHop);

        src->setSourceId(sourceNeighId);
        src->setMetricParams(newParam);
        // Compute metric
        metric = eigrpMetric->computeMetric(newParam, this->kValues);
        src->setMetric(metric);
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

inline void EigrpIpv4Pdm::resetTimer(EigrpTimer *timer, int interval)
{
    cancelEvent(timer);
    // Uniform je zde, protoze bez nej nefunguje simulace jak by mela
    // (pri ustaveni sousedstvi se detekuji spatne K-hodnoty (coz je spravne),
    // odesle se Goodbye (spravne), ale hned po ni take Hello (spatne, goodbye by
    // mela resetovat Hello timer, ale asi to nestihne).
    //      UZ TO NEDELA - NEVIM PROC
    scheduleAt(simTime() + interval /*- uniform(0,0.4)*/, timer);
}

void EigrpIpv4Pdm::sendHelloMsg(int destIfaceId, int holdInt, EigrpKValues kValues)
{
    EigrpHello *msg = createHelloMsg(destIfaceId, holdInt, kValues);
    send(msg, PDM_OUTPUT_GW);

    EV << eigrp::UserMsgs[eigrp::M_HELLO_SEND] << " to interface " << destIfaceId << endl;
}

void EigrpIpv4Pdm::sendGoodbyeMsg(int destIfaceId, int holdInt)
{
    EigrpHello *msg = createHelloMsg(destIfaceId, holdInt, KVALUES_MAX);
    send(msg, PDM_OUTPUT_GW);

    EV << eigrp::UserMsgs[eigrp::M_HELLO_SEND] << "(Goodbye)" << " to interface " << destIfaceId << endl;
}

EigrpIpv4Hello *EigrpIpv4Pdm::createHelloMsg(int destIfaceId, int holdInt, EigrpKValues kValues)
{
    EigrpIpv4Hello *msg = new EigrpIpv4Hello("EIGRPHello");
    EigrpTlvParameter paramTlv;

    addMessageHeader(msg, EIGRP_HELLO_MSG);
    addCtrInfo(msg, destIfaceId, EIGRP_IPV4_MULT);

    // add parameter type TLV
    paramTlv.holdTimer = holdInt;
    paramTlv.kValues = kValues;
    msg->setParameterTlv(paramTlv);

    return msg;
}

EigrpIpv4Update *EigrpIpv4Pdm::createUpdateMsg(int destIfaceId, IPv4Address destAddress)
{
    EigrpIpv4Update *msg = new EigrpIpv4Update("EIGRPUpdate");

    addMessageHeader(msg, EIGRP_UPDATE_MSG);
    addCtrInfo(msg, destIfaceId, destAddress);

    return msg;
}

EigrpIpv4Query *EigrpIpv4Pdm::createQueryMsg(int destIfaceId,IPv4Address destAddress)
{
    EigrpIpv4Query *msg = new EigrpIpv4Query("EIGRPQuery");

    addMessageHeader(msg, EIGRP_QUERY_MSG);
    addCtrInfo(msg, destIfaceId, destAddress);

    return msg;
}

EigrpIpv4Reply *EigrpIpv4Pdm::createReplyMsg(int destIfaceId,IPv4Address destAddress)
{
    EigrpIpv4Reply *msg = new EigrpIpv4Reply("EIGRPReply");

    addMessageHeader(msg, EIGRP_REPLY_MSG);
    addCtrInfo(msg, destIfaceId, destAddress);

    return msg;
}

void EigrpIpv4Pdm::addMessageHeader(EigrpMessage *msg, int opcode)
{
    msg->setOpcode(opcode);
    msg->setAsNum(this->asNum);
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

/**
 * In the RT may not be all EIGRP routes due to administrative distance of route.
 * Send all path from EIGRP TT.
 */
void EigrpIpv4Pdm::sendAllEigrpPaths(int destIface, IPv4Address& destAddress)
{
    int cnt = eigrpTt->getNumRoutes();
    EigrpRoute<IPv4Address> *route;
    EigrpRouteSource<IPv4Address> *routeSrc;
    EigrpIpv4Update *msg = createUpdateMsg(destIface, destAddress);

    msg->setInterRoutesArraySize(cnt);

    // TODO vsechny cesty nebyvaji v 1 zprave, ale je to nejak rozdeleny. V posledni zprave je flag EOT

    for (int i = 0; i < cnt; i++)
    {
        routeSrc = eigrpTt->getRoute(i);

        if (routeSrc->isSuccessor())
        {
            route = routeSrc->getRouteInfo();

            EigrpIpv4Internal routeTlv;
            routeTlv.destAddress = route->getRouteAddress();
            routeTlv.destMask = route->getRouteMask();
            routeTlv.nextHop = EIGRP_SELF_ADDR;    // Sender is next hop (convention)
            routeTlv.metric = routeSrc->getMetricParams();

            msg->setInterRoutes(i, routeTlv);
        }
    }

    msg->setEot(true);
    send(msg, PDM_OUTPUT_GW);

    EV << eigrp::UserMsgs[eigrp::M_UPDATE_SEND] << " with all paths from RT to " << destAddress << endl;
}

void EigrpIpv4Pdm::sendMsgToAllNeighbors(EigrpMessage *msg)
{
    EigrpNeighbor<IPv4Address> *neigh;
    EigrpMessage *msgCopy;
    int neighCount = eigrpNt->getNumNeighbors();

    for (int i = 0; i < neighCount; i++)
    {
        neigh = eigrpNt->getNeighbor(i);

        msgCopy = msg->dup();
        addCtrInfo(msgCopy, neigh->getIfaceId(), neigh->getIPAddress());
        send(msgCopy, PDM_OUTPUT_GW);
    }

    // Original msg is not used, delete it
    delete msg;
}

void EigrpIpv4Pdm::sendQueryToAllNeighbors(EigrpIpv4Query *msg, int nextHopId)
{
    EigrpNeighbor<IPv4Address> *neigh;
    EigrpIpv4Query *msgCopy;
    int neighCount = eigrpNt->getNumNeighbors();

    for (int i = 0; i < neighCount; i++)
    {
        neigh = eigrpNt->getNeighbor(i);
        msgCopy = msg->dup();

        if (neigh->getNeighborId() == nextHopId)
        { // infinite metrik only to nexthop
            EigrpIpv4Internal tlv = msgCopy->getInterRoutes(0);
            tlv.metric.delay = UINT32_MAX;
            msgCopy->setInterRoutes(0, tlv);
        }

        addCtrInfo(msgCopy, neigh->getIfaceId(), neigh->getIPAddress());
        send(msgCopy, PDM_OUTPUT_GW);
    }

    // Original msg is not used, delete it
    delete msg;
}

/**
 * Creates relationship with neighbor.
 *
 * @param srcAddress address of the neighbor
 * @param ifaceId ID of interface where the neighbor is connected
 * @param helloMe
 */
void EigrpIpv4Pdm::processNewNeighbor(int ifaceId,
        IPv4Address &srcAddress, EigrpHello *rcvMsg)
{
    EigrpUpdate *updateMsg;
    EigrpNeighbor<IPv4Address> *neigh;
    EigrpInterface *iface = eigrpIft->findInterfaceById(ifaceId);
    EigrpTlvParameter paramTlv = rcvMsg->getParameterTlv();
    int ecode;      // Code of user message

    // Check rules for establishing neighborship
    if ((ecode = checkNeighborshipRules(ifaceId, rcvMsg->getAsNum(), srcAddress,
            paramTlv.kValues)) != eigrp::M_OK)
    {
        EV << "EIGRP can't create neighborship with " << srcAddress << ", " << eigrp::UserMsgs[ecode] << endl;

        if (ecode == eigrp::M_NEIGH_BAD_KVALUES)
        { // Send Goodbye message and reset Hello timer
            resetTimer(iface->getHelloTimer(), iface->getHelloInt());
            sendGoodbyeMsg(ifaceId, iface->getHelloInt());
        }

        return;
    }

    // Create record in the neighbor table
    neigh = new EigrpNeighbor<IPv4Address>(ifaceId, srcAddress);
    EigrpTimer *holdt = createTimer(EIGRP_HOLD_TIMER, neigh);
    neigh->setHoldTimer(holdt);
    neigh->setHoldInt(paramTlv.holdTimer);
    eigrpNt->addNeighbor(neigh);

    EV << "Neighbor " << srcAddress << " is up, new adjacency" << endl;

    // Start Hold timer
    scheduleAt(simTime() + neigh->getHoldInt(), holdt);

    // Reply with Hello message and reset Hello timer
    resetTimer(iface->getHelloTimer(), iface->getHelloInt());
    sendHelloMsg(ifaceId, iface->getHoldInt(), this->kValues);

    // Send Update with INIT flag
    updateMsg = createUpdateMsg(ifaceId, srcAddress);
    updateMsg->setInit(true);
    send(updateMsg, PDM_OUTPUT_GW);
    EV << eigrp::UserMsgs[eigrp::M_UPDATE_SEND] << " to " << srcAddress << endl;
}

/**
 * @param ifaceId ID of interface where the neighbor is connected
 *
 * @return returns code from enumeration eigrp::UserMsgCodes
 */
int EigrpIpv4Pdm::checkNeighborshipRules(int ifaceId, int neighAsNum,
        IPv4Address &neighAddr, const EigrpKValues &neighKValues)
{
    IPv4Address ifaceAddr, ifaceMask;

    if (this->eigrpIft->findInterfaceById(ifaceId) == NULL)
    { // EIGRP must be enabled on interface
        return eigrp::M_DISABLED_ON_IF;
    }
    else if (this->asNum != neighAsNum)
    { // AS nubers must be equal
        // TODO az bude AsSplitter funkcni, tak zpravy s neznamym AS cislem
        // bude zahazovat. Takze tato kontrola je tady zbytecna.
        return eigrp::M_NEIGH_BAD_AS;
    }
    else if (!(this->kValues == neighKValues))
    { // K-values must be same
        return eigrp::M_NEIGH_BAD_KVALUES;
    }
    else
    {
        // get IP address of interface and mask
        InterfaceEntry *iface = ift->getInterfaceById(ifaceId);
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

/**
 * Removes neighbor from neighbor table and delete it.
 */
void EigrpIpv4Pdm::removeNeighbor(EigrpNeighbor<IPv4Address> *neigh, IPv4Address& neighMask)
{
    EigrpRouteSource<IPv4Address> *source;

    // Remove neighbor from NT
    this->eigrpNt->removeNeighbor(neigh);

    // First process route to neighbor
    IPv4Address routeAddr = neigh->getIPAddress().doAnd(neighMask);
    source = eigrpTt->findRoute(routeAddr, neighMask, EigrpRouteSource<IPv4Address>::CONNECTED_ID);
    // Notify DUAL about event
    eigrpDual->processEvent(EigrpDual::NEIGHBOR_DOWN, source, false);

    // Process routes that go via this neighbor
    int routeCount = eigrpTt->getNumRoutes();
    int nextHopId = neigh->getNeighborId();

    for (int i = 0; i < routeCount; i++)
    {
        source = eigrpTt->getRoute(i);
        if (source->getNexthopId() == nextHopId)
        {
            // Notify DUAL about event
            eigrpDual->processEvent(EigrpDual::NEIGHBOR_DOWN, source, false);
        }
    }

    delete neigh;
}

/**
 * Returns EIGRP interface (enabled or disabled) or NULL.
 */
EigrpInterface *EigrpIpv4Pdm::getInterfaceById(int ifaceId)
{
    EigrpInterface *iface;

    if ((iface = eigrpIft->findInterfaceById(ifaceId)) != NULL)
        return iface;
    else
        return eigrpIftDisabled->findInterface(ifaceId);
}

void EigrpIpv4Pdm::processIfaceStateChange(InterfaceEntry *iface)
{
    EigrpInterface *eigrpIface;
    int ifaceId = iface->getInterfaceId();
    // Get address and mask of interface
    IPv4Address ifMask = iface->ipv4Data()->getNetmask();
    IPv4Address ifAddress = iface->ipv4Data()->getIPAddress().doAnd(ifMask);
    int networkId;

    if (iface->isUp())
    { // an interface go up
        if (routingForNetworks->isInterfaceIncluded(ifAddress, ifMask, &networkId))
        { // Interface is included in EIGRP
            if ((eigrpIface = getInterfaceById(ifaceId)) == NULL)
            { // Create EIGRP interface
                eigrpIface = new EigrpInterface(iface, networkId, false);
            }
            if (!eigrpIface->isEnabled())
                enableInterface(iface, eigrpIface, ifAddress, ifMask, networkId);
        }
    }
    else if (!iface->isDown() || !iface->hasCarrier())
    { // an interface go dowdn
        eigrpIface = this->eigrpIft->findInterfaceById(ifaceId);

        if (eigrpIface != NULL && eigrpIface->isEnabled())
        {
            disableInterface(eigrpIface, ifMask);
        }
    }
}

void EigrpIpv4Pdm::processIfaceConfigChange(InterfaceEntry *iface, EigrpInterface *eigrpIface)
{
    EigrpRouteSource<IPv4Address> *source;
    int routeCount = eigrpTt->getNumRoutes();
    int ifaceId = eigrpIface->getInterfaceId();
    EigrpMetricPar ifParam = eigrpMetric->getParam(eigrpIface, iface);
    EigrpMetricPar newParam;
    uint32_t metric;

    // Update routes through the interface
    for (int i = 0; i < routeCount; i++)
    {
        source = eigrpTt->getRoute(i);
        if (source->getIfaceId() == ifaceId)
        {
            // Update metric of source
            if (source->getNexthopId() == EigrpRouteSource<IPv4Address>::CONNECTED_ID)
            { // connected route
                source->setMetricParams(ifParam);
                metric = eigrpMetric->computeMetric(ifParam, this->kValues);
                source->setMetric(metric);
            }
            else
            {
                newParam = eigrpMetric->adjustParam(ifParam, source->getRdParams());
                source->setMetricParams(newParam);
                metric = eigrpMetric->computeMetric(newParam, this->kValues);
                source->setMetric(metric);
            }

            // Notify DUAL about event
            eigrpDual->processEvent(EigrpDual::RECV_UPDATE, source, false);
        }
    }
}

void EigrpIpv4Pdm::disableInterface(EigrpInterface *eigrpIface, IPv4Address& ifMask)
{
    EigrpTimer* hellot;
    EigrpNeighbor<IPv4Address> *neigh;
    int neighCount;
    int ifaceId = eigrpIface->getInterfaceId();;

    EV << "EIGRP disabled on interface " << ifaceId << endl;

    // Move interface to EIGRP interface table
    eigrpIft->removeInterface(eigrpIface);
    eigrpIftDisabled->addInterface(eigrpIface);

    eigrpIface->setEnabling(false);

    // stop hello timer
    hellot = eigrpIface->getHelloTimer();
    cancelEvent(hellot);

    // Delete all neighbors on the interface
    neighCount = eigrpNt->getNumNeighbors();
    for (int i = 0; i < neighCount; i++)
    {
        neigh = eigrpNt->getNeighbor(i);
        if (neigh->getIfaceId() == ifaceId)
        {
            removeNeighbor(neigh, ifMask);
        }
    }
}

void EigrpIpv4Pdm::enableInterface(InterfaceEntry *iface, EigrpInterface *eigrpIface, IPv4Address& ifAddress, IPv4Address& ifMask, int networkId)
{
    EigrpTimer* hellot;
    int ifaceId = eigrpIface->getInterfaceId();
    EigrpRouteSource<IPv4Address> *src;
    EigrpMetricPar metricPar;
    bool newSourceResult;

    EV << "EIGRP enabled on interface " << ifaceId << endl;

    // Remove interface from EIGRP interface table
    eigrpIftDisabled->removeInterface(eigrpIface);
    eigrpIft->addInterface(eigrpIface);

    eigrpIface->setEnabling(true);

    eigrpIface->setNetworkId(networkId);

    // Start Hello timer on interface
    if ((hellot = eigrpIface->getHelloTimer()) == NULL)
    {
        hellot = createTimer(EIGRP_HELLO_TIMER, eigrpIface);
        eigrpIface->setHelloTimerPtr(hellot);
    }
    scheduleAt(simTime() + uniform(0, 5), hellot);

    // Create route
    src = eigrpTt->findOrCreateRoute(ifAddress, ifMask, ifaceId, EigrpRouteSource<IPv4Address>::CONNECTED_ID, newSourceResult);

    metricPar = eigrpMetric->getParam(eigrpIface, iface);
    // Compute metric
    src->setMetricParams(metricPar);
    uint32_t metric = eigrpMetric->computeMetric(metricPar, this->kValues);
    src->setMetric(metric);

    // Notify DUAL about event
    eigrpDual->processEvent(EigrpDual::NEW_NETWORK, src, true);
}

EigrpInterface *EigrpIpv4Pdm::addInterfaceToEigrp(int ifaceId, int networkId, bool enabled)
{
    InterfaceEntry *iface = ift->getInterfaceById(ifaceId);
    IPv4Address ifAddress, ifMask;
    // create EIGRP interface and hello timer
    EigrpInterface *eigrpIface = new EigrpInterface(iface, networkId, enabled);

    if (enabled)
    {
        // Get address and mask of interface
        ifMask = iface->ipv4Data()->getNetmask();
        ifAddress = iface->ipv4Data()->getIPAddress().doAnd(ifMask);

        enableInterface(iface, eigrpIface, ifAddress, ifMask, networkId);
    }
    else
    {
        eigrpIftDisabled->addInterface(eigrpIface);
    }

    return eigrpIface;
}

//
// interface IEigrpModule
//

EigrpNetwork *EigrpIpv4Pdm::addNetwork(IPv4Address address, IPv4Address mask)
{
    // TODO duplicity control
    return routingForNetworks->addNetwork(address, mask);
}

void EigrpIpv4Pdm::setHelloInt(int interval, int ifaceId)
{
    EigrpInterface *iface = getInterfaceById(ifaceId);

    if (iface == NULL)
    {
        iface = addInterfaceToEigrp(ifaceId, EigrpNetworkTable::UNSPEC_NETID, false);
    }
    iface->setHelloInt(interval);
}

void EigrpIpv4Pdm::setHoldInt(int interval, int ifaceId)
{
    EigrpInterface *iface = getInterfaceById(ifaceId);

    if (iface == NULL)
    { iface = addInterfaceToEigrp(ifaceId, EigrpNetworkTable::UNSPEC_NETID, false); }
    iface->setHoldInt(interval);
}

//
// interface IEigrpPdm
//

void EigrpIpv4Pdm::addRouteToRT(EigrpRouteSource<IPv4Address> *successor)
{
    ANSAIPv4Route *rtEntry = new ANSAIPv4Route();
    int ifaceId = successor->getIfaceId();
    EigrpRoute<IPv4Address> *route = successor->getRouteInfo();

    rtEntry->setDestination(route->getRouteAddress());
    rtEntry->setNetmask(route->getRouteMask());
    rtEntry->setInterface(this->ift->getInterfaceById(ifaceId));
    rtEntry->setGateway(successor->getNextHop());
    rtEntry->setMetric(successor->getMetric());

    if (successor->isInternal())
    {
        // Set any source except IFACENETMASK and MANUAL
        rtEntry->setSource(IPv4Route::ZEBRA);
        // Set right protocol source
        rtEntry->setRoutingProtocolSource(ANSAIPv4Route::pEIGRP);
        rtEntry->setAdminDist(ANSAIPv4Route::dEIGRPInternal);
    }

    if (rt->prepareForAddRoute(rtEntry))
    {
        EV << "Add EIGRP route " << route->getRouteAddress() << " to RT" << endl;
        this->eigrpRtRoutes.push_back(rtEntry);
        rt->addRoute(rtEntry);
    }
    else
    {
        delete rtEntry;
        EV << "EIGRP route " << route->getRouteAddress() << " can not be added to RT" << endl;
    }
}

void EigrpIpv4Pdm::updateRouteInRT(EigrpRouteSource<IPv4Address> *source)
{
    RouteVector::iterator it;
    EigrpRoute<IPv4Address> *route = source->getRouteInfo();

    if ((it = findRouteInRT(source, route)) != eigrpRtRoutes.end())
    {
        EV << "DUAL: update route " << route->getRouteAddress() << " in RT" << endl;
        (*it)->setMetric(route->getDij());
    }
    else
    {
#ifdef DEBUG
        // For example there is connected route in RT with lower AD
        EV << "DUAL: route "<< route->getRouteAddress() << " can not be updated, not found in RT" << endl;
#endif
    }
}

void EigrpIpv4Pdm::removeRouteFromRT(EigrpRouteSource<IPv4Address> *source)
{
    EigrpRoute<IPv4Address> *route = source->getRouteInfo();
    RouteVector::iterator it;

    if ((it = findRouteInRT(source, route)) != eigrpRtRoutes.end())
    {
        EV << "DUAL: delete route " << route->getRouteAddress() << " from RT" << endl;
        this->eigrpRtRoutes.erase(it);
        rt->deleteRoute(*it);
    }
    else
    {
#ifdef DEBUG
        EV << "DUAL: route "<< route->getRouteAddress() << " can not be removed, not found in RT" << endl;
#endif
    }
}

EigrpIpv4Pdm::RouteVector::iterator EigrpIpv4Pdm::findRouteInRT(EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route)
{
    // kdyby nekdo pomoci prepareForAddRoute smazal moji cestu, tak
    // neprijmu udalost informujici o smazani (pouziva se deleteRouteSilent). Takze bych mohl mit vector s odkazy na
    // kusy pameti, ktere mi nepatri -> problem.

    RouteVector::iterator it;

    ANSAIPv4Route *rtEntry;
    IPv4Address routeAddr = route->getRouteAddress();
    IPv4Address routeMask = route->getRouteMask();
    IPv4Address nextHop = source->getNextHop();

    for (it = eigrpRtRoutes.begin(); it < eigrpRtRoutes.end(); it++)
    {
        rtEntry = *it;
        if (rtEntry->getDestination() == routeAddr && rtEntry->getNetmask() == routeMask && rtEntry->getGateway() == nextHop)
            return it;
    }

    return it;
}

/**
 * @param destiation If dest is 0.0.0.0, then send message to all neighbors.
 */
void EigrpIpv4Pdm::sendUpdate(EigrpRoute<IPv4Address> *route, int destNeighbor, EigrpRouteSource<IPv4Address> *source)
{
    EigrpIpv4Update *updateMsg;

    // Create message
    updateMsg = new EigrpIpv4Update("EIGRPUpdate");
    addMessageHeader(updateMsg, EIGRP_UPDATE_MSG);

    EigrpIpv4Internal routeTlv;
    routeTlv.destAddress = route->getRouteAddress();
    routeTlv.destMask = route->getRouteMask();
    routeTlv.nextHop = IPv4Address::UNSPECIFIED_ADDRESS;
    // TODO
    if (source != NULL)
        routeTlv.metric = source->getMetricParams();
    else
    { // Unreachable route
        routeTlv.metric.delay = UINT32_MAX;
        EigrpNeighbor<IPv4Address> *neigh = eigrpNt->findNeighborById(destNeighbor);
        routeTlv.metric.mtu = ift->getInterfaceById(neigh->getIfaceId())->getMTU();
    }

    updateMsg->setInterRoutesArraySize(1);
    updateMsg->setInterRoutes(0, routeTlv);

    // Send message
    if (destNeighbor == IEigrpPdm::UNSPEC_RECEIVER)
    { // Send message to all neighbors
        sendMsgToAllNeighbors(updateMsg);
        EV << "Send Update message about route " << route->getRouteAddress() << " to all neighbors" << endl;
    }
    else
    { // Send message to the given address
        EigrpNeighbor<IPv4Address> *neigh = eigrpNt->findNeighborById(destNeighbor);

        addCtrInfo(updateMsg, neigh->getIfaceId(), neigh->getIPAddress());
        send(updateMsg, PDM_OUTPUT_GW);
        EV << "Send Update message about route " << route->getRouteAddress() << " to " << neigh->getIPAddress() << endl;
    }
}

/**
 * @param destiation If dest is 0.0.0.0, then send message to all neighbors.
 */
void EigrpIpv4Pdm::sendQuery(EigrpRoute<IPv4Address> *route, int destNeighbor, EigrpRouteSource<IPv4Address> *source)
{
    EigrpIpv4Query *queryMsg;
    EigrpNeighbor<IPv4Address> *neigh = eigrpNt->findNeighborById(destNeighbor);

    // Create message
    queryMsg = new EigrpIpv4Query("EIGRPQuery");
    addMessageHeader(queryMsg, EIGRP_QUERY_MSG);

    EigrpIpv4Internal routeTlv;
    routeTlv.destAddress = route->getRouteAddress();
    routeTlv.destMask = route->getRouteMask();
    routeTlv.nextHop = IPv4Address::UNSPECIFIED_ADDRESS;
    routeTlv.metric = route->getRdPar();

    queryMsg->setInterRoutesArraySize(1);
    queryMsg->setInterRoutes(0, routeTlv);

    // Send message
    if (destNeighbor == IEigrpPdm::UNSPEC_RECEIVER)
    { // Send message to all neighbors
        sendQueryToAllNeighbors(queryMsg, source->getNexthopId());
        EV << "Send Query message about route " << route->getRouteAddress() << " to all neighbors" << endl;
    }
    else
    { // Send message to the given address
        addCtrInfo(queryMsg, neigh->getIfaceId(), neigh->getIPAddress());
        send(queryMsg, PDM_OUTPUT_GW);
        EV << "Send Query message about route " << route->getRouteAddress() << " to " << neigh->getIPAddress() << endl;
    }
}

void EigrpIpv4Pdm::sendReply(EigrpRoute<IPv4Address> *route, int destNeighbor, EigrpRouteSource<IPv4Address> *source, bool isUnreachable)
{
    EigrpNeighbor<IPv4Address> *neigh = eigrpNt->findNeighborById(destNeighbor);
    int ifaceId = neigh->getIfaceId();
    EigrpIpv4Reply *replyMsg = createReplyMsg(ifaceId, neigh->getIPAddress());

    EigrpIpv4Internal routeTlv;
    routeTlv.destAddress = route->getRouteAddress();
    routeTlv.destMask = route->getRouteMask();
    routeTlv.nextHop = IPv4Address::UNSPECIFIED_ADDRESS;
    routeTlv.metric = route->getRdPar();
    if (isUnreachable)
        routeTlv.metric.delay = UINT32_MAX;

    replyMsg->setInterRoutesArraySize(1);
    replyMsg->setInterRoutes(0, routeTlv);

    send(replyMsg, PDM_OUTPUT_GW);
    EV << "Send Reply message about route " << route->getRouteAddress() << " to " << neigh->getIPAddress() << endl;
}

void EigrpIpv4Pdm::removeSourceFromTT(EigrpRouteSource<IPv4Address> *source)
{
    EV << "DUAL: remove route " << source->getRouteInfo()->getRouteAddress();
    EV << " via " << source->getNextHop() << " from TT" << endl;

    eigrpTt->removeRoute(source);
    delete source;
}

void EigrpIpv4Pdm::addSourceToTT(EigrpRouteSource<IPv4Address> *source)
{
    eigrpTt->addRoute(source);
}

int EigrpIpv4Pdm::getNumPeers()
{
    return eigrpNt->getNumNeighbors();
}

uint32_t EigrpIpv4Pdm::findRouteDMin(EigrpRoute<IPv4Address> *route)
{
    return eigrpTt->findRouteDMin(route);
}

EigrpRouteSource<IPv4Address> * EigrpIpv4Pdm::findSuccessor(EigrpRoute<IPv4Address> *route, uint32_t dmin)
{
    return eigrpTt->findSuccessor(route, dmin);
}

bool EigrpIpv4Pdm::hasFeasibleSuccessor(EigrpRoute<IPv4Address> *route, uint32_t& resultDmin)
{
    return eigrpTt->hasFeasibleSuccessor(route, resultDmin);
}

bool EigrpIpv4Pdm::checkFeasibleSuccessor(EigrpRoute<IPv4Address> *route)
{
    return eigrpTt->checkFeasibleSuccessor(route);
}

EigrpRouteSource<IPv4Address> *EigrpIpv4Pdm::getFirstSuccessor(EigrpRoute<IPv4Address> *route)
{
    return eigrpTt->getFirstSuccessor(route);
}
