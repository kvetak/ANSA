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
* @file BabelMain.cc
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
* @brief Babel Main module
* @detail Represents Babel routing process
*/

#include "ansa/routing/babel/BabelMain.h"
//#include "InterfaceTableAccess.h"
#include "ansa/routing/babel/BabelDeviceConfigurator.h"
//#include "IPv6InterfaceData.h"
#include "inet/networklayer/ipv6/IPv6InterfaceData.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"

#include "inet/transportlayer/contract/udp/UDPControlInfo.h"
#include <cmath>

#include "inet/common/NotifierConsts.h"
#include "inet/common/ModuleAccess.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#include "winsock2.h"  // htonl, ntohl, ...
#else
#include <netinet/in.h>  // htonl, ntohl, ...
#endif
namespace inet {
using namespace Babel;

Define_Module(BabelMain);

void BabelMain::initialize(int stage)
{
    /*
    if(stage != 3)
    {
        return;
    }
    */
    cSimpleModule::initialize(stage);

#ifdef BABEL_DEBUG
    EV << "BabelMain starting initialization..." << endl;
#endif

    //generate random seqno
    seqno = intuniform(0,UINT16_MAX);
#ifdef BABEL_DEBUG
    EV << "Generated sequence number: " << seqno << endl;
#endif

    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        host = getParentModule()->getParentModule();
        //set main interface
        //ift = InterfaceTableAccess().get();
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        setMainInterface();

        socket4mcast = createSocket();
        socket4mcast->setMulticastLoop(false);
        socket4mcast->bind(defval::MCASTG4, port);
        socket6mcast = createSocket();
        socket6mcast->setMulticastLoop(false);
        socket6mcast->bind(defval::MCASTG6, port);

        //rt4 = AnsaRoutingTableAccess().get();
        //rt6 = ANSARoutingTable6Access().get();
        rt4 = check_and_cast<IPv4RoutingTable*>(getModuleByPath(par("routingTableModule"))->getSubmodule("ipv4"));
        rt6 = check_and_cast<IPv6RoutingTable*>(getModuleByPath(par("routingTableModule"))->getSubmodule("ipv6"));

        //bit = BabelInterfaceTableAccess().get();
        //bsend = BabelSenderAccess().get();

        //BabelDeviceConfigurator *conf = ModuleAccess<BabelDeviceConfigurator>("babelDeviceConfigurator").get();
        BabelDeviceConfigurator *conf = new BabelDeviceConfigurator(par("deviceId"),par("deviceType"),par("configFile"), ift);
        conf->loadBabelConfig(this);

        WATCH(routerId);
        WATCH(seqno);
        WATCH_PTRVECTOR(buffers);
        WATCH_PTRVECTOR(ackwait);
        WATCH_PTRVECTOR(bit.getInterfaces());
        WATCH_PTRVECTOR(bnt.getNeighbours());
        WATCH_PTRVECTOR(btt.getRoutes());
        WATCH_PTRVECTOR(bst.getSources());
        WATCH_PTRVECTOR(bpsrt.getRequests());


        for (auto k = bit.getInterfaces().begin(); k != bit.getInterfaces().end(); ++k) {
                InterfaceEntry *ie = (*k)->getInterface();
            if (ie->isMulticast() && !ie->isLoopback()) {
#ifdef BABEL_DEBUG
    EV << "Generated sequence number: " << ie->getFullName() << endl;
#endif
                socket4mcast->joinMulticastGroup(defval::MCASTG4, ie->getInterfaceId());
                socket6mcast->joinMulticastGroup(defval::MCASTG6, ie->getInterfaceId());
            }
        }


        buffgc = createTimer(timerT::BUFFERGC, NULL, NULL, false);
        resetTimer(buffgc, defval::BUFFER_GC_INTERVAL);

        //nb = NotificationBoardAccess().get();
        host->subscribe(NF_INTERFACE_STATE_CHANGED, this);
    }

}

void BabelMain::finish()
{
    EV << this->getParentModule()->getParentModule()->getFullName() << " Babel routing:" << endl;
#ifdef BABEL_DEBUG
    EV << bit.printStats();
#endif
    for (std::vector<BabelInterface *>::iterator it = bit.getInterfaces().begin(); it != bit.getInterfaces().end(); ++it)
    {
        if((*it)->getAfSend() != AF::NONE)
        {
            recordStatistic(&((*it)->rxStat.messages));
            recordStatistic(&((*it)->txStat.messages));

            for(int i = tlvT::PAD1; i <= tlvT::SEQNOREQ; ++i)
            {
                recordStatistic(&((*it)->rxStat.tlv[i]));
                recordStatistic(&((*it)->txStat.tlv[i]));
            }
        }
    }

}

void BabelMain::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        processTimer(msg);
    }
    else if (msg->getKind() == UDP_I_DATA)
    {
        cPacket *pk = PK(msg);

        UDPDataIndication *ctrl = check_and_cast<UDPDataIndication *>(pk->getControlInfo());
        InterfaceEntry *iface = ift->getInterfaceById(ctrl->getInterfaceId());
        if(iface == NULL)
        {
            throw cRuntimeError("Received UDP datagram on unknown interface");
        }
        BabelInterface *biface = bit.findInterfaceById(iface->getInterfaceId());

        //InterfaceEntry::State status = ift->getInterfaceById(ctrl->getInterfaceId())->getState();
        if((biface != NULL && !biface->getEnabled()) || !iface->isUp())
        {// message received on DOWN iface -> ignore
#ifdef BABEL_DEBUG
            EV << "Received message on DOWN interface - message ignored" << endl;
#endif
            delete msg;
            return;
        }

        // process incoming packet
        EV << "Received packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;

        BabelMessage *bm = check_and_cast<BabelMessage *>(pk);

        if(bm->getMagic() == defval::MAGIC && bm->getVersion() == defval::VERSION)
        {// correct magic and version fields -> process message
            processMessage(bm);
        }
        delete pk;

    }
    else if (msg->getKind() == UDP_I_ERROR)
    {
        EV << "Ignoring UDP error report\n";
        delete msg;
    }
    else
    {
        error("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }
}
/*
void BabelMain::receiveChangeNotification(int category, const cObject *details)
{
    // ignore notifications during initialization
    if(simulation.getContextType() == CTX_INITIALIZE)
        return;

    Enter_Method_Silent();
    printNotificationBanner(category, details);

    if(category == NF_INTERFACE_STATE_CHANGED)
    {
        InterfaceEntry *iface = check_and_cast<InterfaceEntry*>(details);
        BabelInterface *biface = bit.findInterfaceById(iface->getInterfaceId());


        if(iface->isUp())
        { // an interface goes up
            if(biface != NULL)
            {
                activateInterface(biface);
            }
        }
        else if(!iface->isUp() || !iface->hasCarrier())
        { // an interface goes down
            if(biface != NULL)
            {
                deactivateInterface(biface);
            }
        }
    }
}
*/

void BabelMain::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj DETAILS_ARG)
{
    Enter_Method_Silent("BabelRouting::receiveChangeNotification(%s)", notificationCategoryName(signalID));

    const InterfaceEntryChangeDetails *change = nullptr;
    if(signalID == NF_INTERFACE_STATE_CHANGED)
    {
        change = check_and_cast<const InterfaceEntryChangeDetails *>(obj);
        InterfaceEntry *iface = change->getInterfaceEntry();
        BabelInterface *biface = bit.findInterfaceById(iface->getInterfaceId());

        if(iface->isUp())
        { // an interface goes up
            if(biface != NULL)
            {
                activateInterface(biface);
            }
        }
        else if(!(iface->isUp()) || !iface->hasCarrier())
        { // an interface goes down
            if(biface != NULL)
            {
                deactivateInterface(biface);
            }
        }
    }
}
/**
 * Process received timer
 *
 * @param   timer   Received timer
 */
void BabelMain::processTimer(BabelTimer *timer)
{
#ifdef BABEL_DEBUG
    EV << "Received timer " << timerT::toStr(timer->getKind()) << endl;
#endif
    BabelInterface *iface = NULL;

    switch(timer->getKind())
    {
    case timerT::HELLO:
        sendHelloTLV(check_and_cast<BabelInterface *>(static_cast<cObject *>(timer->getContextPointer())));
        break;
    case timerT::UPDATE:
        iface = check_and_cast<BabelInterface *>(static_cast<cObject *>(timer->getContextPointer()));
        sendFullDump(iface);
        iface->resetUTimer();
        break;
    case timerT::BUFFER:
        flushBuffer(static_cast<BabelBuffer *>(timer->getContextPointer()));
        break;
    case timerT::BUFFERGC:
        deleteUnusedBuffers();
        resetTimer(buffgc, defval::BUFFER_GC_INTERVAL);
        break;
    case timerT::TOACKRESEND:
        checkAndResendToAck(static_cast<BabelToAck *>(timer->getContextPointer()));
        break;
    case timerT::NEIGHHELLO:
        processNeighHelloTimer(static_cast<BabelNeighbour *>(timer->getContextPointer()));
        break;
    case timerT::NEIGHIHU:
        processNeighIhuTimer(static_cast<BabelNeighbour *>(timer->getContextPointer()));
        break;
    case timerT::ROUTEEXPIRY:
        processRouteExpiryTimer(static_cast<BabelRoute *>(timer->getContextPointer()));
        break;
    case timerT::ROUTEBEFEXPIRY:
        processBefRouteExpiryTimer(static_cast<BabelRoute *>(timer->getContextPointer()));
        break;
    case timerT::SOURCEGC:
        processSourceGCTimer(static_cast<BabelSource *>(timer->getContextPointer()));
        break;
    case timerT::SRRESEND:
        processRSResendTimer(static_cast<BabelPenSR *>(timer->getContextPointer()));
        break;
    default:
        throw cRuntimeError("Received timer of unknown type: %d", timer->getKind());
    }
}

/**
 * Send Hello (and IHU) TLV
 *
 * @param   iface   Outgoing interface
 * @param   mt      Maximal buffering time
 *
 * @note    Every third interval sends IHU TLV
 * @note    If iface is NULL, sends TLV on all enabled interfaces
 */
void BabelMain::sendHelloTLV(BabelInterface *iface, double mt)
{
    if(iface == NULL)
    {// send on all interfaces
       for(std::vector<BabelInterface *>::iterator it = bit.getInterfaces().begin(); it != bit.getInterfaces().end(); ++it)
       {// for all interfaces
           sendHelloTLV((*it), mt);
       }
       return;
    }

    uint16_t nexthseqno = iface->getIncHSeqno();

    sendTLV(iface, new BabelHelloFtlv(nexthseqno, iface->getHInterval()), mt);

    //send IHU TLV
    if(nexthseqno % defval::IHU_INTERVAL_MULT == 0)
    {// send IHU every IHU_INTERVAL_MULT hello timer
        for(std::vector<BabelNeighbour *>::iterator it = bnt.getNeighbours().begin(); it != bnt.getNeighbours().end(); ++it)
        {// through all neighbours
            if((*it)->getInterface() == iface)
            {// neighbour on same interface -> send IHU TLV
                sendTLV((((*it)->getAddress().getType()==L3Address::IPv6) ? defval::MCASTG6 : defval::MCASTG4),
                        iface,
                        new BabelIhuFtlv((*it)->computeRxcost(),
                            defval::IHU_INTERVAL_MULT * iface->getHInterval(),
                            (*it)->getAddress()), mt);
            }
        }
    }
    else
    {// on lossy links send frequently
        for(std::vector<BabelNeighbour *>::iterator it = bnt.getNeighbours().begin(); it != bnt.getNeighbours().end(); ++it)
        {// through all neighbours
            if((*it)->getInterface() == iface
                    && ((((*it)->getHistory() & 0xF000) != 0xF000) || (*it)->getTxcost() >= 384))
            {// neighbour on same interface with loss in last 4 hellos or with txcost >= 384 -> send IHU TLV
                sendTLV((((*it)->getAddress().getType()==L3Address::IPv6) ? defval::MCASTG6 : defval::MCASTG4),
                        iface,
                        new BabelIhuFtlv((*it)->computeRxcost(),
                            defval::IHU_INTERVAL_MULT * iface->getHInterval(),
                            (*it)->getAddress()), mt);
            }
        }
    }

    iface->resetHTimer();
}

