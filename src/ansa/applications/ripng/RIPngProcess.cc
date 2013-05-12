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

#include "RIPngProcess.h"

#include "IPv6InterfaceData.h"
#include "IPv6ControlInfo.h"

#include "RIPngRouting.h"
#include "deviceConfigurator.h"

std::ostream& operator<<(std::ostream& os, const RIPng::RoutingTableEntry& e)
{
    os << e.RIPngInfo();
    return os;
};

RIPngProcess::RIPngProcess(const char *name, RIPngRouting *RIPngModule) :
        processName(name),
        RIPng(RIPngModule)
{
    regularUpdateTimer = NULL;
    triggeredUpdateTimer = NULL;

    poisonReverse = false;
    splitHorizon = true;

    numOfDefaultInformationInterfaces = 0;

    setDeviceId("");
    setHostName("");
    setRouterText("");
    setConnNetworkMetric(RIPngModule->getConnNetworkMetric());
    setInfiniteMetric(RIPngModule->getInfinityMetric());

    hostName = RIPngModule->getHostName();
    routeTimeout = RIPngModule->getRouteTimeout();
    routeGarbageCollectionTimeout = RIPngModule->getRouteGarbageCollectionTimeout();
    regularUpdateTimeout = RIPngModule->getRegularUpdateTimeout();
    setRIPngAddress(RIPngModule->getRIPngAddress());
    setRIPngPort(RIPngModule->getRIPngPort());
    setDistance(RIPngModule->getDistance());

    bSendTriggeredUpdateMessage = false;
    bBlockTriggeredUpdateMessage = false;

    // access to the routing and interface table
    rt = ANSARoutingTable6Access().get();
    ift = InterfaceTableAccess().get();

    numRoutes = 0;
    regularUpdates = 0;
    triggerUpdates = 0;
    //WATCH(numRoutes);
    //WATCH_PTRVECTOR(routingTable);
}

RIPngProcess::~RIPngProcess()
{
    RIPng->deleteTimer(regularUpdateTimer);
    RIPng->deleteTimer(triggeredUpdateTimer);

    //removeAllRoutingTableEntries();
    RIPng::RoutingTableEntry *routingTableEntry;
    for (RoutingTableIt it = routingTable.begin(); it != routingTable.end(); it++)
    {
        routingTableEntry = (*it);
        RIPng->deleteTimer(routingTableEntry->getGCTimer());
        RIPng->deleteTimer(routingTableEntry->getTimer());
        delete routingTableEntry;
    }
    routingTable.clear();

    unsigned long downIntCount = downInterfaces.size();
    for (unsigned long i = 0; i < downIntCount; ++i)
    {
        delete downInterfaces[i];
    }
    downInterfaces.clear();

    removeAllEnabledInterfaces();
}

void RIPngProcess::start()
{
    // start REGULAR UPDATE TIMER
    regularUpdateTimer = RIPng->createAndStartTimer(RIPNG_GENERAL_UPDATE, this, regularUpdateTimeout);
    triggeredUpdateTimer = RIPng->createTimer(RIPNG_TRIGGERED_UPDATE, this);

    sendAllRoutesRequest();
}

void RIPngProcess::stop()
{
    unsigned long intCount = getEnabledInterfacesCount();
    for (unsigned long i = 0; i < intCount; ++i)
    {
        RIPng::Interface *interface = getEnabledInterface(i);
        RIPng->setOutputPortOnInterface(interface, -1);
        delete enabledInterfaces[i];
    }
    enabledInterfaces.clear();

    unsigned long downIntCount = downInterfaces.size();
    for (unsigned long i = 0; i < downIntCount; ++i)
    {
        delete downInterfaces[i];
    }
    downInterfaces.clear();
}

bool RIPngProcess::setDistance(unsigned int distance)
{
    if (distance > 1 && distance < 254)
    {
        this->distance = distance;
        RIPng::RoutingTableEntry *routingTableEntry;
        for (RoutingTableIt it = routingTable.begin(); it != routingTable.end(); it++)
        {
            routingTableEntry = (*it);
            routingTableEntry->setAdminDist(distance);
            if (routingTableEntry->getCopy())
                routingTableEntry->getCopy()->setAdminDist(distance);
        }

        return true;
    }

    return false;
}

