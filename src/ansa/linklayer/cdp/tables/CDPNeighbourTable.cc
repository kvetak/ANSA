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

#include "ansa/linklayer/cdp/tables/CDPNeighbourTable.h"

namespace inet {

CDPNeighbour::CDPNeighbour():
        interface(nullptr), address(nullptr), odrDefaultGateway(nullptr)
{
    holdtimeTimer = new CDPTimer();
    holdtimeTimer->setTimerType(Holdtime);
    holdtimeTimer->setContextPointer(this);
}

CDPNeighbour::~CDPNeighbour() {
    if(holdtimeTimer != nullptr)
    {
        //if is scheduled, get his sender module, otherwise get owner module
        cSimpleModule *owner = dynamic_cast<cSimpleModule *>((holdtimeTimer->isScheduled()) ? holdtimeTimer->getSenderModule() : holdtimeTimer->getOwner());
        if(owner != NULL)
        {// owner is cSimpleModule object -> can call his methods
            owner->cancelAndDelete(holdtimeTimer);
            holdtimeTimer = NULL;
        }
    }
}

std::string CDPNeighbour::info() const
{
    std::stringstream out;

    out << name << ", local int: " << interface->getName();
    out << ", holdtime: " << round(ttl.dbl()-(simTime()-lastUpdate).dbl()) << ", cap: " << capabilities;
    out << ", send int: " << portSend;
    return out.str();
}

/**
 * Get cdp neighbour from neighbour table by name and receive port
 *
 * @param   name    name of neighbour
 * @param   port    receive port number
 *
 * @return  cdp neighbour
 */
CDPNeighbour * CDPNeighbourTable::findNeighbour(std::string name, int port)
{
    std::vector<CDPNeighbour *>::iterator it;

    for (it = neighbours.begin(); it != neighbours.end(); ++it)
    {// through all neighbours search for same neighbour with same name and port
        if((*it)->getName() == name && (*it)->getPortReceive() == port)
        {// found same
            return (*it);
        }
    }

    return nullptr;
}

/**
 * add neighbour to neighbour table
 *
 * @param   neighbour   neighbour to add
 */
void CDPNeighbourTable::addNeighbour(CDPNeighbour * neighbour)
{
    if(findNeighbour(neighbour->getName(), neighbour->getPortReceive()) != NULL)
    {// neighbour already in table
        //TODO: otestovat %s
        throw cRuntimeError("Adding to CDPNeighbourTable neighbour, which is already in it - name %s, port %d", neighbour->getName(), neighbour->getPortReceive());
    }

    neighbours.push_back(neighbour);
}


/**
 * Remove all neighbours
 *
 */
void CDPNeighbourTable::removeNeighbours()
{
    std::vector<CDPNeighbour *>::iterator it;

    for (it = neighbours.begin(); it != neighbours.end();)
    {
        delete (*it);
        it = neighbours.erase(it);
    }
}

/**
 * Remove neighbour
 *
 * @param   neighbour   neighbour to delete
 *
 */
void CDPNeighbourTable::removeNeighbour(CDPNeighbour * neighbour)
{
    std::vector<CDPNeighbour *>::iterator it;

    for (it = neighbours.begin(); it != neighbours.end();)
    {// through all neighbours
        if((*it) == neighbour)
        {// found same
            delete (*it);
            it = neighbours.erase(it);
            return;
        }
        else
        {// do not delete -> get next
            ++it;
        }
    }
}

/**
 * Removes neighbours
 *
 * @param   name    name of neighbour
 * @param   port    receive port number
 */
void CDPNeighbourTable::removeNeighbour(std::string name, int port)
{
    std::vector<CDPNeighbour *>::iterator it;

    for (it = neighbours.begin(); it != neighbours.end();)
    {// through all neighbours search for same neighbour with same name and port
        if((*it)->getName() == name && (*it)->getPortReceive() == port)
        {// found same
            delete (*it);
            it = neighbours.erase(it);
            return;
        }
        else
        {// do not delete -> get next
            ++it;
        }
    }
}

CDPNeighbourTable::~CDPNeighbourTable()
{
    std::vector<CDPNeighbour *>::iterator it;

    for (it = neighbours.begin(); it != neighbours.end(); ++it)
    {// through all neighbours
        delete (*it);
    }
    neighbours.clear();
}

} /* namespace inet */
