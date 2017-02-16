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

package inet.examples.ansa.Basics.SimpleRelay;
import ned.DatarateChannel;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.networklayer.configurator.ipv6.FlatNetworkConfigurator6;

import ansa.node.ANSA_Host;
import ansa.node.ANSA_Router;
import inet.node.ethernet.EtherSwitch;


network SimpleRelay
{
    @display("bgb=358,329");
    types:
        channel Channel extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;

        }
    submodules:
        ClientA: ANSA_Host {
            @display("p=57,268");
        }
        ClientB: ANSA_Host {
            @display("p=195,268");
        }
        Router: ANSA_Router {
            @display("p=128,164");
        }
        Server: ANSA_Host {
            @display("p=128,36");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                config = xml("<config>"+
                            "<interface among='ClientA Router' address='192.168.1.x' netmask='255.255.255.0' />"+
                            "<interface among='ClientB Router' address='192.168.2.x' netmask='255.255.255.0' />"+
                            "<interface among='Server Router'  address='192.168.3.x' netmask='255.255.255.0' />"+
                            "<route hosts='ClientA ClientB Server' destination='*' netmask='0.0.0.0' interface='eth0' />"+
                            "</config>");
                addStaticRoutes = false;
                addDefaultRoutes = false;
                @display("p=298,128");
        }
        configurator6: FlatNetworkConfigurator6 {
            @display("p=298,176;is=s");
        }

    connections:
        Router.ethg++ <--> Channel <--> Server.ethg++;
        ClientA.ethg++ <--> Channel <--> Router.ethg++;
        ClientB.ethg++ <--> Channel <--> Router.ethg++;
}
