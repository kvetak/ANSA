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

package inet.examples.ansa.ISIS2;

import inet.nodes.ethernet.Eth100M;
//import inet.ansa.ANSASwitch;
import inet.nodes.ethernet.EtherSwitch;
import inet.ansa.networklayer.isis.AnsaISISRouter;
//import inet.ansa.ANSARouter;
//import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.networklayer.autorouting.ipv4.FlatNetworkConfigurator;
//import inet.networklayer.autorouting.FlatNetworkConfigurator6;
import inet.networklayer.autorouting.ipv6.FlatNetworkConfigurator6;
import inet.nodes.ethernet.EtherSwitch;
import inet.nodes.mpls.LDP_LSR;
import ned.DatarateChannel;





network ISIStest2
{
    @display("bgb=701,389");
    types:
        channel ethernetline extends DatarateChannel
        {
            delay = 0.1us;
        }
    submodules:

        router1: AnsaISISRouter {
            parameters:
                @display("p=57,191");
                
            gates:
                ethg[1];
        }
        router2: AnsaISISRouter {
            parameters:
                @display("p=269,293");
            gates:
                ethg[3];
        }
        router3: AnsaISISRouter {
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
        router4: AnsaISISRouter {
            parameters:
                @display("p=409,293");
            gates:
                ethg[3];
        }
        router5: AnsaISISRouter {
            parameters:
                @display("p=338,214");
            gates:
                ethg[3];
        }
        router6: AnsaISISRouter {
            parameters:
                @display("p=530,293");
            gates:
                ethg[2];
        }
        router7: AnsaISISRouter {
            parameters:
                @display("p=657,293");
            gates:
                ethg[1];
        }
        etherSwitch2: EtherSwitch {
            parameters:
                @display("p=338,110");
            gates:
                ethg[3];
        }
        router8: AnsaISISRouter {
            parameters:
                @display("p=268,29");
            gates:
                ethg[1];
        }
        router9: AnsaISISRouter {
            parameters:
                @display("p=401,27");
            gates:
                ethg[1];
        }
    connections allowunconnected:
        router1.ethg[0] <--> Eth100M <--> etherSwitch1.ethg[0];
        router2.ethg[1] <--> Eth100M <--> etherSwitch1.ethg[2];
        router3.ethg[0] <--> Eth100M <--> etherSwitch1.ethg[1];
        router2.ethg[2] <--> Eth100M <--> router4.ethg[0];
        router2.ethg[0] <--> Eth100M <--> router5.ethg[1];
        router5.ethg[0] <--> Eth100M <--> router4.ethg[1];
        router8.ethg[0] <--> Eth100M <--> etherSwitch2.ethg[0];
        router9.ethg[0] <--> Eth100M <--> etherSwitch2.ethg[1];
        router5.ethg[2] <--> Eth100M <--> etherSwitch2.ethg[2];
        router6.ethg[0] <--> Eth100M <--> router4.ethg[2];
        router6.ethg[1] <--> Eth100M <--> router7.ethg[0];

}
