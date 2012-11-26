/**
 * @file pimDM.cc
 * @date 29.10.2011
 * @author: Veronika Rybova
 * @brief File implements PIM dense mode.
 * @details Implementation according to RFC3973.
 */

#include "pimDM.h"


Define_Module(pimDM);

using namespace std;

/**
 * SEND PIM JOIN PRUNE
 *
 * The method is used to create PIMJoinPrune Packet and send it to next hop router.
 *
 * @param nextHop IP Address of receiver.
 * @param src IP address of multicast source.
 * @param grp IP address of multicast group.
 * @param intId ID of outgoing interface.
 * @see PIMJoinPrune()
 */
void pimDM::sendPimJoinPrune(IPAddress nextHop, IPAddress src, IPAddress grp, int intId)
{
	EV << "pimDM::sendPimJoinPrune" << endl;
	EV << "UpstreamNeighborAddress: " << nextHop << ", Source: " << src << ", Group: " << grp << ", IntId: " << intId << endl;

	PIMJoinPrune *msg = new PIMJoinPrune();
	msg->setName("PIMJoinPrune");
	msg->setUpstreamNeighborAddress(nextHop);
	msg->setHoldTime(PT);
	msg->setMulticastGroupsArraySize(1);

	//FIXME change to add also join groups
	// we do not need it at this time

	// set multicast groups
	MulticastGroup *group = new MulticastGroup();
	group->setGroupAddress(grp);
	group->setJoinedSourceAddressArraySize(0);
	group->setPrunedSourceAddressArraySize(1);
	group->setPrunedSourceAddress(0, src);
	msg->setMulticastGroups(0, *group);

	// set IP Control info
	IPControlInfo *ctrl = new IPControlInfo();
	IPAddress ga1("224.0.0.13");
	ctrl->setDestAddr(ga1);
	//ctrl->setProtocol(IP_PROT_PIM);
	ctrl->setProtocol(103);
	ctrl->setTimeToLive(1);
	ctrl->setInterfaceId(intId);
	msg->setControlInfo(ctrl);
	send(msg, "spiltterOut");
}

/**
 * SEND PIM GRAFT ACK
 *
 * The method is used to create PIMGraftAck packet and send it to next hop router.
 * PIMGraftAck pkt is copy of received PIMGraft pkt. Only type and IP control info
 * has to be changed.
 *
 * @param msg Pointer to PIMGraft packet.
 * @see PIMGraftAck()
 */
void pimDM::sendPimGraftAck(PIMGraftAck *msg)
{
	msg->setName("PIMGraftAck");
	msg->setType(GraftAck);

	// set IP Control info
	IPControlInfo *oldCtrl = (IPControlInfo*) (msg->removeControlInfo());
	IPControlInfo *ctrl = new IPControlInfo();
	ctrl->setDestAddr(oldCtrl->getSrcAddr());
	ctrl->setSrcAddr(oldCtrl->getDestAddr());
	ctrl->setProtocol(103);
	ctrl->setTimeToLive(1);
	ctrl->setInterfaceId(oldCtrl->getInterfaceId());
	delete oldCtrl;
	msg->setControlInfo(ctrl);
	send(msg, "spiltterOut");
}

/**
 * SEND PIM GRAFT
 *
 * The method is used to create PIMGraft packet and send it to next hop router.
 * Only  JoinedSource part of message is used, because Graft pkt is sent when
 * router wants to join to multicast tree again.
 *
 * @param nextHop IP Address of receiver.
 * @param src IP address of multicast source.
 * @param grp IP address of multicast group.
 * @param intId ID of outgoing interface.
 * @see PIMGraft()
 */
void pimDM::sendPimGraft(IPAddress nextHop, IPAddress src, IPAddress grp, int intId)
{
	EV << "pimDM::sendPimGraft" << endl;
	EV << "UpstreamNeighborAddress: " << nextHop << ", Source: " << src << ", Group: " << grp << ", IntId: " << intId << endl;

	PIMGraft *msg = new PIMGraft();
	msg->setName("PIMGraft");
	msg->setHoldTime(0);
	msg->setUpstreamNeighborAddress(nextHop);
	msg->setMulticastGroupsArraySize(1);

	// set multicast groups
	MulticastGroup *group = new MulticastGroup();
	group->setGroupAddress(grp);
	group->setJoinedSourceAddressArraySize(1);
	group->setPrunedSourceAddressArraySize(0);
	group->setJoinedSourceAddress(0, src);
	msg->setMulticastGroups(0, *group);

	// set IP Control info
	IPControlInfo *ctrl = new IPControlInfo();
	ctrl->setDestAddr(nextHop);
	//ctrl->setProtocol(IP_PROT_PIM);
	ctrl->setProtocol(103);
	ctrl->setTimeToLive(1);
	ctrl->setInterfaceId(intId);
	msg->setControlInfo(ctrl);
	send(msg, "spiltterOut");
}

/**
 * SEND PIM STATE REFRESH
 *
 * The method is used to create PIMStateRefresh packet and send it to next hop router.
 * By using the message we do not need to flood the network every 3 minutes.
 *
 * @param originator IP Address of source router.
 * @param src IP address of multicast source.
 * @param grp IP address of multicast group.
 * @param intId ID of outgoing interface.
 * @param P Indicator of pruned outgoing interface. If interface is pruned it is set to 1, otherwise to 0.
 * @see PIMStateRefresh()
 */
void pimDM::sendPimStateRefresh(IPAddress originator, IPAddress src, IPAddress grp, int intId, bool P)
{
	EV << "pimDM::sendPimStateRefresh" << endl;

	PIMStateRefresh *msg = new PIMStateRefresh();
	msg->setName("PIMStateRefresh");
	msg->setGroupAddress(grp);
	msg->setSourceAddress(src);
	msg->setOriginatorAddress(originator);
	msg->setInterval(SRT);
	msg->setP(P);

	// set IP Control info
	IPControlInfo *ctrl = new IPControlInfo();
	ctrl->setDestAddr(grp);
	//ctrl->setProtocol(IP_PROT_PIM);
	ctrl->setProtocol(103);
	ctrl->setTimeToLive(1);
	ctrl->setInterfaceId(intId);
	msg->setControlInfo(ctrl);
	send(msg, "spiltterOut");
}

