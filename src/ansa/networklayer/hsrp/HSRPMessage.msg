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

//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |   Version     |   Op Code     |     State     |   Hellotime   |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |   Holdtime    |   Priority    |     Group     |   Reserved    |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                      Authentication  Data                     |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                      Authentication  Data                     |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                      Virtual IP Address                       |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 
// Author: Jan Holusa

cplusplus {{
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
const int HSRP_HEADER_SIZE = 96;
const std::string HSRP_MULTICAST_ADDRESS = "224.0.0.2";
const int HSRP_UDP_PORT = 1985;
}}

class noncobject IPv4Address;

namespace inet;

///
/// HSRP message opcodes
///
enum OP_CODE
{
    HELLO = 0;
    COUP = 1;
    RESIGN = 2;
    //TODO ADVERTISE = 3;
    //each change into ACTIVE or from the ACTIVE
    //CISCO only.
}

///
/// HSRP states
///
enum STATE
{
    INIT = 0;
    LEARN = 1;
    LISTEN = 2;
    SPEAK = 4;
    STANDBY = 8;
    ACTIVE = 16;
    DISABLED = 32; //Just local state- not sending in message
}

packet HSRPMessage {
    unsigned char version = 0;	///version, RFC 2281 declares 0.
    unsigned char op_code;		///opcode of message
    unsigned char state;		///HSRP state
	unsigned char hellotime;	///hellotime value
	unsigned char holdtime;		///holdtime value
	unsigned char priority;		///priority value
	unsigned char group;		///group number value
	IPv4Address address;		///IPv4 address of this message
}
