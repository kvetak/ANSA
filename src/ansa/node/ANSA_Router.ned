//Based on:
//@see /src/inet/node/inet/Router
//
// The MIT License (MIT)
//
// Copyright (c) 2016 Brno University of Technology
//
//@author Vladimir Vesely (iveselyATfitDOTvutbrDOTcz)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

package ansa.node;

import ansa.routing.babel.BabelRouting;
import inet.applications.contract.IUDPApp;
import inet.routing.contract.IBGPRouting;
import inet.routing.contract.IIPRouting;
import inet.routing.contract.IPIMRouting;
import inet.transportlayer.contract.ITCP;
import inet.transportlayer.contract.IUDP;

import ansa.node.ANSA_NodeBase;

module ANSA_Router extends ANSA_NodeBase
{
    parameters:
        @display("i=abstract/router");
        bool hasOSPF = default(false);
        bool hasRIP = default(false);
        bool hasBGP = default(false);
        bool hasPIM = default(false);
        bool hasEIGRP = default(false);
        bool hasBABEL = default(false);
        bool hasLISP = default(false);
        string tcpType = default(firstAvailableOrEmpty("TCP", "TCP_lwIP", "TCP_NSC"));  // tcp implementation (e.g. ~TCP, ~TCP_lwIP, ~TCP_NSC) or ~TCPSpoof
        string udpType = default("UDP");

        forwarding = true;
    submodules:
        ospf: <"OSPFRouting"> like IIPRouting if hasOSPF {
            parameters:
                @display("p=573,281,row");
        }
        bgp: <"BGPRouting"> like IBGPRouting if hasBGP {
            parameters:
                ospfRoutingModule = default(hasOSPF ? "^.ospf" : "");
                @display("p=307,80;i=block/network");
        }
        tcp: <tcpType> like ITCP if hasBGP && tcpType != "" {
            parameters:
                @display("p=307,180;i=block/transport");
        }
        rip: <"RIPRouting"> like IUDPApp if hasRIP {
            parameters:
                @display("p=418,80");
        }

        babel: <"BabelRouting"> like IUDPApp if hasBABEL {
            parameters:
                @display("p=511,80");
        }

        udp: <udpType> like IUDP if (hasRIP || hasBABEL) {
            @display("p=472,180");
        }
        pim: <"PIMRouting"> like IPIMRouting if hasPIM {
            @display("p=277,281");
        }

    connections allowunconnected:
        if hasOSPF {
            ospf.ipOut --> networkLayer.transportIn++;
            ospf.ipIn <-- networkLayer.transportOut++;
        }

        if hasBGP {
            bgp.tcpOut --> tcp.appIn++;
            bgp.tcpIn <-- tcp.appOut++;

            tcp.ipOut --> networkLayer.transportIn++;
            tcp.ipIn <-- networkLayer.transportOut++;
        }

        if hasRIP {
            rip.udpOut --> udp.appIn++;
            rip.udpIn <-- udp.appOut++;

            udp.ipOut --> networkLayer.transportIn++;
            udp.ipIn <-- networkLayer.transportOut++;
        }
        
		if hasBABEL {
            babel.udpOut --> udp.appIn++;
            babel.udpIn <-- udp.appOut++;

            udp.ipOut --> networkLayer.transportIn++;
            udp.ipIn <-- networkLayer.transportOut++;
        }

        if hasPIM {
            pim.networkLayerOut --> networkLayer.transportIn++;
            pim.networkLayerIn <-- networkLayer.transportOut++;
        }
}
