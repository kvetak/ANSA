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
// @author Jan Holusa
// @author Vladimir Vesely (ivesely@fit.vutbr.cz)
// @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3


cplusplus {{
#include "inet/common/TlvOptions.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/linklayer/common/MacAddress.h"
//#include "inet/networklayer/common/IPProtocolId_m.h"
}}

namespace inet;

cplusplus {{
const int GLBP_HEADER_BYTES = 12;
const short GLBP_REQRESP_BYTES = 20;
const short GLBP_HELLO_BYTES = 28;
}}


class noncobject TlvOptionBase;
class noncobject TlvOptions;

class noncobject Ipv4Address;
class noncobject MacAddress;

///
/// GLBP TLV type
///
enum GLBPType{
	HELLO = 1;
	REQRESP = 2;
	//AUTH = 3;
}

///
/// GLBP Hello Address Type
///
enum GLBPAddressType{
	Ipv4 = 1;
//	Ipv6 = 2;
}

///
/// GLBP VG + VF State
///
enum GLBPState{
    UNKNOWN = 0;
    DISABLED=1;
    INIT = 2;
    LISTEN = 4;
    SPEAK = 8; 		//VG only
    STANDBY = 16; 	//VG only
    ACTIVE = 32;
}

class GLBPOption extends TlvOptionBase
{
}

///
/// GLBP Hello TLV
///
class GLBPHello extends GLBPOption
{
    type = HELLO;
    length = GLBP_HELLO_BYTES;
	
	short vgState;			///VG state
	short priority;			///Priority of this router
	uint32_t helloint;		///hellotimer
	uint32_t holdint;		///holdtimer
	uint16_t redirect;		///redirecttimer
	uint16_t timeout;		///timeout timer
	short addressType = Ipv4;///address type (Ipv4 or Ipv6)
	short addresLength = 4;	///address length; 4 for Ipv4
	Ipv4Address address;	///Ipv4 address of glbp group
}

///
/// GLBP RequestResponse LV
///
class GLBPRequestResponse extends GLBPOption{
    type = REQRESP;
	length = GLBP_REQRESP_BYTES;
	
	short forwarder;	///Number of forwarder
	short vfState;		///VF state of this forwarder
	short priority;		///priority of this VF (167 for primary, 135 for secondary)
	short weight;		///Weight of this VF
	MacAddress macAddress; ///MAC address of this VF 
}

///
/// GLBP Packet Structure
///
packet GLBPMessage {
    @customize(true);
    
    short version = 1;	///version is always 1
	uint16_t group;		///group id
	MacAddress ownerId;	///owner id which is MAC of interface
	  
	TlvOptions TLV; 	///array of tlvs	
}
