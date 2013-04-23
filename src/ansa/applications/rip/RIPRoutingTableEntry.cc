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

#include "RIPRoutingTableEntry.h"

#include "omnetpp.h"

#include "InterfaceTableAccess.h"

namespace RIP
{

RoutingTableEntry::RoutingTableEntry(IPv4Address network, IPv4Address netMask) :
    ANSAIPv4Route(),
    _changeFlag(false),
    _routeTag(0)
{
    setDestination(network);
    setNetmask(netMask);

    setRoutingProtocolSource(pRIP);
    setAdminDist(dRIP);
    setTimer(NULL);
    setGCTimer(NULL);
    setCopy(NULL);
}

RoutingTableEntry::RoutingTableEntry(RoutingTableEntry& entry) :
    ANSAIPv4Route(),
    _changeFlag(false)
{
    setDestination(entry.getDestination());
    setNetmask(entry.getNetmask());

    setRoutingTable(NULL);

    setRoutingProtocolSource(entry.getRoutingProtocolSource());
    setAdminDist(entry.getAdminDist());
    setTimer(NULL);
    setGCTimer(NULL);
    setGateway(entry.getGateway());
    setInterface(entry.getInterface());
    setMetric(entry.getMetric());
    setRouteTag(entry.getRouteTag());
    setCopy(&entry);
}

RoutingTableEntry::~RoutingTableEntry()
{
}

std::string RoutingTableEntry::RIPInfo() const
{
    simtime_t timerLen;
    std::string expTime;
    std::stringstream out;

    out << getDestination() << "/" << getNetmask();
    out << ", metric " << getMetric();
    if (getCopy() != NULL)
        out << ", installed";

    if (getGateway() != IPv4Address::UNSPECIFIED_ADDRESS)
    {//Not directly connected
        RIPTimer *timer = getTimer();
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
            RIPTimer *GCTimer = getGCTimer();
            if (GCTimer != NULL && GCTimer->isScheduled())
            {
                timerLen = GCTimer->getArrivalTime() - simTime();
                expTime = timerLen.str();
                expTime = expTime.substr(0, expTime.find_first_of(".,"));
                out << ", [advertise " << expTime << "]";
            }
        }
    }

    out << ", " << getInterface()->getName();
    if (getGateway() != IPv4Address::UNSPECIFIED_ADDRESS)
        out << "/" << getGateway();

    return out.str();
}

} /* namespace RIPng */
