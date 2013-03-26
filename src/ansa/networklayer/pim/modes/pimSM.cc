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
 * @file pimSM.cc
 * @date 29.10.2011
 * @author: Veronika Rybova, Tomas Prochazka (mailto:xproch21@stud.fit.vutbr.cz), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @brief File implements PIM sparse mode.
 * @details Implementation will be done in the future according to RFC4601.
 */

#include "pimSM.h"
#include "deviceConfigurator.h"


Define_Module(pimSM);

void pimSM::handleMessage(cMessage *msg)
{
	EV << "PIMSM::handleMessage" << endl;

	// self message (timer)
	if (msg->isSelfMessage())
	{
	   EV << "PIMSM::handleMessage:Timer" << endl;
	   PIMTimer *timer = check_and_cast <PIMTimer *> (msg);
	   processPIMTimer(timer);
	}
	else if (dynamic_cast<PIMPacket *>(msg))
	{
	   EV << "PIMSM::handleMessage: PIM-SM packet" << endl;
	   PIMPacket *pkt = check_and_cast<PIMPacket *>(msg);
	   EV << "Version: " << pkt->getVersion() << ", type: " << pkt->getType() << endl;
	   processPIMPkt(pkt);
	}
	else
	   EV << "PIMSM::handleMessage: Wrong message" << endl;
}

void pimSM::initialize(int stage)
{
    if (stage == 4)
    {
        EV << "PIMSM::initialize: entering..." << endl;

        // Pointer to routing tables, interface tables, notification board
        rt = AnsaRoutingTableAccess().get();
        ift = InterfaceTableAccess().get();
        nb = NotificationBoardAccess().get();
        pimIft = PimInterfaceTableAccess().get();
        pimNbt = PimNeighborTableAccess().get();

        // is PIM enabled?
        if (pimIft->getNumInterface() == 0)
        {
            EV << "PIM is NOT enabled on device " << endl;
            return;
        }

        // subscribe for notifications
        nb->subscribe(this, NF_IPv4_NEW_MULTICAST_SPARSE);
        nb->subscribe(this, NF_IPv4_MDATA_REGISTER);
        nb->subscribe(this, NF_IPv4_REC_REGISTER_STOP);
        nb->subscribe(this, NF_IPv4_NEW_IGMP);

        DeviceConfigurator *devConf = ModuleAccess<DeviceConfigurator>("deviceConfigurator").get();
        devConf->loadPimGlobalConfig(this);

    }
}

/**
 * CREATE KEEP ALIVE TIMER
 *
 * The method is used to create PIMKeepAliveTimer timer. The timer is set when source of multicast is
 * connected directly to the router.  If timer expires, router will remove the route from multicast
 * routing table. It is set to (S,G).
 *
 * @param source IP address of multicast source.
 * @param group IP address of multicast group.
 * @return Pointer to new Keep Alive Timer
 * @see PIMkat()
 */
PIMkat* pimSM::createKeepAliveTimer(IPv4Address source, IPv4Address group)
{
    PIMkat *timer = new PIMkat();
    timer->setName("PIMKeepAliveTimer");
    timer->setSource(source);
    timer->setGroup(group);
    if (group == IPv4Address::UNSPECIFIED_ADDRESS)
        scheduleAt(simTime() + 2*KAT, timer);
    else
        scheduleAt(simTime() + KAT, timer);
    return timer;
}


/**
 * CREATE REGISTER-STOP TIMER
 *
 * The method is used to create PIMRegisterStopTimer timer.
 *
 * @param source IP address of multicast source.
 * @param group IP address of multicast group.
 * @return Pointer to new Register Stop Timer
 * @see PIMrst()
 */
PIMrst* pimSM::createRegisterStopTimer(IPv4Address source, IPv4Address group)
{
    PIMrst *timer = new PIMrst();
    timer->setName("PIMRegisterStopTimer");
    timer->setSource(source);
    timer->setGroup(group);

    scheduleAt(simTime() + RST - REGISTER_PROBE_TIME, timer);
    return timer;
}

/**
 * CREATE EXPIRY TIMER
 *
 * The method is used to create PIMExpiryTimer.
 *
 * @param source IP address of multicast source.
 * @param group IP address of multicast group.
 * @return Pointer to new Expiry Timer
 * @see PIMet()
 */
