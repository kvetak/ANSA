// The MIT License (MIT)
//
// Copyright (c) 2016 Brno University of Technology
//
//@author Vladimir Vesely (iveselyATfitDOTvutbrDOTcz)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
/**
* @file BabelNeighbourTable.cc
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
* @brief Babel Neighbour Table
* @detail Represents data structure of Babel routing neighbour
*/

#include "ansa/routing/babel/BabelNeighbourTable.h"
#include <bitset>
namespace inet {
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

BabelNeighbour *BabelNeighbourTable::findNeighbour(BabelInterface *iface, const L3Address& addr)
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

}
