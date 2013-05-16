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

package inet.examples.ansa.MLDv1;
import inet.ansa.applications.ripng.RIPngRouter;
import inet.ansa.nodes.ipv6.ANSAStandardHost6;
import ned.DatarateChannel;
import inet.nodes.ethernet.Eth100M;


//
// TODO documentation
//
network MLD
{
    types:
        channel ethernetline extends DatarateChannel
        {
            delay = 0.1us;
        }

    submodules:
        ansaStandardHost6: ANSAStandardHost6 {
            @display("p=123,118");
        }
        riPngRouter: RIPngRouter {
            @display("p=628,118");
        }
    connections allowunconnected:
        ansaStandardHost6.ethg++ <--> Eth100M <--> riPngRouter.ethg++;
}

