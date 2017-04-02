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

//!!!!! !!!!! !!!!! !!!!! !!!!! !!!!! !!!!! !!!!! !!!!! !!!!! !!!!! !!!!! !!!!!  
//This is not a real node, it is here only for purposes of drawing various
//mutations of ANSARouter for research papers
//!!!!! !!!!! !!!!! !!!!! !!!!! !!!!! !!!!! !!!!! !!!!! !!!!! !!!!! !!!!! !!!!!

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
import inet.common.lifecycle.NodeStatus;
import inet.common.packet.PcapRecorder;
import inet.linklayer.contract.IExternalNic;
import inet.linklayer.contract.ITunNic;
import inet.linklayer.contract.IWiredNic;
import inet.linklayer.contract.IWirelessNic;
import inet.linklayer.loopback.LoopbackInterface;
import inet.mobility.contract.IMobility;
import inet.networklayer.contract.IRoutingTable;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.contract.INetworkLayer;
import inet.power.contract.IEnergyStorage;
import inet.power.contract.IEnergyGenerator;

import ansa.networklayer.multi.ANSA_MultiNetworkLayer;
import ansa.linklayer.ethernet.ANSA_EthernetInterface;

import ansa.node.ANSA_NodeBase;

module ANSARouter
{
    parameters:
        @display("i=abstract/router;bgb=459.592,595.432,white");
        @networkNode;
        @labels(node,ethernet-node,wireless-node);
        bool hasStatus = default(false);

        bool enableIPv4 = default(true);
        bool enableIPv6 = default(true);
        bool enableCLNS = default(true);

        *.enableIPv4 = enableIPv4;
        *.enableIPv6 = enableIPv6;
        *.enableCLNS = enableCLNS;

        int numExtInterfaces = default(0);
        int numRadios = default(0);        // the number of radios in the router. by default no wireless
        int numPcapRecorders = default(0); // no of PcapRecorders.
        int numTunInterfaces = default(0);
        int numLoopbacks = default(1);
        string mobilityType = default(numRadios > 0 ? "StationaryMobility" : "");

        string networkLayerType = default("ANSA_MultiNetworkLayer");
        string routingTableType = default("ANSA_MultiRoutingTable");
        //string ethernetType = default(firstAvailableOrEmpty("EthernetInterface","AnsaEthernetInterface"));

        bool forwarding = default(true);
        bool multicastForwarding = default(false);
        string energyStorageType = default("");
        string energyGeneratorType = default("");

        routingTable.forwarding = forwarding;
        routingTable.multicastForwarding = multicastForwarding;   // for IPv4, IPv6, Generic

        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(routingTableType != "" ? absPath(".routingTable") : "");
        //*.energySourceModule = default(energyStorageType != "" ? absPath(".energyStorage") : "");
        //*.mobilityModule = default(mobilityType != "" ? absPath(".mobility") : "");

        xml configData = default(xmldoc("config.xml"));
        networkLayer.configurator.configData = configData;

        @display("i=abstract/router;bgb=867.112,779.948");
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

        //ethernetType = default("ANSA_EthernetInterface");

        networkLayer.enableCDP = hasCDP;
        networkLayer.enableLLDP = hasLLDP;

