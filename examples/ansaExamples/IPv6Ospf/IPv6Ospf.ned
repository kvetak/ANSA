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

package inet.examples.ansaExamples.IPv6Ospf;

import inet.ansa.AnsaDualStackRouter;
import inet.nodes.ethernet.EtherSwitch;
import inet.nodes.inet.ExtRouter;


network IPv6Ospf
{
    submodules:
        R1: AnsaDualStackRouter {
            @display("p=248,118");
        }
        R2: AnsaDualStackRouter {
            @display("p=132,164");
        }
        R3: AnsaDualStackRouter {
            @display("p=362,164");
        }
        R4: AnsaDualStackRouter {
            @display("p=171,302");
        }
        R5: AnsaDualStackRouter {
            @display("p=323,302");
        }
        SW: EtherSwitch;
    connections allowunconnected:
        R1.ethg++ <--> SW.ethg++;
        R2.ethg++ <--> SW.ethg++;
        R3.ethg++ <--> SW.ethg++;
        R4.ethg++ <--> SW.ethg++;
        R5.ethg++ <--> SW.ethg++;
}
