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

package inet.examples.ansa.lisp.lispSimple;

import inet.ansa.ANSASwitch;
import inet.ansa.applications.rip.RIPRouter;
import inet.ansa.nodes.ANSA_DualStackHost;
import inet.ansa.nodes.lisp.*;
import inet.ansa.nodes.inet.ANSAStandardHost;
import inet.ansa.nodes.ipv6.ANSAStandardHost6;
import inet.nodes.ethernet.EtherSwitch;
import inet.world.scenario.ScenarioManager;
import inet.nodes.ethernet.Eth100M;
import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;

network lispSimple
{
    @display("bgb=665,374,white");
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
                @display("p=55,34");
        }
        xTR_A: ANSA_xTR {
            @display("p=206,145");
        }
        MRMS: ANSA_MRMS {
            @display("p=337,34");
        }
        xTR_B: ANSA_xTR {
            @display("p=461,145");
        }
        S1: EtherSwitch {
            @display("p=206,244");
        }
        S2: EtherSwitch {
            @display("p=461,244");
        }
        Hv4_A: ANSA_DualStackHost {
            @display("p=206,338");
        }
        Hv6_A: ANSA_DualStackHost {
            @display("p=293,245");
        }
        Hv4_B: ANSA_DualStackHost {
            @display("p=461,338");
        }
        Hv6_B: ANSA_DualStackHost {
            @display("p=385,245");
        }
        Core: RIPRouter {
            @display("p=337,145");
        }
    connections:
        S1.ethg++ <--> Eth100M <--> xTR_A.ethg++;
        xTR_B.ethg++ <--> Eth100M <--> S2.ethg++;
        Hv4_A.ethg++ <--> Eth100M <--> S1.ethg++;
        S1.ethg++ <--> Eth100M <--> Hv6_A.ethg++;
        Hv4_B.ethg++ <--> Eth100M <--> S2.ethg++;
        Hv6_B.ethg++ <--> Eth100M <--> S2.ethg++;
        xTR_A.ethg++ <--> Eth100M <--> Core.ethg++;
        Core.ethg++ <--> Eth100M <--> xTR_B.ethg++;
        Core.ethg++ <--> Eth100M <--> MRMS.ethg++;
}
