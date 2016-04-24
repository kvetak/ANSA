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

package inet.examples.ansa.cdp.mixedNetwork;

import ansa.node.ANSA_CDPEtherSwitch;
import ansa.node.ANSA_CDPRouter;
import ansa.node.ANSA_Host;
import inet.common.lifecycle.LifecycleController;
import inet.node.ethernet.Eth100M;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.common.scenario.ScenarioManager;

network CDPMixedNetwork
{
    @display("bgb=521,431;bgl=3");
    types:
        channel Unnamed
        {
        }

    submodules:
        configurator: IPv4NetworkConfigurator {
            parameters:
                addStaticRoutes = false;
                addDefaultRoutes = false;
                addSubnetRoutes = false;
                optimizeRoutes = false;
                dumpRoutes = true;
                config = xmldoc("./networkConfig.xml");
                @display("p=48,181");
        }

        R1: ANSA_CDPRouter {
            parameters:
                @display("p=282,66");
            gates:
                ethg[3];
        }
        R2: ANSA_CDPRouter {
            parameters:
                @display("p=186,208");
            gates:
                ethg[3];
        }
        R3: ANSA_CDPRouter {
            parameters:
                @display("p=354,208");
            gates:
                ethg[3];
        }
        lifecycleController: LifecycleController {
            @display("p=48,68");
        }

        scenarioManager: ScenarioManager {
            @display("p=48,123");
        }
        Host1: ANSA_Host {
            @display("p=186,393");
        }
        Host2: ANSA_Host {
            @display("p=312,393");
        }
        S1: ANSA_CDPEtherSwitch {
            @display("p=186,303");
        }
        S2: ANSA_CDPEtherSwitch {
            @display("p=354,303");
        }
        Host3: ANSA_Host {
            @display("p=397,393");
        }
    connections allowunconnected:
        R1.ethg[0] <--> Eth100M <--> R2.ethg[0];
        R2.ethg[1] <--> Eth100M <--> R3.ethg[1];
        R3.pppg++ <--> Eth100M <--> R1.pppg++;
        R2.ethg[2] <--> Eth100M <--> S1.ethg++;
        R3.ethg[2] <--> Eth100M <--> S2.ethg++;
        Host1.ethg++ <--> Eth100M <--> S1.ethg++;
        Host2.ethg++ <--> Eth100M <--> S2.ethg++;
        Host3.ethg++ <--> Eth100M <--> S2.ethg++;
}
