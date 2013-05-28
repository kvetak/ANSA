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
// @file RIPngTimer.msg
// @author Jiri Trhlik (mailto:jiritm@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
// @brief RIPng timer
// @detail RIPng timer

cplusplus {{
#include "IPv6Address.h"
}}

class noncobject IPv6Address;

enum RIPngTimerType
{
    RIPNG_GENERAL_UPDATE = 0;
    RIPNG_TRIGGERED_UPDATE = 1;
    RIPNG_ROUTE_TIMEOUT = 2;
    RIPNG_ROUTE_GARBAGE_COLECTION_TIMEOUT = 3;
}

//
// RIPng Timers
// For General Update Message,
// Route Timeout and
// Garbage-Collection Time.
//
packet RIPngTimer 
{
    char timerKind enum(RIPngTimerType) = RIPNG_GENERAL_UPDATE;
}
