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
/**
* @file EigrpIpv6Pdm.h
* @author Vit Rek, xrekvi00@stud.fit.vutbr.cz
* @date 6. 11. 2014
* @brief EIGRP IPv6 Protocol Dependent Module header file
* @detail Main module, it mediates control exchange between DUAL, routing table and
topology table.
*/

#ifndef EIGRPIPV6PDM_H_
#define EIGRPIPV6PDM_H_

#include <omnetpp.h>

//#include "InterfaceTable.h"
//#include "InterfaceEntry.h"
//#include "IPv6Address.h"
//#include "AnsaRoutingTable.h"
//#include "NotificationBoard.h"

#include "ansa/routing/eigrp/tables/EigrpInterfaceTable.h"
#include "ansa/routing/eigrp/tables/EigrpNeighborTable.h"
#include "ansa/routing/eigrp/tables/EigrpRoute.h"
#include "ansa/routing/eigrp/tables/EigrpTopologyTable.h"

#include "ansa/routing/eigrp/pdms/IEigrpModule.h"
#include "ansa/routing/eigrp/pdms/IEigrpPdm.h"

#include "ansa/routing/eigrp/messages/EigrpMessage_m.h"
#include "ansa/routing/eigrp/tables/EigrpDisabledInterfaces.h"
#include "ansa/routing/eigrp/EigrpDual.h"
#include "ansa/routing/eigrp/tables/EigrpNetworkTable.h"
#include "ansa/routing/eigrp/pdms/EigrpMetricHelper.h"
#include "ansa/routing/eigrp/messages/EigrpMsgReq.h"

#include "inet/networklayer/ipv6/IPv6RoutingTable.h"
#include "ansa/networklayer/common/ANSA_InterfaceEntry.h"
#include "inet/networklayer/contract/ipv6/IPv6ControlInfo.h"

//#include "EigrpDualStack.h"
//#include "ANSARoutingTable6.h"
//#include "IPv6InterfaceData.h"
namespace inet {
/**
 * Class represents EIGRP Protocol Dependent Module for IPv6. It contains IPv6 specific things.
 */
class EigrpIpv6Pdm : public cSimpleModule, public IEigrpModule<IPv6Address>, public IEigrpPdm<IPv6Address>, protected cListener
{
  protected:
    struct IPv6netPrefix {
       int ifaceId;
       IPv6Address network;
       short int prefixLength;
    };
    typedef std::vector<IPv6Route *> RouteVector;
    typedef std::vector<EigrpMsgReq *> RequestVector;
    typedef std::vector<IPv6netPrefix> PrefixVector;

    const char* SPLITTER_OUTGW;         /**< Output gateway to the EIGRP Splitter module */
    const char* RTP_OUTGW;              /**< Output gateway to the RTP module */
    const IPv6Address EIGRP_IPV6_MULT; /**< Multicast address for EIGRP messages */
    EigrpKValues KVALUES_MAX;           /**< K-values (from K1 to K5) are set to max */
    const IPv6Address EIGRP_SELF_ADDR;  /**< Next hop address 0.0.0.0 (self address) */

    int asNum;                  /**< Autonomous system number */
    EigrpKValues kValues;       /**< K-values for calculation of metric */
    int maximumPath;            /**< Maximum number of parallel routes that EIGRP will support */
    int variance;               /**< Parameter for unequal cost load balancing */
    unsigned int adminDistInt; /**< Administrative distance */
    bool useClassicMetric;      /**< Use classic metric computation or wide metric computation */
    int ribScale;               /**< Scaling factor for Wide metric */
    bool eigrpStubEnabled;      /**< True when EIGRP stub is on */
    EigrpStub eigrpStub;        /**< EIGRP stub configuration */

    IInterfaceTable *ift;
    //ANSARoutingTable6 *rt;
    IPv6RoutingTable* rt;
    //NotificationBoard *nb;



    EigrpDual<IPv6Address> *eigrpDual;
    EigrpMetricHelper *eigrpMetric;
    EigrpInterfaceTable *eigrpIft;                   /**< Table with enabled EIGRP interfaces */
    EigrpDisabledInterfaces *eigrpIftDisabled;       /**< Disabled EIGRP interfaces */
    EigrpNeighborTable<IPv6Address> *eigrpNt;                /**< Table with EIGRP neighbors */
    EigrpTopologyTable<IPv6Address> *eigrpTt;                /**< Topology table */
    EigrpNetworkTable<IPv6Address> *routingForNetworks;          /**< Networks included in EIGRP */
    RequestVector reqQueue;                         /**< Requests for sending EIGRP messages from DUAL */
    PrefixVector netPrefixes;

    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    /**< Multi-stage initialization. */
    virtual int numInitStages() const override { return 4; }
    //virtual void receiveChangeNotification(int category, const cObject *details);
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG) override;

