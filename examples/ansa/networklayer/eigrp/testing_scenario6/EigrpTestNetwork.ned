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
// @date 24.11.2014
// @author xrekvi00@stud.fit.vutbr.cz
// @brief Network for testing purposes of EIGRP for IPv6-only

package inet.examples.ansa.networklayer.eigrp.testing_scenario6;

import inet.ansa.networklayer.eigrp.EigrpRouter6;
import inet.ansa.nodes.ipv6.ANSAStandardHost6;
import inet.nodes.ethernet.Eth100M;
import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.world.scenario.ScenarioManager;


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

        R1: EigrpRouter6 {
            parameters:
                @display("p=250,357");
            gates:
                ethg[3];
        }
        R2: EigrpRouter6 {
            parameters:
                @display("p=250,207");
            gates:
                ethg[4];
        }
        R3: EigrpRouter6 {
            parameters:
                @display("p=450,207");
            gates:
                ethg[3];
        }
        R4: EigrpRouter6 {
            parameters:
                @display("p=50,207");
            gates:
                ethg[2];
        }
        LAN1: ANSAStandardHost6 {
            @display("p=50,357;i=cloud_s");
        }
        LAN2: ANSAStandardHost6 {
            @display("p=250,57;i=cloud_s");
        }
        scenarioManager: ScenarioManager {
            @display("p=450,357");
        }
        LAN3: ANSAStandardHost6 {
            @display("p=450,57;i=cloud_s");
        }
        LAN4: ANSAStandardHost6 {
            @display("p=50,57;i=cloud_s");
        }
    connections:
        R1.ethg[1] <--> Eth100M <--> R2.ethg[1];
        R1.ethg[2] <--> Eth100M <--> R3.ethg[2];
        R2.ethg[2] <--> Eth100M <--> R3.ethg[1];
        R2.ethg[3] <--> Eth100M <--> R4.ethg[1];

        R1.ethg[0] <--> Eth100M <--> LAN1.ethg++;
        R2.ethg[0] <--> Eth100M <--> LAN2.ethg++;
        R3.ethg[0] <--> Eth100M <--> LAN3.ethg++;
        R4.ethg[0] <--> Eth100M <--> LAN4.ethg++;
}

