// The MIT License (MIT)
//
// Copyright (c) 2016 Brno University of Technology
//
//@author Vladimir Vesely (iveselyATfitDOTvutbrDOTcz)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
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
        string deviceId = default("");
        string deviceType = default("Router");        
        xml configFile = default(xmldoc("config.xml"));
        
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
            deviceId = deviceId;
            deviceType = deviceType;
            configFile = configFile;
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
