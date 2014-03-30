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
    static const int UNSPEC_SENDER = 0;  /**< Unspecified address of input event source */
    static const bool RT_UNREACHABLE = true;

    virtual EigrpRouteSource<IPv4Address> *updateRoute(EigrpRoute<IPv4Address> *route, uint32_t dmin) = 0;
    virtual void sendUpdate(int destNeighbor, EigrpRoute<IPv4Address> *route, EigrpRouteSource<IPv4Address> *source) = 0;
    virtual void sendQuery(int destNeighbor, EigrpRoute<IPv4Address> *route, EigrpRouteSource<IPv4Address> *source, bool updateFromS = false) = 0;
    virtual void sendReply(EigrpRoute<IPv4Address> *route, int destNeighbor, EigrpRouteSource<IPv4Address> *source, bool isUnreachable = false) = 0;
    virtual void removeSourceFromTT(EigrpRouteSource<IPv4Address> *source) = 0;
    virtual void addSourceToTT(EigrpRouteSource<IPv4Address> *source) = 0;
    virtual uint32_t findRouteDMin(EigrpRoute<IPv4Address> *route) = 0;
    virtual bool hasFeasibleSuccessor(EigrpRoute<IPv4Address> *route, uint32_t &resultDmin) = 0;
    virtual int setReplyStatusTable(EigrpRoute<IPv4Address> *route, EigrpRouteSource<IPv4Address> *source, bool updateFromS = false) = 0;
    virtual bool hasNeighbor(EigrpRouteSource<IPv4Address> *source) = 0;
    virtual EigrpRouteSource<IPv4Address> *getFirstSuccessor(EigrpRoute<IPv4Address> *route) = 0;
};


#endif /* IEIGRPPDM_H_ */
