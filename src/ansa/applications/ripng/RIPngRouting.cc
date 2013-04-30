// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
/**
* @file RIPngInterface.cc
* @author Jiri Trhlik (mailto:), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
* @brief
* @detail
*/

#include "RIPngRouting.h"

#include "IPv6InterfaceData.h"
#include "IPv6ControlInfo.h"

#include "deviceConfigurator.h"

Define_Module(RIPngRouting);

std::ostream& operator<<(std::ostream& os, const RIPng::RoutingTableEntry& e)
{
    os << e.RIPngInfo();
    return os;
};

RIPngRouting::RIPngRouting()
{
    regularUpdateTimer = NULL;
    triggeredUpdateTimer = NULL;
}

RIPngRouting::~RIPngRouting()
{
    deleteTimer(regularUpdateTimer);
    deleteTimer(triggeredUpdateTimer);

    //removeAllRoutingTableEntries();
    RIPng::RoutingTableEntry *routingTableEntry;
    for (RoutingTableIt it = routingTable.begin(); it != routingTable.end(); it++)
    {
        routingTableEntry = (*it);
        deleteTimer(routingTableEntry->getGCTimer());
        deleteTimer(routingTableEntry->getTimer());
        delete routingTableEntry;
    }
    routingTable.clear();

    //removeAllEnabledInterfaces();
    unsigned long intCount = getEnabledInterfacesCount();
    for (unsigned long i = 0; i < intCount; ++i)
    {
        delete enabledInterfaces[i];
        delete sockets[i];
    }
    sockets.clear();
    enabledInterfaces.clear();
}

//
//
//-- RIPNG ROUTING TABLE METHODS
//
//
RIPng::RoutingTableEntry* RIPngRouting::getRoutingTableEntry(const IPv6Address &prefix, int prefixLength)
{
    RIPng::RoutingTableEntry *route = NULL;
    for (RoutingTableIt it=routingTable.begin(); it!=routingTable.end(); it++)
    {
        if ((*it)->getDestPrefix()==prefix && (*it)->getPrefixLength()==prefixLength)
        {
            route = (*it);
            break;
        }
    }

    return route;
}

void RIPngRouting::addRoutingTableEntry(RIPng::RoutingTableEntry* entry, bool createTimers)
{
    if (createTimers == true)
    {
        RIPngTimer *timer = createAndStartTimer(RIPNG_ROUTE_TIMEOUT, routeTimeout);
        timer->setIPv6Prefix(entry->getDestPrefix());
        timer->setPrefixLen(entry->getPrefixLength());
        entry->setTimer(timer);

        RIPngTimer *GCTimer = createTimer(RIPNG_ROUTE_GARBAGE_COLECTION_TIMEOUT);
        GCTimer->setIPv6Prefix(entry->getDestPrefix());
        GCTimer->setPrefixLen(entry->getPrefixLength());
        entry->setGCTimer(GCTimer);
    }

    routingTable.push_back(entry);

    //Do not try to add directly connected routes to the "global" routing table
    if (!entry->getNextHop().isUnspecified())
        addRoutingTableEntryToGlobalRT(entry);

    ++numRoutes;
}
void RIPngRouting::removeRoutingTableEntry(IPv6Address &prefix, int prefixLength)
{
    for (RoutingTableIt it=routingTable.begin(); it!=routingTable.end(); it++)
    {
        if ((*it)->getDestPrefix()==prefix && (*it)->getPrefixLength()==prefixLength)
        {
            removeRoutingTableEntry(it);
            break;
        }
    }
}

void RIPngRouting::removeRoutingTableEntry(RoutingTableIt it)
{
    ASSERT(it != routingTable.end());

    // delete timers
    deleteTimer((*it)->getGCTimer());
    deleteTimer((*it)->getTimer());
    removeRoutingTableEntryFromGlobalRT((*it));
    // delete routing table entry
    delete (*it);
    routingTable.erase(it);

    --numRoutes;
}

void RIPngRouting::removeAllRoutingTableEntries()
{
    RIPng::RoutingTableEntry *routingTableEntry;

    for (RoutingTableIt it = routingTable.begin(); it != routingTable.end(); it++)
    {
        routingTableEntry = (*it);
        deleteTimer(routingTableEntry->getGCTimer());
        deleteTimer(routingTableEntry->getTimer());
        removeRoutingTableEntryFromGlobalRT((*it));
        delete routingTableEntry;
    }

    routingTable.clear();
}

