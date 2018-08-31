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
// @file LLDPUpdate.ned
// @author Tomas Rajca
// @author Vladimir Vesely (ivesely@fit.vutbr.cz)
// @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3


import inet.common.TlvOptions;

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

 
enum LLDPChassisIdSubtype	//lcis
{
    lcisReserved = 0;
    lcisChassComp = 1;
    lcisIfaceAlias = 2;
    lcisPortComp = 3;
    lcisMacAdd = 4;
    lcisNetAdd = 5;
    lcisIfaceNam = 6;
    lcisLocAss = 7;
}

 
enum LLDPPortIdSubtype	//pcis
{
    pcisReserved = 0;
    pcisIfaceAlias = 1;
    pcisPortComp = 2;
    pcisMacAdd = 3;
    pcisNetAdd = 4;
    pcisIfaceNam = 5;
    pcisAgentCirId = 6;
    pcisLocAss = 7;
}

class LLDPOptionChassisId extends TlvOptionBase
{
    type = LLDPTLV_CHASSIS_ID;
    uint8_t	subtype;
    string	value;
}

class LLDPOptionPortId extends TlvOptionBase
{
    type = LLDPTLV_PORT_ID;
    uint8_t	subtype;
    string	value;
}

class LLDPOptionTTL extends TlvOptionBase
{
    type = LLDPTLV_TTL;
    uint16_t	ttl;
}

class LLDPOptionEndOf extends TlvOptionBase
{
    type = LLDPTLV_END_OF;
    length = 1;				//TODO: should be 0
}

class LLDPOptionPortDes extends TlvOptionBase
{
    type = LLDPTLV_PORT_DES;
    string	value;
}

class LLDPOptionSystemName extends TlvOptionBase
{
    type = LLDPTLV_SYSTEM_NAME;
    string	value;
}

class LLDPOptionSystemDes extends TlvOptionBase
{
    type = LLDPTLV_SYSTEM_DES;
    string	value;
}

class LLDPOptionCap extends TlvOptionBase
{
    type = LLDPTLV_SYSTEM_CAP;
    uint8_t	chasId = 0;
    char	sysCap[2];		// system capabilities
    char	enCap[2];		// enabled capabilities
}

class LLDPOptionManAdd extends TlvOptionBase
{
    type = LLDPTLV_MAN_ADD;
    uint8_t		addLength;			// length, in octets, of the (management addresssubtype + management address) fields
    uint8_t		addSubtype = 6;		// type of address that is listed in the management address field
    string		address;			// management address associated with this TLV
    uint8_t		ifaceSubtype = 3;	// numbering method used for defining the interface number
    uint32_t	ifaceNum;			// assigned number within the system that identifies the specific interface 
    uint8_t		oidLength;			// object identifier length
    string		oid;				// object identifier
}

class LLDPOptionOrgSpec extends TlvOptionBase
{
    type = LLDPTLV_ORG_SPEC;
    uint32_t	oui;			// organizationally unique identifier
    uint8_t		subtype;		// organizationally defined subtype
    string		value;			// organizationally defined information string
}


//LLDP update packet
packet LLDPUpdate
{
    @customize(true);
    
    TlvOptions		options;
}
