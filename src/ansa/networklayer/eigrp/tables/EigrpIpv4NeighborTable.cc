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

#include "EigrpIpv4NeighborTable.h"
#include "IPv4Address.h"

Define_Module(EigrpIpv4NeighborTable);

std::ostream& operator<<(std::ostream& os, const EigrpNeighbor<IPv4Address>& neigh)
{
    const char *state = neigh.isStateUp() ? "up" : "pending";
    os << "ID:" << neigh.getNeighborId();
    os << "  IP = " << neigh.getIPAddress();
    os << "  IF ID:" << neigh.getIfaceId();
    os << "  holdInt = " << neigh.getHoldInt();
    os << "  state is " << state;
    os << "  lastSeqNum = " << neigh.getSeqNumber();
    os << "  waitForAck = " << neigh.getAck();
    return os;
}

// Must be there (cancelHoldTimer method)
EigrpIpv4NeighborTable::~EigrpIpv4NeighborTable()
{
    int cnt = neighborVec.size();
    EigrpNeighbor<IPv4Address> *neigh;

    for (int i = 0; i < cnt; i++)
    {
        neigh = neighborVec[i];
        cancelHoldTimer(neigh);
        delete neigh;
    }
    neighborVec.clear();
}

void EigrpIpv4NeighborTable::initialize()
{
    WATCH_PTRVECTOR(neighborVec);
}

void EigrpIpv4NeighborTable::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module does not process messages");
}

// Must be there (EigrpNeighborTable has no method cancelEvent)
void EigrpIpv4NeighborTable::cancelHoldTimer(EigrpNeighbor<IPv4Address> *neigh)
{
    EigrpTimer *timer;

    if ((timer = neigh->getHoldTimer()) != NULL)
    {
        if (timer->isScheduled())
            cancelEvent(timer);
    }
}

EigrpNeighbor<IPv4Address> *EigrpIpv4NeighborTable::findNeighbor(
        const IPv4Address& ipAddress)
{
    NeighborVector::iterator it;
    EigrpNeighbor<IPv4Address> * neigh;

    for (it = neighborVec.begin(); it != neighborVec.end(); it++)
    {
        neigh = *it;
        if (neigh->getIPAddress() == ipAddress)
        {
            return neigh;
        }
    }

    return NULL;
}

EigrpNeighbor<IPv4Address> *EigrpIpv4NeighborTable::findNeighborById(int id)
{
    NeighborVector::iterator it;

    for (it = neighborVec.begin(); it != neighborVec.end(); it++)
    {
        if ((*it)->getNeighborId() == id)
        {
            return *it;
        }
    }

    return NULL;
}

int EigrpIpv4NeighborTable::addNeighbor(EigrpNeighbor<IPv4Address> *neighbor)
{
    neighbor->setNeighborId(neighborCounter);
    this->neighborVec.push_back(neighbor);
    return neighborCounter++;
}

/**
 * Removes neighbor form the table, but the record still exists.
 */
EigrpNeighbor<IPv4Address> *EigrpIpv4NeighborTable::removeNeighbor(EigrpNeighbor<IPv4Address> *neighbor)
{
    NeighborVector::iterator it;

    cancelHoldTimer(neighbor);

    for (it = neighborVec.begin(); it != neighborVec.end(); ++it)
    {
        if (*it == neighbor)
        {
            neighborVec.erase(it);
            return neighbor;
        }
    }

    return NULL;
}

EigrpNeighbor<IPv4Address> *EigrpIpv4NeighborTable::getFirstNeighborOnIf(int ifaceId)
{
    NeighborVector::iterator it;

    for (it = neighborVec.begin(); it != neighborVec.end(); ++it)
    {
        if ((*it)->getIfaceId() == ifaceId)
            return *it;
    }

    return NULL;
}

int EigrpIpv4NeighborTable::setAckOnIface(int ifaceId, uint32_t ackNum)
{
    NeighborVector::iterator it;
    int neighCnt = 0;

    for (it = neighborVec.begin(); it != neighborVec.end(); ++it)
    {
        if ((*it)->getIfaceId() == ifaceId)
        {
            neighCnt++;
            (*it)->setAck(ackNum);
        }
    }

    return neighCnt;
}

