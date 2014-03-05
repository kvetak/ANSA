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
* @file RIPngRouting.cc
* @author Jiri Trhlik (mailto:jiritm@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
* @brief RIPng module
* @detail RIPng module, handles RIPng processes, adds/removes processes, sets parameters to processes.
*/

#include "IPv6InterfaceData.h"

#include "RIPngRouting.h"
#include "RIPngProcess.h"
#include "deviceConfigurator.h"

Define_Module(RIPngRouting);

std::ostream& operator<<(std::ostream& os, RIPngProcess& p)
{
    os << "RIP process '" << p.getProcessName() << "', port " << p.getRIPngPort();
    os << ",  multicast-group " << p.getRIPngAddress().str() << endl;
    os << "   Administrative distance is " << p.getDistance() << "." << endl;
    os << "   Updates every " << p.getRegularUpdateTimeout() << " seconds, expire after " << p.getRouteTimeout() << endl;
    os << "   Garbage collect after " << p.getRouteGarbageCollectionTimeout() << endl;
    os << "   Split horizon is ";
        if (p.isSplitHorizon()) os << "on"; else os << "off";
    os << "; poison reverse is ";
        if (p.isPoisonReverse()) os << "on"; else os << "off";
    os << endl;
    os << "   Default routes are ";
    if (!p.sendingDefaultInformation())
        os << "not ";
    os << "generated" << endl;
    os << "   Periodic updates " << p.getRegularUpdates() << ", trigger updates " << p.getTriggerUpdates() << endl;
    os << " Interfaces:" << endl;
    os << "   ";
    unsigned long numInterfaces = p.getEnabledInterfacesCount();
    for (unsigned long i = 0; i < numInterfaces; ++i)
        os << p.getEnabledInterfaceName(i) << " ";

    os << endl;
    os << " Database:" << endl;
    os << p.getRoutingTable();
    return os;
};

RIPngRouting::RIPngRouting()
{

}

RIPngRouting::~RIPngRouting()
{
    int numProcesses = processes.size();
    for (int i = 0; i < numProcesses; ++i)
    {
       delete processes[i];
    }
    processes.clear();

    for (GlobalSockets::iterator it = globalSockets.begin(); it != globalSockets.end(); ++it)
    {
        delete it->second;
    }
    globalSockets.clear();

    for (Sockets::iterator it = sockets.begin(); it != sockets.end(); ++it)
    {
        delete it->second;
    }
    sockets.clear();
}

//
//
//-- INTERFACES METHODS
//
//
RIPng::Interface *RIPngRouting::enableRIPngOnInterface(const char *processName, const char *interfaceName)
{
    RIPngProcess *process = getProcess(processName);
    if (process == NULL)
        process = addProcess(processName);

    ev << "   Enabling RIPng on " << interfaceName << routerText << endl;

    InterfaceEntry *interface = ift->getInterfaceByName(interfaceName);
    if (interface == NULL)
        return NULL;

    return enableRIPngOnInterface(process, interface);
}

RIPng::Interface *RIPngRouting::enableRIPngOnInterface(RIPngProcess *process, InterfaceEntry *interface)
{
    ASSERT(process != NULL);

    RIPng::Interface *RIPngInterface = process->enableRIPngOnInterface(interface);
    if (RIPngInterface == NULL)
        return NULL;

    return RIPngInterface;
}

void RIPngRouting::disableRIPngOnInterface(const char *processName, const char *interfaceName)
{
    RIPngProcess *process = getProcess(processName);
    if (process != NULL)
    {
        ev << "   Disabling RIPng on " << interfaceName << routerText << endl;

        int interfaceIndex = process->getEnabledInterfaceIndexByName(interfaceName);
        if (interfaceIndex != -1)
            disableRIPngOnInterface(process, interfaceIndex);
    }
}

void RIPngRouting::disableRIPngOnInterface(RIPngProcess *process, int RIPngInterfaceIndex)
{
    ASSERT(process != NULL);

    delete process->disableRIPngOnInterface(RIPngInterfaceIndex);
}

RIPng::Interface *RIPngRouting::setOutputPortOnInterface(RIPng::Interface *interface, int port)
{
    ASSERT(interface != NULL);

    moveInterfaceToSocket(interface, port);

    return interface;
}

void RIPngRouting::moveInterfaceToSocket(RIPng::Interface *interface, int port)
{
    int interfaceId = interface->getId();

    RIPng::Socket *oldSocket = interface->getOutputSocket();
    if (oldSocket)
    {
        int numOldSocketInterfaces = oldSocket->RIPngInterfaces.size();
        if (numOldSocketInterfaces <= 1)
        {//If this was last interface which has been using that socket structure
            oldSocket->socket.close();

            sockets.erase(SocketsKey(interfaceId, oldSocket->port));
            delete oldSocket;
        }
        else
        {
            oldSocket->removeInterface(interface);
        }

        interface->setOutputSocket(NULL);
    }

    if (port == -1)
        return;

    Sockets::iterator it = sockets.find(SocketsKey(interfaceId, port));
    if (it == sockets.end())
    {
        UDPSocket socket;
        socket.setOutputGate(gate("udpOut"));
        socket.setReuseAddress(true);
        //so every RIPng message sent from RIPng interface uses correct link-local source address
        socket.bind(ift->getInterfaceById(interfaceId)->ipv6Data()->getLinkLocalAddress(), port);

        int timeToLive = par("timeToLive");
        if (timeToLive != -1)
            socket.setTimeToLive(timeToLive);

        RIPng::Socket *socketStruct = new RIPng::Socket();
        socketStruct->socket = socket;
        socketStruct->port = port;
        sockets.insert(std::make_pair(SocketsKey(interfaceId, port), socketStruct));

        socketStruct->RIPngInterfaces.push_back(interface);
        interface->setOutputSocket(socketStruct);
    }
    else
    {
        it->second->RIPngInterfaces.push_back(interface);
        interface->setOutputSocket(it->second);
    }
}

//
//
//-- PROCESS MANAGEMENT
//
//
RIPngProcess *RIPngRouting::getProcess(const char *processName)
{
    int numProcesses = processes.size();
    for (int i = 0; i < numProcesses; ++i)
    {
        if (processes[i]->getProcessName() == std::string(processName))
            return processes[i];
    }

    return NULL;
}

unsigned int RIPngRouting::getProcessIndex(const char *processName)
{
    int numProcesses = processes.size();
    for (int i = 0; i < numProcesses; ++i)
    {
        if (processes[i]->getProcessName() == std::string(processName))
            return i;
    }

    return -1;
}

RIPngProcess *RIPngRouting::addProcess(const char *processName)
{
    ASSERT(processName != NULL);

    RIPngProcess *newProcess;
    if ((newProcess = getProcess(processName)) != NULL)
        return newProcess;

    newProcess = new RIPngProcess(processName, this);
    processes.push_back(newProcess);
    moveProcessToSocket(newProcess, -1, RIPngPort, RIPngAddress);

    updateDisplayString();

    return newProcess;
}

void RIPngRouting::removeProcess(const char *processName)
{
    int processIndex = getProcessIndex(processName);
    if (processIndex != -1)
    {
        RIPngProcess *process = processes[processIndex];
        process->stop();
        delete process;
        processes.erase(processes.begin() + processIndex);

        updateDisplayString();
    }
}

void RIPngRouting::moveProcessToSocket(RIPngProcess *process, int oldPort, int port, IPv6Address &multicastAddress)
{
    GlobalSockets::iterator it;
    if (oldPort != -1)
    {
        it = globalSockets.find(oldPort);
        if (it != globalSockets.end())
        {
            RIPng::GlobalSocket *oldSocket = it->second;
            int numOldSocketProcesses = oldSocket->processes.size();
            if (numOldSocketProcesses <= 1)
            {//If this was last process which has been using that socket for receiving messages
                //Leaving multicast groups is not necessary, since it has no effect when
                //closing socket (multicast addresses cannot be removed from the interfaces).
                oldSocket->socket.close();
                globalSockets.erase(it);
                delete oldSocket;
            }
            else
            {
                oldSocket->removeProcess(process);
            }
        }
    }

    it = globalSockets.find(port);
    if (it == globalSockets.end())
    {
        RIPng::GlobalSocket *globalSocket = new RIPng::GlobalSocket();
        globalSocket->socket.setOutputGate(gate("udpOut"));
        globalSocket->socket.setReuseAddress(true);
        globalSocket->socket.bind(port);
        globalSocket->socket.joinMulticastGroup(multicastAddress, -1);
        globalSockets[port] = globalSocket;

        globalSocket->processes.push_back(process);
    }
    else
    {
        it->second->processes.push_back(process);
        it->second->socket.joinMulticastGroup(multicastAddress, -1);
    }

    unsigned long numInterfaces = process->getEnabledInterfacesCount();
    for (unsigned long i = 0; i < numInterfaces; ++i)
    {
        RIPng::Interface *RIPngInterface = process->getEnabledInterface(i);
        moveInterfaceToSocket(RIPngInterface, port);
    }
}

//
//
//-- MESSAGES HANDLING
//
//
void RIPngRouting::sendMessage(RIPngMessage *msg, IPv6Address &addr, int port, int interfaceId, bool globalSourceAddress)
{
    if (globalSourceAddress)
    {// "uses" global-unicast address as the source address
        GlobalSockets::iterator it = globalSockets.find(port);
        if (it != globalSockets.end())
        {
            it->second->socket.sendTo(msg, addr, port, interfaceId);
        }
    }
    else
    {
        Sockets::iterator it = sockets.find(SocketsKey(interfaceId, port));
        if (it != sockets.end())
        {
            it->second->socket.sendTo(msg, addr, port, interfaceId);
        }
    }
}

void RIPngRouting::forwardRIPngMessage(RIPngMessage *msg)
{
    UDPDataIndication *controlInfo = check_and_cast<UDPDataIndication *>(msg->getControlInfo());
    int sourceInterfaceId = controlInfo->getInterfaceId();
    int destPort = controlInfo->getDestPort();
    const IPv6Address &destAddr = controlInfo->getDestAddr().get6();

    Sockets::iterator it = sockets.find(SocketsKey(sourceInterfaceId, destPort));
    if (it != sockets.end())
    {//Forward the message to the RIPng process running on the given port
        std::vector<RIPng::Interface *> ifaces = it->second->RIPngInterfaces;
        int numInterfaces = ifaces.size();
        for (int i = 0; i < numInterfaces; ++i)
        {
            RIPngProcess *process = ifaces[i]->getProcess();
            // If destAddr is unicast forward to the first process on the interface
            // else forward to the process with the given multicast-address
            if (process && (destAddr.isUnicast() || process->getRIPngAddress() == destAddr))
            {
                process->handleRIPngMessage(msg);
                break;
            }
        }
    }

    delete msg;
}

//
//
//-- COMMANDS
//
//
void RIPngRouting::setPortAndAddress(const char *processName, int port, IPv6Address &address)
{
    RIPngProcess *process = getProcess(processName);
    if (process != NULL)
        setPortAndAddress(process, port, address);
}

void RIPngRouting::setPortAndAddress(RIPngProcess *process, int port, IPv6Address &address)
{
    int oldPort = process->getRIPngPort();

    if (!process->setRIPngPort(port))
    {
        ev << "   Invalid port: " << port << routerText << endl;
        return;
    }

    if (!process->setRIPngAddress(address))
    {
        ev << "   Invalid multicast address: " << address << routerText << endl;
        process->setRIPngPort(oldPort);
        return;
    }

    ev << "   Setting RIPng port: " << port << " and address: " << address.str() << routerText << endl;

    moveProcessToSocket(process, oldPort, port, address);
}

void RIPngRouting::setDistance(const char *processName, int distance)
{
    RIPngProcess *process = getProcess(processName);
    if (process != NULL)
        setDistance(process, distance);
}

void RIPngRouting::setDistance(RIPngProcess *process, int distance)
{
    if (!process->setDistance(distance))
        ev << "   Invalid administrative distance: " << distance << routerText << endl;

    ev << "   Setting RIPng distance: " << distance << routerText << endl;
}

void RIPngRouting::setPoisonReverse(const char *processName, bool poisonReverse)
{
    RIPngProcess *process = getProcess(processName);
    if (process != NULL)
        setPoisonReverse(process, poisonReverse);
}

void RIPngRouting::setPoisonReverse(RIPngProcess *process, bool poisonReverse)
{
    if (poisonReverse == true)
    {
        process->enablePoisonReverse();
        ev << "   Enabling RIPng poison-reverse" << routerText << endl;
    }
    else
    {
        process->disablePoisonReverse();
        ev << "   Disabling RIPng poison-reverse" << routerText << endl;
    }

    RIPng::Interface *interface;
    unsigned long numInterfaces = process->getEnabledInterfacesCount();
    for (unsigned long i = 0; i < numInterfaces; ++i)
    {
        interface = process->getEnabledInterface(i);
        if (poisonReverse)
            interface->enablePoisonReverse();
        else
            interface->disablePoisonReverse();
    }
}

void RIPngRouting::setSplitHorizon(const char *processName, bool splitHorizon)
{
    RIPngProcess *process = getProcess(processName);
    if (process != NULL)
        setSplitHorizon(process, splitHorizon);

}

void RIPngRouting::setSplitHorizon(RIPngProcess *process, bool splitHorizon)
{
    if (splitHorizon == true)
    {
        process->enableSplitHorizon();
        ev << "   Enabling RIPng split-horizon" << routerText << endl;
    }
    else
    {
        process->disableSplitHorizon();
        ev << "   Disabling RIPng split-horizon" << routerText << endl;
    }

    RIPng::Interface *interface;
    unsigned long numInterfaces = process->getEnabledInterfacesCount();
    for (unsigned long i = 0; i < numInterfaces; ++i)
    {
        interface = process->getEnabledInterface(i);
        if (splitHorizon)
            interface->enableSplitHorizon();
        else
            interface->disableSplitHorizon();
    }
}

void RIPngRouting::setTimers(const char *processName, int update, int route, int garbage)
{
    RIPngProcess *process = getProcess(processName);
    if (process != NULL)
        setTimers(process, update, route, garbage);
}

void RIPngRouting::setTimers(RIPngProcess *process, int update, int route, int garbage)
{
    ev << "   Setting RIPng timers: " << update << " " << route << " " << garbage << routerText << endl;
    if (update > -1)
        process->setRegularUpdateTimeout(update);
    if (route > -1)
        process->setRouteTimeout(route);
    if (garbage > -1)
        process->setRouteGarbageCollectionTimeout(garbage);
}

//
//
//-- TIMERS
//
//
RIPngTimer *RIPngRouting::createTimer(int timerKind, void *context)
{
    RIPngTimer *timer = new RIPngTimer();
    timer->setTimerKind(timerKind);
    timer->setContextPointer(context);

    return timer;
}

RIPngTimer *RIPngRouting::createAndStartTimer(int timerKind, void *context, simtime_t timerLen)
{
    RIPngTimer *timer = createTimer(timerKind, context);

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

void RIPngRouting::forwardRIPngTimer(RIPngTimer *timer)
{
    int type = timer->getTimerKind();
    RIPngProcess *process = NULL;
    RIPng::RoutingTableEntry *rte = NULL;

    switch (type)
    {
        case RIPNG_GENERAL_UPDATE :
        case RIPNG_TRIGGERED_UPDATE :
            process = reinterpret_cast<RIPngProcess *>(timer->getContextPointer());
            break;
        case RIPNG_ROUTE_TIMEOUT :
        case RIPNG_ROUTE_GARBAGE_COLECTION_TIMEOUT :
            rte = reinterpret_cast<RIPng::RoutingTableEntry *>(timer->getContextPointer());
            if (rte)
                process = rte->getProcess();
            break;
        default:
            break;
    }

    if (process)
        process->handleTimer(timer);
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

    // access to the interface table
    ift = InterfaceTableAccess().get();

    // get the hostname
    cModule *containingMod = findContainingNode(this);
    if (!containingMod)
        hostName = "";
    else
        hostName = containingMod->getFullName();

    routerText = " (Router " + hostName + ") ";

    const char *RIPngAddressString = par("RIPngAddress");
    RIPngAddress = IPv6Address(RIPngAddressString);
    RIPngPort = par("RIPngPort");

    connNetworkMetric = par("connectedNetworkMetric");
    infinityMetric = par("infinityMetric");

    distance = par("distance");

    routeTimeout = par("routeTimeout").doubleValue();
    routeGarbageCollectionTimeout = par("routeGarbageCollectionTimeout").doubleValue();
    regularUpdateTimeout = par("regularUpdateInterval").doubleValue();

    // get deviceId
    deviceId = par("deviceId");

    // subscribe for changes in the device
    nb = NotificationBoardAccess().get();
    nb->subscribe(this, NF_INTERFACE_STATE_CHANGED);
    nb->subscribe(this, NF_IPv6_ROUTE_DELETED);

    // read the RIPng process configuration
    DeviceConfigurator *devConf = ModuleAccess<DeviceConfigurator>("deviceConfigurator").get();
    devConf->loadRIPngConfig(this);

    WATCH_PTRVECTOR(processes);

    updateDisplayString();

    int numProcesses = processes.size();
    for (int i = 0; i < numProcesses; ++i)
    {//Start every RIPng process;
       processes[i]->start();
    }
}

void RIPngRouting::updateDisplayString()
{
    if (ev.isGUI())
    {
        char buf[40];
        const char *suffix = "es";
        if (processes.size() == 1)
            suffix = "";
        sprintf(buf, "%d process%s", processes.size(), suffix);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

void RIPngRouting::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {// timers
        forwardRIPngTimer(check_and_cast<RIPngTimer*> (msg));
    }
    else if (msg->getKind() == UDP_I_DATA)
    {// process incoming message
        forwardRIPngMessage(check_and_cast<RIPngMessage*> (msg));
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

void RIPngRouting::receiveChangeNotification(int category, const cObject *details)
{
   // ignore notifications during initialization
   if (simulation.getContextType() == CTX_INITIALIZE)
       return;

   Enter_Method_Silent();
   printNotificationBanner(category, details);

   //Deliver notification to every process
   int numProcesses = processes.size();
   for (int i = 0; i < numProcesses; ++i)
       processes[i]->handleNotification(category, details);
}
