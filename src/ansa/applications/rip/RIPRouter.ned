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
// @file RIPRouter.ned
// @author Jiri Trhlik (mailto:jiritm@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
// @brief Simple RIP router
// @detail RIP router module for testing purposes 

package inet.ansa.applications.rip;

import inet.ansa.networklayer.ipSplitter.IpSplitter;
import inet.ansa.util.InterfaceStateManager.InterfaceStateManager;
import inet.ansa.applications.rip.RIPRouting;
//import inet.ansa.applications.rip.RIPDeviceConfigurator;
import inet.nodes.inet.NetworkLayer;
import inet.networklayer.common.InterfaceTable;
import inet.ansa.networklayer.ipv4.AnsaRoutingTable;
import inet.base.NotificationBoard;
import inet.ansa.transport.udp.ANSAUDP;
import inet.linklayer.ethernet.EthernetInterface;
import inet.linklayer.ppp.PPPInterface;
import inet.linklayer.IWiredNic;

module RIPRouter
{
    parameters:
        @display("i=abstract/router;bgb=402,421");
        @node;
        @labels(node,ethernet-node);
        xml routingTable = default(xml("<routingTable/>"));  // Routing table is filled by deviceConfigurator
        bool IPForward = default(true);
        string configFile = default("config.xml");           // file name of the XML file which contains configuration for the router
        string deviceId;                                     // needed for proper load of the device XML configuration
    gates:
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);
    submodules:

        interfaceStateManager: InterfaceStateManager {
            @display("p=53,188");
        }

        notificationBoard: NotificationBoard {
            parameters:
                @display("p=53,38");
        }

        // applications layer
        RIP: RIPRouting {
            @display("p=204,113");
            configFile = configFile;
            deviceId = deviceId;
            deviceType = "Router";
        }

        // transort layer
        udp: ANSAUDP {
            parameters:
                @display("p=290,172;i=block/transport");
            gates:
                appIn[1];
                appOut[1];
        }

        // network layer
        networkLayer: NetworkLayer {
            parameters:
                @display("p=204,221;q=queue");
            gates:
                ifIn[sizeof(ethg)+sizeof(pppg)];
                ifOut[sizeof(ethg)+sizeof(pppg)];
        }

        routingTable: AnsaRoutingTable {
            parameters:
                @display("p=290,38");
                routingTable = routingTable;
        }

        // linklayer
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=204,38");
        }

        ipSplitter: IpSplitter {
            parameters:
                @display("p=204,304");
            gates:
                ifIn[sizeof(ethg)+sizeof(pppg)];
                ifOut[sizeof(ethg)+sizeof(pppg)];
                ipv4In[sizeof(ethg)+sizeof(pppg)];
                ipv4Out[sizeof(ethg)+sizeof(pppg)];
                ipv6In[sizeof(ethg)+sizeof(pppg)];
                ipv6Out[sizeof(ethg)+sizeof(pppg)];
                isisIn[sizeof(ethg)+sizeof(pppg)];
                isisOut[sizeof(ethg)+sizeof(pppg)];
        }

        eth[sizeof(ethg)]: <default("EthernetInterface")> like IWiredNic {
            parameters:
                @display("p=127,361,row,90;q=txQueue");
        }
        ppp[sizeof(pppg)]: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=290,361,row,90;q=txQueue");
        }

    connections allowunconnected:
        RIP.udpOut --> udp.appIn[0];
        RIP.udpIn <-- udp.appOut[0];

        udp.ipOut --> networkLayer.udpIn;
        udp.ipIn <-- networkLayer.udpOut;

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].upperLayerOut --> ipSplitter.ifIn[i];
            eth[i].upperLayerIn <-- ipSplitter.ifOut[i];
            ipSplitter.ipv4Out[i] --> networkLayer.ifIn[i];
            ipSplitter.ipv4In[i] <-- networkLayer.ifOut[i];
        }

        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].upperLayerOut --> ipSplitter.ifIn[sizeof(ethg)+i];
            ppp[i].upperLayerIn <-- ipSplitter.ifOut[sizeof(ethg)+i];
            ipSplitter.ipv4Out[sizeof(ethg)+i] --> networkLayer.ifIn[sizeof(ethg)+i];
            ipSplitter.ipv4In[sizeof(ethg)+i] <-- networkLayer.ifOut[sizeof(ethg)+i];
        }
}
