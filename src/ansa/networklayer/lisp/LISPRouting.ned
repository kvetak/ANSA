//
// Copyright (C) 2013, 2014 Brno University of Technology
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
// @author Vladimir Vesely / ivesely@fit.vutbr.cz / http://www.fit.vutbr.cz/~ivesely/ 

package inet.ansa.networklayer.lisp;


module LISPRouting
{
    parameters:
        @display("i=block/network2;bgb=329,179,white");
        bool hasSiteDB = default(false);
        xml configData = default(xml("<LISP />"));
        string packetHandling = default("Vanilla"); //Either Vanilla or Symmetric

    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
        output ipv4Out;
        input ipv4In;
        output ipv6Out;
        input ipv6In;

    submodules:
        lispMapCache: LISPMapCache {
          parameters:  
            @display("p=163,41");
            configData = configData;
        }
        lispCore: LISPCore {
          parameters:  
            @display("p=163,108");
            configData = configData;
        }
        lispMapDatabase: LISPMapDatabase  {
          parameters:  
            @display("p=57,41");
            configData = configData;
        }
        lispSiteDatabase: LISPSiteDatabase if hasSiteDB {
          parameters:  
            @display("p=268,41");
            configData = configData;
        }
        
    connections allowunconnected:

        lispCore.udpOut --> udpOut;
        lispCore.udpIn <-- udpIn;

        lispCore.ipv4Out --> ipv4Out;
        lispCore.ipv6Out --> ipv6Out;
        ipv4In --> lispCore.ipv4In;
        ipv6In --> lispCore.ipv6In;
}
