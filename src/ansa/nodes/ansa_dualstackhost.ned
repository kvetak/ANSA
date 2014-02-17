//
// Copyright (C) 2013 Brno University of Technology
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
//@author Vladimir Vesely (<a href="mailto:ivesely@fit.vutbr.cz">ivesely@fit.vutbr.cz</a>)

package inet.ansa.nodes;

import inet.ansa.networklayer.ipSplitter.IpSplitter;
import inet.applications.IPingApp;
import inet.applications.ISCTPApp;
import inet.applications.ITCPApp;
import inet.applications.IUDPApp;
import inet.transport.ISCTP;
import inet.transport.ITCP;
import inet.transport.IUDP;
import inet.networklayer.ipv4.RoutingTable;
import inet.networklayer.ipv6.RoutingTable6;
import inet.nodes.inet.NetworkLayer;
import inet.nodes.ipv6.NetworkLayer6;
import inet.networklayer.common.InterfaceTable;
import inet.mobility.IMobility;
import inet.linklayer.IWirelessNic;
import inet.linklayer.IWiredNic;
import inet.linklayer.IExternalNic;
import inet.base.NotificationBoard;
import inet.util.PcapRecorder;
import inet.ansa.util.InterfaceStateManager.InterfaceStateManager;
import inet.ansa.util.deviceConfigurator.DeviceConfigurator;


module ANSA_DualStackHost
{
    parameters:
        @display("i=device/pc;bgb=701,607");

        int numTcpApps = default(0);  // no of TCP apps. Specify the app types in INI file with tcpApp[0..1].typename="TCPEchoApp" syntax
        int numUdpApps = default(0);  // no of UDP apps. Specify the app types in INI file with udpApp[0..1].typename="UDPVideoStreamCli" syntax
        int numSctpApps = default(0); // no of SCTP apps. Specify the app types in INI file with sctpApp[0..1].typename="SCTPServer" syntax
        int numPingApps = default(0);  // no of PING apps. Specify the app types in INI file with pingApp[0..1].typename="PingApp" syntax

        bool hasTcp = default(numTcpApps>0);
        bool hasUdp = default(numUdpApps>0);
        bool hasSctp = default(numSctpApps>0);
        string tcpType = default(firstAvailable("TCP", "TCP_lwIP", "TCP_NSC", "TCP_None"));  // tcp implementation (e.g. ~TCP, ~TCP_lwIP, ~TCP_NSC) or ~TCPSpoof
        string udpType = default(firstAvailable("UDP","UDP_None"));
        string sctpType = default(firstAvailable("SCTP","SCTP_None"));

        bool IPForward = default(false);  // disable routing by default
        networkLayer.proxyARP = default(false);
        bool forwardMulticast = default(false);

        int numExtInterfaces = default(0);
        int numRadios = default(0);               // the number of radios in the router. by default no wireless
        int numPcapRecorders = default(0); // no of PcapRecorders.

        string mobilityType = default(numRadios > 0 ? "StationaryMobility" : "");
        string routingFile = default("");
		xml routingTable = default(xml("<routingTable/>"));
		
        int IGMPVersion = default(2);
        
        string configFile = default("config.xml");           // file name of the XML file which contains configuration for the router
        string deviceId;
        
    gates:

