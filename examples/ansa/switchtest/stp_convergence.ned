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

package inet.examples.ansa.switchtest;

import inet.ansa.util.linkBreaker.LinkBreaker;
import inet.nodes.inet.StandardHost;
import inet.nodes.ethernet.EtherHost;
//import inet.ansa.AnsaDualStackHost;
import inet.ansa.ANSASwitch;


network stp_convergence
{
    @display("bgb=309,273");
    submodules:
        Switch1: ANSASwitch {
            switchID = "Switch1";
            configFile = "stp_convergence.xml";
            @display("p=157,102");
            gates:
                ethg[3];
        }
        Switch2: ANSASwitch {
            switchID = "Switch2";
            configFile = "stp_convergence.xml";
            @display("p=96,175");
            gates:
                ethg[3];
        }
        Switch3: ANSASwitch {
            switchID = "Switch3";
            configFile = "stp_convergence.xml";
            @display("p=205,175");
            gates:
                ethg[3];
        }
        Host1: StandardHost {
            @display("p=157,36");
        }
        Host2: StandardHost {
            @display("p=35,214");
        }
        Host3: StandardHost {
            @display("p=266,214");
        }
        l: LinkBreaker {
            @display("p=184,141");
        }
    connections:
        Switch1.ethg[0] <--> Switch2.ethg[0];
        Switch2.ethg[1] <--> Switch3.ethg[1];
        Switch3.ethg[0] <--> l.B;
        l.A <--> Switch1.ethg[1];
        Host1.ethg++ <--> Switch1.ethg[2];
        Host2.ethg++ <--> Switch2.ethg[2];
        Host3.ethg++ <--> Switch3.ethg[2];
}
