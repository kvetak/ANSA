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

package inet.examples.ansa.rip;

import inet.nodes.ethernet.Eth100M;
import inet.ansa.applications.rip.RIPRouter;
import inet.ansa.nodes.inet.ANSAStandardHost;
import inet.world.scenario.ScenarioManager;


network RIPngTestNetwork
{
    @display("bgb=376,240");
    types:
    submodules:
        router1: RIPRouter {
            parameters:
                @display("p=122,78");
            gates:
                ethg[4];

        }
        router2: RIPRouter {
            parameters:
                @display("p=220,78");
            gates:
                ethg[4];
        }
        router3: RIPRouter {
            parameters:
                @display("p=173,152");
            gates:
                ethg[5];
        }
        router4: RIPRouter {
            parameters:
                @display("p=98,199");
            gates:
                ethg[2];
        }
        LAN2: ANSAStandardHost {
            @display("p=53,118;i=cloud_s");
        }
        LAN4: ANSAStandardHost {
            @display("p=304,118;i=cloud_s");
        }
        LAN3: ANSAStandardHost {
            @display("p=304,37;i=cloud_s");
        }
        LAN1: ANSAStandardHost {
            @display("p=53,37;i=cloud_s");
        }
        LAN5: ANSAStandardHost {
            @display("p=173,208;i=cloud_s");
        }
        LAN6: ANSAStandardHost {
            @display("p=251,208;i=cloud_s");
        }
        LAN7: ANSAStandardHost {
            @display("p=31,208;i=cloud_s");
        }
        scenarioManager: ScenarioManager {
            @display("p=327,208");
            script = xmldoc("scenario.xml");
        }
    connections:
        router1.ethg[0] <--> Eth100M <--> router2.ethg[0];
        router1.ethg[1] <--> Eth100M <--> router3.ethg[0];
        router1.ethg[2] <--> Eth100M <--> LAN1.ethg++;
        router1.ethg[3] <--> Eth100M <--> LAN2.ethg++;
        router2.ethg[2] <--> Eth100M <--> LAN3.ethg++;
        router2.ethg[3] <--> Eth100M <--> LAN4.ethg++;
        router3.ethg[1] <--> Eth100M <--> router2.ethg[1];
        router3.ethg[2] <--> Eth100M <--> LAN5.ethg++;
        router3.ethg[3] <--> Eth100M <--> LAN6.ethg++;
        router3.ethg[4] <--> Eth100M <--> router4.ethg[0];
        router4.ethg[1] <--> Eth100M <--> LAN7.ethg++;
}
