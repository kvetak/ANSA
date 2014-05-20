/*
 * IEigrpModule.h
 *
 *  Created on: Feb 7, 2014
 *      Author: honza
 */

#ifndef IEIGRPMODULE_H_
#define IEIGRPMODULE_H_

#include "EigrpMessage_m.h"
#include "EigrpNetworkTable.h"

/**
 * Interface for EIGRP configuration.
 */
class IEigrpModule
{
  public:
    virtual ~IEigrpModule() {}
    //-- Process configuration
    /**
     * Adds interface to EIGRP.
     * @param interfaceId ID of interface
     * @param networkID ID of network in EigrpNetworkTable
     */
    virtual void addInterface(int interfaceId, int networkId, bool enabled) = 0;
    /**
     * Adds new network to EigrpNetworkTable for routing.
     */
    virtual EigrpNetwork *addNetwork(IPv4Address address, IPv4Address mask) = 0;
    virtual void setASNum(int asNum) = 0;
    virtual void setKValues(const EigrpKValues& kValues) = 0;
    virtual void setMaximumPath(int maximumPath) = 0;
    virtual void setVariance(int variance) = 0;
    virtual void setStub(const EigrpStub& stub) = 0;

    // Interface configuration
    virtual void setHelloInt(int interval, int interfaceId) = 0;
    virtual void setHoldInt(int interval, int interfaceId) = 0;
    virtual void setSplitHorizon(bool shEnabled, int interfaceId) = 0;
    virtual void setPassive(bool passive, int ifaceId) = 0;
};

#endif /* IEIGRPMODULE_H_ */
