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

package inet.examples.ansa.rBridge.multidest;

import inet.ansa.RBridge;
import inet.ansa.nodes.inet.AnsaHost;
import inet.nodes.ethernet.Eth100M;


//
// TODO Auto-generated network
//
network MultiDest
{
    @display("bgb=509,354");
    submodules:
        rBridge1: RBridge {
            @display("p=113,90");
            gates:
                ethg[4];
        }
        rBridge2: RBridge {
            @display("p=269,90");
            gates:
                ethg[3];
        }
        rBridge3: RBridge {
            @display("p=428,90");
            gates:
                ethg[2];
        }
        rBridge4: RBridge {
            @display("p=269,223");
            gates:
                ethg[3];
        }
        rBridge5: RBridge {
            @display("p=428,223");
            gates:
                ethg[2];
        }
        rBridge6: RBridge {
            @display("p=113,223");
            gates:
                ethg[2];
        }
        ansaHost2: AnsaHost {
            @display("p=113,26");
            gates:
                ethg[1];
        }
        ansaHost1: AnsaHost {
            @display("p=39,90");
            gates:
                ethg[1];
        }
    connections:
        rBridge1.ethg[0] <--> Eth100M <--> rBridge2.ethg[0];
        rBridge1.ethg[1] <--> Eth100M <--> rBridge6.ethg[1];
        rBridge2.ethg[1] <--> Eth100M <--> rBridge4.ethg[0];
        rBridge4.ethg[1] <--> Eth100M <--> rBridge6.ethg[0];
        rBridge4.ethg[2] <--> Eth100M <--> rBridge5.ethg[0];
        rBridge2.ethg[2] <--> Eth100M <--> rBridge3.ethg[0];
        rBridge5.ethg[1] <--> Eth100M <--> rBridge3.ethg[1];
        ansaHost1.ethg[0] <--> Eth100M <--> rBridge1.ethg[2];
        ansaHost2.ethg[0] <--> Eth100M <--> rBridge1.ethg[3];
}
