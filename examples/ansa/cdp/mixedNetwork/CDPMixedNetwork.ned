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

package ansa.examples.ansa.cdp.mixedNetwork;

import ansa.node.ANSA_EtherSwitch;
import ansa.node.ANSA_LLDPCDPRouter;
import ansa.node.ANSA_Host;
import inet.common.lifecycle.LifecycleController;
import inet.node.ethernet.Eth100M;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.common.scenario.ScenarioManager;

network CDPMixedNetwork
{
    @display("bgb=457,371;bgl=3");
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

        R1: ANSA_LLDPCDPRouter {
            parameters:
                @display("p=282,66");
            gates:
                ethg[3];
        }
        R2: ANSA_LLDPCDPRouter {
            parameters:
                @display("p=186,185");
            gates:
                ethg[3];
        }
        R3: ANSA_LLDPCDPRouter {
            parameters:
                @display("p=355,181");
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
            @display("p=186,291");
        }
        Host2: ANSA_Host {
            @display("p=355,287");
        }
        S1: ANSA_EtherSwitch {
            @display("p=150,65");
        }
    connections allowunconnected:
        R1.ethg[0] <--> Eth100M <--> R2.ethg[0];
        R1.ethg[1] <--> Eth100M <--> R3.ethg[1];
        R1.ethg[2] <--> Eth100M <--> S1.ethg++;
        R2.ethg[2] <--> Eth100M <--> Host1.ethg++;
        R3.ethg[2] <--> Eth100M <--> Host2.ethg++;
}