/**
 * CREATE PRUNE TIMER
 *
 * The method is used to create PIMPrune timer. The timer is set when outgoing interface
 * goes to pruned state. After expiration (usually 3min) interface goes back to forwarding
 * state. It is set to (S,G,I).
 *
 * @param source IP address of multicast source.
 * @param group IP address of multicast group.
 * @param intId ID of outgoing interface.
 * @param holdTime Time of expiration (usually 3min).
 * @return Pointer to new Prune Timer.
 * @see PIMpt()
 */
PIMpt* pimDM::createPruneTimer(IPAddress source, IPAddress group, int intId, int holdTime)
{
	EV << "pimDM::createPruneTimer" << endl;
	PIMpt *timer = new PIMpt();
	timer->setName("PimPruneTimer");
	timer->setSource(source);
	timer->setGroup(group);
	timer->setIntId(intId);
	scheduleAt(simTime() + holdTime, timer);
	return timer;
}

/**
 * CREATE GRAFT RETRY TIMER
 *
 * The method is used to create PIMGraftRetry timer. The timer is set when router wants to join to
 * multicast tree again and send PIM Prune message to upstream. Router waits for Graft Retry Timer
 * (3 s) for PIM PruneAck message from upstream. If timer expires, router will send PIM Prune message
 * again. It is set to (S,G).
 *
 * @param source IP address of multicast source.
 * @param group IP address of multicast group.
 * @return Pointer to new Graft Retry Timer.
 * @see PIMgrt()
 */
PIMgrt* pimDM::createGraftRetryTimer(IPAddress source, IPAddress group)
{
	EV << "pimDM::createPruneTimer" << endl;
	PIMgrt *timer = new PIMgrt();
	timer->setName("PIMGraftRetryTimer");
	timer->setSource(source);
	timer->setGroup(group);
	scheduleAt(simTime() + GRT, timer);
	return timer;
}

/**
 * CREATE SOURCE ACTIVE TIMER
 *
 * The method is used to create PIMSourceActive timer. The timer is set when source of multicast is
 * connected directly to the router.  If timer expires, router will remove the route from multicast
 * routing table. It is set to (S,G).
 *
 * @param source IP address of multicast source.
 * @param group IP address of multicast group.
 * @return Pointer to new Source Active Timer
 * @see PIMsat()
 */
PIMsat* pimDM::createSourceActiveTimer(IPAddress source, IPAddress group)
{
	EV << "pimDM::createSourceActiveTimer" << endl;
	PIMsat *timer = new PIMsat();
	timer->setName("PIMSourceActiveTimer");
	timer->setSource(source);
	timer->setGroup(group);
	scheduleAt(simTime() + SAT, timer);
	return timer;
}

/**
 * CREATE STATE REFRESH TIMER
 *
 * The method is used to create PIMStateRefresh timer. The timer is set when source of multicast is
 * connected directly to the router. If timer expires, router will send StateRefresh message, which
 * will propagate through all network and wil reset Prune Timer. It is set to (S,G).
 *
 * @param source IP address of multicast source.
 * @param group IP address of multicast group.
 * @return Pointer to new Source Active Timer
 * @see PIMsrt()
 */
PIMsrt* pimDM::createStateRefreshTimer(IPAddress source, IPAddress group)
{
	EV << "pimDM::createStateRefreshTimer" << endl;
	PIMsrt *timer = new PIMsrt();
	timer->setName("PIMStateRefreshTimer");
	timer->setSource(source);
	timer->setGroup(group);
	scheduleAt(simTime() + SRT, timer);
	return timer;
}


/**
 * PROCESS GRAFT PACKET
 *
 * The method is used to process PIMGraft packet. Packet means that downstream router wants to join to
 * multicast tree, so the packet cannot come to RPF interface. Router finds correct outgoig interface
 * towards downstream router. Change its state to forward if it was not before and cancel Prune Timer.
 * If route was in pruned state, router will send also Graft message to join multicast tree.
 *
 * @param source IP address of multicast source.
 * @param group IP address of multicast group.
 * @param sender IP Address of Graft packet sender.
 * @param intId ID of Graft packet incoming interface.
 * @see sendPimGraft()
 * @see createGraftRetryTimer()
 * @see PIMGraft()
 * @see PIMgrt()
 * @see processJoinPruneGraftPacket()
 */
void pimDM::processGraftPacket(IPAddress source, IPAddress group, IPAddress sender, int intId)
{
	EV << "pimDM::processGraftPacket" << endl;

	MulticastIPRoute *route = mrt->getRouteFor(group, source);
	bool forward = false;

	// check if message come to non-RPF interface
	if (route->isRpf(intId))
	{
		EV << "ERROR: Graft message came to RPF interface." << endl;
		return;
	}

	// find outgoing interface to neighbor
	InterfaceVector outInt = route->getOutInt();
	for (unsigned int l = 0; l < outInt.size(); l++)
	{
		if(outInt[l].intId == intId)
		{
			forward = true;
			if (outInt[l].forwarding == Pruned)
			{
				EV << "Interface " << outInt[l].intId << " transit to forwarding state (Graft)." << endl;
				outInt[l].forwarding = Forward;

				//cancel Prune Timer
				PIMpt* timer = outInt[l].pruneTimer;
				cancelEvent(timer);
				delete timer;
				outInt[l].pruneTimer = NULL;
			}
		}
	}
	route->setOutInt(outInt);

	// if all route was pruned, remove prune flag
	// if upstrem is not source, send Graft message
	if (route->isFlagSet(P) && forward && (route->getGrt() == NULL))
	{
		if (!route->isFlagSet(A))
		{
			EV << "Route is not pruned any more, send Graft to upstream" << endl;
			sendPimGraft(route->getInIntNextHop(), source, group, route->getInIntId());
			PIMgrt* timer = createGraftRetryTimer(source, group);
			route->setGrt(timer);
		}
		else
			route->removeFlag(P);
	}
}