PIMet* pimSM::createExpiryTimer(int holdtime, IPv4Address group)
{
    PIMet *timer = new PIMet();
    timer->setName("PIMExpiryTimer");
    timer->setGroup(group);

    scheduleAt(simTime() + holdtime, timer);
    return timer;
}

/**
 * CREATE JOIN TIMER
 *
 * The method is used to create PIMJoinTimer.
 *
 * @param source IP address of multicast source.
 * @param group IP address of multicast group.
 * @return Pointer to new Join Timer
 * @see PIMjt()
 */
PIMjt* pimSM::createJoinTimer(IPv4Address group, IPv4Address JPaddr, IPv4Address upstreamNbr)
{
    PIMjt *timer = new PIMjt();
    timer->setName("PIMJoinTimer");
    timer->setGroup(group);
    timer->setJoinPruneAddr(JPaddr);
    timer->setUpstreamNbr(upstreamNbr);

    scheduleAt(simTime() + JT, timer);
    return timer;
}

void pimSM::setRPAddress(std::string address)
{
    if (address != "")
    {
        std::string RP (address);
        RPAddress = IPv4Address(RP.c_str());
    }
    else
        EV << "PIMSM::setRPAddress: empty RP address" << endl;
}

void pimSM::setSPTthreshold(std::string threshold)
{
    if (threshold != "")
        SPTthreshold.append(threshold);
    else
        EV << "PIMSM::setSPTthreshold: bad SPTthreshold" << endl;
}

bool pimSM::IamRP ()
{
    InterfaceEntry *intf;

    for (int i=0; i < ift->getNumInterfaces(); i++)
    {
        intf = ift->getInterface(i);
        if (intf->ipv4Data()->getIPAddress() == this->getRPAddress())
            return true;
    }
    return false;
}

bool pimSM::IamDR (IPv4Address sourceAddr)
{
    InterfaceEntry *intf;
    IPv4Address intfAddr;

    for (int i=0; i < ift->getNumInterfaces(); i++)
    {
        intf = ift->getInterface(i);
        intfAddr = intf->ipv4Data()->getIPAddress();

        //if(intfAddr.getNetmaskLength() != sourceAddr.getNetmaskLength())
            //continue;

        if (IPv4Address::maskedAddrAreEqual(intfAddr,sourceAddr,sourceAddr.getNetworkMask()))
        {
            EV << "I AM DR for: " << intfAddr << endl;
            return true;
        }
    }
    EV << "I AM NOT DR " << endl;
    return false;
}

/**
 * PROCESS KEEP ALIVE TIMER
 *
 * The method is used to process PIM Keep Alive Timer. It is (S,G) timer. When Keep Alive Timer expires,
 * route is removed from multicast routing table.
 *
 * @param timer Pointer to Keep Alive Timer.
 * @see PIMkat()
 */
void pimSM::processKeepAliveTimer(PIMkat *timer)
{
    EV << "pimSM::processKeepAliveTimer: route will be deleted" << endl;
    AnsaIPv4MulticastRoute *route = rt->getRouteFor(timer->getGroup(), timer->getSource());

    delete timer;
    route->setKat(NULL);
    rt->deleteMulticastRoute(route);

}

void pimSM::processRegisterStopTimer(PIMrst *timer)
{
    EV << "pimSM::processRegisterStopTimer: " << endl;

    sendPIMRegisterNull(timer->getSource(), timer->getGroup());
    //TODO set RST to probe time, if RST expires, add encapsulation tunnel
}

void pimSM::processExpiryTimer(PIMet *timer)
{
    EV << "pimSM::processExpiryTimer:" << endl;
    AnsaIPv4MulticastRoute *route = rt->getRouteFor(timer->getGroup(), IPv4Address::UNSPECIFIED_ADDRESS);

    delete timer;
    route->setEt(NULL);
    rt->deleteMulticastRoute(route);
}

void pimSM::processJoinTimer(PIMjt *timer)
{
    EV << "pimSM::processJoinTimer:" << endl;
    AnsaIPv4MulticastRoute *route = rt->getRouteFor(timer->getGroup(), IPv4Address::UNSPECIFIED_ADDRESS);

    if (route)
    {
        // send periodic join (*,G)
        if (!IamRP())
            sendPIMJoin(timer->getGroup(),timer->getJoinPruneAddr(),timer->getUpstreamNbr());
        // restart JT timer
        cancelEvent(route->getJt());
        scheduleAt(simTime() + JT, route->getJt());
    }
    else
        delete timer;

}


