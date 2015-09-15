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

#include "networklayer/common/InterfaceTable.h"
#include "networklayer/common/InterfaceEntry.h"
#include "networklayer/contract/ipv6/IPv6Address.h"
#include "ansa/networklayer/ipv4/AnsaRoutingTable.h"
#include "base/NotificationBoard.h"

#include "ansa/networklayer/eigrp/tables/EigrpInterfaceTable.h"
#include "ansa/networklayer/eigrp/tables/EigrpNeighborTable.h"
#include "ansa/networklayer/eigrp/tables/EigrpNeighborTable.h"
#include "ansa/networklayer/eigrp/tables/EigrpRoute.h"
#include "ansa/networklayer/eigrp/tables/EigrpTopologyTable.h"

#include "ansa/networklayer/eigrp/pdms/IEigrpModule.h"
#include "ansa/networklayer/eigrp/pdms/IEigrpPdm.h"

#include "ansa/networklayer/eigrp/messages/EigrpMessage_m.h"
#include "ansa/networklayer/eigrp/tables/EigrpDisabledInterfaces.h"
#include "ansa/networklayer/eigrp/EigrpDual.h"
#include "ansa/networklayer/eigrp/tables/EigrpNetworkTable.h"
#include "ansa/networklayer/eigrp/pdms/EigrpMetricHelper.h"
#include "ansa/networklayer/eigrp/messages/EigrpMsgReq.h"

#include "ansa/networklayer/eigrp/EigrpDualStack.h"
#include "ansa/networklayer/ipv6/ANSARoutingTable6.h"
#include "networklayer/ipv6/IPv6InterfaceData.h"

/**
 * Class represents EIGRP Protocol Dependent Module for IPv6. It contains IPv6 specific things.
 */
class EigrpIpv6Pdm : public cSimpleModule, public IEigrpModule<inet::IPv6Address>, public IEigrpPdm<inet::IPv6Address>, public INotifiable
{
  protected:
    struct IPv6netPrefix {
       int ifaceId;
       inet::IPv6Address network;
       short int prefixLength;
    };
    typedef std::vector<ANSAIPv6Route *> RouteVector;
    typedef std::vector<EigrpMsgReq *> RequestVector;
    typedef std::vector<IPv6netPrefix> PrefixVector;

    const char* SPLITTER_OUTGW;         /**< Output gateway to the EIGRP Splitter module */
    const char* RTP_OUTGW;              /**< Output gateway to the RTP module */
    const inet::IPv6Address EIGRP_IPV6_MULT; /**< Multicast address for EIGRP messages */
    EigrpKValues KVALUES_MAX;           /**< K-values (from K1 to K5) are set to max */
    const inet::IPv6Address EIGRP_SELF_ADDR;  /**< Next hop address 0.0.0.0 (self address) */

    int asNum;                  /**< Autonomous system number */
    EigrpKValues kValues;       /**< K-values for calculation of metric */
    int maximumPath;            /**< Maximum number of parallel routes that EIGRP will support */
    int variance;               /**< Parameter for unequal cost load balancing */
    unsigned int adminDistInt; /**< Administrative distance */
    bool useClassicMetric;      /**< Use classic metric computation or wide metric computation */
    int ribScale;               /**< Scaling factor for Wide metric */
    bool eigrpStubEnabled;      /**< True when EIGRP stub is on */
    EigrpStub eigrpStub;        /**< EIGRP stub configuration */

    inet::IInterfaceTable *ift;
    ANSARoutingTable6 *rt;
    NotificationBoard *nb;

