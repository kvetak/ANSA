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
// @file RIPngMessage.msg
// @author Jiri Trhlik (mailto:), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
// @brief
// @detail


cplusplus {{
#include "IPv6Address.h"
}}

class noncobject IPv6Address;

//
//  RIPng command
//
enum RIPngCommand
{
    RIPngRequest = 1;
    RIPngResponse = 2;  
}

//
// Routing table entry structure
//
class RIPngRTE
{
    IPv6Address IPv6Prefix;   //128 bytes
    uint16_t    routeTag;     //2 bytes
    char        prefixLen;    //1 byte
    char        metric;       //1 byte
}

//
// Represents a RIPng message
//
packet RIPngMessage 
{
    char     command enum(RIPngCommand); //1 byte
    char     version = 1;                //1 byte
    uint16_t mustBeZero  = 0;            //2 bytes
    RIPngRTE rtes[];                     //Routing Table Entries
}
