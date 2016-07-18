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
* @file EigrpIpv4Pdm.h
* @author Jan Bloudicek (mailto:jbloudicek@gmail.com)
* @brief EIGRP IPv4 Protocol Dependent Module
* @detail Main module, it mediates control exchange between DUAL, routing table and
topology table.
*/

#ifndef __INET_EIGRPIPV4PDM_H_
#define __INET_EIGRPIPV4PDM_H_

#include <omnetpp.h>

//#include "InterfaceTable.h"
//#include "InterfaceEntry.h"
//#include "IPv4Address.h"
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

#include "inet/networklayer/ipv4/IPv4RoutingTable.h"
#include "ansa/networklayer/common/ANSA_InterfaceEntry.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"

namespace inet {
/**
 * Class represents EIGRP Protocol Dependent Module for IPv4. It contains IPv4 specific things.
 */
class EigrpIpv4Pdm : public cSimpleModule, public IEigrpModule<IPv4Address>, public IEigrpPdm<IPv4Address>, protected cListener
{
  protected:
    typedef std::vector<IPv4Route *> RouteVector;
    typedef std::vector<EigrpMsgReq *> RequestVector;

    const char* SPLITTER_OUTGW;         /**< Output gateway to the EIGRP Splitter module */
    const char* RTP_OUTGW;              /**< Output gateway to the RTP module */
    const IPv4Address EIGRP_IPV4_MULT; /**< Multicast address for EIGRP messages */
    EigrpKValues KVALUES_MAX;           /**< K-values (from K1 to K5) are set to max */
    const IPv4Address EIGRP_SELF_ADDR;  /**< Next hop address 0.0.0.0 (self address) */

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
    //AnsaRoutingTable *rt;
    IPv4RoutingTable* rt;
    //NotificationBoard *nb;

    EigrpDual<IPv4Address> *eigrpDual;
    EigrpMetricHelper *eigrpMetric;
    EigrpInterfaceTable *eigrpIft;                   /**< Table with enabled EIGRP interfaces */
    EigrpDisabledInterfaces *eigrpIftDisabled;       /**< Disabled EIGRP interfaces */
    EigrpIpv4NeighborTable *eigrpNt;                /**< Table with EIGRP neighbors */
    EigrpIpv4TopologyTable *eigrpTt;                /**< Topology table */
    EigrpNetworkTable<IPv4Address> *routingForNetworks;          /**< Networks included in EIGRP */
    RequestVector reqQueue;                         /**< Requests for sending EIGRP messages from DUAL */

    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    /**< Multi-stage initialization. */
    virtual int numInitStages() const override { return 4; }
    //virtual void receiveChangeNotification(int category, const cObject *details);
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG) override;

    void printSentMsg(int routeCnt, IPv4Address& destAddress, EigrpMsgReq *msgReq);
    void printRecvMsg(EigrpMessage *msg, IPv4Address& addr, int ifaceId);


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


    //-- METHODS FOR CREATING MESSAGES
    EigrpIpv4Hello *createHelloMsg(int holdInt, EigrpKValues kValues, IPv4Address& destAddress, EigrpMsgReq *msgReq);
    EigrpIpv4Ack *createAckMsg(IPv4Address& destAddress, EigrpMsgReq *msgReq);
    EigrpIpv4Update *createUpdateMsg(const IPv4Address& destAddress, EigrpMsgReq *msgReq);
    EigrpIpv4Query *createQueryMsg(IPv4Address& destAddress, EigrpMsgReq *msgReq);
    EigrpIpv4Reply *createReplyMsg(IPv4Address& destAddress, EigrpMsgReq *msgReq);
    /**
     * Adds ControlInfo for network layer module.
     */
    void addCtrInfo(EigrpMessage *msg, int ifaceId, const IPv4Address &destAddress);
    void addMessageHeader(EigrpMessage *msg, int opcode, EigrpMsgReq *msgReq);
    void createRouteTlv(EigrpMpIpv4Internal *routeTlv, EigrpRoute<IPv4Address> *route, bool unreachable = false);
    /**
     * Add routes from request to the message.
     */
    void addRoutesToMsg(EigrpIpv4Message *msg, const EigrpMsgReq *msgReq);
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
    void processAckMsg(cMessage *msg, IPv4Address& srcAddress, int ifaceId, EigrpNeighbor<IPv4Address> *neigh);
    void processHelloMsg(cMessage *msg, IPv4Address& srcAddress, int ifaceId, EigrpNeighbor<IPv4Address> *neigh);
    void processUpdateMsg(cMessage *msg, IPv4Address& srcAddress, int ifaceId, EigrpNeighbor<IPv4Address> *neigh);
    void processQueryMsg(cMessage *msg, IPv4Address& srcAddress, int ifaceId, EigrpNeighbor<IPv4Address> *neigh);
    void processReplyMsg(cMessage *msg, IPv4Address& srcAddress, int ifaceId, EigrpNeighbor<IPv4Address> *neigh);
    /**
     * Process route TLV.
     */
    EigrpRouteSource<IPv4Address> *processInterRoute(EigrpMpIpv4Internal& tlv, IPv4Address& nextHop, int sourceNeighId, EigrpInterface *eigrpIface, bool *notifyDual, bool *isSourceNew);