/**
 * Send Update TLV
 *
 * @param   da      Destination address
 * @param   iface   Outgoing interface
 * @param   update  Update FTLV
 * @param   mt      Maximal buffering time
 * @param   reliably    Send reliably?
 *
 * @note If iface is NULL, sends TLV on all enabled interfaces
 */
void BabelMain::sendUpdateTLV(L3Address da, BabelInterface *iface, BabelUpdateFtlv *update, double mt, bool reliably)
{
    if(iface == NULL)
    {// send on all interfaces
       for(std::vector<BabelInterface *>::iterator it = bit.getInterfaces().begin(); it != bit.getInterfaces().end(); ++it)
       {// for all interfaces
           if((*it)->getEnabled() && (*it)->getAfSend() != AF::NONE)
           {// enabled and no passive
               sendUpdateTLV(da, (*it), update->dup(), mt, reliably);
           }
       }
       delete update;
       return;
    }

    int sendnum = 1;

    if(reliably)
    {
        if(bnt.getNumOfNeighOnIface(iface) > 3)
        {// many neighbours -> send multiple times
            if(iface->getWired())
            {
                sendnum = 2;
            }
            else
            {
                sendnum = 5;
            }
        }
        else
        {// a few neighbours -> use ACK REQ
            sendnum = USE_ACK;
        }
    }

    update->setSendNum(sendnum);


    if(update->getDistance().getMetric() != 0xFFFF)
    {//not retraction -> update FD in source table
        addOrupdateSource(update->getPrefix(), update->getRouterId(), update->getDistance());
    }

    sendTLV(da, iface, update, mt);
}

/**
 * Send Update TLV to multicast
 *
 * @param   iface   Outgoing interface
 * @param   update  Update FTLV
 * @param   mt      Maximal buffering time
 * @param   reliably    Send reliably?
 *
 * @note    Sends to multicast
 * @note    If iface is NULL, sends TLV on all enabled interfaces
 */
void BabelMain::sendUpdateTLV(BabelInterface *iface, BabelUpdateFtlv *update, double mt, bool reliably)
{
    if(iface == NULL)
    {// send on all interfaces
       for(std::vector<BabelInterface *>::iterator it = bit.getInterfaces().begin(); it != bit.getInterfaces().end(); ++it)
       {// for all interfaces
           if((*it)->getEnabled() && (*it)->getAfSend() != AF::NONE)
           {// enabled and no passive
               sendUpdateTLV((*it), update->dup(), mt, reliably);
           }
       }
       delete update;
       return;
    }

    int ifaceaf = iface->getAfSend();

    if(ifaceaf == AF::IPv4)
    {// send IPv4
        sendUpdateTLV(defval::MCASTG4, iface, update, mt, reliably);
    }
    else if(ifaceaf == AF::IPv6)
    {// send IPv6
        sendUpdateTLV(defval::MCASTG6, iface, update, mt, reliably);
    }
    else if(ifaceaf == AF::IPvX)
    {// send IPv4 and IPv6
        sendUpdateTLV(defval::MCASTG4, iface, update->dup(), mt, reliably);
        sendUpdateTLV(defval::MCASTG6, iface, update, mt, reliably);
    }
}

/**
 * Send SeqnoReq TLV
 *
 * @param   da      Destination address
 * @param   iface   Outgoing interface
 * @param   request SeqnoReq FTLV
 * @param   recfrom Neighbour from was SeqnoReq received, in case of forwarding
 * @param   mt      Maximal buffering time
 *
 * @note    If iface is NULL, sends TLV on all enabled interfaces
 */
void BabelMain::sendSeqnoReqTLV(L3Address da, BabelInterface *iface, BabelSeqnoReqFtlv *request, BabelNeighbour *recfrom, double mt)
{
    if(iface == NULL)
    {// send on all interfaces
       for(std::vector<BabelInterface *>::iterator it = bit.getInterfaces().begin(); it != bit.getInterfaces().end(); ++it)
       {// for all interfaces
           if((*it)->getEnabled() && (*it)->getAfSend() != AF::NONE)
           {// enabled and no passive
               sendSeqnoReqTLV(da, (*it), request->dup(), recfrom, mt);
           }
       }
       delete request;
       return;
    }

    BabelPenSR *intable = bpsrt.findPenSR(request->getPrefix(), iface);
    if(!intable)
    {// new -> create
        BabelPenSR *newreq = new BabelPenSR(*request, recfrom, defval::RESEND_NUM, iface, da, createTimer(timerT::SRRESEND, NULL, NULL, false));
        newreq->resetResendTimer();

        if(bpsrt.addPenSR(newreq) != newreq)
        {
            delete newreq;
        }
    }


    sendTLV(da, iface, request, mt);
}

/**
 * Send SeqnoReq TLV to multicast
 *
 * @param   da      Destination address
 * @param   iface   Outgoing interface
 * @param   request SeqnoReq FTLV
 * @param   recfrom Neighbour from was SeqnoReq received, in case of forwarding
 * @param   mt      Maximal buffering time
 *
 * @note    Sends to multicast
 * @note    If iface is NULL, sends TLV on all enabled interfaces
 */
void BabelMain::sendSeqnoReqTLV(BabelInterface *iface, BabelSeqnoReqFtlv *request, BabelNeighbour *recfrom, double mt)
{
    if(iface == NULL)
    {// send on all interfaces
       for(std::vector<BabelInterface *>::iterator it = bit.getInterfaces().begin(); it != bit.getInterfaces().end(); ++it)
       {// for all interfaces
           if((*it)->getEnabled() && (*it)->getAfSend() != AF::NONE)
           {// enabled and no passive
               sendSeqnoReqTLV((*it), request->dup(), recfrom, mt);
           }
       }
       delete request;
       return;
    }

    int ifaceaf = iface->getAfSend();

    if(ifaceaf == AF::IPv4)
    {// send IPv4
        sendSeqnoReqTLV(defval::MCASTG4, iface, request, recfrom, mt);
    }
    else if(ifaceaf == AF::IPv6)
    {// send IPv6
        sendSeqnoReqTLV(defval::MCASTG6, iface, request, recfrom, mt);
    }
    else if(ifaceaf == AF::IPvX)
    {// send IPv4 and IPv6
        sendSeqnoReqTLV(defval::MCASTG4, iface, request->dup(), recfrom, mt);
        sendSeqnoReqTLV(defval::MCASTG6, iface, request, recfrom, mt);
    }
}

/**
 * Send Update
 *
 * @param   da      Destination address
 * @param   iface   Outgoing interface
 * @param   route   Route for which send update
 * @param   mt      Maximal buffering time
 * @param   reliably    Send reliably?
 *
 * @note    If iface is NULL, sends TLV on all enabled interfaces
 */
void BabelMain::sendUpdate(L3Address da, BabelInterface *iface, BabelRoute *route, double mt, bool reliably)
{
    ASSERT(route != NULL);

    if(iface == NULL)
    {// send on all interfaces
       for(std::vector<BabelInterface *>::iterator it = bit.getInterfaces().begin(); it != bit.getInterfaces().end(); ++it)
       {// for all interfaces
           if((*it)->getEnabled() && (*it)->getAfSend() != AF::NONE)
           {// enabled and no passive
               sendUpdate(da, (*it), route, mt, reliably);
           }
       }
       return;
    }



    if(route->getNeighbour() == NULL || !iface->getSplitHorizon() ||
         (iface->getSplitHorizon() && route->getNeighbour()->getInterface() != iface))
    {// local route or disabled split-horizon or learned on another interface -> send
        L3Address nh;

        if(route->getPrefix().getAddr().getType()==L3Address::IPv6)
        {// IPv6
          nh = iface->getInterface()->ipv6Data()->getLinkLocalAddress();
        }
        else
        {// IPv4
          nh = iface->getInterface()->ipv4Data()->getIPAddress();
        }

        if(nh.isUnspecified())
        {// interface has not address of same AF
          return;
        }

        sendUpdateTLV(da, iface, new BabelUpdateFtlv(iface->getUInterval(),
             route->getPrefix(), routeDistance(route->getRDistance().getSeqno(), route->metric()),
             route->getOriginator(), nh), mt, reliably);
    }

}

/**
 * Send Update to multicast
 *
 * @param   iface   Outgoing interface
 * @param   route   Route for which send update
 * @param   mt      Maximal buffering time
 * @param   reliably    Send reliably?
 *
 * @note    If iface is NULL, sends TLV on all enabled interfaces
 */
void BabelMain::sendUpdate(BabelInterface *iface, BabelRoute *route, double mt, bool reliably)
{
    if(iface == NULL)
    {// send on all interfaces
       for(std::vector<BabelInterface *>::iterator it = bit.getInterfaces().begin(); it != bit.getInterfaces().end(); ++it)
       {// for all interfaces
           if((*it)->getEnabled() && (*it)->getAfSend() != AF::NONE)
           {// enabled and no passive
               sendUpdate((*it), route, mt, reliably);
           }
       }
       return;
    }

    int ifaceaf = iface->getAfSend();

    if(ifaceaf == AF::IPv4)
    {// send IPv4
        sendUpdate(defval::MCASTG4, iface, route, mt, reliably);
    }
    else if(ifaceaf == AF::IPv6)
    {// send IPv6
        sendUpdate(defval::MCASTG6, iface, route, mt, reliably);
    }
    else if(ifaceaf == AF::IPvX)
    {// send IPv4 and IPv6
        sendUpdate(defval::MCASTG4, iface, route, mt, reliably);
        sendUpdate(defval::MCASTG6, iface, route, mt, reliably);
    }
}

/**
 * Send Updates for all routes to multicast
 *
 * @param   iface   Outgoing interface
 *
 * @note    If iface is NULL, sends TLV on all enabled interfaces
 */
void BabelMain::sendFullDump(BabelInterface *iface)
{
    if(iface == NULL)
    {// send on all interfaces
       for(std::vector<BabelInterface *>::iterator it = bit.getInterfaces().begin(); it != bit.getInterfaces().end(); ++it)
       {// for all interfaces
           if((*it)->getEnabled() && (*it)->getAfSend() != AF::NONE)
           {// enabled and no passive
               sendFullDump((*it));
           }
       }
       return;
    }


    for(std::vector<BabelRoute *>::iterator it = btt.getRoutes().begin(); it != btt.getRoutes().end(); ++it)
    {
       if((*it)->getSelected())
       {// route is selected -> send update
           sendUpdate(iface, (*it));

       }
    }
}

/**
 * Process end of neighbour hello interval
 *
 * @param   neigh   Neighbour
 */
