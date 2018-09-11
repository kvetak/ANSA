//
// Copyright (C) 2005 Andras Babos
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//               2010 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// @author Tomas Rajca
// @author Vladimir Vesely (ivesely@fit.vutbr.cz)
// @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3

package ansa.node;

import ansa.linklayer.cdp.CDP;
import ansa.linklayer.contract.ANSA_IMACRelayUnit;
import ansa.linklayer.lldp.LLDP;
import inet.common.MessageDispatcher;
import inet.common.lifecycle.NodeStatus;
import inet.linklayer.contract.IEthernetInterface;
import inet.linklayer.contract.IMacAddressTable;
import inet.networklayer.common.InterfaceTable;



//
// Model of an Ethernet switch.
//
// The duplexChannel attributes of the MACs must be set according to the
// medium connected to the port; if collisions are possible (it's a bus or hub)
// it must be set to false, otherwise it can be set to true.
// By default used half duples CSMA/CD mac
//
//


//import inet.common.MessageDispatcher;
//import inet.common.lifecycle.NodeStatus;
import inet.linklayer.configurator.L2NodeConfigurator;
//import inet.linklayer.contract.IEthernetInterface;
//import inet.linklayer.contract.IMacAddressTable;
//import inet.linklayer.contract.IMacRelayUnit;
import inet.linklayer.contract.ISpanningTree;
import inet.linklayer.ethernet.IEtherEncap;
//import inet.networklayer.common.InterfaceTable;

//
// Model of an Ethernet switch.
//
// The duplexChannel attributes of the MACs must be set according to the
// medium connected to the port; if collisions are possible (it's a bus or hub)
// it must be set to false, otherwise it can be set to true.
// By default used half duples CSMA/CD mac
//
//
module ANSA_EtherSwitch
{
    parameters:
        @networkNode();
        @labels(node,ethernet-node);
        @display("i=device/switch");
        bool hasStatus = default(false);
        bool hasCDP = default(false);
        bool hasLLDP = default(false);
        bool hasStp = default(false);
        bool csmacdSupport = default(true);  // by default use CSMA/CD --FIXME remove
        string fcsMode @enum("declared","computed") = default("declared");
        string spanningTreeProtocol = default("Stp");
        eth[*].encap.typename = "EtherEncapDummy";
        eth[*].csmacdSupport = csmacdSupport;
        eth[*].mac.typename = default(csmacdSupport ? "EtherMac" : "EtherMacFullDuplex");
        *.fcsMode = fcsMode;
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.macTableModule = default(absPath(".macTable"));
        
        relayUnit.hasStp = hasStp;
        relayUnit.hasCDP = hasCDP;
        relayUnit.hasLLDP = hasLLDP;

    gates:
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
             
        status: NodeStatus if hasStatus {
            @display("p=100,400;is=s");
        }
        l2NodeConfigurator: L2NodeConfigurator if hasStp {
            @display("p=100,300;is=s");
        }
        interfaceTable: InterfaceTable {
            @display("p=100,200;is=s");
        }
        macTable: <default("MacAddressTable")> like IMacAddressTable {
            @display("p=100,100;is=s");
        }
        relayUnit: <default(firstAvailable("ANSA_RelayUnit", "Ieee8021dRelay","MacRelayUnit"))> like ANSA_IMACRelayUnit {
            @display("p=800,300;is=m");

        }
        stp: <spanningTreeProtocol> like ISpanningTree if hasStp {
            @display("p=474,92");
        }
        up: MessageDispatcher {
            parameters:
                @display("p=800,160;b=1200,5");
        }
        down: MessageDispatcher {
            parameters:
                @display("p=800,380;b=1200,5");
        }
        eth[sizeof(ethg)]: <default("EthernetInterface")> like IEthernetInterface {
            parameters:
                mac.promiscuous = true;
                @display("p=250,450,row,150;q=txQueue");
        }
        encap: <default("ANSA_EtherEncap")> like IEtherEncap {
            @display("p=800,220");
        }
        cdp: CDP if hasCDP {
            parameters:
                @display("p=800,100");
            gates:
                ifIn;
                ifOut;
        }
        lldp: LLDP if hasLLDP {
            parameters:
                @display("p=1145,92");
            gates:
                ifIn;
                ifOut;
        }
    connections:
        relayUnit.ifOut --> down.in++;
        relayUnit.ifIn <-- down.out++;

        encap.lowerLayerIn <-- relayUnit.upperLayerOut;
        encap.lowerLayerOut --> relayUnit.upperLayerIn;

