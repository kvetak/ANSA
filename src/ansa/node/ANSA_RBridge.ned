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

///**
// * @file RBridge.ned
// * @author Marcel Marek (mailto:imarek@fit.vutbr.cz), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
// * @date 1.3.2013
// * @brief
// * @detail
// * @todo 
// */
package ansa.node;

//import ansa.util.deviceConfigurator.DeviceConfigurator;
//import inet.ansa.linklayer.switch.stp.Stp;
//import inet.nodes.inet.BurstHost;
//import inet.base.NotificationBoard;
import inet.linklayer.ethernet.EtherMAC;
//import inet.ansa.linklayer.ethernet.AnsaEtherMAC;
//import inet.linklayer.IWiredNic;
//import inet.ansa.linklayer.switch.ANSASwitchCore;
import ansa.linklayer.rbridge.RBMACTable;
import ansa.linklayer.rbridge.RBridgeSplitter;

//import ansa.linklayer.switch.sink;
import ansa.linklayer.rbridge.RBVLANTable;
//import inet.ansa.linklayer.rbridge.RBPortTable;

import ansa.linklayer.rbridge.RBridgeSplitter;
//import inet.ansa.linklayer.rbridge.RBEthernetInterface;
import inet.linklayer.ethernet.EthernetInterface;


//import inet.transport.rtp.RTPAVProfilePayload10Receiver;
//import inet.ansa.linklayer.switch.PortTable;
//import inet.ansa.acl.acl;
//import inet.nodes.inet.NetworkLayer;
import inet.networklayer.contract.INetworkLayer; 
//import inet.ansa.TrafficGenerator.TrafGen;
//import inet.ansa.rip.RIPRouting;
//import inet.transport.udp.UDP;
//import inet.networklayer.ospfv2.OSPFRouting;
//import inet.ansa.InterfaceStateManager.InterfaceStateManager;
import inet.networklayer.common.InterfaceTable;
//import inet.ansa.routingTable.AnsaInterfaceTable;
//import inet.ansa.routingTable.AnsaRoutingTable;

import ansa.networklayer.clns.CLNSTable;
import ansa.networklayer.isis.ISIS;

import ansa.linklayer.rbridge.TRILL;
module RBridge
{
    parameters:
        @node();
        @display("i=abstract/switch;bgb=589,469,white");
        xml configFile = default(xmldoc("config.xml"));
        string hostname = default("");
        string deviceId = default("");
        string deviceType = "RBridge";
        xml configData = default(xmldoc("config.xml"));
        
        bool hasISIS = true;
        bool hasTRILL = true;
        
        isis.configData = configData;
        isis.deviceId = deviceId;
        isis.deviceType = deviceType;
        isis.configFile = configFile;
        
        trill.configData = configData;
        trill.deviceId = deviceId;
        trill.deviceType = deviceType;
        trill.configFile = configFile;
        
        
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        
    gates:
        inout ethg[];
        //inout pppg[];

    submodules:
//        deviceConfigurator: DeviceConfigurator {
//            parameters:
//                deviceId = deviceId;
//                deviceType = deviceType;
//                configFile = configFile;
//                @display("p=403,118");
//        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=527,44");
        }
        trill: TRILL {
            parameters:
                @display("p=243,173;is=l");
            gates:
                lowerLayerIn[sizeof(ethg)];
                lowerLayerOut[sizeof(ethg)];

        }
        //        mac[sizeof(ethg)]: AnsaEtherMAC {
        //            parameters:
        //                @display("p=154,416,row,110;q=l2queue;is=l");
        //                promiscuous = true;
        //        }
//        eth[sizeof(ethg)]: <default("EthernetInterface")> like IWiredNic {
        eth[sizeof(ethg)]: EthernetInterface {
            parameters:
                @display("p=154,417,row,110;q=l2queue");
                encapType="EtherEncapDummy";
        }
        rBMACTable: RBMACTable {
            @display("p=53,44;is=l");
        }
        rBVLANTable: RBVLANTable {
            parameters:
                configFile = configFile;
                rBridgeId = deviceId;
                portCount = sizeof(ethg);
                @display("p=165,44;is=l");
        }
//        rBPortTable: RBPortTable {
//            parameters:
//                @display("p=527,118");
//        }

        clnsTable: CLNSTable {
            parameters:
                @display("p=277,44;is=l");

        }
        isis: ISIS {
            parameters:

                @display("p=53,173;is=l");
            gates:
//                lowerLayerIn[sizeof(pppg)+sizeof(ethg)];
//                lowerLayerOut[sizeof(pppg)+sizeof(ethg)];
                trillIn;
                trillOut;
        }
        rbridgeSplitter: RBridgeSplitter {
            parameters:
                @display("p=154,295;is=l");
            gates:
                ifIn[sizeof(ethg)];
                ifOut[sizeof(ethg)];
                isisIn[sizeof(ethg)];
                isisOut[sizeof(ethg)];
                trillIn[sizeof(ethg)];
                trillOut[sizeof(ethg)];

        }
    connections allowunconnected:

        for i=0..sizeof(ethg)-1 {
            //            ethg[i] <--> mac[i].phys;
            ethg[i] <--> eth[i].phys;
            //            mac[i].upperLayerOut --> rbridgeSplitter.lowerLayerIn[i];
            //            mac[i].upperLayerIn <-- rbridgeSplitter.lowerLayerOut[i];
            
            
//            eth[i].upperLayerOut --> rbridgeSplitter.lowerLayerIn[i];
//            eth[i].upperLayerIn <-- rbridgeSplitter.lowerLayerOut[i];
            
            eth[i].upperLayerOut --> rbridgeSplitter.ifIn[i];
            eth[i].upperLayerIn <-- rbridgeSplitter.ifOut[i];
            
            
//            rbridgeSplitter.isisOut[i] --> isis.lowerLayerIn[i];
//            rbridgeSplitter.isisIn[i] <-- isis.lowerLayerOut[i];

			
            rbridgeSplitter.trillOut[i] --> trill.lowerLayerIn[i];
            rbridgeSplitter.trillIn[i] <-- trill.lowerLayerOut[i];

            


        }
        isis.lowerLayerOut --> trill.isisIn;
            isis.lowerLayerIn <-- trill.isisOut;


//        trill.toSink --> sink.ifIn;
}
