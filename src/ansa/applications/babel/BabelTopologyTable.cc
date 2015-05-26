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
* @file BabelTopologyTable.cc
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
* @brief Babel Topology table
* @detail Represents data structure for saving all known routes
*/

#include <BabelTopologyTable.h>

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

BabelRoute *BabelTopologyTable::findRoute(const Babel::netPrefix<IPvXAddress>& p, BabelNeighbour *n, const Babel::rid& orig)
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
BabelRoute *BabelTopologyTable::findRoute(const Babel::netPrefix<IPvXAddress>& p, BabelNeighbour *n)
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

BabelRoute *BabelTopologyTable::findSelectedRoute(const Babel::netPrefix<IPvXAddress>& p)
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

bool BabelTopologyTable::containShorterCovRoute(const Babel::netPrefix<IPvXAddress>& p)
{
    std::vector<BabelRoute *>::iterator it;

    for (it = routes.begin(); it != routes.end(); ++it)
    {// through all routes search for shorter prefix that covering prefix p
        if(((*it)->getPrefix().getAddr().isIPv6() == p.getAddr().isIPv6()) &&
                (*it)->getPrefix().getLen() < p.getLen())
        {// same AF and shorter
            if((*it)->getPrefix().getAddr().isIPv6())
            {// IPv6
                if((*it)->getPrefix().getAddr().get6() == p.getAddr().get6().getPrefix((*it)->getPrefix().getLen()))
                {
                    return true;
                }
            }
            else
            {// IPv4
                if((*it)->getPrefix().getAddr().get4() == p.getAddr().get4().doAnd(IPv4Address::makeNetmask((*it)->getPrefix().getLen())))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

BabelRoute *BabelTopologyTable::findRouteNotNH(const Babel::netPrefix<IPvXAddress>& p, const IPvXAddress& nh)
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

