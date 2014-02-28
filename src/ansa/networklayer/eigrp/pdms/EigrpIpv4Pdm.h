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

#ifndef __INET_EIGRPIPV4PDM_H_
#define __INET_EIGRPIPV4PDM_H_

#include <omnetpp.h>

#include "InterfaceTable.h"
#include "InterfaceEntry.h"
#include "IPv4Address.h"
#include "AnsaRoutingTable.h"
#include "NotificationBoard.h"

#include "EigrpInterfaceTable.h"
#include "EigrpNeighborTable.h"
#include "EigrpIpv4NeighborTable.h"
#include "EigrpRoute.h"
#include "EigrpIpv4TopologyTable.h"

#include "IEigrpModule.h"
#include "EigrpMessage_m.h"
#include "EigrpDisabledInterfaces.h"
#include "IEigrpPdm.h"
#include "EigrpDual.h"


class EigrpNetworkTable : cObject
{
  public:
    struct EigrpNetwork
    {
        int networkId;
        IPv4Address address;
        IPv4Address mask;

        EigrpNetwork(IPv4Address &address, IPv4Address &mask, int id)
                : networkId(id), address(address), mask(mask) {}
    };

  protected:
    std::vector<EigrpNetwork *> networkVec;
    int networkCnt;

  public:
    static const int UNSPEC_NETID = 0;

    EigrpNetworkTable() : networkCnt(1) { }
    virtual ~EigrpNetworkTable();

    int addNetwork(IPv4Address &address, IPv4Address &mask);
    EigrpNetwork *findNetworkById(int netId);
    std::vector<EigrpNetwork *> *getAllNetworks() { return &networkVec; }
    bool isAddressIncluded(IPv4Address& address, IPv4Address& mask);
    bool isInterfaceIncluded(const IPv4Address& ifAddress, const IPv4Address& ifMask, int *resultNetId);
};

/**
 * TODO - Generated class
 */
class EigrpIpv4Pdm : public cSimpleModule, public IEigrpModule, public IEigrpPdm, public INotifiable
{
  protected:
    typedef std::vector<ANSAIPv4Route *> RouteVector;

    const char* PDM_OUTPUT_GW;  /**< Output gateway to the RTP module */
    const IPv4Address EIGRP_IPV4_MULT; /**< Multicast address for EIGRP messages */
    EigrpKValues KVALUES_MAX;    /**< K-values (from K1 to K5) are set to max */
    const IPv4Address EIGRP_SELF_ADDR;  /**< Next hop address 0.0.0.0 (self address) */

    int asNum;                  /**< Autonomous system number */
    EigrpKValues kValues;       /**< K-values for calculation of metric */

    IInterfaceTable *ift;
    AnsaRoutingTable *rt;
    NotificationBoard *nb;      /**< */

    EigrpDual *eigrpDual;
    EigrpInterfaceTable *eigrpIft;                   /**< Table with enabled EIGRP interfaces */
    EigrpDisabledInterfaces *eigrpIftDisabled;       /**< Disabled EIGRP interfaces */
    EigrpIpv4NeighborTable *eigrpNt;                /**< Table with EIGRP neighbors */
    EigrpIpv4TopologyTable *eigrpTt;                /**< Topology table */
    EigrpNetworkTable *routingForNetworks;          /**< Networks included in EIGRP */
    RouteVector eigrpRtRoutes;                      /**< EIGRP routes in the global routing table */

    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    /**< Multi-stage initialization. */
    virtual int numInitStages() const { return 4; }

    EigrpTimer *createTimer(char timerKind, void *context);
    EigrpIpv4Hello *createHelloMsg(int destIfaceId, int holdInt, EigrpKValues kValues);
    EigrpIpv4Update *createUpdateMsg(int destIfaceId, IPv4Address destAddress);
    EigrpIpv4Query *createQueryMsg(int destIfaceId,IPv4Address destAddress);
    EigrpIpv4Reply *createReplyMsg(int destIfaceId,IPv4Address destAddress);
    void addCtrInfo(EigrpMessage *msg, int ifaceId, const IPv4Address &destAddress);
    void addMessageHeader(EigrpMessage *msg, int opcode);

    void processTimer(cMessage *msg);
    void processHelloMsg(cMessage *msg, IPv4Address& srcAddress, int ifaceId, EigrpNeighbor<IPv4Address> *neigh);
    void processUpdateMsg(cMessage *msg, IPv4Address& srcAddress, int ifaceId, EigrpNeighbor<IPv4Address> *neigh);
    void processQueryMsg(cMessage *msg, IPv4Address& srcAddress, int ifaceId, EigrpNeighbor<IPv4Address> *neigh);
    void processReplyMsg(cMessage *msg, IPv4Address& srcAddress, int ifaceId, EigrpNeighbor<IPv4Address> *neigh);
    EigrpRouteSource<IPv4Address> *processInterRoute(EigrpIpv4Internal& tlv, IPv4Address& srcAddr, int sourceNeighId,
            EigrpInterface *eigrpIface, bool &sourceNewResult);