/**
 * PROCESS PIM TIMER
 *
 * The method is used to process PIM timers. According to type of PIM timer, the timer is sent to
 * appropriate method.
 *
 * @param timer Pointer to PIM timer.
 * @see PIMTimer()
 * @see processPruneTimer()
 * @see processGraftRetryTimer()
 */
void pimSM::processPIMTimer(PIMTimer *timer)
{
    EV << "pimSM::processPIMTimer: ";

    switch(timer->getTimerKind())
    {
        case JoinTimer:
            EV << "JoinTimer" << endl;
            processJoinTimer(check_and_cast<PIMjt *> (timer));
            break;
        case ExpiryTimer:
            EV << "ExpiryTimer" << endl;
            processExpiryTimer(check_and_cast<PIMet *> (timer));
            break;
        case KeepAliveTimer:
            EV << "KeepAliveTimer" << endl;
            processKeepAliveTimer(check_and_cast<PIMkat *> (timer));
            break;
        case RegisterStopTimer:
            EV << "RegisterStopTimer" << endl;
            processRegisterStopTimer(check_and_cast<PIMrst *> (timer));
            break;
        default:
            EV << "BAD TYPE, DROPPED" << endl;
            delete timer;
            break;
    }
}


void pimSM::processJoinPrunePacket(PIMJoinPrune *pkt)
{

    // go through list of multicast groups
    for (unsigned int i = 0; i < pkt->getMulticastGroupsArraySize(); i++)
    {
        MulticastGroup group = pkt->getMulticastGroups(i);
        IPv4Address multGroup = group.getGroupAddress();
        IPv4Address multOrigin;
        AnsaIPv4MulticastRoute *newRouteG = new AnsaIPv4MulticastRoute();
        AnsaIPv4MulticastRoute *routePointer;
        IPv4ControlInfo *ctrl = (IPv4ControlInfo *) pkt->getControlInfo();
        InterfaceEntry *newInIntG = rt->getInterfaceForDestAddr(this->RPAddress);
        PimNeighbor *neighborToRP = pimNbt->getNeighborByIntID(newInIntG->getInterfaceId());

        // check if (*,G) exist
        routePointer = newRouteG;
        if (!(newRouteG = rt->getRouteFor(multGroup, IPv4Address::UNSPECIFIED_ADDRESS)))
        {
            newRouteG = routePointer;
            // set source, mult. group, etc...
            newRouteG->setOrigin(IPv4Address::UNSPECIFIED_ADDRESS);
            newRouteG->setMulticastGroup(multGroup);
            newRouteG->setRP(this->getRPAddress());
            newRouteG->addFlag(S);
            newRouteG->addFlag(C);

            // set incoming interface
            if (!IamRP())
                newRouteG->setInInt(newInIntG, newInIntG->getInterfaceId(), neighborToRP->getAddr());

            // set outgoing interface
            InterfaceEntry *interface = rt->getInterfaceForDestAddr(ctrl->getSrcAddr());

            outInterface newOutInt;
            newOutInt.intId = interface->getInterfaceId();
            newOutInt.intPtr = interface;
            newOutInt.forwarding = Forward;
            newOutInt.mode = Sparsemode;
            newOutInt.pruneTimer = NULL;
            newRouteG->addOutInt(newOutInt);

            // create and set (*,G) ET and JT timers
            PIMet* timerEt = createExpiryTimer(pkt->getHoldTime(), multGroup);
            PIMjt* timerJt = createJoinTimer(multGroup, this->getRPAddress(), neighborToRP->getAddr());
            newRouteG->setEt(timerEt);
            newRouteG->setJt(timerJt);

            rt->addMulticastRoute(newRouteG);

            // oilist != NULL && I am not RP -> send Join (*,G) to RP
            if (IamRP() != true)
                sendPIMJoin(multGroup,this->getRPAddress(),neighborToRP->getAddr());
        }
        else
        {
            EV << "pimSM::processJoinPrunePacket - (*,G) Join exist" << endl;

            std::vector<AnsaIPv4MulticastRoute*> routes = rt->getRouteFor(multGroup);
            AnsaIPv4MulticastRoute *route = new AnsaIPv4MulticastRoute();

            for (unsigned i=0; i<routes.size();i++)
            {
                route = routes[i];
                multOrigin = route->getOrigin();
                if (route->getOrigin() != IPv4Address::UNSPECIFIED_ADDRESS)     // only if (S,G exist)
                {
                    if (IamRP() || IamDR(multOrigin))
                    {
                        if (IamRP())
                        {
                            route->removeFlag(P);                   // update flags
                            route->addFlag(T);
                            newRouteG->removeFlag(P);
                        }

                        InterfaceVector vect = route->getOutInt();
                        if (vect.size() == 0)                   // Has route any outgoing interface?
                        {
                            // set outgoing interface
                            InterfaceEntry *interface = rt->getInterfaceForDestAddr(ctrl->getSrcAddr());
                            outInterface newOutInt;
                            newOutInt.intId = interface->getInterfaceId();
                            newOutInt.intPtr = interface;
                            newOutInt.forwarding = Forward;
                            newOutInt.mode = Sparsemode;
                            newOutInt.pruneTimer = NULL;
                            route->addOutInt(newOutInt);        // add outgoing interface for (S,G)

                            newRouteG->addOutInt(newOutInt);    // add outgoing interface for (*,G)
                            rt->generateShowIPMroute();         // refresh output in MRT

                            // oilist != NULL -> send Join (S,G) to register DR
                            sendPIMJoin(multGroup, multOrigin, route->getInIntNextHop());
                        }
                        else        // Source DR has hiden outgoing interface
                        {
                            route->removeFlag(P);
                            newRouteG->removeFlag(F);
                            for (unsigned j=0; j < vect.size(); j++)
                            {
                                vect[j].forwarding = Forward;
                                route->setOutInt(vect);
                            }
                            route->setOutShowIntStatus(true);
                            rt->generateShowIPMroute();         // refresh output in MRT
                        }
                    }
                }
            }

            // restart ET timer
            if (newRouteG->getEt())
            {
                EV << " (*,G) ET timer refresh" << endl;
                cancelEvent(newRouteG->getEt());
                scheduleAt(simTime() + pkt->getHoldTime(), newRouteG->getEt());
            }
        }
    }
}

