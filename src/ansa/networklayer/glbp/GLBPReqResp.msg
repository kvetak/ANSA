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
#include "inet/linklayer/common/MACAddress.h"
#include "GLBP_m.h"
}}

class noncobject MACAddress;

namespace inet;

packet GLBPReqResp{
    //glbp header
	unsigned char version = 1;
	uint16_t group;
	uint64_t ownerId;	
	
	//REQ/RESP or Hello
	unsigned char type = REQRESP;
	unsigned char length;
	
	//hello
	unsigned char forwarder;
	unsigned char vfState;
	unsigned char priority;
	unsigned char weight;
	unsigned char addressType;
	unsigned char addresLength;
	MACAddress macAddress;	     
}