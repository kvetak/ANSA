/**
 * @file pimDM.h
 * @date 29.10.2011
 * @author: Veronika Rybova
 * @brief File implements PIM dense mode.
 * @details Implementation according to RFC3973.
 */

#ifndef HLIDAC_PIMDM
#define HLIDAC_PIMDM

#include <omnetpp.h>
#include "PIMPacket_m.h"
#include "PIMTimer_m.h"
#include "AnsaInterfaceTableAccess.h"
#include "MulticastRoutingTableAccess.h"
#include "RoutingTableAccess.h"
#include "NotificationBoard.h"
#include "NotifierConsts.h"
#include "PimNeighborTable.h"
#include "PimInterfaceTable.h"
#include "IPControlInfo.h"
#include "IPv4InterfaceData.h"

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
		IRoutingTable           	*rt;           	/**< Pointer to routing table. */
		MulticastRoutingTable 		*mrt;			/**< Pointer to multicast routing table. */
	    IInterfaceTable         	*ift;          	/**< Pointer to interface table. */
	    NotificationBoard 			*nb; 		   	/**< Pointer to notification table. */
	    PimInterfaceTable			*pimIft;		/**< Pointer to table of PIM interfaces. */
	    PimNeighborTable			*pimNbt;		/**< Pointer to table of PIM neighbors. */

	    // process events
	    void receiveChangeNotification(int category, const cPolymorphic *details);
	    void newMulticast(MulticastIPRoute *newRoute);
	    void newMulticastAddr(addRemoveAddr *members);
	    void oldMulticastAddr(addRemoveAddr *members);
	    void dataOnPruned(IPAddress destAddr, IPAddress srcAddr);
	    void dataOnNonRpf(IPAddress group, IPAddress source, int intId);
	    void dataOnRpf(MulticastIPRoute *route);
	    void rpfIntChange(MulticastIPRoute *route);

	    // process timers
	    void processPIMTimer(PIMTimer *timer);
	    void processPruneTimer(PIMpt * timer);
	    void processGraftRetryTimer(PIMgrt *timer);
	    void processSourceActiveTimer(PIMsat * timer);
	    void processStateRefreshTimer(PIMsrt * timer);

	    // create timers
	    PIMpt* createPruneTimer(IPAddress source, IPAddress group, int intId, int holdTime);
	    PIMgrt* createGraftRetryTimer(IPAddress source, IPAddress group);
	    PIMsat* createSourceActiveTimer(IPAddress source, IPAddress group);
	    PIMsrt* createStateRefreshTimer(IPAddress source, IPAddress group);

	    // process PIM packets
	    void processPIMPkt(PIMPacket *pkt);
	    void processJoinPruneGraftPacket(PIMJoinPrune *pkt, PIMPacketType type);
	    void processPrunePacket(MulticastIPRoute *route, int intId, int holdTime);
	    void processGraftPacket(IPAddress source, IPAddress group, IPAddress sender, int intId);
	    void processGraftAckPacket(MulticastIPRoute *route);
	    void processStateRefreshPacket(PIMStateRefresh *pkt);

	    //create PIM packets
	    void sendPimJoinPrune(IPAddress nextHop, IPAddress src, IPAddress grp, int intId);
	    void sendPimGraft(IPAddress nextHop, IPAddress src, IPAddress grp, int intId);
	    void sendPimGraftAck(PIMGraftAck *msg);
	    void sendPimStateRefresh(IPAddress originator, IPAddress src, IPAddress grp, int intId, bool P);



	protected:
		virtual int numInitStages() const  {return 5;}
		virtual void handleMessage(cMessage *msg);
		virtual void initialize(int stage);
};

#endif