void pimSM::processRegisterPacket(PIMRegister *pkt)
{
    EV << "pimSM:processRegisterPacket" << endl;

    AnsaIPv4MulticastRoute *newRouteG = new AnsaIPv4MulticastRoute();
    AnsaIPv4MulticastRoute *newRoute = new AnsaIPv4MulticastRoute();
    AnsaIPv4MulticastRoute *routePointer;
    IPv4Datagram *datagram = &(pkt->getMultDatagram());
    IPv4Address multOrigin = datagram->getSrcAddress();
    IPv4Address multGroup = datagram->getDestAddress();
    EV << "origin: " << multOrigin << endl;
    EV << "group: " << multGroup << endl;

    if (!pkt->getN())
    {
        // check if exist (*,G)
        routePointer = newRouteG;
        if (!(newRouteG = rt->getRouteFor(multGroup, IPv4Address::UNSPECIFIED_ADDRESS)))
        {
            EV << "pimSM:processRegisterPacket - (*,G) not created - creating" << endl;
            newRouteG = routePointer;
            newRouteG->setMulticastGroup(multGroup);
            newRouteG->setOrigin(IPv4Address::UNSPECIFIED_ADDRESS);
            newRouteG->setRP(this->getRPAddress());
            newRouteG->addFlag(S);
            newRouteG->addFlag(P);

            // create and set (*,G) KAT timer
            PIMkat* timerKatG = createKeepAliveTimer(IPv4Address::UNSPECIFIED_ADDRESS, newRouteG->getMulticastGroup());
            newRouteG->setKat(timerKatG);

            rt->addMulticastRoute(newRouteG);
        }

        // check if exist (S,G)
        routePointer = newRoute;
        if (!(newRoute = rt->getRouteFor(multGroup,multOrigin)))
        {
            InterfaceEntry *newInIntG = rt->getInterfaceForDestAddr(multOrigin);
            PimNeighbor *pimIntfToDR = pimNbt->getNeighborByIntID(newInIntG->getInterfaceId());
            newRoute = routePointer;
            newRoute->setInInt(newInIntG, newInIntG->getInterfaceId(), pimIntfToDR->getAddr());
            newRoute->setMulticastGroup(multGroup);
            newRoute->setOrigin(multOrigin);
            newRoute->setRP(this->getRPAddress());
            newRoute->addFlag(P);

            // create and set (S,G) KAT timer
            PIMkat* timerKat = createKeepAliveTimer(newRoute->getOrigin(), newRoute->getMulticastGroup());
            newRoute->setKat(timerKat);

            rt->addMulticastRoute(newRoute);
        }

        // refresh KAT timers
        if (newRoute->getKat())
        {
            EV << " (S,G) KAT timer refresh" << endl;
            cancelEvent(newRoute->getKat());
            scheduleAt(simTime() + KAT, newRoute->getKat());
        }
        if (newRouteG->getKat())
        {
            EV << " (*,G) KAT timer refresh" << endl;
            cancelEvent(newRouteG->getKat());
            scheduleAt(simTime() + 2*KAT, newRouteG->getKat());
        }
    }

    //TODO
    // if exist (*,G)
        // check if PIM Register isn't null register message
        // decapsulate mult data and send them to (*,G)
        // send join(S,G) to registrating DR first, after come mult data to RP, send register-stop
    // else

    // send register-stop packet
    IPv4ControlInfo *PIMctrl =  (IPv4ControlInfo *) pkt->getControlInfo();
    sendPIMRegisterStop(PIMctrl->getDestAddr(),PIMctrl->getSrcAddr(),multGroup,multOrigin);
}

