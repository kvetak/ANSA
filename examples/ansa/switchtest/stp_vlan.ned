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
//import inet.ansa.ANSARouter;
import inet.ansa.ANSASwitch;
import inet.nodes.inet.StandardHost;


network stp_vlan
{
    @display("bgb=431,235");
    submodules:

        Office1: StandardHost {
            @display("p=42,38;i=,#FF8040");
        }
        Server1: StandardHost {
            @display("p=42,115;i=,#FF8040");
        }
        Server2: StandardHost {
            @display("p=42,178;i=,#80FF80");
        }
        Server3: StandardHost {
            @display("p=384,178;i=,#FF8040");
        }
        Server4: StandardHost {
            @display("p=384,115;i=,#80FF80");
        }
        Office2: StandardHost {
            @display("p=384,38;i=,#80FF80");
        }
        HQ1: ANSASwitch {
            parameters:
                switchID = "hq1";
                configFile = "stp_vlan.xml";
                @display("p=219,38");
            gates:
                ethg[4];
        }
        Branch1: ANSASwitch {
            parameters:
                switchID = "branch1";
                configFile = "stp_vlan.xml";
                @display("p=116,38;i=,#FF8040");
            gates:
                ethg[3];
        }
        Branch2: ANSASwitch {
            parameters:
                switchID = "branch2";
                configFile = "stp_vlan.xml";
                @display("p=310,38;i=,#80FF80");
            gates:
                ethg[3];
        }
        HQ2: ANSASwitch {
            parameters:
                switchID = "hq2";
                configFile = "stp_vlan.xml";
                @display("p=116,178");
            gates:
                ethg[5];
        }
        HQ3: ANSASwitch {
            parameters:
                switchID = "hq3";
                configFile = "stp_vlan.xml";
                @display("p=310,178");
            gates:
                ethg[5];
        }
        LB1: LinkBreaker {
            @display("p=169,103");
        }
    connections:

		// Redundant triangle in HQ
        HQ1.ethg[0] <--> LB1.A;
        LB1.B <--> HQ2.ethg[0];
        HQ1.ethg[1] <--> HQ3.ethg[0];
        HQ2.ethg[1] <--> HQ3.ethg[1];

        // Branch connections
        HQ1.ethg[3] <--> Branch2.ethg[0];
        HQ1.ethg[2] <--> Branch1.ethg[0];
        Branch1.ethg[1] <--> HQ2.ethg[2];
        Branch2.ethg[1] <--> HQ3.ethg[2];

        // Branch hosts
        Branch1.ethg[2] <--> Office1.ethg++;
        Branch2.ethg[2] <--> Office2.ethg++;

        // Server connections
        HQ2.ethg[3] <--> Server1.ethg++;
        HQ2.ethg[4] <--> Server2.ethg++;
        HQ3.ethg[3] <--> Server3.ethg++;
        HQ3.ethg[4] <--> Server4.ethg++;
}
