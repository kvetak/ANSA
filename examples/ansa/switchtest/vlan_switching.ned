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


import inet.ansa.ANSASwitch;
import inet.nodes.inet.StandardHost;
//import inet.world.ChannelInstaller;
import inet.world.scenario.ScenarioManager;
import ned.DatarateChannel;

network vlan_switching
{
    parameters:
        @display("p=10,10;b=712,152;bgb=419,337");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
        }
    submodules:

        // Switch
        Switch1: ANSASwitch {
            parameters:
                switchID = "Switch1";
                configFile = "vlan_switching.xml";
                @display("p=117,199");
            gates:
                ethg[4];
        }

        // Switch
        Switch2: ANSASwitch {
            parameters:
                switchID = "Switch2";
                configFile = "vlan_switching.xml";
                @display("p=219,135");
            gates:
                ethg[5];
        }

        Switch3: ANSASwitch {
            parameters:
                switchID = "Switch3";
                configFile = "vlan_switching.xml";
                @display("p=310,199");
            gates:
                ethg[4];
        }

        // Hosts
        Host1: StandardHost {
            parameters:
                @display("p=54,283;i=device/pc2");
            gates:
                ethg[1];
        }
        Host2: StandardHost {
            parameters:
                @display("p=117,283;i=device/pc2");
            gates:
                ethg[1];
        }
        Host3: StandardHost {
            parameters:
                @display("p=172,283;i=device/pc2");
            gates:
                ethg[1];
        }
        Host4: StandardHost {
            parameters:
                @display("p=166,46;i=device/pc2");
            gates:
                ethg[1];
        }
        Host5: StandardHost {
            parameters:
                @display("p=219,46;i=device/pc2");
            gates:
                ethg[1];
        }
        Host6: StandardHost {
            parameters:
                @display("p=271,46;i=device/pc2");
            gates:
                ethg[1];
        }
        Host7: StandardHost {
            parameters:
                @display("p=258,283;i=device/pc2");
            gates:
                ethg[1];
        }
        Host8: StandardHost {
            parameters:
                @display("p=365,283;i=device/pc2");
            gates:
                ethg[1];
        }
        Host9: StandardHost {
            parameters:
                @display("p=310,283;i=device/pc2");
            gates:
                ethg[1];
        }


    connections:

		// Switch links (shared VLANs)
        Switch1.ethg[0] <--> { @display("ls=,3"); } <--> Switch2.ethg[0];
        Switch3.ethg[0] <--> { @display("ls=,3"); } <--> Switch2.ethg[1];

		// Switch 1 Hosts
        Host1.ethg[0] <--> Switch1.ethg[1];
        Host2.ethg[0] <--> Switch1.ethg[2];
        Host3.ethg[0] <--> Switch1.ethg[3];

        // Switch 2 Hosts
        Host4.ethg[0] <--> Switch2.ethg[2];
        Host5.ethg[0] <--> Switch2.ethg[3];
        Host6.ethg[0] <--> Switch2.ethg[4];

        // Switch 3 Hosts
        Host7.ethg[0] <--> Switch3.ethg[1];
        Host8.ethg[0] <--> Switch3.ethg[2];
        Host9.ethg[0] <--> Switch3.ethg[3];



}

