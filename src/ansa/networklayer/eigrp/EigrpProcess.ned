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

package inet.ansa.networklayer.eigrp;

import inet.ansa.networklayer.eigrp.pdms.EigrpIpv4Pdm;
import inet.ansa.networklayer.eigrp.tables.EigrpInterfaceTable;
import inet.ansa.networklayer.eigrp.tables.EigrpIpv4NeighborTable;
import inet.ansa.networklayer.eigrp.tables.EigrpIpv4TopologyTable;


module eigrpProcess
{
    parameters:

        @display("bgb=263,229");
    gates:
        input splitterIn;
        output splitterOut;

    submodules:
        // vlevo nazev, vpravo typ
        pdmv4: EigrpIpv4Pdm {
            @display("p=78,68");
        }
        eigrpInterfaceTable: EigrpInterfaceTable {
            @display("p=207,188");
        }
        eigrpIpv4NeighborTable: EigrpIpv4NeighborTable {
            @display("p=196,29");
        }
        eigrpIpv4TopologyTable: EigrpIpv4TopologyTable {
            @display("p=207,105");
        }
    connections:
        splitterIn --> pdmv4.rtpIn;
        splitterOut <-- pdmv4.rtpOut;
}
