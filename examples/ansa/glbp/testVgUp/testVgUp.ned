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

package inet.examples.ansa.glbp.testVgUp;

import ansa.node.GLBPRouter;
import ansa.node.ANSA_Host;
import inet.node.ethernet.EtherSwitch;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.common.scenario.ScenarioManager;

network testVgUp
{
    @display("bgb=434,281");

    submodules:
        R1: GLBPRouter {
            @display("p=111,46");
        }
        R2: GLBPRouter {
            @display("p=205,46");
        }
        R3: GLBPRouter {
            @display("p=299,46");
        }
        PC1: ANSA_Host {
            @display("p=131,246");
        }
        PC2: ANSA_Host {
            @display("p=285,246");
        }
        SW1: EtherSwitch {
            @display("p=205,169");
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
                @display("p=36,19");
        }
        scenarioManager: ScenarioManager {
            @display("p=36,69");
        }
    connections:
        R1.ethg++ <--> Eth100M <--> SW1.ethg++;
        R2.ethg++ <--> Eth100M <--> SW1.ethg++;
        R3.ethg++ <--> Eth100M <--> SW1.ethg++;
        SW1.ethg++ <--> Eth100M <--> PC1.ethg++;
        SW1.ethg++ <--> Eth100M <--> PC2.ethg++;
}
