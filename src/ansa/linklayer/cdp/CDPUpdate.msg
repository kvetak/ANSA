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
	TLV_ADDRESS = 2;
	TLV_CAPABILITIES = 4;
	TLV_DEVICE_ID = 1;
	TLV_DUPLEX = 11;
	TLV_IP_PREFIX = 7;
	TLV_LOCATION = 12;
	TLV_LOCATION_SERVER = 13;
	TLV_NATIVE_VLAN = 10;
	TLV_PLATFORM = 6;
	TLV_PORT_ID = 3;
	TLV_VERSION = 5;
	TLV_VTP = 9;
	TLV_ODR = 99; //TODO:zjistit cislo
}

// TLV structure
class CDPTLV
{
    uint16_t	type @enum(CDPTLVType); 
    uint16_t	length;
    string		value;
}

//CDP update packet
packet CDPUpdate
{
    char		version = 2;
    unsigned char		ttl;
    uint16_t	checksum;
    CDPTLV		tlv[];
}
