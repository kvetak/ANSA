//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package ansa.networklayer.arp.ipv4;

import inet.networklayer.arp.ipv4.Arp;


//
// Implements the Address Resolution Protocol for Ipv4 and IEEE 802 6-byte
// MAC addresses.
//
// Arp packets are represented by the ~ArpPacket class.
//
// Communication Between Arp and Ipv4 modules:
// Ipv4 module call the Arp::getMACAddressFor(ipv4Address) method for get the MacAddress
// of next hop. If this method could not resolve an Ipv4 address to MAC address, then Ipv4
// uses the Arp::startAddressResolution() method.
// Arp emit a completedArpResolution signal, when added/modified an entry in Arp table.
// Arp emit a failedArpResolution signal when an Arp request failed (Arp request sent
// retryCount times and retryTimeout elapsed).
//
// Arp resolution is used over <i>broadcast</i> interfaces only,
// that is, over interfaces that have the <tt>isBroadcast()</tt> flag set in
// <tt>InterfaceEntry</tt> (see ~InterfaceTable). Since routing files
// (<tt>.irt</tt> or <tt>.mrt</tt> files, given as parameter to ~Ipv4RoutingTable)
// may modify the default interface configuration, you must take care that
// these files don't contain the word <tt>BROADCAST</tt> e.g. for Ppp
// interfaces.
//
simple AnsaARP extends Arp
{
    parameters:
        @class(AnsaARP);
        @signal[recvRequest](type=bool);
}
