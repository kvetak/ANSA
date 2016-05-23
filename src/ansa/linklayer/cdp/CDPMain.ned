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
// @file CDPMain.ned
// @author Tomas Rajca
//

package ansa.linklayer.cdp;

simple CDPMain
{
    parameters:
    	@display("i=block/network2");
    	
    	int timer @unit(s);
    	int holdTime @unit(s);
    	int version;
    	int ODRRouteInvalidTime;
    	int ODRRouteHolddownTime;
    	int ODRRouteFlushTime;
    	int maxDestinationPaths;
        int defaultRouteInvalide;
    	bool odr;
        string routingTableModule;
        
        string deviceId;
        string deviceType;
        xml configFile;
        
        string cdpNeighbourTableModule; 
        string cdpInterfaceTableModule; 
        string cdpODRRouteTableModule; 
	gates:
        input ifIn[];
        output ifOut[];
}