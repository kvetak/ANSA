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

package inet.examples.ansa.cdp.basic_router;

import ansa.node.ANSA_Host;
import inet.common.lifecycle.LifecycleController;
import ansa.node.ANSA_Router;
import inet.node.ethernet.Eth100M;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.common.scenario.ScenarioManager;

network basicRouter
{
    @display("bgb=521,431");
    types:
        channel Unnamed
        {
        }

    submodules:
        configurator: IPv4NetworkConfigurator {
            parameters:
                config = xmldoc("./networkConfig.xml");
                @display("p=48,181");
        }

        R1: ANSA_Router {
            parameters:
                @display("p=153,289");
            gates:
                ethg[3];
        }
        R2: ANSA_Router {
            parameters:
                @display("p=324,289");
            gates:
                ethg[3];
        }
        R3: ANSA_Router {
            parameters:
                @display("p=324,142");
            gates:
                ethg[3];
        }

        lifecycleController: LifecycleController {
            @display("p=48,68");
        }

        scenarioManager: ScenarioManager {
            @display("p=48,123");
        }

        H: ANSA_Host {
            @display("p=58,288");
        }
    connections allowunconnected:
        R1.ethg[0] <--> Eth100M <--> R2.ethg[0];
        R2.ethg[1] <--> Eth100M <--> R3.ethg[1];
        R3.ethg[0] <--> Eth100M <--> R1.ethg[1];
        H.ethg++ <--> Eth100M <--> R1.ethg[2];
}
