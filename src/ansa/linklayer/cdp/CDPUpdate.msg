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

enum CDPTLVType
{
	CDPTLV_ADDRESS = 2;
	CDPTLV_CAP = 4;
	CDPTLV_DEV_ID = 1;
	CDPTLV_DUPLEX = 11;
	CDPTLV_IP_PREF = 7;
	CDPTLV_LOCATION = 12;
	CDPTLV_LOC_SER = 13;
	CDPTLV_NAT_VLAN = 10;
	CDPTLV_PLATFORM = 6;
	CDPTLV_PORT_ID = 3;
	CDPTLV_VERSION = 5;
	CDPTLV_VTP = 9;
	CDPTLV_ODR = 99; //TODO:zjistit cislo
	CDPTLVCOUNT = 13;
}

// TLV structure
class CDPTLV
{
    uint16_t	type @enum(CDPTLVType); 	
    uint16_t	length;
    string		value;		//TODO: void a pretypovat?
}

class CDPTLVString extends CDPTLV
{
    string value;
}

//CDP update packet
packet CDPUpdate
{
    char		version = 2;
    unsigned char		ttl;
    uint16_t	checksum;
    CDPTLV		tlv[];
}
