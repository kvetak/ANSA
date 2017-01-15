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

package ansa.routing.ospfv3;

import inet.networklayer.contract.INetworkLayer;



module OSPFv3Routing
{
    @display("bgb=510,238;i=block/network2");
    gates:
        input ipv6In @labels(OSPFv3Packet/up);
        output ipv6Out @labels(OSPFv3Packet/down);

    submodules:
        OSPFv3Splitter: OSPFv3Splitter {
            @display("p=254,176");
        }

    connections:
        ipv6In --> OSPFv3Splitter.ipIn;
        ipv6Out <-- OSPFv3Splitter.ipOut;
}
