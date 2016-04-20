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
cplusplus {{
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "GLBP_m.h"
const int GLBP_HELLO_SIZE = 40;
const std::string GLBP_MULTICAST_ADDRESS = "224.0.0.102";
const int GLBP_UDP_PORT = 3222; 
}}

class noncobject IPv4Address;

namespace inet;

packet GLBPHello{
    //glbp header
	unsigned char version = 1;
	uint16_t group;
	uint64_t ownerId;	
	
	//REQ/RESP or Hello
	unsigned char type = HELLO;
	unsigned char length;
	
	//hello
	unsigned char vgState;
	unsigned char priority;
	uint32_t helloint;
	uint32_t holdint;
	uint16_t redirect;
	uint16_t timeout;
	unsigned char addressType = IPv4;
	unsigned char addresLength = 4;
	IPv4Address address;	     
}