void pimSM::processRegisterStopPacket(PIMRegisterStop *pkt)
{
    EV << "pimSM:processRegisterStopPacket" << endl;

    AnsaIPv4MulticastRoute *routeSG = new AnsaIPv4MulticastRoute();
    InterfaceEntry *intToRP = rt->getInterfaceForDestAddr(this->getRPAddress());
    int intToRPId;
    // Set RST timer
    PIMrst* timerRST = createRegisterStopTimer(pkt->getSourceAddress(), pkt->getGroupAddress());

    routeSG = rt->getRouteFor(pkt->getGroupAddress(),pkt->getSourceAddress());
    routeSG->setRst(timerRST);

    if (routeSG == NULL)
        throw cRuntimeError("pimSM::processRegisterStopPacket - route for (S,G) not found!");

    EV << "interface to RP:" << intToRP->getInterfaceId() << endl;
    intToRPId = intToRP->getInterfaceId();
    if (routeSG->getRegStatus(intToRPId) == Join)
    {
        EV << "Register tunnel is connect - has to be disconnect" << endl;
        routeSG->setRegStatus(intToRPId,Prune);
    }
    else
        EV << "Register tunnel is disconnect" << endl;

}

/**
 * PROCESS PIM PACKET
 *
 * The method is used to process PIM packets. According to type of PIM packet, the packet is sent to
 * appropriate method.
 *
 * @param pkt Pointer to PIM packet.
 * @see PIMPacket()
 */
void pimSM::processPIMPkt(PIMPacket *pkt)
{
    EV << "pimSM::processPIMPkt: ";

    switch(pkt->getType())
    {
        case JoinPrune:
            EV << "JoinPrune" << endl;
            processJoinPrunePacket(check_and_cast<PIMJoinPrune *> (pkt));
            break;

        case Register:
            EV << "Register" << endl;
            processRegisterPacket(check_and_cast<PIMRegister *> (pkt));
            break;
        case RegisterStop:
            EV << "Register-stop" << endl;
            processRegisterStopPacket(check_and_cast<PIMRegisterStop *> (pkt));
            break;
        case Assert:
            EV << "Assert" << endl;
            // FIXME for future use
            break;
        default:
            EV << "BAD TYPE, DROPPED" << endl;
            delete pkt;
            break;
    }
}

