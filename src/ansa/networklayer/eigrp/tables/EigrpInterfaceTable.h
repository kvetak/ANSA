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

#ifndef __INET_EIGRPINTERFACETABLE_H_
#define __INET_EIGRPINTERFACETABLE_H_

#include <omnetpp.h>

#include "IPv4Address.h"
#include "EigrpTimer_m.h"

/**
 * TODO nastavovat BW, DLY dle typu linky, Reliability pocitat
 * na zacatku simulace, Load prubezne po 5 min (melo by byt konfigurovatelne)
 */
class EigrpInterface: public cObject
{
  protected:
    int interfaceId;        /**< ID of interface */
    int networkId;          /**< ID of network in RoutingOfNetworks table */
    int helloInt;           /**< Hello interval in seconds (<1-65535>) */
    int holdInt;            /**< Router's hold interval in seconds (<1-65535>) */
    EigrpTimer *hellot;     /**< pointer to hello timer */
    bool enabled;           /**< true, if EIGRP is enabled on interface */

    unsigned int bandwidth; /**< Bandwidth in Kbps (<1-10 000 000>) */
    unsigned int delay;     /**< Delay in ms (<1-16 777 215>) */
    unsigned int reliability; /**< Reliability in percent (<1-255>) */
    unsigned int load;      /**< Load in percent (<1-255>) */

  public:
    EigrpInterface(int interfaceId, int networkId, bool enabled);
    ~EigrpInterface();

    bool operator==(const EigrpInterface& iface) const
    {
        return interfaceId == iface.getInterfaceId();
    }

    /**< Set identifier of interface. */
    void setInterfaceId(int id) { interfaceId = id; }
    /**< Set pointer to hello timer. */
    void setHelloTimerPtr(EigrpTimer *timer) { this->hellot = timer; }
    void setHelloInt(int helloInt) { this->helloInt = helloInt; }
    void setHoldInt(int holdInt) { this->holdInt = holdInt; }
    void setEnabling(bool enabled) { this->enabled = enabled; }
    void setNetworkId(int netId) { this->networkId = netId; }

    void setBandwidth(int bw) { this->bandwidth = bw; }
    void setDelay(int dly) { this->delay = dly; }
    void setReliability(int rel) { this->reliability = rel; }
    void setLoad(int load) { this->load = load; }

    /**< Return identifier of interface. */
    int getInterfaceId() const { return interfaceId; }
    /**< Return pointer to hello timer. */
    EigrpTimer *getHelloTimer() const { return this->hellot; }
    int getHelloInt() const { return this->helloInt; }
    int getHoldInt() const { return this->holdInt; }
    bool isEnabled() const { return this->enabled; }
    int getNetworkId() const { return this->networkId; }

    unsigned int getBandwidth() const { return bandwidth; }
    unsigned int getDelay() const { return delay; }
    unsigned int getReliability() const { return reliability; }
    unsigned int getLoad() const { return load; }
};

/**
 * TODO - Generated class
 */
class EigrpInterfaceTable : public cSimpleModule
{
  protected:
    typedef typename std::vector<EigrpInterface *> InterfaceVector;

    InterfaceVector eigrpInterfaces;

    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual int numInitStages() const { return 4; }

    void cancelHelloTimer(EigrpInterface *iface);

  public:
    ~EigrpInterfaceTable();

    /**< Adds interface to table. */
    void addInterface(EigrpInterface *interface);
    /**< Removes interface from table */
    EigrpInterface *removeInterface(EigrpInterface *iface);

    /**< Gets interface from table by interface ID */
    EigrpInterface *findInterfaceById(int ifaceId);
    /**< Gets interface from table by interface name */
    //EigrpInterface *findInterfaceByName(const char *ifaceName);
};

#endif
