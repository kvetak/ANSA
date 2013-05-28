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

package inet.examples.ansa.IGMPv2;
import inet.ansa.networklayer.pim.PIMRouter;
import inet.ansa.nodes.inet.ANSAStandardHost;
import inet.examples.bgpv4.BGPUpdate.BGPRouter;
import inet.nodes.ethernet.EtherSwitch;
import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;
import ned.DatarateChannel;
import inet.nodes.ethernet.Eth100M;


//
// TODO documentation
//
network IGMP
{
    types:
        channel ethernetline extends DatarateChannel
        {
            delay = 0.1us;
        }

    submodules:
        Alice: ANSAStandardHost {
            @display("p=76,62");
            gates:
                ethg[1];
        }
        FITgate1: PIMRouter {
            @display("p=314,58");
            gates:
                ethg[1];
        }
        FITgate2: PIMRouter {
            @display("p=314,168");
            gates:
                ethg[2];
        }
        Bob: ANSAStandardHost {
            @display("p=76,172");
            gates:
                ethg[1];
        }
        Carlos: ANSAStandardHost {
            @display("p=314,257");
            gates:
                ethg[1];
        }
        etherSwitch: EtherSwitch {
            @display("p=203,124");
            gates:
                ethg[4];
        }
    connections allowunconnected:
        Alice.ethg[0] <--> Eth100M <--> etherSwitch.ethg[0];
        Bob.ethg[0] <--> Eth100M <--> etherSwitch.ethg[1];
        FITgate1.ethg[0] <--> Eth100M <--> etherSwitch.ethg[2];
        FITgate2.ethg[0] <--> Eth100M <--> etherSwitch.ethg[3];
        Carlos.ethg[0] <--> Eth100M <--> FITgate2.ethg[1];
}