/**
 * PROCESS GRAFT ACK PACKET
 *
 * The method is used to process PIMGraftAck packet. Packet means that the router was successfully joined
 * to multicast tree. Route is now in forwarding state and Graft Retry timer was canceled.
 *
 * @param route Pointer to multicast route which GraftAck belongs to.
 * @see processJoinPruneGraftPacket()
 * @see PIMgrt()
 */
void pimDM::processGraftAckPacket(MulticastIPRoute *route)
{
	EV << "pimDM::processGraftAckPacket" << endl;
	PIMgrt *grt = route->getGrt();
	if (grt != NULL)
	{
		cancelEvent(grt);
		delete grt;
		route->setGrt(NULL);
		route->removeFlag(P);
	}
}

/**
 * PROCESS PRUNE PACKET
 *
 * The method process PIM Prune packet. First the method has to find correct outgoing interface
 * where PIM Prune packet came to. The method also checks if there is still any forwarding outgoing
 * interface. Forwarding interfaces, where Prune packet come to, goes to prune state. If all outgoing
 * interfaces are pruned, the router will prune from multicast tree.
 *
 * @param route Pointer to multicast route which GraftAck belongs to.
 * @param intId
 * @see processJoinPruneGraftPacket()
 * @see createPruneTimer()
 * @see sendPimJoinPrune()
 * @see PIMpt()
 */
void pimDM::processPrunePacket(MulticastIPRoute *route, int intId, int holdTime)
{
	EV << "pimDM::processPrunePacket" << endl;
	InterfaceVector outInt = route->getOutInt();
	int i = route->getOutIdByIntId(intId);
	bool change = false;

	// we find correct outgoing interface
	if (i < (int) outInt.size())
	{
		// if interface was already pruned, restart Prune Timer
		if (outInt[i].forwarding == Pruned)
		{
			EV << "Outgoing interface is already pruned, restart Prune Timer." << endl;
			PIMpt* timer = outInt[i].pruneTimer;
			cancelEvent(timer);
			scheduleAt(simTime() + holdTime, timer);
		}
		// if interface is forwarding, transit its state to pruned and set Prune timer
		else
		{
			EV << "Outgoing interfaces is forwarding now -> change to Pruned, set the timer." << endl;
			outInt[i].forwarding = Pruned;
			PIMpt* timer = createPruneTimer(route->getSource(), route->getGroup(), intId, holdTime);
			outInt[i].pruneTimer = timer;
			change = true;
		}
	}
	route->setOutInt(outInt);

	// if there is no forwarding outgoing int, transit route to pruned state
	if (route->isOilistNull() && change)
	{
		EV << "All interfaces are pruned, send Pruned to upstream." << endl;
		route->addFlag(P);

		// if GRT is running now, do not send Prune msg
		if (route->isFlagSet(P) && (route->getGrt() != NULL))
		{
			cancelEvent(route->getGrt());
			delete route->getGrt();
			route->setGrt(NULL);
		}
		else if (!route->isFlagSet(A))
			sendPimJoinPrune(route->getInIntNextHop(), route->getSource(), route->getGroup(), route->getInIntId());
	}
}


/**
 * PROCESS JOIN PRUNE GRAFT PACKET
 *
 * The method is used to process PIM JoinePrune, PIMGraft, and PIMGraftAck packet. All these packets have
 * the same structure. If packet is for this router the method goes through all multicast groups in message.
 * There could be list of joined and pruned sources for each multicast group. Joine Prune message can contains
 * both. Graft and Graft Ack can contain only join list. According to type of packet, appropriate method is
 * called to process info.
 *
 * @param pkt Pointer to packet.
 * @param type Type of packet.
 * @see processGraftPacket()
 * @see processGraftAckPacket()
 * @see processPrunePacket()
 * @see sendPimGraftAck()
 */
void pimDM::processJoinPruneGraftPacket(PIMJoinPrune *pkt, PIMPacketType type)
{
	EV << "pimDM::processJoinePruneGraftPacket" << endl;

	IPControlInfo *ctrl =  (IPControlInfo *) pkt->getControlInfo();
	IPAddress sender = ctrl->getSrcAddr();
	InterfaceEntry * nt = rt->getInterfaceForDestAddr(sender);
	vector<PimNeighbor> neighbors = pimNbt->getNeighborsByIntID(nt->getInterfaceId());
	IPAddress addr = nt->ipv4Data()->getIPAddress();

	// does packet belong to this router?
	if (pkt->getUpstreamNeighborAddress() != nt->ipv4Data()->getIPAddress() && type != GraftAck)
	{
		EV << "Paket neni urceny pro tento router" << endl;
		delete pkt;
		return;
	}

	// go through list of multicast groups
	for (unsigned int i = 0; i < pkt->getMulticastGroupsArraySize(); i++)
	{
		MulticastGroup group = pkt->getMulticastGroups(i);
		IPAddress groupAddr = group.getGroupAddress();

		// go through list of joined sources
		//EV << "JoinedSourceAddressArraySize: " << group.getJoinedSourceAddressArraySize() << endl;
		for (unsigned int j = 0; j < group.getJoinedSourceAddressArraySize(); j++)
		{
			IPAddress source = group.getJoinedSourceAddress(j);
			MulticastIPRoute *route = mrt->getRouteFor(groupAddr, source);

			if (type == JoinPrune)
			{
				//FIXME join action
				// only if there is more than one PIM neighbor on one interface
				// interface change to forwarding state
				// cancel Prune Timer
				// send Graft to upstream
			}
			else if (type == Graft)
				processGraftPacket(source, groupAddr, sender, nt->getInterfaceId());
			else if (type == GraftAck)
				processGraftAckPacket(route);
		}

		// go through list of pruned sources (only for PIM Join Prune msg)
		if (type == JoinPrune)
		{
			//EV << "JoinedPrunedAddressArraySize: " << group.getPrunedSourceAddressArraySize() << endl;
			for(unsigned int k = 0; k < group.getPrunedSourceAddressArraySize(); k++)
			{
				IPAddress source = group.getPrunedSourceAddress(k);
				MulticastIPRoute *route = mrt->getRouteFor(groupAddr, source);

				if (source != route->getSource())
					continue;

				// if there could be more than one PIM neighbor on interface
				if (neighbors.size() > 1)
				{
					EV << "Vice sousedu na rozhrani" << endl;
					; //FIXME set PPT timer
				}
				// if there is only one PIM neighbor on interface
				else
					processPrunePacket(route, nt->getInterfaceId(), pkt->getHoldTime());
			}
		}
	}

	// Send GraftAck for this Graft message
	if (type == Graft)
		sendPimGraftAck((PIMGraftAck *) (pkt));
	mrt->generateShowIPMroute();
}

