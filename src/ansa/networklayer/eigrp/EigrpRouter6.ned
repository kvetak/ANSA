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

package inet.ansa.networklayer.eigrp;

import inet.ansa.networklayer.ipv4.AnsaRoutingTable;
import inet.ansa.nodes.inet.ansaNetworkLayer;
import inet.ansa.util.InterfaceStateManager.InterfaceStateManager;
import inet.ansa.util.deviceConfigurator.DeviceConfigurator;
import inet.base.NotificationBoard;
import inet.linklayer.contract.IWiredNic;
import inet.networklayer.common.InterfaceTable;
//import inet.ansa.networklayer.ipSplitter.IpSplitter;

//import inet.node.ipv6.NetworkLayer6;
import inet.ansa.nodes.ipv6.AnsaNetworkLayer6;
import inet.ansa.networklayer.ipv6.ANSARoutingTable6;



module EigrpRouter6
{
    parameters:
        @display("i=abstract/router;bgb=571,484");
        @node;
        @labels(node,ethernet-node);
        xml routingTable = default(xml("<routingTable/>"));  // Routing table is filled by deviceConfigurator
        bool IPForward = default(true);
        string configFile = default("config.xml");           // file name of the XML file which contains configuration for the router
        string deviceId;                                     // needed for proper load of the device XML configuration
    gates:
        //inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);
    submodules:

        deviceConfigurator: DeviceConfigurator {
            @display("p=71,137");
            deviceId = deviceId;
            deviceType = "Router";
            configFile = configFile;
        }

        interfaceStateManager: InterfaceStateManager {
            @display("p=71,212");
        }

        notificationBoard: NotificationBoard {
            parameters:
                @display("p=71,62");
        }

//        routingTable: AnsaRoutingTable {
//            parameters:
//                @display("p=290,62");
//                routingTable = routingTable;
//        }


        routingTable6: ANSARoutingTable6 {
            parameters:
                @display("p=380,62");
                //routingTable = routingTable6;
                isRouter = IPForward;
        }

//        ipSplitter: IpSplitter {
//            parameters:
//                @display("p=262,304");
//            gates:
//                ifIn[sizeof(ethg)+sizeof(pppg)];
//                ifOut[sizeof(ethg)+sizeof(pppg)];
//                ipv4In[sizeof(ethg)+sizeof(pppg)];
//                ipv4Out[sizeof(ethg)+sizeof(pppg)];
//                ipv6In[sizeof(ethg)+sizeof(pppg)];
//                ipv6Out[sizeof(ethg)+sizeof(pppg)];
               // isisIn[sizeof(ethg)+sizeof(pppg)];
               // isisOut[sizeof(ethg)+sizeof(pppg)];
//        }


        // linklayer
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=204,62");
        }


        eth[sizeof(ethg)]: <default("EthernetInterface")> like IWiredNic {
            parameters:
                @display("p=211,382,row,90;q=txQueue");
        }
   //     ppp[sizeof(pppg)]: <default("PPPInterface")> like IWiredNic {
   //         parameters:
   //             @display("p=323,382,row,90;q=txQueue");
   //     }

        eigrp: eigrpProcess6 {
            @display("p=262,137;i=block/network2");
        }
//        ansaNetworkLayer: ansaNetworkLayer {
//            parameters:
//                @display("p=211,212");
//            gates:
//                ifIn[sizeof(ethg)+sizeof(pppg)];
//                ifOut[sizeof(ethg)+sizeof(pppg)];
//        }

        AnsaNetworkLayer6: AnsaNetworkLayer6 {
            parameters:
                @display("p=323,212");
            gates:
                ifIn[sizeof(ethg)+sizeof(pppg)];
                ifOut[sizeof(ethg)+sizeof(pppg)];
        }
    connections allowunconnected:
        //eigrp.splitterOut --> ansaNetworkLayer.eigrpIn;
        //eigrp.splitterIn <-- ansaNetworkLayer.eigrpOut;
        
        eigrp.splitter6Out --> AnsaNetworkLayer6.eigrpIn;
        eigrp.splitter6In <-- AnsaNetworkLayer6.eigrpOut;

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].upperLayerOut --> AnsaNetworkLayer6.ifIn[i];
            eth[i].upperLayerIn <-- AnsaNetworkLayer6.ifOut[i];


//            eth[i].upperLayerOut --> ipSplitter.ifIn[i];
//            eth[i].upperLayerIn <-- ipSplitter.ifOut[i];
//            ipSplitter.ipv6Out[i] --> AnsaNetworkLayer6.ifIn[i];
//            ipSplitter.ipv6In[i] <-- AnsaNetworkLayer6.ifOut[i];
//            ipSplitter.ipv4Out[i] --> ansaNetworkLayer.ifIn[i];
//            ipSplitter.ipv4In[i] <-- ansaNetworkLayer.ifOut[i];
            //ethg[i] <--> eth[i].phys;
            //eth[i].upperLayerOut --> ansaNetworkLayer.ifIn++;
            //eth[i].upperLayerIn <-- ansaNetworkLayer.ifOut++;
        }

   //     for i=0..sizeof(pppg)-1 {
   //        pppg[i] <--> ppp[i].phys;
   //         ppp[i].upperLayerOut --> ipSplitter.ifIn[sizeof(ethg)+i];
   //         ppp[i].upperLayerIn <-- ipSplitter.ifOut[sizeof(ethg)+i];
   //         ipSplitter.ipv6Out[sizeof(ethg)+i] --> NetworkLayer6.ifIn[sizeof(ethg)+i];
   //         ipSplitter.ipv6In[sizeof(ethg)+i] <-- NetworkLayer6.ifOut[sizeof(ethg)+i];
   //         ipSplitter.ipv4Out[sizeof(ethg)+i] --> ansaNetworkLayer.ifIn[sizeof(ethg)+i];
   //         ipSplitter.ipv4In[sizeof(ethg)+i] <-- ansaNetworkLayer.ifOut[sizeof(ethg)+i];
   //         //pppg[i] <--> ppp[i].phys;
   //         //ppp[i].upperLayerOut --> ansaNetworkLayer.ifIn++;
   //         //ppp[i].upperLayerIn <-- ansaNetworkLayer.ifOut++;
   //     }
}
