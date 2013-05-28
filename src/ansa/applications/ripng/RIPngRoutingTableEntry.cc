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
* @file RIPngRoutingTableEntry.cc
* @author Jiri Trhlik (mailto:jiritm@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
* @brief RIPng RTE
* @detail Represents RIPng Routing Table Entry
*/

#include "RIPngRoutingTableEntry.h"

#include "omnetpp.h"

#include "InterfaceTableAccess.h"

namespace RIPng
{

RoutingTableEntry::RoutingTableEntry(IPv6Address destPrefix, int length) :
    ANSAIPv6Route(destPrefix, length, IPv6Route::ROUTING_PROT),
    _changeFlag(false),
    _routeTag(0)
{
    setRoutingProtocolSource(pRIP);
    setProcess(NULL);
    setTimer(NULL);
    setGCTimer(NULL);
    setCopy(NULL);
}

RoutingTableEntry::RoutingTableEntry(RoutingTableEntry& entry) :
    ANSAIPv6Route(entry.getDestPrefix(), entry.getPrefixLength(), IPv6Route::ROUTING_PROT),
    _changeFlag(false)
{
    setProcess(entry.getProcess());
    setRoutingProtocolSource(entry.getRoutingProtocolSource());
    setAdminDist(entry.getAdminDist());
    setTimer(NULL);
    setGCTimer(NULL);
    setNextHop(entry.getNextHop());
    setInterfaceId(entry.getInterfaceId());
    setMetric(entry.getMetric());
    setRouteTag(entry.getRouteTag());
    setCopy(&entry);
}

RoutingTableEntry::~RoutingTableEntry()
{
    if (_copy != NULL)
        _copy->setCopy(NULL);
}

std::string RoutingTableEntry::RIPngInfo() const
{
    simtime_t timerLen;
    std::string expTime;
    std::stringstream out;

    if (getDestPrefix().isUnspecified())
        out << "::";
    else
        out << getDestPrefix();
    out << "/" << getPrefixLength();
    out << ", metric " << getMetric();
    if (getCopy() != NULL)
        out << ", installed";

    if (!getNextHop().isUnspecified())
    {//Not directly connected
        RIPngTimer *timer = getTimer();
        if (timer != NULL && timer->isScheduled())
        {
            timerLen = timer->getArrivalTime() - simTime();
            expTime = timerLen.str();
            expTime = expTime.substr(0, expTime.find_first_of(".,"));
            out << ", expires in " << expTime << " secs";
        }
        else
        {//Route expired
            out << ", expired";
            RIPngTimer *GCTimer = getGCTimer();
            if (GCTimer != NULL && GCTimer->isScheduled())
            {
                timerLen = GCTimer->getArrivalTime() - simTime();
                expTime = timerLen.str();
                expTime = expTime.substr(0, expTime.find_first_of(".,"));
                out << ", [advertise " << expTime << "]";
            }
        }
    }

    out << ", " << ift->getInterfaceById(getInterfaceId())->getName();
    if (!getNextHop().isUnspecified())
        out << "/" << getNextHop();

    return out.str();
}

} /* namespace RIPng */
