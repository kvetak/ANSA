//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#include <omnetpp.h>
#include <stdlib.h>
#include <string.h>

#include "AnsaIP.h"
#include "IPDatagram.h"
#include "IPControlInfo.h"
#include "ICMPMessage_m.h"
#include "IPv4InterfaceData.h"
#include "ARPPacket_m.h"

Define_Module(AnsaIP);

void AnsaIP::initialize()
{
	INET_API IP::initialize();
}

void AnsaIP::handlePacketFromNetwork(IPDatagram *datagram)
{
    //
    // "Prerouting"
    //

    // check for header biterror
    if (datagram->hasBitError())
    {
        // probability of bit error in header = size of header / size of total message
        // (ignore bit error if in payload)
        double relativeHeaderLength = datagram->getHeaderLength() / (double)datagram->getByteLength();
        if (dblrand() <= relativeHeaderLength)
        {
            EV << "bit error found, sending ICMP_PARAMETER_PROBLEM\n";
            icmpAccess.get()->sendErrorMessage(datagram, ICMP_PARAMETER_PROBLEM, 0);
            return;
        }
    }

    // remove control info
    //if (datagram->getTransportProtocol()!=IP_PROT_DSR && datagram->getTransportProtocol()!=IP_PROT_MANET)
    //{
    //    delete datagram->removeControlInfo();
    //}
    if (datagram->getMoreFragments()) delete datagram->removeControlInfo(); // delete all control message except the last

    // hop counter decrement; FIXME but not if it will be locally delivered
    datagram->setTimeToLive(datagram->getTimeToLive()-1);

    // FIXME send IGMP packet to IGMP module 
    if (datagram->getTransportProtocol() == IP_PROT_IGMP)
    {
       	cPacket *packet = decapsulateIP(datagram);
        send(packet, "transportOut", mapping.getOutputGateForProtocol(IP_PROT_IGMP));
	return;
    }

    // send PIM packet to PIM module
    if (datagram->getTransportProtocol() == IP_PROT_PIM)
	{
		cPacket *packet = decapsulateIP(datagram);
		send(packet, "transportOut", mapping.getOutputGateForProtocol(IP_PROT_PIM));
	return;
	}
        
    // route packet
    if (!datagram->getDestAddress().isMulticast())
        routePacket(datagram, NULL, false);
    else
        routeMulticastPacket(datagram, NULL, getSourceInterfaceFrom(datagram));
}