void RIPngRouting::updateRoutingTableEntry(RIPng::RoutingTableEntry *routingTableEntry, RIPngRTE &rte, int sourceIntId, IPv6Address &sourceAddr)
{
    const IPv6Address &nextHop = routingTableEntry->getNextHop();
    int newMetric = rte.getMetric();
    ++newMetric;
    int oldMetric = routingTableEntry->getMetric();

    RIPng::RoutingTableEntry *routingTableEntryInGlobalRT = routingTableEntry->getCopy();

    if (nextHop == sourceAddr)
    {// RTE is from the same router
        RIPngTimer *routeTimer = routingTableEntry->getTimer();

        if (newMetric < infinityMetric)
            resetTimer(routeTimer, routeTimeout);

        if (newMetric != oldMetric)
        {
            if (newMetric >= infinityMetric && oldMetric < infinityMetric)
            {//Invalidate route if isn't already invalidated
                // bSendTriggeredUpdateMessage is set in startRouteDeletionProcess() function
                cancelTimer(routeTimer);
                startRouteDeletionProcess(routingTableEntry);
            }
            else if (newMetric < infinityMetric)
            {
                // route is from the same router, always set the metric and the change flag
                routingTableEntry->setMetric(newMetric);
                routingTableEntry->setChangeFlag();

                // stop garbage collection timer
                RIPngTimer *GCTimer = routingTableEntry->getGCTimer();
                ASSERT(GCTimer != NULL);
                if (GCTimer->isScheduled())
                {
                   cancelTimer(GCTimer);
                   // route was deleted from "global routing table" in the route deletion process, add it again
                   addRoutingTableEntryToGlobalRT(routingTableEntry);
                }
                else if (routingTableEntryInGlobalRT != NULL)
                {
                    routingTableEntryInGlobalRT->setMetric(newMetric);
                }

                bSendTriggeredUpdateMessage = true;
            }
        }
    }
    else
    {
        if (newMetric < oldMetric)
        {
            routingTableEntry->setMetric(newMetric);
            routingTableEntry->setNextHop(sourceAddr);
            routingTableEntry->setInterfaceId(sourceIntId);
            resetTimer(routingTableEntry->getTimer(), routeTimeout);
            // stop garbage collection timer
            RIPngTimer *GCTimer = routingTableEntry->getGCTimer();
            ASSERT(GCTimer != NULL);
            if (GCTimer->isScheduled())
            {
                cancelTimer(GCTimer);
                // route was deleted from "global routing table" in the route deletion process, add it again
                addRoutingTableEntryToGlobalRT(routingTableEntry);
            }
            else if (routingTableEntryInGlobalRT != NULL)
            {
                routingTableEntryInGlobalRT->setInterfaceIdSilent(sourceIntId);
                routingTableEntryInGlobalRT->setNextHopSilent(sourceAddr);
                routingTableEntryInGlobalRT->setMetric(newMetric);
            }
            routingTableEntry->setChangeFlag();

            bSendTriggeredUpdateMessage = true;
        }
        // TODO: OPTIMALIZATION: else if (routeTimer is nearly expired for the routingTableEntry)
    }
}

//
//
//-- RIPNG INTERFACES METHODS
//
//
int RIPngRouting::getEnabledInterfaceIndexById(int id)
{
    int i = 0, size = getEnabledInterfacesCount();
    while (i < size && getEnabledInterface(i)->getId() != id) i++;
    return i == size ? -1 : i;
}

void RIPngRouting::addEnabledInterface(RIPng::Interface *interface)
{
    enabledInterfaces.push_back(interface);
    sockets.push_back(createAndSetSocketForInt(interface));
}

void RIPngRouting::removeEnabledInterface(unsigned long i)
{
    delete enabledInterfaces[i];
    enabledInterfaces.erase(enabledInterfaces.begin() + i);
    sockets[i]->close();
    delete sockets[i];
    sockets.erase(sockets.begin() + i);

    //if (i == 0)
        //sockets[0]->joinMulticastGroup(RIPngAddress, -1);
}

void RIPngRouting::removeAllEnabledInterfaces()
{
    unsigned long intCount = getEnabledInterfacesCount();
    for (unsigned long i = 0; i < intCount; ++i)
    {
        delete enabledInterfaces[i];
        sockets[i]->close();
        delete sockets[i];
    }
    sockets.clear();
    enabledInterfaces.clear();
}

//
//
//-- GENERAL METHODS
//
//
void RIPngRouting::showRoutingTable()
{
    RoutingTableIt it;
    RIPng::RoutingTableEntry *routingTableEntry;

    ev << routerText << endl;

    for (it = routingTable.begin(); it != routingTable.end(); it++)
    {
        routingTableEntry = (*it);
        ev << routingTableEntry->info() << endl;
    }
}

RIPngMessage *RIPngRouting::createMessage()
{
    char msgName[32] = "RIPngMessage";

    RIPngMessage *msg = new RIPngMessage(msgName);
    return msg;
}

