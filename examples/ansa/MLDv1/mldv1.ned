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

package inet.examples.ansa.MLDv1;
import inet.ansa.applications.ripng.RIPngRouter;
import inet.ansa.nodes.ipv6.ANSAStandardHost6;
import inet.nodes.ethernet.EtherSwitch;
import ned.DatarateChannel;
import inet.nodes.ethernet.Eth100M;


//
// TODO documentation
//
network MLD
{
    types:
        channel ethernetline extends DatarateChannel
        {
            delay = 0.1us;
        }

    submodules:
        Bratislava1: ANSAStandardHost6 {
            @display("p=145,77");
            gates:
                ethg[1];
        }
        Router1: RIPngRouter {
            @display("p=423,50");
            gates:
                ethg[2];
        }
        Router2: RIPngRouter {
            @display("p=423,214");
            gates:
                ethg[2];
        }
        etherSwitch: EtherSwitch {
            @display("p=277,124");
            gates:
                ethg[4];
        }
        Bratislava2: ANSAStandardHost6 {
            @display("p=145,182");
            gates:
                ethg[1];
        }
        etherSwitch1: EtherSwitch {
            @display("p=563,133");
            gates:
                ethg[4];
        }
        Brno1: ANSAStandardHost6 {
            @display("p=765,62");
            gates:
                ethg[1];
        }
        Brno2: ANSAStandardHost6 {
            parameters:
            @display("p=765,232");
            gates:
                ethg[1];
        }
    connections allowunconnected:
        Bratislava1.ethg[0] <--> Eth100M <--> etherSwitch.ethg[0];
        Bratislava2.ethg[0] <--> Eth100M <--> etherSwitch.ethg[1];
        Router1.ethg[0] <--> Eth100M <--> etherSwitch.ethg[2];
        Router2.ethg[0] <--> Eth100M <--> etherSwitch.ethg[3];
        Brno1.ethg[0] <--> Eth100M <--> etherSwitch1.ethg[0];
        Brno2.ethg[0] <--> Eth100M <--> etherSwitch1.ethg[1];
        Router1.ethg[1] <--> Eth100M <--> etherSwitch1.ethg[2];
        Router2.ethg[1] <--> Eth100M <--> etherSwitch1.ethg[3];
}

