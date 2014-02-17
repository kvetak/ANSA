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
        @display("i=block/network2;bgb=202,179");
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
        output ipsplitOut @labels(IPControlInfo/down);
    submodules:
        lispMapCache: LISPMapCache {
            @display("p=95,35");
        }
        lispCore: LISPCore {
            @display("p=95,120");
        }
    connections allowunconnected:

        lispCore.udpOut --> udpOut;
        lispCore.udpIn <-- udpIn;


}