void pimSM::sendPIMJoin(IPv4Address multGroup, IPv4Address joinIPaddr, IPv4Address upstreamNbr)
{
    EV << "pimSM::sendPIMJoinToRP - assembling (*,G) Joint to RP" << endl;

    // create PIM Register datagram
    PIMJoinPrune *msg = new PIMJoinPrune();
    MulticastGroup *group = new MulticastGroup();

    // set PIM packet
    msg->setName("PIMJoin/Prune");
    msg->setType(JoinPrune);
    msg->setUpstreamNeighborAddress(upstreamNbr);
    msg->setHoldTime(HOLDTIME);

    msg->setMulticastGroupsArraySize(1);
    group->setGroupAddress(multGroup);
    group->setJoinedSourceAddressArraySize(1);
    group->setPrunedSourceAddressArraySize(0);
    group->setJoinedSourceAddress(0,joinIPaddr);
    msg->setMulticastGroups(0, *group);

    // set IP Control info
    IPv4ControlInfo *ctrl = new IPv4ControlInfo();
    InterfaceEntry *interfaceToRP = rt->getInterfaceForDestAddr(joinIPaddr);

    IPv4Address ga1("224.0.0.13");
    ctrl->setSrcAddr(interfaceToRP->ipv4Data()->getIPAddress());                            //FIXME check if it works!
    ctrl->setDestAddr(ga1);
    ctrl->setProtocol(IP_PROT_PIM);
    ctrl->setTimeToLive(1);
    ctrl->setInterfaceId(interfaceToRP->getInterfaceId());
    msg->setControlInfo(ctrl);

    send(msg, "spiltterOut");
}

void pimSM::sendPIMRegisterNull(IPv4Address multSource, IPv4Address multDest)
{
    // create PIM Register NULL datagram
    PIMRegister *msg = new PIMRegister();
    IPv4ControlInfo *dummyCtrl = new IPv4ControlInfo();
    IPv4Datagram *dummyDatagram = new IPv4Datagram();

    // only if (S,G exist)
    if (rt->getRouteFor(multDest,multSource))
    {
        msg->setName("PIMRegisterNull");
        msg->setType(Register);

        msg->setN(true);
        msg->setB(false);

        dummyCtrl->setDestAddr(multDest);
        dummyCtrl->setSrcAddr(multSource);
        dummyCtrl->setProtocol(IP_PROT_PIM);
        dummyDatagram->setSrcAddress(multSource);
        dummyDatagram->setDestAddress(multDest);
        dummyDatagram->setHeaderLength(5);
        dummyDatagram->setByteLength(20);
        dummyDatagram->setTimeToLive(255);
        dummyDatagram->setControlInfo(dummyCtrl);
        msg->setMultDatagram(*dummyDatagram);    // "encapsulate" multicast datagram to PIM-Register message

        // set IP Control info
        InterfaceEntry *interfaceToRP = rt->getInterfaceForDestAddr(RPAddress);
        IPv4ControlInfo *ctrl = new IPv4ControlInfo();
        ctrl->setDestAddr(RPAddress);
        ctrl->setProtocol(IP_PROT_PIM);
        ctrl->setSrcAddr(interfaceToRP->ipv4Data()->getIPAddress());
        ctrl->setInterfaceId(interfaceToRP->getInterfaceId());
        //TODO check wich TTL value should be there
        ctrl->setTimeToLive(255);
        msg->setControlInfo(ctrl);

        send(msg, "spiltterOut");
    }
}

void pimSM::sendPIMRegister(IPv4Datagram *datagram)
{
    IPv4ControlInfo *multDatactrl = (IPv4ControlInfo *) datagram->getControlInfo();
    IPv4Address multDataDest = multDatactrl->getDestAddr();
    IPv4Address multDataSource = multDatactrl->getSrcAddr();

    AnsaIPv4MulticastRoute *routeSG = new AnsaIPv4MulticastRoute();
    AnsaIPv4MulticastRoute *routeG = new AnsaIPv4MulticastRoute();
    InterfaceEntry *intToRP = rt->getInterfaceForDestAddr(this->getRPAddress());

    routeSG = rt->getRouteFor(multDataDest,multDataSource);
    routeG = rt->getRouteFor(multDataDest, IPv4Address::UNSPECIFIED_ADDRESS);
    if (routeSG == NULL)
        throw cRuntimeError("pimSM::sendPIMRegister - route for (S,G) not found!");

    // refresh KAT timers
    if (routeSG->getKat())
    {
        EV << " (S,G) KAT timer refresh" << endl;
        cancelEvent(routeSG->getKat());
        scheduleAt(simTime() + KAT, routeSG->getKat());
    }
    if (routeG->getKat())
    {
        EV << " (*,G) KAT timer refresh" << endl;
        cancelEvent(routeG->getKat());
        scheduleAt(simTime() + 2*KAT, routeG->getKat());
    }

    // Check if is register tunnel connected
    if (routeSG->getRegStatus(intToRP->getInterfaceId()) == Join)
    {
        // create PIM Register datagram
        PIMRegister *msg = new PIMRegister();

        msg->setName("PIMRegister");
        msg->setType(Register);

        msg->setN(false);
        msg->setB(false);
        msg->setMultDatagram(*datagram);    // "encapsulate" multicast datagram to PIM-Register message

        // set IP Control info
        InterfaceEntry *interfaceToRP = rt->getInterfaceForDestAddr(RPAddress);
        IPv4ControlInfo *ctrl = new IPv4ControlInfo();
        ctrl->setDestAddr(RPAddress);
        ctrl->setProtocol(IP_PROT_PIM);
        ctrl->setSrcAddr(interfaceToRP->ipv4Data()->getIPAddress());
        ctrl->setInterfaceId(interfaceToRP->getInterfaceId());
        //TODO check wich TTL value should be there
        ctrl->setTimeToLive(255);
        msg->setControlInfo(ctrl);

        send(msg, "spiltterOut");
    }
    else if (routeSG->getRegStatus(intToRP->getInterfaceId()) == Prune)
        EV << "PIM-SM:sendPIMRegister - register tunnel is disconnect." << endl;
}

