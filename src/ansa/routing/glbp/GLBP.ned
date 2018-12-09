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


package ansa.routing.glbp;

import inet.applications.contract.IApp;

simple GLBP like IApp
{
    parameters:
        @display("i=block/routing");
        xml configData = default(xml("<GLBP />"));
        //        bool jittered;                    
        string interfaceTableModule = default(".interfaceTableModule");
        int glbpUdpPort = default(3222);
        string glbpMulticastAddress = default("224.0.0.102");

    gates:
        input glbpIn[];		/// from GLBPVR module to GLBP
        output glbpOut[]; 	/// from GLBP module to GLBPVR	 
        input socketIn @labels(UdpControlInfo/up);		///from Udp module to GLBP
        output socketOut @lables(UdpControlInfo/down);	///form GLBP module to Udp
}