UDPSocket *RIPngRouting::createAndSetSocketForInt(RIPng::Interface* interface)
{
    UDPSocket *socket = new UDPSocket();
    socket->setOutputGate(gate("udpOut"));
    //so every RIPng message sent from RIPng interface uses correct link-local source address
    socket->bind(ift->getInterfaceById(interface->getId())->ipv6Data()->getLinkLocalAddress(), RIPngPort);

    int timeToLive = par("timeToLive");
    if (timeToLive != -1)
        socket->setTimeToLive(timeToLive);

    return socket;
}

RIPng::Interface *RIPngRouting::enableRIPngOnInterface(const char *interfaceName)
{
    ev << "Enabling RIPng on " << interfaceName << routerText << endl;

    InterfaceEntry *interface = ift->getInterfaceByName(interfaceName);
    int interfaceId = interface->getInterfaceId();
    RIPng::Interface *RIPngInterface = new RIPng::Interface(interfaceId);
    // add interface to local RIPng interface table
    addEnabledInterface(RIPngInterface);

    return RIPngInterface;
}

void RIPngRouting::setInterfacePassiveStatus(RIPng::Interface *RIPngInterface, bool status)
{
    if (status == true)
        ev << "Setting RIPng passive interface (interface id: " << RIPngInterface->getId() << ")." << routerText << endl;

    if (status)
        RIPngInterface->enablePassive();
    else
        RIPngInterface->disablePassive();
}

void RIPngRouting::setInterfaceSplitHorizon(RIPng::Interface *RIPngInterface, bool status)
{
    if (status)
        RIPngInterface->enableSplitHorizon();
    else
        RIPngInterface->disableSplitHorizon();
}

void RIPngRouting::setInterfacePoisonReverse(RIPng::Interface *RIPngInterface, bool status)
{
    if (status)
        RIPngInterface->enablePoisonReverse();
    else
        RIPngInterface->disablePoisonReverse();
}

void RIPngRouting::addRoutingTableEntryToGlobalRT(RIPng::RoutingTableEntry* entry)
{
    RIPng::RoutingTableEntry *newEntry = new RIPng::RoutingTableEntry(*entry);
    if (rt->prepareForAddRoute(newEntry))
    {
        rt->addRoutingProtocolRoute(newEntry);
        entry->setCopy(newEntry);
    }
    else
    {// route exists with lower administrative distance
        delete newEntry;
    }
}

void RIPngRouting::removeRoutingTableEntryFromGlobalRT(RIPng::RoutingTableEntry* entry)
{
    if (entry->getCopy() != NULL)
    // corresponding route from "global routing table" to the entry from "RIPng routing table"
        rt->removeRoute(entry->getCopy());
}

//
//
//-- OUTPUT PROCESSING
//
//
void RIPngRouting::sendRegularUpdateMessage()
{
    int numInterfaces = getEnabledInterfacesCount();
    RIPng::Interface *interface;

    // sent update on every interface, where is enabled RIPng and that interface is not passive
    for (int i = 0; i < numInterfaces; i++)
    {
        interface = getEnabledInterface(i);
        if (interface->isPassive())
            // do not send updates out of the passive interface
            continue;

        RIPngMessage *msg = makeUpdateMessageForInterface(interface, false);
        if (msg != NULL)
        // no rtes to send
            sendMessage(msg, RIPngAddress, RIPngPort, i, false);
    }

    //reset Route Change Flags
    clearRouteChangeFlags();

    bSendTriggeredUpdateMessage = false;
}

void RIPngRouting::sendTriggeredUpdateMessage()
{
    if (triggeredUpdateTimer->isScheduled())
    // method will be called again when regularUpdateTimer expired
        return;

   // random timeout from 1 to 5 seconds
   simtime_t triggeredUpdateTimeout = uniform(1, 5);
   if (regularUpdateTimer->getArrivalTime() - simTime() < triggeredUpdateTimeout)  // regulardUpdateTimer can't be NULL
   // regular update message is going to be sent soon
       return;

    int numInterfaces = getEnabledInterfacesCount();
    RIPng::Interface *interface;

    // sent update on every interface, where is enabled RIPng and that interface is not passive
    for (int i = 0; i < numInterfaces; i++)
    {
        interface = getEnabledInterface(i);
        if (interface->isPassive())
            // do not send updates out of the passive interface
            continue;

        RIPngMessage *msg = makeUpdateMessageForInterface(interface, true);
        if (msg != NULL)
            sendMessage(msg, RIPngAddress, RIPngPort, i, false);
    }

    //reset Route Change Flags
    clearRouteChangeFlags();

    bSendTriggeredUpdateMessage = false;

    resetTimer(triggeredUpdateTimer, uniform(1, 5));
}

