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
#include "IEigrpPdm.h"

#include "EigrpMessage_m.h"
#include "EigrpDisabledInterfaces.h"
#include "EigrpDual.h"
#include "EigrpNetworkTable.h"
#include "EigrpMetricHelper.h"
#include "EigrpMsgReq.h"

/**
 * Class represents EIGRP Protocol Dependent Module for IPv4. It contains IPv4 specific things.
 */
class EigrpIpv4Pdm : public cSimpleModule, public IEigrpModule, public IEigrpPdm, public INotifiable
{
  protected:
    typedef std::vector<ANSAIPv4Route *> RouteVector;
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
    AnsaRoutingTable *rt;
    NotificationBoard *nb;

    EigrpDual *eigrpDual;
    EigrpMetricHelper *eigrpMetric;
    EigrpInterfaceTable *eigrpIft;                   /**< Table with enabled EIGRP interfaces */
    EigrpDisabledInterfaces *eigrpIftDisabled;       /**< Disabled EIGRP interfaces */
    EigrpIpv4NeighborTable *eigrpNt;                /**< Table with EIGRP neighbors */
    EigrpIpv4TopologyTable *eigrpTt;                /**< Topology table */
    EigrpNetworkTable *routingForNetworks;          /**< Networks included in EIGRP */
    RequestVector reqQueue;                         /**< Requests for sending EIGRP messages from DUAL */

    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    /**< Multi-stage initialization. */
    virtual int numInitStages() const { return 4; }
    virtual void receiveChangeNotification(int category, const cObject *details);

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
    void disableInterface(InterfaceEntry *iface, EigrpInterface *eigrpIface, IPv4Address& ifAddress, IPv4Address& ifMask);
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
    void processIfaceStateChange(InterfaceEntry *iface);
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
    bool removeRouteFromRT(EigrpRouteSource<IPv4Address> *successor, ANSAIPv4Route::RoutingProtocolSource *removedRtSrc);
    ANSAIPv4Route *createRTRoute(EigrpRouteSource<IPv4Address> *successor);
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

  public:
    EigrpIpv4Pdm();
    ~EigrpIpv4Pdm();

    //-- INTERFACE IEigrpModule
    void addInterface(int ifaceId, int networkId, bool enabled) { addInterfaceToEigrp(ifaceId, networkId, enabled); }
    EigrpNetwork *addNetwork(IPv4Address address, IPv4Address mask);
    void setASNum(int asNum) { this->asNum = asNum; }
    void setKValues(const EigrpKValues& kValues) { this->kValues = kValues; }
    void setMaximumPath(int maximumPath) { this->maximumPath = maximumPath; }
    void setVariance(int variance) { this->variance = variance; }
    void setHelloInt(int interval, int ifaceId);
    void setHoldInt(int interval, int ifaceId);
    void setSplitHorizon(bool shenabled, int ifaceId);
    void setPassive(bool passive, int ifaceId);
    void setStub(const EigrpStub& stub) { this->eigrpStub = stub; this->eigrpStubEnabled = true; }

    //-- INTERFACE IEigrpPdm;
    void sendUpdate(int destNeighbor, EigrpRoute<IPv4Address> *route, EigrpRouteSource<IPv4Address> *source, bool forcePoisonRev, const char *reason);
    void sendQuery(int destNeighbor, EigrpRoute<IPv4Address> *route, EigrpRouteSource<IPv4Address> *source, bool forcePoisonRev = false);
    void sendReply(EigrpRoute<IPv4Address> *route, int destNeighbor, EigrpRouteSource<IPv4Address> *source, bool forcePoisonRev = false, bool isUnreachable = false);
    EigrpRouteSource<IPv4Address> *updateRoute(EigrpRoute<IPv4Address> *route, uint64_t dmin, bool *rtableChanged, bool removeUnreach = false);
    uint64_t findRouteDMin(EigrpRoute<IPv4Address> *route) { return eigrpTt->findRouteDMin(route); }
    bool hasFeasibleSuccessor(EigrpRoute<IPv4Address> *route, uint64_t& resultDmin) { return eigrpTt->hasFeasibleSuccessor(route, resultDmin); }
    EigrpRouteSource<IPv4Address> *getBestSuccessor(EigrpRoute<IPv4Address> *route) { return eigrpTt->getBestSuccessor(route); }
    bool setReplyStatusTable(EigrpRoute<IPv4Address> *route, EigrpRouteSource<IPv4Address> *source, bool forcePoisonRev, int *neighCount, int *stubCount);
    bool hasNeighborForUpdate(EigrpRouteSource<IPv4Address> *source);
    void setDelayedRemove(int neighId, EigrpRouteSource<IPv4Address> *src);
    void sendUpdateToStubs(EigrpRouteSource<IPv4Address> *succ ,EigrpRouteSource<IPv4Address> *oldSucc, EigrpRoute<IPv4Address> *route);
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
