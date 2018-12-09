//
// Copyright (C) 2009 - today, Brno University of Technology, Czech Republic
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
// @author Petr Vitek (ivesely@fit.vutbr.cz)
// @author Vladimir Vesely (ivesely@fit.vutbr.cz)
// @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3

package ansa.routing.vrrp;

import inet.routing.contract.IManetRouting;

simple VRRPv2 like IManetRouting
{
    parameters:
        @display("i=block/network2");
        xml configData = default(xml("<VRRP />"));              
        string interfaceTableModule = default(".interfaceTableModule");        
           
    gates:
        input vrIn[] @labels(IPv4Datagram);
        output vrOut[] @labels(IPv4Datagram);       
        input ipIn @labels(Ipv4ControlInfo/up);
        output ipOut @labels(Ipv4ControlInfo/down);
}
