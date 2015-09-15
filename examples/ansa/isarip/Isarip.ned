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

package inet.examples.ansa.isarip;

import inet.ansa.applications.rip.RIPRouter;
import inet.ansa.nodes.inet.ANSAStandardHost;
import inet.node.bgp.BGPRouter;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherLink;
import inet.node.ethernet.EtherSwitch;


//
// TODO Auto-generated network
//
network Isarip
{
    @display("bgb=556,212");
    submodules:
        R1: RIPRouter {
            @display("p=162,34");
        }
        R2: RIPRouter {
            @display("p=376,34");
        }
        R3: RIPRouter {
            @display("p=269,110");
        }
        S1: EtherSwitch {
            @display("p=72,81");
        }
        S2: EtherSwitch {
            @display("p=455,81");
        }
        PC1: ANSAStandardHost {
            @display("p=45,169");
        }
        PC2: ANSAStandardHost {
            @display("p=474,169");
        }
    connections:
        R3.ethg++ <--> Eth100M <--> R1.ethg++;
        R3.ethg++ <--> Eth100M <--> R2.ethg++;
        R1.ethg++ <--> Eth100M <--> S1.ethg++;
        R2.ethg++ <--> Eth100M <--> S2.ethg++;
        S2.ethg++ <--> Eth100M <--> PC2.ethg++;
        S1.ethg++ <--> Eth100M <--> PC1.ethg++;
}
