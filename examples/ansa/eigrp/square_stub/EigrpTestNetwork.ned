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

package inet.examples.ansa.eigrp.square_stub;

import ansa.node.ANSA_EIGRPRouter;
import ansa.node.ANSA_Host;
import inet.node.ethernet.Eth100M;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.networklayer.configurator.ipv6.FlatNetworkConfigurator6;
import inet.common.scenario.ScenarioManager;

network EigrpTestNetwork
{
    @display("bgb=448,275");
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
                @display("p=446,223");
        }

        R1: ANSA_EIGRPRouter {
            parameters:
                @display("p=139,38");
            gates:
                ethg[3];
        }
        R2: ANSA_EIGRPRouter {
            parameters:
                @display("p=120,159");
            gates:
                ethg[3];
        }
        R3: ANSA_EIGRPRouter {
            @display("p=314,38");
            gates:
                ethg[3];
        }
        R4: ANSA_EIGRPRouter {
            @display("p=353,159");
            gates:
                ethg[3];
        }
        LAN1: ANSA_Host {
            @display("p=24,38;i=misc/cloud");
        }
        LAN2: ANSA_Host {
            @display("p=120,246;i=misc/cloud");
        }
        LAN3: ANSA_Host {
            @display("p=425,38;i=misc/cloud");
        }
        LAN4: ANSA_Host {
            @display("p=353,245;i=misc/cloud");
        }
        scenarioManager: ScenarioManager {
            @display("p=35,246");
        }

    connections:
        R1.ethg[0] <--> Eth100M <--> R2.ethg[0];
        R1.ethg[1] <--> Eth100M <--> R3.ethg[0];
        R2.ethg[1] <--> Eth100M <--> R4.ethg[0];        
        R3.ethg[1] <--> Eth100M <--> R4.ethg[1];

        R1.ethg[2] <--> Eth100M <--> LAN1.ethg++;
        R2.ethg[2] <--> Eth100M <--> LAN2.ethg++;
        R3.ethg[2] <--> Eth100M <--> LAN3.ethg++;
        R4.ethg[2] <--> Eth100M <--> LAN4.ethg++;
}

