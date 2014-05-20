/*
 * IEigrpPdm.h
 *
 *  Created on: Feb 17, 2014
 *      Author: honza
 */

#ifndef IEIGRPPDM_H_
#define IEIGRPPDM_H_

#include "IPv4Address.h"

#include "EigrpRoute.h"

/**
 * Interface for DUAL automaton.
 */
class IEigrpPdm
{
  public:
    static const int UNSPEC_RECEIVER = 0;  /**< Unspecified address of receiver - all neighbors */
    static const int STUB_RECEIVER = -1;  /**< All stub neighbors */

    static const int CONNECTED_ROUTE = 0;
    static const int UNSPEC_SENDER = 0;  /**< Unspecified address of sender - input event source */
    static const bool RT_UNREACHABLE = true;

    /**
     * Function finds successors to the destination network.
     * @param route destination network
     * @param dmin minimal distance to the destination
     * @param rtableChanged return parameter, true if occurs change in the routing table
     * @param removeUnreach invalidate unreachable sources of the route.
     * @return pointer to successor or NULL.
     */
    virtual EigrpRouteSource<IPv4Address> *updateRoute(EigrpRoute<IPv4Address> *route, uint64_t dmin, bool *rtableChanged, bool removeUnreach = false) = 0;
    /**
     * Sends update message to specified neighbor.
     * @param destNeighbor ID of destination neighbor. If parameter is  set to 0 then sends message to all neighbors.
     * @param route route network
     * @param source route
     * @param forcePoisonRev force poison reverse rule instead of split horizon
     * @param reason text description for user.
     */
    virtual void sendUpdate(int destNeighbor, EigrpRoute<IPv4Address> *route, EigrpRouteSource<IPv4Address> *source, bool forcePoisonRev, const char *reason) = 0;
    /**
     * Sends query message to specified neighbor.
     * @param destNeighbor ID of destination neighbor. If parameter is  set to 0 then sends message to all neighbors.
     * @param route route network
     * @param source route
     * @param forcePoisonRev apply Poison Reverse instead of Split Horizon to the route.
     */
    virtual void sendQuery(int destNeighbor, EigrpRoute<IPv4Address> *route, EigrpRouteSource<IPv4Address> *source, bool forcePoisonRev = false) = 0;
    /**
     * Sends reply message to specified neighbor.
     * @param route route network
     * @param destNeighbor ID of destination neighbor.
     * @param source route
     * @param forcePoisonRev apply Poison Reverse rule to the route
     * @param isUnreachable route in message will have inf metric (regardless of Poisson Reverse)
     */
    virtual void sendReply(EigrpRoute<IPv4Address> *route, int destNeighbor, EigrpRouteSource<IPv4Address> *source, bool forcePoisonRev = false, bool isUnreachable = false) = 0;
    /**
     * Returns minimal distance to destination network.
     */
    virtual uint64_t findRouteDMin(EigrpRoute<IPv4Address> *route) = 0;
    /**
     * Determine whether there are Feasibles Successors for specified route.
     * @param resultDmin return parameter with minimal distance to the destination.
     */
    virtual bool hasFeasibleSuccessor(EigrpRoute<IPv4Address> *route, uint64_t &resultDmin) = 0;
    /**
     * Sets Reply Status Table for specified network.
     * @param route route network
     * @param forcePoisonRev use Poison Reverse instead of Split Horizon
     * @param neighCount number of all neighbors.
     * @param stubCount number of stub neighbors.
     */
    virtual bool setReplyStatusTable(EigrpRoute<IPv4Address> *route, EigrpRouteSource<IPv4Address> *source, bool forcePoisonRev, int *neighCount, int *stubCount) = 0;
    /**
     * Returns tru if there are recipients for update message, else false.
     */
    virtual bool hasNeighborForUpdate(EigrpRouteSource<IPv4Address> *source) = 0;
    /**
     * Returns best successor for specified network.
     */
    virtual EigrpRouteSource<IPv4Address> *getBestSuccessor(EigrpRoute<IPv4Address> *route) = 0;
    /**
     * After receiving Ack from neighbor with neighId will be route removed from TT.
     * @param neighId ID of neighbor
     * @param src route
     */
    virtual void setDelayedRemove(int neighId, EigrpRouteSource<IPv4Address> *src) = 0;
    /**
     * Sends update message to all stub neighbors.
     */
    virtual void sendUpdateToStubs(EigrpRouteSource<IPv4Address> *succ ,EigrpRouteSource<IPv4Address> *oldSucc, EigrpRoute<IPv4Address> *route) = 0;
};


#endif /* IEIGRPPDM_H_ */
