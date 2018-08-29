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

package ansa.examples.ansa.isis.lan;

import inet.node.ethernet.Eth100M;
//import inet.ansa.ANSASwitch;
import inet.node.ethernet.EtherSwitch;
//import ansa.networklayer.isis.AnsaISISRouter;
import ansa.node.ANSA_ISISRouter;
//import inet.ansa.ANSARouter;
//import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.networklayer.configurator.ipv4.FlatNetworkConfigurator;
//import inet.networklayer.autorouting.FlatNetworkConfigurator6;
import inet.networklayer.configurator.ipv6.FlatNetworkConfigurator6;
import inet.node.ethernet.EtherSwitch;
import inet.node.mpls.LDP_LSR;
import ned.DatarateChannel;

network lan
{
    @display("bgb=701,389");
    types:
        channel ethernetline extends DatarateChannel
        {
            delay = 0.1us;
        }
    submodules:
        router1: ANSA_ISISRouter {
            parameters:
                @display("p=57,191");

            gates:
                ethg[1];
        }
        router2: ANSA_ISISRouter {
            parameters:
                @display("p=269,293");
            gates:
                ethg[1];
        }
        router3: ANSA_ISISRouter {
            parameters:
                @display("p=44,347");
            gates:
                ethg[1];
        }
        etherSwitch1: EtherSwitch {
            parameters:
                @display("p=148,284");
            gates:
                ethg[3];
        }
    connections allowunconnected:
        router1.ethg[0] <--> Eth100M <--> etherSwitch1.ethg[0];
        router2.ethg[0] <--> Eth100M <--> etherSwitch1.ethg[2];
        router3.ethg[0] <--> Eth100M <--> etherSwitch1.ethg[1];
}