void BabelMain::processNeighHelloTimer(BabelNeighbour *neigh)
{
    ASSERT(neigh != NULL);

    neigh->noteLoss();
    neigh->setExpHSeqno(plusmod16(neigh->getExpHSeqno(), 1));


    bool changed = false;
    if(neigh->getHistory() == 0)
    {// in whole history did not received -> delete neighbour

        EV  << "Neighbour " << neigh->getAddress() << " on " << neigh->getInterface()->getIfaceName() << " lost" << endl;
        bpsrt.removePenSRsByNeigh(neigh);
        changed = removeRoutesByNeigh(neigh) || changed;
        bnt.removeNeighbour(neigh);
    }
    else
    {
        neigh->resetNHTimer();

        if(neigh->recomputeCost())
        {// cost has changed
            changed = true;
        }
    }

    if(changed)
    {
        selectRoutes();
    }
}

/**
 * Process end of neighbour IHU interval
 *
 * @param   neigh   Neighbour
 */
void BabelMain::processNeighIhuTimer(BabelNeighbour *neigh)
{
    ASSERT(neigh != NULL);

    neigh->setTxcost(COST_INF);

    if(neigh->recomputeCost())
    {// cost has changed
        selectRoutes();
    }
}

/**
 * Process expiration of route
 *
 * @param   route   Expiring route
 */
void BabelMain::processRouteExpiryTimer(BabelRoute *route)
{
    ASSERT(route != NULL);

    if(route->metric() != 0xFFFF)
    {
        route->getRDistance().setMetric(0xFFFF);
        route->resetETimer();

        //send retraction
        sendUpdate(NULL, route);

        selectRoutes();
    }
    else
    {
        if(route->getRTEntry())
        {// route is installed in RT
            removeFromRT(route);
        }
        btt.removeRoute(route);
    }
}

/**
 * React before expiration of route
 *
 * @param   route   Early expiring route
 */
void BabelMain::processBefRouteExpiryTimer(BabelRoute *route)
{
    ASSERT(route != NULL);

    sendTLV(route->getNeighbour()->getAddress(), route->getNeighbour()->getInterface(), new BabelRouteReqFtlv(route->getPrefix()));
}

/**
 * Process expiration of source
 *
 * @param   source   Source
 */
void BabelMain::processSourceGCTimer(BabelSource *source)
{
    ASSERT(source != NULL);

#ifdef BABEL_DEBUG
    EV << "Deleting source: " << *source << endl;
#endif

    bst.removeSource(source);
}

/**
 * Process SeqnoReq resend timer
 *
 * @param   request   Request
 */
void BabelMain::processRSResendTimer(BabelPenSR *request)
{
    ASSERT(request != NULL);

    if(request->decResendNum() >= 0)
    {// resend number is not exhausted -> resend

        sendSeqnoReqTLV(request->getForwardTo(), request->getOutIface(), request->getRequest().dup(), request->getReceivedFrom(), roughly(SEND_URGENT));
        request->resetResendTimer();
    }
    else
    {// resend number is  exhausted -> delete
        bpsrt.removePenSR(request);
    }
}




BabelMain::~BabelMain()
{
    deleteBuffers();
    deleteToAcks();
    delete socket4mcast;
    delete socket6mcast;
    Babel::deleteTimer(&buffgc);
    deleteTimers(); //should be done last
}

/**
 * Increment sequence number
 */
void BabelMain::incSeqno()
{
    seqno = plusmod16(seqno, 1);

    //set in local routes
    for(std::vector<BabelRoute *>::iterator it = btt.getRoutes().begin(); it != btt.getRoutes().end(); ++it)
    {
        if((*it)->getNeighbour() == NULL)
        {
            (*it)->getRDistance().setSeqno(seqno);
        }
    }
}

/**
 * Automatically choose interface (gets first interface) and set it as main
 *
 * Main interface is used for generating router-id and for binding multicast UDP sockets
 */
void BabelMain::setMainInterface()
{
    ASSERT(ift != NULL);

    if(ift->getNumInterfaces() <= 0)
    {
        throw cRuntimeError("Router has't any interface");
    }

    mainInterface = ift->getInterface(0);

    if(mainInterface == NULL)
    {
        throw cRuntimeError("Can't set main interface");
    }

#ifdef BABEL_DEBUG
    EV << "Interface " << mainInterface->getName() << " (" << mainInterface->getMacAddress() << ")" << " set as main"<< endl;
#endif
}

/**
 * Generates router-ID
 *
 * If main interface is set, uses MAC address to generate EUI-64 format, otherwise generates random number
 */
void BabelMain::generateRouterId()
{
    if(mainInterface != NULL)
    {// have main interface -> use eui64
        routerId.setRid(mainInterface->getInterfaceToken().normal(), mainInterface->getInterfaceToken().low());
    }
    else
    {// no main interface -> generate random number
        routerId.setRid(uniform(0,UINT32_MAX),uniform(0,UINT32_MAX));
    }

#ifdef BABEL_DEBUG
    EV << "Generated router-id: " << routerId << endl;
#endif
}

void BabelMain::setRouterId(uint32_t h, uint32_t l)
{
    routerId.setRid(h, l);
}

/**
 * Create timer
 *
 * @param   kind    Timer category
 * @param   context Context of timer
 * @param   suffix  Suffix of timer name
 * @param   autodelete  Should babelMain automatically delete timer?
 */
BabelTimer *BabelMain::createTimer(short kind, void *context, const char *suffix, bool autodelete)
{
    std::string timerstr = timerT::toStr(kind);
    if(suffix)
    {
        timerstr += "-";
        timerstr += suffix;
    }

    BabelTimer *timer = new BabelTimer(timerstr.c_str(), kind);
    timer->setContextPointer(context);

    if(autodelete)
    {// save pointer for managed deletion
        timers.push_back(timer);
    }

    return timer;
}

/**
 * Delete maintained timer
 *
 * @param   todel   Timer to delete
 */
void BabelMain::deleteTimer(BabelTimer *todel)
{
    std::vector<BabelTimer *>::iterator it;

    if(todel == NULL)
    {// nothing to delete
        return;
    }

    for (it = timers.begin(); it != timers.end();)
    {// find timer
        if((*it) == todel)
        {// found -> cancel and delete
            cancelAndDelete(*it);
            it = timers.erase(it);
            return;
        }
        else
        {// do not delete -> get next
            ++it;
        }
    }
}

/**
 * Delete all maintained timers
 */
void BabelMain::deleteTimers()
{
    std::vector<BabelTimer *>::iterator it;

    for (it = timers.begin(); it != timers.end(); ++it)
    {// cancel and delete all timers
        cancelAndDelete(*it);
    }
    timers.clear();
}

UDPSocket *BabelMain::createSocket()
{
    UDPSocket *socket = new UDPSocket();
    socket->setOutputGate(gate("udpOut"));
    socket->setTimeToLive(1);

    return socket;
}

/**
 * Activate interface for babel routing
 *
 * @param   iface   Interface to activation
 */
void BabelMain::activateInterface(BabelInterface *iface)
{
    ASSERT(iface != NULL);

    if(!iface->getInterface()->isUp())
    {// interface is down
        return;
    }

    //create UDP sockets
    switch(iface->getAfSend())
    {
    case AF::IPvX:
        //IPv4
        if(iface->getSocket4() == NULL)
        {
            iface->setSocket4(createSocket());
        }
        iface->getSocket4()->bind(iface->getInterface()->ipv4Data()->getIPAddress(), port);

        //IPv6
        if(iface->getSocket6() == NULL)
        {
            iface->setSocket6(createSocket());
        }
        iface->getSocket6()->bind(iface->getInterface()->ipv6Data()->getLinkLocalAddress(), port);
       break;
    case AF::IPv4:
        if(iface->getSocket4() == NULL)
        {
            iface->setSocket4(createSocket());
        }
        iface->getSocket4()->bind(iface->getInterface()->ipv4Data()->getIPAddress(), port);
       break;
    case AF::IPv6:
        if(iface->getSocket6() == NULL)
        {
            iface->setSocket6(createSocket());
        }
        iface->getSocket6()->bind(iface->getInterface()->ipv6Data()->getLinkLocalAddress(), port);
       break;
    case AF::NONE:
        //nothing to do
        break;
    default:
        throw cRuntimeError("Bad address family (%d) on interface %s", iface->getAfSend(), iface->getIfaceName());
    }

    if(iface->getAfSend() != AF::NONE)
    {// no passive -> activate timers
        if(iface->getHTimer() == NULL)
        {
            iface->setHTimer(createTimer(timerT::HELLO, iface, iface->getIfaceName(), false));
        }
        iface->resetHTimer(uniform(SEND_URGENT, CStoS(iface->getHInterval())));     //randomly

        if(iface->getUTimer() == NULL)
        {
            iface->setUTimer(createTimer(timerT::UPDATE, iface, iface->getIfaceName(), false));
        }
        iface->resetUTimer(uniform(SEND_URGENT, CStoS(iface->getUInterval())));     //randomly
    }


    iface->setEnabled(true);

    bool changed = false;
    // add local prefixes to TopologyTable
    for (std::vector<netPrefix<L3Address> >::const_iterator it = iface->getDirectlyConn().begin(); it != iface->getDirectlyConn().end(); ++it)
    {// add all prefixes to tt
        if(iface->getAfDist() == AF::IPvX
                || (iface->getAfDist() == AF::IPv4 && !((*it).getAddr().getType()==L3Address::IPv6) )
                || (iface->getAfDist() == AF::IPv6 &&   (*it).getAddr().getType()==L3Address::IPv6  )
          )
        {
            BabelRoute *newroute = new BabelRoute((*it), NULL, routerId, routeDistance(seqno, 0), L3Address(), 0, NULL, NULL);

            if(btt.addRoute(newroute) != newroute)
            {// route already in table
                delete newroute;
            }
            else
            {
                changed = true;
            }
        }
    }

    if(changed)
    {
        selectRoutes();
    }


#ifdef BABEL_DEBUG
        EV << "Interface " << iface->getIfaceName() << " activated for Babel routing: " << *iface << endl;
#endif

    if(iface->getAfSend() != AF::NONE)
    {
        // say hi to others
        sendTLV(iface, new BabelHelloFtlv(iface->getIncHSeqno(), iface->getHInterval()), uniform(0.0, SEND_URGENT));

        // request for routes
        sendTLV(iface, new BabelRouteReqFtlv());
    }
}


/**
 * Deactivate interface for babel routing
 *
 * @param   iface   Interface to deactivation
 */
void BabelMain::deactivateInterface(BabelInterface *iface)
{
    ASSERT(iface != NULL);

    //close UDP sockets
    if(iface->getSocket4() != NULL)
    {
        iface->getSocket4()->close();
    }

    if(iface->getSocket6() != NULL)
    {
        iface->getSocket6()->close();
    }

    //cancel timers
    if(iface->getHTimer() != NULL)
    {
        cancelEvent(iface->getHTimer());
    }

    if(iface->getUTimer() != NULL)
    {
        cancelEvent(iface->getUTimer());
    }


    bool changed = false;
    // remove local prefixes from TopologyTable
    for (std::vector<netPrefix<L3Address> >::const_iterator it = iface->getDirectlyConn().begin(); it != iface->getDirectlyConn().end(); ++it)
    {// remove all prefixes from tt

        changed = btt.removeRoute(btt.findRoute((*it), NULL)) || changed;

    }

    changed = btt.retractRoutesOnIface(iface) || changed;


    iface->setEnabled(false);

    if(changed)
    {
        selectRoutes();
    }

#ifdef BABEL_DEBUG
        EV << "Interface " << iface->getIfaceName() << " deactivated for Babel routing" << endl;
#endif
}


