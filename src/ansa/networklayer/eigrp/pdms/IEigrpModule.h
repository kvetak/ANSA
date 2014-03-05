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

class IEigrpModule
{
  public:
    virtual ~IEigrpModule() {}
    virtual void addInterface(int ifaceId, int networkId, bool enabled) = 0;
    virtual EigrpNetwork *addNetwork(IPv4Address address, IPv4Address mask) = 0;
    virtual void setASNum(int asNum) = 0;
    virtual void setKValues(const EigrpKValues& kValues) = 0;

    virtual void setHelloInt(int interval, int ifaceId) = 0;
    virtual void setHoldInt(int interval, int ifaceId) = 0;
};

#endif /* IEIGRPMODULE_H_ */
