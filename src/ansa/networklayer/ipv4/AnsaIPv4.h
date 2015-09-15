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
 * @file AnsaIPv4.h
 * @date 21.10.2011
 * @author Veronika Rybova, Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @brief IPv4 implementation with changes for multicast
 * @details File contains extension of class IP, which can work also with multicast data and multicast
 */

#ifndef __INET_ANSAIPV4_H
#define __INET_ANSAIPV4_H

#include "common/INETDefs.h"

#ifdef WITH_MANET
#include "routing/extras/base/ControlManetRouting_m.h"
#endif

#include "networklayer/arp/ipv4/ARPPacket_m.h"
#include "networklayer/ipv4/IPv4.h"
#include "ansa/networklayer/pim/PimSplitter.h"
#include "ansa/networklayer/common/AnsaInterfaceEntry.h"
#include "ansa/networklayer/ipv4/AnsaRoutingTable.h"
#include "ansa/networklayer/ipv4/AnsaRoutingTableAccess.h"
#include "ansa/networklayer/pim/tables/PimInterfaceTable.h"
#include "ansa/networklayer/pim/PIMPacket_m.h"
#include "ansa/networklayer/pim/modes/pimSM.h"
#include "common/NotifierConsts.h"

/*
 * Migration towards ANSAINET2.2
enum AnsaIPProtocolId
{
    IP_PROT_PIM = 103
};
*/
/**
 * @brief Class is extension of the IP protocol implementation for multicast.
 * @details It extends class IP mainly by methods processing multicast stream.
 */

class LISPCore;

class INET_API AnsaIPv4 : public inet::IPv4
{
    private:
        AnsaRoutingTable            *rt;
        PimInterfaceTable           *pimIft;        /**< Pointer to table of PIM interfaces. */
        LISPCore* lispmod;

    protected:

        virtual void handlePacketFromNetwork(inet::IPv4Datagram *datagram, inet::InterfaceEntry *fromIE);
        virtual void routeMulticastPacket(inet::IPv4Datagram *datagram, inet::InterfaceEntry *destIE, inet::InterfaceEntry *fromIE);
        virtual void routePimSM (AnsaIPv4MulticastRoute *route, AnsaIPv4MulticastRoute *routeG, inet::IPv4Datagram *datagram, inet::IPv4ControlInfo *ctrl);
        virtual void routePimDM (AnsaIPv4MulticastRoute *route, inet::IPv4Datagram *datagram, inet::IPv4ControlInfo *ctrl);
        virtual inet::IPv4Datagram *encapsulate(cPacket *transportPacket, inet::IPv4ControlInfo *controlInfo);
        virtual void routeUnicastPacket(inet::IPv4Datagram *datagram, inet::InterfaceEntry *destIE, inet::IPv4Address destNextHopAddr);
        virtual void handleMessageFromHL(cPacket *msg);
        virtual void reassembleAndDeliver(inet::IPv4Datagram *datagram);
        virtual inet::InterfaceEntry *determineOutgoingInterfaceForMulticastDatagram(inet::IPv4Datagram *datagram, inet::InterfaceEntry *multicastIFOption);

        virtual void fragmentAndSend(inet::IPv4Datagram *datagram, inet::InterfaceEntry *ie, inet::IPv4Address nextHopAddr, int vforwarder);
        virtual void sendDatagramToOutput(inet::IPv4Datagram *datagram, inet::InterfaceEntry *ie, inet::IPv4Address nextHopAddr, int vforwarderId);
        int getVirtualForwarderId(inet::InterfaceEntry *ie, inet::MACAddress addr);
    public:
        AnsaIPv4() {}

    protected:
      virtual int numInitStages() const  {return 5;}
      virtual void initialize(int stage);
};


#endif
