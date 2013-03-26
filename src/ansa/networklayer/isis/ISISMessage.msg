/// Author: Matej Hrncirik
// FIT VUT 2012
//
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
    
//#include "ISISLSPPacket.h"
//class ISISLSPPacket;
    typedef struct NSAP_t
    {
        unsigned char AreaAddr[3];
        unsigned char sysID[6];
        unsigned char NSEL;
        
    } NSAP_t;
    
    typedef struct TLV_t
    {
        unsigned char type;
        unsigned char length;
        unsigned char *value;
    } TLV_t;
    
    typedef unsigned char* charP;
    
   

}}
class noncobject charP;
class noncobject TLV_t;
class noncobject NSAP_t;


enum PDU_Type
{
    TRILL_HELLO = 14; //TODO FIX can't find the real value
	LAN_L1_HELLO = 15;
	LAN_L2_HELLO = 16;
	PTP_HELLO = 17;
	L1_LSP = 18;
	L2_LSP = 20;
	L1_CSNP = 24;
	L2_CSNP = 25;
	L1_PSNP = 26;
	L2_PSNP = 27;
};


enum CIRCUIT_TYPES
{
	RESERVED_TYPE = 0;		//reserved value (if specified the entire PDU shall be ignored) 
	L1_TYPE = 1;				//Level 1 only
	L2_TYPE = 2;				//Level 2 only (sender is Level 2 Intermediate system with manualL2OnlyMode set True for this circuit, and will use this link only for Level 2 traffic)
	L1L2_TYPE = 3;			//both Level 1 and Level 2 (sender is Level 2 Intermediate system, and will use this link both for Level 1 and Level 2 traffic) 
}


//TLV TYPES
enum TLVtypes
{
	// Types 1 to 10 are defined in  ISO 10589 
	// -------------------------------------------------------
	AREA_ADDRESS = 1; 		// Area address(es) of source node  
	IS_NEIGHBOURS_LSP = 2;	// Neighboring routers and pseudonodes (appended to the link-state packet)  
	ES_NEIGHBOURS = 3;		// Connected workstations  
	PDL2_IS = 4;  			// Level 2 neighbors that interconnect pieces of a partitioned area with a virtual link over the Level 2 area  
	PREFIX_NEIGHBOURS = 5;    // Reachable NSAP prefixes (not including local area prefixes)  
	IS_NEIGHBOURS_HELLO = 6; 	// LAN-connected routers from which IS-IS hello packets have been received (appended to LAN hello)  
	//NOT_SPECIFIED  = 7;    
	PADDING = 8; 				// Padding for hello packets to maximum transmission unit (MTU)  
	LSP_ENTRIES = 9; 			// Link-state information  
	AUTH_INFO_ISO = 10;		// Information for IS-IS packet authentication 
	//--------------------------------------------------------
	
	// Types 128 to 133 are defined in RFC 1195
	// -------------------------------------------------------
	IP_IRI = 128;				// IP Internal Reachability Information ->  Intradomain IS-IS routes  
	PROTOCOLS_SOPPORTED = 129; 	// Protocol identifiers of supported network layer protocols (for example, IP and CLNP)  
	IP_ERI = 130;				// IP External Reachability Information  ->  Routes external to the IS-IS domain, such as those imported from other sources via redistribution  
	IRPI = 131;					// Interdomain Routing Protocol Information ->  For transparent distribution of interdomain routes  
	IP_ADDRESS = 132;			// IP address of the outgoing interface  
	AUTH_INFO_RFC = 133;  		//IS-IS packet authentication (similar to Type 10, but doesn't define authentication Type 255)  
	// -------------------------------------------------------
	
	//RFC 6326
	TLV_TRILL_NEIGHBOR = 145;
	
	
	PTP_HELLO_STATE = 240; // RFC5303 Adjacency state for 3-way handshake on point-to-point link
}


//
// general IS-IS packet header
// DO NOT USE INSTANCE OF THIS PACKET TYPE
// USE PACKET TYPES DERIVED FROM THIS ONE -> SEE BELOW

packet ISISMessage extends cPacket
{
    unsigned char IRPD = 0x83; 			//Intradomain Routing Protocol Discriminator = 0x83 for IS-IS
    unsigned char length; 				//Specifies the length of the PDU Header in bytes.
    unsigned char version = 0x01;		//Version/Protocol ID Extension Applies to the IS-IS protocol (defined as 1).
    unsigned char idLength = 0x00;		//Gives the length of the system ID field in an NSAP(NET).  value 0 = 6Bytes (default). STRONGLY RECOMMENDED NOT TO CHANGE THIS VALUE!!
    unsigned char type enum(PDU_Type);	//Specifies the type of packet.
    unsigned char version2 = 0x01;		//This is also set to 1.
    unsigned char reserved = 0x00;		//Future use
    unsigned char maxAreas = 0x00;		//Specifies the number of areas permitted.  value 0 = default = 3 areas
    TLV_t TLV[];
};


