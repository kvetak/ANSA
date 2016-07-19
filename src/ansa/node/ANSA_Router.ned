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

package ansa.node;

import ansa.routing.babel.BabelRouting;
import ansa.routing.eigrp.EigrpProcessDS;

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
        bool hasHSRP = default(false);
        bool hasGLBP = default(false);
        bool hasCDP = default(false);
        bool hasLLDP = default(false);

        string tcpType = default(firstAvailableOrEmpty("TCP", "TCP_lwIP", "TCP_NSC"));  // tcp implementation (e.g. ~TCP, ~TCP_lwIP, ~TCP_NSC) or ~TCPSpoof
        string udpType = default("UDP");
        forwarding = true;
        ethernetType = default("ANSA_EthernetInterface");

        networkLayer.enableCDP = hasCDP;
        networkLayer.enableLLDP = hasLLDP;

    submodules:
        ospf: <"OSPFRouting"> like IIPRouting if hasOSPF {
            parameters:
                @display("p=573,300,row");
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

        hsrp: <"HSRP"> like IUDPApp if hasHSRP {
            parameters:
                @display("p=590,80");
        }

        glbp: <"GLBP"> like IUDPApp if hasGLBP {
            parameters:
                @display("p=669,80");
        }

        udp: <udpType> like IUDP if (hasRIP || hasBABEL || hasHSRP || hasGLBP) {
            @display("p=472,180");
        }

        pim: <"PIMRouting"> like IPIMRouting if hasPIM {
            @display("p=277,281");
        }

        eigrp: EigrpProcessDS if hasEIGRP {

            @display("p=573,242");
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
}
