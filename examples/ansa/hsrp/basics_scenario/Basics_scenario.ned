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

package inet.examples.ansa.hsrp.basics_scenario;

import ansa.node.HSRPRouter;
import ansa.node.ANSA_Host;
import inet.common.lifecycle.LifecycleController;
import inet.common.scenario.ScenarioManager;
import inet.node.ethernet.EtherSwitch;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
//import inet.node.ethernet.EtherLink;

network Basics_scenario
{
    @display("bgb=529,442");

    types:
        channel C extends ned.DatarateChannel
        {
            datarate = 100Mbps;
        }
    submodules:
        R1: HSRPRouter {
            @display("p=75,176");
        }
        R2: HSRPRouter {
            @display("p=258,176");
        }
        H1: ANSA_Host {
            @display("p=236,374");
        }
        SW1: EtherSwitch {
            @display("p=236,299");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                config = xml("<config>"+
                            "<interface hosts='R1' towards='SW1 H1' address='192.168.1.1' netmask='255.255.255.0' />"+
                            "<interface hosts='R2' towards='SW1 H1' address='192.168.1.2' netmask='255.255.255.0' />"+
                            "<interface hosts='R3' towards='SW1 H1' address='192.168.1.3' netmask='255.255.255.0' />"+

       						"<interface hosts='H1' address='192.168.1.5' netmask='255.255.255.0' />"+
                            "</config>");
                addStaticRoutes = true;
                addDefaultRoutes = true;
                @display("p=36,19");
        }
        lifecycleController: LifecycleController {
            @display("p=122,19");
        }

        scenarioManager: ScenarioManager {
            script = xmldoc("scenario.xml");
            @display("p=36,69");
        }
        R3: HSRPRouter {
            @display("p=410,176");
        }
    connections:
        R1.ethg++ <--> C <--> SW1.ethg++;
        R2.ethg++ <--> C <--> SW1.ethg++;
        SW1.ethg++ <--> C <--> R3.ethg++;
        SW1.ethg++ <--> C <--> H1.ethg++;        
}
