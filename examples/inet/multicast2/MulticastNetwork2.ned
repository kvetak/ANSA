//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


package inet.examples.inet.multicast2;

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;
import ned.DatarateChannel;


network MulticastNetwork2
{
    types:
        channel ethernet extends DatarateChannel
        {
            delay = normal(0.00015s, 0.00005s);
            datarate = 10Mbps;
        }
        channel dialup extends DatarateChannel
        {
            delay = normal(0.004s, 0.0018s);
            datarate = 64kbps;
        }
        channel fiberrouter extends DatarateChannel
        {
            delay = normal(0.000001s, 0.0000001s);
            datarate = 512Mbps;
        }
    submodules:
        host1: StandardHost {
            parameters:
                IPForward = false;
                @display("p=125,52;i=device/pc2");
            gates:
                pppg[1];
        }
        router1: Router {
            parameters:
                forwardMulticast = true;
                IGMPVersion = 3;
                @display("p=124,135");
            gates:
                pppg[1];
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=275,26");
        }
    connections:
        // * Domain 1 * (172.0.0.x)
        host1.pppg[0] <--> ethernet <--> router1.pppg[0];
}
