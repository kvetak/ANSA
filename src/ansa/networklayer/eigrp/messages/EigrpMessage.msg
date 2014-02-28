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
#include "EigrpRoute.h"
}}

class noncobject IPv4Address;
struct EigrpMetricPar;

// Codes of EIGRP messages
enum HeaderOpcode
{
    EIGRP_UPDATE_MSG = 1;
    EIGRP_QUERY_MSG = 3;
    EIGRP_REPLY_MSG = 4;
    EIGRP_HELLO_MSG = 5;
};

// Struct for K-values
struct EigrpKValues
{
	uint16_t K1;
	uint16_t K2;
	uint16_t K3;
	uint16_t K4;
	uint16_t K5;
	uint16_t K6;
};

struct EigrpRouteFlags
{
    // 8 bit flags field, only three flags specified
    bool SW;				// Source Withdraw			
    bool CD;				// Candidate Default
    bool active;			// Active path
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
struct EigrpIpv4Internal
{
    // type
    char typeHigh = 1;
    char typeLow = 2;
    // length
    uint16_t length = 28;
    // value
 	IPv4Address nextHop;   
 	EigrpMetricPar metric;
 	EigrpRouteFlags flags;
 	IPv4Address destMask;	 	// Destination
 	IPv4Address destAddress;
};


// General structure of EIGRP message
packet EigrpMessage 
{
    // header
    char version = 2;	// Version of EIGRP header
	char opcode;		// Type of message
	// flags
	bool init;			// Initialization - establishment of neighborship
	bool cr;			// Conditionally Received
	bool rs;			// Reset
	bool eot;			// End of table
	
	int seqNum;			// Sequence number
	int seqAck;			// Acknowledgement number
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

// EIGRP Update Message
packet EigrpUpdate extends EigrpMessage
{
};

packet EigrpIpv4Update extends EigrpUpdate
{
    EigrpIpv4Internal interRoutes[];
};

// EIGRP Query Message
packet EigrpQuery extends EigrpMessage
{
};

packet EigrpIpv4Query extends EigrpQuery
{
    EigrpIpv4Internal interRoutes[];
};

// EIGRP Reply Message
packet EigrpReply extends EigrpMessage
{
};

packet EigrpIpv4Reply extends EigrpReply
{
    EigrpIpv4Internal interRoutes[];
};