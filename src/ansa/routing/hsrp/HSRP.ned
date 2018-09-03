//
// Copyright (C) 2009 - today, Brno University of Technology, Czech Republic
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
// @author Jan Holusa
// @author Vladimir Vesely (ivesely@fit.vutbr.cz)
// @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3


package ansa.routing.hsrp;

import inet.applications.contract.IApp;

simple HSRP like IApp
{
    parameters:
        @display("i=block/routing");
        xml configData = default(xml("<HSRP />"));
        string interfaceTableModule = default(".interfaceTableModule");

    gates:
        input hsrpIn[];					///from HSRPVR to GLBP
        output hsrpOut[];    			///from GLBP module to HSRPVR
        input socketIn @labels(UDPControlInfo/up);		///from UDP module to GLBP
        output socketOut @lables(UDPControlInfo/down);	///from GLBP module to UDP
}
