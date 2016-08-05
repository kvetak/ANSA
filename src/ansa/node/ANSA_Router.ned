//
// Copyright (C) 2009 - today, Brno University of Technology, Czech Republic
// Copyright (C) 2005 Andras Babos
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//               2010 Zoltan Bojthe
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
// @author Vladimir Vesely (ivesely@fit.vutbr.cz)
// @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3

package ansa.node;

import ansa.routing.eigrp.EigrpProcessDS;
import ansa.routing.lisp.LISPRouting;
import inet.applications.contract.IUDPApp;
import inet.routing.contract.IBGPRouting;
import inet.routing.contract.IIPRouting;
import inet.routing.contract.IPIMRouting;
import inet.transportlayer.contract.ITCP;
import inet.transportlayer.contract.IUDP;
import ansa.networklayer.isis.ISIS;

import ansa.node.ANSA_NodeBase;

module ANSA_Router extends ANSA_NodeBase
{
    parameters:
        @display("i=abstract/router;bgb=868,562");
        bool hasOSPF = default(false);
        bool hasRIP = default(false);
        bool hasBGP = default(false);
        bool hasPIM = default(false);
        bool hasEIGRP = default(false);
        bool hasBABEL = default(false);
        bool hasLISP = default(false);
        bool hasHSRP = default(false);
        bool hasGLBP = default(false);
        bool hasVRRP = default(false);
        bool hasCDP = default(false);
        bool hasLLDP = default(false);
        bool hasISIS = default(false);

        string tcpType = default(firstAvailableOrEmpty("TCP", "TCP_lwIP", "TCP_NSC"));  // tcp implementation (e.g. ~TCP, ~TCP_lwIP, ~TCP_NSC) or ~TCPSpoof
        string udpType = default("UDP");
        forwarding = true;
        ethernetType = default("ANSA_EthernetInterface");

        networkLayer.enableCDP = hasCDP;
        networkLayer.enableLLDP = hasLLDP;

    submodules:
        ospf: <"OSPFRouting"> like IIPRouting if hasOSPF {
            parameters:
                @display("p=611,269,row");
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
                @display("p=495,80");
        }

        babel: <"BabelRouting"> like IUDPApp if hasBABEL {
            parameters:
                @display("p=564,80");
        }

        hsrp: <"HSRP"> like IUDPApp if hasHSRP {
            parameters:
                @display("p=630,80");
        }

        glbp: <"GLBP"> like IUDPApp if hasGLBP {
            parameters:
                @display("p=698,80");
        }

        vrrp: <"VRRPv2"> like IIPRouting if hasVRRP {
            parameters:
                @display("p=225,269");
        }

        udp: <udpType> like IUDP if (hasRIP || hasBABEL || hasHSRP || hasGLBP) {
            parameters:
                @display("p=534,180");
        }

        pim: <"PIMRouting"> like IPIMRouting if hasPIM {
            parameters:
                @display("p=307,269");
        }

        eigrp: EigrpProcessDS if hasEIGRP {
            parameters:
                @display("p=534,269");
        }

        lisp: LISPRouting if hasLISP {
            parameters:
                @display("p=427,80");
        }

        isis: ISIS if hasISIS {
            parameters:
                @display("p=689.388,268.284");
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

        if hasHSRP {
            udp.appOut++ --> hsrp.udpIn;
            hsrp.udpOut --> udp.appIn++;

            udp.ipOut --> networkLayer.transportIn++;
            udp.ipIn <-- networkLayer.transportOut++;
        }

        if hasGLBP {
            udp.appOut++ --> glbp.udpIn;
            glbp.udpOut --> udp.appIn++;

            udp.ipOut --> networkLayer.transportIn++;
            udp.ipIn <-- networkLayer.transportOut++;
        }

        if hasEIGRP {
            eigrp.splitterOut --> networkLayer.transportIn++ if enableIPv4;
            eigrp.splitterIn <-- networkLayer.transportOut++ if enableIPv4;
            eigrp.splitter6Out --> networkLayer.transportIn++ if enableIPv6;
            eigrp.splitter6In <-- networkLayer.transportOut++ if enableIPv6;
        }
        
        if hasISIS {
            isis.lowerLayerOut --> networkLayer.transportIn++ if enableCLNS;
            isis.lowerLayerIn <-- networkLayer.transportOut++ if enableCLNS;
        }

        if hasVRRP {
            vrrp.ipOut --> networkLayer.transportIn++ if enableIPv4;
            vrrp.ipIn <-- networkLayer.transportOut++ if enableIPv4;
        }

        if hasLISP {
            udp.appOut++ --> lisp.udpContrIn;
            udp.appIn++ <-- lisp.udpContrOut;
            udp.appOut++ --> lisp.udpDataIn;
            udp.appIn++ <-- lisp.udpDataOut;
            networkLayer.transportOut++ --> lisp.ipv4In if enableIPv4;
            networkLayer.transportIn++ <-- lisp.ipv4Out if enableIPv4;
            networkLayer.transportOut++ --> lisp.ipv6In if enableIPv6;
            networkLayer.transportIn++ <-- lisp.ipv6Out if enableIPv6;
        }
}
