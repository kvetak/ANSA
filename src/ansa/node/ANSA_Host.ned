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

import inet.applications.contract.IPingApp;
import inet.applications.contract.ISCTPApp;
import inet.applications.contract.ITCPApp;
import inet.applications.contract.IUDPApp;
import inet.transportlayer.contract.ISCTP;
import inet.transportlayer.contract.ITCP;
import inet.transportlayer.contract.IUDP;
import inet.applications.contract.ITunApp;

import ansa.node.ANSA_NodeBase;

module ANSA_Host extends ANSA_NodeBase
{
    parameters:
        int numTcpApps = default(0);  // no of TCP apps. Specify the app types in INI file with tcpApp[0..1].typename="TCPEchoApp" syntax
        int numUdpApps = default(0);  // no of UDP apps. Specify the app types in INI file with udpApp[0..1].typename="UDPVideoStreamCli" syntax
        int numSctpApps = default(0); // no of SCTP apps. Specify the app types in INI file with sctpApp[0..1].typename="SCTPServer" syntax
        int numPingApps = default(0);  // no of PING apps. Specify the app types in INI file with pingApp[0..1].typename="PingApp" syntax
        bool hasTcp = default(numTcpApps > 0);
        bool hasUdp = default(numUdpApps > 0);
        bool hasSctp = default(numSctpApps > 0);
        bool hasTun = default(numTunInterfaces > 0);
        string tcpType = default(firstAvailableOrEmpty("TCP", "TCP_lwIP", "TCP_NSC"));  // tcp implementation (e.g. ~TCP, ~TCP_lwIP, ~TCP_NSC) or ~TCPSpoof
        string udpType = default(firstAvailableOrEmpty("UDP"));
        string sctpType = default(firstAvailableOrEmpty("SCTP"));

        forwarding = default(false);
        multicastForwarding = default(false);
        networkLayer.proxyARP = default(false);
		networkLayer.enableCDP = false;
		networkLayer.enableLLDP = false;        
        routingTable.forwarding = forwarding;
        routingTable.multicastForwarding = multicastForwarding;
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(routingTableType != "" ? absPath(".routingTable") : "");
        *.energySourceModule = default(energyStorageType != "" ? absPath(".energyStorage") : "");
        *.mobilityModule = default(mobilityType != "" ? absPath(".mobility") : "");                

        @display("i=device/pc;bgb=,-");
    submodules:

        tunApp[numTunInterfaces]: <> like ITunApp {
            parameters:
                @display("p=805,54");
        }
        tcpApp[numTcpApps]: <> like ITCPApp {
            parameters:
                @display("p=201,50,row,60");
        }
        tcp: <tcpType> like ITCP if hasTcp {
            parameters:
                @display("p=201,138");
        }
        udpApp[numUdpApps]: <> like IUDPApp {
            parameters:
                @display("p=345,54,row,60");
        }
        udp: <udpType> like IUDP if hasUdp {
            parameters:
                @display("p=345,142");
        }
        sctpApp[numSctpApps]: <> like ISCTPApp {
            parameters:
                @display("p=500,54,row,60");
        }
        sctp: <sctpType> like ISCTP if hasSctp {
            @display("p=500,142");
        }
        pingApp[numPingApps]: <default("PingApp")> like IPingApp {
            parameters:
                @display("p=653,142,row,60");
        }

    connections allowunconnected:
        // connections to network outside
        for i=0..numTcpApps-1 {
            tcpApp[i].tcpOut --> tcp.appIn++;
            tcpApp[i].tcpIn <-- tcp.appOut++;
        }

        tcp.ipOut --> networkLayer.transportIn++ if hasTcp;
        tcp.ipIn <-- networkLayer.transportOut++ if hasTcp;

        for i=0..numUdpApps-1 {
            udpApp[i].udpOut --> udp.appIn++;
            udpApp[i].udpIn <-- udp.appOut++;
        }

        udp.ipOut --> networkLayer.transportIn++ if hasUdp;
        udp.ipIn <-- networkLayer.transportOut++ if hasUdp;

        for i=0..numSctpApps-1 {
            sctpApp[i].sctpOut --> sctp.from_appl++;
            sctp.to_appl++ --> sctpApp[i].sctpIn;
        }
        sctp.to_ip --> networkLayer.transportIn++ if hasSctp;
        networkLayer.transportOut++ --> sctp.from_ip if hasSctp;

        for i=0..numPingApps-1 {
            networkLayer.pingOut++ --> pingApp[i].pingIn;
            networkLayer.pingIn++ <-- pingApp[i].pingOut;
        }

        for i=0..numTunInterfaces-1 {
            tunApp[i].tunOut --> tun[i].appIn if hasTun;
            tunApp[i].tunIn <-- tun[i].appOut if hasTun;
        }
}