        input radioIn[numRadios] @directIn;
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);

    submodules:

        tcpApp[numTcpApps]: <> like ITCPApp {
            parameters:
                @display("p=222,132,row,60");
        }
        tcp: <tcpType> like ITCP if hasTcp {
            parameters:
                @display("p=222,225");
        }
        udpApp[numUdpApps]: <> like IUDPApp {
            parameters:
                @display("p=357,132,row,60");
        }
        udp: <udpType> like IUDP if hasUdp {
            parameters:
                @display("p=357,225");
        }
        sctpApp[numSctpApps]: <> like ISCTPApp {
            parameters:
                @display("p=473,132,row,60");
        }
        sctp: <sctpType> like ISCTP if hasSctp {
            @display("p=473,225");
        }
        pingApp[numPingApps]: <default("PingApp")> like IPingApp {
            parameters:
                @display("p=596,225,row,60");
        }

        notificationBoard: NotificationBoard {
            parameters:
                @display("p=72,44;is=n");
        }

        interfaceStateManager: InterfaceStateManager {
            parameters:
                @display("p=75,125");
        }

        deviceConfigurator: DeviceConfigurator {
            parameters:
                @display("p=75,208");
                deviceId = deviceId;
                deviceType = "Host";
                configFile = configFile;
        }

        // optional mobility module. Required only if wireless cards are present
        mobility: <mobilityType> like IMobility if mobilityType != "" {
            parameters:
                @display("p=75,374;is=n");
        }

        // network layer
        networkLayer: NetworkLayer {
            parameters:
                @display("p=290,348;q=queue");
            gates:
                ifIn[sizeof(ethg)+sizeof(pppg)+sizeof(radioIn)];
                ifOut[sizeof(ethg)+sizeof(pppg)+sizeof(radioIn)];
        }

        routingTable: RoutingTable {
            parameters:
                @display("p=518,44;is=n");
                IPForward = IPForward;
                forwardMulticast = forwardMulticast;
                routingFile = routingFile;
        }

        networkLayer6: NetworkLayer6 {
            parameters:
                @display("p=512,348;q=queue");
            gates:
                ifIn[sizeof(ethg)+sizeof(pppg)+sizeof(radioIn)];
                ifOut[sizeof(ethg)+sizeof(pppg)+sizeof(radioIn)];
        }

        routingTable6: RoutingTable6 {
            parameters:
                @display("p=627,44");
                isRouter = IPForward;
                routingTable = routingTable;
        }

        // linklayer
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=357,44;is=n");
        }

        pcapRecorder[numPcapRecorders]: PcapRecorder {
            @display("p=75,287,r,10;is=n;i=abstract/cache");
        }

        wlan[numRadios]: <default("Ieee80211Nic")> like IWirelessNic {
            parameters:
                @display("p=208,542,row,60;q=queue");
        }
        eth[sizeof(ethg)]: <default("EthernetInterface")> like IWiredNic {
            parameters:
                @display("p=329,542,row,60;q=txQueue");
        }
        ppp[sizeof(pppg)]: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=473,542,row,60;q=txQueue");
        }
        ext[numExtInterfaces]: <default("ExtInterface")> like IExternalNic {
            parameters:
                @display("p=589,542,row,10;q=txQueue;i=block/ifcard");
        }

        ipSplitter: IpSplitter {
            parameters:
                @display("p=402,455");
            gates:
                ifIn[sizeof(ethg)+sizeof(pppg)+sizeof(radioIn)];
                ifOut[sizeof(ethg)+sizeof(pppg)+sizeof(radioIn)];
                ipv4In[sizeof(ethg)+sizeof(pppg)+sizeof(radioIn)];
                ipv4Out[sizeof(ethg)+sizeof(pppg)+sizeof(radioIn)];
                ipv6In[sizeof(ethg)+sizeof(pppg)+sizeof(radioIn)];
                ipv6Out[sizeof(ethg)+sizeof(pppg)+sizeof(radioIn)];
                isisIn[sizeof(ethg)+sizeof(pppg)+sizeof(radioIn)];
                isisOut[sizeof(ethg)+sizeof(pppg)+sizeof(radioIn)];

        }
    connections allowunconnected:

        // connections to network outside
        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].upperLayerOut --> ipSplitter.ifIn[i];
            eth[i].upperLayerIn <-- ipSplitter.ifOut[i];
            ipSplitter.ipv6Out[i] --> networkLayer6.ifIn[i];
            ipSplitter.ipv6In[i] <-- networkLayer6.ifOut[i];
            ipSplitter.ipv4Out[i] --> networkLayer.ifIn[i];
            ipSplitter.ipv4In[i] <-- networkLayer.ifOut[i];
        }
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].upperLayerOut --> ipSplitter.ifIn[sizeof(ethg)+i];
            ppp[i].upperLayerIn <-- ipSplitter.ifOut[sizeof(ethg)+i];
            ipSplitter.ipv6Out[sizeof(ethg)+i] --> networkLayer6.ifIn[sizeof(ethg)+i];
            ipSplitter.ipv6In[sizeof(ethg)+i] <-- networkLayer6.ifOut[sizeof(ethg)+i];
            ipSplitter.ipv4Out[sizeof(ethg)+i] --> networkLayer.ifIn[sizeof(ethg)+i];
            ipSplitter.ipv4In[sizeof(ethg)+i] <-- networkLayer.ifOut[sizeof(ethg)+i];
        }
        for i=0..sizeof(radioIn)-1 {
            radioIn[i] --> wlan[i].radioIn;
            wlan[i].upperLayerOut --> ipSplitter.ifIn[sizeof(ethg)+sizeof(pppg)+i];
            wlan[i].upperLayerIn <-- ipSplitter.ifOut[sizeof(ethg)+sizeof(pppg)+i];
            ipSplitter.ipv6Out[sizeof(ethg)+sizeof(pppg)+i] --> networkLayer6.ifIn[sizeof(ethg)+sizeof(pppg)+i];
            ipSplitter.ipv6In[sizeof(ethg)+sizeof(pppg)+i] <-- networkLayer6.ifOut[sizeof(ethg)+sizeof(pppg)+i];
            ipSplitter.ipv4Out[sizeof(ethg)+sizeof(pppg)+i] --> networkLayer.ifIn[sizeof(ethg)+sizeof(pppg)+i];
            ipSplitter.ipv4In[sizeof(ethg)+sizeof(pppg)+i] <-- networkLayer.ifOut[sizeof(ethg)+sizeof(pppg)+i];
        }


		//TCP
        tcp.ipOut --> networkLayer.tcpIn if hasTcp;
        tcp.ipIn <-- networkLayer.tcpOut if hasTcp;
        tcp.ipv6Out --> networkLayer6.tcpIn if hasTcp;
        tcp.ipv6In <-- networkLayer6.tcpOut if hasTcp;        
        for i=0..numTcpApps-1 {
            tcpApp[i].tcpOut --> tcp.appIn++;
            tcpApp[i].tcpIn <-- tcp.appOut++;
        }
        //UDP
        udp.ipOut --> networkLayer.udpIn if hasUdp;
        udp.ipIn <-- networkLayer.udpOut if hasUdp;
		udp.ipv6Out --> networkLayer6.udpIn if hasUdp;
        udp.ipv6In <-- networkLayer6.udpOut if hasUdp;
        for i=0..numUdpApps-1 {
            udpApp[i].udpOut --> udp.appIn++;
            udpApp[i].udpIn <-- udp.appOut++;
        }
        //SCTP
        sctp.to_ip --> networkLayer.sctpIn if hasSctp;
        networkLayer.sctpOut --> sctp.from_ip if hasSctp;
        sctp.to_ipv6 --> networkLayer6.sctpIn if hasSctp;
        networkLayer6.sctpOut --> sctp.from_ipv6 if hasSctp;        
        for i=0..numSctpApps-1 {
            sctpApp[i].sctpOut --> sctp.from_appl++;
            sctp.to_appl++ --> sctpApp[i].sctpIn;
        }
		//PING 		
        for i=0..numPingApps-1 {
            networkLayer.pingOut++ --> pingApp[i].pingIn;
            networkLayer.pingIn++ <-- pingApp[i].pingOut;
            networkLayer6.pingOut++ --> pingApp[i].pingv6In;
            networkLayer6.pingIn++ <-- pingApp[i].pingv6Out;
        }

}
