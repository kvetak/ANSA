// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
// @file RIPRouting.ned
// @author Jiri Trhlik (mailto:), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
// @brief
// @detail

package inet.ansa.applications.rip;

//
// This module implements RIP version 2 for IP protocol.
//
simple RIPRouting
{
    parameters:
        int RIPPort = default(520);                      // RIP UDP com. port
        string RIPAddress = default("224.0.0.9");          // RIP multicast group
        
        int routeTimeout @unit(s) = default(180s);                   // The Route Timer len. in sec.
        int routeGarbageCollectionTimeout @unit(s) = default(120s);  // The Route Garbage-Collection Timer len. in sec.
        int regularUpdateInterval @unit(s) = default(30s);           // Regulal Update Timer len. in sec.
        
        int adminDist = default(120);  // Cisco-like administrative distance
        
        int connectedNetworkMetric = default(1);           // default metric (RIP) for the connected network
        int infinityMetric = default(16);                  // RIP metric for infinity (unavaible network) 
        
        int timeToLive = default(255);                     // if not -1 set Hop Limit field of sent packets to this value
                
        string configFile;                                 // file name of the XML file which contains configuration for RIPng process
        string deviceId;                                   // needed for proper load of RIPng process XML configuration (deviceId which contains RIPRouting)
        string deviceType;
        
        @display("i=block/network2");  
       
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}