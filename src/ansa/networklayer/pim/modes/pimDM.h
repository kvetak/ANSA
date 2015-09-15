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
 * @file pimDM.h
 * @date 29.10.2011
 * @author: Veronika Rybova, Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @brief File implements PIM dense mode.
 * @details Implementation according to RFC3973.
 */

#ifndef HLIDAC_PIMDM
#define HLIDAC_PIMDM

#include <omnetpp.h>
#include "ansa/networklayer/pim/PIMPacket_m.h"
#include "ansa/networklayer/pim/PIMTimer_m.h"
#include "networklayer/common/InterfaceTableAccess.h"
#include "ansa/networklayer/ipv4/AnsaRoutingTableAccess.h"
#include "base/NotificationBoard.h"
#include "common/NotifierConsts.h"
#include "ansa/networklayer/pim/tables/PimNeighborTable.h"
#include "ansa/networklayer/pim/tables/PimInterfaceTable.h"
#include "networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "networklayer/ipv4/IPv4InterfaceData.h"
#include "ansa/networklayer/ipv4/AnsaIPv4Route.h"



#define PT 180.0						/**< Prune Timer = 180s (3min). */
#define GRT 3.0							/**< Graft Retry Timer = 3s. */
#define SAT 210.0						/**< Source Active Timer = 210s, Cisco has 180s, after that, route is flushed */
#define SRT 60.0						/**< State Refresh Timer = 60s. */

/**
 * @brief Class implements PIM-DM (dense mode).
 */
class pimDM : public cSimpleModule, protected INotifiable
{
	private:
		AnsaRoutingTable           	*rt;           	/**< Pointer to routing table. */
	    inet::IInterfaceTable         	*ift;          	/**< Pointer to interface table. */
	    NotificationBoard 			*nb; 		   	/**< Pointer to notification table. */
	    PimInterfaceTable			*pimIft;		/**< Pointer to table of PIM interfaces. */
	    PimNeighborTable			*pimNbt;		/**< Pointer to table of PIM neighbors. */

	    // process events
	    void receiveChangeNotification(int category, const cPolymorphic *details);
	    void newMulticast(AnsaIPv4MulticastRoute *newRoute);
	    void newMulticastAddr(addRemoveAddr *members);
	    void oldMulticastAddr(addRemoveAddr *members);
	    void dataOnPruned(inet::IPv4Address destAddr, inet::IPv4Address srcAddr);
	    void dataOnNonRpf(inet::IPv4Address group, inet::IPv4Address source, int intId);
	    void dataOnRpf(AnsaIPv4MulticastRoute *route);
	    void rpfIntChange(AnsaIPv4MulticastRoute *route);

	    // process timers
	    void processPIMTimer(PIMTimer *timer);
	    void processPruneTimer(PIMpt * timer);
	    void processGraftRetryTimer(PIMgrt *timer);
	    void processSourceActiveTimer(PIMsat * timer);
	    void processStateRefreshTimer(PIMsrt * timer);

	    // create timers
	    PIMpt* createPruneTimer(inet::IPv4Address source, inet::IPv4Address group, int intId, int holdTime);
	    PIMgrt* createGraftRetryTimer(inet::IPv4Address source, inet::IPv4Address group);
	    PIMsat* createSourceActiveTimer(inet::IPv4Address source, inet::IPv4Address group);
	    PIMsrt* createStateRefreshTimer(inet::IPv4Address source, inet::IPv4Address group);

	    // process PIM packets
	    void processPIMPkt(PIMPacket *pkt);
	    void processJoinPruneGraftPacket(PIMJoinPrune *pkt, PIMPacketType type);
	    void processPrunePacket(AnsaIPv4MulticastRoute *route, int intId, int holdTime);
	    void processGraftPacket(inet::IPv4Address source, inet::IPv4Address group, inet::IPv4Address sender, int intId);
	    void processGraftAckPacket(AnsaIPv4MulticastRoute *route);
	    void processStateRefreshPacket(PIMStateRefresh *pkt);

	    //create PIM packets
	    void sendPimJoinPrune(inet::IPv4Address nextHop, inet::IPv4Address src, inet::IPv4Address grp, int intId);
	    void sendPimGraft(inet::IPv4Address nextHop, inet::IPv4Address src, inet::IPv4Address grp, int intId);
	    void sendPimGraftAck(PIMGraftAck *msg);
	    void sendPimStateRefresh(inet::IPv4Address originator, inet::IPv4Address src, inet::IPv4Address grp, int intId, bool P);

	    void setUpInterface();



	protected:
		virtual int numInitStages() const  {return 5;}
		virtual void handleMessage(cMessage *msg);
		virtual void initialize(int stage);
};

#endif
