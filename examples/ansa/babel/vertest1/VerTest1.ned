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

package inet.examples.ansa.babel.vertest1;

import inet.ansa.applications.babel.BabelRouter;
import inet.ansa.nodes.ANSA_DualStackHost;
import inet.node.ethernet.Eth100M;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.common.scenario.ScenarioManager;
//import inet.world.radio.ChannelControl;

network VerTest1
{
    @display("bgb=521,431");
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
                @display("p=450,290");
        }

        R1: BabelRouter {
            parameters:
                @display("p=250,357");
            gates:
                ethg[3];
        }
        R2: BabelRouter {
            parameters:
                @display("p=250,207");
            gates:
                ethg[4];
        }
        
       
        LAN1: ANSA_DualStackHost {
            @display("p=50,357;i=cloud_s");
        }
        LAN2: ANSA_DualStackHost {
            @display("p=250,57;i=cloud_s");
        }

        scenarioManager: ScenarioManager {
            @display("p=450,357");
        }

    connections allowunconnected:
        R1.ethg[0] <--> Eth100M <--> R2.ethg[0];

        R1.ethg[2] <--> Eth100M <--> LAN1.ethg++;
        R2.ethg[3] <--> Eth100M <--> LAN2.ethg++;


}

