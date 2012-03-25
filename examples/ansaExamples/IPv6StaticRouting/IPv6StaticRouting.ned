//
// Marek Cerny, 2MSK
// FIT VUT 2011
//
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

package inet.examples.ansaExamples.IPv6StaticRouting;

import inet.ansa.AnsaDualStackRouter;
import inet.ansa.AnsaDualStackHost;
import inet.nodes.ethernet.EtherSwitch;
import inet.world.ScenarioManager;

network IPv6StaticRouting
{
    submodules:
        scenarioManager: ScenarioManager;
        H1: AnsaDualStackHost {
            @display("p=149,235");
        }
        H2: AnsaDualStackHost {
            @display("p=120,158");
        }
        H3: AnsaDualStackHost {
            @display("p=139,70");
        }
        R1: AnsaDualStackRouter {
            @display("p=324,235");
        }
        R2: AnsaDualStackRouter {
            @display("p=454,242");
        }
        R3: AnsaDualStackRouter {
            @display("p=396,140");
        }
        S1: AnsaDualStackHost {
            @display("p=553,205");
        }
        SW1: EtherSwitch {
            @display("p=245,157");
        }
    connections allowunconnected:
        R1.ethg++ <--> R2.ethg++;
        R1.ethg++ <--> R3.ethg++;
        R2.ethg++ <--> R3.ethg++;
        R2.ethg++ <--> S1.ethg++;
        R1.ethg++ <--> SW1.ethg++;

        H1.ethg++ <--> SW1.ethg++;
        H2.ethg++ <--> SW1.ethg++;
        H3.ethg++ <--> SW1.ethg++;
}
