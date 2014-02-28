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
import inet.linklayer.IWiredNic;
import inet.networklayer.common.InterfaceTable;
import inet.world.scenario.ScenarioManager;


module EigrpRouter
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


        eth[sizeof(ethg)]: <default("EthernetInterface")> like IWiredNic {
            parameters:
                @display("p=127,317,row,90;q=txQueue");
        }
        ppp[sizeof(pppg)]: <default("PPPInterface")> like IWiredNic {
            parameters:
                @display("p=290,317,row,90;q=txQueue");
        }

        eigrpProcess: eigrpProcess {
            @display("p=229,153");
        }
        ansaNetworkLayer: ansaNetworkLayer {
            @display("p=236,229");
        }
    connections allowunconnected:
        eigrpProcess.splitterOut --> ansaNetworkLayer.eigrpIn;
        eigrpProcess.splitterIn <-- ansaNetworkLayer.eigrpOut;

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].upperLayerOut --> ansaNetworkLayer.ifIn++;
            eth[i].upperLayerIn <-- ansaNetworkLayer.ifOut++;
        }

        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].upperLayerOut --> ansaNetworkLayer.ifIn++;
            ppp[i].upperLayerIn <-- ansaNetworkLayer.ifOut++;
        }
}