void RIPngRouting::sendDelayedTriggeredUpdateMessage()
{
    // we are using delayed triggered update message because if one route went down,
    // more than one route (with the next hop using that unavailable route) TIMEOUT can EXPIRE in the same time -
    // this way we prevent to send multiple triggered update messages containing just one route update
    // for this we can use triggered update timer

    bSendTriggeredUpdateMessage = true;

    if (!triggeredUpdateTimer->isScheduled())
    {
        // random timeout from 1 to 5 seconds
        simtime_t triggeredUpdateTimeout = uniform(1, 5);
        resetTimer(triggeredUpdateTimer, triggeredUpdateTimeout);
    }
    // else - do nothing, a triggered update is already planned
}

RIPngMessage *RIPngRouting::makeUpdateMessageForInterface(RIPng::Interface *interface, bool changed)
{
    int size;
    std::vector<RIPngRTE> rtes;

    getRTEs(rtes, interface, changed);

    size = rtes.size();
    if (size <= 0)
        // there's no RTE to send from this interface (no message will be created)
        return NULL;

    RIPngMessage *msg = createMessage();
    msg->setCommand(RIPngResponse);
    msg->setRtesArraySize(size);
    // set RTEs to response
    for(int j = 0; j < size; j++)
        msg->setRtes(j, rtes[j]);

    return msg;
}

void RIPngRouting::sendMessage(RIPngMessage *msg, IPv6Address &addr, int port, unsigned long enabledInterfaceIndex, bool globalSourceAddress)
{
    ASSERT(enabledInterfaceIndex < getEnabledInterfacesCount());
    int outInterface = getEnabledInterface(enabledInterfaceIndex)->getId();
    if (globalSourceAddress)
    {// "uses" global-unicast address as the source address
        globalSocket.sendTo(msg, addr, port, outInterface);
    }
    else
    {
        sockets[enabledInterfaceIndex]->sendTo(msg, addr, port, outInterface);
    }
}

void RIPngRouting::sendAllRoutesRequest()
{
    int numInterfaces = getEnabledInterfacesCount();
    RIPng::Interface *interface;

    RIPngRTE rte = RIPngRTE();
    rte.setIPv6Prefix(IPv6Address()); // IPv6 Address ::0
    rte.setMetric(16);
    rte.setPrefixLen(0);
    rte.setRouteTag(0);

    // sent update on every interface, where is enabled RIPng and that interface is not passive
    for (int i = 0; i < numInterfaces; i++)
    {
        interface = getEnabledInterface(i);
        if (interface->isPassive())
            // do not send request out of the passive interface
            continue;

        RIPngMessage *msg = createMessage();

        msg->setCommand(RIPngRequest);
        msg->setRtesArraySize(1);
        msg->setRtes(0, rte);

        sendMessage(msg, RIPngAddress, RIPngPort, i, false);
    }
}

void RIPngRouting::clearRouteChangeFlags()
{
    RIPng::RoutingTableEntry *routingTableEntry;
    RoutingTableIt it;

    for (it = routingTable.begin(); it != routingTable.end(); it++)
    {
        routingTableEntry = (*it);
        routingTableEntry->clearChangeFlag();
    }
}

void RIPngRouting::getRTEs(std::vector<RIPngRTE> &rtes, RIPng::Interface *interface, bool onlyChanged)
{
    RIPng::RoutingTableEntry *routingTableEntry;
    RoutingTableIt it;
    bool splitHorizon = false;
    bool poisonReverse = false;
    int interfaceId = -1;


    if (interface != NULL)
    {
        interfaceId = interface->getId();
        splitHorizon = interface->isSplitHorizon();
        poisonReverse = interface->isPoisonReverse();
    }

    bool setInfMetric;
    for (it = routingTable.begin(); it != routingTable.end(); it++)
    {
        setInfMetric = false;
        routingTableEntry = (*it);
        if (splitHorizon && (routingTableEntry->getInterfaceId() == interfaceId))
        {
            if (poisonReverse)
            // split horizon with poison reverse
                setInfMetric = true;
            else
            // split horizon
                continue;
        }

        if (!onlyChanged || (onlyChanged && routingTableEntry->isChangeFlagSet()))
        {
            RIPngRTE rte = makeRTEFromRoutingTableEntry(routingTableEntry);
            if (setInfMetric)
                rte.setMetric(infinityMetric);
            rtes.push_back(rte);
        }
    }
}

