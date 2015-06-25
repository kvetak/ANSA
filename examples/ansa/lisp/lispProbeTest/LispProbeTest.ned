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

package inet.examples.ansa.lisp.lispProbeTest;

import inet.ansa.applications.rip.RIPRouter;
import inet.ansa.nodes.ANSA_DualStackHost;
import inet.ansa.nodes.lisp.*;
import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.ethernet.EtherSwitch;
import inet.nodes.ethernet.Eth100M;
import inet.world.scenario.ScenarioManager;

network LispProbeTest
{
    @display("bgb=536,279,white");
    submodules:
        Host1: ANSA_DualStackHost {
            @display("p=74,168");
        }
        xTR1: ANSA_xTR {
            @display("p=198,168");
        }
        MRMS: ANSA_MRMS {
            @display("p=326,43");
        }
        Core: RIPRouter {
            @display("p=326,168");
        }
        xTR_Responder1: ANSA_xTR_Responder {
            @display("p=461,114");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                config = xml("<config></config>");
                assignAddresses = false;
                assignDisjunctSubnetAddresses = false;
                addStaticRoutes = false;
                addDefaultRoutes = false;
                addSubnetRoutes = false;
                optimizeRoutes = false;
                @display("p=46,43");
        }
        xTR_Responder2: ANSA_xTR_Responder {
            @display("p=461,224");
        }
    connections:
        Core.ethg++ <--> Eth100M <--> MRMS.ethg++;
        xTR1.ethg++ <--> Eth100M <--> Core.ethg++;
        Core.ethg++ <--> Eth100M <--> xTR_Responder2.ethg++;
        Core.ethg++ <--> Eth100M <--> xTR_Responder1.ethg++;        
        xTR1.ethg++ <--> Eth100M <--> Host1.ethg++;
}
