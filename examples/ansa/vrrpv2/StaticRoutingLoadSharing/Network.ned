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

package inet.examples.ansa.vrrpv2.StaticRoutingLoadSharing;

import inet.examples.ansa.vrrpv2.router.VRRPv2SimpleRouter;
import inet.nodes.ethernet.Eth100M;
import inet.nodes.ethernet.EtherSwitch;
import inet.world.scenario.ScenarioManager;
import inet.ansa.nodes.inet.ANSAStandardHost;

network Network
{
    @display("bgb=300,415;bgl=2");
    submodules:
        ISP: VRRPv2SimpleRouter {
            @display("p=150,50");
            gates:
                ethg[2];
        }
        GW1: VRRPv2SimpleRouter {
            @display("p=50,150");
            gates:
                ethg[2];
        }
        GW2: VRRPv2SimpleRouter {
            @display("p=250,150");
            gates:
                ethg[2];
        }
        SW: EtherSwitch {
            @display("p=150,250");
            gates:
                ethg[4];
        }
        Host1: ANSAStandardHost {
            @display("p=50,350");
            gates:
                ethg[1];
        }
        Host2: ANSAStandardHost {
            @display("p=250,350");
            gates:
                ethg[1];
        }        
        Manager: ScenarioManager {
            @display("p=250,50");
        }
    connections:
        ISP.ethg[0] <--> Eth100M <--> GW1.ethg[0];
        ISP.ethg[1] <--> Eth100M <--> GW2.ethg[0];

        GW1.ethg[1] <--> Eth100M <--> SW.ethg[0];
        GW2.ethg[1] <--> Eth100M <--> SW.ethg[1];

        Host1.ethg[0] <--> Eth100M <--> SW.ethg[2];
		Host2.ethg[0] <--> Eth100M <--> SW.ethg[3];        
}