/**
 * PROCESS STATE REFRESH PACKET
 *
 * The method is used to process PIMStateRefresh packet. The method checks if there is route in mroute
 * and that packet has came to RPF interface. Then it goes through all outgoing interfaces. If the
 * interface is pruned, it resets Prune Timer. For each interface State Refresh message is copied and
 * correct prune indicator is set according to state of outgoing interface (pruned/forwarding).
 *
 * State Refresh message is used to stop flooding of network each 3 minutes.
 *
 * @param pkt Pointer to packet.
 * @see sendPimJoinPrune()
 * @see sendPimStateRefresh()
 */
void pimDM::processStateRefreshPacket(PIMStateRefresh *pkt)
{
	EV << "pimDM::processStateRefreshPacket" << endl;

	// FIXME actions of upstream automat according to pruned/forwarding state and Prune Indicator from msg

	// first check if there is route for given group address and source
	MulticastIPRoute *route = mrt->getRouteFor(pkt->getGroupAddress(), pkt->getSourceAddress());
	if (route == NULL)
	{
		delete pkt;
		return;
	}
	InterfaceVector outInt = route->getOutInt();
	bool pruneIndicator;

	// chceck if State Refresh msg has came to RPF interface
	IPControlInfo *ctrl = (IPControlInfo*) pkt->getControlInfo();
	if (ctrl->getInterfaceId() != route->getInIntId())
	{
		delete pkt;
		return;
	}

	// this router is pruned, but outgoing int of upstream router leading to this router is forwarding
	if (route->isFlagSet(P) && !pkt->getP())
	{
		// send Prune msg to upstream
		if (route->getGrt() == NULL)
			sendPimJoinPrune(route->getInIntNextHop(), route->getSource(), route->getGroup(), route->getInIntId());
		else
		{
			cancelEvent(route->getGrt());
			delete route->getGrt();
			route->setGrt(NULL);
		}
	}

	// go through all outgoing interfaces, reser Prune Timer and send out State Refresh msg
	for (unsigned int i = 0; i < outInt.size(); i++)
	{
		if (outInt[i].forwarding == Pruned)
		{
			// P = true
			pruneIndicator = true;
			// reset PT
			cancelEvent(outInt[i].pruneTimer);
			scheduleAt(simTime() + PT, outInt[i].pruneTimer);
		}
		else if (outInt[i].forwarding == Forward)
		{
			// P = false
			pruneIndicator = false;
		}
		sendPimStateRefresh(pkt->getOriginatorAddress(), pkt->getSourceAddress(), pkt->getGroupAddress(), outInt[i].intId, pruneIndicator);
	}
	delete pkt;
}


/**
 * PROCESS PRUNE TIMER
 *
 * The method is used to process PIM Prune timer. It is (S,G,I) timer. When Prune timer expires, it
 * means that outgoing interface transits back to forwarding state. If the router is pruned from
 * multicast tree, join again.
 *
 * @param timer Pointer to Prune timer.
 * @see PIMpt()
 * @see sendPimGraft()
 * @see createGraftRetryTimer()
 */
void pimDM::processPruneTimer(PIMpt *timer)
{
	EV << "pimDM::processPruneTimer" << endl;

	IPAddress source = timer->getSource();
	IPAddress group = timer->getGroup();
	int intId = timer->getIntId();

	// find correct (S,G) route which timer belongs to
	MulticastIPRoute *route = mrt->getRouteFor(group, source);
	if (route == NULL)
	{
		delete timer;
		return;
	}

	// state of interface is changed to forwarding
	int i = route->getOutIdByIntId(intId);
	InterfaceVector outInt = route->getOutInt();
	if (i < (int) outInt.size())
	{
		EV << "Nalezen out int" << endl;
		delete timer;
		outInt[i].pruneTimer = NULL;
		outInt[i].forwarding = Forward;
		route->setOutInt(outInt);

		// if the router is pruned from multicast tree, join again
		if (route->isFlagSet(P) && (route->getGrt() == NULL))
		{
			if (!route->isFlagSet(A))
			{
				EV << "Pruned cesta prejde do forwardu, posli Graft" << endl;
				sendPimGraft(route->getInIntNextHop(), source, group, route->getInIntId());
				PIMgrt* timer = createGraftRetryTimer(source, group);
				route->setGrt(timer);
			}
			else
				route->removeFlag(P);
		}
		mrt->generateShowIPMroute();
	}
}

/**
 * PROCESS GRAFT RETRY TIMER
 *
 * The method is used to process PIM Graft Retry Timer. It is (S,G) timer. When Graft Retry timer expires,
 * it means that the router did'n get GraftAck message from upstream router. The router has to send Prune
 * packet again.
 *
 * @param timer Pointer to Graft Retry timer.
 * @see PIMgrt()
 * @see sendPimGraft()
 * @see createGraftRetryTimer()
 */
void pimDM::processGraftRetryTimer(PIMgrt *timer)
{
	EV << "pimDM::processGraftRetryTimer" << endl;
	MulticastIPRoute *route = mrt->getRouteFor(timer->getGroup(), timer->getSource());
	sendPimGraft(route->getInIntNextHop(), timer->getSource(), timer->getGroup(), route->getInIntId());
	timer = createGraftRetryTimer(timer->getSource(), timer->getGroup());
}

/**
 * PROCESS GRAFT RETRY TIMER
 *
 * The method is used to process PIM Source Active Timer. It is (S,G) timer. When Source Active Timer expires,
 * route is removed from multicast routing table.
 *
 * @param timer Pointer to Source Active Timer.
 * @see PIMsat()
 */
void pimDM::processSourceActiveTimer(PIMsat * timer)
{
	EV << "pimDM::processSourceActiveTimer: route will be deleted" << endl;
	MulticastIPRoute *route = mrt->getRouteFor(timer->getGroup(), timer->getSource());

	delete timer;
	route->setSat(NULL);
	mrt->deleteRoute(route);
}

