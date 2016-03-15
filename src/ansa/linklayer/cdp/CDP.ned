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

package ansa.linklayer.cdp;

import inet.networklayer.contract.INetworkLayerLower;

//TODO: davat INetworkLayerLower? (nebude mit nekdy switch problem)
simple CDP like INetworkLayerLower
{
    parameters:
    	@display("i=block/network2");
    	
    	int timer @unit(s) = default(60s);
    	int holdTime = default(180);
    	int version = default(2);
    	bool odr = default(false);
	gates:
        input ifIn[] @labels(INetworkDatagram/up);
        output ifOut[] @labels(INetworkDatagram/down);
}
