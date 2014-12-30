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

import inet.ansa.networklayer.vrrpv2.VRRPv2;
import inet.ansa.linklayer.ethernet.AnsaEthernetInterfaceWithVF;

module ANSA_xTR_HA extends ANSA_xTR
{	
    parameters:
        ethType = "AnsaEthernetInterfaceWithVF";
    submodules:
        VRRP: VRRPv2 {
            parameters:
            	@display("p=242,226");
            	deviceId = deviceId;
                configFile = configFile;
        }
    connections allowunconnected:
        VRRP.ipOut --> networkLayer.vrrpIn;
        VRRP.ipIn <-- networkLayer.vrrpOut;
}
