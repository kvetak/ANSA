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

package inet.examples.ansa.xmlparsing.devConfigTest;

import inet.ansa.nodes.ANSA_RouterRIPDual;

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.ethernet.EtherSwitch;
import inet.nodes.ethernet.Eth100M;

network DevConfigTest
{
    @display("bgb=440,246");
    submodules:
        configurator: IPv4NetworkConfigurator {
            parameters:
                config = xml("<config></config>");
                assignAddresses = false;
                assignDisjunctSubnetAddresses = false;
                addStaticRoutes = false;
                addDefaultRoutes = false;
                addSubnetRoutes = false;
                optimizeRoutes = false;
                @display("p=175,20");
        }
        R1: ANSA_RouterRIPDual {
            @display("p=184,88");
        }
        S1: EtherSwitch {
            @display("p=106,149");
        }
        S2: EtherSwitch {
            @display("p=276,155");
        }
    connections:
        R1.ethg++ <--> Eth100M <--> S1.ethg++;
        R1.ethg++ <--> Eth100M <--> S2.ethg++;
}
