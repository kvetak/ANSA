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
#include "ModuleAccess.h"

#include "EigrpNeighborTable.h"

/**
 * Class represents EIGRP Neighbor Table.
 */
class EigrpIpv4NeighborTable : public cSimpleModule
{
  protected:
    typedef typename std::vector<EigrpNeighbor<IPv4Address> *> NeighborVector;

    NeighborVector neighborVec;    /**< Table with neighbors. */
    int neighborCounter;            /**< For unique ID of neighbor */
    int stubCount;                  /**< Number of stub neighbors for optimization */

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    /**
     * Stops Hold timer.
     */
    virtual void cancelHoldTimer(EigrpNeighbor<IPv4Address> *neigh);

  public:
    EigrpIpv4NeighborTable() { neighborCounter = 1; stubCount = 0; }
    virtual ~EigrpIpv4NeighborTable();
    /**
     * Adds neighbor to the table.
     */
    int addNeighbor(EigrpNeighbor<IPv4Address> *neighbor);
    /**
     * Finds neighbor by IP address. Returns null if neighbor is not found.
     */
    EigrpNeighbor<IPv4Address> *findNeighbor(const IPv4Address& ipAddress);
    /**
     * Finds neighbor by ID.
     */
    EigrpNeighbor<IPv4Address> *findNeighborById(int id);
    EigrpNeighbor<IPv4Address> * removeNeighbor(EigrpNeighbor<IPv4Address> *neighbor);
    /**
     * Returns first neighbor that resides on specified interface.
     */
    EigrpNeighbor<IPv4Address> *getFirstNeighborOnIf(int ifaceId);
    int getNumNeighbors() const { return neighborVec.size(); }
    EigrpNeighbor<IPv4Address> *getNeighbor(int k) const { return neighborVec[k]; }
    int setAckOnIface(int ifaceId, uint32_t ackNum);
    int getStubCount() const { return stubCount; }
    void incStubCount() { stubCount++; }
    void decStubCount() { stubCount--; }
};

class INET_API Eigrpv4NeighTableAccess : public ModuleAccess<EigrpIpv4NeighborTable>
{
    public:
    Eigrpv4NeighTableAccess() : ModuleAccess<EigrpIpv4NeighborTable>("eigrpIpv4NeighborTable") {}
};

#endif
