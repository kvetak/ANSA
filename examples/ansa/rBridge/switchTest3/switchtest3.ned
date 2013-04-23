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

package inet.examples.ansa.rBridge.switchTest3;

import inet.ansa.RBridge;
import inet.ansa.nodes.inet.AnsaHost;
//import inet.nodes.ethernet.Eth100M;
import ned.DatarateChannel;
import inet.ansa.ANSASwitch;


//
// TODO Auto-generated network
//
network switchtest3
{
    @display("bgb=509,354");
     types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
        }
    submodules:
        switch1: ANSASwitch {
            switchID = "switch1";
            configFile = "config.xml";
            @display("p=113,90");
            gates:
                ethg[3];
        }
        switch2: ANSASwitch {
            @display("p=269,90");
            gates:
                ethg[3];
        }
        switch3: ANSASwitch {
            @display("p=428,90");
            gates:
                ethg[2];
        }
        switch4: ANSASwitch {
            @display("p=269,223");
            gates:
                ethg[3];
        }
        switch5: ANSASwitch {
            @display("p=428,223");
            gates:
                ethg[3];
        }
        switch6: ANSASwitch {
            @display("p=113,223");
            gates:
                ethg[2];
        }
        ansaHost2: AnsaHost {
            @display("p=428,315");
            gates:
                ethg[1];
        }
        ansaHost1: AnsaHost {
            @display("p=39,90");
            gates:
                ethg[1];
        }
    connections:
        switch1.ethg[0] <--> switch2.ethg[0];
        switch1.ethg[1] <--> switch6.ethg[1];
        switch2.ethg[1] <--> switch4.ethg[0];
        switch4.ethg[1] <--> switch6.ethg[0];
        switch4.ethg[2] <--> switch5.ethg[0];
        switch2.ethg[2] <--> switch3.ethg[0];
        switch5.ethg[1] <--> switch3.ethg[1];
        ansaHost1.ethg[0] <--> switch1.ethg[2];
        ansaHost2.ethg[0] <--> switch5.ethg[2];
//        switch3.ethg[0] <--> n/a: DatarateChannel <--> switch5.ethg[0];
//        switch5.ethg[0] <--> n/a: DatarateChannel <--> switch4.ethg[0];
//        switch4.ethg[0] <--> n/a: DatarateChannel <--> switch2.ethg[0];
//        switch1.ethg[0] <--> n/a: DatarateChannel <--> switch2.ethg[0];
//        switch6.ethg[0] <--> n/a: DatarateChannel <--> switch4.ethg[0];
//        switch6.ethg[0] <--> n/a: DatarateChannel <--> switch1.ethg[0];
//        switch5.ethg[0] <--> n/a: DatarateChannel <--> ansaHost2.ethg[0];
}
