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

package inet.examples.ansa.hsrp.testActiveUp;

import ansa.node.ANSA_HSRPRouter;
import ansa.node.ANSA_Host;
import inet.common.lifecycle.LifecycleController;
import inet.common.scenario.ScenarioManager;
import inet.node.ethernet.EtherSwitch;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;

network testActiveUp
{
    @display("bgb=338,280");

    submodules:
        R1: ANSA_HSRPRouter {
            @display("p=111,46");
        }
        R2: ANSA_HSRPRouter {
            @display("p=205,46");
        }
        R3: ANSA_HSRPRouter {
            @display("p=288,46");
        }
        PC1: ANSA_Host {
            @display("p=136,201");
        }
        PC2: ANSA_Host {
            @display("p=273,200");
        }
        SW1: EtherSwitch {
            @display("p=205,139");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                config = xml("<config>"+
                            "<interface hosts='R1' towards='SW1 PC1' address='192.168.1.1' netmask='255.255.255.0' />"+
                            "<interface hosts='R2' towards='SW1 PC1' address='192.168.1.2' netmask='255.255.255.0' />"+
                            "<interface hosts='R3' towards='SW1 PC1' address='192.168.1.3' netmask='255.255.255.0' />"+

       						"<interface hosts='PC1' address='192.168.1.10' netmask='255.255.255.0' />"+
       						"<interface hosts='PC2' address='192.168.1.11' netmask='255.255.255.0' />"+
                            "</config>");
                addStaticRoutes = true;
                addDefaultRoutes = true;
                @display("p=48,31");
        }

        scenarioManager: ScenarioManager {
            @display("p=48,73");
        }

        lifecycleController: LifecycleController {
            @display("p=48,115");
        }
    connections:
        R1.ethg++ <--> Eth100M <--> SW1.ethg++;
        R2.ethg++ <--> Eth100M <--> SW1.ethg++;
        R3.ethg++ <--> Eth100M <--> SW1.ethg++;
        SW1.ethg++ <--> Eth100M <--> PC1.ethg++;
        SW1.ethg++ <--> Eth100M <--> PC2.ethg++;
}
