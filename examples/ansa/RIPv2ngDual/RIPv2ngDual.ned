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

package inet.examples.ansa.RIPv2ngDual;

import inet.ansa.ANSASwitch;
import inet.ansa.nodes.ANSA_DualStackHost;
import inet.ansa.nodes.ANSA_RouterRIPDual;
import inet.ansa.nodes.inet.ANSAStandardHost;
import inet.ansa.nodes.ipv6.ANSAStandardHost6;
import inet.nodes.ethernet.EtherSwitch;
import inet.world.scenario.ScenarioManager;
import inet.nodes.ethernet.Eth100M;

//
// TODO Auto-generated network
//
network RIPv2ngDual
{
    @display("bgb=665,263");
    submodules:
        R1: ANSA_RouterRIPDual {
            @display("p=185,81");
        }
        R2: ANSA_RouterRIPDual {
            @display("p=333,81");
        }
        R3: ANSA_RouterRIPDual {
            @display("p=484,81");
        }
        S1: EtherSwitch {
            @display("p=118,141");
        }
        S2: EtherSwitch {
            @display("p=558,135");
        }
        Hv4_1: ANSA_DualStackHost {
            @display("p=59,214");
        }
        Hv6_1: ANSA_DualStackHost {
            @display("p=182,214");
        }
        Hv4_3: ANSA_DualStackHost {
            @display("p=502,214");
        }
        Hv6_3: ANSA_DualStackHost {
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
