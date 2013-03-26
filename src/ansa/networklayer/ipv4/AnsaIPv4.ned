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
//
// @file AnsaIPv4.ned
// @date 21.10.2011
// @author Veronika Rybova, Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
// @brief Inherited module frmo IPv4
// @details Simple module for AnsaIPv4


package inet.ansa.networklayer.ipv4;

//
// Implements the \IP protocol. The protocol header is represented
// by the IPDatagram message class.
//
// <b>Interfacing with higher layer protocols</b>
//
// To send a packet over IP from a higher layer protocol, the module should
// fill in an IPControlInfo object, attach it to the packet with cMessage's
// setControlInfo() method, the send it to the IP module.
//
// When IP sends up a packet to a higher layer protocol, it will also attach
// an IPControlInfo to the packet, with the source and destination IP address,
// etc. of the IP datagram in which the packet arrived.
//
// IP can serve several higher-layer protocols. When delivering packets
// to them, the output gate is determined from the Protocol field in the
// \IP header. The protocol-to-gateindex mapping must be given in the
// protocolMapping string parameter, in the following format:
// <pre>
// protocolnumber:gateindex, protocolnumber:gateindex, ...
// </pre>
//
// The recommended setting is:
// - \TCP (6) : 0
// - \UDP (17) : 1
// - \ICMP (1) : 2
// - \IGMP (2) : 3
// - \RSVP (46) : 4
//
// That is, <tt>protocolMapping="6:0,17:1,1:2,2:3,46:4"</tt>
//
// <b>Routing and interfacing with lower layers</b>
//
// The routing table is stored in the module RoutingTable. When a datagram
// needs to be routed, IP queries RoutingTable for the output interface
// (or "port") and next hop address of the packet. This is done by directly
// calling C++ methods (such as findBestMatchingRoute(destAddress)) of RoutingTable.
// No message exchange with RoutingTable takes place.
//
// A routed datagram will be sent to the queueOut gate, with an
// IPRoutingDecision control info object attached which contains
// next hop address and interface number. queueOut is expected to be
// connected to ARP.
//
// Routing protocol implementations (e.g. OSPF and ISIS) can also query
// and manipulate the route table by calling RoutingTable's methods in C++.
//
// <b>Performance model, QoS</b>
//
// In the current form, IP contains a FIFO which queues up \IP datagrams;
// datagrams are processed in order. The processing time is determined by the
// procDelay module parameter.
//
// The current performance model comes from the QueueBase C++ base class.
// If you need a more sophisticated performance model, you may change the
// module implementation (the IP class), and: (1) override the startService()
// method which determines processing time for a packet, or (2) use a
// different base class.
//
// @see RoutingTable, IPControlInfo, IPRoutingDecision, ARP
//
// @author Andras Varga
//

import inet.networklayer.ipv4.IPv4;

simple AnsaIPv4 extends IPv4
{
    @class(AnsaIPv4);
}
