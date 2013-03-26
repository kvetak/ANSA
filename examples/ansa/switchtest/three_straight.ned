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


network Three_straight
{
    @display("bgb=580,594");
    submodules:
        ansaSwitch: ANSASwitch {
            @display("p=83,112");
        }
        ansaSwitch1: ANSASwitch {
            @display("p=294,67");
        }
        ansaSwitch2: ANSASwitch {
            @display("p=474,106");
        }
        ansaSwitch3: ANSASwitch {
            @display("p=203,187");
        }
        ansaSwitch4: ANSASwitch {
            @display("p=358,187");
        }
        ansaSwitch5: ANSASwitch {
            @display("p=100,282");
        }
        ansaSwitch6: ANSASwitch {
            @display("p=454,286");
        }
        ansaSwitch7: ANSASwitch {
            @display("p=282,295");
        }
        ansaSwitch8: ANSASwitch {
            @display("p=172,430");
        }
        ansaSwitch9: ANSASwitch {
            @display("p=377,424");
        }
        ansaSwitch10: ANSASwitch {
            @display("p=278,537");
        }
        Host1: StandardHost {
            @display("p=416,544");
        }
        Host2: StandardHost {
            @display("p=179,37");
        }
    connections:
        ansaSwitch.ethg++ <--> ansaSwitch1.ethg++;
        ansaSwitch1.ethg++ <--> ansaSwitch2.ethg++;
        ansaSwitch.ethg++ <--> ansaSwitch3.ethg++;
        ansaSwitch3.ethg++ <--> ansaSwitch4.ethg++;
        ansaSwitch4.ethg++ <--> ansaSwitch2.ethg++;
        ansaSwitch4.ethg++ <--> ansaSwitch1.ethg++;
        ansaSwitch1.ethg++ <--> ansaSwitch3.ethg++;
        ansaSwitch5.ethg++ <--> ansaSwitch.ethg++;
        ansaSwitch5.ethg++ <--> ansaSwitch3.ethg++;
        ansaSwitch5.ethg++ <--> ansaSwitch8.ethg++;
        ansaSwitch8.ethg++ <--> ansaSwitch7.ethg++;
        ansaSwitch7.ethg++ <--> ansaSwitch9.ethg++;
        ansaSwitch9.ethg++ <--> ansaSwitch6.ethg++;
        ansaSwitch9.ethg++ <--> ansaSwitch10.ethg++;
        ansaSwitch10.ethg++ <--> ansaSwitch8.ethg++;
        ansaSwitch7.ethg++ <--> ansaSwitch3.ethg++;
        ansaSwitch7.ethg++ <--> ansaSwitch4.ethg++;
        ansaSwitch4.ethg++ <--> ansaSwitch6.ethg++;
        ansaSwitch6.ethg++ <--> ansaSwitch2.ethg++;
        ansaSwitch10.ethg++ <--> ansaSwitch7.ethg++;
        Host2.ethg++ <--> ansaSwitch1.ethg++;
        Host1.ethg++ <--> ansaSwitch10.ethg++;
}
