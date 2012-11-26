//
// Copyright (C) 2009
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

package inet.ansa;


import inet.ansa.acl.acl;
import inet.ansa.deviceConfigurator.DeviceConfigurator;
import inet.ansa.ethernet.AnsaEthernetInterface;
import inet.ansa.InterfaceStateManager.InterfaceStateManager;
import inet.ansa.IPGen.AnsaIPTrafGen;
import inet.ansa.ospfv2.AnsaOSPFRouting;
import inet.ansa.ppp.PPPInterface;
import inet.ansa.rip.RIPRouting;
import inet.ansa.routingTable.AnsaInterfaceTable;
import inet.ansa.routingTable.AnsaRoutingTable;
import inet.ansa.AnsaNetworkLayer;
import inet.ansa.pim.pim;
import inet.ansa.pim.tables.PimInterfaceTable;
import inet.ansa.multicastRoutingTable.MulticastRoutingTable;

import inet.base.NotificationBoard;
import inet.nodes.inet.NetworkLayer;
import inet.transport.udp.UDP;
//import inet.applications.pingapp.PingApp;
//
// \IP router.
//
module PimANSAHost
{
    parameters:
        @node();
        string hostname = default("");
        string deviceId;
        string configFile;
        @display("i=device/server;bgb=437,396");
    gates:
        inout pppg[];
        inout ethg[];
    submodules:
        deviceConfigurator: DeviceConfigurator {
            parameters:
                deviceId = deviceId;
                deviceType = "Router";
                configFile = configFile;
                @display("p=349,273");
        }
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=52,42");
        }
        interfaceStateManager: InterfaceStateManager {
            parameters:
                @display("p=45,273");
        }
        interfaceTable: AnsaInterfaceTable {
            parameters:
                @display("p=143,42");
        }
        routingTable: AnsaRoutingTable {
            parameters:
                routerId = deviceId;
                IPForward = true;
                configFile = configFile;
                @display("p=235,42");
        }
        udp: UDP {
            parameters:
                @display("p=172,115;i=block/transport");
            gates:
                appIn[1];
                appOut[1];
        }
        acl: acl {
            parameters:
                @display("p=172,261,row;i=block/network2");
            gates:
                toNetworkLayerIn[sizeof(pppg)+sizeof(ethg)];
                toNetworkLayerOut[sizeof(pppg)+sizeof(ethg)];
                ifIn[sizeof(pppg)+sizeof(ethg)];
                ifOut[sizeof(pppg)+sizeof(ethg)];
        }
        networkLayer: NetworkLayer {
            parameters:
                @display("p=172,188;q=queue");
            gates:
                ifIn[sizeof(pppg)+sizeof(ethg)];
                ifOut[sizeof(pppg)+sizeof(ethg)];
        }
        ppp[sizeof(pppg)]: PPPInterface {
            parameters:
                @display("p=104,342,row,110;q=l2queue");
        }
        eth[sizeof(ethg)]: AnsaEthernetInterface {
            parameters:
                @display("p=235,342,row,110;q=l2queue");
        }
        ipTrafGen: AnsaIPTrafGen {
            parameters:
                @display("p=303,188;i=block/circle");
        }
        //pingApp4: PingApp {
        //    parameters:
        //        @display("p=34,110");
        //}
    connections allowunconnected:

        udp.ipOut --> networkLayer.udpIn;
        udp.ipIn <-- networkLayer.udpOut;

        ipTrafGen.ipOut --> networkLayer.rsvpIn;
        ipTrafGen.ipIn <-- networkLayer.rsvpOut;

//        pingApp4.pingOut --> networkLayer.pingIn;
//        pingApp4.pingIn <-- networkLayer.pingOut;

        // connections to network outside
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].netwOut --> acl.ifIn[i];
            acl.toNetworkLayerOut[i] --> networkLayer.ifIn[i];
            ppp[i].netwIn <-- acl.ifOut[i];
            acl.toNetworkLayerIn[i] <-- networkLayer.ifOut[i];
        }

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].netwOut --> acl.ifIn[sizeof(pppg)+i];
            acl.toNetworkLayerOut[sizeof(pppg)+i] --> networkLayer.ifIn[sizeof(pppg)+i];
            eth[i].netwIn <-- acl.ifOut[sizeof(pppg)+i];
            acl.toNetworkLayerIn[sizeof(pppg)+i] <-- networkLayer.ifOut[sizeof(pppg)+i];
        }

}


