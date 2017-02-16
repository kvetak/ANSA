//
// Copyright (C) 2009 - today, Brno University of Technology, Czech Republic
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
/**
* @file CDPODRRouteTable.cc
* @author Tomas Rajca
*/


#include "ansa/linklayer/cdp/tables/CDPODRRouteTable.h"
#include <algorithm>
#include <math.h>

namespace inet {

Register_Abstract_Class(CDPODRRoute);
Define_Module(CDPODRRouteTable);

std::string CDPODRRoute::info() const
{
    std::stringstream out;
    if (dest.isUnspecified())
        out << "0.0.0.0";
    else
        out << dest;

    out << "/" << prefixLength;

    out << " via ";
    if (nextHop.isUnspecified())
        out << "*";
    else
        out << nextHop;
    out << ", " <<  ie->getName();

    if(invalide)
        out << ", invalid route (flush in " << round((ODRFlush->getArrivalTime()-simTime()).dbl()) <<  "s)";
    else
        out << ", " << round((simTime() - lastUpdateTime).dbl()) << "s";

    return out.str();
}


CDPODRRoute::CDPODRRoute(L3Address des, int pre, L3Address nex, InterfaceEntry *i)
    : route(nullptr), invalide(false)
{
    dest = des;
    prefixLength = pre;
    nextHop = nex;
    ie = i;

    createTimers();
}

CDPODRRoute::CDPODRRoute(IRoute *route)
    : route(route), invalide(false)
{
    dest = route->getDestinationAsGeneric();
    prefixLength = route->getPrefixLength();
    nextHop = route->getNextHopAsGeneric();
    ie = route->getInterface();

    createTimers();
}

CDPODRRoute::~CDPODRRoute()
{
    deleteTimer(ODRInvalideTime);
    deleteTimer(ODRHolddown);
    deleteTimer(ODRFlush);
}

void CDPODRRoute::createTimers()
{
    ODRInvalideTime = new CDPTimer();
    ODRInvalideTime->setTimerType(CDPTimerType::CDPODRInvalideTime);
    ODRInvalideTime->setContextPointer(this);

    ODRHolddown = new CDPTimer();
    ODRHolddown->setTimerType(CDPTimerType::CDPODRHolddown);
    ODRHolddown->setContextPointer(this);

    ODRFlush = new CDPTimer();
    ODRFlush->setTimerType(CDPTimerType::CDPODRFlush);
    ODRFlush->setContextPointer(this);
}

void CDPODRRoute::deleteTimer(CDPTimer *timer)
{
    if(timer != nullptr)
    {
        //if is scheduled, get his sender module, otherwise get owner module
        cSimpleModule *owner = dynamic_cast<cSimpleModule *>((timer->isScheduled()) ? timer->getSenderModule() : timer->getOwner());
        if(owner != nullptr)
        {// owner is cSimpleModule object -> can call his methods
            owner->cancelAndDelete(timer);
            timer = nullptr;
        }
    }
}

void CDPODRRouteTable::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        WATCH_PTRVECTOR(routes);
    }
}

void CDPODRRouteTable::handleMessage(cMessage *)
{

}

CDPODRRoute *CDPODRRouteTable::findRoute(const L3Address& destination, int prefixLength, const L3Address& nextHope)
{
    for (auto it = routes.begin(); it != routes.end(); ++it){
        if ((*it)->getDestination() == destination && (*it)->getPrefixLength() == prefixLength
                && (*it)->getNextHop() == nextHope)
            return *it;
    }
    return nullptr;
}

CDPODRRoute *CDPODRRouteTable::findRoute(const IRoute *route)
{
    for (auto it = routes.begin(); it != routes.end(); ++it)
        if ((*it)->getRoute() == route)
            return *it;

    return nullptr;
}

int CDPODRRouteTable::countDestinationPaths(const L3Address& destination, uint8_t prefixLength)
{
    int destinationPaths = 0;
    for (auto it = routes.begin(); it != routes.end(); ++it)
        if ((*it)->getDestination() == destination && (*it)->getPrefixLength() == prefixLength
                && (*it)->getODRFlush())
            destinationPaths++;

    return destinationPaths;
}

void CDPODRRouteTable::addRoute(CDPODRRoute * route)
{
    if(findRoute(route->getDestination(), route->getPrefixLength(), route->getNextHop()) != nullptr)
    {// route already in table
        throw cRuntimeError("Adding to CDPODRRoute route, which is already in it.");
    }

    routes.push_back(route);
}

void CDPODRRouteTable::removeRoutes()
{
    for (auto it = routes.begin(); it != routes.end();)
    {
        delete *it;
        routes.erase(it);
    }
}

void CDPODRRouteTable::removeRoute(CDPODRRoute *route)
{
    auto r = find(routes.begin(), routes.end(), route);
    if (r != routes.end())
    {
        delete *r;
        routes.erase(r);
    }
}

CDPODRRouteTable::~CDPODRRouteTable()
{
    for (auto & route : routes)
        delete route;
}
} /* namespace inet */