    void processNewNeighbor(int ifaceId, IPv4Address &srcAddress, EigrpHello *helloMessage);
    int checkNeighborshipRules(int ifaceId, int neighAsNum, IPv4Address &neighAddr,
            const EigrpKValues &neighKValues);
    void removeNeighbor(EigrpNeighbor<IPv4Address> *neigh, IPv4Address& neighMask);

    void disableInterface(EigrpInterface *iface, IPv4Address& ifMask);
    void enableInterface(InterfaceEntry *iface, EigrpInterface *eigrpIface, IPv4Address& ifAddress, IPv4Address& ifMask);
    EigrpInterface *getInterfaceById(int ifaceId);
    EigrpInterface *addInterfaceToEigrp(int ifaceId, int networkId, bool enabled);

    /**< Sends EIGRP message to RTP module */
    //void sendEigrpMsg(EigrpMessage *msg, int userMsgIndex);
    void sendHelloMsg(int destIfaceId, int holdInt, EigrpKValues kValues);
    void sendGoodbyeMsg(int destIfaceId, int holdInt);
    void sendMsgToAllNeighbors(EigrpMessage *msg);
    void sendAllEigrpPaths(int destIface, IPv4Address& destAddress);

    void resetTimer(EigrpTimer *timer, int interval);

    uint32_t computeClassicMetric(EigrpMetricPar& par);
    EigrpMetricPar adjustMetricPar(EigrpMetricPar& metricPar, EigrpInterface *eigrpIface, InterfaceEntry *iface);
    EigrpMetricPar getMetricPar(EigrpInterface *eigrpIface, InterfaceEntry *iface);
    bool compareParamters(EigrpMetricPar& par1, EigrpMetricPar& par2);


    void setSource(EigrpRouteSource<IPv4Address> *src, bool isConnected, EigrpMetricPar& newMetricPar, EigrpMetricPar& neighMetricPar);
    /**< Returns next hop address. If next hop in message is 0.0.0.0, then next hop must be replaced by source IP of sender */
    IPv4Address getNextHopAddr(IPv4Address& nextHopAddr, IPv4Address& senderAddr)
    {
        return (nextHopAddr.isUnspecified()) ? senderAddr : nextHopAddr;
    }

    RouteVector::iterator findRouteInRT(EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route);

  public:
    EigrpIpv4Pdm();
    ~EigrpIpv4Pdm();

    virtual void receiveChangeNotification(int category, const cObject *details);

    // Interface IEigrpModule
    void addInterface(int ifaceId, int networkId, bool enabled)
    { addInterfaceToEigrp(ifaceId, networkId, enabled); }
    int addNetwork(IPv4Address address, IPv4Address mask);
    void setASNum(int asNum) { this->asNum = asNum; }
    void setKValues(const EigrpKValues& kValues) { this->kValues = kValues; }
    void setHelloInt(int interval, int ifaceId);
    void setHoldInt(int interval, int ifaceId);

    // Interface IEigrpPdm
    void addRouteToRT(EigrpRouteSource<IPv4Address> *successor);
    void removeRouteFromRT(EigrpRouteSource<IPv4Address> *successor);
    void updateRouteInRT(EigrpRouteSource<IPv4Address> *source);
    void sendUpdate(EigrpRoute<IPv4Address> *route, int destNeighbor, EigrpRouteSource<IPv4Address> *source = NULL);
    void sendQuery(EigrpRoute<IPv4Address> *route, int destNeighbor, EigrpRouteSource<IPv4Address> *source = NULL);
    void sendReply(EigrpRoute<IPv4Address> *route, int destNeighbor, EigrpRouteSource<IPv4Address> *source = NULL);
    void removeSourceFromTT(EigrpRouteSource<IPv4Address> *source);
    void addSourceToTT(EigrpRouteSource<IPv4Address> *source);
    int getNumPeers();
    uint32_t findRouteDMin(EigrpRoute<IPv4Address> *route);
    EigrpRouteSource<IPv4Address> * findSuccessor(EigrpRoute<IPv4Address> *route, uint32_t dmin);
    bool hasFeasibleSuccessor(EigrpRoute<IPv4Address> *route, uint32_t &resultDmin);
    bool checkFeasibleSuccessor(EigrpRoute<IPv4Address> *route);
    EigrpRouteSource<IPv4Address> *getFirstSuccessor(EigrpRoute<IPv4Address> *route);
};

/**
 * @brief Class gives access to the PimNeighborTable.
 */
class INET_API EigrpIpv4PdmAccess : public ModuleAccess<EigrpIpv4Pdm>
{
  public:
    EigrpIpv4PdmAccess() : ModuleAccess<EigrpIpv4Pdm>("EigrpIpv4Pdm") {}
};

#endif
