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

namespace inet;

enum LLDPTLVType
{
	LLDPTLV_END_OF = 0;
	LLDPTLV_CHASSIS_ID = 1;
	LLDPTLV_PORT_ID = 2;
	LLDPTLV_TTL = 3;
	LLDPTLV_PORT_DES = 4;
	LLDPTLV_SYSTEM_NAME = 5;
	LLDPTLV_SYSTEM_DES = 6;
	LLDPTLV_SYSTEM_CAP = 7;
	LLDPTLV_MAN_ADD = 8;
	LLDPTLV_ORG_SPEC = 127;		//basic organizationally specific
}

// TLV structure
class LLDPTLV
{
    uint8_t	type @enum(LLDPTLVType); 	//TODO: melo by byt 7 bitu	
    uint16_t	length;					//TODO: melo by byt 9 bitu
    string		value;		//TODO: void a pretypovat?
}

//LLDP update packet
packet LLDPUpdate
{
    LLDPTLV		tlv[];
}