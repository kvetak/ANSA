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
// @file AnsaPIMRouter.ned
// @date 25.1.2013
// @author: Tomas Prochazka (mailto:xproch21@stud.fit.vutbr.cz), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
// @brief File implements compbound module AnsaPIMRouter
// @details AnsaPIMRouter is router with PIM module, fow now only with PIM-DM implementation


package inet.ansa.networklayer.pim;

import inet.ansa.util.InterfaceStateManager.InterfaceStateManager;
import inet.ansa.networklayer.pim.pim;
import inet.ansa.util.deviceConfigurator.DeviceConfigurator;
import inet.ansa.nodes.inet.ansaNetworkLayer;
import inet.networklayer.common.InterfaceTable;
import inet.ansa.networklayer.ipv4.AnsaRoutingTable;
//import inet.networklayer.ipv6.RoutingTable6;
import inet.base.NotificationBoard;
import inet.linklayer.ethernet.EthernetInterface;
import inet.linklayer.ppp.PPPInterface;
import inet.linklayer.IWiredNic;

module PIMRouter
{
    parameters:
        @display("i=abstract/router");
        @node;
        @labels(node,ethernet-node);
        xml routingTable = default(xml("<routingTable/>"));  // Routing table is filled by deviceConfigurator
        bool IPForward = default(true);
        bool multicastForward = default(true);
        string configFile = default("config.xml");           // file name of the XML file which contains configuration for router
        string deviceId;                                     // needed for proper load of device XML configuration
        string hostname = default("Router");
    gates:
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);
    submodules:

        deviceConfigurator: DeviceConfigurator {
            @display("p=80,113");
            deviceId = deviceId;
            deviceType = "Router";
            configFile = configFile;
        }

        notificationBoard: NotificationBoard {
            parameters:
                @display("p=80,38");
        }

        // applications layer
        PIM: pim {
            parameters:
                @display("p=204,113");
                configFile = configFile;
                deviceType = "Router";
                deviceId = deviceId;
                hostname = hostname;
        }

        // network layer
        networkLayer: ansaNetworkLayer {
            parameters:
                @display("p=204,240;q=queue");
        }    
        		
        routingTable: AnsaRoutingTable {
            parameters:
                @display("p=305,38");
                IPForward = true;
                routerId = deviceId;
                configFile = configFile;
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

        interfaceStateManager: InterfaceStateManager {
            @display("p=80,210");
        }
//        routingTable6: RoutingTable6 {
//            @display("p=304,110");
//            isRouter = IPForward;
//           	routingTable = routingTable;
//        }
    connections allowunconnected:
        PIM.networkLayerOut --> networkLayer.pimIn;
        PIM.networkLayerIn <-- networkLayer.pimOut;


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