    //-- NEIGHBORSHIP MANAGEMENT
    /**
     * Creates and sends message with all routes from routing table to specified neighbor.
     */
    void sendAllEigrpPaths(EigrpInterface *eigrpIface, EigrpNeighbor<IPv4Address> *neigh);
    /**
     * Creates relationship with neighbor.
     * @param srcAddress address of the neighbor
     * @param ifaceId ID of interface where the neighbor is connected
     */
    void processNewNeighbor(int ifaceId, IPv4Address &srcAddress, EigrpIpv4Hello *helloMessage);
    /**
     * Checks neighborship rules.
     * @param ifaceId ID of interface where the neighbor is connected.
     * @return returns code from enumeration eigrp::UserMsgCodes.
     */
    int checkNeighborshipRules(int ifaceId, int neighAsNum, IPv4Address &neighAddr,
            const EigrpKValues &neighKValues);
    /**
     * Create record in the neighbor table and start hold timer.
     */
    EigrpNeighbor<IPv4Address> *createNeighbor(EigrpInterface *eigrpIface, IPv4Address& address, uint16_t holdInt);
    /**
     * Removes neighbor from neighbor table and delete it. Notifies DUAL about event.
     */
    void removeNeighbor(EigrpNeighbor<IPv4Address> *neigh);


    //-- INTERFACE MANAGEMENT
    /**
     * Remove interface from EIGRP interface table. Removes all neighbors on the interface.
     */
    void disableInterface(ANSA_InterfaceEntry *iface, EigrpInterface *eigrpIface, IPv4Address& ifAddress, IPv4Address& ifMask);
    /**
     * Add interface to the EIGRP interface table and notifies DUAL.
     */
    void enableInterface(EigrpInterface *eigrpIface, IPv4Address& ifAddress, IPv4Address& ifMask, int networkId);
    /**
     * Returns EIGRP interface (enabled or disabled) or NULL.
     */
    EigrpInterface *getInterfaceById(int ifaceId);
    /**
     * Creates interface and inserts it to the table.
     */
    EigrpInterface *addInterfaceToEigrp(int ifaceId, int networkId, bool enabled);


    //-- PROCESSING EVENTS FROM NOTIFICATION BOARD
    void processIfaceStateChange(ANSA_InterfaceEntry *iface);
    void processIfaceConfigChange(EigrpInterface *eigrpIface);
    void processRTRouteDel(const cObject *details);

    /**
     * Returns next hop address. If next hop in message is 0.0.0.0, then next hop must be
     * replaced by IP address of sender.
     */
    IPv4Address getNextHopAddr(IPv4Address& nextHopAddr, IPv4Address& senderAddr)
        { return (nextHopAddr.isUnspecified()) ? senderAddr : nextHopAddr; }
    /**
     * Returns IP address for sending EIGRP message.
     */
    bool getDestIpAddress(int destNeigh, IPv4Address *resultAddress);


    //-- ROUTING TABLE MANAGEMENT
    bool removeRouteFromRT(EigrpRouteSource<IPv4Address> *successor, IRoute::SourceType *removedRtSrc);
    IPv4Route *createRTRoute(EigrpRouteSource<IPv4Address> *successor);
    /**
     * Updates existing route in the routing table or creates new one.
     */
    bool installRouteToRT(EigrpRoute<IPv4Address> *route, EigrpRouteSource<IPv4Address> *source, uint64_t dmin, IPv4Route *rtEntry);
    /**
     * Returns true, if routing table does not contain route with given address, mask and
     * smaller administrative distance.
     */
    bool isRTSafeForAdd(EigrpRoute<IPv4Address> *route, unsigned int eigrpAd);
    /**
     * Changes metric of route in routing table. For wide metric uses scale.
     */
    void setRTRouteMetric(IPv4Route *route, uint64_t metric) { if (!useClassicMetric) { metric = metric / ribScale; } route->setMetric(metric); }
    /**
     * Removes route from routing table and changes old successor's record in topology table.
     */
    bool removeOldSuccessor(EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route);