RIPngRTE RIPngRouting::makeRTEFromRoutingTableEntry(RIPng::RoutingTableEntry *routingTableEntry)
{
    RIPngRTE rte;
    // create RTE for message to neighbor
    rte.setPrefixLen(routingTableEntry->getPrefixLength());
    rte.setIPv6Prefix(routingTableEntry->getDestPrefix());
    rte.setMetric(routingTableEntry->getMetric());
    rte.setRouteTag(routingTableEntry->getRouteTag());

    return rte;
}

//
//
//-- INPUT PROCESSING
//
//
void RIPngRouting::handleRIPngMessage(RIPngMessage *msg)
{
    UDPDataIndication *controlInfo = check_and_cast<UDPDataIndication *>(msg->getControlInfo());
    IPv6Address sourceAddr = controlInfo->getSrcAddr().get6();
    IPv6Address destAddr = controlInfo->getDestAddr().get6();
    int sourcePort = controlInfo->getSrcPort();
    int sourceInterfaceId = controlInfo->getInterfaceId();

    int ripngIntInd = getEnabledInterfaceIndexById(sourceInterfaceId);
    if (ripngIntInd < 0)
    {//message is from an interface with disabled RIPng
        delete msg;
        return;
    }

    EV << "RIPng: Received packet: " << UDPSocket::getReceivedPacketInfo(msg) << endl;
    int command = msg->getCommand();
    int version = msg->getVersion();
    if (version != 1)
        EV << "This implementation of RIPng does not support version '" << version << "' of this protocol." << endl;

    if (command == RIPngRequest)
    {
        handleRequest(msg, sourcePort, sourceAddr, destAddr, ripngIntInd);
    }
    else if (command == RIPngResponse)
    {
        handleResponse(msg, sourceInterfaceId, sourceAddr);
    }

    delete msg;
}

//
//
//-- RESPONSE PROCESSING
//
//
void RIPngRouting::handleResponse(RIPngMessage *response, int srcInt, IPv6Address &srcAddr)
{
    if (!checkMessageValidity(response))
        return;
    EV << "RIPng message: RIPng - Response" << endl;
    processRTEs(response, srcInt, srcAddr);
}

bool RIPngRouting::checkMessageValidity(RIPngMessage *response)
{
    UDPDataIndication *controlInfo = check_and_cast<UDPDataIndication *>(response->getControlInfo());

    // is from RIPng port
    if (controlInfo->getSrcPort() != RIPngPort)
        return false;

    // source addr. is link-local
    IPv6Address sourceAddr = controlInfo->getSrcAddr().get6();
    if (!sourceAddr.isLinkLocal())
        return false;

    // source addr. is not from this device
    if (rt->isLocalAddress(sourceAddr))
        return false;

    // hop-count is 255
    if (controlInfo->getTtl() != 255)
        return false;

    return true;
}

void RIPngRouting::processRTEs(RIPngMessage *response, int sourceIntId, IPv6Address &sourceAddr)
{
    unsigned int rtesSize = response->getRtesArraySize();
    RIPngRTE rte;

    // if startRouteDeletionProcess() was called we dont want to call sendTriggeredUpdateMessage() in that function,
    // so we'll call sendTriggeredUpdateMessage() after "all startRouteDeletionProcess()"
    bBlockTriggeredUpdateMessage = true;

    //Process every RTE
    for (unsigned int i = 0; i < rtesSize; i++)
    {
        rte = response->getRtes(i);
        EV << "RTE [" << i << "]: " << rte.getIPv6Prefix() << "/" << int(rte.getPrefixLen()) << endl;
        processRTE(rte, sourceIntId, sourceAddr);
    }

    if (bSendTriggeredUpdateMessage)
    {
        sendTriggeredUpdateMessage();
    }

    bBlockTriggeredUpdateMessage = false;
}

void RIPngRouting::processRTE(RIPngRTE &rte, int sourceIntId, IPv6Address &sourceAddr)
{
    checkAndLogRTE(rte, sourceAddr);

    IPv6Address prefix = rte.getIPv6Prefix();
    int prefixLen = rte.getPrefixLen();

    // Check if a route with the prefix exists
    RIPng::RoutingTableEntry *routingTableEntry = getRoutingTableEntry(prefix, prefixLen);

    if (routingTableEntry != NULL)
    {// Update Routing Table Entry
        updateRoutingTableEntry(routingTableEntry, rte, sourceIntId, sourceAddr);
    }
    else
    {// Create and add new Routing Table Entry
        int metric = rte.getMetric();
        ++metric;

        if (metric < infinityMetric)
        {
            RIPng::RoutingTableEntry *route = new RIPng::RoutingTableEntry(prefix, prefixLen);
            route->setInterfaceId(sourceIntId);
            route->setNextHop(sourceAddr);
            route->setMetric(metric);
            route->setChangeFlag();

            addRoutingTableEntry(route);

            bSendTriggeredUpdateMessage = true;
        }
    }
}

