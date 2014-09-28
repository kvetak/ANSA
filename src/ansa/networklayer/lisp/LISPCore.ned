//
// Copyright (C) 2013, 2014 Brno University of Technology
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
//
// @author Vladimir Vesely / ivesely@fit.vutbr.cz / http://www.fit.vutbr.cz/~ivesely/ 



package inet.ansa.networklayer.lisp;


simple LISPCore
{
    parameters:
        @display("i=block/switch");        
        string deviceId;    
        xml configData = default(xml("<LISP />"));
        bool mapServerV4 = default(false);				//Node acts as Map Server for IPv4 EIDs, overriden by XML config
        bool mapServerV6 = default(false);				//Node acts as Map Server for IPv6 EIDs, overriden by XML config
		bool mapResolverV4 = default(false);			//Node acts as Map Resolver for IPv4 EIDs, overriden by XML config
        bool mapResolverV6 = default(false);    		//Node acts as Map Resolver for IPv6 EIDs, overriden by XML config
        
        string rlocProbingAlgo = default("Cisco"); 		// Cisco OR EidRoundRobin OR EidGrouped
        bool acceptMapRequestMapping = default(false);	//Whether MapReplyRecord from Map-Request should be cached
        int mapCacheTtl = default(1440); 				//CacheTTL in minutes for send Map-Replies
                                                    
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
        input  ipv4In;
        output ipv4Out;
        input  ipv6In;
        output ipv6Out;
}