    void printSentMsg(int routeCnt, IPv6Address& destAddress, EigrpMsgReq *msgReq);
    void printRecvMsg(EigrpMessage *msg, IPv6Address& addr, int ifaceId);


    //-- TIMERS
    /**
     * Creates timer of specified type.
     */
    EigrpTimer *createTimer(char timerKind, void *context);
    /**
     * Sets specified timer to given interval.
     */
    void resetTimer(EigrpTimer *timer, int interval) { cancelEvent(timer); scheduleAt(simTime() + interval /*- uniform(0,0.4)*/, timer); }
    /**
     * Schedule hello timer to the specified interval.
     */
    void startHelloTimer(EigrpInterface *eigrpIface, simtime_t interval);


    //-- METHODS FOR CREATING MESSAGES  //TODO
    EigrpIpv6Hello *createHelloMsg(int holdInt, EigrpKValues kValues, IPv6Address& destAddress, EigrpMsgReq *msgReq);
    EigrpIpv6Ack *createAckMsg(IPv6Address& destAddress, EigrpMsgReq *msgReq);
    EigrpIpv6Update *createUpdateMsg(const IPv6Address& destAddress, EigrpMsgReq *msgReq);
    EigrpIpv6Query *createQueryMsg(IPv6Address& destAddress, EigrpMsgReq *msgReq);
    EigrpIpv6Reply *createReplyMsg(IPv6Address& destAddress, EigrpMsgReq *msgReq);
    /**
     * Adds ControlInfo for network layer module.
     */
    void addCtrInfo(EigrpMessage *msg, int ifaceId, const IPv6Address &destAddress);
    void addMessageHeader(EigrpMessage *msg, int opcode, EigrpMsgReq *msgReq);
    void createRouteTlv(EigrpMpIpv6Internal *routeTlv, EigrpRoute<IPv6Address> *route, bool unreachable = false);
    /**
     * Add routes from request to the message.
     */
    void addRoutesToMsg(EigrpIpv6Message *msg, const EigrpMsgReq *msgReq);
    void setRouteTlvMetric(EigrpWideMetricPar *msgMetric, EigrpWideMetricPar *rtMetric);
    /**
     * Creates request for sending of EIGRP message for RTP.
     */
    EigrpMsgReq *createMsgReq(HeaderOpcode msgType, int destNeighbor, int destIface);


    //-- PROCESSING MESSAGES
    void processTimer(cMessage *msg);
    /**
     * Process message from network layer.
     */
    void processMsgFromNetwork(cMessage *msg);
    /**
     * Process message request from RTP.
     */
    void processMsgFromRtp(cMessage *msg);
    void processAckMsg(cMessage *msg, IPv6Address& srcAddress, int ifaceId, EigrpNeighbor<IPv6Address> *neigh);
    void processHelloMsg(cMessage *msg, IPv6Address& srcAddress, int ifaceId, EigrpNeighbor<IPv6Address> *neigh);
    void processUpdateMsg(cMessage *msg, IPv6Address& srcAddress, int ifaceId, EigrpNeighbor<IPv6Address> *neigh);
    void processQueryMsg(cMessage *msg, IPv6Address& srcAddress, int ifaceId, EigrpNeighbor<IPv6Address> *neigh);
    void processReplyMsg(cMessage *msg, IPv6Address& srcAddress, int ifaceId, EigrpNeighbor<IPv6Address> *neigh);
    /**
     * Process route TLV.
     */
    EigrpRouteSource<IPv6Address> *processInterRoute(EigrpMpIpv6Internal& tlv, IPv6Address& nextHop, int sourceNeighId, EigrpInterface *eigrpIface, bool *notifyDual, bool *isSourceNew);


    //-- NEIGHBORSHIP MANAGEMENT
    /**
     * Creates and sends message with all routes from routing table to specified neighbor.
     */
    void sendAllEigrpPaths(EigrpInterface *eigrpIface, EigrpNeighbor<IPv6Address> *neigh);
    /**
     * Creates relationship with neighbor.
     * @param srcAddress address of the neighbor
     * @param ifaceId ID of interface where the neighbor is connected
     */
    void processNewNeighbor(int ifaceId, IPv6Address &srcAddress, EigrpIpv6Hello *helloMessage);
    /**
     * Checks neighborship rules.
     * @param ifaceId ID of interface where the neighbor is connected.
     * @return returns code from enumeration eigrp::UserMsgCodes.
     */
    int checkNeighborshipRules(int ifaceId, int neighAsNum, IPv6Address &neighAddr,
            const EigrpKValues &neighKValues);
    /**
     * Create record in the neighbor table and start hold timer.
     */
    EigrpNeighbor<IPv6Address> *createNeighbor(EigrpInterface *eigrpIface, IPv6Address& address, uint16_t holdInt);
    /**
     * Removes neighbor from neighbor table and delete it. Notifies DUAL about event.
     */
    void removeNeighbor(EigrpNeighbor<IPv6Address> *neigh);