bool RIPngRouting::checkAndLogRTE(RIPngRTE &rte, IPv6Address &sourceAddr)
{
    // prefix is valid (not multicast, link-local)
    // prefix len. is valid (0-128)
    // metric is valid (0-16)
    if (!rte.getIPv6Prefix().isGlobal() ||
         rte.getPrefixLen() > 128 ||
         rte.getMetric() > 16)
    {
        EV << "Bad RTE from: " << sourceAddr << endl;
        return false;
    }

    return true;
}

//
//
//-- REQUEST PROCESSING
//
//
void RIPngRouting::handleRequest(RIPngMessage *request, int srcPort, IPv6Address &srcAddr, IPv6Address &destAddr, unsigned long ripngIntInd)
{
    ASSERT(ripngIntInd < getEnabledInterfacesCount());
    RIPng::Interface *ripngInt = getEnabledInterface(ripngIntInd);

    unsigned int rteNum = request->getRtesArraySize();
    std::vector<RIPngRTE> responseRtes;

    if (rteNum == 1)
    {// could be a request for all routes
        EV << "RIPng message: RIPng - General Request" << endl;
        RIPngRTE &rte = request->getRtes(0);
        if (rte.getIPv6Prefix().isUnspecified() &&
            rte.getPrefixLen() == 0 &&
            rte.getMetric() == infinityMetric &&
            rte.getRouteTag() == 0)
        {
            getRTEs(responseRtes, ripngInt, false);
        }
    }
    else
    {
        RIPng::RoutingTableEntry *routingTableEntry;
        EV << "RIPng message: RIPng - Request" << endl;
        for (unsigned int i = 0; i < rteNum; i++)
        {
            RIPngRTE rte = request->getRtes(i);
            routingTableEntry = getRoutingTableEntry(rte.getIPv6Prefix(), rte.getPrefixLen());
            EV << "RTE [" << i << "]: " << rte.getIPv6Prefix() << "/" << int(rte.getPrefixLen()) << endl;
            if (routingTableEntry != NULL)
            {// match for the requested rte
                responseRtes.push_back(makeRTEFromRoutingTableEntry(routingTableEntry));
            }
            else
            {
                rte.setMetric(infinityMetric);
                responseRtes.push_back(rte);
            }
        }
    }

    int size = responseRtes.size();
    //if (size <= 0)
        //break;

    RIPngMessage *response = createMessage();
    response->setCommand(RIPngResponse);
    response->setRtesArraySize(size);
    // set RTEs to response
    for(int j = 0; j < size; j++)
        response->setRtes(j, responseRtes[j]);

    if (destAddr == RIPngAddress && srcPort == RIPngPort)
        sendMessage(response, srcAddr, srcPort, ripngIntInd, false);
    else
        sendMessage(response, srcAddr, srcPort, ripngIntInd, true);
}

//
//
//-- TIMEOUTS
//
//
RIPngTimer *RIPngRouting::createTimer(int timerKind)
{
    RIPngTimer *timer = new RIPngTimer();
    timer->setTimerKind(timerKind);

    return timer;
}

RIPngTimer *RIPngRouting::createAndStartTimer(int timerKind, simtime_t timerLen)
{
    RIPngTimer *timer = createTimer(timerKind);

    scheduleAt(simTime() + timerLen, timer);

    return timer;
}

void RIPngRouting::resetTimer(RIPngTimer *timer, simtime_t timerLen)
{
    ASSERT(timer != NULL);
    if (timer->isScheduled())
        cancelEvent(timer);

    scheduleAt(simTime() + timerLen, timer);
}

void RIPngRouting::cancelTimer(RIPngTimer *timer)
{
    if (timer != NULL)
    {
        if (timer->isScheduled())
            cancelEvent(timer);
    }
}

void RIPngRouting::deleteTimer(RIPngTimer *timer)
{
    if (timer != NULL)
    {
        if (timer->isScheduled())
            cancelEvent(timer);

        delete timer;
    }
}

void RIPngRouting::handleTimer(RIPngTimer *msg)
{
    int type = msg->getTimerKind();

    switch (type)
    {
        case RIPNG_GENERAL_UPDATE :
            handleRegularUpdateTimer();
            break;
        case RIPNG_TRIGGERED_UPDATE :
            handleTriggeredUpdateTimer();
            break;
        case RIPNG_ROUTE_TIMEOUT :
            startRouteDeletionProcess(msg);
            break;
        case RIPNG_ROUTE_GARBAGE_COLECTION_TIMEOUT :
            deleteRoute(msg);
            break;
        default:
            break;
    }
}

