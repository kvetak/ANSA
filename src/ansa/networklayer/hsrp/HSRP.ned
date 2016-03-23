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

package ansa.networklayer.hsrp;

import inet.applications.contract.IUDPApp;

simple HSRP like IUDPApp
{
    parameters:
     	@display("i=block/network2");
     	xml configData = default(xml("<HSRP />"));
     	string deviceId;     	
		string interfaceTableModule = default(".interfaceTableModule");
        string routingTableModule = default(".routingTableModule");
//      	string arp; //path to the arp cache
        int version				= default(1);	//default version of HSRP is 1
    	int hellotime @unit(s)	= default(3s);	//hellotime of actual device
    	int holdtime @unit(s)	= default(10s);	//holdtime of actual device
    	int priority			= default(100);	//priority of actual device
    	int group				= default(0);	//group number of actual device
//    	IPv4Address address;     	
      	string virtualIP;	//virtual IP address of actual HSRP device  
//      	string virtualMAC	= default("00-00-0C-07-AC-00");	//MAC address of actual HSRP device 
      	string multicastIPv4 = default("224.0.0.2");	//multicast address of HSRP
      	
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}