bool RIPngProcess::setRouteTimeout(simtime_t routeTimeout)
{
    if (routeTimeout >= 1 && routeTimeout <= 65535)
    {
        RIPngTimer *timer;
        RIPng::RoutingTableEntry *routingTableEntry;
        for (RoutingTableIt it = routingTable.begin(); it != routingTable.end(); it++)
        {
            routingTableEntry = (*it);
            timer = routingTableEntry->getTimer();

            if (timer != NULL && timer->isScheduled())
            {
                simtime_t timerLen = timer->getArrivalTime() - simTime();
                if (timerLen > routeTimeout)
                {
                    RIPng->resetTimer(timer, routeTimeout);
                }
            }
        }

        this->routeTimeout = routeTimeout;

        return true;
    }

    return false;
}

bool RIPngProcess::setRouteGarbageCollectionTimeout(simtime_t routeGarbageCollectionTimeout)
{
    if (routeGarbageCollectionTimeout >= 1 && routeGarbageCollectionTimeout <= 65535)
    {
        RIPngTimer *timer;
        RIPng::RoutingTableEntry *routingTableEntry;
        for (RoutingTableIt it = routingTable.begin(); it != routingTable.end(); it++)
        {
            routingTableEntry = (*it);
            timer = routingTableEntry->getGCTimer();

            if (timer != NULL && timer->isScheduled())
            {
                simtime_t timerLen = timer->getArrivalTime() - simTime();
                if (timerLen > routeGarbageCollectionTimeout)
                {
                    RIPng->resetTimer(timer, routeGarbageCollectionTimeout);
                }
            }
        }

        this->routeGarbageCollectionTimeout = routeGarbageCollectionTimeout;

        return true;
    }

    return false;
}

bool RIPngProcess::setRegularUpdateTimeout(simtime_t regularUpdateTimeout)
{
    if (regularUpdateTimeout >= 5 && regularUpdateTimeout <= 65535)
    {
        if (regularUpdateTimer != NULL && regularUpdateTimer->isScheduled())
        {
            simtime_t timerLen = regularUpdateTimer->getArrivalTime() - simTime();
            if (timerLen > regularUpdateTimeout)
            {
                RIPng->resetTimer(regularUpdateTimer, regularUpdateTimeout);
            }
        }

        this->regularUpdateTimeout = regularUpdateTimeout;

        return true;
    }

    return false;
}


//
//
//-- RIPNG ROUTING TABLE METHODS
//
//
RIPng::RoutingTableEntry* RIPngProcess::getRoutingTableEntry(const IPv6Address &prefix, int prefixLength)
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