void RIPngRouting::handleRegularUpdateTimer()
{
     // send regular update message
    sendRegularUpdateMessage();
    // plan next regular update
    resetTimer(regularUpdateTimer, regularUpdateTimeout);
}

void RIPngRouting::handleTriggeredUpdateTimer()
{
    if (bSendTriggeredUpdateMessage)
        sendTriggeredUpdateMessage();
}

void RIPngRouting::startRouteDeletionProcess(RIPngTimer *timer)
{
    IPv6Address &prefix = timer->getIPv6Prefix();
    int prefixLen = timer->getPrefixLen();
    RIPng::RoutingTableEntry *routingTableEntry = getRoutingTableEntry(prefix, prefixLen);

    startRouteDeletionProcess(routingTableEntry);
}

void RIPngRouting::startRouteDeletionProcess(RIPng::RoutingTableEntry *routingTableEntry)
{
    ASSERT(routingTableEntry != NULL);
    routingTableEntry->setMetric(infinityMetric);
    routingTableEntry->setChangeFlag();

    RIPngTimer *GCTimer = routingTableEntry->getGCTimer();
    // (re)set the timer
    resetTimer(GCTimer, routeGarbageCollectionTimeout);

    removeRoutingTableEntryFromGlobalRT(routingTableEntry);

    if (bBlockTriggeredUpdateMessage)
    // if response is processing and a rte with the metric of 16 was received, wait until all the RTEs are proccesed
        bSendTriggeredUpdateMessage = true;
    else
        sendDelayedTriggeredUpdateMessage();
}

void RIPngRouting::deleteRoute(RIPngTimer *timer)
{
    IPv6Address &prefix = timer->getIPv6Prefix();
    int prefixLen = timer->getPrefixLen();

    removeRoutingTableEntry(prefix, prefixLen);
}

//
//
//-- OVERRIDDEN METHODS
//
//
void RIPngRouting::initialize(int stage)
{
    if (stage != 3)
        return;

    // get the hostname
    cModule *containingMod = findContainingNode(this);
    if (!containingMod)
        hostName = "";
    else
        hostName = containingMod->getFullName();

    routerText = " (Router " + hostName + ") ";

    bSendTriggeredUpdateMessage = false;
    bBlockTriggeredUpdateMessage = false;

    // access to the routing and interface table
    rt = ANSARoutingTable6Access().get();
    ift = InterfaceTableAccess().get();
    // subscribe for changes in the device
    nb = NotificationBoardAccess().get();
    nb->subscribe(this, NF_INTERFACE_STATE_CHANGED);
    nb->subscribe(this, NF_IPv6_ROUTE_DELETED);

    numRoutes = 0;
    WATCH(numRoutes);

    WATCH_PTRVECTOR(routingTable);

    const char *RIPngAddressString = par("RIPngAddress");
    RIPngAddress = IPv6Address(RIPngAddressString);
    RIPngPort = par("RIPngPort");

    connNetworkMetric = par("connectedNetworkMetric");
    infinityMetric = par("infinityMetric");

    routeTimeout = par("routeTimeout").doubleValue();
    routeGarbageCollectionTimeout = par("routeGarbageCollectionTimeout").doubleValue();
    regularUpdateTimeout = par("regularUpdateInterval").doubleValue();

    // get deviceId
    deviceId = par("deviceId");

    // read the RIPng process configuration
    DeviceConfigurator *devConf = ModuleAccess<DeviceConfigurator>("deviceConfigurator").get();
    devConf->loadRIPngConfig(this);

    globalSocket.setOutputGate(gate("udpOut"));
    globalSocket.bind(RIPngPort);
    globalSocket.joinMulticastGroup(RIPngAddress, -1);

    // start REGULAR UPDATE TIMER
    regularUpdateTimer = createAndStartTimer(RIPNG_GENERAL_UPDATE, regularUpdateTimeout);
    triggeredUpdateTimer = createTimer(RIPNG_TRIGGERED_UPDATE);

    sendAllRoutesRequest();
}