    gates:
        input radioIn[numRadios] @directIn;
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);
    submodules:

        status: NodeStatus if hasStatus {
            @display("p=37.356,33.96");
        }
        energyStorage: <energyStorageType> like IEnergyStorage if energyStorageType != "" {
            parameters:
                @display("p=330.544,372.428;i=block/plug;is=s");
        }
        energyGenerator: <energyGeneratorType> like IEnergyGenerator if energyGeneratorType != "" {
            parameters:
                @display("p=330.544,429.028;i=block/plug;is=s");
        }
        // optional mobility module. Required only if wireless cards are present
        mobility: <mobilityType> like IMobility if mobilityType != "" {
            parameters:
                @display("p=37.356,88.296");
        }
        // network layer
        networkLayer: ANSA_MultiNetworkLayer {
            parameters:
                @display("p=187.912,393.936;q=queue");
        }
        // routing table
        routingTable: <routingTableType> like IRoutingTable if routingTableType != "" {
            parameters:
                @display("p=166.404,32.828;is=n");
        }
        // linklayer
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=268.284,33.96;is=n");
        }
        pcapRecorder[_]: PcapRecorder {
            @display("p=63.392002,447.14,r,10;i=block/buffer2");
        }
        lo[_]: LoopbackInterface {
            @display("p=63.392002,530.908;is=s");
        }
        wlan[_]: <default("Ieee80211Nic")> like IWirelessNic {
            parameters:
                @display("p=122.256004,530.908,row,60;q=queue");
        }
        eth[_]: <ethernetType> like IWiredNic {
            parameters:
                @display("p=187.912,530.908,row,60;q=txQueue");
        }
        ppp[_]: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=250.172,530.908,row,60;q=txQueue");
        }
        ext[_]: <default("ExtInterface")> like IExternalNic {
            parameters:
                @display("p=312.432,530.908;q=txQueue;i=block/ifcard;is=s");
        }
        tun[_]: <default("TunInterface")> like ITunNic {
            parameters:
                @display("p=367.9,530.908;q=txQueue;i=block/ifcard;is=s");
        }

        ospf: <"OSPFRouting"> like IIPRouting if hasOSPF {
            parameters:
                @display("p=330.544,304.508;i=block/network2;is=s");
        }
        bgp: <"BGPRouting"> like IBGPRouting if hasBGP {
            parameters:
                ospfRoutingModule = default(hasOSPF ? "^.ospf" : "");
                @display("p=122.256004,114.332;i=block/network;is=s");
        }
        tcp: <tcpType> like ITCP if hasBGP && tcpType != "" {
            parameters:
                @display("p=122.256004,216.212;i=block/transport;is=s");
        }
        rip: <"RIPRouting"> like IUDPApp if hasRIP {
            parameters:
                @display("p=241.116,114.332;is=s");
        }

        babel: <"BabelRouting"> like IUDPApp if hasBABEL {
            parameters:
                @display("p=286.396,114.332;is=s");
        }

        hsrp: <"HSRP"> like IUDPApp if hasHSRP {
            parameters:
                @display("p=350.92,114.332;i=block/routing");
        }

        glbp: <"GLBP"> like IUDPApp if hasGLBP {
            parameters:
                @display("p=419.97202,114.332;i=block/routing");
        }

        vrrp: <"VRRPv2"> like IIPRouting if hasVRRP {
            parameters:
                @display("p=37.356,304.508;i=block/routing");
        }

        udp: <udpType> like IUDP if (hasRIP || hasBABEL || hasHSRP || hasGLBP || hasLISP) {
            parameters:
                @display("p=268.284,216.212");
        }

        pim: <"PIMRouting"> like IPIMRouting if hasPIM {
            parameters:
                @display("p=110.936005,304.508;i=block/network2;is=s");
        }

        eigrp: EigrpProcessDS if hasEIGRP {
            parameters:
                @display("p=268.284,304.508;is=s");
        }

        lisp: LISPRouting if hasLISP {
            parameters:
                @display("p=187.912,114.332;is=s");
        }

        isis: ISIS if hasISIS {
            parameters:
                @display("p=393.936,304.508;is=s");
        }

    connections allowunconnected:
        // connections to network outside

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> { @display("m=s"); } <--> eth[i].phys;
            eth[i].upperLayerOut --> networkLayer.ifIn++;
            eth[i].upperLayerIn <-- networkLayer.ifOut++;
        }

        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> { @display("m=s"); } <--> ppp[i].phys;
            ppp[i].upperLayerOut --> networkLayer.ifIn++;
            ppp[i].upperLayerIn <-- networkLayer.ifOut++;
        }

        for i=0..sizeof(radioIn)-1 {
            radioIn[i] --> { @display("m=s"); } --> wlan[i].radioIn;
            wlan[i].upperLayerOut --> networkLayer.ifIn++;
            wlan[i].upperLayerIn <-- networkLayer.ifOut++;
        }

        for i=0..numLoopbacks-1 {
            networkLayer.ifOut++ --> lo[i].upperLayerIn;
            lo[i].upperLayerOut --> networkLayer.ifIn++;
        }

        for i=0..numExtInterfaces-1 {
            ext[i].upperLayerOut --> networkLayer.ifIn++;
            ext[i].upperLayerIn <-- networkLayer.ifOut++;
        }

        for i=0..numTunInterfaces-1 {
            tun[i].upperLayerOut --> networkLayer.ifIn++;
            tun[i].upperLayerIn <-- networkLayer.ifOut++;
        }
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

            udp.ipOut --> networkLayer.transportIn++;
            udp.ipIn <-- networkLayer.transportOut++;

            networkLayer.transportOut++ --> lisp.ipv4In if enableIPv4;
            networkLayer.transportIn++ <-- lisp.ipv4Out if enableIPv4;
            networkLayer.transportOut++ --> lisp.ipv6In if enableIPv6;
            networkLayer.transportIn++ <-- lisp.ipv6Out if enableIPv6;
        }
}
