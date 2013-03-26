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
// @file RIPngRouter.msg
// @author Jiri Trhlik (mailto:), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
// @brief
// @detail

package inet.ansa.applications.ripng;

import inet.ansa.util.InterfaceStateManager.InterfaceStateManager;
import inet.ansa.applications.ripng.RIPngRouting;
import inet.ansa.util.deviceConfigurator.DeviceConfigurator;
import inet.nodes.ipv6.NetworkLayer6;
import inet.networklayer.common.InterfaceTable;
import inet.ansa.networklayer.ipv6.ANSARoutingTable6;
import inet.base.NotificationBoard;
import inet.transport.udp.UDP;
import inet.linklayer.ethernet.EthernetInterface;
import inet.linklayer.ppp.PPPInterface;
import inet.linklayer.IWiredNic;

module RIPngRouter
{
    parameters:
        @display("i=abstract/router");
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

        deviceConfigurator: DeviceConfigurator {
            @display("p=53,113");
            deviceId = deviceId;
            deviceType = "Router";
            configFile = configFile;
        }
        
        interfaceStateManager: InterfaceStateManager {
            @display("p=53,188");
        }

        notificationBoard: NotificationBoard {
            parameters:
                @display("p=53,38");
        }

        // applications layer
        RIPng: RIPngRouting {
            @display("p=204,113");
            configFile = configFile;
            deviceId = deviceId;
            deviceType = "Router";
        }

        // transort layer
        udp: UDP {
            parameters:
                @display("p=290,172;i=block/transport");
            gates:
                appIn[1];
                appOut[1];
        }

        // network layer
        networkLayer: NetworkLayer6 {
            parameters:
                @display("p=204,240;q=queue");
        }

        routingTable6: ANSARoutingTable6 {
            parameters:
                @display("p=290,38");
                isRouter = IPForward;
                routingTable = routingTable;
        }

        // linklayer
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=204,38");
        }


        eth[sizeof(ethg)]: <default("EthernetInterface")> like IWiredNic {
            parameters:
                @display("p=127,317,row,90;q=txQueue");
        }
        ppp[sizeof(pppg)]: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=290,317,row,90;q=txQueue");
        }

    connections allowunconnected:
        RIPng.udpOut --> udp.appIn[0];
        RIPng.udpIn <-- udp.appOut[0];

        udp.ipv6Out --> networkLayer.udpIn;
        udp.ipv6In <-- networkLayer.udpOut;

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].upperLayerOut --> networkLayer.ifIn++;
            eth[i].upperLayerIn <-- networkLayer.ifOut++;
        }

        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].upperLayerOut --> networkLayer.ifIn++;
            ppp[i].upperLayerIn <-- networkLayer.ifOut++;
        }
}
