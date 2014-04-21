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
    const char *ifaceName;
    int neighborId;     /**< ID of neighbor */
    // TODO - tady se uloží RID směrovače (ještě ověřit na novém IOSu), které je vždy Ipv4 adresa (i při Ipv6 směrování)
    IPAddress ipAddress;/**< IP address of neighbor. */

    bool isUp;          /**< State of neighbor. Possible states are pending or up. */
    int holdInt;        /**< Neighbor's Hold interval used to set Hold timer */
    uint32_t seqNumber; /**< Sequence number received from neighbor */
    bool routesForDeletion; /**< After receiving Ack from the neighbor will be removed unreachable routes from TT */
    uint32_t waitForAck;        /**< Waiting for ack number from neighbor (for reliable transport) */
    EigrpTimer *holdt;  /**< Pointer to Hold timer */

  public:
    static const int UNSPEC_ID = 0;

    virtual ~EigrpNeighbor() { delete holdt; holdt = NULL; }
    EigrpNeighbor(int ifaceId, const char *ifaceNname, IPAddress ipAddress) :
        ifaceId(ifaceId), ifaceName(ifaceNname), neighborId(UNSPEC_ID), ipAddress(ipAddress), isUp(false), holdt(NULL)
    { seqNumber = 0; holdInt = 0; routesForDeletion = false; waitForAck = 0; }

    void setStateUp(bool stateUp) { this->isUp = stateUp; }
    /**< Sets neighbor's Hold interval value */
    void setHoldInt(int holdInt) { this->holdInt = holdInt; }
    /**< Sets timer for a neighbor */
    void setHoldTimer(EigrpTimer *holdt) { ASSERT(this->holdt == NULL); this->holdt = holdt; }
    void setNeighborId(int neighborId) {  this->neighborId = neighborId; }
    void setSeqNumber(int seqNumber) { this->seqNumber = seqNumber; }
    void setAck(uint32_t waitForAck) { this->waitForAck = waitForAck; }
    void setRoutesForDeletion(bool routesForDeletion) { this->routesForDeletion = routesForDeletion; }

    IPAddress getIPAddress() const { return this->ipAddress; }
    int getIfaceId() const { return this->ifaceId; }
    bool isStateUp() const { return this->isUp; }
    int getHoldInt() const { return this->holdInt; }
    EigrpTimer *getHoldTimer() const { return this->holdt; }
    int getNeighborId() const { return neighborId; }
    int getSeqNumber() const { return seqNumber; }
    uint32_t getAck() const { return waitForAck; }
    bool getRoutesForDeletion() const { return this->routesForDeletion; }

    const char *getIfaceName() const { return ifaceName; }
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
