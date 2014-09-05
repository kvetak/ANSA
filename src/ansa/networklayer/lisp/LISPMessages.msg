//
// Copyright (C) 2013 Brno University of Technology
// Copyright (C) 2010 Michael Hoefling
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
//@author Vladimir Vesely (<a href="mailto:ivesely@fit.vutbr.cz">ivesely@fit.vutbr.cz</a>)

cplusplus {{
    #include "INETDefs.h"    
    #include "LISPMapEntry.h"              
    
    // ---------- Map-Request Rec ----------
    typedef std::list<LISPEidPrefix> TRecs;       		
            
    // ---------- AFI-Address tupple ---------- 
    typedef std::pair<unsigned char, IPvXAddress> TAfiAddr;
    typedef std::list<TAfiAddr> TAfiAddrs;
    
}}

class noncobject IPvXAddress;
class noncobject TRecord;
class noncobject TRecords;
class noncobject TRecs;
class noncobject TAfiAddr;
class noncobject TAfiAddrs;

enum LISP_MsgType
{
    REQUEST  	= 1;
    REPLY    	= 2;
    REGISTER 	= 3;
    NOTIFY		= 4;
}

packet LISPHeader {
    string displayString = "b=15,15,oval,blue,blue,0"; 
    bool NonceBit; 						// nonce-present Bit
    bool LocStatBit; 					// locator-status-Bits field enabled
    bool EchoNonceBit; 					// echo-nonce-request Bit
    bool VerMapBit; 					// map-version persent Bit
    bool InstanceBit; 					// instance id Bit
    unsigned char flags;							// currently unused flags
    unsigned int nonce; 				// nonce
    unsigned int mapVersion; 			// map-version
    unsigned int instanceId; 			// instanceId
    unsigned int locatorStatusBits; 	// locator-status-Bits field
}

packet LISPMessage {
    string displayString;
    unsigned char type enum(LISP_MsgType);	// message type    
    unsigned char recordCount;				// number of records
    unsigned long nonce; 					// nonce
}

//        0                   1                   2                   3
//        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |Type=1 |A|M|P|S|p|s|    Reserved     |   IRC   | Record Count  |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |                         Nonce . . .                           |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |                         . . . Nonce                           |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |         Source-EID-AFI        |   Source EID Address  ...     |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |         ITR-RLOC-AFI 1        |    ITR-RLOC Address 1  ...    |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |                              ...                              |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |         ITR-RLOC-AFI n        |    ITR-RLOC Address n  ...    |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//     / |   Reserved    | EID mask-len  |        EID-Prefix-AFI         |
//   Rec +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//     \ |                       EID-Prefix  ...                         |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |                   Map-Reply Record  ...                       |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

packet LISPMapRequest extends LISPMessage {
    string displayString = "b=15,15,oval,orange,orange,0";
    type = REQUEST;    
    bool ABit;							// Authoritative Bit 
    bool MapDataBit;					// Map-data-present Bit
    bool ProbeBit; 						// probe/proxy Bit
    bool SmrBit;						// Solicit-Map-Request Bit
    bool pITRBit;						// PITR Bit
    bool smrInvokedBit;					// SMR-invoked Bit
    
    unsigned char itrRlocCount;			// number of itrRlocs       
    
    TAfiAddr sourceEid;					// source AFIs-addresses
    TAfiAddrs itrRlocs;					// list of ITR's AFIs-addresses    
    
    TRecs recs;							// <-- see structure above 
    
    TRecord mapReply; 					// Map-Reply with own info
}

//        0                   1                   2                   3
//        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |Type=2 |P|E|S|          Reserved               | Record Count  |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |                         Nonce . . .                           |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |                         . . . Nonce                           |
//   +-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |   |                          Record TTL                           |
//   |   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   R   | Locator Count | EID mask-len  | ACT |A|      Reserved         |
//   e   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   c   | Rsvd  |  Map-Version Number   |       EID-Prefix-AFI          |
//   o   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   r   |                          EID-Prefix                           |
//   d   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |  /|    Priority   |    Weight     |  M Priority   |   M Weight    |
//   | L +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   | o |        Unused Flags     |L|p|R|           Loc-AFI             |
//   | c +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |  \|                             Locator                           |
//   +-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

packet LISPMapReply extends LISPMessage {
	string displayString = "b=15,15,oval,green,green,0";
	type = REPLY;    
	bool ProbeBit;						//Proxy Map-Reply Bit
	bool EchoNonceBit;					// Echo-Nonce Bit
	bool SecBit;						// Security Bit       
        
    TRecords records;    				// <-- see structure above 
}

//        0                   1                   2                   3
//        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |Type=3 |P|            Reserved               |M| Record Count  |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |                         Nonce . . .                           |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |                         . . . Nonce                           |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |            Key ID             |  Authentication Data Length   |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       ~                     Authentication Data                       ~
//   +-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |   |                          Record TTL                           |
//   |   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   R   | Locator Count | EID mask-len  | ACT |A|      Reserved         |
//   e   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   c   | Rsvd  |  Map-Version Number   |        EID-Prefix-AFI         |
//   o   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   r   |                          EID-Prefix                           |
//   d   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |  /|    Priority   |    Weight     |  M Priority   |   M Weight    |
//   | L +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   | o |        Unused Flags     |L|p|R|           Loc-AFI             |
//   | c +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |  \|                             Locator                           |
//   +-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

packet LISPMapRegister extends LISPMessage {
	string displayString = "b=15,15,oval,yellow,yellow,0";
	type = REGISTER; 	
	bool ProxyBit;							//Proxy Map-Reply Bit
 	bool MapNotifyBit; 						// map-notify
 	 	
 	unsigned char keyId; 					// key id
 	unsigned short authDataLen;				// length of authentization data
 	string authData;						// authentization data
 	
 	TRecords records;					// <-- see structure above      
}

//        0                   1                   2                   3
//        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |Type=4 |              Reserved                 | Record Count  |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |                         Nonce . . .                           |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |                         . . . Nonce                           |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |            Key ID             |  Authentication Data Length   |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       ~                     Authentication Data                       ~
//   +-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |   |                          Record TTL                           |
//   |   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   R   | Locator Count | EID mask-len  | ACT |A|      Reserved         |
//   e   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   c   | Rsvd  |  Map-Version Number   |         EID-Prefix-AFI        |
//   o   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   r   |                          EID-Prefix                           |
//   d   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |  /|    Priority   |    Weight     |  M Priority   |   M Weight    |
//   | L +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   | o |        Unused Flags     |L|p|R|           Loc-AFI             |
//   | c +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |  \|                             Locator                           |
//   +-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

packet LISPMapNotify extends LISPMapRegister {
    string displayString = "b=15,15,oval,cyan,cyan,0";
    type = NOTIFY;
}