void pimSM::sendPIMRegisterStop(IPv4Address source, IPv4Address dest, IPv4Address multGroup, IPv4Address multSource)
{
    // create PIM Register datagram
    PIMRegisterStop *msg = new PIMRegisterStop();

    // set PIM packet
    msg->setName("PIMRegisterStop");
    msg->setType(RegisterStop);
    msg->setSourceAddress(multSource);
    msg->setGroupAddress(multGroup);

    // set IP packet
    InterfaceEntry *interfaceToDR = rt->getInterfaceForDestAddr(dest);
    IPv4ControlInfo *ctrl = new IPv4ControlInfo();
    ctrl->setDestAddr(dest);
    ctrl->setSrcAddr(source);
    ctrl->setProtocol(IP_PROT_PIM);
    ctrl->setInterfaceId(interfaceToDR->getInterfaceId());
    //TODO check wich TTL value should be there
    ctrl->setTimeToLive(255);
    msg->setControlInfo(ctrl);

    send(msg, "spiltterOut");
}


/**
 * NEW MULTICAST
 *
 * The method process notification about new multicast data stream.
 *
 * @param newRoute Pointer to new entry in the multicast routing table.
 * @see s
 */
void pimSM::newMulticastRegisterDR(AnsaIPv4MulticastRoute *newRoute)
{
    EV << "pimSM::newMulticast" << endl;

    AnsaIPv4MulticastRoute *newRouteG = new AnsaIPv4MulticastRoute();

    // Set Keep Alive timer for routes
    PIMkat* timerKat = createKeepAliveTimer(newRoute->getOrigin(), newRoute->getMulticastGroup());
    PIMkat* timerKatG = createKeepAliveTimer(IPv4Address::UNSPECIFIED_ADDRESS, newRoute->getMulticastGroup());
    newRoute->setKat(timerKat);
    newRouteG->setKat(timerKatG);

    //TODO Could register?

    //Create (*,G) state
    InterfaceEntry *newInIntG = rt->getInterfaceForDestAddr(this->getRPAddress());

    newRouteG->setInInt(newInIntG, newInIntG->getInterfaceId(), this->getRPAddress());
    newRouteG->setMulticastGroup(newRoute->getMulticastGroup());
    newRouteG->setOrigin(IPv4Address::UNSPECIFIED_ADDRESS);
    newRouteG->setRP(this->getRPAddress());
    newRouteG->addFlag(S);
    newRouteG->addFlag(P);
    newRouteG->addFlag(F);

    //Create (S,G) state - set flags and Register state, other is set by  PimSplitter
    newRoute->addFlag(P);
    newRoute->addFlag(F);
    newRoute->addFlag(T);

    // create new outgoing interface to RP and set register state
    outInterface newOutInt;
    newOutInt.intId = newInIntG->getInterfaceId();
    newOutInt.intPtr = newInIntG;
    newOutInt.regState = Join;
    newOutInt.mode = Sparsemode;
    newOutInt.forwarding = Pruned;
    newOutInt.pruneTimer = NULL;
    newRoute->setOutShowIntStatus(false);                   //we need to set register state to output interface, but output interface has to be null for now
    newRoute->addOutInt(newOutInt);

    rt->addMulticastRoute(newRouteG);
    rt->addMulticastRoute(newRoute);

    EV << "pimSM::newMulticast: New routes was added to the multicast routing table." << endl;
}

