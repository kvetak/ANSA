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
// Author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
// Babel Router module - router supports Babel routing protocol

package inet.ansa.applications.babel;


import inet.ansa.acl.acl;
import inet.ansa.applications.babel.BabelRouting;
import inet.ansa.applications.babel.BabelMain;
import inet.ansa.networklayer.ipSplitter.IpSplitter;
import inet.ansa.networklayer.ipv6.ANSARoutingTable6;
import inet.ansa.util.InterfaceStateManager.InterfaceStateManager;
import inet.ansa.applications.babel.BabelDeviceConfigurator;
import inet.applications.IPingApp;
import inet.applications.pingapp.PingApp;
import inet.transport.IUDP;
import inet.util.PcapRecorder;
import inet.ansa.networklayer.ipv4.AnsaRoutingTable;
import inet.networklayer.common.InterfaceTable;
import inet.mobility.IMobility;
import inet.linklayer.IWirelessNic;
import inet.linklayer.IWiredNic;
import inet.linklayer.IExternalNic;
import inet.base.NotificationBoard;
import inet.nodes.inet.NetworkLayer;
import inet.nodes.ipv6.NetworkLayer6;
import inet.ansa.nodes.inet.ansaNetworkLayer;

module BabelRouter
{
    parameters:
        @display("i=abstract/router;bgb=395,637,white");
        @node;
        @labels(node,ethernet-node,wireless-node);

        int numUdpApps = default(1);  // 1 UDPapp for Babel
        int numPingApps = default(1);
        bool hasUdp = default(numUdpApps>0);
        string udpType = default(firstAvailable("UDP","UDP_None"));

        int numExtInterfaces = default(0);
        int numRadios = default(0);               // the number of radios in the router. by default no wireless
        int numPcapRecorders = default(0); // no of PcapRecorders.

        string mobilityType = default(numRadios > 0 ? "StationaryMobility" : "");

        bool IPForward = default(true);
        bool forwardMulticast = default(false);
        int IGMPVersion = default(2);

        string configFile = default("config.xml");           // file name of the XML file which contains configuration for the router
        string deviceId;
        xml routingTable = default(xml("<routingTable/>"));  // Routing table is filled by deviceConfigurator

    gates:

