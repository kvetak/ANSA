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

package inet.examples.ansa.vrrpv2.StaticRoutingLoadSharing;

import ansa.node.ANSA_Host;
import ansa.node.ANSA_VRRP_Router;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherSwitch;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;

network Network
{
    @display("bgb=300,415;bgl=2");
    submodules:
        ISP: ANSA_VRRP_Router {
            @display("p=150,50");
            gates:
                ethg[2];
        }
        GW1: ANSA_VRRP_Router {
            @display("p=50,150");
            gates:
                ethg[2];
        }
        GW2: ANSA_VRRP_Router {
            @display("p=250,150");
            gates:
                ethg[2];
        }
        SW: EtherSwitch {
            @display("p=150,250");
            gates:
                ethg[4];
        }
        Host1: ANSA_Host {
            @display("p=50,350");
            gates:
                ethg[1];
        }
        Host2: ANSA_Host {
            @display("p=250,350");
            gates:
                ethg[1];
        }
        Manager: ScenarioManager {
            @display("p=250,50");
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
            	@display("p=50,50");
        }
    connections:
        ISP.ethg[0] <--> Eth100M <--> GW1.ethg[0];
        ISP.ethg[1] <--> Eth100M <--> GW2.ethg[0];

        GW1.ethg[1] <--> Eth100M <--> SW.ethg[0];
        GW2.ethg[1] <--> Eth100M <--> SW.ethg[1];

        Host1.ethg[0] <--> Eth100M <--> SW.ethg[2];
        Host2.ethg[0] <--> Eth100M <--> SW.ethg[3];
}
