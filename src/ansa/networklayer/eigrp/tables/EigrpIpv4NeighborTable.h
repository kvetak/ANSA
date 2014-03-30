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

#ifndef __INET_EIGRPIPV4NEIGHBORTABLE_H_
#define __INET_EIGRPIPV4NEIGHBORTABLE_H_

#include <omnetpp.h>

#include "IPv4Address.h"

#include "EigrpNeighborTable.h"

/**
 * TODO - Generated class
 */
class EigrpIpv4NeighborTable : public cSimpleModule
{
  protected:
    typedef typename std::vector<EigrpNeighbor<IPv4Address> *> NeighborVector;

    NeighborVector neighborVec;    /**< Table with neighbors. */
    int neighborCounter;        /**< For unique ID of neighbor */

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    virtual void cancelHoldTimer(EigrpNeighbor<IPv4Address> *neigh);

  public:
    EigrpIpv4NeighborTable() { neighborCounter = 1; }
    virtual ~EigrpIpv4NeighborTable();
    int addNeighbor(EigrpNeighbor<IPv4Address> *neighbor);
    /**< Find neighbor by IP address and interface ID. If the neighbor
     * is not found, it returns NULL. */
    EigrpNeighbor<IPv4Address> *findNeighbor(const IPv4Address& ipAddress);
    EigrpNeighbor<IPv4Address> *findNeighborById(int id);
    EigrpNeighbor<IPv4Address> * removeNeighbor(EigrpNeighbor<IPv4Address> *neighbor);
    EigrpNeighbor<IPv4Address> *getFirstNeighborOnIf(int ifaceId);
    int getNumNeighbors() const { return neighborVec.size(); }
    EigrpNeighbor<IPv4Address> *getNeighbor(int k) const { return neighborVec[k]; }
    int setAckOnIface(int ifaceId, uint32_t ackNum);
};

#endif
