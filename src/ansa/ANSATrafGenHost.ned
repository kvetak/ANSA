//
// Copyright (C) 2010 Martin Danko
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


package inet.ansa;

import inet.ansa.TrafficGenerator.TrafGen;
import inet.ansa.TrafficGenerator.DscpTtlCorrector;
import inet.applications.pingapp.PingApp;
import inet.base.NotificationBoard;
import inet.linklayer.ethernet.EthernetInterface;
import inet.linklayer.ppp.PPPInterface;
import inet.nodes.inet.NetworkLayer;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.transport.tcp.TCP;
import inet.transport.udp.UDP;
import inet.util.NAMTraceWriter;


//
// \IP Traffic generator
//
module ANSATrafGenHost
{
    parameters:
        @node();
        bool IPForward = default(false);
        int namid = default(-1);
        string routingFile = default("");
        @display("i=device/server");
    gates:
        inout pppg[];
        inout ethg[];
    submodules:
        namTrace: NAMTraceWriter {
            parameters:
                namid = namid;
                @display("p=60,310");
        }
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=60,70");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=60,150");
        }
        routingTable: RoutingTable {
            parameters:
                IPForward = IPForward;
                routerId = "";
                routingFile = routingFile;
                @display("p=60,230");
        }
        tcp: TCP {
            parameters:
                @display("p=163,154");
        }
        udp: UDP {
            parameters:
                @display("p=272,154");
        }
        trafgen: TrafGen {
            parameters:
                @display("p=219,85");
        }
        pingApp: PingApp {
            parameters:
                @display("p=343,200");
        }
        networkLayer: NetworkLayer {
            parameters:
                proxyARP = false;
                @display("p=219,240;q=queue");
            gates:
                ifIn[sizeof(pppg)+sizeof(ethg)];
                ifOut[sizeof(pppg)+sizeof(ethg)];
        }
        ppp[sizeof(pppg)]: PPPInterface {
            parameters:
                @display("p=205,350,row,90;q=txQueue");
        }
        eth[sizeof(ethg)]: EthernetInterface {
            parameters:
                @display("p=219,370,row,90;q=txQueue");
        }
        dtc: DscpTtlCorrector {
            parameters:
                @display("p=219,300,row;i=block/tx");
            gates:
                toNetworkLayerIn[sizeof(pppg)+sizeof(ethg)];
                toNetworkLayerOut[sizeof(pppg)+sizeof(ethg)];
                ifIn[sizeof(pppg)+sizeof(ethg)];
                ifOut[sizeof(pppg)+sizeof(ethg)];
        }
    connections allowunconnected:

        trafgen.tcpOut --> tcp.appIn++;
        trafgen.tcpIn <-- tcp.appOut++;

        tcp.ipOut --> networkLayer.tcpIn;
        tcp.ipIn <-- networkLayer.tcpOut;

        trafgen.udpOut --> udp.appIn++;
        trafgen.udpIn <-- udp.appOut++;

        udp.ipOut --> networkLayer.udpIn;
        udp.ipIn <-- networkLayer.udpOut;

        networkLayer.pingOut --> pingApp.pingIn;
        networkLayer.pingIn <-- pingApp.pingOut;

        // connections to network outside
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].netwOut --> dtc.ifIn[i];
            dtc.toNetworkLayerOut[i] --> networkLayer.ifIn[i];
            ppp[i].netwIn <-- dtc.ifOut[i];
            dtc.toNetworkLayerIn[i] <-- networkLayer.ifOut[i];
        }

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].netwOut --> dtc.ifIn[sizeof(pppg)+i];
            dtc.toNetworkLayerOut[sizeof(pppg)+i] --> networkLayer.ifIn[sizeof(pppg)+i];
            eth[i].netwIn <-- dtc.ifOut[sizeof(pppg)+i];
            dtc.toNetworkLayerIn[sizeof(pppg)+i] <-- networkLayer.ifOut[sizeof(pppg)+i];
        }
}

