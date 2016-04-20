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

package inet.examples.ansa.lldp.basic_switch;

import ansa.node.ANSA_EtherSwitch;
import inet.node.ethernet.Eth100M;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.common.scenario.ScenarioManager;
import inet.common.lifecycle.LifecycleController;

network simpleSwitch
{
    @display("bgb=521,431");
    submodules:

        S1: ANSA_EtherSwitch {
            parameters:
                @display("p=153,289");
            gates:
                ethg[3];
        }
        S2: ANSA_EtherSwitch {
            parameters:
                @display("p=324,289");
            gates:
                ethg[3];
        }
        S3: ANSA_EtherSwitch {
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
        

    connections allowunconnected:
        //S1.ethg[0] <--> Eth100M <--> S2.ethg[0];
        //S2.ethg[1] <--> Eth100M <--> S3.ethg[1];
        //S3.ethg[0] <--> Eth100M <--> S1.ethg[1];
}