    //-- INTERFACE MANAGEMENT
    /**
     * Remove interface from EIGRP interface table. Removes all neighbors on the interface.
     */
    //void disableInterface(InterfaceEntry *iface, EigrpInterface *eigrpIface, IPv6Address& ifAddress, IPv6Address& ifMask);
    void disableInterface(ANSA_InterfaceEntry*iface, EigrpInterface *eigrpIface);
    /**
     * Add interface to the EIGRP interface table and notifies DUAL.
     */
    //void enableInterface(EigrpInterface *eigrpIface, IPv6Address& ifAddress, IPv6Address& ifMask, int networkId);
    void enableInterface(EigrpInterface *eigrpIface);
    /**
     * Returns EIGRP interface (enabled or disabled) or NULL.
     */
    EigrpInterface *getInterfaceById(int ifaceId);
    /**
     * Creates interface and inserts it to the table.
     */
    //EigrpInterface *addInterfaceToEigrp(int ifaceId, int networkId, bool enabled);


    EigrpInterface *addInterfaceToEigrp(int ifaceId, bool enabled);

    //-- PROCESSING EVENTS FROM NOTIFICATION BOARD
    void processIfaceStateChange(ANSA_InterfaceEntry* iface);
    void processIfaceConfigChange(EigrpInterface *eigrpIface);
    void processRTRouteDel(const cObject *details);

    /**
     * Returns next hop address. If next hop in message is 0.0.0.0, then next hop must be
     * replaced by IP address of sender.
     */
    IPv6Address getNextHopAddr(IPv6Address& nextHopAddr, IPv6Address& senderAddr)
        { return (nextHopAddr.isUnspecified()) ? senderAddr : nextHopAddr; }
    /**
     * Returns IP address for sending EIGRP message.
     */
    bool getDestIpAddress(int destNeigh, IPv6Address *resultAddress);


    //-- ROUTING TABLE MANAGEMENT
    bool removeRouteFromRT(EigrpRouteSource<IPv6Address> *successor, IRoute::SourceType *removedRtSrc);
    IPv6Route *createRTRoute(EigrpRouteSource<IPv6Address> *successor);
    /**
     * Updates existing route in the routing table or creates new one.
     */
    bool installRouteToRT(EigrpRoute<IPv6Address> *route, EigrpRouteSource<IPv6Address> *source, uint64_t dmin, IPv6Route *rtEntry);
    /**
     * Returns true, if routing table does not contain route with given address, mask and
     * smaller administrative distance.
     */
    bool isRTSafeForAdd(EigrpRoute<IPv6Address> *route, unsigned int eigrpAd);
    /**
     * Changes metric of route in routing table. For wide metric uses scale.
     */
    void setRTRouteMetric(IPv6Route *route, uint64_t metric) { if (!useClassicMetric) { metric = metric / ribScale; } route->setMetric(metric); }
    /**
     * Removes route from routing table and changes old successor's record in topology table.
     */
    bool removeOldSuccessor(EigrpRouteSource<IPv6Address> *source, EigrpRoute<IPv6Address> *route);


    //-- METHODS FOR MESSAGE REQUESTS
    /**
     * Records request to send message to all neighbors.
     */
    void msgToAllIfaces(int destination, HeaderOpcode msgType, EigrpRouteSource<IPv6Address> *source, bool forcePoisonRev, bool forceUnreachable);
    /**
     * Creates request for sending message on specified interface.
     */
    void msgToIface(HeaderOpcode msgType, EigrpRouteSource<IPv6Address> *source, EigrpInterface *eigrpIface,  bool forcePoisonRev = false, bool forceUnreachable = false);
    /**
     * Sends all message requests to RTP.
     * */
    void flushMsgRequests();
    /**
     * Insert route into the queue with requests.
     */
    EigrpMsgReq *pushMsgRouteToQueue(HeaderOpcode msgType, int ifaceId, int neighId, const EigrpMsgRoute& msgRt);


