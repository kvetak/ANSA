// Author: Matej Hrncirik
// FIT VUT 2012
//
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

package inet.ansa.networklayer.isis;

import inet.ansa.networklayer.clns.CLNSTable;
//import inet.ansa.acl.acl;
import inet.ansa.util.deviceConfigurator.DeviceConfigurator;
//import inet.ansa.ethernet.AnsaEthernetInterface;
import inet.linklayer.ethernet.EthernetInterface;
import inet.linklayer.IWiredNic;
import inet.ansa.util.InterfaceStateManager.InterfaceStateManager;
//import inet.ansa.IPGen.AnsaIPTrafGen;
//import inet.ansa.ipv6.AnsaRoutingTable6;
import inet.ansa.networklayer.ipSplitter.IpSplitter;

//import inet.ansa.ppp.PPPInterface;
import inet.linklayer.ppp.PPPInterface;
//import inet.ansa.routingTable.AnsaInterfaceTable;
import inet.networklayer.common.InterfaceTable;
//import inet.ansa.routingTable.AnsaRoutingTable; //it has configFile parameter
import inet.networklayer.ipv4.RoutingTable;

//import inet.ansa.AnsaNetworkLayer;

//import inet.ansa.pim.pim;
//import inet.ansa.networklayer.isis.ISIS;

import inet.applications.pingapp.PingApp;
import inet.base.NotificationBoard;
import inet.networklayer.ipv6.RoutingTable6;
import inet.nodes.inet.NetworkLayer;
import inet.nodes.ipv6.NetworkLayer6;

module AnsaISISRouter
{
    parameters:
        @node();

        string hostname = default("");
        string deviceId;
        string configFile;
        xml routingTable = default(xml("<routingTable/>"));  // Routing table is filled by deviceConfigurator
        bool IPForward = default(true);
//        xml table = default(xml("<clnsTable/>"));


        @display("i=srouter;bgb=437,396,white");
    gates:
        inout pppg[];
        inout ethg[];
    submodules:
        clnsTable: CLNSTable {
            parameters:
                @display("p=356,115");

        }
        deviceConfigurator: DeviceConfigurator {
            parameters:
                deviceId = deviceId;
                deviceType = "Router";
                configFile = configFile;
                @display("p=362,42");
        }
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=66,42");
        }
        interfaceStateManager: InterfaceStateManager {
            parameters:
                @display("p=211,42");
        }
        //        interfaceTable: AnsaInterfaceTable {
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=51,115");
        }
        //        routingTable: RoutingTable {
        //            routingTable: RoutingTable {
        //            parameters:
        //                routerId = deviceId;
        //                IPForward = true;
        //                configFile = configFile;
        //                @display("p=323,42");
        //        }
        routingTable: RoutingTable {
            parameters:
                @display("p=154,115");
                IPForward = true;
                routerId = deviceId;

        }
        //        routingTable6: AnsaRoutingTable6 {
        //            parameters:
        //                isRouter = true;
        //                @display("p=395,42");
        //        }
        routingTable6: RoutingTable6 {
            parameters:
                @display("p=257,115");
                isRouter = IPForward;
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
        //        eth[sizeof(ethg)]: AnsaEthernetInterface {
        eth[sizeof(ethg)]: <default("EthernetInterface")> like IWiredNic {
            parameters:
                @display("p=284,342,row,110;q=l2queue");
        }

        isis: ISIS {
            parameters:
                deviceId = deviceId;
                deviceType = "Router";
                configFile = configFile;
                @display("p=98,196");
            gates:
                lowerLayerIn[sizeof(pppg)+sizeof(ethg)];
                lowerLayerOut[sizeof(pppg)+sizeof(ethg)];
        }
    connections allowunconnected:
        // connections to network outside
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;

            //            ppp[i].netwOut --> ipSplitter.ifIn[i];
            ppp[i].upperLayerOut --> ipSplitter.ifIn[i];
            //            ppp[i].netwIn <-- ipSplitter.ifOut[i];
            ppp[i].upperLayerIn <-- ipSplitter.ifOut[i];

            ipSplitter.isisOut[i] --> isis.lowerLayerIn[i];
            ipSplitter.isisIn[i] <-- isis.lowerLayerOut[i];
        }

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;

            //            eth[i].netwOut --> ipSplitter.ifIn[sizeof(pppg)+i];
            eth[i].upperLayerOut --> ipSplitter.ifIn[sizeof(pppg)+i];
            //            eth[i].netwIn <-- ipSplitter.ifOut[sizeof(pppg)+i];
            eth[i].upperLayerIn <-- ipSplitter.ifOut[sizeof(pppg)+i];

            ipSplitter.isisOut[i] --> isis.lowerLayerIn[i];
            ipSplitter.isisIn[i] <-- isis.lowerLayerOut[i];
        }
}