void BabelMain::sendMessage(L3Address dst, BabelInterface *outIface, BabelMessage *msg)
{
    ASSERT(outIface != NULL);
    ASSERT(msg != NULL);

    if(!outIface->getEnabled() || !outIface->getInterface()->isUp() || outIface->getAfSend() == AF::NONE)
    {// trying send message on DOWN iface -> cancel
#ifdef BABEL_DEBUG
        EV << "Trying send message on DOWN or PASSIVE interface - sending canceled" << endl;
#endif
        delete msg;
        return;
    }

    // Check AddressFamily of destination -> choose corresponding socket
    UDPSocket *socket = NULL;
    if(dst.getType()==L3Address::IPv6)
    {
        socket = outIface->getSocket6();
    }
    else
    {
        socket = outIface->getSocket4();
    }

    if(socket != NULL)
    {// socket is ready
        msg->countStats(&(outIface->txStat));
        //socket->sendTo(msg, dst, port, outIface->getInterfaceId());
        UDPSocket::SendOptions options;
        options.outInterfaceId = outIface->getInterfaceId();
        socket->sendTo(msg, dst, port, &options);
    }
    else
    {
        EV << "Packet not send - UDP socket (" << ((dst.getType()==L3Address::IPv6) ? "IPv6" : "IPv4") << ") on interface " << outIface->getIfaceName() << " is not ready" << endl;
    }
}

/**
 * Process received message
 *
 * @param   msg     Received message
 */
void BabelMain::processMessage(BabelMessage *msg)
{
    ASSERT(msg != NULL);

#ifdef BABEL_DEBUG
    EV << msg->detailedInfo() << endl;
#else
    EV << msg->info() << endl;
#endif

    UDPDataIndication *ctrl = check_and_cast<UDPDataIndication *>(PK(msg)->getControlInfo());
    L3Address src = ctrl->getSrcAddr();
    L3Address dst = ctrl->getDestAddr();
    BabelInterface *iniface = bit.findInterfaceById(ctrl->getInterfaceId());

    if(iniface == NULL)
    {
#ifdef BABEL_DEBUG
        throw cRuntimeError("Received babel message on non-babel interface: %s", ift->getInterfaceById(ctrl->getInterfaceId())->getName());
#endif
        return;
    }

    msg->countStats(&(iniface->rxStat));

    bool changed = false;
    rid prevrid;
    L3Address prevnh4 =  !(src.getType()==L3Address::IPv6) ? src : L3Address(IPv4Address());
    L3Address prevnh6 =    src.getType()==L3Address::IPv6  ? src : L3Address(IPv6Address());
    netPrefix<L3Address> prevprefix4 = netPrefix<L3Address>(IPv4Address(), 0);
    netPrefix<L3Address> prevprefix6 = netPrefix<L3Address>(IPv6Address(), 0);


    char *msgbody = msg->getBody();
    for(int tlvoffset = msg->getNextTlv(-1); tlvoffset != -1; tlvoffset = msg->getNextTlv(tlvoffset))
    {
        uint8_t tmplen;
        uint8_t tmpae;
        uint8_t tmpflags;
        uint8_t tmpplen;
        uint16_t tmpmetric;

        uint8_t tlvtype = static_cast<uint8_t>(msgbody[tlvoffset]);
        switch(tlvtype)
        {
        case tlvT::PAD1:
            // nothing to do
            break;
        case tlvT::PADN:
            // nothing to do
            break;
        case tlvT::ACKREQ:
            processAckReqTlv(msgbody + tlvoffset, iniface, src);
            break;
        case tlvT::ACK:
            processAckTlv(msgbody + tlvoffset, src);
            break;
        case tlvT::HELLO:
            processHelloTlv(msgbody + tlvoffset, iniface, src);
            break;
        case tlvT::IHU:
            changed = processIhuTlv(msgbody + tlvoffset, iniface, src, dst) || changed;
            break;
        case tlvT::ROUTERID:
            prevrid.setRid(ntohl(*reinterpret_cast<uint32_t *>(msgbody + tlvoffset + 4)),
                           ntohl(*reinterpret_cast<uint32_t *>(msgbody + tlvoffset + 8)));
            break;
        case tlvT::NEXTHOP:
            tmpae = *reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 2);

            if(tmpae == AE::IPv4)
            {// encoded IPv4 address -> read
                prevnh4 = readRawAddr(tmpae, msgbody + tlvoffset + 4);
            }
            else if(tmpae == AE::IPv6 || tmpae == AE::LLIPv6)
            {// encoded IPv6 address -> read
                prevnh6 = readRawAddr(tmpae, msgbody + tlvoffset + 4);
            }
            else
            {//unknown AE or WILDCARD -> ignore
                #ifdef BABEL_DEBUG
                EV << "Received Next-hop TLV with bad AE value: " << static_cast<unsigned int>(tmpae) << " - TLV ignored" << endl;
                #endif
                break;
            }
            break;
        case tlvT::UPDATE:
            tmplen = *reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 1);
            tmpae = *reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 2);
            tmpflags = *reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 3);
            tmpplen = *reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 4);
            tmpmetric = ntohs(*reinterpret_cast<uint16_t *>(msgbody + tlvoffset + 10));

            if(tmpflags & 0x80)
            {// update contains full prefix -> remember as previous for next Update TLVs
                if(tmpae == AE::IPv4)
                {// encoded IPv4 prefix -> read
                    prevprefix4.set(tmpae, msgbody + tlvoffset + 12, tmpplen);
                }
                else if(tmpae == AE::IPv6)
                {// encoded IPv6 prefix -> read
                    prevprefix6.set(tmpae, msgbody + tlvoffset + 12, tmpplen);
                }
                else
                {//unknown AE or WILDCARD or LLIPv6 -> ignore
                    #ifdef BABEL_DEBUG
                    EV << "Received Update TLV with bad AE value: " << static_cast<unsigned int>(tmpae) << " - TLV ignored" << endl;
                    #endif
                    break;
                }
            }

            if((tmpflags & 0x40) && tmpmetric != 0xFFFF && tmplen >= 18)
            {// update contains router-id (in last 8 octets) -> remember
                // WARNING: babeld sends 0x40 flag in retraction updates (= without router-id) -> MUST CHECK metric

                prevrid.setRid(ntohl(*reinterpret_cast<uint32_t *>(msgbody + tlvoffset + 2 + tmplen - 8)),
                               ntohl(*reinterpret_cast<uint32_t *>(msgbody + tlvoffset + 2 + tmplen - 4)));
            }


            if(tmpae == AE::IPv4)
            {// encoded IPv4 prefix -> process
                changed = processUpdateTlv(msgbody + tlvoffset, iniface, src, prevrid, prevnh4, &prevprefix4) || changed;
            }
            else if(tmpae == AE::IPv6)
            {// encoded IPv6 prefix -> process
                changed = processUpdateTlv(msgbody + tlvoffset, iniface, src, prevrid, prevnh6, &prevprefix6) || changed;
            }
            else if(tmpae == AE::WILDCARD && tmpmetric == 0xFFFF)
            {// wildcard retraction - router-id, next-hop and previous-prefix are not used
                changed = processUpdateTlv(msgbody + tlvoffset, iniface, src, rid(), L3Address(), NULL) || changed;
            }
            else
            {//unknown AE -> ignore
                #ifdef BABEL_DEBUG
                EV << "Received Update TLV with bad AE value: " << static_cast<unsigned int>(tmpae) << " - TLV ignored" << endl;
                #endif
                break;
            }


            break;
        case tlvT::ROUTEREQ:
            processRouteReqTlv(msgbody + tlvoffset, iniface, src, dst);
            break;
        case tlvT::SEQNOREQ:
            processSeqnoReqTlv(msgbody + tlvoffset, iniface, src);
            break;
        default:
            break;
        }
    }

    if(changed)
    {
        selectRoutes();
    }

}

/**
 * Process received AckReq TLV
 *
 * @param   tlv     Pointer to TLV in memory
 * @param   iniface Interface of which was TLV received
 * @param   src     Souce address of message in which was TLV received
 */
void BabelMain::processAckReqTlv(char *tlv, BabelInterface *iniface, const L3Address& src)
{
    ASSERT(tlv != NULL);
    ASSERT(iniface != NULL);

    uint16_t nonce = ntohs(*reinterpret_cast<uint16_t *>(tlv + 4));
    uint16_t interval = ntohs(*reinterpret_cast<uint16_t *>(tlv + 6));

    // send ACK back - as unicast, buffer maximally for half of propageted interval
    sendTLV(src, iniface, new BabelAckFtlv(nonce), CStoS(interval / 2));
}

/**
 * Process received Ack TLV
 *
 * @param   tlv     Pointer to TLV in memory
 * @param   src     Souce address of message in which was TLV received
 */
void BabelMain::processAckTlv(char *tlv, const L3Address& src)
{
    ASSERT(tlv != NULL);

    uint16_t nonce = ntohs(*reinterpret_cast<uint16_t *>(tlv + 2));

    BabelToAck *toack = findToAck(nonce);

    if(toack != NULL)
    {// node acknowledged receiving -> remove dst node
        toack->removeDstNode(src);

        if(toack->dstNodesSize() == 0)
        {// all nodes acknowledged receiving -> remove ToAck
            deleteToAck(toack);
        }
    }
}

/**
 * Process received Hello TLV
 *
 * @param   tlv     Pointer to TLV in memory
 * @param   iniface Interface of which was TLV received
 * @param   src     Souce address of message in which was TLV received
 */
void BabelMain::processHelloTlv(char *tlv, BabelInterface *iniface, const L3Address& src)
{
    ASSERT(tlv != NULL);
    ASSERT(iniface != NULL);

    uint16_t seqno = ntohs(*reinterpret_cast<uint16_t *>(tlv + 4));
    uint16_t interval = ntohs(*reinterpret_cast<uint16_t *>(tlv + 6));

    BabelNeighbour *neigh = bnt.findNeighbour(iniface, src);


    if(neigh == NULL)
    {// not in table - new neighbour -> create
        neigh = bnt.addNeighbour(new BabelNeighbour(iniface, src,
                createTimer(timerT::NEIGHHELLO, NULL, NULL, false),
                createTimer(timerT::NEIGHIHU, NULL, NULL, false)));

        EV  << "New neighbour " << neigh->getAddress() << " acquired on " << neigh->getInterface()->getIfaceName() << endl;
    }
    else
    {
        if(seqno != neigh->getExpHSeqno())
        {//
            if(abs(minusmod16(seqno, neigh->getExpHSeqno())) > HISTORY_LEN)
            {// sending node probably rebooted -> reset history
                neigh->setHistory(0);
            }
            else if(comparemod16(seqno, neigh->getExpHSeqno()) == -1)
            {// sending node has increased hello interval -> undo history
                neigh->setHistory(neigh->getHistory() << minusmod16(neigh->getExpHSeqno(), seqno));
            }
            else if(comparemod16(seqno, neigh->getExpHSeqno()) == 1)
            {// sending node has decreased hello interval, and some hello tlv were lost -> fast-forward
                neigh->setHistory(neigh->getHistory() >> minusmod16(seqno, neigh->getExpHSeqno()));
            }
        }
    }

    neigh->noteReceive();
    neigh->setNeighHelloInterval(interval);
    neigh->setExpHSeqno(plusmod16(seqno, 1));
    neigh->resetNHTimer(CStoS(interval) * 1.5); // extra margin for compensate jitter


    ///NEW NEIGHBOUR
    if((neigh->getHistory() & 0xBF00) == 0x8000)
    {// first or second message from neighbour (from last 8 intervals) = [1X00 0000 XXXX XXXX]
        // get feedback immediately -> send hello
        sendHelloTLV(iniface, SEND_URGENT);    //implicitly sends IHU, because of bad history
    }

    if((neigh->getHistory() & 0xFC00) == 0xC000)
    {// second message from neighbour (from last 6 intervals) = [1100 00XX XXXX XXXX] ... now rxcost (computed using 2-out-of-3) is not 0xFFFF
        // unicast wildcard full route dump request
        sendTLV(src, iniface, new BabelRouteReqFtlv(), SEND_URGENT);
    }

    if(neigh->recomputeCost())
    {// cost has changed
        selectRoutes();
    }
}

