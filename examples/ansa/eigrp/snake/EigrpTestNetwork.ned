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

package inet.examples.ansa.eigrp.snake;

import ansa.node.ANSA_EIGRPRouter;
import ansa.node.ANSA_Host;
import inet.node.ethernet.Eth100M;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.networklayer.configurator.ipv6.FlatNetworkConfigurator6;
import inet.common.scenario.ScenarioManager;

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

        R1: ANSA_EIGRPRouter {
            parameters:
                @display("p=46,37");
            gates:
                ethg[2];
        }
        R2: ANSA_EIGRPRouter {
            parameters:
                @display("p=275,37");
            gates:
                ethg[3];
        }
        R3: ANSA_EIGRPRouter {
            parameters:
                @display("p=494,37");
            gates:
                ethg[2];
        }
        LAN1: ANSA_Host {
            @display("p=46,150;i=cloud_s");
        }
        LAN2: ANSA_Host {
            @display("p=275,150;i=cloud_s");
        }
        LAN3: ANSA_Host {
            @display("p=494,150;i=cloud_s");
        }
        scenarioManager: ScenarioManager {
            @display("p=117,213");
        }
    connections:
        R1.ethg[0] <--> Eth100M <--> R2.ethg[0];
        R2.ethg[1] <--> Eth100M <--> R3.ethg[0];

        R1.ethg[1] <--> Eth100M <--> LAN1.ethg++;
        R2.ethg[2] <--> Eth100M <--> LAN2.ethg++;
        R3.ethg[1] <--> Eth100M <--> LAN3.ethg++;
}

