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
import ansa.linklayer.ethernet.ANSA_EthernetInterface;

module ANSA_NodeBase
{
    parameters:
        @display("i=abstract/server;bgb=868,545");
        @networkNode;
        @labels(node,ethernet-node,wireless-node);
        bool hasStatus = default(false);

        bool enableIPv4 = default(true);
        bool enableIPv6 = default(true);
        bool enableCLNS = default(true);

        *.enableIPv4 = enableIPv4;
        *.enableIPv6 = enableIPv6;
        *.enableCLNS = enableCLNS;

        int numExtInterfaces = default(0);
        int numRadios = default(0);        // the number of radios in the router. by default no wireless
        int numPcapRecorders = default(0); // no of PcapRecorders.
        int numTunInterfaces = default(0);
        int numLoopbacks = default(1);
        string mobilityType = default(numRadios > 0 ? "StationaryMobility" : "");

        string networkLayerType = default("ANSA_MultiNetworkLayer");
        string routingTableType = default("ANSA_MultiRoutingTable");
        string ethernetType = default(firstAvailableOrEmpty("EthernetInterface","AnsaEthernetInterface"));

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

        xml configData = default(xmldoc("config.xml"));
        networkLayer.configurator.configData = configData;
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
                @display("p=424,358;q=queue");
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
            @display("p=192,496");
        }
        wlan[numRadios]: <default("Ieee80211Nic")> like IWirelessNic {
            parameters:
                @display("p=313,496,row,60;q=queue");
        }
        eth[sizeof(ethg)]: <ethernetType> like IWiredNic {
            parameters:
                @display("p=424,496,row,60;q=txQueue");
        }
        ppp[sizeof(pppg)]: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=531,496,row,60;q=txQueue");
        }
        ext[numExtInterfaces]: <default("ExtInterface")> like IExternalNic {
            parameters:
                @display("p=666,496,row,10;q=txQueue;i=block/ifcard");
        }
        tun[numTunInterfaces]: <default("TunInterface")> like ITunNic {
            parameters:
                @display("p=802,496,row,10;q=txQueue;i=block/ifcard");
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
