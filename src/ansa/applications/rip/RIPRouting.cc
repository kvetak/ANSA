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
* @file RIPRouting.cc
* @author Jiri Trhlik (mailto:jiritm@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
* @brief RIP
* @detail Implements RIP protocol.
*/

#include "RIPRouting.h"

#include "IPv4InterfaceData.h"
#include "IPv4ControlInfo.h"

#include "deviceConfigurator.h"

Define_Module(RIPRouting);

//TODO: set context pointer to the timer messages - like RIPng

std::ostream& operator<<(std::ostream& os, const RIP::RoutingTableEntry& e)
{
    os << e.RIPInfo();
    return os;
};

RIPRouting::RIPRouting()
{
    regularUpdateTimer = NULL;
    triggeredUpdateTimer = NULL;
}

RIPRouting::~RIPRouting()
{
    deleteTimer(regularUpdateTimer);
    deleteTimer(triggeredUpdateTimer);

    //removeAllRoutingTableEntries();
    RIP::RoutingTableEntry *routingTableEntry;
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
//-- RIP ROUTING TABLE METHODS
//
//
RIP::RoutingTableEntry* RIPRouting::getRoutingTableEntry(const IPv4Address &network, const IPv4Address &netmask)
{
    RIP::RoutingTableEntry *route = NULL;
    for (RoutingTableIt it=routingTable.begin(); it!=routingTable.end(); it++)
    {
        if ((*it)->getDestination()==network && (*it)->getNetmask()==netmask)
        {
            route = (*it);
            break;
        }
    }

    return route;
}

void RIPRouting::addRoutingTableEntry(RIP::RoutingTableEntry* entry, bool createTimers)
{
    if (createTimers == true)
    {
        RIPTimer *timer = createAndStartTimer(RIP_ROUTE_TIMEOUT, routeTimeout);
        timer->setIPv4Address(entry->getDestination());
        timer->setNetmask(entry->getNetmask().getNetmaskLength());
        entry->setTimer(timer);

        RIPTimer *GCTimer = createTimer(RIP_ROUTE_GARBAGE_COLECTION_TIMEOUT);
        GCTimer->setIPv4Address(entry->getDestination());
        GCTimer->setNetmask(entry->getNetmask().getNetmaskLength());
        entry->setGCTimer(GCTimer);
    }

    routingTable.push_back(entry);

    //Do not try to add directly connected routes to the "global" routing table
    if (!entry->getGateway().isUnspecified())
        addRoutingTableEntryToGlobalRT(entry);

    ++numRoutes;
}
void RIPRouting::removeRoutingTableEntry(IPv4Address &network, int netmask)
{
    for (RoutingTableIt it=routingTable.begin(); it!=routingTable.end(); it++)
    {
        if ((*it)->getDestination()==network && (*it)->getNetmask().getNetmaskLength()==netmask)
        {
            removeRoutingTableEntry(it);
            break;
        }
    }
}

void RIPRouting::removeRoutingTableEntry(RoutingTableIt it)
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

void RIPRouting::removeAllRoutingTableEntries()
{
    RIP::RoutingTableEntry *routingTableEntry;

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

void RIPRouting::updateRoutingTableEntry(RIP::RoutingTableEntry *routingTableEntry, RIPRTE &rte, int sourceIntId, IPv4Address &sourceAddr)
{
    const IPv4Address &nextHop = routingTableEntry->getGateway();
    unsigned int newMetric = rte.getMetric();
    ++newMetric;
    unsigned int oldMetric = routingTableEntry->getMetric();

    RIP::RoutingTableEntry *routingTableEntryInGlobalRT = routingTableEntry->getCopy();

    if (nextHop == sourceAddr)
    {// RTE is from the same router
        RIPTimer *routeTimer = routingTableEntry->getTimer();

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
                RIPTimer *GCTimer = routingTableEntry->getGCTimer();
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
            routingTableEntry->setGateway(sourceAddr);
            routingTableEntry->setInterface(ift->getInterfaceById(sourceIntId));
            resetTimer(routingTableEntry->getTimer(), routeTimeout);
            // stop garbage collection timer
            RIPTimer *GCTimer = routingTableEntry->getGCTimer();
            ASSERT(GCTimer != NULL);
            if (GCTimer->isScheduled())
            {
                cancelTimer(GCTimer);
                // route was deleted from "global routing table" in the route deletion process, add it again
                addRoutingTableEntryToGlobalRT(routingTableEntry);
            }
            else if (routingTableEntryInGlobalRT != NULL)
            {
                //TODO: silent methods - RIPng-like
                routingTableEntryInGlobalRT->setInterface(ift->getInterfaceById(sourceIntId));
                routingTableEntryInGlobalRT->setGateway(sourceAddr);
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
//-- RIP INTERFACES METHODS
//
//
RIP::Interface *RIPRouting::getEnabledInterfaceByName(const char *interfaceName)
{
    RIP::Interface *enabledRIPInterface = NULL;

    InterfaceEntry *interface = ift->getInterfaceByName(interfaceName);
    if (interface != NULL)
    {
        int interfaceId = interface->getInterfaceId();
        int RIPInterfaceIndex = getEnabledInterfaceIndexById(interfaceId);
        if (RIPInterfaceIndex != -1)
        {
            enabledRIPInterface = getEnabledInterface(RIPInterfaceIndex);
        }
    }

    return enabledRIPInterface;
}

int RIPRouting::getEnabledInterfaceIndexById(int id)
{
    int i = 0, size = getEnabledInterfacesCount();
    while (i < size && getEnabledInterface(i)->getId() != id) i++;
    return i == size ? -1 : i;
}

void RIPRouting::addEnabledInterface(RIP::Interface *interface)
{
    enabledInterfaces.push_back(interface);
    sockets.push_back(createAndSetSocketForInt(interface));
}

void RIPRouting::removeEnabledInterface(unsigned long i)
{
    delete enabledInterfaces[i];
    enabledInterfaces.erase(enabledInterfaces.begin() + i);
    sockets[i]->close();
    delete sockets[i];
    sockets.erase(sockets.begin() + i);

    //if (i == 0)
        //sockets[0]->joinMulticastGroup(RIPAddress, -1);
}

void RIPRouting::removeAllEnabledInterfaces()
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
void RIPRouting::showRoutingTable()
{
    RoutingTableIt it;
    RIP::RoutingTableEntry *routingTableEntry;

    ev << routerText << endl;

    for (it = routingTable.begin(); it != routingTable.end(); it++)
    {
        routingTableEntry = (*it);
        ev << routingTableEntry->info() << endl;
    }
}

RIPMessage *RIPRouting::createMessage()
{
    char msgName[32] = "RIPMessage";

    RIPMessage *msg = new RIPMessage(msgName);
    return msg;
}

UDPSocket *RIPRouting::createAndSetSocketForInt(RIP::Interface* interface)
{
    UDPSocket *socket = new UDPSocket();
    socket->setOutputGate(gate("udpOut"));
    //so every RIP message sent from RIP interface uses correct source address
    socket->bind(ift->getInterfaceById(interface->getId())->ipv4Data()->getIPAddress(), RIPPort);

    int timeToLive = par("timeToLive");
    if (timeToLive != -1)
        socket->setTimeToLive(timeToLive);

    return socket;
}

RIP::Interface *RIPRouting::enableRIPOnInterface(const char *interfaceName)
{
    InterfaceEntry *interface = ift->getInterfaceByName(interfaceName);
    int interfaceId = interface->getInterfaceId();

    return enableRIPOnInterface(interfaceId);
}

RIP::Interface *RIPRouting::enableRIPOnInterface(int interfaceId)
{
    ev << "Enabling RIP on " << ift->getInterfaceById(interfaceId)->getName() << routerText << endl;

    RIP::Interface *RIPInterface = new RIP::Interface(interfaceId);
    // add interface to local RIP interface table
    addEnabledInterface(RIPInterface);

    return RIPInterface;
}

void RIPRouting::setInterfacePassiveStatus(RIP::Interface *RIPInterface, bool status)
{
    if (status == true)
        ev << "Setting RIP passive interface (interface id: " << RIPInterface->getId() << ")." << routerText << endl;

    if (status)
        RIPInterface->enablePassive();
    else
        RIPInterface->disablePassive();
}

void RIPRouting::setInterfaceSplitHorizon(RIP::Interface *RIPInterface, bool status)
{
    if (status)
        RIPInterface->enableSplitHorizon();
    else
        RIPInterface->disableSplitHorizon();
}

void RIPRouting::setInterfacePoisonReverse(RIP::Interface *RIPInterface, bool status)
{
    if (status)
        RIPInterface->enablePoisonReverse();
    else
        RIPInterface->disablePoisonReverse();
}

void RIPRouting::addRoutingTableEntryToGlobalRT(RIP::RoutingTableEntry* entry)
{
    RIP::RoutingTableEntry *newEntry = new RIP::RoutingTableEntry(*entry);
    if (rt->prepareForAddRoute(newEntry))
    {
        rt->addRoute(newEntry);
        entry->setCopy(newEntry);
    }
    else
    {// route exists with lower administrative distance
        delete newEntry;
    }
}

void RIPRouting::removeRoutingTableEntryFromGlobalRT(RIP::RoutingTableEntry* entry)
{
    if (entry->getCopy() != NULL)
    // corresponding route from "global routing table" to the entry from "RIP routing table"
        rt->removeRoute(entry->getCopy());
}

//
//
//-- OUTPUT PROCESSING
//
//
void RIPRouting::sendRegularUpdateMessage()
{
    int numInterfaces = getEnabledInterfacesCount();
    RIP::Interface *interface;

    // sent update on every interface, where is enabled RIP and that interface is not passive
    for (int i = 0; i < numInterfaces; i++)
    {
        interface = getEnabledInterface(i);
        if (interface->isPassive())
            // do not send updates out of the passive interface
            continue;

        RIPMessage *msg = makeUpdateMessageForInterface(interface, false);
        if (msg != NULL)
        // no rtes to send
            sendMessage(msg, RIPAddress, RIPPort, i, false);
    }

    //reset Route Change Flags
    clearRouteChangeFlags();

    bSendTriggeredUpdateMessage = false;
}

void RIPRouting::sendTriggeredUpdateMessage()
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
    RIP::Interface *interface;

    // sent update on every interface, where is enabled RIP and that interface is not passive
    for (int i = 0; i < numInterfaces; i++)
    {
        interface = getEnabledInterface(i);
        if (interface->isPassive())
            // do not send updates out of the passive interface
            continue;

        RIPMessage *msg = makeUpdateMessageForInterface(interface, true);
        if (msg != NULL)
            sendMessage(msg, RIPAddress, RIPPort, i, false);
    }

    //reset Route Change Flags
    clearRouteChangeFlags();

    bSendTriggeredUpdateMessage = false;

    resetTimer(triggeredUpdateTimer, uniform(1, 5));
}

void RIPRouting::sendDelayedTriggeredUpdateMessage()
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

RIPMessage *RIPRouting::makeUpdateMessageForInterface(RIP::Interface *interface, bool changed)
{
    int size;
    std::vector<RIPRTE> rtes;

    getRTEs(rtes, interface, changed);

    size = rtes.size();
    if (size <= 0)
        // there's no RTE to send from this interface (no message will be created)
        return NULL;

    RIPMessage *msg = createMessage();
    msg->setCommand(RIPResponse);
    msg->setRtesArraySize(size);
    // set RTEs to response
    for(int j = 0; j < size; j++)
        msg->setRtes(j, rtes[j]);

    return msg;
}

void RIPRouting::sendMessage(RIPMessage *msg, IPv4Address &addr, int port, unsigned long enabledInterfaceIndex, bool globalSourceAddress)
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

void RIPRouting::sendAllRoutesRequest()
{
    int numInterfaces = getEnabledInterfacesCount();
    RIP::Interface *interface;

    RIPRTE rte = RIPRTE();
    rte.setIPv4Address(IPv4Address()); // IPv4 Address 0.0.0.0
    rte.setMetric(16);
    rte.setNetMask(0);
    rte.setRouteTag(0);

    // send update on every interface, where is enabled RIP and that interface is not passive
    for (int i = 0; i < numInterfaces; i++)
    {
        interface = getEnabledInterface(i);
        if (interface->isPassive())
            // do not send request out of the passive interface
            continue;

        RIPMessage *msg = createMessage();

        msg->setCommand(RIPRequest);
        msg->setRtesArraySize(1);
        msg->setRtes(0, rte);

        sendMessage(msg, RIPAddress, RIPPort, i, false);
    }
}

void RIPRouting::clearRouteChangeFlags()
{
    RIP::RoutingTableEntry *routingTableEntry;
    RoutingTableIt it;

    for (it = routingTable.begin(); it != routingTable.end(); it++)
    {
        routingTableEntry = (*it);
        routingTableEntry->clearChangeFlag();
    }
}

void RIPRouting::getRTEs(std::vector<RIPRTE> &rtes, RIP::Interface *interface, bool onlyChanged)
{
    RIP::RoutingTableEntry *routingTableEntry;
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
        if (splitHorizon && (routingTableEntry->getInterface()->getInterfaceId() == interfaceId))
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
            RIPRTE rte = makeRTEFromRoutingTableEntry(routingTableEntry);
            if (setInfMetric)
                rte.setMetric(infinityMetric);
            rtes.push_back(rte);
        }
    }
}

RIPRTE RIPRouting::makeRTEFromRoutingTableEntry(RIP::RoutingTableEntry *routingTableEntry)
{
    RIPRTE rte;
    // create RTE for message to neighbor
    rte.setNetMask(routingTableEntry->getNetmask().getNetmaskLength());
    rte.setIPv4Address(routingTableEntry->getDestination());
    rte.setMetric(routingTableEntry->getMetric());
    rte.setRouteTag(routingTableEntry->getRouteTag());

    return rte;
}

//
//
//-- INPUT PROCESSING
//
//
void RIPRouting::handleRIPMessage(RIPMessage *msg)
{
    UDPDataIndication *controlInfo = check_and_cast<UDPDataIndication *>(msg->getControlInfo());
    IPv4Address sourceAddr = controlInfo->getSrcAddr().get4();
    IPv4Address destAddr = controlInfo->getDestAddr().get4();
    int sourcePort = controlInfo->getSrcPort();
    int sourceInterfaceId = controlInfo->getInterfaceId();

    int RIPIntInd = getEnabledInterfaceIndexById(sourceInterfaceId);
    if (RIPIntInd < 0)
    {//message is from an interface with disabled RIP
        delete msg;
        return;
    }

    EV << "RIP: Received packet: " << UDPSocket::getReceivedPacketInfo(msg) << endl;
    int command = msg->getCommand();
    int version = msg->getVersion();
    if (version != 2)
        EV << "This implementation of RIP does not support version '" << version << "' of this protocol." << endl;

    if (command == RIPRequest)
    {
        handleRequest(msg, sourcePort, sourceAddr, destAddr, RIPIntInd);
    }
    else if (command == RIPResponse)
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
void RIPRouting::handleResponse(RIPMessage *response, int srcInt, IPv4Address &srcAddr)
{
    if (!checkMessageValidity(response))
        return;
    EV << "RIP message: RIP - Response" << endl;
    processRTEs(response, srcInt, srcAddr);
}

bool RIPRouting::checkMessageValidity(RIPMessage *response)
{
    UDPDataIndication *controlInfo = check_and_cast<UDPDataIndication *>(response->getControlInfo());

    // is from RIP port
    if (controlInfo->getSrcPort() != RIPPort)
    {
        EV << "RIP message: bad source port." << endl;
        return false;
    }

    IPv4Address sourceAddr = controlInfo->getSrcAddr().get4();
    // source addr. is not from this device
    if (rt->isLocalAddress(sourceAddr))
    {
        EV << "RIP message: is local address - ignoring..." << endl;
        return false;
    }

    if (controlInfo->getTtl() < 254)
    {
        EV << "RIP message: bad ttl." << endl;
        return false;
    }

    return true;
}

void RIPRouting::processRTEs(RIPMessage *response, int sourceIntId, IPv4Address &sourceAddr)
{
    unsigned int rtesSize = response->getRtesArraySize();
    RIPRTE rte;

    // if startRouteDeletionProcess() was called we dont want to call sendTriggeredUpdateMessage() in that function,
    // so we'll call sendTriggeredUpdateMessage() after "all startRouteDeletionProcess()"
    bBlockTriggeredUpdateMessage = true;

    //Process every RTE
    for (unsigned int i = 0; i < rtesSize; i++)
    {
        rte = response->getRtes(i);
        EV << "RTE [" << i << "]: " << rte.getIPv4Address() << "/" << int(rte.getNetMask()) << endl;
        processRTE(rte, sourceIntId, sourceAddr);
    }

    if (bSendTriggeredUpdateMessage)
    {
        sendTriggeredUpdateMessage();
    }

    bBlockTriggeredUpdateMessage = false;
}

void RIPRouting::processRTE(RIPRTE &rte, int sourceIntId, IPv4Address &sourceAddr)
{
    if (!checkAndLogRTE(rte, sourceAddr))
        return;

    IPv4Address &network = rte.getIPv4Address();
    IPv4Address netmask = IPv4Address::makeNetmask(rte.getNetMask());

    // Check if a route with the prefix exists
    RIP::RoutingTableEntry *routingTableEntry = getRoutingTableEntry(network, netmask);

    if (routingTableEntry != NULL)
    {// Update Routing Table Entry
        updateRoutingTableEntry(routingTableEntry, rte, sourceIntId, sourceAddr);
    }
    else
    {// Create and add new Routing Table Entry
        unsigned int metric = rte.getMetric();
        ++metric;

        if (metric < infinityMetric)
        {
            RIP::RoutingTableEntry *route = new RIP::RoutingTableEntry(network, netmask);
            route->setInterface(ift->getInterfaceById(sourceIntId));
            route->setGateway(sourceAddr);
            route->setMetric(metric);
            route->setChangeFlag();

            addRoutingTableEntry(route);

            bSendTriggeredUpdateMessage = true;
        }
    }
}

bool RIPRouting::checkAndLogRTE(RIPRTE &rte, IPv4Address &sourceAddr)
{
    // netmask is valid (0-32)
    // metric is valid (0-16)
    if ( rte.getNetMask() > 32 ||
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
void RIPRouting::handleRequest(RIPMessage *request, int srcPort, IPv4Address &srcAddr, IPv4Address &destAddr, unsigned long RIPIntInd)
{
    ASSERT(RIPIntInd < getEnabledInterfacesCount());
    RIP::Interface *RIPInt = getEnabledInterface(RIPIntInd);

    unsigned int rteNum = request->getRtesArraySize();
    std::vector<RIPRTE> responseRtes;

    if (rteNum == 1)
    {// could be a request for all routes
        EV << "RIP message: RIP - General Request" << endl;
        RIPRTE &rte = request->getRtes(0);
        if (rte.getIPv4Address().isUnspecified() &&
            rte.getNetMask() == 0 &&
            rte.getMetric() == infinityMetric &&
            rte.getRouteTag() == 0)
        {
            getRTEs(responseRtes, RIPInt, false);
        }
    }
    else
    {
        RIP::RoutingTableEntry *routingTableEntry;
        EV << "RIP message: RIP - Request" << endl;
        for (unsigned int i = 0; i < rteNum; i++)
        {
            RIPRTE rte = request->getRtes(i);
            routingTableEntry = getRoutingTableEntry(rte.getIPv4Address(), IPv4Address::makeNetmask(rte.getNetMask()));
            EV << "RTE [" << i << "]: " << rte.getIPv4Address() << "/" << int(rte.getNetMask()) << endl;
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

    RIPMessage *response = createMessage();
    response->setCommand(RIPResponse);
    response->setRtesArraySize(size);
    // set RTEs to response
    for(int j = 0; j < size; j++)
        response->setRtes(j, responseRtes[j]);

    if (destAddr == RIPAddress && srcPort == RIPPort)
        sendMessage(response, srcAddr, srcPort, RIPIntInd, false);
    else
        sendMessage(response, srcAddr, srcPort, RIPIntInd, true);
}

//
//
//-- TIMEOUTS
//
//
RIPTimer *RIPRouting::createTimer(int timerKind)
{
    RIPTimer *timer = new RIPTimer();
    timer->setTimerKind(timerKind);

    return timer;
}

RIPTimer *RIPRouting::createAndStartTimer(int timerKind, simtime_t timerLen)
{
    RIPTimer *timer = createTimer(timerKind);

    scheduleAt(simTime() + timerLen, timer);

    return timer;
}

void RIPRouting::resetTimer(RIPTimer *timer, simtime_t timerLen)
{
    ASSERT(timer != NULL);
    if (timer->isScheduled())
        cancelEvent(timer);

    scheduleAt(simTime() + timerLen, timer);
}

void RIPRouting::cancelTimer(RIPTimer *timer)
{
    if (timer != NULL)
    {
        if (timer->isScheduled())
            cancelEvent(timer);
    }
}

void RIPRouting::deleteTimer(RIPTimer *timer)
{
    if (timer != NULL)
    {
        if (timer->isScheduled())
            cancelEvent(timer);

        delete timer;
    }
}

void RIPRouting::handleTimer(RIPTimer *msg)
{
    int type = msg->getTimerKind();

    switch (type)
    {
        case RIP_GENERAL_UPDATE :
            handleRegularUpdateTimer();
            break;
        case RIP_TRIGGERED_UPDATE :
            handleTriggeredUpdateTimer();
            break;
        case RIP_ROUTE_TIMEOUT :
            startRouteDeletionProcess(msg);
            break;
        case RIP_ROUTE_GARBAGE_COLECTION_TIMEOUT :
            deleteRoute(msg);
            break;
        default:
            break;
    }
}

void RIPRouting::handleRegularUpdateTimer()
{
     // send regular update message
    sendRegularUpdateMessage();
    // plan next regular update
    resetTimer(regularUpdateTimer, regularUpdateTimeout);
}

void RIPRouting::handleTriggeredUpdateTimer()
{
    if (bSendTriggeredUpdateMessage)
        sendTriggeredUpdateMessage();
}

void RIPRouting::startRouteDeletionProcess(RIPTimer *timer)
{
    IPv4Address &network = timer->getIPv4Address();
    IPv4Address netmask = IPv4Address::makeNetmask(timer->getNetmask());
    RIP::RoutingTableEntry *routingTableEntry = getRoutingTableEntry(network, netmask);

    startRouteDeletionProcess(routingTableEntry);
}

void RIPRouting::startRouteDeletionProcess(RIP::RoutingTableEntry *routingTableEntry)
{
    ASSERT(routingTableEntry != NULL);
    routingTableEntry->setMetric(infinityMetric);
    routingTableEntry->setChangeFlag();

    RIPTimer *GCTimer = routingTableEntry->getGCTimer();
    // (re)set the timer
    resetTimer(GCTimer, routeGarbageCollectionTimeout);

    removeRoutingTableEntryFromGlobalRT(routingTableEntry);

    if (bBlockTriggeredUpdateMessage)
    // if response is processing and a rte with the metric of 16 was received, wait until all the RTEs are proccesed
        bSendTriggeredUpdateMessage = true;
    else
        sendDelayedTriggeredUpdateMessage();
}

void RIPRouting::deleteRoute(RIPTimer *timer)
{
    IPv4Address &network = timer->getIPv4Address();
    int netmask = timer->getNetmask();

    removeRoutingTableEntry(network, netmask);
}

//
//
//-- OVERRIDDEN METHODS
//
//
void RIPRouting::initialize(int stage)
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
    rt = AnsaRoutingTableAccess().get();
    ift = InterfaceTableAccess().get();
    // subscribe for changes in the device
    nb = NotificationBoardAccess().get();
    nb->subscribe(this, NF_INTERFACE_STATE_CHANGED);
    nb->subscribe(this, NF_IPv4_ROUTE_DELETED);

    numRoutes = 0;
    WATCH(numRoutes);

    WATCH_PTRVECTOR(routingTable);

    const char *RIPAddressString = par("RIPAddress");
    RIPAddress = IPv4Address(RIPAddressString);
    RIPPort = par("RIPPort");

    connNetworkMetric = par("connectedNetworkMetric");
    infinityMetric = par("infinityMetric");

    routeTimeout = par("routeTimeout").doubleValue();
    routeGarbageCollectionTimeout = par("routeGarbageCollectionTimeout").doubleValue();
    regularUpdateTimeout = par("regularUpdateInterval").doubleValue();

    // get deviceId
    deviceId = par("deviceId");

    // read the RIP process configuration
    DeviceConfigurator *devConf = ModuleAccess<DeviceConfigurator>("deviceConfigurator").get();
    devConf->loadRIPConfig(this);

    //Multicast must be enabled on every interface because of globalSocket.joinMulticastGroup(RIPAddress, -1);
    //else IGMP module in the network layer will end up with ASSERT error
    for (int i=0; i < ift->getNumInterfaces(); ++i)
        ift->getInterface(i)->setMulticast(true);

    globalSocket.setOutputGate(gate("udpOut"));
    globalSocket.bind(RIPPort);
    globalSocket.joinMulticastGroup(RIPAddress, -1);

    // start REGULAR UPDATE TIMER
    regularUpdateTimer = createAndStartTimer(RIP_GENERAL_UPDATE, regularUpdateTimeout);
    triggeredUpdateTimer = createTimer(RIP_TRIGGERED_UPDATE);

    updateDisplayString();

    sendAllRoutesRequest();
}

void RIPRouting::updateDisplayString()
{
    if (ev.isGUI())
    {
        char buf[40];
        sprintf(buf, "%d routes", numRoutes);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

void RIPRouting::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {// timers
        handleTimer(check_and_cast<RIPTimer*> (msg));
    }
    else if (msg->getKind() == UDP_I_DATA)
    {// process incoming message
        handleRIPMessage(check_and_cast<RIPMessage*> (msg));
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

    updateDisplayString();
}

void RIPRouting::receiveChangeNotification(int category, const cObject *details)
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
           // delete interface from RIP interfaces
           int size = getEnabledInterfacesCount();
           RIP::Interface* RIPInterface;
           bool alreadyDisabled = true;

           for (int i = 0; i < size; i++)
           {
               RIPInterface = getEnabledInterface(i);
               if (RIPInterface->getId() == interfaceEntryId)
               {
                   alreadyDisabled = false;
                   removeEnabledInterface(i);
                   break;
               }
           }

           if (!alreadyDisabled)
           {
               bBlockTriggeredUpdateMessage = true;

               // delete associated routes from RIP routing table
               RoutingTableIt it;
               for (it = routingTable.begin(); it != routingTable.end(); ++it)
               {
                   if ((*it)->getInterface()->getInterfaceId() == interfaceEntryId)
                   {
                       if ((*it)->getGateway().isUnspecified())
                       {// directly connected
                           (*it)->setMetric(infinityMetric);
                           (*it)->setChangeFlag();
                           /* XXX: directly connected routes have to remain in the RIP routing table
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
           RIP::Interface* RIPInterface;

           for (int i = 0; i < size; i++)
           {
               RIPInterface = getEnabledInterface(i);
               if (RIPInterface->getId() == interfaceEntryId)
               {
                   alreadyEnabled = true;
                   break;
               }
           }

           if (!alreadyEnabled)
           {
               // add interface to RIP interfaces
               enableRIPOnInterface(interfaceEntry->getName());

               bBlockTriggeredUpdateMessage = true;

               // delete associated routes from RIP routing table
               RoutingTableIt it;
               for (it = routingTable.begin(); it != routingTable.end(); ++it)
               {
                   if ((*it)->getInterface()->getInterfaceId() == interfaceEntryId)
                   {
                       if ((*it)->getGateway().isUnspecified())
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


   if (category == NF_IPv4_ROUTE_DELETED)
   {
       // if route from other routing protocol was deleted, check "RIP routing table"
       IPv4Route *route = check_and_cast<IPv4Route *>(details);

       RIP::RoutingTableEntry *routingTableEntryInRIPRT;
       RIP::RoutingTableEntry *RIPRoute = dynamic_cast<RIP::RoutingTableEntry *>(route);

       if (RIPRoute != NULL)
       {// notification about RIP route
           routingTableEntryInRIPRT = RIPRoute->getCopy();
           ASSERT(routingTableEntryInRIPRT != NULL);

           routingTableEntryInRIPRT->setCopy(NULL);
       }
       else
       {// check if RIP has that route and install it
           routingTableEntryInRIPRT = getRoutingTableEntry(route->getDestination(), route->getNetmask());
           if (routingTableEntryInRIPRT != NULL)
           {
               if (!routingTableEntryInRIPRT->getGateway().isUnspecified())
                   addRoutingTableEntryToGlobalRT(routingTableEntryInRIPRT);
           }
       }
   }

   //observing route_changed is not necessarily, each protocol has its own metric
}
