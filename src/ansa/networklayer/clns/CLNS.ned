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

package ansa.networklayer.clns;

// @author Marcel Marek
simple CLNS 
{
    parameters:
        @display("i=block/routing");
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
    gates:
        
        input transportIn[] @labels(CLNSControlInfo/down,ISISMessage);
        output transportOut[] @labels(CLNSControlInfo/up,ISISMessage);
        
        input queueIn[] @labels(ISISMessage,Ieee802Ctrl);
        output queueOut[] @labels(ISISMessage,Ieee802Ctrl);
}
