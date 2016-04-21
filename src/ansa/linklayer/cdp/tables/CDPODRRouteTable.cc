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

#include "ansa/linklayer/cdp/CDPODRRouteTable.h"

namespace inet {

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
    ODRInvalideTime->setTimerType(CDPTimerType::ODRInvalideTime);
    ODRInvalideTime->setContextPointer(this);

    ODRHolddown = new CDPTimer();
    ODRHolddown->setTimerType(CDPTimerType::ODRHolddown);
    ODRHolddown->setContextPointer(this);

    ODRFlush = new CDPTimer();
    ODRFlush->setTimerType(CDPTimerType::ODRFlush);
    ODRFlush->setContextPointer(this);
}

/**
 * Cancel and delete event
 *
 * @param   timer   event to delete
 */
void CDPODRRoute::deleteTimer(CDPTimer *timer)
{
    if(timer != nullptr)
    {
        //if is scheduled, get his sender module, otherwise get owner module
        cSimpleModule *owner = dynamic_cast<cSimpleModule *>((timer->isScheduled()) ? timer->getSenderModule() : timer->getOwner());
        if(owner != NULL)
        {// owner is cSimpleModule object -> can call his methods
            owner->cancelAndDelete(timer);
            timer = NULL;
        }
    }
}

/**
 * find odr route by destination, prefix length and next hope
 */
CDPODRRoute *CDPODRRouteTable::findRoute(const L3Address& destination, int prefixLength, const L3Address& nextHope)
{
    for (auto it = routes.begin(); it != routes.end(); ++it){
        if ((*it)->getDestination() == destination && (*it)->getPrefixLength() == prefixLength
                && (*it)->getNextHop() == nextHope)
            return *it;
    }
    return nullptr;
}

/**
 * find odr route by route
 */
CDPODRRoute *CDPODRRouteTable::findRoute(const IRoute *route)
{
    for (auto it = routes.begin(); it != routes.end(); ++it)
        if ((*it)->getRoute() == route)
            return *it;

    return nullptr;
}


/**
 * return other default route than that in param. In case that no other default route exist
 * return null
 *
 * @param   odrRoute    not look for this route
 *
 * @return  other       default route or null
 */
CDPODRRoute *CDPODRRouteTable::existOtherDefaultRoute(CDPODRRoute *route)
{
    for (auto it = routes.begin(); it != routes.end(); ++it)
    {
        if ((*it)->getRoute() != nullptr && route->getRoute() != nullptr &&
                (*it)->getRoute()->getDestinationAsGeneric().isUnspecified() && (*it)->getRoute() != route->getRoute())
            return *it;
    }

    return nullptr;
}

/**
 * count all paths to destination
 *
 * @param   destination
 * @param   prefixLength
 *
 * @return  number of paths
 */
int CDPODRRouteTable::countDestinationPaths(const L3Address& destination, int prefixLength)
{
    int destinationPaths = 0;
    for (auto it = routes.begin(); it != routes.end(); ++it)
        if ((*it)->getDestination() == destination && (*it)->getPrefixLength() == prefixLength
                && (*it)->getODRFlush())
            destinationPaths++;

    return destinationPaths;
}

/**
 * add neighbour to neighbour table
 *
 * @param   neighbour   neighbour to add
 */
void CDPODRRouteTable::addRoute(CDPODRRoute * route)
{
    if(findRoute(route->getDestination(), route->getPrefixLength(), route->getNextHop()) != NULL)
    {// neighbour already in table
        throw cRuntimeError("Adding to CDPODRRoute route, which is already in it.");
    }

    routes.push_back(route);
}


/**
 * Remove all neighbours
 *
 */
void CDPODRRouteTable::removeRoutes()
{
    std::vector<CDPODRRoute *>::iterator it;

    for (it = routes.begin(); it != routes.end();)
    {
        delete (*it);
        it = routes.erase(it);
    }
}

/**
 * Remove route
 *
 * @param   route   route to delete
 *
 */
void CDPODRRouteTable::removeRoute(CDPODRRoute * route)
{
    std::vector<CDPODRRoute *>::iterator it;

    for (it = routes.begin(); it != routes.end();)
    {// through all routes
        if((*it) == route)
        {// found same
            delete (*it);
            it = routes.erase(it);
            return;
        }
        else
        {// do not delete -> get next
            ++it;
        }
    }
}

CDPODRRouteTable::~CDPODRRouteTable()
{
    std::vector<CDPODRRoute *>::iterator it;

    for (it = routes.begin(); it != routes.end(); ++it)
    {// through all routes
        delete (*it);
    }
    routes.clear();
}
} /* namespace inet */
