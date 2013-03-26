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

import inet.nodes.inet.StandardHost;
import inet.ansa.ANSASwitch;


network example
{
    submodules:
        ansaSwitch: ANSASwitch {
            // configFile = "config.xml"
            // switchID = "192.168.0.1"
            @display("p=258,47");
            gates:
                ethg[5];
        }
        Host1: StandardHost {
            @display("p=56,160");
        }
        Host2: StandardHost {
            @display("p=155,160");
        }
        Host3: StandardHost {
            @display("p=258,160");
        }
        Host4: StandardHost {
            @display("p=352,160");
        }
        Host5: StandardHost {
            @display("p=454,160");
        }
    connections:
        Host1.ethg++ <--> ansaSwitch.ethg[0];
        Host2.ethg++ <--> ansaSwitch.ethg[1];
        Host3.ethg++ <--> ansaSwitch.ethg[2];
        Host4.ethg++ <--> ansaSwitch.ethg[3];
        Host5.ethg++ <--> ansaSwitch.ethg[4];
}
