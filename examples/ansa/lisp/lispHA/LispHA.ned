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

package inet.examples.ansa.lisp.lispHA;

import inet.ansa.ANSASwitch;
import inet.ansa.applications.rip.RIPRouter;
import inet.ansa.nodes.ANSA_DualStackHost;
import inet.ansa.nodes.lisp.ANSA_xTR;
import inet.ansa.nodes.inet.ANSAStandardHost;
import inet.ansa.nodes.ipv6.ANSAStandardHost6;
import inet.nodes.ethernet.EtherSwitch;
import inet.world.scenario.ScenarioManager;
import inet.nodes.ethernet.Eth100M;
import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;

network LispHA
{
    @display("bgb=623,387");
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
            @display("p=186,92");
            gates:
                ethg[3];
        }
        MRMS: ANSA_xTR {
            @display("p=306,34");
        }
        xTR_B1: ANSA_xTR {
            @display("p=434,92");
            gates:
                ethg[3];
        }
        S1: EtherSwitch {
            @display("p=86,140");
        }
        S2: EtherSwitch {
            @display("p=546,140");
        }
        Hv4_A: ANSA_DualStackHost {
            @display("p=27,277");
        }
        Hv6_A: ANSA_DualStackHost {
            @display("p=150,277");
        }
        Hv4_B: ANSA_DualStackHost {
            @display("p=490,267");
        }
        Hv6_B: ANSA_DualStackHost {
            @display("p=589,267");
        }
        Core: RIPRouter {
            @display("p=306,145");
        }
        xTR_A2: ANSA_xTR {
            @display("p=186,176");
            gates:
                ethg[3];
        }
        xTR_B2: ANSA_xTR {
            @display("p=434,176");
            gates:
                ethg[3];
        }
    connections:
        xTR_A1.ethg[2] <--> Eth100M <--> xTR_A2.ethg[2];
        xTR_B1.ethg[2] <--> Eth100M <--> xTR_B2.ethg[2];
        
        S1.ethg++ <--> Eth100M <--> xTR_A1.ethg[1];
        S1.ethg++ <--> Eth100M <--> xTR_A2.ethg[1];
        S1.ethg++ <--> Eth100M <--> Hv4_A.ethg++;        
        S1.ethg++ <--> Eth100M <--> Hv6_A.ethg++;        

        S2.ethg++ <--> Eth100M <--> xTR_B1.ethg[1];
        S2.ethg++ <--> Eth100M <--> xTR_B2.ethg[1];        
        S2.ethg++ <--> Eth100M <--> Hv4_B.ethg++;
        S2.ethg++ <--> Eth100M <--> Hv6_B.ethg++;

        Core.ethg++ <--> Eth100M <--> MRMS.ethg++;
        Core.ethg++ <--> Eth100M <--> xTR_A1.ethg[0];
        Core.ethg++ <--> Eth100M <--> xTR_A2.ethg[0];
        Core.ethg++ <--> Eth100M <--> xTR_B1.ethg[0];
        Core.ethg++ <--> Eth100M <--> xTR_B2.ethg[0];
        
}