    EigrpDual<inet::IPv6Address> *eigrpDual;
    EigrpMetricHelper *eigrpMetric;
    EigrpInterfaceTable *eigrpIft;                   /**< Table with enabled EIGRP interfaces */
    EigrpDisabledInterfaces *eigrpIftDisabled;       /**< Disabled EIGRP interfaces */
    EigrpNeighborTable<inet::IPv6Address> *eigrpNt;                /**< Table with EIGRP neighbors */
    EigrpTopologyTable<inet::IPv6Address> *eigrpTt;                /**< Topology table */
    EigrpNetworkTable<inet::IPv6Address> *routingForNetworks;          /**< Networks included in EIGRP */
    RequestVector reqQueue;                         /**< Requests for sending EIGRP messages from DUAL */
    PrefixVector netPrefixes;

    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    /**< Multi-stage initialization. */
    virtual int numInitStages() const { return 4; }
    virtual void receiveChangeNotification(int category, const cObject *details);

    void printSentMsg(int routeCnt, inet::IPv6Address& destAddress, EigrpMsgReq *msgReq);
    void printRecvMsg(EigrpMessage *msg, inet::IPv6Address& addr, int ifaceId);


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
    EigrpIpv6Hello *createHelloMsg(int holdInt, EigrpKValues kValues, inet::IPv6Address& destAddress, EigrpMsgReq *msgReq);
    EigrpIpv6Ack *createAckMsg(inet::IPv6Address& destAddress, EigrpMsgReq *msgReq);
    EigrpIpv6Update *createUpdateMsg(const inet::IPv6Address& destAddress, EigrpMsgReq *msgReq);
    EigrpIpv6Query *createQueryMsg(inet::IPv6Address& destAddress, EigrpMsgReq *msgReq);
    EigrpIpv6Reply *createReplyMsg(inet::IPv6Address& destAddress, EigrpMsgReq *msgReq);
    /**
     * Adds ControlInfo for network layer module.
     */
    void addCtrInfo(EigrpMessage *msg, int ifaceId, const inet::IPv6Address &destAddress);
    void addMessageHeader(EigrpMessage *msg, int opcode, EigrpMsgReq *msgReq);
    void createRouteTlv(EigrpMpIpv6Internal *routeTlv, EigrpRoute<inet::IPv6Address> *route, bool unreachable = false);
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
    void processAckMsg(cMessage *msg, inet::IPv6Address& srcAddress, int ifaceId, EigrpNeighbor<inet::IPv6Address> *neigh);
    void processHelloMsg(cMessage *msg, inet::IPv6Address& srcAddress, int ifaceId, EigrpNeighbor<inet::IPv6Address> *neigh);
    void processUpdateMsg(cMessage *msg, inet::IPv6Address& srcAddress, int ifaceId, EigrpNeighbor<inet::IPv6Address> *neigh);
    void processQueryMsg(cMessage *msg, inet::IPv6Address& srcAddress, int ifaceId, EigrpNeighbor<inet::IPv6Address> *neigh);
    void processReplyMsg(cMessage *msg, inet::IPv6Address& srcAddress, int ifaceId, EigrpNeighbor<inet::IPv6Address> *neigh);
    /**
     * Process route TLV.
     */
    EigrpRouteSource<inet::IPv6Address> *processInterRoute(EigrpMpIpv6Internal& tlv, inet::IPv6Address& nextHop, int sourceNeighId, EigrpInterface *eigrpIface, bool *notifyDual, bool *isSourceNew);


    //-- NEIGHBORSHIP MANAGEMENT
    /**
     * Creates and sends message with all routes from routing table to specified neighbor.
     */
    void sendAllEigrpPaths(EigrpInterface *eigrpIface, EigrpNeighbor<inet::IPv6Address> *neigh);
    /**
     * Creates relationship with neighbor.
     * @param srcAddress address of the neighbor
     * @param ifaceId ID of interface where the neighbor is connected
     */
    void processNewNeighbor(int ifaceId, inet::IPv6Address &srcAddress, EigrpIpv6Hello *helloMessage);
    /**
     * Checks neighborship rules.
     * @param ifaceId ID of interface where the neighbor is connected.
     * @return returns code from enumeration eigrp::UserMsgCodes.
     */
    int checkNeighborshipRules(int ifaceId, int neighAsNum, inet::IPv6Address &neighAddr,
            const EigrpKValues &neighKValues);
    /**
     * Create record in the neighbor table and start hold timer.
     */
    EigrpNeighbor<inet::IPv6Address> *createNeighbor(EigrpInterface *eigrpIface, inet::IPv6Address& address, uint16_t holdInt);
    /**
     * Removes neighbor from neighbor table and delete it. Notifies DUAL about event.
     */
    void removeNeighbor(EigrpNeighbor<inet::IPv6Address> *neigh);


