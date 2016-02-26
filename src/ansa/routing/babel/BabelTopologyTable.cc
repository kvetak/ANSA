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
* @file BabelTopologyTable.cc
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
* @brief Babel Topology table
* @detail Represents data structure for saving all known routes
*/

#include "ansa/routing/babel/BabelTopologyTable.h"
namespace inet {
using namespace Babel;

BabelRoute::~BabelRoute()
{
    deleteETimer();
    deleteBETimer();
}
std::ostream& operator<<(std::ostream& os, const BabelRoute& br)
{
    return os << br.str();
}

std::string BabelRoute::str() const
{
    std::stringstream string;
#ifdef BABEL_DEBUG
    if(selected)
    {
        string << "> ";
    }
    else
    {
        string << "  ";
    }

    string << prefix;
    if(neighbour != NULL)
    {
        string << ", next hop: " << nexthop;
    }
    else
    {
        string << ", local ";
    }

    string << ", metric: " << metric();
    string << ", originator: " << originator;

    if(neighbour != NULL)
    {
        string << ", learned from: " << neighbour->getAddress();
        string << ", reported distance: " << rdistance;
    }

    if(rtentry)
    {
        string<< ", installed";
    }

#else
    if(selected)
    {
        string << "> ";
    }
    else
    {
        string << "  ";
    }

    string << prefix;
    if(neighbour != NULL)
    {
        string << " NH:" << nexthop;
    }
    else
    {
        string << " local";
    }

    string << " metric:" << metric();
    string << " orig:" << originator;

    if(neighbour != NULL)
    {
        string << " from:" << neighbour->getAddress();
        string << " RD:" << rdistance;
    }

    if(rtentry)
    {
        string<< ", in RT";
    }
#endif
    return string.str();
}

uint16_t BabelRoute::metric() const
{
    if(neighbour != NULL)
    {
        if(neighbour->getCost() == COST_INF)
        {
            return COST_INF;
        }


        unsigned int tmpmetric = rdistance.getMetric() + neighbour->getCost();

        if(tmpmetric >= COST_INF)
        {// sum is greater than INFINITY -> return INFINITY
            return COST_INF;
        }

        return tmpmetric;
    }
    else
    {
        return rdistance.getMetric();
    }
}


void BabelRoute::resetETimer()
{
    ASSERT(expiryTimer != NULL);

    resetTimer(expiryTimer, defval::ROUTE_EXPIRY_INTERVAL_MULT * CStoS(updateInterval));
}

void BabelRoute::resetETimer(double delay)
{
    ASSERT(expiryTimer != NULL);

    resetTimer(expiryTimer, delay);
}

void BabelRoute::deleteETimer()
{
    deleteTimer(&expiryTimer);
}


void BabelRoute::resetBETimer()
{
    ASSERT(befExpiryTimer != NULL);

    resetTimer(befExpiryTimer, (defval::ROUTE_EXPIRY_INTERVAL_MULT * CStoS(updateInterval) * 7.0) / 8.0);
}

void BabelRoute::resetBETimer(double delay)
{
    ASSERT(befExpiryTimer != NULL);

    resetTimer(befExpiryTimer, delay);
}

void BabelRoute::deleteBETimer()
{
    deleteTimer(&befExpiryTimer);
}


BabelTopologyTable::~BabelTopologyTable()
{
    removeRoutes();
}

void BabelTopologyTable::removeRoutes()
{
    std::vector<BabelRoute *>::iterator it;

    for (it = routes.begin(); it != routes.end(); ++it)
    {// through all routes
        delete (*it);
    }
    routes.clear();
}

BabelRoute *BabelTopologyTable::findRoute(const Babel::netPrefix<L3Address>& p, BabelNeighbour *n, const Babel::rid& orig)
{
    std::vector<BabelRoute *>::iterator it;

    for (it = routes.begin(); it != routes.end(); ++it)
    {// through all routes search for same prefix and routerid
        if(((*it)->getPrefix() == p) && ((*it)->getNeighbour() == n) && ((*it)->getOriginator() == orig))
        {// found same
            return (*it);
        }
    }

    return NULL;
}
BabelRoute *BabelTopologyTable::findRoute(const Babel::netPrefix<L3Address>& p, BabelNeighbour *n)
{
    std::vector<BabelRoute *>::iterator it;

    for (it = routes.begin(); it != routes.end(); ++it)
    {// through all routes search for same prefix and routerid
        if(((*it)->getPrefix() == p) && ((*it)->getNeighbour() == n))
        {// found same
            return (*it);
        }
    }

    return NULL;
}

BabelRoute *BabelTopologyTable::findSelectedRoute(const Babel::netPrefix<L3Address>& p)
{
    std::vector<BabelRoute *>::iterator it;

    for (it = routes.begin(); it != routes.end(); ++it)
    {// through all routes search for same prefix and selected flag
        if(((*it)->getPrefix() == p) && ((*it)->getSelected()))
        {// found same
            return (*it);
        }
    }

    return NULL;
}

bool BabelTopologyTable::containShorterCovRoute(const Babel::netPrefix<L3Address>& p)
{
    std::vector<BabelRoute *>::iterator it;

    for (it = routes.begin(); it != routes.end(); ++it)
    {// through all routes search for shorter prefix that covering prefix p
        if( ( ((*it)->getPrefix().getAddr().getType()==L3Address::IPv6) == (p.getAddr().getType()==L3Address::IPv6)) &&
            (*it)->getPrefix().getLen() < p.getLen()
          )
        {// same AF and shorter
            if((*it)->getPrefix().getAddr().getType()==L3Address::IPv6)
            {// IPv6
                if((*it)->getPrefix().getAddr().toIPv6() == p.getAddr().toIPv6().getPrefix((*it)->getPrefix().getLen()))
                {
                    return true;
                }
            }
            else
            {// IPv4
                if((*it)->getPrefix().getAddr().toIPv4() == p.getAddr().toIPv4().doAnd(IPv4Address::makeNetmask((*it)->getPrefix().getLen())))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

BabelRoute *BabelTopologyTable::findRouteNotNH(const Babel::netPrefix<L3Address>& p, const L3Address& nh)
{
    std::vector<BabelRoute *>::iterator it;

    for (it = routes.begin(); it != routes.end(); ++it)
    {// through all routes search for same prefix with different next-hop
        if(((*it)->getPrefix() == p) && ((*it)->getNextHop() != nh))
        {// found
            return (*it);
        }
    }

    return NULL;
}

BabelRoute *BabelTopologyTable::addRoute(BabelRoute *route)
{
    ASSERT(route != NULL);

    BabelRoute *intable = findRoute(route->getPrefix(), route->getNeighbour());

    if(intable != NULL)
    {// route already in table
        return intable;
    }

    routes.push_back(route);

    return route;
}
bool BabelTopologyTable::retractRoutesOnIface(BabelInterface *iface)
{
    std::vector<BabelRoute *>::iterator it;
    bool changed = false;

    for (it = routes.begin(); it != routes.end(); ++it)
    {// through all routes
       if((*it)->getNeighbour() != NULL && (*it)->getNeighbour()->getInterface() == iface)
       {// found same
           (*it)->getRDistance().setMetric(0xFFFF);
           changed = true;
       }
    }

    return changed;
}

bool BabelTopologyTable::removeRoute(BabelRoute *route)
{
    std::vector<BabelRoute *>::iterator it;

    for (it = routes.begin(); it != routes.end();)
    {// through all routes
        if((*it) == route)
        {// found same
            delete (*it);
            it = routes.erase(it);
            return true;
        }
        else
        {// do not delete -> get next
            ++it;
        }
    }

    return false;
}

}
