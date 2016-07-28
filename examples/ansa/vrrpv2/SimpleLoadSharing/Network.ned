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

package inet.examples.ansa.vrrpv2.SimpleLoadSharing;

import ansa.node.ANSA_Host;
import ansa.node.ANSA_VRRP_Router;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherSwitch;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;

network Network
{
    @display("bgb=300,314;bgl=2");
    submodules:
        GW1: ANSA_VRRP_Router {
            //@display("p=75,75");
            @display("p=50,50");
            gates:
                ethg[1];
        }
        GW2: ANSA_VRRP_Router {
            //@display("p=325,75");
            @display("p=250,50");
            gates:
                ethg[1];
        }
        SW: EtherSwitch {
            //@display("p=200,200");
            @display("p=150,150");
            gates:
                ethg[4];
        }
        Host1: ANSA_Host {
            //@display("p=200,350");
            @display("p=50,250");
            gates:
                ethg[1];
        }
        Host2: ANSA_Host {
            //@display("p=200,350");
            @display("p=250,250");
            gates:
                ethg[1];
        }
        Manager: ScenarioManager {
            //@display("p=75,350");
            @display("p=150,258");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
				config = xml("<config></config>");
                assignAddresses = false;
                assignDisjunctSubnetAddresses = false;
                addStaticRoutes = false;
                addDefaultRoutes = false;
                addSubnetRoutes = false;
                optimizeRoutes = false;            
            	@display("p=155,48");
        }
    connections:
        GW1.ethg[0] <--> Eth100M <--> SW.ethg[0];
        GW2.ethg[0] <--> Eth100M <--> SW.ethg[1];
        Host1.ethg[0] <--> Eth100M <--> SW.ethg[2];
        Host2.ethg[0] <--> Eth100M <--> SW.ethg[3];
}
