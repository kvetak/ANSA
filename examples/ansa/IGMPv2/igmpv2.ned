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

package inet.examples.ansa.IGMPv2;
import inet.ansa.networklayer.pim.PIMRouter;
import inet.ansa.nodes.inet.ANSAStandardHost;
import inet.examples.bgpv4.BGPUpdate.BGPRouter;
import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;
import ned.DatarateChannel;
import inet.nodes.ethernet.Eth100M;


//
// TODO documentation
//
network IGMP
{
    types:
        channel ethernetline extends DatarateChannel
        {
            delay = 0.1us;
        }

    submodules:
        ansaStandardHost: ANSAStandardHost {
            @display("p=68,101");
        }
        pimRouter: PIMRouter {
            @display("p=440,101;is=l");
            //forwardMulticast = true;
            multicastForward = true;
        }
    connections allowunconnected:
        pimRouter.ethg++ <--> Eth100M <--> ansaStandardHost.ethg++;
}

