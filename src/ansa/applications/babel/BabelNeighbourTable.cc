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
* @file BabelNeighbourTable.cc
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
* @brief Babel Neighbour Table
* @detail Represents data structure of Babel routing neighbour
*/

#include "ansa/applications/babel/BabelNeighbourTable.h"
#include <bitset>

using namespace Babel;


BabelNeighbour::~BabelNeighbour()
{
    deleteNHTimer();
    deleteNITimer();
}


std::ostream& operator<<(std::ostream& os, const BabelNeighbour& bn)
{
    return os << bn.str();
}



std::string BabelNeighbour::str() const
{
    std::stringstream string;

#ifdef BABEL_DEBUG
    string << "Neighbour: " << address;
    string << ", on interface: " << interface->getIfaceName();
    string << ", history: " <<  static_cast<std::bitset<16> >(history);
    string << ", cost: " << cost;
    string << ", txcost: " << txcost;
    string << ", rxcost: " << computeRxcost();
    string << ", expected H-seqno: " << expHSeqno;
    string << ", hello interval: " << neighHelloInterval;
    string << ", ihu interval: " << neighIhuInterval;
    if(neighHelloTimer != NULL && neighHelloTimer->isScheduled()) string << ", hello timeout at T=" << neighHelloTimer->getArrivalTime();
    if(neighIhuTimer != NULL && neighIhuTimer->isScheduled()) string << ", ihu timeout at T=" << neighIhuTimer->getArrivalTime();
#else
    string << address;
    string << " on " << interface->getIfaceName();
    string << " H:" <<  static_cast<std::bitset<16> >(history);
    string << " cost:" << cost;
    string << " txc:" << txcost;
    string << " rxc:" << computeRxcost();
    string << " eHsn:" << expHSeqno;
    string << " Hint:" << neighHelloInterval;
    string << " Iint:" << neighIhuInterval;
#endif
    return string.str();
}

/**
 * Recomputes cost from actual rxcost and txcost
 *
 * @return  True if cost has changed, otherwise false
 */
bool BabelNeighbour::recomputeCost()
{
    uint16_t newcost = COST_INF;

    if(!interface->getInterface()->isUp())
    {// interface is down
        newcost = COST_INF;
    }
    else if(txcost == COST_INF)
    {// txcost is INFINITY -> cost MUST be INFINITY
        newcost = COST_INF;
    }
    else
    {
        if(interface->getCostComputationModule())
        {
            newcost = interface->getCostComputationModule()->computeCost(history, interface->getNominalRxcost(), txcost);
        }
    }



    if(cost != newcost)
    {// new cost is different -> set and indicate change
        cost = newcost;
        return true;
    }

    return false;
}

/**
 * Recomputes RXCOST
 */
uint16_t BabelNeighbour::computeRxcost() const
{
    uint16_t rxcost = COST_INF;

    if(interface->getCostComputationModule())
    {
        rxcost = interface->getCostComputationModule()->computeRxcost(history, interface->getNominalRxcost());
    }

    return rxcost;
}

void BabelNeighbour::resetNHTimer()
{
    ASSERT(neighHelloTimer != NULL);

    resetTimer(neighHelloTimer, CStoS(neighHelloInterval));
}


void BabelNeighbour::resetNITimer()
{
    ASSERT(neighIhuTimer != NULL);

    resetTimer(neighIhuTimer, CStoS(neighIhuInterval));
}


void BabelNeighbour::resetNHTimer(double delay)
{
    ASSERT(neighHelloTimer != NULL);

    resetTimer(neighHelloTimer, delay);
}


void BabelNeighbour::resetNITimer(double delay)
{
    ASSERT(neighIhuTimer != NULL);

    resetTimer(neighIhuTimer, delay);
}



void BabelNeighbour::deleteNHTimer()
{
    if(neighHelloTimer != NULL)
    {// existing timer -> get his owner (cSimpleModule)

        //if is scheduled, get his sender module, otherwise get owner module
        cSimpleModule *owner = dynamic_cast<cSimpleModule *>((neighHelloTimer->isScheduled()) ? neighHelloTimer->getSenderModule() : neighHelloTimer->getOwner());
        if(owner != NULL)
        {// owner is cSimpleModule object -> can call his methods
            owner->cancelAndDelete(neighHelloTimer);
            neighHelloTimer = NULL;
        }
    }
}


void BabelNeighbour::deleteNITimer()
{
    if(neighIhuTimer != NULL)
    {// existing timer -> get his owner (cSimpleModule)

        //if is scheduled, get his sender module, otherwise get owner module
        cSimpleModule *owner = dynamic_cast<cSimpleModule *>((neighIhuTimer->isScheduled()) ? neighIhuTimer->getSenderModule() : neighIhuTimer->getOwner());
        if(owner != NULL)
        {// owner is cSimpleModule object -> can call his methods
            owner->cancelAndDelete(neighIhuTimer);
            neighIhuTimer = NULL;
        }
    }
}

int BabelNeighbourTable::getNumOfNeighOnIface(BabelInterface *iface)
{
    std::vector<BabelNeighbour *>::iterator it;
    int numofneigh = 0;

    for (it = neighbours.begin(); it != neighbours.end(); ++it)
    {// through all neighbours search for same interface
        if((*it)->getInterface() == iface)
        {// found same
            ++numofneigh;
        }
    }

    return numofneigh;
}

BabelNeighbour *BabelNeighbourTable::findNeighbour(BabelInterface *iface, const inet::L3Address& addr)
{
    std::vector<BabelNeighbour *>::iterator it;

//TODO - kontrolovat i interface??
    for (it = neighbours.begin(); it != neighbours.end(); ++it)
    {// through all neighbours search for same address
        if((*it)->getAddress() == addr && (*it)->getInterface() == iface)
        {// found same
            return (*it);
        }
    }

    return NULL;
}


BabelNeighbour *BabelNeighbourTable::addNeighbour(BabelNeighbour *neigh)
{
    ASSERT(neigh != NULL);

    BabelNeighbour *intable = findNeighbour(neigh->getInterface(), neigh->getAddress());

    if(intable != NULL)
    {// neigh already in table
        return intable;
    }

    neighbours.push_back(neigh);

    return neigh;
}

void BabelNeighbourTable::removeNeighbour(BabelNeighbour *neigh)
{
    std::vector<BabelNeighbour *>::iterator it;

    for (it = neighbours.begin(); it != neighbours.end();)
    {// through all neighbours
        if((*it) == neigh)
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

void BabelNeighbourTable::removeNeighboursOnIface(BabelInterface *iface)
{
    std::vector<BabelNeighbour *>::iterator it;

    for (it = neighbours.begin(); it != neighbours.end();)
    {// through all neighbours
        if((*it)->getInterface() == iface)
        {// found neighbour on same interface
            delete (*it);
            it = neighbours.erase(it);
        }
        else
        {// do not delete -> get next
            ++it;
        }
    }
}


BabelNeighbourTable::~BabelNeighbourTable()
{
    std::vector<BabelNeighbour *>::iterator it;

    for (it = neighbours.begin(); it != neighbours.end(); ++it)
    {// through all interfaces
        delete (*it);
    }
    neighbours.clear();
}

