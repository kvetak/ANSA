//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


cplusplus {{
#include "IPvXAddress.h"
#include "UDPControlInfo_m.h"
}}

class noncobject IPvXAddress;


class AnsaUDPControlInfo
{
    int sockId = -1;   // uniquely identifies the \UDP socket
    int userId = -1;   // id than can be freely used by the app
    IPvXAddress srcAddr;  // source \IP or \IPv6 address, or local address with BIND
    IPvXAddress destAddr; // destination \IP or \IPv6 address
    unsigned char diffServCodePoint;  // for QoS
    short timeToLive;
    int srcPort;   // \UDP source port in packet, or local port with BIND
    int destPort;  // \UDP destination port in packet
    int interfaceId = -1; // interface on which pk was received/should be sent (see InterfaceTable)
}

