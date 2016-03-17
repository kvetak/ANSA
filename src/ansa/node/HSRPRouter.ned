//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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

package ansa.node;

import inet.applications.contract.IUDPApp;
import inet.routing.contract.IIPRouting;
import inet.routing.contract.IPIMRouting;
import inet.transportlayer.contract.IUDP;
import ansa.networklayer.hsrp.HSRP;
import inet.node.inet.NodeBase;

//
// IPv4 router that supports wireless, Ethernet, PPP and external interfaces.
// By default, no wireless and external interfaces are added; the number of
// Ethernet and PPP ports depends on the external connections.
//
// - Can be connected via ethernet interface to other nodes using
//   the ethg gate. By default full-duplex connections are supported
//   only (twisted pair). Set **.eth.typename="EthernetInterface" for
//   a full/half-duplex CSMA/CD implementation (coaxial cable)
// - By default contains no wireless cards, however it can be configured
//   by the numRadios parameter. Wirless card type is configured by the
//   **.wlan.typename parameter. see:  inet.linklayer.ieee80211 or other
//   modules implementing ~IWirelessNic
// - Also external interfaces can be configured for HW in the loop simulation
//   using the numExtInterfaces parameter and setting the type using
//   **.ext.typename in the INI file. see: ~ExtInterface and ~IExternalNic
// - PPP queueing type can be set using **.ppp.queueType see: ~PPPInterface
// - If wireless card is present, node mobility can be set using **.mobility.typename
//   see: inet.mobility and ~IMobility
//
// By default, dynamic routing is not supported. Specific routing protocols can
// be added by setting the hasOSPF/hasRIP/hasBGP parameters.
//
module HSRPRouter extends NodeBase
{
    parameters:
        @display("i=abstract/router");
        forwarding = true;
        string udpType = default("UDP");

    submodules:
        hsrp: <"HSRP"> like IUDPApp {
            parameters:
                @display("p=472,108");
        }
        udp: <udpType> like IUDP {
            @display("p=423,225");
        }
    connections:
        hsrp.udpOut --> udp.appIn++;
        hsrp.udpIn <-- udp.appOut++;
        
        udp.ipOut --> networkLayer.transportIn++;
        udp.ipIn <-- networkLayer.transportOut++;
}
