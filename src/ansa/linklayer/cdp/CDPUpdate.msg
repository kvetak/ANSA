//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
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
// @file CDPUpdate.msg
// @authors Tomas Rajca, Vladimir Vesely (ivesely@fit.vutbr.cz)
// @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
//

import inet.common.TlvOptions;

namespace inet;

enum CDPTLVType
{
	CDPTLV_DEV_ID = 1;
	CDPTLV_ADDRESS = 2;
	CDPTLV_PORT_ID = 3;
	CDPTLV_CAP = 4;
	CDPTLV_VERSION = 5;
	CDPTLV_PLATFORM = 6;
	CDPTLV_IP_PREF = 7;
	CDPTLV_VTP = 9;
	CDPTLV_NAT_VLAN = 10;
	CDPTLV_DUPLEX = 11;
	CDPTLV_LOCATION = 12;
	CDPTLV_LOC_SER = 13;
	CDPTLV_POW_CON = 16;
	CDPTLV_POW_REQ = 25;
	CDPTLV_POW_AVA = 26;
}

class CDPOptionDevId extends TlvOptionBase
{
    string	value;
}

class CDPOptionPortId extends TlvOptionBase
{
    string	value;
}

class CDPOptionVersion extends TlvOptionBase
{
    string	value;
}

class CDPOptionPlatform extends TlvOptionBase
{
    string	value;
}

class CDPOptionVtp extends TlvOptionBase
{
    string	value;
}

class CDPOptionCapa extends TlvOptionBase
{
    type = CDPTLV_CAP;
    char	cap[4];
}

class CDPOptionDupl extends TlvOptionBase
{
    type = CDPTLV_DUPLEX;
    bool	fullDuplex;
}

class CDPOptionNumb extends TlvOptionBase
{
    uint16_t	number;
}

class addressType
{
    uint8_t protocolType = 1;	// 1 - (NLPID format) or 2 (802.2 format)
    uint8_t length;				// length of the protocol field. 1 byte for NLPID, or for 802.2 it is either 3 or 8 bytes
    uint8_t protocol[];			// 0xCC (204) for IP
    uint16_t addressLen = 4;	// length of the address field in bytes.
    string address;			// address of the interface
};

class CDPOptionAddr extends TlvOptionBase
{
    type = CDPTLV_ADDRESS;
    addressType	addresses[];
}

class prefixType
{
    string	network;
    uint8_t		mask;
};

class CDPOptionPref extends TlvOptionBase
{
    type = CDPTLV_IP_PREF;
    prefixType	prefixes[];
}

class CDPOptionODRDef extends TlvOptionBase
{
    type = CDPTLV_IP_PREF;
    string defaultRoute;
}

//CDP update packet
packet CDPUpdate
{
    @customize(true);
    
    uint8_t			version = 2;
    uint8_t			ttl;
    uint16_t		checksum;
    TlvOptions 		options;
}
