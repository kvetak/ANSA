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

import inet.ansa.acl.acl;
import inet.ansa.deviceConfigurator.DeviceConfigurator;
import inet.ansa.ethernet.AnsaEthernetInterface;
import inet.ansa.InterfaceStateManager.InterfaceStateManager;
import inet.ansa.IPGen.AnsaIPTrafGen;
import inet.ansa.ipv6.AnsaRoutingTable6;
import inet.ansa.ipSplitter.IpSplitter;
import inet.ansa.ospfv2.AnsaOSPFRouting;
import inet.ansa.ospfv3.AnsaOspfRouting6;
import inet.ansa.ppp.PPPInterface;
import inet.ansa.rip.RIPRouting;
import inet.ansa.routingTable.AnsaInterfaceTable;
import inet.ansa.routingTable.AnsaRoutingTable;
import inet.ansa.AnsaNetworkLayer;
import inet.ansa.pim.pim;
import inet.ansa.isis.ISIS;

import inet.applications.pingapp.PingApp;
import inet.base.NotificationBoard;
import inet.networklayer.ipv6.RoutingTable6;
import inet.nodes.inet.NetworkLayer;
import inet.nodes.ipv6.NetworkLayer6;
import inet.transport.udp.UDP;

module AnsaDualStackRouter
{
    parameters:
        @node();

        string hostname = default("");
        string deviceId;
        string configFile;

        @display("i=srouter;bgb=437,396");
    gates:
        inout pppg[];
        inout ethg[];
    submodules:
        deviceConfigurator: DeviceConfigurator {
            parameters:
                deviceId = deviceId;
                deviceType = "Router";
                configFile = configFile;
                @display("p=395,342");
        }
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=45,42");
        }
        interfaceStateManager: InterfaceStateManager {
            parameters:
                @display("p=154,42");
        }
        interfaceTable: AnsaInterfaceTable {
            parameters:
                @display("p=250,42");
        }
        routingTable: AnsaRoutingTable {
            parameters:
                routerId = deviceId;
                IPForward = true;
                configFile = configFile;
                @display("p=323,42");
        }
        routingTable6: AnsaRoutingTable6 {
            parameters:
                isRouter = true;
                @display("p=395,42");
        }
        rip: RIPRouting {
            parameters:
                hostname = hostname;
                configFile = configFile;
                @display("p=104,110,row;i=block/network2");
        }
        udp: UDP {
            parameters:
                @display("p=211,110;i=block/transport");
            gates:
                appIn[1];
                appOut[1];
        }
        ospf: AnsaOSPFRouting {
            parameters:
                configFile = configFile;
                @display("p=34,188,row");
        }
        pim: pim {

            @display("p=34,265");
        }
        ospf6: AnsaOspfRouting6 {
            parameters:
                deviceId = deviceId;
                deviceType = "Router";
                configFile = configFile;
                @display("p=395,188,row");
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
        acl: acl {
            parameters:
                configFile = configFile;
                @display("p=104,286,row;i=block/layer");
            gates:
                toNetworkLayerIn[sizeof(pppg)+sizeof(ethg)];
                toNetworkLayerOut[sizeof(pppg)+sizeof(ethg)];
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
                isisIn[sizeof(pppg)+sizeof(ethg)];
                isisOut[sizeof(pppg)+sizeof(ethg)];
        }
        ppp[sizeof(pppg)]: PPPInterface {
            parameters:
                @display("p=143,342,row,110;q=l2queue");
        }
        eth[sizeof(ethg)]: AnsaEthernetInterface {
            parameters:
                @display("p=284,342,row,110;q=l2queue");
        }

        isis: ISIS {
            parameters:
                deviceId = deviceId;
                deviceType = "Router";
                configFile = configFile;
                @display("p=333,286");
            gates:
                ifIn[sizeof(pppg)+sizeof(ethg)];
                ifOut[sizeof(pppg)+sizeof(ethg)];
        }
    connections allowunconnected:
        ospf.ipOut --> networkLayer.ospfIn;
        ospf.ipIn <-- networkLayer.ospfOut;

        ospf6.ipv6Out --> networkLayer6.ospfIn;
        ospf6.ipv6In <-- networkLayer6.ospfOut;

        pim.networkLayerOut --> networkLayer.pimIn;
        pim.networkLayerIn <-- networkLayer.pimOut;

        rip.udpOut --> udp.appIn[0];
        rip.udpIn <-- udp.appOut[0];

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

        pingApp6.pingOut --> networkLayer6.pingIn;
        pingApp6.pingIn <-- networkLayer6.pingOut;
		

        // connections to network outside
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;

            ppp[i].netwOut --> ipSplitter.ifIn[i];
            ppp[i].netwIn <-- ipSplitter.ifOut[i];

            ipSplitter.ipv4Out[i] --> acl.ifIn[i];
            ipSplitter.ipv4In[i] <-- acl.ifOut[i];

            acl.toNetworkLayerOut[i] --> networkLayer.ifIn[i];
            acl.toNetworkLayerIn[i] <-- networkLayer.ifOut[i];

            ipSplitter.ipv6Out[i] --> networkLayer6.ifIn[i];
            ipSplitter.ipv6In[i] <-- networkLayer6.ifOut[i];
            
            ipSplitter.isisOut[i] --> isis.ifIn[i];
        	ipSplitter.isisIn[i] <-- isis.ifOut[i];
        }

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;

            eth[i].netwOut --> ipSplitter.ifIn[sizeof(pppg)+i];
            eth[i].netwIn <-- ipSplitter.ifOut[sizeof(pppg)+i];

            ipSplitter.ipv4Out[sizeof(pppg)+i] --> acl.ifIn[sizeof(pppg)+i];
            ipSplitter.ipv4In[sizeof(pppg)+i] <-- acl.ifOut[sizeof(pppg)+i];

            acl.toNetworkLayerOut[sizeof(pppg)+i] --> networkLayer.ifIn[sizeof(pppg)+i];
            acl.toNetworkLayerIn[sizeof(pppg)+i] <-- networkLayer.ifOut[sizeof(pppg)+i];

            ipSplitter.ipv6Out[sizeof(pppg)+i] --> networkLayer6.ifIn[i];
            ipSplitter.ipv6In[sizeof(pppg)+i] <-- networkLayer6.ifOut[i];
            
            ipSplitter.isisOut[i] --> isis.ifIn[i];
        	ipSplitter.isisIn[i] <-- isis.ifOut[i];
        }
}


