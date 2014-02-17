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
    #include "IPvXAddress.h"       
    // ---------- Locators ---------- 
    typedef struct _locators
    {       
        unsigned char priority;				// unicast priority
        unsigned char weight;				// unicast weight
        unsigned char Mpriority;			// multicast priority
        unsigned char Mweight;				// multicast weight
        bool LocalLocBit;					// local locator Bit
        bool piggybackBit;					// piggyback Bit
        bool RouteRlocBit;					// route RLOC Bit
        unsigned short locAFI;				// locator AFI
        IPvXAddress locator;     			// locator
    } Locators;
    
    // ---------- Map-Request Recs ----------
    typedef struct _requestrecs
    {
        unsigned short eidLen;				// EID length
        unsigned short eidPrefixAFI;		// eid-prefix AFIs     
        IPvXAddress eidPrefix; 				// eid-prefix    
    } RequestRecs;
    
    // ---------- Map-Reply Recs ----------
    typedef struct _replyrecs
    {
        unsigned short mapVersion;			// EID length
        unsigned short eidPrefixAFI;		// eid-prefix AFIs     
        IPvXAddress eidPrefix; 				// eid-prefix 
           
        Locators locators[];      			//<-- see struct above    
    } ReplyRecs;
    
    
    // ---------- Map-Reply/Register Records ----------
    typedef struct _records
    {
        simtime_t recordTTL;				// TTL
        unsigned char locatorCount;   		// locator count
        unsigned char eidMaskLen;			// EID mask
        unsigned char ACT;					// Action flags
        bool ABit; 							// Authoritative Bit
        ReplyRecs recs;             		//<-- see struct above
    } Records;
}}
//struct records;
//struct locators;
class noncobject IPvXAddress;
class noncobject Records;
class noncobject ReplyRecs;
class noncobject RequestRecs;

enum LISP_MsgType
{
    REQUEST  = 1;
    REPLY    = 2;
    REGISTER = 3;
    NOTIFY	 = 4;
}

packet LISPHeader {
    string displayString = "b=15,15,oval,blue,black,2"; 
    bool NonceBit; 						// nonce-present Bit
    bool LocStatBit; 					// locator-status-Bits field enabled
    bool EchoNonceBit; 					// echo-nonce-request Bit
    bool VerMapBit; 					// map-version persent Bit
    bool InstanceBit; 					// instance id Bit
    char flags;							// currently unused flags
    unsigned int nonce; 				// nonce
    unsigned int mapVersion; 			// map-version
    unsigned int instanceId; 			// instanceId
    unsigned int locatorStatusBits; 	// locator-status-Bits field
}

packet LISPMessage {
    string displayString;
    unsigned char type enum(LISP_MsgType);	// message type    
    unsigned char recordCount;			// number of records
    unsigned long nonce; 				// nonce
}

packet LISPMapRequest extends LISPMessage {
    string displayString = "b=15,15,oval,red,black,2";
    type = REQUEST;    
    bool ABit;							// Authoritative Bit 
    bool MapDataBit;					// Map-data-present Bit
    bool ProbeBit; 						// probe/proxy Bit
    bool SmrBit;						// Solicit-Map-Request Bit
    bool pITRBit;						// PITR Bit
    bool smrInvokedBit;					// SMR-invoked Bit
    
    unsigned char itrRlocCount;			// number of itrRlocs       
    
	unsigned short itrAFI[];			// list of eid-prefix AFIs         
    IPvXAddress itrRlocs[]; 			// addresses of itr used to send map-reply to
    
    RequestRecs recs[];					// <-- see structure above 
    
    Records mapReply; 					//Map-Reply with own info
}

packet LISPMapReply extends LISPMessage {
    string displayString = "b=15,15,oval,green,black,2";
	type = REPLY;    
    bool EchoNonceBit;					// Echo-Nonce Bit
    bool SecBit;						// Security Bit       
    
    Records records[];    				// <-- see structure above 
}

packet LISPMapRegister extends LISPMessage {
    string displayString = "b=15,15,oval,yellow,black,2";
 	type = REGISTER; 	
 	bool MapNotifyBit; 					// map-notify
 	 	
 	unsigned short keyId;				// HMAC key id
 	unsigned short authDataLen;			// length of authentization data
 	char authData[];					// authentization data
 	
 	Records records[];					// <-- see structure above      
}

packet LISPMapNotify extends LISPMapRegister {
    string displayString = "b=15,15,oval,orange,black,2";
    type = NOTIFY;
}
