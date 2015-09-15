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
// @date 1.12.2014
// @author xrekvi00@stud.fit.vutbr.cz
// @brief Network for testing purposes of EIGRP Dual-stack

package inet.examples.ansa.networklayer.eigrp.testing_scenario_ds;

import inet.ansa.networklayer.eigrp.EigrpRouterDS;
import inet.ansa.nodes.ANSA_DualStackHost;
import inet.node.ethernet.Eth100M;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.common.scenario.ScenarioManager;


network EigrpTestNetwork
{
    @display("bgb=521,431");
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
                @display("p=450,290");
        }

        R1: EigrpRouterDS {
            parameters:
                @display("p=250,357");
            gates:
                ethg[3];
        }
        R2: EigrpRouterDS {
            parameters:
                @display("p=250,207");
            gates:
                ethg[4];
        }
        R3: EigrpRouterDS {
            parameters:
                @display("p=450,207");
            gates:
                ethg[3];
        }
        R4: EigrpRouterDS {
            parameters:
                @display("p=50,207");
            gates:
                ethg[2];
        }
        LAN1: ANSA_DualStackHost {
            @display("p=50,357;i=cloud_s");
        }
        LAN2: ANSA_DualStackHost {
            @display("p=250,57;i=cloud_s");
        }
        scenarioManager: ScenarioManager {
            @display("p=450,357");
        }
        LAN3: ANSA_DualStackHost {
            @display("p=450,57;i=cloud_s");
        }
        LAN4: ANSA_DualStackHost {
            @display("p=50,57;i=cloud_s");
        }
    connections:
        R1.ethg[0] <--> Eth100M <--> R2.ethg[0];
        R1.ethg[1] <--> Eth100M <--> R3.ethg[0];
        R2.ethg[1] <--> Eth100M <--> R3.ethg[1];
        R2.ethg[2] <--> Eth100M <--> R4.ethg[0];

        R1.ethg[2] <--> Eth100M <--> LAN1.ethg++;
        R2.ethg[3] <--> Eth100M <--> LAN2.ethg++;
        R3.ethg[2] <--> Eth100M <--> LAN3.ethg++;
        R4.ethg[1] <--> Eth100M <--> LAN4.ethg++;
}

