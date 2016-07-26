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

package ansa.routing.vrrp;

simple VRRPv2
{
    parameters:
        @display("i=block/network2");
        string configFile;
        string deviceId;      
            
    gates:
        input vrIn[] @labels(IPv4Datagram);
        output vrOut[] @labels(IPv4Datagram);       
        input ipIn @labels(IPv4Datagram);
        output ipOut @lables(IPv4Datagram);
}