/**
 * Process received Ihu TLV
 *
 * @param   tlv     Pointer to TLV in memory
 * @param   iniface Interface of which was TLV received
 * @param   src     Souce address of message in which was TLV received
 * @param   dst     Destination address of message in which was TLV received
 *
 * @return  True if neighbours cost has changed, otherwise false
 */
bool BabelMain::processIhuTlv(char *tlv, BabelInterface *iniface, const L3Address& src, const L3Address& dst)
{
    ASSERT(tlv != NULL);
    ASSERT(iniface != NULL);

    uint8_t ae = *reinterpret_cast<uint8_t *>(tlv + 2);
    uint16_t rxcost = ntohs(*reinterpret_cast<uint16_t *>(tlv + 4));
    uint16_t interval = ntohs(*reinterpret_cast<uint16_t *>(tlv + 6));
    L3Address address;


    if(ae == AE::WILDCARD)
    {// wildcard -> use dst address
        if(dst.isMulticast())
        {// wildcard MUST NOT be sent to multicast address
           return false;
        }

        address = dst;
    }
    else if(ae == AE::IPv4 || ae == AE::IPv6 || ae == AE::LLIPv6)
    {// encoded address -> read
        address = readRawAddr(ae, tlv + 8);
    }
    else
    {//unknown AE -> silently ignore
#ifdef BABEL_DEBUG
        EV << "Received IHU TLV with unknown AE value: " << static_cast<unsigned int>(ae) << " - TLV ignored" << endl;
#endif
        return false;
    }


    if(!(      (!(address.getType()==L3Address::IPv6) && address.toIPv4() == iniface->getInterface()->ipv4Data()->getIPAddress())
            || (  address.getType()==L3Address::IPv6  && iniface->getInterface()->ipv6Data()->hasAddress(address.toIPv6()))
        )
      )
    {// address is not my address on ingress interface -> ignore
        return false;
    }

    BabelNeighbour *neigh = bnt.findNeighbour(iniface, src);

    if(neigh)
    {
        neigh->setTxcost(rxcost);
        neigh->setNeighIhuInterval(interval);
        neigh->resetNITimer(CStoS(defval::IHU_HOLD_INTERVAL_MULT * interval));

        return neigh->recomputeCost();
    }


    return false;
}

/**
 * Process received Update TLV
 *
 * @param   tlv     Pointer to TLV in memory
 * @param   iniface Interface of which was TLV received
 * @param   src     Souce address of message in which was TLV received
 * @param   originator  RouterId of originator
 * @param   nh      Address of next hop
 * @param   prevprefix  Previous prefix from same message
 *
 * @return  True if topology table has changed, otherwise false
 */
bool BabelMain::processUpdateTlv(char *tlv, BabelInterface *iniface, const L3Address& src, const rid& originator, const L3Address& nh, netPrefix<L3Address> *prevprefix)
{
    ASSERT(tlv != NULL);
    ASSERT(iniface != NULL);

    uint8_t ae = *reinterpret_cast<uint8_t *>(tlv + 2);
    uint8_t plen = *reinterpret_cast<uint8_t *>(tlv + 4);
    uint8_t omitted = *reinterpret_cast<uint8_t *>(tlv + 5);
    uint16_t interval = ntohs(*reinterpret_cast<uint16_t *>(tlv + 6));
    uint16_t seqno = ntohs(*reinterpret_cast<uint16_t *>(tlv + 8));
    uint16_t metric = ntohs(*reinterpret_cast<uint16_t *>(tlv + 10));

    bool changed = false;
    routeDistance dist = routeDistance(seqno, metric);
    netPrefix<L3Address> prefix = ((ae == AE::IPv4 || ae == AE::IPv6) ? netPrefix<L3Address>(ae, tlv + 12, plen, omitted, prevprefix) : netPrefix<L3Address>());
    BabelNeighbour *neigh = bnt.findNeighbour(iniface, src);

    if(!neigh)
    {// update from node i am not associated with -> ignore
        return false;
    }


    BabelPenSR *req = bpsrt.findPenSR(prefix, iniface); // TODO - is really needed to check interface?
    if(req && req->getReceivedFrom() != NULL)
    {// forwarded seqnoRequest for this prefix exists - this update is reply -> forward
        L3Address measnh;

        if(req->getRequest().getPrefix().getAddr().getType()==L3Address::IPv6)
        {// IPv6
            measnh = req->getReceivedFrom()->getInterface()->getInterface()->ipv6Data()->getLinkLocalAddress();
        }
        else
        {// IPv4
            measnh = req->getReceivedFrom()->getInterface()->getInterface()->ipv4Data()->getIPAddress();
        }

        if(!measnh.isUnspecified())   //TODO - verify
        {// interface has address of same AF
            EV << "Received answer to forwarded SeqnoRequest (for " << prefix << ") - forwarding on " << req->getReceivedFrom()->getInterface()->getIfaceName() << endl;
            sendUpdateTLV(req->getReceivedFrom()->getInterface(),
                    new BabelUpdateFtlv(req->getReceivedFrom()->getInterface()->getUInterval(), prefix,
                    routeDistance(dist.getSeqno(), dist.getMetric() + neigh->getCost()), originator, measnh), roughly(SEND_URGENT));

            bpsrt.removePenSR(req);
        }
    } else if(req && req->getReceivedFrom() == NULL && dist.getSeqno() >= req->getRequest().getSeqno())
    {// this update is reply to mine request and update satisfy required seqno -> delete all requests for this prefix
        bpsrt.removePenSR(prefix);
    }


    if(interval == 0xFFFF)
    {// node will not automatically sends updates -> use my own Update interval (before expire will be send RouteReq)
        interval = iniface->getUInterval();
    }

    //WILDCARD RETRACTION
    if(metric == 0xFFFF && ae == AE::WILDCARD)
    {// wildcard retraction -> retracts all of the routes by this neighbour

        EV << "Received wildcard retraction - retracting all routes from neighbour " << neigh->getAddress() << endl;
        for(std::vector<BabelRoute *>::iterator it = btt.getRoutes().begin(); it != btt.getRoutes().end(); ++it)
        {
            if((*it)->getNeighbour() == neigh)
            {
                changed = addOrUpdateRoute((*it)->getPrefix(), neigh, (*it)->getOriginator(), routeDistance((*it)->getRDistance().getSeqno(), 0xFFFF), (*it)->getNextHop(), interval) || changed;
            }
        }
        return changed;
    }



    BabelRoute *intable = btt.findRoute(prefix, neigh, originator);

    if(intable)
    {// route already in topology table
        if(intable->getSelected() && !isFeasible(prefix, originator, dist))
        {// route is selected and update is not feasible
            if(intable->getOriginator() != originator)
            {// router-ids are different -> update is retraction
                changed = addOrUpdateRoute(prefix, neigh, originator, routeDistance(dist.getSeqno(), 0xFFFF), nh, interval);
            }

            EV << "Received Unfeasible update for selected route - sending Seqno Request" << endl;
            //3.8.2.2 - UNFEASIBLE updated for SELECTED ROUTE -> send SEQNO REQ
            sendSeqnoReqTLV(neigh->getAddress(),
                    neigh->getInterface(),
                    new BabelSeqnoReqFtlv(plusmod16(intable->getRDistance().getSeqno(), 1), defval::SEQNUMREQ_HOPCOUNT, intable->getOriginator(), intable->getPrefix()));
        }
        else
        {
            if(isFeasible(prefix, originator, dist))
            {
                changed = addOrUpdateRoute(prefix, neigh, originator, dist, nh, interval);
            }// else ignore
        }
    }
    else
    {// no such route
        if(!isFeasible(prefix, originator, dist))
        {
            return changed;
        }

        if(dist.getMetric() == 0xFFFF)
        {
            return changed;
        }

        changed = addOrUpdateRoute(prefix, neigh, originator, dist, nh, interval);
    }

    return changed;
}

/**
 * Process received RouteReq TLV
 *
 * @param   tlv     Pointer to TLV in memory
 * @param   iniface Interface of which was TLV received
 * @param   src     Souce address of message in which was TLV received
 * @param   dst     Destination address of message in which was TLV received
 *
 */
void BabelMain::processRouteReqTlv(char *tlv, BabelInterface *iniface, const L3Address& src, const L3Address& dst)
{
    ASSERT(tlv != NULL);
    ASSERT(iniface != NULL);

    uint8_t ae = *reinterpret_cast<uint8_t *>(tlv + 2);
    uint8_t plen = *reinterpret_cast<uint8_t *>(tlv + 3);

    if(ae == AE::WILDCARD)
    {// request for full table dump
        sendFullDump(iniface);
    }
    else if(ae == AE::IPv4 || ae == AE::IPv6)
    {
        netPrefix<L3Address> prefix = netPrefix<L3Address>(ae, tlv + 4, plen);
        BabelRoute *intable = btt.findSelectedRoute(prefix);

        if(intable)
        {// have selected route to requested prefix -> send update
            if(dst.isMulticast())
            {// request send as multicast -> answer as multicast
                sendUpdate(iniface, intable);
            }
            else
            {// request send as unicast -> answer as unicast
                sendUpdate(src, iniface, intable);
            }
        }
        else
        {// no route -> send retraction
            if(dst.isMulticast())
            {// request send as multicast -> answer as multicast
                sendUpdateTLV(iniface, new BabelUpdateFtlv(iniface->getUInterval(),
                             prefix, routeDistance(0, 0xFFFF), rid() , L3Address()));
            }
            else
            {// request send as unicast -> answer as unicast
                sendUpdateTLV(src, iniface, new BabelUpdateFtlv(iniface->getUInterval(),
                             prefix, routeDistance(0, 0xFFFF), rid() , L3Address()));
            }
        }
    }
    else
    {//unknown AE -> silently ignore
        #ifdef BABEL_DEBUG
        EV << "Received RouteReq TLV with unknown AE value: " << static_cast<unsigned int>(ae) << " - TLV ignored" << endl;
        #endif
    }
}

/**
 * Process received SourceReq TLV
 *
 * @param   tlv     Pointer to TLV in memory
 * @param   iniface Interface of which was TLV received
 * @param   src     Souce address of message in which was TLV received
 */
