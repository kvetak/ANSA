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

package inet.examples.ansa.lisp.lispIariaFunc;

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


network LispIariaFunc
{
    @display("bgb=623,506,white");
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
        xTR_A1: ANSA_xTR {
            @display("p=169,265");
            gates:
                ethg[2];
        }
        MRMS: ANSA_MRMS {
            @display("p=306,53");
        }
        xTR_B1: ANSA_xTR {
            @display("p=345,265");
            gates:
                ethg[2];
        }
        S1: EtherSwitch {
            @display("p=218,341");
        }
        S2: EtherSwitch {
            @display("p=394,341");
        }
        HostA: ANSA_DualStackHost {
            @display("p=218,407");
        }
        HostB: ANSA_DualStackHost {
            @display("p=394,407");
        }
        Core: RIPRouter {
            @display("p=306,145");
        }
        xTR_A2: ANSA_xTR {
            @display("p=267,265");
            gates:
                ethg[2];
        }
        xTR_B2: ANSA_xTR {
            @display("p=443,265");
            gates:
                ethg[2];
        }
    connections:
        S1.ethg++ <--> Eth100M <--> xTR_A1.ethg[1];
        S1.ethg++ <--> Eth100M <--> xTR_A2.ethg[1];
        S1.ethg++ <--> Eth100M <--> HostA.ethg++;
        S2.ethg++ <--> Eth100M <--> xTR_B1.ethg[1];
        S2.ethg++ <--> Eth100M <--> xTR_B2.ethg[1];
        S2.ethg++ <--> Eth100M <--> HostB.ethg++;
        Core.ethg++ <--> Eth100M <--> MRMS.ethg++;
        Core.ethg++ <--> Eth100M <--> xTR_A1.ethg[0];
        Core.ethg++ <--> Eth100M <--> xTR_A2.ethg[0];
        Core.ethg++ <--> Eth100M <--> xTR_B1.ethg[0];
        Core.ethg++ <--> Eth100M <--> xTR_B2.ethg[0];

}
