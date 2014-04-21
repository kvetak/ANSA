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
#include "IPv4Address.h"
}}

class noncobject IPv4Address;

// Codes of EIGRP messages
enum HeaderOpcode
{
    EIGRP_UPDATE_MSG = 1;
    EIGRP_REQUEST_MSG = 2;
    EIGRP_QUERY_MSG = 3;
    EIGRP_REPLY_MSG = 4;
    EIGRP_HELLO_MSG = 5;
    EIGRP_SIAQUERY_MSG = 10;
    EIGRP_SIAREPLY_MSG = 11;
};

// Struct for K-values
struct EigrpKValues
{
	uint16_t K1 = 1;
	uint16_t K2 = 0;
	uint16_t K3 = 1;
	uint16_t K4 = 0;
	uint16_t K5 = 0;
	uint16_t K6 = 0;
};

// 8 bit flags field, only three flags specified
struct EigrpRouteFlags
{
    bool SW;				// Source Withdraw			
    bool CD;				// Candidate Default
    bool active;			// Active path
}

// Struct for wild metric paramaters
struct EigrpWideMetricPar
{
    uint8_t offset = 0;
    uint8_t priority = 0;
    uint8_t reliability = 0;    // Min rel. on path
    uint8_t load = 0;           // Max load on path
    uint32_t mtu = 0;           // Min MTU on path, only 24 bits used
    uint8_t hopCount = 0;       // Hop count to destination
    uint64_t delay = 0;         // Delay sum on path, only 48 bits used
    uint64_t bandwidth = 0;     // Min BW on path, only 48 bits used
}

// TLV Parameter Type for Hello message
struct EigrpTlvParameter
{
    // type
    char typeHigh = 0;
    char typeLow = 0;
    // length
    uint16_t length = 12;
    // value
    EigrpKValues kValues;
    uint16_t holdTimer;
};

// IPv4 TLV Internal route
//struct EigrpIpv4Internal
//{
//    // type
//    char typeHigh = 1;
//    char typeLow = 2;
//    // length
//    uint16_t length = 28;
//    // value
//    IPv4Address routerID;		// IP of originating router (also in exterior TLV)
// 	IPv4Address nextHop;   
// 	EigrpMetricPar metric;
// 	EigrpRouteFlags flags;
// 	IPv4Address destMask;	 	// Destination
// 	IPv4Address destAddress;
//};

// Multiprotocol Ipv4 (IPv4Address used) Internal TLV route
struct EigrpMpIpv4Internal
{
    // type
    char typeHigh = 6;
    char typeLow = 2;
    // length
    uint16_t length = 44;
    // value
    uint16_t afi;				// Address Family ID, IPv4 = 1, Ipv6 = 2
    uint16_t tid;				// Topology ID
    IPv4Address routerID;		// Router ID

 	EigrpWideMetricPar metric;
 	EigrpRouteFlags flags;
 	
 	IPv4Address nextHop;
 	IPv4Address destMask;
 	IPv4Address destAddress;
};


// General structure of EIGRP message with header
packet EigrpMessage 
{
    char version = 2;	// Version of EIGRP header
	int8_t opcode;		// Type of message
	// flags
	bool init;			// Initialization - establishment of neighborship
	bool cr;			// Conditionally Received
	bool rs;			// Reset
	bool eot;			// End of table
	
	int seqNum;			// Sequence number
	int ackNum;			// Acknowledgement number
	uint16_t vrid;		// Virtual Router ID
	uint16_t asNum;		// Autonomous system number
};

// EIGRP Hello message
packet EigrpHello extends EigrpMessage
{
	EigrpTlvParameter parameterTlv;
};

packet EigrpIpv4Hello extends EigrpHello
{
};

// Ack message - temporarily
packet EigrpIpv4Ack extends EigrpMessage
{
};

packet EigrpIpv4Message extends EigrpMessage
{
    EigrpMpIpv4Internal interRoutes[];
};

// EIGRP Update Message
packet EigrpIpv4Update extends EigrpIpv4Message
{ 
};

// EIGRP Query Message
packet EigrpIpv4Query extends EigrpIpv4Message
{
};

// EIGRP Reply Message
packet EigrpIpv4Reply extends EigrpIpv4Message
{
};

//
// EIGRP RTP
//
struct EigrpMsgRoute
{
    int sourceId;           // ID of source for route
    int routeId;           	// ID of route 
    bool unreachable;       // Route with maximal metric 
    bool invalid;
};

// Message request
message EigrpMsgReq
{
    @customize(true);
   	
   	int8_t opcode;			// Type of message
   	bool goodbyeMsg = false; // Message is Hello goodbye
   	
   	// flags
	bool init;			// Initialization - establishment of neighborship
	bool cr;			// Conditionally Received
	bool rs;			// Reset
	bool eot;			// End of table
	
    int destNeighbor;    	// ID of neighbor that is destination of message 
    int destInterface;      // ID of destination interface

    uint32_t seqNumber;     // Sequence number for reliable transmission
    uint32_t ackNumber;     // Ack number for reliable transmission

    int numOfAck;			// Number of acknowledges 
    EigrpMsgRoute routes[];
};

    
    
    