void pimSM::newMulticastReciever(IPv4ControlInfo *igmpCtrl)
{
    AnsaIPv4MulticastRoute *newRouteG = new AnsaIPv4MulticastRoute();
    AnsaIPv4MulticastRoute *routePointer;
    IPv4Address multGroup = igmpCtrl->getDestAddr();
    InterfaceEntry *newInIntG = rt->getInterfaceForDestAddr(this->RPAddress);
    PimNeighbor *neighborToRP = pimNbt->getNeighborByIntID(newInIntG->getInterfaceId());

    // create new (*,G) route
    routePointer = newRouteG;
    if (!(newRouteG = rt->getRouteFor(multGroup,IPv4Address::UNSPECIFIED_ADDRESS)))
    {
        newRouteG = routePointer;
        // set source, mult. group, etc...
        newRouteG->setOrigin(IPv4Address::UNSPECIFIED_ADDRESS);
        newRouteG->setMulticastGroup(multGroup);
        newRouteG->setRP(this->getRPAddress());
        newRouteG->addFlag(S);
        newRouteG->addFlag(C);

        // set incoming interface
        newRouteG->setInInt(newInIntG, newInIntG->getInterfaceId(), neighborToRP->getAddr());

        // set outgoing interface to RP
        InterfaceEntry *interface = ift->getInterfaceById(igmpCtrl->getInterfaceId());
        outInterface newOutInt;
        newOutInt.intId = interface->getInterfaceId();
        newOutInt.intPtr = interface;
        newOutInt.forwarding = Forward;
        newOutInt.mode = Sparsemode;
        newOutInt.pruneTimer = NULL;
        newRouteG->addOutInt(newOutInt);

        // create and set (*,G) ET timer
        PIMet* timerEt = createExpiryTimer(HOLDTIME,multGroup);
        PIMjt* timerJt = createJoinTimer(multGroup, this->getRPAddress(), neighborToRP->getAddr());
        newRouteG->setEt(timerEt);
        newRouteG->setJt(timerJt);

        rt->addMulticastRoute(newRouteG);

        // oilist != NULL -> send Join(*,G) to 224.0.0.13
        sendPIMJoin(multGroup,this->getRPAddress(),neighborToRP->getAddr());
    }

    // restart ET timer
    if (newRouteG->getEt())
    {
        EV << " (*,G) ET timer refresh" << endl;
        cancelEvent(newRouteG->getEt());
        scheduleAt(simTime() + HOLDTIME, newRouteG->getEt());
    }


}

/**
 * RECEIVE CHANGE NOTIFICATION
 *
 * The method from class Notification Board is used to catch its events.
 *
 * @param category Category of notification.
 * @param details Additional information for notification.
 * @see newMulticast()
 * @see newMulticastAddr()
 */
void pimSM::receiveChangeNotification(int category, const cPolymorphic *details)
{
    // ignore notifications during initialize
    if (simulation.getContextType()==CTX_INITIALIZE)
        return;

    // PIM needs addition info for each notification
    if (details == NULL)
        return;

    Enter_Method_Silent();
    printNotificationBanner(category, details);
    AnsaIPv4MulticastRoute *route;
    IPv4ControlInfo *myCtrl;

    // according to category of event...
    switch (category)
    {
        case NF_IPv4_NEW_IGMP:
            EV <<  "pimSM::receiveChangeNotification - NEW IGMP" << endl;
            myCtrl = (IPv4ControlInfo *)(details);
            newMulticastReciever(myCtrl);
            break;

        // new multicast data appears in router
        case NF_IPv4_NEW_MULTICAST_SPARSE:
            EV <<  "pimSM::receiveChangeNotification - NEW MULTICAST SPARSE" << endl;
            route = (AnsaIPv4MulticastRoute *)(details);
            newMulticastRegisterDR(route);
            break;

        // create PIM register packet
        case NF_IPv4_MDATA_REGISTER:
            EV <<  "pimSM::receiveChangeNotification - REGISTER DATA" << endl;
            IPv4Datagram *datagram;
            datagram = (IPv4Datagram *)(details);
            sendPIMRegister(datagram);
            break;
    }
}