void RIPngProcess::addRoutingTableEntry(RIPng::RoutingTableEntry* entry, bool createTimers)
{
    if (createTimers == true)
    {
        RIPngTimer *timer = RIPng->createAndStartTimer(RIPNG_ROUTE_TIMEOUT, entry, routeTimeout);
        timer->setIPv6Prefix(entry->getDestPrefix());
        timer->setPrefixLen(entry->getPrefixLength());
        entry->setTimer(timer);

        RIPngTimer *GCTimer = RIPng->createTimer(RIPNG_ROUTE_GARBAGE_COLECTION_TIMEOUT, entry);
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
void RIPngProcess::removeRoutingTableEntry(IPv6Address &prefix, int prefixLength)
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

void RIPngProcess::removeRoutingTableEntry(RoutingTableIt it)
{
    ASSERT(it != routingTable.end());

    // delete timers
    RIPng->deleteTimer((*it)->getGCTimer());
    RIPng->deleteTimer((*it)->getTimer());
    removeRoutingTableEntryFromGlobalRT((*it));
    // delete routing table entry
    delete (*it);
    routingTable.erase(it);

    --numRoutes;
}

void RIPngProcess::removeAllRoutingTableEntries()
{
    RIPng::RoutingTableEntry *routingTableEntry;

    for (RoutingTableIt it = routingTable.begin(); it != routingTable.end(); it++)
    {
        routingTableEntry = (*it);
        RIPng->deleteTimer(routingTableEntry->getGCTimer());
        RIPng->deleteTimer(routingTableEntry->getTimer());
        removeRoutingTableEntryFromGlobalRT((*it));
        delete routingTableEntry;
    }

    routingTable.clear();
}

void RIPngProcess::updateRoutingTableEntry(RIPng::RoutingTableEntry *routingTableEntry, RIPngRTE &rte, int srcRIPngIntInd, IPv6Address &sourceAddr)
{
    RIPng::Interface *interface = getEnabledInterface(srcRIPngIntInd);
    const IPv6Address &nextHop = routingTableEntry->getNextHop();
    int newMetric = rte.getMetric();
    newMetric += interface->getMetricOffset();
    int oldMetric = routingTableEntry->getMetric();

    RIPng::RoutingTableEntry *routingTableEntryInGlobalRT = routingTableEntry->getCopy();

    if (nextHop == sourceAddr)
    {// RTE is from the same router
        RIPngTimer *routeTimer = routingTableEntry->getTimer();

        if (newMetric < infinityMetric)
            RIPng->resetTimer(routeTimer, routeTimeout);

        if (newMetric != oldMetric)
        {
            if (newMetric >= infinityMetric && oldMetric < infinityMetric)
            {//Invalidate route if isn't already invalidated
                // bSendTriggeredUpdateMessage is set in startRouteDeletionProcess() function
                RIPng->cancelTimer(routeTimer);
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
                    RIPng->cancelTimer(GCTimer);
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
            routingTableEntry->setInterfaceId(interface->getId());
            RIPng->resetTimer(routingTableEntry->getTimer(), routeTimeout);
            // stop garbage collection timer
            RIPngTimer *GCTimer = routingTableEntry->getGCTimer();
            ASSERT(GCTimer != NULL);
            if (GCTimer->isScheduled())
            {
                RIPng->cancelTimer(GCTimer);
                // route was deleted from "global routing table" in the route deletion process, add it again
                addRoutingTableEntryToGlobalRT(routingTableEntry);
            }
            else if (routingTableEntryInGlobalRT != NULL)
            {
                routingTableEntryInGlobalRT->setInterfaceIdSilent(interface->getId());
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
int RIPngProcess::getEnabledInterfaceIndexById(int id)
{
    int i = 0, size = getEnabledInterfacesCount();
    while (i < size && getEnabledInterface(i)->getId() != id) i++;
    return i == size ? -1 : i;
}

int RIPngProcess::getEnabledInterfaceIndexByName(const char *name)
{
    InterfaceEntry *interface = ift->getInterfaceByName(name);
    if (interface == NULL)
        return -1;

    int interfaceId = interface->getInterfaceId();
    return getEnabledInterfaceIndexById(interfaceId);
}

int RIPngProcess::getDownInterfaceIndexById(int id)
{
    int i = 0, size = downInterfaces.size();
    while (i < size && downInterfaces[i]->getId() != id) i++;
    return i == size ? -1 : i;
}

void RIPngProcess::addEnabledInterface(RIPng::Interface *interface)
{
    enabledInterfaces.push_back(interface);
}

RIPng::Interface *RIPngProcess::removeEnabledInterface(unsigned long i)
{
    RIPng::Interface *interface = enabledInterfaces[i];
    enabledInterfaces.erase(enabledInterfaces.begin() + i);
    return interface;
}

void RIPngProcess::removeAllEnabledInterfaces()
{
    unsigned long intCount = getEnabledInterfacesCount();
    for (unsigned long i = 0; i < intCount; ++i)
    {
        delete enabledInterfaces[i];
    }
    enabledInterfaces.clear();
}

//
//
//-- GENERAL METHODS
//
//
std::string RIPngProcess::getRoutingTable()
{
    std::stringstream out;

    RoutingTableIt it;
    RIPng::RoutingTableEntry *routingTableEntry;

    for (it = routingTable.begin(); it != routingTable.end(); ++it)
    {
        routingTableEntry = (*it);
        out << "   " << routingTableEntry->RIPngInfo() << endl;
    }

    return out.str();
}

RIPngMessage *RIPngProcess::createMessage()
{
    char msgName[32] = "RIPngMessage";

    RIPngMessage *msg = new RIPngMessage(msgName);
    return msg;
}

RIPng::Interface *RIPngProcess::enableRIPngOnInterface(InterfaceEntry *interface)
{
    ASSERT(interface != NULL);

    int interfaceId = interface->getInterfaceId();
    RIPng::Interface *RIPngInterface = new RIPng::Interface(interfaceId, this);
    // add interface to local RIPng interface table
    addEnabledInterface(RIPngInterface);

    RIPng->setOutputPortOnInterface(RIPngInterface, RIPngPort);

    return RIPngInterface;
}

RIPng::Interface *RIPngProcess::disableRIPngOnInterface(unsigned long RIPngInterfaceIndex)
{
    ASSERT(RIPngInterfaceIndex < getEnabledInterfacesCount());

    RIPng::Interface *interface = getEnabledInterface(RIPngInterfaceIndex);

    RIPng->setOutputPortOnInterface(interface, -1);

    return removeEnabledInterface(RIPngInterfaceIndex);
}

void RIPngProcess::setInterfacePassiveStatus(RIPng::Interface *RIPngInterface, bool status)
{
    if (status == true)
        ev << "   Setting RIPng passive interface (interface id: " << RIPngInterface->getId() << ")." << routerText << endl;

    if (status)
        RIPngInterface->enablePassive();
    else
        RIPngInterface->disablePassive();
}

void RIPngProcess::setInterfaceMetricOffset(RIPng::Interface *RIPngInterface, int offset)
{
    if (RIPngInterface->setMetricOffset(offset))
    {
        ev << "   Setting RIPng metric-offset " << offset << " (interface id: " << RIPngInterface->getId() << ")." << routerText << endl;
    }
}

void RIPngProcess::setInterfaceDefaultInformation(RIPng::Interface *RIPngInterface, bool enabled, bool defaultOnly, int metric)
{
    if (enabled)
    {
        RIPngInterface->setDefaultInformationOriginate();
        // default-information originate by default
        if (defaultOnly)
        {
            RIPngInterface->setDefaultInformationOnly();
            ev << "   default-information only ";
        }
        else
        {
            ev << "   default-information originate ";
        }

        if (metric >= 1)
        {
            RIPngInterface->setDefaultRouteMetric(metric);
            ev << "metric " << metric << " ";
        }
    }
    else
    {
        ev << "   no default-information ";
    }

    ev << "(interface id: " << RIPngInterface->getId() << ")." << routerText << endl;
}

void RIPngProcess::setInterfaceSplitHorizon(RIPng::Interface *RIPngInterface, bool status)
{
    if (status)
    {
        ev << "   Enabling";
        RIPngInterface->enableSplitHorizon();
    }
    else
    {
        ev << "   Disabling";
        RIPngInterface->disableSplitHorizon();
    }

    ev << " split horizon (interface id: " << RIPngInterface->getId() << ")." << routerText << endl;
}

void RIPngProcess::setInterfacePoisonReverse(RIPng::Interface *RIPngInterface, bool status)
{
    if (status)
    {
        ev << "   Enabling";
        RIPngInterface->enablePoisonReverse();
    }
    else
    {
        ev << "   Disabling";
        RIPngInterface->disablePoisonReverse();
    }

    ev << " poison reverse (interface id: " << RIPngInterface->getId() << ")." << routerText << endl;
}

void RIPngProcess::addRoutingTableEntryToGlobalRT(RIPng::RoutingTableEntry* entry)
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

void RIPngProcess::removeRoutingTableEntryFromGlobalRT(RIPng::RoutingTableEntry* entry)
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
void RIPngProcess::sendRegularUpdateMessage()
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

    ++regularUpdates;

    bSendTriggeredUpdateMessage = false;
}

void RIPngProcess::sendTriggeredUpdateMessage()
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

    ++triggerUpdates;

    RIPng->resetTimer(triggeredUpdateTimer, uniform(1, 5));
}

void RIPngProcess::sendDelayedTriggeredUpdateMessage()
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
        RIPng->resetTimer(triggeredUpdateTimer, triggeredUpdateTimeout);
    }
    // else - do nothing, a triggered update is already planned
}

RIPngMessage *RIPngProcess::makeUpdateMessageForInterface(RIPng::Interface *interface, bool changed)
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

void RIPngProcess::sendMessage(RIPngMessage *msg, IPv6Address &addr, int port, unsigned long enabledInterfaceIndex, bool globalSourceAddress)
{
    ASSERT(enabledInterfaceIndex < getEnabledInterfacesCount());
    int outInterface = getEnabledInterface(enabledInterfaceIndex)->getId();
    RIPng->sendMessage(msg, addr, port, outInterface, globalSourceAddress);
}

void RIPngProcess::sendAllRoutesRequest()
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

void RIPngProcess::clearRouteChangeFlags()
{
    RIPng::RoutingTableEntry *routingTableEntry;
    RoutingTableIt it;

    for (it = routingTable.begin(); it != routingTable.end(); it++)
    {
        routingTableEntry = (*it);
        routingTableEntry->clearChangeFlag();
    }
}

void RIPngProcess::getRTEs(std::vector<RIPngRTE> &rtes, RIPng::Interface *interface, bool onlyChanged)
{
    RIPng::RoutingTableEntry *routingTableEntry;
    RoutingTableIt it;
    bool splitHorizon = false;
    bool poisonReverse = false;
    int interfaceId = -1;

    if (interface->defaultInformation() && !onlyChanged)
    {//add default route to the update
        RIPngRTE defaultRouteRte;
        defaultRouteRte.setPrefixLen(0);
        defaultRouteRte.setIPv6Prefix(IPv6Address::UNSPECIFIED_ADDRESS);
        defaultRouteRte.setMetric(interface->getDefaultRouteMetric());
        defaultRouteRte.setRouteTag(0);

        rtes.push_back(defaultRouteRte);
    }

    // only default route should be in the update?
    if (!interface->defaultRouteOnly())
    {
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
}

RIPngRTE RIPngProcess::makeRTEFromRoutingTableEntry(RIPng::RoutingTableEntry *routingTableEntry)
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
void RIPngProcess::handleRIPngMessage(RIPngMessage *msg)
{
    UDPDataIndication *controlInfo = check_and_cast<UDPDataIndication *>(msg->getControlInfo());
    IPv6Address sourceAddr = controlInfo->getSrcAddr().get6();
    IPv6Address destAddr = controlInfo->getDestAddr().get6();
    int sourcePort = controlInfo->getSrcPort();
    int sourceInterfaceId = controlInfo->getInterfaceId();

    int ripngIntInd = getEnabledInterfaceIndexById(sourceInterfaceId);
    if (ripngIntInd < 0)
    {//message is from an interface with disabled RIPng
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
        handleResponse(msg, ripngIntInd, sourceAddr);
    }
}

//
//
//-- RESPONSE PROCESSING
//
//
void RIPngProcess::handleResponse(RIPngMessage *response, int srcRIPngIntInd, IPv6Address &srcAddr)
{
    if (!checkMessageValidity(response))
        return;
    EV << "RIPng message: RIPng - Response" << endl;
    processRTEs(response, srcRIPngIntInd, srcAddr);
}

bool RIPngProcess::checkMessageValidity(RIPngMessage *response)
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

void RIPngProcess::processRTEs(RIPngMessage *response, int srcRIPngIntInd, IPv6Address &sourceAddr)
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
        processRTE(rte, srcRIPngIntInd, sourceAddr);
    }

    if (bSendTriggeredUpdateMessage)
    {
        sendTriggeredUpdateMessage();
    }

    bBlockTriggeredUpdateMessage = false;
}

void RIPngProcess::processRTE(RIPngRTE &rte, int srcRIPngIntInd, IPv6Address &sourceAddr)
{
    if (!checkAndLogRTE(rte, sourceAddr))
        return;

    IPv6Address prefix = rte.getIPv6Prefix();
    int prefixLen = rte.getPrefixLen();

    // Check if a route with the prefix exists
    RIPng::RoutingTableEntry *routingTableEntry = getRoutingTableEntry(prefix, prefixLen);

    if (routingTableEntry != NULL)
    {// Update Routing Table Entry (do not try to update dir. connected routes!)
        if (!routingTableEntry->getNextHop().isUnspecified())
                updateRoutingTableEntry(routingTableEntry, rte, srcRIPngIntInd, sourceAddr);
    }
    else
    {// Create and add new Routing Table Entry
        int metric = rte.getMetric();
        metric += getEnabledInterface(srcRIPngIntInd)->getMetricOffset();

        if (metric < infinityMetric)
        {
            RIPng::RoutingTableEntry *route = new RIPng::RoutingTableEntry(prefix, prefixLen);
            route->setAdminDist(distance);
            route->setProcess(this);
            route->setInterfaceId(getEnabledInterface(srcRIPngIntInd)->getId());
            route->setNextHop(sourceAddr);
            route->setMetric(metric);
            route->setChangeFlag();

            addRoutingTableEntry(route);

            bSendTriggeredUpdateMessage = true;
        }
    }
}

bool RIPngProcess::checkAndLogRTE(RIPngRTE &rte, IPv6Address &sourceAddr)
{
    // default route
    if (rte.getIPv6Prefix().isUnspecified() && rte.getPrefixLen() == 0)
    {
        if (sendingDefaultInformation())
        { // default route and default-information is set on one of the interfaces
            EV << "RIPng - Ignoring default route RTE from: " << sourceAddr << endl;
            return false;
        }
        else
        {
            return true;
        }
    }

    // prefix is valid (not multicast, link-local)
    // prefix len. is valid (0-128)
    // metric is valid (0-16)
    if (!rte.getIPv6Prefix().isGlobal() ||
         rte.getPrefixLen() > 128 ||
         rte.getMetric() > 16)
    {
        EV << "RIPng - Bad RTE from: " << sourceAddr << endl;
        return false;
    }

    return true;
}

//
//
//-- REQUEST PROCESSING
//
//
void RIPngProcess::handleRequest(RIPngMessage *request, int srcPort, IPv6Address &srcAddr, IPv6Address &destAddr, unsigned long ripngIntInd)
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
//-- TIMERS
//
//
void RIPngProcess::handleTimer(RIPngTimer *msg)
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

void RIPngProcess::handleRegularUpdateTimer()
{
     // send regular update message
    sendRegularUpdateMessage();
    // plan next regular update
    RIPng->resetTimer(regularUpdateTimer, regularUpdateTimeout);
}

void RIPngProcess::handleTriggeredUpdateTimer()
{
    if (bSendTriggeredUpdateMessage)
        sendTriggeredUpdateMessage();
}

void RIPngProcess::startRouteDeletionProcess(RIPngTimer *timer)
{
    IPv6Address &prefix = timer->getIPv6Prefix();
    int prefixLen = timer->getPrefixLen();
    RIPng::RoutingTableEntry *routingTableEntry = getRoutingTableEntry(prefix, prefixLen);

    startRouteDeletionProcess(routingTableEntry);
}

void RIPngProcess::startRouteDeletionProcess(RIPng::RoutingTableEntry *routingTableEntry)
{
    ASSERT(routingTableEntry != NULL);
    routingTableEntry->setMetric(infinityMetric);
    routingTableEntry->setChangeFlag();

    RIPngTimer *GCTimer = routingTableEntry->getGCTimer();
    // (re)set the timer
    RIPng->resetTimer(GCTimer, routeGarbageCollectionTimeout);

    removeRoutingTableEntryFromGlobalRT(routingTableEntry);

    if (bBlockTriggeredUpdateMessage)
    // if response is processing and a rte with the metric of 16 was received, wait until all the RTEs are proccesed
        bSendTriggeredUpdateMessage = true;
    else
        sendDelayedTriggeredUpdateMessage();
}

void RIPngProcess::deleteRoute(RIPngTimer *timer)
{
    IPv6Address &prefix = timer->getIPv6Prefix();
    int prefixLen = timer->getPrefixLen();

    removeRoutingTableEntry(prefix, prefixLen);
}

//
//
//-- Notification
//
void RIPngProcess::handleNotification(int category, const cObject *details)
{
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
                   downInterfaces.push_back(disableRIPngOnInterface(i));
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
                           RIPng->cancelTimer((*it)->getTimer());
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
               int downRIPngInterfaceIndex = getDownInterfaceIndexById(interfaceEntryId);
               if (downRIPngInterfaceIndex != -1)
               {
                   RIPng::Interface *downRIPngInterface = downInterfaces[downRIPngInterfaceIndex];
                   addEnabledInterface(downRIPngInterface);
                   RIPng->setOutputPortOnInterface(downRIPngInterface, RIPngPort);
                   downInterfaces.erase(downInterfaces.begin() + downRIPngInterfaceIndex);
               }
               else
               {
                   enableRIPngOnInterface(interfaceEntry);
               }

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