    //-- INTERFACE MANAGEMENT
    /**
     * Remove interface from EIGRP interface table. Removes all neighbors on the interface.
     */
    //void disableInterface(inet::InterfaceEntry *iface, EigrpInterface *eigrpIface, inet::IPv6Address& ifAddress, IPv6Address& ifMask);
    void disableInterface(inet::InterfaceEntry *iface, EigrpInterface *eigrpIface);
    /**
     * Add interface to the EIGRP interface table and notifies DUAL.
     */
    //void enableInterface(EigrpInterface *eigrpIface, inet::IPv6Address& ifAddress, IPv6Address& ifMask, int networkId);
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
    void processIfaceStateChange(inet::InterfaceEntry *iface);
    void processIfaceConfigChange(EigrpInterface *eigrpIface);
    void processRTRouteDel(const cObject *details);

    /**
     * Returns next hop address. If next hop in message is 0.0.0.0, then next hop must be
     * replaced by IP address of sender.
     */
    inet::IPv6Address getNextHopAddr(inet::IPv6Address& nextHopAddr, inet::IPv6Address& senderAddr)
        { return (nextHopAddr.isUnspecified()) ? senderAddr : nextHopAddr; }
    /**
     * Returns IP address for sending EIGRP message.
     */
    bool getDestIpAddress(int destNeigh, inet::IPv6Address *resultAddress);


    //-- ROUTING TABLE MANAGEMENT
    bool removeRouteFromRT(EigrpRouteSource<inet::IPv6Address> *successor, ANSAIPv6Route::RoutingProtocolSource *removedRtSrc);
    ANSAIPv6Route *createRTRoute(EigrpRouteSource<inet::IPv6Address> *successor);
    /**
     * Updates existing route in the routing table or creates new one.
     */
    bool installRouteToRT(EigrpRoute<inet::IPv6Address> *route, EigrpRouteSource<inet::IPv6Address> *source, uint64_t dmin, inet::IPv6Route *rtEntry);
    /**
     * Returns true, if routing table does not contain route with given address, mask and
     * smaller administrative distance.
     */
    bool isRTSafeForAdd(EigrpRoute<inet::IPv6Address> *route, unsigned int eigrpAd);
    /**
     * Changes metric of route in routing table. For wide metric uses scale.
     */
    void setRTRouteMetric(inet::IPv6Route *route, uint64_t metric) { if (!useClassicMetric) { metric = metric / ribScale; } route->setMetric(metric); }
    /**
     * Removes route from routing table and changes old successor's record in topology table.
     */
    bool removeOldSuccessor(EigrpRouteSource<inet::IPv6Address> *source, EigrpRoute<inet::IPv6Address> *route);


    //-- METHODS FOR MESSAGE REQUESTS
    /**
     * Records request to send message to all neighbors.
     */
    void msgToAllIfaces(int destination, HeaderOpcode msgType, EigrpRouteSource<inet::IPv6Address> *source, bool forcePoisonRev, bool forceUnreachable);
    /**
     * Creates request for sending message on specified interface.
     */
    void msgToIface(HeaderOpcode msgType, EigrpRouteSource<inet::IPv6Address> *source, EigrpInterface *eigrpIface,  bool forcePoisonRev = false, bool forceUnreachable = false);
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
    bool applySplitHorizon(EigrpInterface *destInterface, EigrpRouteSource<inet::IPv6Address> *source, EigrpRoute<inet::IPv6Address> *route);
    /**
     * Apply stub configuration to the route in outgoing Update message.
     * @return true, if stub setting limits sending of the route, otherwise false
     */
    bool applyStubToUpdate(EigrpRouteSource<inet::IPv6Address> *src);
    /**
     * Apply stub configuration to the route in outgoing Query message.
     * @return true, if stub setting limits sending of the route, otherwise false
     */
    bool applyStubToQuery(EigrpInterface *eigrpIface, int numOfNeigh);