        input radioIn[numRadios] @directIn @labels(AirFrame);
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);

    submodules:

        notificationBoard: NotificationBoard {
            parameters:
                @display("p=73,46;is=n");
        }
        udp: <udpType> like IUDP if hasUdp {
            parameters:
                @display("p=190,302");
        }

        babel: BabelRouting {
            parameters:
                @display("p=73,214");
        }
             
        networkLayer: ansaNetworkLayer {
            parameters:
                @display("p=73,393;q=queue");
            gates:
                ifIn[sizeof(ethg)+sizeof(pppg)+sizeof(radioIn)];
                ifOut[sizeof(ethg)+sizeof(pppg)+sizeof(radioIn)];
        }
        networkLayer6: NetworkLayer6 {
            parameters:
                @display("p=298,393");
            gates:
                ifIn[sizeof(ethg)+sizeof(pppg)+sizeof(radioIn)];
                ifOut[sizeof(ethg)+sizeof(pppg)+sizeof(radioIn)];
        }

        //routing tables
        routingTable: AnsaRoutingTable {
            parameters:
                @display("p=73,129");
                routingTable = routingTable;
        }
        routingTable6: ANSARoutingTable6 {
            parameters:
                @display("p=298,129");
                isRouter = IPForward;
                routingTable = routingTable;
        }

        pcapRecorder[numPcapRecorders]: PcapRecorder {
            parameters:
                @display("p=298,214,r,10;i=abstract/cache;is=n");
        }


        interfaceStateManager: InterfaceStateManager {
            parameters:
                @display("p=298,46");
        }

        babelDeviceConfigurator: BabelDeviceConfigurator {
            parameters:
                @display("p=189,46");
                deviceId = deviceId;
                deviceType = "Router";
                configFile = configFile;
        }

        ipSplitter: IpSplitter {
            parameters:
                @display("p=190,489");
            gates:
                ifIn[sizeof(ethg)+sizeof(pppg)+sizeof(radioIn)];
                ifOut[sizeof(ethg)+sizeof(pppg)+sizeof(radioIn)];
                ipv4In[sizeof(ethg)+sizeof(pppg)+sizeof(radioIn)];
                ipv4Out[sizeof(ethg)+sizeof(pppg)+sizeof(radioIn)];
                ipv6In[sizeof(ethg)+sizeof(pppg)+sizeof(radioIn)];
                ipv6Out[sizeof(ethg)+sizeof(pppg)+sizeof(radioIn)];
                isisIn[sizeof(ethg)+sizeof(pppg)+sizeof(radioIn)];
                isisOut[sizeof(ethg)+sizeof(pppg)+sizeof(radioIn)];
        }

        // linklayer
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=189,129;is=n");
        }
        eth[sizeof(ethg)]: <default("EthernetInterface")> like IWiredNic {
            parameters:
                @display("p=189,574,row,60;q=txQueue");
        }
        ppp[sizeof(pppg)]: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=298,574,row,60;q=txQueue");
        }
        wlan[numRadios]: <default("Ieee80211Nic")> like IWirelessNic {
            parameters:
                @display("p=73,574,row,60;q=queue");
        }

    //Connections between submodules    
    connections allowunconnected:

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].upperLayerOut --> ipSplitter.ifIn[i];
            eth[i].upperLayerIn <-- ipSplitter.ifOut[i];
            ipSplitter.ipv6Out[i] --> networkLayer6.ifIn[i];
            ipSplitter.ipv6In[i] <-- networkLayer6.ifOut[i];
            ipSplitter.ipv4Out[i] --> networkLayer.ifIn[i];
            ipSplitter.ipv4In[i] <-- networkLayer.ifOut[i];
        }
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].upperLayerOut --> ipSplitter.ifIn[sizeof(ethg)+i];
            ppp[i].upperLayerIn <-- ipSplitter.ifOut[sizeof(ethg)+i];
            ipSplitter.ipv6Out[sizeof(ethg)+i] --> networkLayer6.ifIn[sizeof(ethg)+i];
            ipSplitter.ipv6In[sizeof(ethg)+i] <-- networkLayer6.ifOut[sizeof(ethg)+i];
            ipSplitter.ipv4Out[sizeof(ethg)+i] --> networkLayer.ifIn[sizeof(ethg)+i];
            ipSplitter.ipv4In[sizeof(ethg)+i] <-- networkLayer.ifOut[sizeof(ethg)+i];

        }
        for i=0..sizeof(radioIn)-1 {
            radioIn[i] --> wlan[i].radioIn;
            wlan[i].upperLayerOut --> ipSplitter.ifIn[sizeof(ethg)+sizeof(pppg)+i];
            wlan[i].upperLayerIn <-- ipSplitter.ifOut[sizeof(ethg)+sizeof(pppg)+i];
            ipSplitter.ipv6Out[sizeof(ethg)+sizeof(pppg)+i] --> networkLayer6.ifIn[sizeof(ethg)+sizeof(pppg)+i];
            ipSplitter.ipv6In[sizeof(ethg)+sizeof(pppg)+i] <-- networkLayer6.ifOut[sizeof(ethg)+sizeof(pppg)+i];
            ipSplitter.ipv4Out[sizeof(ethg)+sizeof(pppg)+i] --> networkLayer.ifIn[sizeof(ethg)+sizeof(pppg)+i];
            ipSplitter.ipv4In[sizeof(ethg)+sizeof(pppg)+i] <-- networkLayer.ifOut[sizeof(ethg)+sizeof(pppg)+i];
        }

        networkLayer.udpOut --> udp.ipIn;
        udp.ipOut --> networkLayer.udpIn;

        udp.ipv6Out --> networkLayer6.udpIn;
        networkLayer6.udpOut --> udp.ipv6In;

        udp.appOut++ --> babel.udpIn;
        babel.udpOut --> udp.appIn++;
}
