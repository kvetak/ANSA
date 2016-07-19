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

package ansa.routing.eigrp;

import ansa.routing.eigrp.pdms.EigrpIpv4Pdm;
import ansa.routing.eigrp.tables.EigrpInterfaceTable;
import ansa.routing.eigrp.tables.EigrpIpv4NeighborTable;
import ansa.routing.eigrp.tables.EigrpIpv4TopologyTable;

import ansa.routing.eigrp.pdms.EigrpIpv6Pdm;
import ansa.routing.eigrp.tables.EigrpIpv6NeighborTable;
import ansa.routing.eigrp.tables.EigrpIpv6TopologyTable;

module EigrpProcessDS
{
    parameters:
        @display("bgb=494,229,white;i=block/network2");
        bool enableIPv4 = default(true);
        bool enableIPv6 = default(true);
        xml configData = default(xmldoc("config.xml"));
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;

    gates:
        input splitterIn;
        input splitter6In;
        output splitterOut;
        output splitter6Out;

    submodules:
        // vlevo nazev, vpravo typ
        eigrpIpv4Pdm: EigrpIpv4Pdm if enableIPv4 {
            @display("p=64,66;i=block/circle");
            configData = configData;
            routingTableModule = routingTableModule;
            interfaceTableModule = interfaceTableModule;
        }
        eigrpInterfaceTable: EigrpInterfaceTable if enableIPv4 {
            @display("p=185,180");
        }
        eigrpIpv4NeighborTable: EigrpIpv4NeighborTable if enableIPv4 {
            @display("p=185,45");
        }
        eigrpIpv4TopologyTable: EigrpIpv4TopologyTable if enableIPv4 {
            @display("p=185,112");
        }
        eigrpRtp: EigrpRtp if enableIPv4 {
            @display("p=64,157;i=block/dispatch");
        }

        eigrpInterfaceTable6: EigrpInterfaceTable if enableIPv6 {
            @display("p=320,180");
        }
        eigrpIpv6Pdm: EigrpIpv6Pdm if enableIPv6 {
            @display("p=433,66;i=block/circle");
            configData = configData;
            routingTableModule = routingTableModule;
            interfaceTableModule = interfaceTableModule;
        }
        eigrpIpv6NeighborTable: EigrpIpv6NeighborTable if enableIPv6 {
            @display("p=320,45");
        }
        eigrpIpv6TopologyTable: EigrpIpv6TopologyTable if enableIPv6 {
            @display("p=320,112");
        }
        eigrpRtp6: EigrpRtp6 if enableIPv6 {
            @display("p=433,157;i=block/dispatch");
        }
    connections allowunconnected:
        splitterIn --> eigrpIpv4Pdm.splitterIn if enableIPv4;
        splitterOut <-- eigrpIpv4Pdm.splitterOut if enableIPv4;

        eigrpIpv4Pdm.rtpOut --> eigrpRtp.pdmIn if enableIPv4;
        eigrpIpv4Pdm.rtpIn <-- eigrpRtp.pdmOut if enableIPv4;

        splitter6In --> eigrpIpv6Pdm.splitterIn if enableIPv6;
        splitter6Out <-- eigrpIpv6Pdm.splitterOut if enableIPv6;

        eigrpIpv6Pdm.rtpOut --> eigrpRtp6.pdmIn if enableIPv6;
        eigrpIpv6Pdm.rtpIn <-- eigrpRtp6.pdmOut if enableIPv6;
}