void RIPngRouting::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {// timers
        handleTimer(check_and_cast<RIPngTimer*> (msg));
    }
    else if (msg->getKind() == UDP_I_DATA)
    {// process incoming message
        handleRIPngMessage(check_and_cast<RIPngMessage*> (msg));
    }
    else if (msg->getKind() == UDP_I_ERROR)
    {
        ev << "Ignoring UDP error report" << endl;
        delete msg;
    }
    else
    {
        error("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }

    if (ev.isGUI())
    {
        char buf[40];
        sprintf(buf, "%d routes", numRoutes);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

void RIPngRouting::receiveChangeNotification(int category, const cObject *details)
{
   // ignore notifications during initialization
   if (simulation.getContextType() == CTX_INITIALIZE)
       return;

   Enter_Method_Silent();
   printNotificationBanner(category, details);

   if (category == NF_INTERFACE_STATE_CHANGED)
   {
       InterfaceEntry *interfaceEntry = check_and_cast<InterfaceEntry*>(details);
       int interfaceEntryId = interfaceEntry->getInterfaceId();

       // an interface went down
       if (interfaceEntry->isDown())
       {
           // delete interface from ripng interfaces
           int size = getEnabledInterfacesCount();
           RIPng::Interface* ripngInterface;
           bool alreadyDisabled = true;

           for (int i = 0; i < size; i++)
           {
               ripngInterface = getEnabledInterface(i);
               if (ripngInterface->getId() == interfaceEntryId)
               {
                   alreadyDisabled = false;
                   removeEnabledInterface(i);
                   break;
               }
           }

           if (!alreadyDisabled)
           {
               bBlockTriggeredUpdateMessage = true;

               // delete associated routes from ripng routing table
               RoutingTableIt it;
               for (it = routingTable.begin(); it != routingTable.end(); ++it)
               {
                   if ((*it)->getInterfaceId() == interfaceEntryId)
                   {
                       if ((*it)->getNextHop().isUnspecified())
                       {// directly connected
                           (*it)->setMetric(infinityMetric);
                           (*it)->setChangeFlag();
                           /* XXX: directly connected routes have to remain in the RIPng routing table
                            if the interface will go up again (should be changed in the future --
                            route should be deleted and added when the interface go up --, but right now,
                            the INET interface do not provide length of the prefix for the IPv6
                           address, which is configured on that interface -> problem)*/
                           bSendTriggeredUpdateMessage = true;
                       }
                       else
                       {//act as route timeout just expired
                           cancelTimer((*it)->getTimer());
                           startRouteDeletionProcess((*it));
                       }
                   }
               }

               if (bSendTriggeredUpdateMessage)
               {
                   sendTriggeredUpdateMessage();
               }

               bBlockTriggeredUpdateMessage = false;
           }
       }
       else if (!interfaceEntry->isDown())
       {
           bool alreadyEnabled = false;
           int size = getEnabledInterfacesCount();
           RIPng::Interface* ripngInterface;

           for (int i = 0; i < size; i++)
           {
               ripngInterface = getEnabledInterface(i);
               if (ripngInterface->getId() == interfaceEntryId)
               {
                   alreadyEnabled = true;
                   break;
               }
           }

           if (!alreadyEnabled)
           {
               // add interface to ripng interfaces
               enableRIPngOnInterface(interfaceEntry->getName());

               bBlockTriggeredUpdateMessage = true;

               // delete associated routes from ripng routing table
               RoutingTableIt it;
               for (it = routingTable.begin(); it != routingTable.end(); ++it)
               {
                   if ((*it)->getInterfaceId() == interfaceEntryId)
                   {
                       if ((*it)->getNextHop().isUnspecified())
                       {// "renew" directly connected
                           (*it)->setMetric(connNetworkMetric);
                           (*it)->setChangeFlag();
                           bSendTriggeredUpdateMessage = true;
                       }
                   }
               }

               if (bSendTriggeredUpdateMessage)
               {
                   sendTriggeredUpdateMessage();
               }

               bBlockTriggeredUpdateMessage = false;
           }
       }
       // TODO:
       // new network on an interface
       // deleted network on an interface
   }


   if (category == NF_IPv6_ROUTE_DELETED)
   {
       // if route from other routing protocol was deleted, check "RIPng routing table"
       IPv6Route *route = check_and_cast<IPv6Route *>(details);

       RIPng::RoutingTableEntry *routingTableEntryInRIPngRT;
       RIPng::RoutingTableEntry *RIPngRoute = dynamic_cast<RIPng::RoutingTableEntry *>(route);

       if (RIPngRoute != NULL)
       {// notification about RIPng route
           routingTableEntryInRIPngRT = RIPngRoute->getCopy();
           ASSERT(routingTableEntryInRIPngRT != NULL);

           routingTableEntryInRIPngRT->setCopy(NULL);
       }
       else
       {// check if RIPng has that route and install it
           routingTableEntryInRIPngRT = getRoutingTableEntry(route->getDestPrefix(), route->getPrefixLength());
           if (routingTableEntryInRIPngRT != NULL)
           {
               if (!routingTableEntryInRIPngRT->getNextHop().isUnspecified())
                   addRoutingTableEntryToGlobalRT(routingTableEntryInRIPngRT);
           }
       }
   }

   //observing route_changed is not necessarily, each protocol has its own metric
}
