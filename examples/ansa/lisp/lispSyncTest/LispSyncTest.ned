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

package inet.examples.ansa.lisp.lispSyncTest;

import ansa.node.ANSA_Host;
import ansa.node.*;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.ethernet.EtherSwitch;
import inet.node.ethernet.Eth100M;

network LispSyncTest
{
    @display("bgb=590,279,white");
    submodules:
        SW: EtherSwitch {
            @display("p=132,136");
        }
        Host1: ANSA_Host {
            @display("p=132,52");
        }
        Host2: ANSA_Host {
            @display("p=132,226");
        }
        xTR1: ANSA_LISP_VRRP {
            @display("p=244,87");
        }
        xTR2: ANSA_LISP_VRRP {
            @display("p=244,191");
        }
        MRMS: ANSA_MRMS {
            @display("p=360,43");
        }
        Core: ANSA_Router {
            @display("p=360,137");
        }
        xTR_Responder: ANSA_xTR_Responder {
            @display("p=494,137");
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
                @display("p=360,239");
        }
        scenarioManager: ScenarioManager {
            @display("p=452,239");
        }
    connections:
        Core.ethg++ <--> Eth100M <--> MRMS.ethg++;
        xTR1.ethg++ <--> Eth100M <--> Core.ethg++;
        xTR2.ethg++ <--> Eth100M <--> Core.ethg++;
        Core.ethg++ <--> Eth100M <--> xTR_Responder.ethg++;
        xTR1.ethg++ <--> Eth100M <--> SW.ethg++;
        xTR2.ethg++ <--> Eth100M <--> SW.ethg++;
        SW.ethg++ <--> Eth100M <--> Host1.ethg++;
        SW.ethg++ <--> Eth100M <--> Host2.ethg++;
        xTR1.ethg++ <--> Eth100M <--> xTR2.ethg++;
}