void BabelMain::processSeqnoReqTlv(char *tlv, BabelInterface *iniface, const L3Address& src)
{
    ASSERT(tlv != NULL);
    ASSERT(iniface != NULL);

    uint8_t ae = *reinterpret_cast<uint8_t *>(tlv + 2);
    uint8_t plen = *reinterpret_cast<uint8_t *>(tlv + 3);
    uint16_t seqno = ntohs(*reinterpret_cast<uint16_t *>(tlv + 4));
    uint8_t hopcount = *reinterpret_cast<uint8_t *>(tlv + 6);
    rid origrid = rid(ntohl(*reinterpret_cast<uint32_t *>(tlv + 8)),
                       ntohl(*reinterpret_cast<uint32_t *>(tlv + 12)));
    netPrefix<L3Address> prefix;

    if(ae == AE::IPv4 || ae == AE::IPv6)
    {
        prefix = netPrefix<L3Address>(ae, tlv + 16, plen);
    }
    else
    {//unknown AE -> silently ignore
        #ifdef BABEL_DEBUG
        EV << "Received SeqnoReq TLV with bad AE value: " << static_cast<unsigned int>(ae) << " - TLV ignored" << endl;
        #endif
        return;
    }


    if(bpsrt.findPenSR(prefix))
    {// request already forwarded = duplication detection -> discard
        return;
    }


    BabelRoute *intable = btt.findSelectedRoute(prefix);

    if(intable && intable->getRDistance().getMetric() != 0xFFFF)
    {// found selected route

        if(origrid != intable->getOriginator() || comparemod16(seqno, intable->getRDistance().getSeqno()) != 1)
        {// different router-ids, or same router-ids and entrys seqno is no smaller -> send update
            sendUpdate(iniface, intable, roughly(SEND_URGENT));
        }
        else
        {// router-ids match and TT entry have smaller seqno
            if(origrid == routerId)
            {// i am originator -> increment seqno
                incSeqno();
                sendUpdate(iniface, intable, roughly(SEND_URGENT));
            }
            else
            {// try forward
                BabelRoute *anotherroute = btt.findRouteNotNH(prefix, src);
                BabelNeighbour *neigh = bnt.findNeighbour(iniface, src);
                if(anotherroute && neigh && hopcount >= 2)
                {// have another route and hopcount is >=2 -> forward the request
                    sendSeqnoReqTLV(anotherroute->getNeighbour()->getAddress(), anotherroute->getNeighbour()->getInterface(),
                            new BabelSeqnoReqFtlv(seqno, hopcount - 1, origrid, prefix), neigh, roughly(SEND_URGENT));
                }
            }
        }
    }// else ignore

}

/**
 * Find buffer
 *
 * @param   da  Destination address
 * @param   oi  Outgoing interface
 *
 * @return  Pointer to buffer if found, otherwise NULL
 */
BabelBuffer* BabelMain::findBuffer(L3Address da, BabelInterface *oi)
{
    std::vector<BabelBuffer *>::iterator it;

    for (it = buffers.begin(); it != buffers.end(); ++it)
    {// through all buffers search for same destination address and interfaceId
        if((*it)->getDst() == da && (*it)->getOutIface()->getInterfaceId() == oi->getInterfaceId())
        {// found same
            return (*it);
        }
    }

    return NULL;
}

/**
 * Find or create buffer
 *
 * @param   da  Destination address
 * @param   oi  Outgoing interface
 *
 * @return  Pointer to buffer
 *
 */
BabelBuffer* BabelMain::findOrCreateBuffer(L3Address da, BabelInterface *oi)
{
    BabelBuffer *buff = findBuffer(da, oi);

    if(buff == NULL)
    {// not found -> create new buffer
        buff = new BabelBuffer(da, oi, createTimer(timerT::BUFFER, NULL, NULL, false));
        buff->getFlushTimer()->setContextPointer(buff);
        buffers.push_back(buff);
    }

    return buff;
}

/**
 * Send TLV to buffer
 *
 * @param   da  Destination address
 * @param   oi  Outgoing interface
 * @param   ftlv    Ftlv to send
 * @param   mt  Maximal buffering time
 */
void BabelMain::sendTLV(L3Address da, BabelInterface *oi, BabelFtlv *ftlv, double mt)
{
    ASSERT(oi != NULL);
    ASSERT(ftlv != NULL);
    ASSERT(mt >= 0.0);

    if(oi->getAfSend() == AF::NONE)
    {// out interface is passive -> do not send
        return;
    }

    BabelBuffer *buff = findOrCreateBuffer(da, oi);

    if(ftlv->getType() == tlvT::HELLO && buff->containTlv(tlvT::HELLO))
    {// adding Hello TLV, but Hello TLV in buffer already -> flush before adding
        flushBuffer(buff);
    }

    buff->addTlv(ftlv);

    ASSERT(buff->getFlushTimer() != NULL);

    if(mt == SEND_NOW)
    {// send without buffering
        flushBuffer(buff);
    }
    else if(mt == SEND_BUFFERED)
    {// send with buffering
        if(!buff->getFlushTimer()->isScheduled())
        {// flush is not scheduled -> schedule flush to max-buffer-time
            resetTimer(buff->getFlushTimer(), CStoS(roughly(oi->getHInterval() / defval::BUFFER_MT_DIVISOR)));
        }
    }
    else
    {// max-buffer-time is specified -> send with limited buffering
        if(!buff->getFlushTimer()->isScheduled())
        {// flush is not scheduled -> schedule flush to specified max-buffer-time
            resetTimer(buff->getFlushTimer(), mt);
        }
        else
        {// flush is scheduled -> compute delay
            simtime_t flushdelay = buff->getFlushTimer()->getArrivalTime() - simTime();
            if(flushdelay > mt)
            {// planned flush delay is greater than max-buffer-time -> reset flush timer to specified max-buffer-time
                resetTimer(buff->getFlushTimer(), mt);
            }
        }
    }

}

/**
 * Send TLV to buffer to multicast
 *
 * @param   oi  Outgoing interface
 * @param   ftlv    Ftlv to send
 * @param   mt  Maximal buffering time
 */
void BabelMain::sendTLV(BabelInterface *oi, BabelFtlv *ftlv, double mt)
{
    ASSERT(oi != NULL);

    int ifaceaf = oi->getAfSend();

    if(ifaceaf == AF::IPv4)
    {// send IPv4
        sendTLV(defval::MCASTG4, oi, ftlv, mt);
    }
    else if(ifaceaf == AF::IPv6)
    {// send IPv6
        sendTLV(defval::MCASTG6, oi, ftlv, mt);
    }
    else if(ifaceaf == AF::IPvX)
    {// send IPv4 and IPv6
        sendTLV(defval::MCASTG4, oi, ftlv->dup(), mt);
        sendTLV(defval::MCASTG6, oi, ftlv, mt);
    }
}

/**
 * Delete buffer
 *
 * @param   todel   Buffer to delete
 */
void BabelMain::deleteBuffer(BabelBuffer *todel)
{
    std::vector<BabelBuffer *>::iterator it;

    if(todel == NULL)
    {// nothing to delete
        return;
    }

    for (it = buffers.begin(); it != buffers.end();)
    {// find buffer
        if((*it) == todel)
        {// found -> delete
            delete (*it);
            it = buffers.erase(it);
            return;
        }
        else
        {// do not delete -> get next
            ++it;
        }
    }
}


/**
 * Deletes unused buffers
 */
void BabelMain::deleteUnusedBuffers()
{
    std::vector<BabelBuffer *>::iterator it;

    for (it = buffers.begin(); it != buffers.end();)
    {// search all unused buffers
        BabelTimer *timer = (*it)->getFlushTimer();
        if((timer && !timer->isScheduled()) || !timer)
        {// unscheduled timer == unused buffer -> delete
            delete (*it);
            it = buffers.erase(it);
        }
        else
        {// do not delete -> get next
            ++it;
        }
    }
}

/**
 * Deletes all buffers
 */
void BabelMain::deleteBuffers()
{
    std::vector<BabelBuffer *>::iterator it;

    for (it = buffers.begin(); it != buffers.end(); ++it)
    {// delete all buffers
        //cancelAndDelete((*it)->getFlushTimer());
        delete (*it);
    }
    buffers.clear();
}

/**
 * Flush buffer
 *
 * Converts FTLVs to TLV and puts them to message.
 *
 * @param   buff    Buffer to flush
 */
