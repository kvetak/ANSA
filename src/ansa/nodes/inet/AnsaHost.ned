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
// @file AnsaHost.ned
// @date 21.10.2011
// @author: Marek Cerny, Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
// @brief File implements Host


package inet.ansa.nodes.inet;
import inet.ansa.util.deviceConfigurator.DeviceConfigurator;
import inet.applications.IUDPApp;
import inet.linklayer.ethernet.EthernetInterface;
import inet.ansa.util.InterfaceStateManager.InterfaceStateManager;
import inet.ansa.util.IPGen.AnsaIPTrafGen;
import inet.ansa.networklayer.ipSplitter.IpSplitter;
import inet.networklayer.common.InterfaceTable;
import inet.ansa.networklayer.ipv4.AnsaRoutingTable;

import inet.applications.pingapp.PingApp;
import inet.base.NotificationBoard;
import inet.linklayer.ppp.PPPInterface;
import inet.nodes.inet.NetworkLayer;
import inet.nodes.ipv6.NetworkLayer6;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.networklayer.ipv6.RoutingTable6;
import inet.transport.tcp.TCP;
import inet.applications.IUDPApp;
import inet.transport.IUDP;
import inet.transport.udp.UDP;


module AnsaHost
{
    parameters:
        @node();

        string deviceId = default("");
        string configFile = default("");
        xml routingTable = default(xml("<routingTable/>"));  // Routing table is filled by deviceConfigurator
        bool IPForward = default(true);

		int numUdpApps = default(0);
        bool legacyForwardIPv4 = default(false);
        string legacyConfigFileIPv4 = default("");

        @display("i=device/server;bgb=484,396");
    gates:
        inout pppg[];
        inout ethg[];
    submodules:
        deviceConfigurator: DeviceConfigurator {
            parameters:
                deviceId = deviceId;
                deviceType = "Host";
                configFile = configFile;
                @display("p=83,117");
        }
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=83,35");
        }
        interfaceStateManager: InterfaceStateManager {
            parameters:
                @display("p=83,212");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=294,35");
        }
        routingTable: AnsaRoutingTable {
            parameters:
                routerId = deviceId;
                IPForward = true;
                configFile = configFile;
                @display("p=395,35");
        }
        tcp: TCP {
            parameters:
                @display("p=294,117");
        }
        udp: UDP {
            parameters:
                @display("p=395,117");
        }
        ipTrafGen: AnsaIPTrafGen {
            parameters:
                @display("p=395,342;i=block/circle");
        }
        pingApp4: PingApp {
            parameters:
                @display("p=213,117");
        }
        networkLayer: NetworkLayer {
            parameters:
                @display("q=queue;p=213,212");
            gates:
                ifIn[sizeof(pppg)+sizeof(ethg)];
                ifOut[sizeof(pppg)+sizeof(ethg)];
        }
        ppp[sizeof(pppg)]: PPPInterface {
            parameters:
                @display("p=143,342,row,110;q=l2queue");
        }
        eth[sizeof(ethg)]: EthernetInterface {
            parameters:
                @display("p=284,342,row,110;q=l2queue");
        }
        routingTable6: RoutingTable6 {
            @display("p=197,35");
            isRouter = IPForward;
            routingTable = routingTable;
        }
        udpApp[numUdpApps]: <> like IUDPApp {
            parameters:
                @display("p=395,212,row");
        }

    connections allowunconnected:

        tcp.ipOut --> networkLayer.tcpIn;
        tcp.ipIn <-- networkLayer.tcpOut;

        udp.ipOut --> networkLayer.udpIn;
        udp.ipIn <-- networkLayer.udpOut;
        ipTrafGen.ipOut --> networkLayer.rsvpIn;
        ipTrafGen.ipIn <-- networkLayer.rsvpOut;

        pingApp4.pingOut --> networkLayer.pingIn++;
        pingApp4.pingIn <-- networkLayer.pingOut++;

        for i=0..numUdpApps-1 {
            udpApp[i].udpOut --> udp.appIn++;
            udpApp[i].udpIn <-- udp.appOut++;
        }
        // connections to network outside
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].upperLayerOut --> networkLayer.ifIn++;
            ppp[i].upperLayerIn <-- networkLayer.ifOut++;
        }

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].upperLayerOut --> networkLayer.ifIn++;
            eth[i].upperLayerIn <-- networkLayer.ifOut++;
        }
}
