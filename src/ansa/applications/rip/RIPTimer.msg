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
// @file RIPTimer.msg
// @author Jiri Trhlik (mailto:jiritm@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
// @brief RIP timer
// @detail RIP timer

cplusplus {{
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
}}

class noncobject inet::IPv4Address;

enum RIPTimerType
{
    RIP_GENERAL_UPDATE = 0;
    RIP_TRIGGERED_UPDATE = 1;
    RIP_ROUTE_TIMEOUT = 2;
    RIP_ROUTE_GARBAGE_COLECTION_TIMEOUT = 3;
}

//
// RIP Timers
// For General Update Message,
// Route Timeout and
// Garbage-Collection Time.
//
packet RIPTimer 
{
    char timerKind enum(RIPTimerType) = RIP_GENERAL_UPDATE;
}
