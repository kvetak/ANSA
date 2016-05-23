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
//
// @file CDP.ned
// @author Tomas Rajca
//


package ansa.linklayer.cdp;
import ansa.linklayer.cdp.tables.CDPNeighbourTable;
import ansa.linklayer.cdp.tables.CDPInterfaceTable;
import ansa.linklayer.cdp.tables.CDPODRRouteTable;
import ansa.linklayer.cdp.CDPMain;

import inet.networklayer.contract.INetworkLayerLower;

module CDP like INetworkLayerLower
{
    parameters:
        @display("i=block/network2;bgb=417,163");

        string routingTableModule = default("");
        string deviceId = default("");
        string deviceType = default("");
        xml configFile = default(xmldoc("config.xml"));
        
        bool odr = default(false);
    	int timer @unit(s) = default(60s);
    	int holdTime @unit(s) = default(180s);
    	int version = default(2);
    	int ODRRouteInvalidTime = default(180);
    	int ODRRouteHolddownTime = default(0);
    	int ODRRouteFlushTime = default(240);
    	int maxDestinationPaths = default(4);
        int defaultRouteInvalide = default(180);
	
        **.deviceId = deviceId;
        **.deviceType = deviceType;
        **.configFile = default(xmldoc("config.xml"));
        *.cdpNeighbourTableModule = default("^.cdpNeighbourTable");
        *.cdpInterfaceTableModule = default("^.cdpInterfaceTable");
        *.cdpODRRouteTableModule = default("^.cdpODRRouteTable");
        *.routingTableModule = routingTableModule;
        *.odr = odr;
        *.timer = timer;
        *.holdTime = holdTime;
        *.version = version;
        *.ODRRouteInvalidTime = ODRRouteInvalidTime;
        *.ODRRouteHolddownTime = ODRRouteHolddownTime;
        *.ODRRouteFlushTime = ODRRouteFlushTime;
        *.maxDestinationPaths = maxDestinationPaths;
        *.defaultRouteInvalide = defaultRouteInvalide;
    gates:
        input ifIn[] @labels(INetworkDatagram/up);
        output ifOut[] @labels(INetworkDatagram/down);

    submodules:
        cdpNeighbourTable: CDPNeighbourTable {
            parameters:
                @display("p=213,29");
        }

        cdpInterfaceTable: CDPInterfaceTable {
            parameters:
                @display("p=213,107");
        }

        cdpODRRouteTable: CDPODRRouteTable {
            parameters:
                @display("p=326,75");
        }

        cdpMain: CDPMain {
            parameters:
                @display("p=111,75");
        }

    connections:
        for i=0..sizeof(ifIn)-1 {
            ifIn[i] --> { @display("m=w"); } --> cdpMain.ifIn++;
        }

        for i=0..sizeof(ifOut)-1 {
            cdpMain.ifOut++ --> { @display("m=w"); } --> ifOut[i];
        }
}
