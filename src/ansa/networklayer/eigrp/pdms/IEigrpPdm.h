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

class IEigrpPdm
{
  public:
    static const int UNSPEC_RECEIVER = 0;  /**< Unspecified address of receiver - all neighbors */

    virtual void addRouteToRT(EigrpRouteSource<IPv4Address> *successor) = 0;
    virtual void removeRouteFromRT(EigrpRouteSource<IPv4Address> *successor) = 0;
    /**< Updates metric of the route */
    virtual void updateRouteInRT(EigrpRouteSource<IPv4Address> *source) = 0;
    virtual void sendUpdate(EigrpRoute<IPv4Address> *route, int destNeighbor, EigrpRouteSource<IPv4Address> *successor = NULL) = 0;
    virtual void sendQuery(EigrpRoute<IPv4Address> *route, int destNeighbor, EigrpRouteSource<IPv4Address> *source = NULL) = 0;
    virtual void sendReply(EigrpRoute<IPv4Address> *route, int destNeighbor, EigrpRouteSource<IPv4Address> *source = NULL) = 0;
    virtual void removeSourceFromTT(EigrpRouteSource<IPv4Address> *source) = 0;
    virtual void addSourceToTT(EigrpRouteSource<IPv4Address> *source) = 0;
    virtual int getNumPeers() = 0;
    virtual uint32_t findRouteDMin(EigrpRoute<IPv4Address> *route) = 0;
    virtual EigrpRouteSource<IPv4Address> * findSuccessor(EigrpRoute<IPv4Address> *route, uint32_t dmin) = 0;
    virtual bool hasFeasibleSuccessor(EigrpRoute<IPv4Address> *route, uint32_t &resultDmin) = 0;
    virtual bool checkFeasibleSuccessor(EigrpRoute<IPv4Address> *route) = 0;
    virtual EigrpRouteSource<IPv4Address> *getFirstSuccessor(EigrpRoute<IPv4Address> *route) = 0;
};


#endif /* IEIGRPPDM_H_ */
