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

package ansa.examples.ansa.lisp.lisptest;

import ansa.node.ANSA_Host;
import ansa.node.*;
import inet.node.ethernet.EtherSwitch;
import inet.node.ethernet.Eth100M;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;

network LispTest
{
    @display("bgb=665,263");
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
                @display("p=175,20");
        }        
        R1: ANSA_xTR {
            @display("p=185,81");
        }
        R2: ANSA_xTR {
            @display("p=333,81");
        }
        R3: ANSA_xTR {
            @display("p=484,81");
        }
        S1: EtherSwitch {
            @display("p=118,141");
        }
        S2: EtherSwitch {
            @display("p=558,135");
        }
        Hv4_1: ANSA_Host {
            @display("p=59,214");
        }
        Hv6_1: ANSA_Host {
            @display("p=182,214");
        }
        Hv4_3: ANSA_Host {
            @display("p=502,214");
        }
        Hv6_3: ANSA_Host {
            @display("p=605,214");
        }
    connections:
        R1.ethg++ <--> Eth100M <--> R2.ethg++;
        R2.ethg++ <--> Eth100M <--> R3.ethg++;
        S1.ethg++ <--> Eth100M <--> R1.ethg++;
        R3.ethg++ <--> Eth100M <--> S2.ethg++;
        Hv4_1.ethg++ <--> Eth100M <--> S1.ethg++;
        S1.ethg++ <--> Eth100M <--> Hv6_1.ethg++;
        Hv4_3.ethg++ <--> Eth100M <--> S2.ethg++;
        Hv6_3.ethg++ <--> Eth100M <--> S2.ethg++;
}
