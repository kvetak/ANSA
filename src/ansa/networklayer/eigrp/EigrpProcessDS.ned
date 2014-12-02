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

import inet.ansa.networklayer.eigrp.pdms.EigrpIpv6Pdm;
import inet.ansa.networklayer.eigrp.tables.EigrpIpv6NeighborTable;
import inet.ansa.networklayer.eigrp.tables.EigrpIpv6TopologyTable;

import inet.ansa.networklayer.ipv6.ANSARoutingTable6;

module eigrpProcessDS
{
    parameters:

        @display("bgb=494,229,white");
    gates:
        input splitterIn;
        input splitter6In;
        output splitterOut;
        output splitter6Out;

    submodules:
        // vlevo nazev, vpravo typ
        eigrpIpv4Pdm: EigrpIpv4Pdm {
            @display("p=64,66;i=block/circle");
        }
        eigrpInterfaceTable: EigrpInterfaceTable {
            @display("p=185,180");
        }
        eigrpIpv4NeighborTable: EigrpIpv4NeighborTable {
            @display("p=185,45");
        }
        eigrpIpv4TopologyTable: EigrpIpv4TopologyTable {
            @display("p=185,112");
        }
        eigrpRtp: EigrpRtp {
            @display("p=64,157;i=block/dispatch");
        }
		
		eigrpInterfaceTable6: EigrpInterfaceTable {
            @display("p=320,180");
        }
        eigrpIpv6Pdm: EigrpIpv6Pdm {
            @display("p=433,66;i=block/circle");
        }
        eigrpIpv6NeighborTable: EigrpIpv6NeighborTable {
            @display("p=320,45");
        }
        eigrpIpv6TopologyTable: EigrpIpv6TopologyTable {
            @display("p=320,112");
        }
        eigrpRtp6: EigrpRtp6 {
            @display("p=433,157;i=block/dispatch");
        }
    connections:
        splitterIn --> eigrpIpv4Pdm.splitterIn;
        splitterOut <-- eigrpIpv4Pdm.splitterOut;

        eigrpIpv4Pdm.rtpOut --> eigrpRtp.pdmIn;
        eigrpIpv4Pdm.rtpIn <-- eigrpRtp.pdmOut;

        splitter6In --> eigrpIpv6Pdm.splitterIn;
        splitter6Out <-- eigrpIpv6Pdm.splitterOut;

        eigrpIpv6Pdm.rtpOut --> eigrpRtp6.pdmIn;
        eigrpIpv6Pdm.rtpIn <-- eigrpRtp6.pdmOut;
}