/**
 * PROCESS STATE REFRESH TIMER
 *
 * The method is used to process PIM State Refresh Timer. It is (S,G) timer and it is used only on router
 * whoch is connected directly to the source of multicast. When State Refresh Timer expires, the State Refresh
 * messages are sent to all outgoing interfaces. Prune indicator in each msg is set according to state of the
 * outgoing interface. State Refresh Timer is then reset.
 *
 * @param timer Pointer to Source Active Timer.
 * @see PIMsrt()
 * @see sendPimStateRefresh()
 * @see createStateRefreshTimer()
 */
void pimDM::processStateRefreshTimer(PIMsrt * timer)
{
	EV << "pimDM::processStateRefreshTimer" << endl;
	MulticastIPRoute *route = mrt->getRouteFor(timer->getGroup(), timer->getSource());
	InterfaceVector outInt = route->getOutInt();
	bool pruneIndicator;

	for (unsigned int i = 0; i < outInt.size(); i++)
	{
		if (outInt[i].forwarding == Pruned)
		{
			// P = true
			pruneIndicator = true;
			// reset PT
			cancelEvent(outInt[i].pruneTimer);
			scheduleAt(simTime() + PT, outInt[i].pruneTimer);
		}
		else if (outInt[i].forwarding == Forward)
		{
			pruneIndicator = false;
		}
		int intId = outInt[i].intId;
		sendPimStateRefresh(ift->getInterfaceById(intId)->ipv4Data()->getIPAddress(), timer->getSource(), timer->getGroup(), intId, pruneIndicator);
	}
	delete timer;
	route->setSrt(createStateRefreshTimer(route->getSource(), route->getGroup()));
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
void pimDM::processPIMTimer(PIMTimer *timer)
{
	EV << "pimDM::processPIMTimer: ";

	switch(timer->getTimerKind())
	{
		case AssertTimer:
			EV << "AssertTimer" << endl;
			break;
		case PruneTimer:
			EV << "PruneTimer" << endl;
			processPruneTimer(check_and_cast<PIMpt *> (timer));
			break;
		case PrunePendingTimer:
			EV << "PrunePendingTimer" << endl;
			break;
		case GraftRetryTimer:
			EV << "GraftRetryTimer" << endl;
			processGraftRetryTimer(check_and_cast<PIMgrt *> (timer));
			break;
		case UpstreamOverrideTimer:
			EV << "UpstreamOverrideTimer" << endl;
			break;
		case PruneLimitTimer:
			EV << "PruneLimitTimer" << endl;
			break;
		case SourceActiveTimer:
			EV << "SourceActiveTimer" << endl;
			processSourceActiveTimer(check_and_cast<PIMsat *> (timer));
			break;
		case StateRefreshTimer:
			EV << "StateRefreshTimer" << endl;
			processStateRefreshTimer(check_and_cast<PIMsrt *> (timer));
			break;
		default:
			EV << "BAD TYPE, DROPPED" << endl;
			delete timer;
	}
}

/**
 * PROCESS PIM PACKET
 *
 * The method is used to process PIM packets. According to type of PIM packet, the packet is sent to
 * appropriate method.
 *
 * @param pkt Pointer to PIM packet.
 * @see PIMPacket()
 * @see processJoinPruneGraftPacket()
 */
void pimDM::processPIMPkt(PIMPacket *pkt)
{
	EV << "pimDM::processPIMPkt: ";

	switch(pkt->getType())
	{
		case JoinPrune:
			EV << "JoinPrune" << endl;
			processJoinPruneGraftPacket(check_and_cast<PIMJoinPrune *> (pkt), (PIMPacketType) pkt->getType());
			break;
		case Assert:
			EV << "Assert" << endl;
			// FIXME for future use
			break;
		case Graft:
			EV << "Graft" << endl;
			processJoinPruneGraftPacket(check_and_cast<PIMJoinPrune *> (pkt), (PIMPacketType) pkt->getType());
			break;
		case GraftAck:
			EV << "GraftAck" << endl;
			processJoinPruneGraftPacket(check_and_cast<PIMJoinPrune *> (pkt), (PIMPacketType) pkt->getType());
			break;
		case StateRefresh:
			EV << "StateRefresh" << endl;
			processStateRefreshPacket(check_and_cast<PIMStateRefresh *> (pkt));
			break;
		default:
			EV << "BAD TYPE, DROPPED" << endl;
			delete pkt;
	}
}


/**
 * HANDLE MESSAGE
 *
 * The method is used to handle new messages. Self messages are timer and they are sent to
 * method which processes PIM timers. Other messages should be PIM packets, so they are sent
 * to method which processes PIM packets.
 *
 * @param msg Pointer to new message.
 * @see PIMPacket()
 * @see PIMTimer()
 * @see processPIMTimer()
 * @see processPIMPkt()
 */
void pimDM::handleMessage(cMessage *msg)
{
	EV << "PIMDM::handleMessage" << endl;

	// self message (timer)
   if (msg->isSelfMessage())
   {
	   EV << "PIMDM::handleMessage:Timer" << endl;
	   PIMTimer *timer = check_and_cast <PIMTimer *> (msg);
	   processPIMTimer(timer);
   }
   // PIM packet from PIM neighbor
   else if (dynamic_cast<PIMPacket *>(msg))
   {
	   EV << "PIMDM::handleMessage: PIM-DM packet" << endl;
	   PIMPacket *pkt = check_and_cast<PIMPacket *>(msg);
	   processPIMPkt(pkt);
   }
   // wrong message, mistake
   else
	   EV << "PIMDM::handleMessage: Wrong message" << endl;
}

/**
 * INITIALIZE
 *
 * The method initializes PIM-DM module. It get access to all needed tables and other objects.
 * It subscribes to important notifications. If there is no PIM interface, all module can be
 * disabled.
 *
 * @param stage Stage of initialization.
 */
void pimDM::initialize(int stage)
{
	if (stage == 4)
	{
		EV << "pimDM::initialize" << endl;

		// Pointer to routing tables, interface tables, notification board
		rt = RoutingTableAccess().get();
		mrt = MulticastRoutingTableAccess().get();
		ift = AnsaInterfaceTableAccess().get();
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
		nb->subscribe(this, NF_IPv4_NEW_MULTICAST_DENSE);
		nb->subscribe(this, NF_IPv4_NEW_IGMP_ADDED);
		nb->subscribe(this, NF_IPv4_NEW_IGMP_REMOVED);
		nb->subscribe(this, NF_IPv4_DATA_ON_PRUNED_INT);
		nb->subscribe(this, NF_IPv4_DATA_ON_NONRPF);
		nb->subscribe(this, NF_IPv4_DATA_ON_RPF);
		//nb->subscribe(this, NF_IPv4_RPF_CHANGE);
		nb->subscribe(this, NF_IPv4_ROUTE_ADDED);
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
void pimDM::receiveChangeNotification(int category, const cPolymorphic *details)
{
	// ignore notifications during initialize
	if (simulation.getContextType()==CTX_INITIALIZE)
		return;

	// PIM needs addition info for each notification
	if (details == NULL)
		return;

	Enter_Method_Silent();
	printNotificationBanner(category, details);
	IPControlInfo *ctrl;
	MulticastIPRoute *route;
	addRemoveAddr *members;

	// according to category of event...
	switch (category)
	{
		// new multicast data appears in router
		case NF_IPv4_NEW_MULTICAST_DENSE:
			EV <<  "pimDM::receiveChangeNotification - NEW MULTICAST DENSE-" << endl;
			route = (MulticastIPRoute *)(details);
			newMulticast(route);
			break;

		// configuration of interface changed, it means some change from IGMP, address were added.
		case NF_IPv4_NEW_IGMP_ADDED:
			EV << "pimDM::receiveChangeNotification - IGMP change - address were added." << endl;
			members = (addRemoveAddr *) (details);
			newMulticastAddr(members);
			break;

		// configuration of interface changed, it means some change from IGMP, address were removed.
		case NF_IPv4_NEW_IGMP_REMOVED:
			EV << "pimDM::receiveChangeNotification - IGMP change - address were removed." << endl;
			members = (addRemoveAddr *) (details);
			oldMulticastAddr(members);
			break;

		case NF_IPv4_DATA_ON_PRUNED_INT:
			EV << "pimDM::receiveChangeNotification - Data appears on pruned interface." << endl;
			ctrl = (IPControlInfo *)(details);
			dataOnPruned(ctrl->getDestAddr(), ctrl->getSrcAddr());
			break;

		// data come to non-RPF interface
		case NF_IPv4_DATA_ON_NONRPF:
			EV << "pimDM::receiveChangeNotification - Data appears on non-RPF interface." << endl;
			ctrl = (IPControlInfo *)(details);
			dataOnNonRpf(ctrl->getDestAddr(), ctrl->getSrcAddr(), ctrl->getInterfaceId());
			break;

		// data come to RPF interface
		case NF_IPv4_DATA_ON_RPF:
			EV << "pimDM::receiveChangeNotification - Data appears on RPF interface." << endl;
			route = (MulticastIPRoute *)(details);
			dataOnRpf(route);
			break;

		// RPF interface has changed
		case NF_IPv4_ROUTE_ADDED:
			EV << "pimDM::receiveChangeNotification - RPF interface has changed." << endl;
			IPRoute *entry = (IPRoute *) (details);
			vector<MulticastIPRoute*> routes = mrt->getRoutesForSource(entry->getHost());
			for (unsigned int i = 0; i < routes.size(); i++)
				rpfIntChange(routes[i]);
			break;
	}
}


/**
 * RPF INTERFACE CHANGE
 *
 * The method process notification about interface change. Multicast routing table will be
 * changed if RPF interface has changed. New RPF interface is set to route and is removed
 * from outgoing interfaces. On the other hand, old RPF interface is added to outgoing
 * interfaces. If route was not pruned, the router has to join to the multicast tree again
 * (by different path).
 *
 * @param newRoute Pointer to new entry in the multicast routing table.
 * @see sendPimGraft()
 * @see createGraftRetryTimer()
 */
void pimDM::rpfIntChange(MulticastIPRoute *route)
{
	IPAddress source = route->getSource();
	IPAddress group = route->getGroup();
	InterfaceEntry *newRpf = rt->getInterfaceForDestAddr(source);
	int rpfId = newRpf->getInterfaceId();

	// is there any change?
	if (rpfId == route->getInIntId())
		return;
	EV << "New RPF int for group " << group << " source " << source << " is " << rpfId << endl;

	// set new RPF
	inInterface oldRpf = route->getInInt();
	route->setInInt(newRpf, rpfId, pimNbt->getNeighborsByIntID(rpfId)[0].getAddr());

	// route was not pruned, join to the multicast tree again
	if (!route->isFlagSet(P))
	{
		sendPimGraft(route->getInIntNextHop(), source, group, rpfId);
		PIMgrt* timer = createGraftRetryTimer(source, group);
		route->setGrt(timer);
	}

	// find rpf int in outgoing imterfaces and delete it
	InterfaceVector outInt = route->getOutInt();
	for(unsigned int i = 0; i < outInt.size(); i++)
	{
		if (outInt[i].intId == rpfId)
		{
			if (outInt[i].pruneTimer != NULL)
			{
				cancelEvent(outInt[i].pruneTimer);
				delete outInt[i].pruneTimer;
				outInt[i].pruneTimer = NULL;
			}
			outInt.erase(outInt.begin() + i);
			break;
		}
	}

	// old RPF should be now outgoing interface if it is not down
	if (!oldRpf.intPtr->isDown())
	{
		outInterface newOutInt;
		newOutInt.intId = oldRpf.intId;
		newOutInt.intPtr = oldRpf.intPtr;
		newOutInt.pruneTimer = NULL;
		newOutInt.forwarding = Forward;
		newOutInt.mode = Densemode;
		outInt.push_back(newOutInt);
	}

	route->setOutInt(outInt);
	mrt->generateShowIPMroute();
}


/**
 * DATA ON RPF INTERFACE
 *
 * The method process notification about data which appears on RPF interface. It means that source
 * is still active. The resault is resetting of Source Active Timer.
 *
 * @param newRoute Pointer to new entry in the multicast routing table.
 * @see PIMsat()
 */
void pimDM::dataOnRpf(MulticastIPRoute *route)
{
	cancelEvent(route->getSat());
	scheduleAt(simTime() + SAT, route->getSat());
}

/**
 * DATA ON NON-RPF INTERFACE
 *
 * The method has to solve the problem when multicast data appears on non-RPF interface. It can
 * happen when there is loop in the network. In this case, router has to prune from the neighbor,
 * so it sends Prune message.
 *
 * @param group Multicast group IP address.
 * @param source Source IP address.
 * @param intID Identificator of incoming interface.
 * @see sendPimJoinPrune()
 * @see createPruneTimer()
 */
void pimDM::dataOnNonRpf(IPAddress group, IPAddress source, int intId)
{
	EV << "pimDM::dataOnNonRpf, intID: " << intId << endl;

	// load route from mroute
	MulticastIPRoute *route = mrt->getRouteFor(group, source);
	if (route == NULL)
		return;

	// in case of p2p link, send prune
	// FIXME There should be better indicator of P2P link
	if (pimNbt->getNumNeighborsOnInt(intId) == 1)
	{
		// send Prune msg to the neighbor who sent these multicast data
		IPAddress nextHop = (pimNbt->getNeighborsByIntID(intId))[0].getAddr();
		sendPimJoinPrune(nextHop, source, group, intId);

		// find incoming interface
		int i = route->getOutIdByIntId(intId);
		InterfaceVector outInt = route->getOutInt();

		// the incoming interface has to change its state to Pruned
		if (outInt[i].forwarding == Forward)
		{
			outInt[i].forwarding = Pruned;
			PIMpt* timer = createPruneTimer(route->getSource(), route->getGroup(), intId, PT);
			outInt[i].pruneTimer = timer;
			route->setOutInt(outInt);

			// if there is no outgoing interface, Prune msg has to be sent on upstream
			if (route->isOilistNull())
			{
				EV << "pimDM::dataOnNonRpf: oilist is NULL, send prune msg to upstream." << endl;
				route->addFlag(P);
				if (!route->isFlagSet(A))
					sendPimJoinPrune(route->getInIntNextHop(), route->getSource(), route->getGroup(), route->getInIntId());
			}
			mrt->generateShowIPMroute();
		}
	}

	//FIXME in case of LAN
}

/**
 * DATA ON PRUNED
 *
 * The method has to solve the problem when multicast data appears on RPF interface and
 * route is pruned. In this case, new PIM JoinPrune has to be sent to upstream.
 *
 * @param group Multicast group IP address.
 * @param source Source IP address.
 * @see sendPimJoinPrune()
 */
void pimDM::dataOnPruned(IPAddress group, IPAddress source)
{
	EV << "pimDM::dataOnPruned" << endl;
	MulticastIPRoute *route = mrt->getRouteFor(group, source);
	// if GRT is running now, do not send Prune msg
	if (route->isFlagSet(P) && (route->getGrt() != NULL))
	{
		cancelEvent(route->getGrt());
		delete route->getGrt();
		route->setGrt(NULL);
	}
	// otherwise send Prune msg to upstream router
	else if (!route->isFlagSet(A))
		sendPimJoinPrune(route->getInIntNextHop(), source, group, route->getInIntId());
}

/**
 * OLD MULTICAST ADDRESS
 *
 * The method process notification about multicast groups removed from interface. For each
 * old address it tries to find route. If there is route, it finds interface in list of outgoing
 * interfaces. If the interface is in the list it will be removed. If the router was not pruned
 * and there is no outgoing interface, the router will prune from the multicast tree.
 *
 * @param members Structure containing old multicast IP addresses.
 * @see sendPimJoinPrune()
 */
void pimDM::oldMulticastAddr(addRemoveAddr *members)
{
	EV << "pimDM::oldMulticastAddr" << endl;
	vector<IPAddress> oldAddr = members->getAddr();
	PimInterface * pimInt = members->getInt();
	bool connected = false;

	// go through all old multicast addresses assigned to interface
	for (unsigned int i = 0; i < oldAddr.size(); i++)
	{
		EV << "Removed multicast address: " << oldAddr[i] << endl;
		vector<MulticastIPRoute*> routes = mrt->getRouteFor(oldAddr[i]);

		// there is no route for group in the table
		if (routes.size() == 0)
			continue;

		// go through all multicast routes
		for (unsigned int j = 0; j < routes.size(); j++)
		{
			MulticastIPRoute *route = routes[j];
			InterfaceVector outInt = route->getOutInt();
			unsigned int k;

			// is interface in list of outgoing interfaces?
			for (k = 0; k < outInt.size(); k++)
			{
				if (outInt[k].intId == pimInt->getInterfaceID())
				{
					EV << "Interface is present, removing it from the list of outgoing interfaces." << endl;
					outInt.erase(outInt.begin() + k);
				}
				else if(outInt[k].forwarding == Forward)
				{
					if ((pimNbt->getNeighborsByIntID(outInt[k].intId)).size() == 0)
						connected = true;
				}
			}
			route->setOutInt(outInt);

			// if there is no directly connected member of group
			if (!connected)
				route->removeFlag(C);

			// there is no receiver of multicast, prune the router from the multicast tree
			if (route->isOilistNull())
			{
				EV << "There is no receiver for the group -> prune from the tree" << endl;
				// if GRT is running now, do not send Prune msg
				if (route->isFlagSet(P) && (route->getGrt() != NULL))
				{
					cancelEvent(route->getGrt());
					delete route->getGrt();
					route->setGrt(NULL);
					sendPimJoinPrune(route->getInIntNextHop(), route->getSource(), route->getGroup(), route->getInIntId());
				}

				// if the source is not directly connected, sent Prune msg
				if (!route->isFlagSet(A) && !route->isFlagSet(P))
					sendPimJoinPrune(route->getInIntNextHop(), route->getSource(), route->getGroup(), route->getInIntId());

				route->addFlag(P);
			}
		}
	}
	mrt->generateShowIPMroute();
}

/**
 * NEW MULTICAST ADDRESS
 *
 * The method process notification about new multicast groups aasigned to interface. For each
 * new address it tries to find route. If there is route, it finds interface in list of outgoing
 * interfaces. If the interface is not in the list it will be added. if the router was pruned
 * from multicast tree, join again.
 *
 * @param members Structure containing new multicast IP addresses.
 * @see sendPimGraft()
 * @see createGraftRetryTimer()
 * @see addRemoveAddr
 */
void pimDM::newMulticastAddr(addRemoveAddr *members)
{
	EV << "pimDM::newMulticastAddr" << endl;
	vector<IPAddress> newAddr = members->getAddr();
	PimInterface * pimInt = members->getInt();
	bool forward = false;

	// go through all new multicast addresses assigned to interface
	for (unsigned int i = 0; i < newAddr.size(); i++)
	{
		EV << "New multicast address: " << newAddr[i] << endl;
		vector<MulticastIPRoute*> routes = mrt->getRouteFor(newAddr[i]);

		// there is no route for group in the table in this moment
		if (routes.size() == 0)
			continue;

		// go through all multicast routes
		for (unsigned int j = 0; j < routes.size(); j++)
		{
			MulticastIPRoute *route = routes[j];
			InterfaceVector outInt = route->getOutInt();
			unsigned int k;

			// check on RPF interface
			if (route->getInIntId() == pimInt->getInterfaceID())
				continue;

			// is interface in list of outgoing interfaces?
			for (k = 0; k < outInt.size(); k++)
			{
				if (outInt[k].intId == pimInt->getInterfaceID())
				{
					EV << "Interface is already on list of outgoing interfaces" << endl;
					if (outInt[k].forwarding == Pruned)
						outInt[k].forwarding = Forward;
					forward = true;
					break;
				}
			}

			// interface is not in list of outgoing interfaces
			if (k == outInt.size())
			{
				EV << "Interface is not on list of outgoing interfaces yet, it will be added" << endl;
				outInterface newInt;
				newInt.intPtr = pimInt->getInterfacePtr();
				newInt.intId = pimInt->getInterfaceID();
				newInt.mode = Densemode;
				newInt.forwarding = Forward;
				newInt.pruneTimer = NULL;
				outInt.push_back(newInt);
				forward = true;
			}
			route->setOutInt(outInt);
			route->addFlag(C);

			// route was pruned, has to be added to multicast tree
			if (route->isFlagSet(P) && forward)
			{
				EV << "Route was pruned -> router has to join to multicast tree" << endl;

				// if source is not directly connected, send Graft to upstream
				if (!route->isFlagSet(A))
				{
					sendPimGraft(route->getInIntNextHop(), route->getSource(), route->getGroup(), route->getInIntId());
					PIMgrt *timer = createGraftRetryTimer(route->getSource(), route->getGroup());
					route->setGrt(timer);
				}
				else
					route->removeFlag(P);
			}
		}
	}
	mrt->generateShowIPMroute();
}

/**
 * NEW MULTICAST
 *
 * The method process notification about new multicast data stream. It goes through all PIM
 * interfaces and tests them if they can be added to the list of outgoing interfaces. If there
 * is no interface on the list at the end, the router will prune from the multicast tree.
 *
 * @param newRoute Pointer to new entry in the multicast routing table.
 * @see sendPimGraft()
 * @see createGraftRetryTimer()
 * @see addRemoveAddr
 */
void pimDM::newMulticast(MulticastIPRoute *newRoute)
{
	EV << "pimDM::newMulticast" << endl;

	// only outgoing interfaces are missing
	PimInterface *rpfInt = pimIft->getInterfaceByIntID(newRoute->getInIntId());
	bool pruned = true;

	// insert all PIM interfaces except rpf int
	for (int i = 0; i < pimIft->getNumInterface(); i++)
	{
		PimInterface *pimIntTemp = pimIft->getInterface(i);
		int intId = pimIntTemp->getInterfaceID();

		//check if PIM interface is not RPF interface
		if (pimIntTemp == rpfInt)
			continue;

		// create new outgoing interface
		outInterface newOutInt;
		newOutInt.intId = pimIntTemp->getInterfaceID();
		newOutInt.intPtr = pimIntTemp->getInterfacePtr();
		newOutInt.pruneTimer = NULL;

		switch (pimIntTemp->getMode())
		{
			case Dense:
				newOutInt.mode = Densemode;
				break;
			case Sparse:
				newOutInt.mode = Sparsemode;
				break;
		}

		// if there are neighbors on interface, we will forward
		if((pimNbt->getNeighborsByIntID(intId)).size() > 0)
		{
			newOutInt.forwarding = Forward;
			pruned = false;
			newRoute->addOutInt(newOutInt);
		}
		// if there is member of group, we will forward
		else if (pimIntTemp->isLocalIntMulticastAddress(newRoute->getGroup()))
		{
			newOutInt.forwarding = Forward;
			pruned = false;
			newRoute->addFlag(C);
			newRoute->addOutInt(newOutInt);
		}
		// in any other case interface is not involved
	}

	// directly connected to source, set State Refresh Timer
	if (newRoute->isFlagSet(A))
	{
		//FIXME record TTL (I do not know why???)
	    PIMsrt* timerSrt = createStateRefreshTimer(newRoute->getSource(), newRoute->getGroup());
	    newRoute->setSrt(timerSrt);
	}

	// set Source Active Timer (liveness of route)
	PIMsat* timerSat = createSourceActiveTimer(newRoute->getSource(), newRoute->getGroup());
    newRoute->setSat(timerSat);

	// if there is no outgoing interface, prune from multicast tree
	if (pruned)
	{
		EV << "pimDM::newMulticast: There is no outgoing interface for multicast, send Prune msg to upstream" << endl;
		newRoute->addFlag(P);

		if (!newRoute->isFlagSet(A))
			sendPimJoinPrune(newRoute->getInIntNextHop(), newRoute->getSource(), newRoute->getGroup(), newRoute->getInIntId());

		// FIXME set timer which I do not use
	}

	// add new route record to multicast routing table
	mrt->addRoute(newRoute);
	EV << "PimSplitter::newMulticast: New route was added to the multicast routing table." << endl;
}

