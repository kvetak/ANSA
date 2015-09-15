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
 * @file pimSM.h
 * @date 29.10.2011
 * @author: Veronika Rybova, Tomas Prochazka (mailto:xproch21@stud.fit.vutbr.cz), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @brief File implements PIM sparse mode.
 * @details Implementation will be done in the future according to RFC4601.
 */

#ifndef PIMSM_H
#define PIMSM_H

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
#include "ansa/networklayer/ipv4/AnsaIPv4.h"

#define KAT 180.0                       /**< Keep alive timer, if RPT is disconnect */
#define KAT2 210.0                      /**< Keep alive timer, if RPT is connect */
#define RST 60.0                        /**< Register-stop Timer*/
#define JT 60.0                         /**< Join Timer */
#define REGISTER_PROBE_TIME 5.0         /**< Register Probe Time */
#define HOLDTIME 210.0                  /**< Holdtime for Expiry Timer */
#define HOLDTIME_HOST 180.0             /**< Holdtime for interface ET connected to host */
#define PPT 3.0                         /**< value for Prune-Pending Timer*/
#define ALL_PIM_ROUTERS "224.0.0.13"    /**< Multicast address for PIM routers */
#define MAX_TTL 255                     /**< Maximum TTL */
#define NO_INT_TIMER -1
#define CISCO_SPEC_SIM 1                /**< Enable Cisco specific simulation; 1 = enable, 0 = disable */


struct multDataInfo
{
    inet::IPv4Address origin;
    inet::IPv4Address group;
    unsigned interface_id;
    inet::IPv4Address srcAddr;
};

enum joinPruneMsg
{
    JoinMsg = 0,
    PruneMsg = 1
};

enum JPMsgType
{
    G = 1,
    SG = 2,
    SGrpt = 3
};


/**
 * @brief Class implements PIM-SM (sparse mode).
 */
class pimSM : public cSimpleModule, protected INotifiable
{
    private:
        AnsaRoutingTable            *rt;            /**< Pointer to routing table. */
        inet::IInterfaceTable             *ift;           /**< Pointer to interface table. */
        NotificationBoard           *nb;            /**< Pointer to notification table. */
        PimInterfaceTable           *pimIft;        /**< Pointer to table of PIM interfaces. */
        PimNeighborTable            *pimNbt;        /**< Pointer to table of PIM neighbors. */

        inet::IPv4Address RPAddress;
        std::string SPTthreshold;

    private:
        void receiveChangeNotification(int category, const cPolymorphic *details);
        void newMulticastRegisterDR(AnsaIPv4MulticastRoute *newRoute);
        void newMulticastReciever(addRemoveAddr *members);
        void removeMulticastReciever(addRemoveAddr *members);


        // process timers
        void processPIMTimer(PIMTimer *timer);
        void processKeepAliveTimer(PIMkat *timer);
        void processRegisterStopTimer(PIMrst *timer);
        void processExpiryTimer(PIMet *timer);
        void processJoinTimer(PIMjt *timer);
        void processPrunePendingTimer(PIMppt *timer);


        void restartExpiryTimer(AnsaIPv4MulticastRoute *route, inet::InterfaceEntry *originIntf, int holdTime);
        void dataOnRpf(AnsaIPv4MulticastRoute *route);

        // set timers
        PIMkat* createKeepAliveTimer(inet::IPv4Address source, inet::IPv4Address group);
        PIMrst* createRegisterStopTimer(inet::IPv4Address source, inet::IPv4Address group);
        PIMet*  createExpiryTimer(int intID, int holdtime, inet::IPv4Address group, inet::IPv4Address source, int StateType);
        PIMjt*  createJoinTimer(inet::IPv4Address group, inet::IPv4Address JPaddr, inet::IPv4Address upstreamNbr, int JoinType);
        PIMppt* createPrunePendingTimer(inet::IPv4Address group, inet::IPv4Address JPaddr, inet::IPv4Address upstreamNbr, JPMsgType JPtype);

        // pim messages
        void sendPIMRegister(inet::IPv4ControlInfo *ctrl);
        void sendPIMRegisterStop(inet::IPv4Address source, inet::IPv4Address dest, inet::IPv4Address multGroup, inet::IPv4Address multSource);
        void sendPIMRegisterNull(inet::IPv4Address multSource, inet::IPv4Address multDest);
        void sendPIMJoinPrune(inet::IPv4Address multGroup, inet::IPv4Address joinPruneIPaddr, inet::IPv4Address upstreamNbr, joinPruneMsg JoinPrune, JPMsgType JPtype);
        void sendPIMJoinTowardSource(multDataInfo *info);
        void forwardMulticastData(multDataInfo *info);

        // process PIM messages
        void processPIMPkt(PIMPacket *pkt);
        void processRegisterPacket(PIMRegister *pkt);
        void processRegisterStopPacket(PIMRegisterStop *pkt);
        void processJoinPacket(PIMJoinPrune *pkt, inet::IPv4Address multGroup, EncodedAddress encodedAddr);
        void processPrunePacket(PIMJoinPrune *pkt, inet::IPv4Address multGroup, EncodedAddress encodedAddr);
        void processJoinPrunePacket(PIMJoinPrune *pkt);
        void processSGJoin(PIMJoinPrune *pkt,inet::IPv4Address multOrigin, inet::IPv4Address multGroup);
        void processJoinRouteGexistOnRP(inet::IPv4Address multGroup, inet::IPv4Address packetOrigin, int msgHoldtime);

    public:
        //PIM-SM clear implementation
        void setRPAddress(std::string address);
        void setSPTthreshold(std::string address);
        inet::IPv4Address getRPAddress () {return RPAddress;}
        std::string getSPTthreshold () {return SPTthreshold;}
        virtual bool IamRP (inet::IPv4Address RPaddress);
        bool IamDR (inet::IPv4Address sourceAddr);
        inet::IPv4ControlInfo *setCtrlForMessage (inet::IPv4Address destAddr,inet::IPv4Address srcAddr,int protocol, int interfaceId, int TTL);

	protected:
		virtual int numInitStages() const  {return 5;}
		virtual void handleMessage(cMessage *msg);
		virtual void initialize(int stage);
};

#endif