    //-- RULES APPLICATION
    /**
     * @return true, if Split Horizon rule is met for the route, otherwise false.
     */
    bool applySplitHorizon(EigrpInterface *destInterface, EigrpRouteSource<IPv6Address> *source, EigrpRoute<IPv6Address> *route);
    /**
     * Apply stub configuration to the route in outgoing Update message.
     * @return true, if stub setting limits sending of the route, otherwise false
     */
    bool applyStubToUpdate(EigrpRouteSource<IPv6Address> *src);
    /**
     * Apply stub configuration to the route in outgoing Query message.
     * @return true, if stub setting limits sending of the route, otherwise false
     */
    bool applyStubToQuery(EigrpInterface *eigrpIface, int numOfNeigh);

    IPv6Route *findRoute(const IPv6Address& prefix, int prefixLength);
    IPv6Route *findRoute(const IPv6Address& prefix, int prefixLength, const IPv6Address& nexthop);


  public:
    EigrpIpv6Pdm();
    ~EigrpIpv6Pdm();

    //-- INTERFACE IEigrpModule
    void addInterface(int ifaceId, int networkId, bool enabled) override { /* useful only for IPv4 */ }
    void addInterface(int ifaceId, bool enabled) override { addInterfaceToEigrp(ifaceId, enabled); }
    EigrpNetwork<IPv6Address> *addNetwork(IPv6Address address, IPv6Address mask) override;
    void setASNum(int asNum) override { this->asNum = asNum; }
    int getASNum() override {return this->asNum; }
    void setKValues(const EigrpKValues& kValues) override { this->kValues = kValues; }
    void setMaximumPath(int maximumPath) override { this->maximumPath = maximumPath; }
    void setVariance(int variance) override { this->variance = variance; }
    void setHelloInt(int interval, int ifaceId) override;
    void setHoldInt(int interval, int ifaceId) override;
    void setSplitHorizon(bool shenabled, int ifaceId) override;
    void setPassive(bool passive, int ifaceId) override;
    void setStub(const EigrpStub& stub) override { this->eigrpStub = stub; this->eigrpStubEnabled = true; }
    void setRouterId(IPv4Address routerID) override { this->eigrpTt->setRouterId(routerID); }

    //-- INTERFACE IEigrpPdm;
    void sendUpdate(int destNeighbor, EigrpRoute<IPv6Address> *route, EigrpRouteSource<IPv6Address> *source, bool forcePoisonRev, const char *reason) override;
    void sendQuery(int destNeighbor, EigrpRoute<IPv6Address> *route, EigrpRouteSource<IPv6Address> *source, bool forcePoisonRev = false) override;
    void sendReply(EigrpRoute<IPv6Address> *route, int destNeighbor, EigrpRouteSource<IPv6Address> *source, bool forcePoisonRev = false, bool isUnreachable = false) override;
    EigrpRouteSource<IPv6Address> *updateRoute(EigrpRoute<IPv6Address> *route, uint64_t dmin, bool *rtableChanged, bool removeUnreach = false) override;
    uint64_t findRouteDMin(EigrpRoute<IPv6Address> *route) override { return eigrpTt->findRouteDMin(route); }
    bool hasFeasibleSuccessor(EigrpRoute<IPv6Address> *route, uint64_t& resultDmin) override { return eigrpTt->hasFeasibleSuccessor(route, resultDmin); }
    EigrpRouteSource<IPv6Address> *getBestSuccessor(EigrpRoute<IPv6Address> *route) override { return eigrpTt->getBestSuccessor(route); }
    bool setReplyStatusTable(EigrpRoute<IPv6Address> *route, EigrpRouteSource<IPv6Address> *source, bool forcePoisonRev, int *neighCount, int *stubCount) override;
    bool hasNeighborForUpdate(EigrpRouteSource<IPv6Address> *source) override;
    void setDelayedRemove(int neighId, EigrpRouteSource<IPv6Address> *src) override;
    void sendUpdateToStubs(EigrpRouteSource<IPv6Address> *succ ,EigrpRouteSource<IPv6Address> *oldSucc, EigrpRoute<IPv6Address> *route) override;

    bool addNetPrefix(const IPv6Address &network, const short int prefixLen, const int ifaceId) override;
};

/**
 * @brief Class gives access to the PimNeighborTable.
 */
/*
class INET_API EigrpIpv6PdmAccess : public ModuleAccess<EigrpIpv6Pdm>
{
  public:
    EigrpIpv6PdmAccess() : ModuleAccess<EigrpIpv6Pdm>("EigrpIpv6Pdm") {}
};
*/
}
#endif /* EIGRPIPV6PDM_H_ */