        encap.upperLayerIn <-- up.out++;
        encap.upperLayerOut --> up.in++;

        for i=0..sizeof(ethg)-1 {
            down.out++ --> eth[i].upperLayerIn;
            down.in++ <-- eth[i].upperLayerOut;
            eth[i].phys <--> { @display("m=s"); } <--> ethg[i];
        }

        if hasStp {
            stp.relayIn <-- up.out++; // relayUnit.stpOut;
            stp.relayOut --> up.in++; // relayUnit.stpIn;
        }

        if hasCDP {
            cdp.ifIn <-- up.out++; // relayUnit.stpOut;
            cdp.ifOut --> up.in++; // relayUnit.stpIn;
        }

        if hasLLDP {
            lldp.ifIn <-- up.out++; // relayUnit.stpOut;
            lldp.ifOut --> up.in++; // relayUnit.stpIn;
        }
        //        cdp.ifIn <-- relayUnit.cdpOut if hasCDP;
//        cdp.ifOut --> relayUnit.cdpIn if hasCDP;
//        lldp.ifIn <-- relayUnit.lldpOut if hasLLDP;
//        lldp.ifOut --> relayUnit.lldpIn if hasLLDP;

}




//module ANSA_EtherSwitch
//{
//    parameters:
//        @networkNode();
//
//        @capabilities("Repeater","MAC Bridge","Switch");
//        @capabilitiesEnabled("Repeater","MAC Bridge","Switch");
//
//        @labels(node,ethernet-node);
//        @display("i=device/switch;bgb=715,395");
//        bool hasStatus = default(true);
//        bool csmacdSupport = default(true);  // by default use CSMA/CD
//        bool hasCDP = default(false);
//        bool hasLLDP = default(false);
//        bool hasSTP = default(false);
//
//        string relayUnitType = default("relayUnit"); // type of the ~IMACRelayUnit;
//        string macTableType = default("MacAddressTable"); // type of the ~IMACAddressTable
//        eth[*].encap.typename = "EtherEncapDummy";
//        eth[*].csmacdSupport = csmacdSupport;
//        eth[*].mac.typename = default(csmacdSupport ? "EtherMac" : "EtherMacFullDuplex");
//        *.interfaceTableModule = default(absPath(".interfaceTable"));
//
//        string deviceId = default("");
//        xml configFile = default(xmldoc("config.xml"));
//        string deviceType = default("Switch");
//        **.deviceId = deviceId;
//        **.deviceType = deviceType;
//        **.configFile = default(xmldoc("config.xml"));
//    gates:
//        inout ethg[] @labels(EtherFrame-conn);
//    submodules:
//        status: NodeStatus if hasStatus {
//            @display("p=73,111;is=s");
//        }
//        interfaceTable: InterfaceTable {
//            @display("p=73,65;is=s");
//        }
//        macTable: <macTableType> like IMacAddressTable {
//            @display("p=73,19;is=s");
//        }
//        relayUnit: <relayUnitType> like ANSA_IMACRelayUnit {
//            parameters:
//                @display("p=405,219;is=m");
//        }
//        up: MessageDispatcher {
//            parameters:
//                @display("p=405,84;b=550,5");
//        }
//
//        down: MessageDispatcher {
//            parameters:
//                @display("p=405,290;b=550,5");
//        }
//        eth[sizeof(ethg)]: <default("EthernetInterface")> like IEthernetInterface {
//            parameters:
//                mac.promiscuous = true;
//                @display("p=157,347,row;q=txQueue");
//        }
//        cdp: CDP if hasCDP {
//            parameters:
//                @display("p=286,34");
//            gates:
//                ifIn;
//                ifOut;
//        }
//        lldp: LLDP if hasLLDP {
//            parameters:
//                @display("p=527,34");
//            gates:
//                ifIn;
//                ifOut;
//        }
//    connections allowunconnected:
//        relayUnit.ifOut --> down.in++;
//        relayUnit.ifIn <-- down.out++;
//        for i=0..sizeof(ethg)-1 {
//            down.out++ --> eth[i].upperLayerIn;
//            down.in++ <-- eth[i].upperLayerOut;
//            eth[i].phys <--> { @display("m=s"); } <--> ethg[i];
//        }
//        cdp.ifIn <-- relayUnit.cdpOut if hasCDP;
//        cdp.ifOut --> relayUnit.cdpIn if hasCDP;
//        lldp.ifIn <-- relayUnit.lldpOut if hasLLDP;
//        lldp.ifOut --> relayUnit.lldpIn if hasLLDP;
//}

