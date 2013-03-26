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
#include "PIMPacket_m.h"
#include "PIMTimer_m.h"

#include "InterfaceTableAccess.h"
#include "AnsaRoutingTableAccess.h"
#include "NotificationBoard.h"
#include "NotifierConsts.h"
#include "PimNeighborTable.h"
#include "PimInterfaceTable.h"
#include "IPv4ControlInfo.h"
#include "IPv4InterfaceData.h"
#include "AnsaIPv4Route.h"
#include "AnsaIPv4.h"

#define KAT 180.0                       /**< Cisco has 180s, RFC4601 KAT = 210s after that, route is flushed */
#define RST 60.0                        /**< Register-stop Timer*/
#define JT 60.0                         /**< Join Timer */
#define REGISTER_PROBE_TIME 5.0
#define HOLDTIME 210.0                  /**< Holdtime for Expiry Timer */


/**
 * @brief Class implements PIM-SM (sparse mode).
 */
class pimSM : public cSimpleModule, protected INotifiable
{
    private:
        AnsaRoutingTable            *rt;            /**< Pointer to routing table. */
        IInterfaceTable             *ift;           /**< Pointer to interface table. */
        NotificationBoard           *nb;            /**< Pointer to notification table. */
        PimInterfaceTable           *pimIft;        /**< Pointer to table of PIM interfaces. */
        PimNeighborTable            *pimNbt;        /**< Pointer to table of PIM neighbors. */

        IPv4Address RPAddress;
        std::string SPTthreshold;

    private:
        void receiveChangeNotification(int category, const cPolymorphic *details);
        void newMulticastRegisterDR(AnsaIPv4MulticastRoute *newRoute);
        void newMulticastReciever(IPv4ControlInfo *igmpCtrl);

        // process timers
        void processPIMTimer(PIMTimer *timer);
        void processKeepAliveTimer(PIMkat *timer);
        void processRegisterStopTimer(PIMrst *timer);
        void processExpiryTimer(PIMet *timer);
        void processJoinTimer(PIMjt *timer);

        // set timers
        PIMkat* createKeepAliveTimer(IPv4Address source, IPv4Address group);
        PIMrst* createRegisterStopTimer(IPv4Address source, IPv4Address group);
        PIMet*  createExpiryTimer(int holdtime, IPv4Address group);
        PIMjt*  createJoinTimer(IPv4Address group, IPv4Address JPaddr, IPv4Address upstreamNbr);

        // pim messages
        void sendPIMRegister(IPv4Datagram *datagram);
        void sendPIMRegisterStop(IPv4Address source, IPv4Address dest, IPv4Address multGroup, IPv4Address multSource);
        void sendPIMRegisterNull(IPv4Address multSource, IPv4Address multDest);
        void sendPIMJoin(IPv4Address multGroup, IPv4Address joinIPaddr, IPv4Address upstreamNbr);

        // process PIM messages
        void processPIMPkt(PIMPacket *pkt);
        void processRegisterPacket(PIMRegister *pkt);
        void processRegisterStopPacket(PIMRegisterStop *pkt);
        void processJoinPrunePacket(PIMJoinPrune *pkt);

    public:
        //PIM-SM clear implementation
        void setRPAddress(std::string address);
        void setSPTthreshold(std::string address);
        IPv4Address getRPAddress () {return RPAddress;}
        std::string getSPTthreshold () {return SPTthreshold;}
        bool IamRP ();
        bool IamDR (IPv4Address sourceAddr);

	protected:
		virtual int numInitStages() const  {return 5;}
		virtual void handleMessage(cMessage *msg);
		virtual void initialize(int stage);
};

#endif
