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

package inet.examples.ansaExamples.ISIS;


import inet.ansa.ANSASwitch;
import inet.ansa.isis.AnsaISISRouter;
import inet.ansa.ANSARouter;
import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.networklayer.autorouting.FlatNetworkConfigurator6;
import inet.nodes.ethernet.EtherSwitch;
import inet.nodes.mpls.LDP_LSR;
import ned.DatarateChannel;





network ISIStest1
{
    @display("bgb=814,418");
    types:
        channel ethernetline extends DatarateChannel
        {
            delay = 0.1us;
        }
    submodules:

        router1: AnsaISISRouter {
            @display("p=191,36");
        }
        router2: AnsaISISRouter {
            @display("p=377,138");
        }
        router3: AnsaISISRouter {
            @display("p=152,192");
        }
        etherSwitch: EtherSwitch {
            @display("p=256,129");
        }
        router4: AnsaISISRouter {
            @display("p=517,138");
        }
        router5: AnsaISISRouter {
            @display("p=694,147");
        }
        router6: AnsaISISRouter {
            @display("p=666,251");
        }
    connections allowunconnected:
        router1.ethg++ <--> ethernetline <--> etherSwitch.ethg++;
        router2.ethg++ <--> ethernetline <--> etherSwitch.ethg++;
        router3.ethg++ <--> ethernetline <--> etherSwitch.ethg++;
        router2.ethg++ <--> ethernetline <--> router4.ethg++;
        router4.ethg++ <--> ethernetline <--> router5.ethg++;
        router5.ethg++ <--> ethernetline <--> router6.ethg++;

}