packet ISISLANHelloPacket extends ISISMessage
{
    unsigned char circuitType enum(CIRCUIT_TYPES);	//Defines whether the link is Level 1 and/or Level 2.
	unsigned char sourceID[6];						//Is the same as the system identifier (SysID).  address in format 0123.4567.89AB
	unsigned short holdTime = 40;					//Defines how long to wait for a hello from this system before clearing the adjacency. default = 40sec.
	unsigned short pduLength;						//Gives the length of the packet (PDU), in bytes.
	unsigned char priority = 64;					//Carries a value between 0 and 127 which is used for DIS (Designated IS) election on a LAN network. The default value is 64.	
	unsigned char lanID[7];							//Consists of the system ID of the designated intermediate system, plus a unique number as an identifier of the LAN. e.g. 0000.0000.0001.02
    
}

//IS-IS Level1 LAN Hello Message
packet ISISL1HelloPacket extends ISISLANHelloPacket
{
	type = LAN_L1_HELLO;							//set PDU type
	
//	TLV_t TLV[];									//Consists of Type (or code), Length, and Value fields. Also known as variable-length fields.
}



//IS-IS Level2 LAN Hello Message
packet ISISL2HelloPacket extends ISISLANHelloPacket
{
	type = LAN_L2_HELLO;							//set PDU type
}

//TRILL Hello
packet TRILLHelloPacket extends ISISLANHelloPacket
{
    type = TRILL_HELLO;
    circuitType = L1_TYPE;
    
}

//IS-IS PTP Hello Message
packet ISISPTPHelloPacket extends ISISMessage
{
    type = PTP_HELLO;								//set PDU type
	unsigned char circuitType enum(CIRCUIT_TYPES);	//Defines whether the link is Level 1 and/or Level 2.
	unsigned char sourceID[6];						//Is the same as the system identifier (SysID).  address in format 0123.4567.89AB
	unsigned short holdTime;						//Defines how long to wait for a hello from this system before clearing the adjacency. (probably in seconds)
	unsigned short pduLength;						//Gives the length of the packet (PDU), in bytes.
	unsigned char localCircuitID;					//Local Circuit ID  1 octet unique ID assigned to this circuit when it is created by this Intermediate system.
//	TLV_t TLV[];									//Consists of Type (or code), Length, and Value fields. Also known as variable-length fields.
}

//IS-IS LSP  message
packet ISISLSPPacket extends ISISMessage
{
//    @customize(true);
	//type = L1_LSP;
	unsigned short pduLength;						//Gives the length of the packet (PDU), in bytes.
	unsigned short remLifeTime = 1200;				//Number of seconds before an LSP is considered expired. Cisco IOS uses 20 mins (1200 seconds).
	unsigned char lspID[8];							//It is the System ID, the Pseudonode ID and the LSP number of the LSP. e.g. 0000.0000.0002.00-00
	unsigned long seqNumber = 0;					//It is the sequence number of the LSP.
	unsigned short checksum;						//It is the checksum of the contents of the LSP.
	unsigned char PATTLSPDBOLIS = 0x01;				//	1b P |  4b ATT | 1b OL | 2b IS TYPE
													//  Partition Repair (P-bit) - no idea, but set to 0 :)
													//  Attachment (ATT): This is a 4-bit field, however, Cisco IOS uses only 1 bit to indicate if the originating router is attached to more than one areas.
													//  Overload (OL) bit: If the originating router is experiencing a memory over-utilization, it will set this bit to 1. The receiving router will then not use this router as the transit.
													//  IS Type: This indicates whether the originating router is a L1 or L2.     01- Level-1    11- Level-2
//	TLV_t TLV[];									//Consists of Type (or code), Length, and Value fields. Also known as variable-length fields.

}

//IS-IS LSP Level1 message
packet ISISLSPL1Packet extends ISISLSPPacket
{
	type = L1_LSP;
}

//IS-IS LSP Level2 message
packet ISISLSPL2Packet extends ISISLSPPacket
{
	type = L2_LSP;
}


packet ISISCSNPPacket extends ISISMessage
{
    unsigned short pduLength;						//Length of the entire PDU in bytes.
	unsigned char sourceID[7];						// Source ID + zero circuit number  e.g. 0000.0000.0002.00
	unsigned char startLspID[8];					// Start LSP ID � the LSP ID of first LSP in the range covered by this Complete Sequence Numbers PDU.  e.g. 0000.0000.0003.00-00
	unsigned char endLspID[8];						//analogical to startLspID
    
}
//IS-IS CSNP Level1 message
packet ISISCSNPL1Packet extends ISISCSNPPacket
{
	type = L1_CSNP;
	
//	TLV_t TLV[];									//Consists of Type (or code), Length, and Value fields. Also known as variable-length fields.
}


//IS-IS CSNP Level2 message
packet ISISCSNPL2Packet extends ISISCSNPPacket
{
	type = L2_CSNP;
}

packet ISISPSNPPacket extends ISISMessage
{
    unsigned short pduLength;						//Length of the entire PDU in bytes.
	unsigned char sourceID[7];						// Source ID + zero circuit number  e.g. 0000.0000.0002.00
}


//IS-IS PSNP Level1 message
packet ISISPSNPL1Packet extends ISISPSNPPacket
{
	type = L1_PSNP;
//	TLV_t TLV[];									//Consists of Type (or code), Length, and Value fields. Also known as variable-length fields.
}  

//IS-IS PSNP Level2 message
packet ISISPSNPL2Packet extends ISISPSNPPacket
{
	type = L2_PSNP;
}

