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

// @file EigrpTestNetwork.ned
// @date 
// @author 

package inet.examples.ansa.networklayer.eigrp.two_node;

import inet.ansa.networklayer.eigrp.EigrpRouter;
import inet.ansa.nodes.inet.ANSAStandardHost;
import inet.nodes.ethernet.Eth100M;
import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.world.scenario.ScenarioManager;


//
// TODO Auto-generated network
//
network EigrpTestNetwork
{
    @display("bgb=540,240");
    submodules:
        configurator: IPv4NetworkConfigurator {
            parameters:
                config = xml("<config></config>");
                assignAddresses = false;
                assignDisjunctSubnetAddresses = false;
                addStaticRoutes = false;
                addDefaultRoutes = false;
                addSubnetRoutes = false;
                optimizeRoutes = false;
                @display("p=29,213");
        }

        R1: EigrpRouter {
            parameters:
                @display("p=67,38");
            gates:
                ethg[2];
        }
        R2: EigrpRouter {
            parameters:
                @display("p=296,38");
            gates:
                ethg[2];
        }
        LAN1: ANSAStandardHost {
            @display("p=67,151;i=cloud_s");
        }
        LAN2: ANSAStandardHost {
            @display("p=296,151;i=cloud_s");
        }
        scenarioManager: ScenarioManager {
            @display("p=117,213");
        }
    connections:
        R1.ethg[0] <--> Eth100M <--> R2.ethg[0];
        R1.ethg[1] <--> Eth100M <--> LAN1.ethg++;
        R2.ethg[1] <--> Eth100M <--> LAN2.ethg++;
}

