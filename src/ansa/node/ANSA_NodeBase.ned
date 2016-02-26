//Based on:
//@see /src/inet/node/inet/NodeBase
//
// The MIT License (MIT)
//
// Copyright (c) 2016 Brno University of Technology
//
//@author Vladimir Vesely (iveselyATfitDOTvutbrDOTcz)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

package ansa.node;

import inet.common.lifecycle.NodeStatus;
import inet.common.packet.PcapRecorder;
import inet.linklayer.contract.IExternalNic;
import inet.linklayer.contract.ITunNic;
import inet.linklayer.contract.IWiredNic;
import inet.linklayer.contract.IWirelessNic;
import inet.linklayer.loopback.LoopbackInterface;
import inet.mobility.contract.IMobility;
import inet.networklayer.contract.IRoutingTable;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.contract.INetworkLayer;
import inet.power.contract.IEnergyStorage;
import inet.power.contract.IEnergyGenerator;

import ansa.networklayer.multi.ANSA_MultiNetworkLayer;

module ANSA_NodeBase
{
    parameters:
        @display("i=abstract/server");
        @networkNode;
        @labels(node,ethernet-node,wireless-node);
        bool hasStatus = default(false);
        int numExtInterfaces = default(0);
        int numRadios = default(0);               // the number of radios in the router. by default no wireless
        int numPcapRecorders = default(0); // no of PcapRecorders.
        int numTunInterfaces = default(0);
        int numLoopbacks = default(1);
        string mobilityType = default(numRadios > 0 ? "StationaryMobility" : "");
        
        string networkLayerType = default("ANSA_MultiNetworkLayer");
        string routingTableType = default("ANSA_MultiRoutingTable");
        
        bool forwarding = default(true);
        bool multicastForwarding = default(false);
        string energyStorageType = default("");
        string energyGeneratorType = default("");
        
        routingTable.forwarding = forwarding;        
        routingTable.multicastForwarding = multicastForwarding;   // for IPv4, IPv6, Generic
        
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(routingTableType != "" ? absPath(".routingTable") : "");
        *.energySourceModule = default(energyStorageType != "" ? absPath(".energyStorage") : "");
        *.mobilityModule = default(mobilityType != "" ? absPath(".mobility") : "");

    gates:
        input radioIn[numRadios] @directIn;
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);

    submodules:
        status: NodeStatus if hasStatus {
            @display("p=50,50");
        }
        energyStorage: <energyStorageType> like IEnergyStorage if energyStorageType != "" {
            parameters:
                @display("p=50,100;i=block/plug;is=s");
        }
        energyGenerator: <energyGeneratorType> like IEnergyGenerator if energyGeneratorType != "" {
            parameters:
                @display("p=50,150;i=block/plug;is=s");
        }
        // optional mobility module. Required only if wireless cards are present
        mobility: <mobilityType> like IMobility if mobilityType != "" {
            parameters:
                @display("p=53,200");
        }
        // network layer
        networkLayer: ANSA_MultiNetworkLayer {
            parameters:
                @display("p=427,281;q=queue");
        }
        // routing table
        routingTable: <routingTableType> like IRoutingTable if routingTableType != "" {
            parameters:
                @display("p=53,250;is=s");
        }
        // linklayer
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=53,300;is=s");
        }
        pcapRecorder[numPcapRecorders]: PcapRecorder {
            @display("p=53,350,r,10");
        }
        lo[numLoopbacks]: LoopbackInterface {
            @display("p=195,419");
        }
        wlan[numRadios]: <default("Ieee80211Nic")> like IWirelessNic {
            parameters:
                @display("p=316,419,row,60;q=queue");
        }
        eth[sizeof(ethg)]: <default("EthernetInterface")> like IWiredNic {
            parameters:
                @display("p=427,419,row,60;q=txQueue");
        }
        ppp[sizeof(pppg)]: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=534,419,row,60;q=txQueue");
        }
        ext[numExtInterfaces]: <default("ExtInterface")> like IExternalNic {
            parameters:
                @display("p=669,419,row,10;q=txQueue;i=block/ifcard");
        }
        tun[numTunInterfaces]: <default("TunInterface")> like ITunNic {
            parameters:
                @display("p=805,419,row,10;q=txQueue;i=block/ifcard");
        }

    connections allowunconnected:
        // connections to network outside

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> { @display("m=s"); } <--> eth[i].phys;
            eth[i].upperLayerOut --> networkLayer.ifIn++;
            eth[i].upperLayerIn <-- networkLayer.ifOut++;
        }

        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> { @display("m=s"); } <--> ppp[i].phys;
            ppp[i].upperLayerOut --> networkLayer.ifIn++;
            ppp[i].upperLayerIn <-- networkLayer.ifOut++;
        }

        for i=0..sizeof(radioIn)-1 {
            radioIn[i] --> { @display("m=s"); } --> wlan[i].radioIn;
            wlan[i].upperLayerOut --> networkLayer.ifIn++;
            wlan[i].upperLayerIn <-- networkLayer.ifOut++;
        }

        for i=0..numLoopbacks-1 {
            networkLayer.ifOut++ --> lo[i].upperLayerIn;
            lo[i].upperLayerOut --> networkLayer.ifIn++;
        }

        for i=0..numExtInterfaces-1 {
            ext[i].upperLayerOut --> networkLayer.ifIn++;
            ext[i].upperLayerIn <-- networkLayer.ifOut++;
        }

        for i=0..numTunInterfaces-1 {
            tun[i].upperLayerOut --> networkLayer.ifIn++;
            tun[i].upperLayerIn <-- networkLayer.ifOut++;
        }

}
