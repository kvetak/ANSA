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

package ansa.networklayer.glbp;

import inet.applications.contract.IUDPApp;

simple GLBP like IUDPApp
{
    parameters:
        @display("i=block/network2");
        xml configData = default(xml("<GLBP />"));
//        string deviceId; 
        
//        bool jittered;     
        
        string interfaceTableModule = default(".interfaceTableModule");  
            
      	int glbpUdpPort			= default(3222);
      	string glbpMulticastAddress = default("224.0.0.102");	

	gates:
        input glbpIn[] @labels(UDPControlInfo/down);
        output glbpOut[] @labels(UDPControlInfo/up);       
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @lables(UDPControlInfo/down);	      
}