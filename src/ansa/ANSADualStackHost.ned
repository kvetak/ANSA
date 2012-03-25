//
// Marek Cerny, 2MSK
// FIT VUT 2011
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

import inet.ansa.deviceConfigurator.DeviceConfigurator;
import inet.ansa.ethernet.AnsaEthernetInterface;
import inet.ansa.InterfaceStateManager.InterfaceStateManager;
import inet.ansa.IPGen.AnsaIPTrafGen;
import inet.ansa.ipSplitter.IpSplitter;
import inet.ansa.ipv6.AnsaRoutingTable6;
import inet.ansa.routingTable.AnsaInterfaceTable;
import inet.ansa.routingTable.AnsaRoutingTable;

import inet.applications.pingapp.PingApp;
import inet.base.NotificationBoard;
import inet.linklayer.ppp.PPPInterface;
//import inet.nodes.inet.NetworkLayer;
import inet.ansa.AnsaNetworkLayer;
import inet.nodes.ipv6.NetworkLayer6;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.networklayer.ipv6.RoutingTable6;
import inet.transport.tcp.TCP;
import inet.transport.udp.UDP;
import inet.util.NAMTraceWriter;


module AnsaDualStackHost
{
    parameters:
        @node();

        string deviceId = default("");
        string configFile = default("");

        bool legacyForwardIPv4 = default(false);
        string legacyConfigFileIPv4 = default("");

        @display("i=device/server;bgb=437,396");
    gates:
        inout pppg[];
        inout ethg[];
    submodules:
        deviceConfigurator: DeviceConfigurator {
            parameters:
                deviceId = deviceId;
                deviceType = "Host";
                configFile = configFile;
                @display("p=395,342");
        }
        namTrace: NAMTraceWriter {
            parameters:
                @display("p=395,286");
        }
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=45,42");
        }
        interfaceStateManager: InterfaceStateManager {
            parameters:
                @display("p=154,42");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=252,42");
        }
        routingTable: RoutingTable {
            parameters:
                routerId = "";
                IPForward = legacyForwardIPv4;
                routingFile = legacyConfigFileIPv4;
                @display("p=323,42");
        }
        routingTable6: AnsaRoutingTable6 {
            parameters:
                isRouter = false;
                @display("p=395,42");
        }
        tcp: TCP {
            parameters:
                @display("p=172,110");
        }
        udp: UDP {
            parameters:
                @display("p=252,110");
        }
        ipTrafGen: AnsaIPTrafGen {
            parameters:
                @display("p=211,188;i=block/circle");
        }
        pingApp4: PingApp {
            parameters:
                @display("p=34,110");
        }
        pingApp6: PingApp {
            parameters:
                @display("p=395,110");
        }
        networkLayer6: NetworkLayer6 {
            parameters:
                @display("p=323,188");
            gates:
                ifIn[sizeof(pppg)+sizeof(ethg)];
                ifOut[sizeof(pppg)+sizeof(ethg)];
        }
        networkLayer: AnsaNetworkLayer {
            parameters:
                @display("p=104,188;q=queue");
            gates:
                ifIn[sizeof(pppg)+sizeof(ethg)];
                ifOut[sizeof(pppg)+sizeof(ethg)];
        }
        ipSplitter: IpSplitter {
            parameters:
                @display("p=211,286");
            gates:
                ifIn[sizeof(pppg)+sizeof(ethg)];
                ifOut[sizeof(pppg)+sizeof(ethg)];
                ipv4In[sizeof(pppg)+sizeof(ethg)];
                ipv4Out[sizeof(pppg)+sizeof(ethg)];
                ipv6In[sizeof(pppg)+sizeof(ethg)];
                ipv6Out[sizeof(pppg)+sizeof(ethg)];
        }
        ppp[sizeof(pppg)]: PPPInterface {
            parameters:
                @display("p=143,342,row,110;q=l2queue");
        }
        eth[sizeof(ethg)]: AnsaEthernetInterface {
            parameters:
                @display("p=284,342,row,110;q=l2queue");
        }
    connections allowunconnected:

        tcp.ipOut --> networkLayer.tcpIn;
        tcp.ipIn <-- networkLayer.TCPOut;

        tcp.ipv6Out --> networkLayer6.tcpIn;
        tcp.ipv6In <-- networkLayer6.tcpOut;

        udp.ipOut --> networkLayer.udpIn;
        udp.ipIn <-- networkLayer.udpOut;

        udp.ipv6Out --> networkLayer6.udpIn;
        udp.ipv6In <-- networkLayer6.udpOut;

        ipTrafGen.ipOut --> networkLayer.rsvpIn;
        ipTrafGen.ipIn <-- networkLayer.rsvpOut;

        ipTrafGen.ipv6Out --> networkLayer6.rsvpIn;
        ipTrafGen.ipv6In <-- networkLayer6.rsvpOut;

        pingApp4.pingOut --> networkLayer.pingIn;
        pingApp4.pingIn <-- networkLayer.pingOut;

        pingApp6.pingv6Out --> networkLayer6.pingIn;
        pingApp6.pingv6In <-- networkLayer6.pingOut;

        // connections to network outside
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;

            ppp[i].netwOut --> ipSplitter.ifIn[i];
            ppp[i].netwIn <-- ipSplitter.ifOut[i];

            ipSplitter.ipv4Out[i] --> networkLayer.ifIn[i];
            ipSplitter.ipv4In[i] <-- networkLayer.ifOut[i];

            ipSplitter.ipv6Out[i] --> networkLayer6.ifIn[i];
            ipSplitter.ipv6In[i] <-- networkLayer6.ifOut[i];
        }

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;

            eth[i].netwOut --> ipSplitter.ifIn[sizeof(pppg)+i];
            eth[i].netwIn <-- ipSplitter.ifOut[sizeof(pppg)+i];

            ipSplitter.ipv4Out[sizeof(pppg)+i] --> networkLayer.ifIn[sizeof(pppg)+i];
            ipSplitter.ipv4In[sizeof(pppg)+i] <-- networkLayer.ifOut[sizeof(pppg)+i];

            ipSplitter.ipv6Out[sizeof(pppg)+i] --> networkLayer6.ifIn[i];
            ipSplitter.ipv6In[sizeof(pppg)+i] <-- networkLayer6.ifOut[i];
        }
}