void BabelMain::flushBuffer(BabelBuffer *buff)
{
    ASSERT(buff != NULL);


    while(buff->tlvsSize() > 0)
    {// while buffer is not empty
        // compute maximum size of body = MTU - IP_HEADER - UDP_HEADER - BABEL_HEADER
        int maxbodysize = buff->getOutIface()->getInterface()->getMTU()
                - ((buff->getDst().getType()==L3Address::IPv6) ? IPV6_HEADER_SIZE : IPV4_HEADER_SIZE)
                - UDP_HEADER_SIZE - BABEL_HEADER_SIZE;

        if(maxbodysize < (512 - BABEL_HEADER_SIZE))
        {// message size is smaller than minimal size (512) -> set minimal size
            maxbodysize = 512 - BABEL_HEADER_SIZE;
        }
        else if((maxbodysize + BABEL_HEADER_SIZE) > UINT16_MAX)
        {// message size is larger than maximal size (65535) -> set maximal size
            maxbodysize = UINT16_MAX - BABEL_HEADER_SIZE;
        }

        char *tmpdata = new char[maxbodysize];  ///< temporary memory
        int tmpend = 0;                         ///< pointer to unused part of temporary memory
        BabelMessage *msg = new BabelMessage("BabelMessage");
        std::vector<BabelFtlv *>::iterator it;
        int msgsn = 1;                          ///< number of send of message
        bool full = (maxbodysize <= tmpend);    ///< state of temporary memory
        bool containackreq = false;             ///< is ACKREQ TLV in MSG?
        bool containhello = false;              ///< is HELLO TLV in MSG?
        uint16_t acknonce;                      ///< nonce field of ACKREQ TLV
        rid prevrid;
        L3Address prevnh4 = !(buff->getDst().getType()==L3Address::IPv6) ? buff->getOutIface()->getInterface()->ipv4Data()->getIPAddress() : L3Address(IPv4Address());
        L3Address prevnh6 =   buff->getDst().getType()==L3Address::IPv6  ? buff->getOutIface()->getInterface()->ipv6Data()->getLinkLocalAddress() : L3Address(IPv6Address());
        netPrefix<L3Address> prevprefix4 = netPrefix<L3Address>(IPv4Address(), 0);
        netPrefix<L3Address> prevprefix6 = netPrefix<L3Address>(IPv6Address(), 0);

        for (it = buff->tlvsBegin(); it != buff->tlvsEnd();)
        {
            int freespace = maxbodysize - tmpend;
            uint8_t tlvtype = (*it)->getType();
            int tlvsn = (*it)->getSendNum();

            // Hello TLV should not be send repeatedly
            if((tlvtype == tlvT::HELLO && (containhello || msgsn != 1))
                    || (containhello && tlvsn != 1))
            {// message can not contain more than one Hello TLV, and same Hello TLV should not be send repeatedly (including repeating using ACK)
             // if message already contain Hello TLV, can not add TLVs with different send number -> skip this TLV

                ++it;   // jump to next TLV
                continue;
            }

            // find maximal send number or add ACKREQ if required
            if(tlvsn == USE_ACK && !containackreq)
            {// TLV require send with ACKREQ, but msg do not contain ACKREQ TLV -> add ACKREQ

                BabelAckReqFtlv ackreq = BabelAckReqFtlv(generateNonce(), buff->getOutIface()->getHInterval() / 2);

                if(freespace < ackreq.rawTlvLength())
                {// not enough free space
                    full = true;
                    break;
                }

                tmpend += ackreq.copyRawTlv(tmpdata + tmpend);

                freespace = maxbodysize - tmpend;   // must recalculate free space
                msgsn = tlvsn;                      // remember USE_ACK
                acknonce = ackreq.getNonce();       // remember used nonce
                containackreq = true;
            }
            else if(tlvsn > msgsn && msgsn != USE_ACK)
            {// send number is greater than previous ones AND using ACK is not required yet -> remember maximal send number
                msgsn = tlvsn;
            }


            // COPY RAW TLV
            if(tlvtype != tlvT::UPDATE)
            {
                if(freespace < (*it)->rawTlvLength())
                {// not enough free space
                    full = true;
                    break;
                }
                tmpend += (*it)->copyRawTlv(tmpdata + tmpend);

                if(tlvtype == tlvT::HELLO)
                {
                    containhello = true;
                }
            }
            else
            {// UPDATE TLV -> optimize
                BabelUpdateFtlv *updatetlv = dynamic_cast<BabelUpdateFtlv *>(*it);
                L3Address *prevnhsameaf = (updatetlv->getNextHop().getType()==L3Address::IPv6) ? &prevnh6 : &prevnh4;
                netPrefix<L3Address> *prevprefixsameaf = (updatetlv->getPrefix().getAddr().getType()==L3Address::IPv6) ? &prevprefix6 : &prevprefix4;

                ASSERT(updatetlv != NULL);

                if(updatetlv->getDistance().getMetric() != 0xFFFF && updatetlv->getRouterId() != prevrid)
                {// update is not retraction and previous RouterId is different -> add RouterId TLV
                    BabelRouterIdFtlv ridtlv = BabelRouterIdFtlv(updatetlv->getRouterId());

                    if(freespace < ridtlv.rawTlvLength())
                    {// not enough free space
                        full = true;
                        break;
                    }

                    tmpend += ridtlv.copyRawTlv(tmpdata + tmpend);

                    freespace = maxbodysize - tmpend;   // must recalculate free space

                    prevrid = updatetlv->getRouterId();
                }

                if(updatetlv->getDistance().getMetric() != 0xFFFF
                        && (   (  updatetlv->getNextHop().getType()==L3Address::IPv6  && updatetlv->getNextHop() != prevnh6)
                            || (!(updatetlv->getNextHop().getType()==L3Address::IPv6) && updatetlv->getNextHop() != prevnh4)
                           )
                  )
                {// update is not retraction and previous NextHop is different -> add NextHop TLV
                    BabelNextHopFtlv nhtlv = BabelNextHopFtlv(updatetlv->getNextHop());

                    if(freespace < nhtlv.rawTlvLength())
                    {// not enough free space
                        full = true;
                        break;
                    }

                    tmpend += nhtlv.copyRawTlv(tmpdata + tmpend);

                    freespace = maxbodysize - tmpend;   // must recalculate free space

                    *prevnhsameaf = updatetlv->getNextHop();
                }

                // Copy
                if(freespace < updatetlv->rawTlvLength(*prevprefixsameaf))
                {// not enough free space
                    full = true;
                    break;
                }
                tmpend += updatetlv->copyRawTlv(tmpdata + tmpend, prevprefixsameaf);

                if(updatetlv->getPrefix().bytesToOmit(*prevprefixsameaf) == 0)
                {// no ommited bytes -> remember as new default prefix
                    *prevprefixsameaf = updatetlv->getPrefix();
                }
            }



            if(full)
            {// message body is full - processed TLV is not included in message -> do not delete
                break;
            }

            /// DELETE PROCESSED FTLV AND CHOOSE NEXT
            if(tlvtype == tlvT::UPDATE)
            {// processed TLV was Update TLV -> get next similar Update TLV (for effective compression)
                BabelUpdateFtlv *updatetlv = dynamic_cast<BabelUpdateFtlv *>(*it);
                ASSERT(updatetlv != NULL);


                std::vector<BabelFtlv *>::iterator nextutlv = buff->getSimilarUpdateTlv(updatetlv->getNextHop(), updatetlv->getRouterId());

                delete (*it);
                buff->eraseTlv(it);

                if(nextutlv == buff->tlvsEnd())
                {// there is no next Update TLV in buffer -> go from start of buffer
                    it = buff->tlvsBegin();
                }
                else
                {// another Update TLV exists -> process as next
                    it = nextutlv;
                }
            }
            else
            {
                delete (*it);
                it = buff->eraseTlv(it);
            }
        }

        /// COPY BODY TO MESSAGE
        msg->addToBody(tmpdata, tmpend);

        /// IF REQUIRED ADD TO TOACK-TABLE
        if(containackreq)
        {
            BabelToAck *toack = new BabelToAck(acknonce, defval::RESEND_NUM, createTimer(timerT::TOACKRESEND, NULL, NULL, false), buff->getDst(), buff->getOutIface(), msg->dup());

            // set destination nodes
            if(buff->getDst().isMulticast())
            {// destination is multicast -> as dst nodes add all neighbors on out interface

                for (std::vector<BabelNeighbour *>::iterator it = bnt.getNeighbours().begin(); it != bnt.getNeighbours().end(); ++it)
                {// through all same AF neighbours
                    if((*it)->getInterface() == buff->getOutIface())
                    {// neighbour on same interface -> add as dst node
                        toack->addDstNode((*it)->getAddress());
                    }
                }
            }
            else
            {// unicast -> add dst node
                toack->addDstNode(buff->getDst());
            }

            // add
            if(addToAck(toack))
            {
                resetTimer(toack->getResendTimer(), CStoS(buff->getOutIface()->getHInterval() / 2));
            }
            else
            {//can not add to ackwait
                cancelAndDelete(toack->getResendTimer());
                delete toack;
            }
        }


        /// SEND MESSAGE
        do
        {// repeatedly if required
            EV << "Sending Babel message to " << buff->getDst() << " on " << buff->getOutIface()->getIfaceName() << ":" << endl << msg->detailedInfo() << endl;
            sendMessage(buff->getDst(), buff->getOutIface(), ((msgsn > 1) ? msg->dup() : msg));
        } while(--msgsn > 0);

        delete [] tmpdata;
    }

    /// CANCEL FLUSH TIMER
    BabelTimer *ft = buff->getFlushTimer();
    if(ft && ft->isScheduled())
    {// flush is scheduled -> cancel
        cancelEvent(ft);
    }
}

/**
 * Flush all buffers
 */
void BabelMain::flushAllBuffers()
{
    std::vector<BabelBuffer *>::iterator it;

    for (it = buffers.begin(); it != buffers.end(); ++it)
    {// flush all buffers
        flushBuffer(*it);
    }
}

/**
 * Adds or updates route
 *
 * @param   prefix    Network prefix
 * @param   neigh     Neighbour from which was information received
 * @param   orig      Originator of information
 * @param   dist      Reported distance of route
 * @param   nh        Address of next hop
 * @param   interval  Interval of periodic updates
 *
 * @return  True if route has changed or new was created, otherwise false
 */
bool BabelMain::addOrUpdateRoute(const Babel::netPrefix<L3Address>& prefix, BabelNeighbour *neigh, const rid& orig, const routeDistance& dist, const L3Address& nh, uint16_t interval)
{
    ASSERT(interval != 0);

    bool changed = false;
    bool resetet = true;
    BabelRoute *route = btt.findRoute(prefix, neigh);

    if(route)
    {// route already in table -> update
        if(route->getRDistance() != dist)
        {// different reported distance
            route->setRDistance(dist);
            changed = true;
        }
        else
        {// distances are same
            if(dist.getMetric() == 0xFFFF)
            {// retraction of already retracted route -> do not reset timer
                resetet = false;
            }
        }

        if(route->getNextHop() != nh)
        {// different next hop -> this should not happen, but just in case
            route->setNextHop(nh);
            changed = true;
        }

        if(route->getOriginator() != orig)
        {// different originator
            route->setOriginator(orig);
            changed = true;

            if(route->getSelected())
            {// change of router-id for selected route -> may indicate routing loop formation
                // RFC 6126 - 3.7.2: send triggered update (repeteadly or with ACK REQ)

                sendUpdateTLV(route->getNeighbour()->getInterface(), new BabelUpdateFtlv(interval, prefix, dist, orig, nh), SEND_NOW, true);
            }
        }


        route->setUpdateInterval(interval);
        if(resetet)
        {
            route->resetETimer();
            if(route->getRDistance().getMetric() != 0xFFFF)
            {
                route->resetBETimer();
            }

        }
    }
    else
    {// new route
        BabelRoute *newroute = new BabelRoute(prefix, neigh, orig, dist, nh, interval, createTimer(timerT::ROUTEEXPIRY, NULL, prefix.str().c_str(), false), createTimer(timerT::ROUTEBEFEXPIRY, NULL, prefix.str().c_str(), false));
        newroute->resetETimer();
        newroute->resetBETimer();

        btt.addRoute(newroute);
        EV << "Learned new route: " << *newroute << endl;
        changed = true;
    }

    return changed;
}

/**
 * Adds or updates source
 *
 * @param   p   Network prefix
 * @param   orig      Originator of information
 * @param   dist      Reported distance of route
 */
void BabelMain::addOrupdateSource(const Babel::netPrefix<L3Address>& p, const Babel::rid& orig, const Babel::routeDistance& dist)
{
    BabelSource *source = bst.findSource(p, orig);

    if(source != NULL)
    {// existing source found
        if(dist < source->getFDistance())
        {// new distance is better -> update FD
            source->setFDistance(dist);
        }
        source->resetGCTimer(); // always reset GC timer
    }
    else
    {// source do not exists -> create
        BabelSource *newsource = new BabelSource(p, orig, dist, createTimer(timerT::SOURCEGC, NULL, NULL, false));
        newsource->resetGCTimer();

        if(bst.addSource(newsource) != newsource)
        {
            delete newsource;
        }
    }
}

/**
 * Adds ToAck
 *
 * @param   toadd   ToAck to add
 *
 * @return  Pointer to added ToAck if sucessfully added, otherwise NULL
 */
BabelToAck *BabelMain::addToAck(BabelToAck *toadd)
{
    if(findToAck(toadd->getNonce()) != NULL)
    {// toAck with same nonce already exists -> do not add (nonce MUST be unique)
#ifdef BABEL_DEBUG
        EV << "Try to add toAck with already existing nonce (" << static_cast<int>(toadd->getNonce()) << ") - ignored" << endl;
#endif
        return NULL;
    }

    ackwait.push_back(toadd);

    return toadd;
}

/**
 * Checks if route is feasible
 *
 * @param   prefix  Network prefix
 * @param   orig    Originator
 * @param   dist    Reported distance
 *
 * @return True if feasible, otherwise false
 */
bool BabelMain::isFeasible(const Babel::netPrefix<L3Address>& prefix, const Babel::rid& orig, const Babel::routeDistance& dist)
{
    if(dist.getMetric() == 0xFFFF)
    {// retraction is always feasible
        return true;
    }

    BabelSource *source = bst.findSource(prefix, orig);

    if(source)
    {// found source entry
        if(dist < source->getFDistance())
        {// distance is strictly better than distance in source entry -> feasible
            return true;
        }
    }
    else
    {// no source entry -> feasible
        return true;
    }

    return false;
}

/**
 * Selects best routes
 */