  public:
    EigrpIpv6Pdm();
    ~EigrpIpv6Pdm();

    //-- INTERFACE IEigrpModule
    void addInterface(int ifaceId, int networkId, bool enabled) { /* useful only for IPv4 */ }
    void addInterface(int ifaceId, bool enabled) { addInterfaceToEigrp(ifaceId, enabled); }
    EigrpNetwork<inet::IPv6Address> *addNetwork(inet::IPv6Address address, inet::IPv6Address mask);
    void setASNum(int asNum) { this->asNum = asNum; }
    int getASNum() {return this->asNum; }
    void setKValues(const EigrpKValues& kValues) { this->kValues = kValues; }
    void setMaximumPath(int maximumPath) { this->maximumPath = maximumPath; }
    void setVariance(int variance) { this->variance = variance; }
    void setHelloInt(int interval, int ifaceId);
    void setHoldInt(int interval, int ifaceId);
    void setSplitHorizon(bool shenabled, int ifaceId);
    void setPassive(bool passive, int ifaceId);
    void setStub(const EigrpStub& stub) { this->eigrpStub = stub; this->eigrpStubEnabled = true; }
    void setRouterId(inet::IPv4Address routerID) { this->eigrpTt->setRouterId(routerID); }

    //-- INTERFACE IEigrpPdm;
    void sendUpdate(int destNeighbor, EigrpRoute<inet::IPv6Address> *route, EigrpRouteSource<inet::IPv6Address> *source, bool forcePoisonRev, const char *reason);
    void sendQuery(int destNeighbor, EigrpRoute<inet::IPv6Address> *route, EigrpRouteSource<inet::IPv6Address> *source, bool forcePoisonRev = false);
    void sendReply(EigrpRoute<inet::IPv6Address> *route, int destNeighbor, EigrpRouteSource<inet::IPv6Address> *source, bool forcePoisonRev = false, bool isUnreachable = false);
    EigrpRouteSource<inet::IPv6Address> *updateRoute(EigrpRoute<inet::IPv6Address> *route, uint64_t dmin, bool *rtableChanged, bool removeUnreach = false);
    uint64_t findRouteDMin(EigrpRoute<inet::IPv6Address> *route) { return eigrpTt->findRouteDMin(route); }
    bool hasFeasibleSuccessor(EigrpRoute<inet::IPv6Address> *route, uint64_t& resultDmin) { return eigrpTt->hasFeasibleSuccessor(route, resultDmin); }
    EigrpRouteSource<inet::IPv6Address> *getBestSuccessor(EigrpRoute<inet::IPv6Address> *route) { return eigrpTt->getBestSuccessor(route); }
    bool setReplyStatusTable(EigrpRoute<inet::IPv6Address> *route, EigrpRouteSource<inet::IPv6Address> *source, bool forcePoisonRev, int *neighCount, int *stubCount);
    bool hasNeighborForUpdate(EigrpRouteSource<inet::IPv6Address> *source);
    void setDelayedRemove(int neighId, EigrpRouteSource<inet::IPv6Address> *src);
    void sendUpdateToStubs(EigrpRouteSource<inet::IPv6Address> *succ ,EigrpRouteSource<inet::IPv6Address> *oldSucc, EigrpRoute<inet::IPv6Address> *route);

    bool addNetPrefix(const inet::IPv6Address &network, const short int prefixLen, const int ifaceId);
};

/**
 * @brief Class gives access to the PimNeighborTable.
 */
class INET_API EigrpIpv6PdmAccess : public inet::ModuleAccess<EigrpIpv6Pdm>
{
  public:
    EigrpIpv6PdmAccess() : inet::ModuleAccess<EigrpIpv6Pdm>("EigrpIpv6Pdm") {}
};

#endif /* EIGRPIPV6PDM_H_ */
