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

package inet.examples.ansa.babel.square;

import ansa.node.ANSA_BabelRouter;
import ansa.node.ANSA_Host;
import inet.node.ethernet.Eth100M;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.networklayer.configurator.ipv6.FlatNetworkConfigurator6;
import inet.common.scenario.ScenarioManager;

network square
{
    @display("bgb=521,431");
    submodules:
        configurator: IPv4NetworkConfigurator {
            parameters:
                config = xmldoc("networkConfig.xml");
                @display("p=450,290");
        }

        R1: ANSA_BabelRouter {
            parameters:
                @display("p=153,289");
            gates:
                ethg[3];
        }
        R2: ANSA_BabelRouter {
            parameters:
                @display("p=324,289");
            gates:
                ethg[3];
        }
        R3: ANSA_BabelRouter {
            parameters:
                @display("p=324,142");
            gates:
                ethg[3];
        }
        R4: ANSA_BabelRouter {
            parameters:
                @display("p=153,142");
            gates:
                ethg[3];
        }

        LAN1: ANSA_Host {
            @display("p=153,357;i=cloud_s");
        }
        LAN2: ANSA_Host {
            @display("p=324,357;i=cloud_s");
        }
        LAN3: ANSA_Host {
            @display("p=324,69;i=cloud_s");
        }
        LAN4: ANSA_Host {
            @display("p=153,69;i=cloud_s");
        }
        scenarioManager: ScenarioManager {
            @display("p=450,357");
        }

    connections allowunconnected:
        R1.ethg[0] <--> Eth100M <--> R2.ethg[0];
        R2.ethg[1] <--> Eth100M <--> R3.ethg[1];
        R3.ethg[0] <--> Eth100M <--> R4.ethg[0];
        R4.ethg[1] <--> Eth100M <--> R1.ethg[1];

        R1.ethg[2] <--> Eth100M <--> LAN1.ethg++;
        R2.ethg[2] <--> Eth100M <--> LAN2.ethg++;
        R3.ethg[2] <--> Eth100M <--> LAN3.ethg++;
        R4.ethg[2] <--> Eth100M <--> LAN4.ethg++;

}