    //-- METHODS FOR MESSAGE REQUESTS
    /**
     * Records request to send message to all neighbors.
     */
    void msgToAllIfaces(int destination, HeaderOpcode msgType, EigrpRouteSource<IPv4Address> *source, bool forcePoisonRev, bool forceUnreachable);
    /**
     * Creates request for sending message on specified interface.
     */
    void msgToIface(HeaderOpcode msgType, EigrpRouteSource<IPv4Address> *source, EigrpInterface *eigrpIface,  bool forcePoisonRev = false, bool forceUnreachable = false);
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
    bool applySplitHorizon(EigrpInterface *destInterface, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route);
    /**
     * Apply stub configuration to the route in outgoing Update message.
     * @return true, if stub setting limits sending of the route, otherwise false
     */
    bool applyStubToUpdate(EigrpRouteSource<IPv4Address> *src);
    /**
     * Apply stub configuration to the route in outgoing Query message.
     * @return true, if stub setting limits sending of the route, otherwise false
     */
    bool applyStubToQuery(EigrpInterface *eigrpIface, int numOfNeigh);

    IPv4Route *findRoute(const IPv4Address& network, const IPv4Address& netmask);
    IPv4Route *findRoute(const IPv4Address& network, const IPv4Address& netmask, const IPv4Address& nexthop);


  public:
    EigrpIpv4Pdm();
    ~EigrpIpv4Pdm();

    //-- INTERFACE IEigrpModule
    void addInterface(int ifaceId, int networkId, bool enabled) override { addInterfaceToEigrp(ifaceId, networkId, enabled); }
    void addInterface(int ifaceId, bool enabled) override { /* useful only for IPv6 */ }
    EigrpNetwork<IPv4Address> *addNetwork(IPv4Address address, IPv4Address mask) override;
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
    bool addNetPrefix(const IPv4Address &network, const short int prefixLen, const int ifaceId) override { return false;/* useful only for IPv6 */ }

    //-- INTERFACE IEigrpPdm;
    void sendUpdate(int destNeighbor, EigrpRoute<IPv4Address> *route, EigrpRouteSource<IPv4Address> *source, bool forcePoisonRev, const char *reason) override;
    void sendQuery(int destNeighbor, EigrpRoute<IPv4Address> *route, EigrpRouteSource<IPv4Address> *source, bool forcePoisonRev = false) override;
    void sendReply(EigrpRoute<IPv4Address> *route, int destNeighbor, EigrpRouteSource<IPv4Address> *source, bool forcePoisonRev = false, bool isUnreachable = false) override;
    EigrpRouteSource<IPv4Address> *updateRoute(EigrpRoute<IPv4Address> *route, uint64_t dmin, bool *rtableChanged, bool removeUnreach = false) override;
    uint64_t findRouteDMin(EigrpRoute<IPv4Address> *route) override { return eigrpTt->findRouteDMin(route); }
    bool hasFeasibleSuccessor(EigrpRoute<IPv4Address> *route, uint64_t& resultDmin) override { return eigrpTt->hasFeasibleSuccessor(route, resultDmin); }
    EigrpRouteSource<IPv4Address> *getBestSuccessor(EigrpRoute<IPv4Address> *route) override { return eigrpTt->getBestSuccessor(route); }
    bool setReplyStatusTable(EigrpRoute<IPv4Address> *route, EigrpRouteSource<IPv4Address> *source, bool forcePoisonRev, int *neighCount, int *stubCount) override;
    bool hasNeighborForUpdate(EigrpRouteSource<IPv4Address> *source) override;
    void setDelayedRemove(int neighId, EigrpRouteSource<IPv4Address> *src) override;
    void sendUpdateToStubs(EigrpRouteSource<IPv4Address> *succ ,EigrpRouteSource<IPv4Address> *oldSucc, EigrpRoute<IPv4Address> *route) override;
};

/**
 * @brief Class gives access to the PimNeighborTable.
 */
/*
class INET_API EigrpIpv4PdmAccess : public ModuleAccess<EigrpIpv4Pdm>
{
  public:
    EigrpIpv4PdmAccess() : ModuleAccess<EigrpIpv4Pdm>("EigrpIpv4Pdm") {}
};
*/
}
#endif
