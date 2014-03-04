/**
 * @file EigrpNeighbor.h
 * @brief File contains interface for neighbor table.
 * @date 7.2.2014
 * @author Jan Bloudicek
 * @details
 */

#ifndef EIGRPNEIGHBORTABLE_H_
#define EIGRPNEIGHBORTABLE_H_

#include <omnetpp.h>

#include "EigrpTimer_m.h"

template<typename IPAddress>
class EigrpNeighbor : public cObject
{
  protected:
    int ifaceId;        /**< ID of interface that is connected to the neighbor. */
    int neighborId;     /**< ID of neighbor */
    IPAddress ipAddress;/**< IP address of neighbor. */
    bool isUp;          /**< State of neighbor. Possible states are pending or up. */
    int holdInt;        /**< Neighbor's Hold interval used to set Hold timer */
    EigrpTimer *holdt;  /**< pointer to Hold timer */

  public:
    virtual ~EigrpNeighbor() { delete holdt; holdt = NULL; }
    EigrpNeighbor(int ifaceId, IPAddress ipAddress) :
        ifaceId(ifaceId), ipAddress(ipAddress), isUp(false), holdt(NULL) {}

    void setState(bool stateUp) { this->isUp = stateUp; }
    /**< Sets neighbor's Hold interval value */
    void setHoldInt(int holdInt) { this->holdInt = holdInt; }
    /**< Sets timer for a neighbor */
    void setHoldTimer(EigrpTimer *holdt) { ASSERT(this->holdt == NULL); this->holdt = holdt; }
    void setNeighborId(int id) { this->neighborId = id; }

    IPAddress getIPAddress() const { return this->ipAddress; }
    int getIfaceId() const { return this->ifaceId; }
    bool getState() const { return this->isUp; }
    int getHoldInt() const { return this->holdInt; }
    EigrpTimer *getHoldTimer() const { return this->holdt; }
    int getNeighborId() const { return neighborId; }
};

/**
 * @brief The interface is designed to support IPv4 and IPv6. Both
 * protocols must have separate neighbor tables.
 * @details
template<typename IPAddress>
class EigrpNeighborTable
{
  protected:
    std::vector<EigrpNeighbor<IPAddress> *> neighborVec;    /**< Table with neighbors.

  public:
    virtual ~EigrpNeighborTable() {}

    EigrpNeighbor<IPAddress> *findNeighbor(IPAddress ipAddress, int iface);
    void addNeighbor(EigrpNeighbor<IPAddress> *neighbor);
    EigrpNeighbor<IPAddress> * removeNeighbor(EigrpNeighbor<IPAddress> *neighbor);
};

// Implementation (template must be defined in header or...)

template<typename IPAddress>
EigrpNeighbor<IPAddress> *EigrpNeighborTable<IPAddress>::findNeighbor(
        IPAddress ipAddress, int iface)
{
    typename std::vector<EigrpNeighbor<IPAddress> *>::iterator it;
    EigrpNeighbor<IPAddress> * neigh;

    for (it = neighborVec.begin(); it != neighborVec.end(); it++)
    {
        neigh = *it;
        if (neigh->getIPAddress() == ipAddress &&
                neigh->getIfaceId() == iface)
        {
            return neigh;
        }
    }

    return NULL;
}

template<typename IPAddress>
void EigrpNeighborTable<IPAddress>::addNeighbor(EigrpNeighbor<IPAddress> *neighbor)
{
    this->neighborVec.push_back(neighbor);
}

/**
 * Removes neighbor form the table, but the record still exists.
template<typename IPAddress>
EigrpNeighbor<IPAddress> *EigrpNeighborTable<IPAddress>::removeNeighbor(EigrpNeighbor<IPAddress> *neighbor)
{
    typename std::vector<EigrpNeighbor<IPAddress> *>::iterator it;

    for (it = neighborVec.begin(); it != neighborVec.end(); ++it)
    {
        if (*it == neighbor)
        {
            neighborVec.erase(it);
            return neighbor;
        }
    }

    return NULL;
}*/


#endif /* EIGRPNEIGHBORTABLE_H_ */
