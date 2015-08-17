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

package inet.examples.ansa.vrrpv2.router;

import inet.ansa.networklayer.vrrpv2.VRRPv2;
import inet.ansa.util.InterfaceStateManager.InterfaceStateManager;
import inet.ansa.util.deviceConfigurator.DeviceConfigurator;
import inet.base.NotificationBoard;
import inet.linklayer.loopback.LoopbackInterface;
import inet.networklayer.common.InterfaceTable;
import inet.ansa.networklayer.ipv4.AnsaRoutingTable;
import inet.ansa.linklayer.ethernet.AnsaEthernetInterfaceWithVF;
import inet.linklayer.IWiredNic;

module VRRPv2SimpleRouter
{
    parameters:
        @node();
        @labels(node,ethernet-node);
        @display("i=abstract/router;bgb=401,374");

        string configFile = default("config.xml");           // file name of the XML file which contains configuration for the router
        string deviceId;                                     // needed for proper load of the device XML configuration

        xml routingTable = default(xml("<routingTable/>"));
        bool IPForward = default(true);

        int numOfLoopbacks = default(1);

    gates:
        inout ethg[] @labels(EtherFrame-conn);

    submodules:

        vrrp: VRRPv2 {
            parameters:
                @display("p=250,140");
                deviceId = deviceId;
                configFile = configFile;
        }

        networkLayer: NetworkLayer {
            parameters:
                @display("p=250,222;q=queue");
        }

        deviceConfigurator: DeviceConfigurator {
            parameters:
                @display("p=80,40");
                deviceId = deviceId;
                deviceType = "Router";
                configFile = configFile;
        }

        notificationBoard: NotificationBoard {
            parameters:
                @display("p=80,120");
        }

        interfaceStateManager: InterfaceStateManager {
            parameters:
                @display("p=80,200");
        }

        routingTable: AnsaRoutingTable {
            parameters:
                @display("p=196,40");
                IPForward = true;
                routerId = deviceId;
                configFile = configFile;
        }

        interfaceTable: InterfaceTable {
            parameters:
                @display("p=300,40");
        }

        //eth[sizeof(ethg)]: <default("AnsaEthernetInterfaceWithVF")> like IWiredNic {
        eth[sizeof(ethg)]: AnsaEthernetInterfaceWithVF {
            parameters:
                @display("p=250,302,row,110;q=l2queue");
        }

        lo[numOfLoopbacks]: LoopbackInterface {
            @display("p=128,302");
        }
    connections allowunconnected:
        vrrp.ipOut --> networkLayer.vrrpIn;
        vrrp.ipIn <-- networkLayer.vrrpOut;

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].upperLayerOut --> networkLayer.ifIn++;
            eth[i].upperLayerIn <-- networkLayer.ifOut++;
        }
        for i=0..numOfLoopbacks-1 {
            lo[i].upperLayerOut --> networkLayer.ifIn++;
            lo[i].upperLayerIn <-- networkLayer.ifOut++;
        }
}