void BabelMain::selectRoutes()
{
    std::vector<BabelRoute *> toselect;     ///< Best candidates to select
    std::vector<BabelRoute *> tocancel;     ///< Previously selected routes

    // FOUND BEST ROUTES, REMEMBER PREVIOUSLY SELECTED
    for (std::vector<BabelRoute *>::iterator it = btt.getRoutes().begin(); it != btt.getRoutes().end(); ++it)
    {// through all routes

        if((*it)->getSelected() || (*it)->getRTEntry())
        {// selected (or installed) route -> remember
            tocancel.push_back((*it));
        }

        (*it)->setSelected(false);  // unselect all routes

        std::vector<BabelRoute *>::iterator ts;
        for (ts = toselect.begin(); ts != toselect.end(); ++ts)
        {// through all toselect

            if((*ts)->getPrefix() == (*it)->getPrefix())
            {// candidate to selection already exists
                break;
            }
        }

        if(ts != toselect.end())
        {// for this prefix is already selected another route -> compare metric
            if((*it)->metric() < (*ts)->metric())
            {// metric is better than metric of selected route -> switch
                (*ts) = (*it);
            }
        }
        else
        {// first route to this prefix -> add
            toselect.push_back((*it));
        }
    }


    // COMPARE PREVIOUSLY SELECTED AND CANDIDATES
    for (std::vector<BabelRoute *>::iterator tc = tocancel.begin(); tc != tocancel.end(); ++tc)
    {// through all tocancel
        std::vector<BabelRoute *>::iterator ts;
        for (ts = toselect.begin(); ts != toselect.end(); ++ts)
        {// through all toselect
            if((*ts)->getPrefix() == (*tc)->getPrefix())
            {// found candidate for same prefix
                break;
            }
        }

        if(ts != toselect.end() && (*ts)->metric() != 0xFFFF && ((*ts)->getNeighbour() == NULL || isFeasible((*ts)->getPrefix(), (*ts)->getOriginator(), (*ts)->getRDistance())))
        {// for this prefix exists feasible candidate
            if((*ts) != (*tc))
            {// change of selected routes = tc goes from TRUE to FALSE -> remove
                removeFromRT((*tc));
            }
        }
        else
        {// for previously selected prefix do not exists candidate = lost -> send SeqNumReq
            sendSeqnoReqTLV(NULL, new BabelSeqnoReqFtlv(plusmod16((*tc)->getRDistance().getSeqno(), 1), defval::SEQNUMREQ_HOPCOUNT, (*tc)->getOriginator(), (*tc)->getPrefix()), NULL, roughly(SEND_URGENT));

            if(!btt.containShorterCovRoute((*tc)->getPrefix()))
            {// do not exists shorter prefix -> can remove from RT
                removeFromRT((*tc));
            }
        }
    }


    for (std::vector<BabelRoute *>::iterator ts = toselect.begin(); ts != toselect.end(); ++ts)
    {// through all toselect

        if(((*ts)->getNeighbour() == NULL || isFeasible((*ts)->getPrefix(), (*ts)->getOriginator(), (*ts)->getRDistance())) && (*ts)->metric() != 0xFFFF)
        {// local or feasible route, which is not retracted
            (*ts)->setSelected(true);

            if((*ts)->getNeighbour())
            {//route learned from neighbour == no local
                if(!(*ts)->getRTEntry())
                {// not installed in RT yet -> install
                    addToRT((*ts));
                }
                else
                {// already in RT -> try update
                    updateRT((*ts));
                }
            }
        }
        /*
        else if((*ts)->getNeighbour() != NULL && !isFeasible((*ts)->getPrefix(), (*ts)->getOriginator(), (*ts)->getRDistance()) && (*ts)->metric() != 0xFFFF)
        {// candidate is not feasible -> send seqno request - MY TWEAK
            sendSeqnoReqTLV((*ts)->getNeighbour()->getAddress(),
                            (*ts)->getNeighbour()->getInterface(),
                                new BabelSeqnoReqFtlv(plusmod16((*ts)->getRDistance().getSeqno(), 1), defval::SEQNUMREQ_HOPCOUNT, (*ts)->getOriginator(), (*ts)->getPrefix()));
        }*/
    }
}


/**
 * Adds route to Routing Table
 *
 * @param   route   Route to add
 */
void BabelMain::addToRT(BabelRoute *route)
{
    ASSERT(route != NULL);

    if(route->getNeighbour() == NULL)
    {// local route -> already in table
        return;
    }
//XXX: Vesely - Immediate rewrite needed!!!
    if(route->getPrefix().getAddr().getType()==L3Address::IPv6)
    {// IPv6

        IPv6Route *newentry = new IPv6Route(route->getPrefix().getAddr().toIPv6(), route->getPrefix().getLen(), IRoute::ZEBRA);
        //newentry->setRoutingProtocolSource(ANSAIPv6Route::pBABEL);
        //newentry->setAdminDist(ANSAIPv6Route::dBABEL);
        newentry->setMetric(route->metric());
        newentry->setNextHop(route->getNextHop().toIPv6());
        //newentry->setInterface(route->getNeighbour()->getInterface()->getInterfaceId());
        newentry->setInterface(route->getNeighbour()->getInterface()->getInterface());

        //if (rt6->prepareForAddRoute(newentry))
        //{
            rt6->addRoutingProtocolRoute(newentry);
        //    route->setRTEntry(newentry);
        //}
        //else
        //{// route exists with lower administrative distance
        //    delete newentry;
        //}
    }
    else
    {// IPv4
        IPv4Route *newentry = new IPv4Route();
        newentry->setDestination(route->getPrefix().getAddr().toIPv4());
        newentry->setNetmask(IPv4Address::makeNetmask(route->getPrefix().getLen()));
        //newentry->setSource(IPv4Route::ZEBRA);  // Set any source except IFACENETMASK and MANUAL
        //newentry->setRoutingProtocolSource(ANSAIPv4Route::pBABEL);
        //newentry->setAdminDist(ANSAIPv4Route::dBABEL);
        newentry->setMetric(route->metric());
        newentry->setGateway(route->getNextHop().toIPv4());
        newentry->setInterface(route->getNeighbour()->getInterface()->getInterface());

        //if (rt4->prepareForAddRoute(newentry))
        //{
            rt4->addRoute(newentry);
        //    route->setRTEntry(newentry);
        //}
        //else
        //{// route exists with lower administrative distance
        //    delete newentry;
        //}
    }
}

/**
 * Remove route from Routing Table
 *
 * @param   route   Route to remove from RT
 */
void BabelMain::removeFromRT(BabelRoute *route)
{
    if(route == NULL)
    {// delete all routes from RT
       for(std::vector<BabelRoute *>::iterator it = btt.getRoutes().begin(); it != btt.getRoutes().end(); ++it)
       {// for all routes
           if((*it)->getRTEntry())
           {// route with entry in RT
               removeFromRT((*it));
           }
       }
       return;
    }

    if(route->getRTEntry() == NULL)
    {// route is not installed in RT
        return;
    }

    if(route->getPrefix().getAddr().getType()==L3Address::IPv6)
    {// IPv6
        rt6->removeRoute(reinterpret_cast<IPv6Route *>(route->getRTEntry()));
    }
    else
    {// IPv4
        rt4->removeRoute(reinterpret_cast<IPv4Route *>(route->getRTEntry()));
    }

    route->setRTEntry(NULL);
}

/**
 * Updates route in Routing Table
 *
 * @param   route   Route to update
 */
void BabelMain::updateRT(BabelRoute *route)
{
    if(route == NULL)
    {// update all routes in RT
       for(std::vector<BabelRoute *>::iterator it = btt.getRoutes().begin(); it != btt.getRoutes().end(); ++it)
       {// for all routes
           if((*it)->getRTEntry())
           {// route with entry in RT
               updateRT((*it));
           }
       }
       return;
    }

    if(route->getRTEntry() == NULL)
    {// route is not installed in RT
        return;
    }

    if(route->getPrefix().getAddr().getType()==L3Address::IPv6)
    {// IPv6
        IPv6Route *entry = reinterpret_cast<IPv6Route *>(route->getRTEntry());

        entry->setMetric(route->metric());
        entry->setNextHop(route->getNextHop().toIPv6());
    }
    else
    {// IPv4
        IPv4Route *entry = reinterpret_cast<IPv4Route *>(route->getRTEntry());

        entry->setMetric(route->metric());
        entry->setGateway(route->getNextHop().toIPv4());
    }
}

/**
 * Deletes ToAck
 *
 * @param   todel   ToAck to deletation
 */
void BabelMain::deleteToAck(BabelToAck *todel)
{
    std::vector<BabelToAck *>::iterator it;

    if(todel == NULL)
    {// nothing to delete
        return;
    }

    for (it = ackwait.begin(); it != ackwait.end();)
    {// find buffer
        if((*it) == todel)
        {// found -> delete
            //cancelAndDelete((*it)->getResendTimer());
            delete (*it);
            it = ackwait.erase(it);
            return;
        }
        else
        {// do not delete -> get next
            ++it;
        }
    }
}

/**
 * Deletes all ToAcks
 */
void BabelMain::deleteToAcks()
{
    std::vector<BabelToAck *>::iterator it;

    for (it = ackwait.begin(); it != ackwait.end(); ++it)
    {// delete all toAcks
        //cancelAndDelete((*it)->getResendTimer());
        delete (*it);
    }
    ackwait.clear();
}

/**
 * Find ToAck
 *
 * @param   n   Nonce of ACK
 *
 * @return Pointer to ToAck if found, NULL otherwise
 */
BabelToAck *BabelMain::findToAck(uint16_t n)
{
    std::vector<BabelToAck *>::iterator it;

    for (it = ackwait.begin(); it != ackwait.end(); ++it)
    {// through all nodes search for same nonce
        if((*it)->getNonce() == n)
        {// found same
            return (*it);
        }
    }

    return NULL;
}

/**
 * Checks and resends ToAck
 *
 * @param   toack   ToAck to resend
 */
void BabelMain::checkAndResendToAck(BabelToAck *toack)
{
    ASSERT(toack != NULL);

    if(toack->decResendNum() >= 0 && toack->dstNodesSize() > 0)
    {// resend number is not exhausted and some nodes did not acknowledged receiving -> resend
        sendMessage(toack->getDst(), toack->getOutIface(), toack->getMsg()->dup());

        resetTimer(toack->getResendTimer(), CStoS(toack->getOutIface()->getHInterval() / 2));
    }
    else
    {// resend number is  exhausted or all nodes acknowledged receiving -> delete
        deleteToAck(toack);
    }
}

/**
 * Generates nonce
 *
 * @return  Unique nonce
 */
uint16_t BabelMain::generateNonce()
{
    uint16_t nonce;

    do
    {// find unused nonce
        nonce = intuniform(0,UINT16_MAX);
    } while(findToAck(nonce) != NULL);

    return nonce;
}


/**
 * Removes neighbours on interfaces
 *
 * @param   iface   Interface on which delete all neighbours
 *
 * @return  True if topology table has changed
 */
bool BabelMain::removeNeighboursOnIface(BabelInterface *iface)
{
    bool changed = false;

    for (std::vector<BabelNeighbour *>::iterator it = bnt.getNeighbours().begin(); it != bnt.getNeighbours().end();)
    {// through all neighbours
        if((*it)->getInterface() == iface)
        {// found neighbour on same interface
            bpsrt.removePenSRsByNeigh((*it));
            changed = removeRoutesByNeigh((*it)) || changed;
            delete (*it);
            it = bnt.getNeighbours().erase(it);
        }
        else
        {// do not delete -> get next
            ++it;
        }
    }

    return changed;
}

/**
 * Removes routes by neighbour
 *
 * @param   neigh   Neighbour from which remove all routes
 *
 * @return  True if topology table has changed
 */
bool BabelMain::removeRoutesByNeigh(BabelNeighbour *neigh)
{
    std::vector<BabelRoute *>::iterator it;
    bool changed = false;

    for (it = btt.getRoutes().begin(); it != btt.getRoutes().end();)
    {// through all routes
        if((*it)->getNeighbour() == neigh)
        {// found same
            if((*it)->getRTEntry())
            {
                removeFromRT((*it));
            }
            delete (*it);
            it = btt.getRoutes().erase(it);
            changed = true;
        }
        else
        {// do not delete -> get next
            ++it;
        }
    }

    return changed;
}
}
