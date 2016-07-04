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

// Author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
// Babel routing module - Babel routing functionality

package ansa.routing.babel;

import ansa.routing.babel.BabelMain;
import ansa.routing.babel.cost.CostKoutofj;
import ansa.routing.babel.cost.CostEtx;

import inet.applications.contract.IUDPApp;


module BabelRouting like IUDPApp
{
    parameters:
        @display("i=block/network2;bgb=205,215,white");
        int udpPort = default(6696);
		xml configData = default(xmldoc("config.xml"));      
        
       	string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);        

    submodules:
        babelMain: BabelMain {
            @display("p=50,50;i=block/circle");
            interfaceTableModule = interfaceTableModule;
            routingTableModule = routingTableModule;
            configData = configData;
        }
        cost2outof3: CostKoutofj {
            @display("p=50,150;");
        }
        costetx: CostEtx {
            @display("p=150,150;");
        }
    connections:
        udpIn --> babelMain.udpIn;
        udpOut <-- babelMain.udpOut;
}
