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
/**
* @file RIPngInterface.cc
* @author Jiri Trhlik (mailto:), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
* @brief
* @detail
*/

#include "RIPngRoutingTableEntry.h"

#include "omnetpp.h"

namespace RIPng
{

RoutingTableEntry::RoutingTableEntry(IPv6Address destPrefix, int length) :
    ANSAIPv6Route(destPrefix, length, IPv6Route::ROUTING_PROT),
    _changeFlag(false),
    _routeTag(0)
{
    setTimer(NULL);
    setGCTimer(NULL);
    setCopy(NULL);
}

RoutingTableEntry::RoutingTableEntry(RoutingTableEntry& entry) :
    ANSAIPv6Route(entry.getDestPrefix(), entry.getPrefixLength(), IPv6Route::ROUTING_PROT),
    _changeFlag(false),
    _routeTag(0)
{
    setTimer(NULL);
    setGCTimer(NULL);
    setNextHop(entry.getNextHop());
    setInterfaceId(entry.getInterfaceId());
    setMetric(entry.getMetric());
    setCopy(&entry);
}

RoutingTableEntry::~RoutingTableEntry()
{
}

} /* namespace RIPng */
