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

package inet.ansa.nodes.lisp;

import inet.linklayer.loopback.LoopbackInterface;


module ANSA_xTR_Responder extends ANSA_xTR
{
    parameters:
        numOfLoopbacks = default(10);
		
    submodules:
        lo[numOfLoopbacks]: LoopbackInterface {
            @display("p=155,510");
        }
        

    connections allowunconnected:
        for i=0..numOfLoopbacks-1 {
            lo[i].upperLayerOut --> ipSplitter.ifIn[sizeof(ethg)+sizeof(pppg)+numRadios+i];
            lo[i].upperLayerIn <-- ipSplitter.ifOut[sizeof(ethg)+sizeof(pppg)+numRadios+i];
            ipSplitter.ipv6Out[sizeof(ethg)+sizeof(pppg)+numRadios+i] --> networkLayer6.ifIn[sizeof(ethg)+sizeof(pppg)+numRadios+i];
            ipSplitter.ipv6In[sizeof(ethg)+sizeof(pppg)+numRadios+i] <-- networkLayer6.ifOut[sizeof(ethg)+sizeof(pppg)+numRadios+i];
            ipSplitter.ipv4Out[sizeof(ethg)+sizeof(pppg)+numRadios+i] --> networkLayer.ifIn[sizeof(ethg)+sizeof(pppg)+numRadios+i];
            ipSplitter.ipv4In[sizeof(ethg)+sizeof(pppg)+numRadios+i] <-- networkLayer.ifOut[sizeof(ethg)+sizeof(pppg)+numRadios+i];
        }
}
