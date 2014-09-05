//
// Copyright (C) 2013 Brno University of Technology
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
//@author Vladimir Vesely (<a href="mailto:ivesely@fit.vutbr.cz">ivesely@fit.vutbr.cz</a>)

package inet.ansa.networklayer.lisp;


module LISPRouting
{
    parameters:
        @display("i=block/network2;bgb=256,179,white");
        bool hasMapDB = default(false);

    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
        output ipv4Out;
        input ipv4In;
        output ipv6Out;
        input ipv6In;

    submodules:
        lispMapCache: LISPMapCache {
            @display("p=63,40");
        }
        lispCore: LISPCore {
            @display("p=124,109");
        }
        lispMapDatabase: LISPMapDatabase if hasMapDB {
            @display("p=185,40");
        }

    connections allowunconnected:

        lispCore.udpOut --> udpOut;
        lispCore.udpIn <-- udpIn;

        lispCore.ipv4Out --> ipv4Out;
        lispCore.ipv6Out --> ipv6Out;
        ipv4In --> lispCore.ipv4In;
        ipv6In --> lispCore.ipv6In;
}
