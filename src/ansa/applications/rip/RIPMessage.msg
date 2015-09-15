// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
// @file RIPMessage.msg
// @author Jiri Trhlik (mailto:jiritm@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
// @brief RIP message
// @detail RIP protocol message (same format as is defined in RFC 2453)


cplusplus {{
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
}}

class noncobject inet::IPv4Address;

//
//  RIP command
//
enum RIPCommand
{
    RIPRequest = 1;
    RIPResponse = 2;  
}

//
// Routing table entry structure
//
class RIPRTE
{
    uint16_t	AFI  = 2;      //Address Family Identifier (2 for IP) 2 bytes 
    uint16_t	routeTag = 0;  //2 bytes
    inet::IPv4Address IPv4Address;   //4 bytes
    uint32_t    netMask;       //4 bytes
    inet::IPv4Address nextHop;       //4 bytes
    uint32_t    metric;        //4 byte
}

//
// Represents a RIP message
//
packet RIPMessage 
{
    char     command enum(RIPCommand);   //1 byte
    char     version = 2;                //1 byte
    uint16_t mustBeZero  = 0;            //2 bytes
    RIPRTE   rtes[];                     //Routing Table Entries
}
