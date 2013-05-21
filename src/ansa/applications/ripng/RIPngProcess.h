// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
/**
* @file RIPngProcess.h
* @author Jiri Trhlik (mailto:jiritm@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
* @brief RIPng process
* @detail RIPng process (Cisco-like), implements RIPng protocol
*/

#ifndef RIPNGPROCESS_H_
#define RIPNGPROCESS_H_

#include <omnetpp.h>
#include "IPv6Address.h"
#include "ANSARoutingTable6.h"
#include "ANSARoutingTable6Access.h"
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"

#include "RIPngInterface.h"
#include "RIPngRoutingTableEntry.h"

#include "UDPControlInfo_m.h"
#include "RIPngMessage_m.h"
#include "RIPngTimer_m.h"

class RIPngRouting;

/**
 *  Represents routing protocol RIPng. RIPng uses UDP and communicates
 *  using UDPSocket.
 */
class RIPngProcess
{
  public:
    RIPngProcess(const char *name, RIPngRouting *RIPngModule);
    virtual ~RIPngProcess();

    /**
     * Enable - starts RIPng process
     */
    virtual void start();

    /**
     * To disable the RIPng process, this method must be called
     * before deleting process. Stopped process MUST be deleted
     * - you can't call start() again.
     */
    virtual void stop();

  protected:
    std::string processName;
    RIPngRouting *RIPng;

    const char  *deviceId;   ///< Id of the device which contains this routing process.
    std::string hostName;    ///< Device name from the network topology.
    std::string routerText;  ///< Text with hostName for ev printing.

    int           connNetworkMetric;
    int           infinityMetric;
    simtime_t     routeTimeout;
    simtime_t     routeGarbageCollectionTimeout;
    simtime_t     regularUpdateTimeout;
    IPv6Address   RIPngAddress;
    int           RIPngPort;
    unsigned int  distance;
    bool          poisonReverse;
    bool          splitHorizon;
    int           numOfDefaultInformationInterfaces; ///< Indicates if some of the interfaces sends default route in the updates

    RIPngTimer *regularUpdateTimer;
    RIPngTimer *triggeredUpdateTimer;

    // statistics
    int numRoutes;
    unsigned long regularUpdates;
    unsigned long triggerUpdates;

    bool bSendTriggeredUpdateMessage;
    bool bBlockTriggeredUpdateMessage;

    IInterfaceTable*                                ift;                ///< Provides access to the interface table.
    ANSARoutingTable6*                              rt;                 ///< Provides access to the IPv6 routing table.
    std::vector<RIPng::Interface*>                  enabledInterfaces;  ///< Interfaces which has allowed RIPng
    std::vector<RIPng::Interface*>                  downInterfaces;     ///< Interfaces which has allowed RIPng and are down - for keeping configuration, on every down interface must be called RIPng->setOutputPortOnInterface(interface, -1);

    typedef std::vector<RIPng::RoutingTableEntry*> RoutingTable;
    RoutingTable routingTable;                                          ///< The RIPng routing table.

  public:
    virtual std::string getRoutingTable();
    std::string getProcessName() const { return processName; }

    unsigned long getRegularUpdates() { return regularUpdates; }
    unsigned long getTriggerUpdates() { return triggerUpdates; }

    /**
     * Used by RIPng interfaces only!
     */
    void setDefaultInformation(bool d) { if (d) ++numOfDefaultInformationInterfaces; else --numOfDefaultInformationInterfaces; }

    bool sendingDefaultInformation() { return (numOfDefaultInformationInterfaces > 0) ? true : false; }

    void enableSplitHorizon()  { splitHorizon = true; }
    void disableSplitHorizon()  { splitHorizon = false; }
    bool isSplitHorizon()   { return splitHorizon; }

    void enablePoisonReverse()  { poisonReverse = true; }
    void disablePoisonReverse()  { poisonReverse = false; }
    bool isPoisonReverse()   { return poisonReverse; }

    unsigned int getDistance() { return distance; }
    int getConnNetworkMetric() { return connNetworkMetric; }
    int getInfinityMetric() { return infinityMetric; }
    simtime_t getRouteTimeout() { return routeTimeout; }
    simtime_t getRouteGarbageCollectionTimeout() { return routeGarbageCollectionTimeout; }
    simtime_t getRegularUpdateTimeout() { return regularUpdateTimeout; }
    IPv6Address getRIPngAddress() { return RIPngAddress; }
    int getRIPngPort() { return RIPngPort; }

    bool setDistance(unsigned int distance);
    void setDeviceId(const char *deviceId) { this->deviceId = deviceId; }
    void setHostName(std::string hostName) { this->hostName = hostName; }
    void setRouterText(std::string routerText) { this->routerText = routerText; }
    void setConnNetworkMetric(int connNetworkMetric) { this->connNetworkMetric = connNetworkMetric; }
    void setInfiniteMetric(int infinityMetric) { this->infinityMetric = infinityMetric; }
    bool setRouteTimeout(simtime_t routeTimeout);
    bool setRouteGarbageCollectionTimeout(simtime_t routeGarbageCollectionTimeout);
    bool setRegularUpdateTimeout(simtime_t regularUpdateTimeout);
    bool setRIPngAddress(IPv6Address RIPngAddress) { if (RIPngAddress.isMulticast()) { this->RIPngAddress = RIPngAddress; return true; } return false; }
    bool setRIPngPort(int RIPngPort) { if (RIPngPort > 0 && RIPngPort <= 65535) { this->RIPngPort = RIPngPort; return true; } return false; }

    //-- RIPNG ROUTING TABLE METHODS
    typedef RoutingTable::iterator  RoutingTableIt;
    unsigned long                   getRoutingTableEntryCount() const { return routingTable.size(); }
    RIPng::RoutingTableEntry*       getRoutingTableEntry(const IPv6Address &prefix, int prefixLength);
    void                            addRoutingTableEntry(RIPng::RoutingTableEntry* entry, bool createTimers = true);
    void                            removeRoutingTableEntry(IPv6Address &prefix, int prefixLength);
    void                            removeRoutingTableEntry(RIPng::RoutingTableEntry *entry);
    void                            removeRoutingTableEntry(RoutingTableIt it);

    /**
     * Update routing table entry from rte.
     * @param routingTableEntry [in] pointer to the routing table entry (in the RIPng routingTable) to update
     * @param rte [in] rte from the update message
     * @param srcRIPngIntInd [in] index of the RIPng interface where was (the update message with) the rte "received"
     * @param sourceAddr [in] source of the received rte
     */
    void                            updateRoutingTableEntry(RIPng::RoutingTableEntry *routingTableEntry, RIPngRTE &rte, int srcRIPngIntInd, IPv6Address &sourceAddr);
    void                            removeAllRoutingTableEntries();

    //-- RIPNG INTERFACES METHODS
    unsigned long            getEnabledInterfacesCount() const  { return enabledInterfaces.size(); }
    RIPng::Interface*        getEnabledInterface(unsigned long i)  { return enabledInterfaces[i]; }
    const RIPng::Interface*  getEnabledInterface(unsigned long i) const  { return enabledInterfaces[i]; }
    const char*              getEnabledInterfaceName(unsigned long i) const  { return ift->getInterfaceById(enabledInterfaces[i]->getId())->getName(); }
    int                      getEnabledInterfaceIndexById(int id);
    int                      getEnabledInterfaceIndexByName(const char *name);

    int                      getDownInterfaceIndexById(int id);
  private:
    void                     addEnabledInterface(RIPng::Interface *interface);
    RIPng::Interface*        removeEnabledInterface(unsigned long i);
    void                     removeAllEnabledInterfaces();

  protected:
    //-- GENERAL METHODS
    /**
     *  Creates a RIPng message with no data.
     *  @return pointer to the new RIPng message.
     */
    RIPngMessage *createMessage();

  public:
    /**
     * Enables RIPng on the interface (creates new RIPng::Interface and adds it to the "RIPng interface table").
     * @param interface [in] The interface on which to enable RIPng.
     * @return created RIPng interface
     */
    RIPng::Interface *enableRIPngOnInterface(InterfaceEntry *interface);

    /**
     * Disables RIPng on the interface (removes it from the "RIPng interface table")
     * Method does not deletes the RIPng interface object, only returns pointer to it!
     * @param RIPngInterfaceIndex [in] index of the RIPng interface in the "RIPng interface table"
     */
    RIPng::Interface *disableRIPngOnInterface(unsigned long RIPngInterfaceIndex);

    void setInterfaceMetricOffset(RIPng::Interface *RIPngInterface, int offset);
    void setInterfaceDefaultInformation(RIPng::Interface *RIPngInterface, bool enabled, bool defaultOnly, int metric);
    void setInterfacePassiveStatus(RIPng::Interface *RIPngInterface, bool status);
    void setInterfaceSplitHorizon(RIPng::Interface *RIPngInterface, bool status);
    void setInterfacePoisonReverse(RIPng::Interface *RIPngInterface, bool status);

  protected:
    /**
     *  Adds RIPng Routing Table Entry to the "global outing table", setting the new route.
     *  The function also checks if the route exists and if so, the administrative
     *  distances are compared.
     *  @param entry [in] Routing Table Entry
     */
    void addRoutingTableEntryToGlobalRT(RIPng::RoutingTableEntry* entry);

    /**
     * Removes RIPng Routing Table Entry from the "global routing table".
     * Before removing the function checks if the RIPng route exists in the "global routing table"
     * (because another routing protocol could add the same route with lower AD).
     * @param entry [in] Routing Table Entry
     */
    void removeRoutingTableEntryFromGlobalRT(RIPng::RoutingTableEntry* entry);

    //-- OUTPUT PROCESSING
    /**
     * Creates and sends regular update messages out of every RIPng (non-passive) interface.
     * @param addr [in] destination address of the message
     * @param port [in] destination port of the message
     */
    void sendRegularUpdateMessage();

    /**
     *  Creates and sends triggered update messages -
     *  only routes with the change flag set are included in the message.
     *  The message is sent to the RIPng mutlicast address.
     */
    void sendTriggeredUpdateMessage();

    void sendDelayedTriggeredUpdateMessage();

    /**
     * Make update message for RIPng interface (using Split Horizon if is enabled on the interface).
     * @param interface [in] interface from enabledInterfaces
     * @param changed [in] if only routes with the changed flag set should be included in the message
     * @return pointer to the new update message, if there is no rtes to send returns NULL (e.g., because of Split Horizon)
     */
    RIPngMessage *makeUpdateMessageForInterface(RIPng::Interface *interface, bool changed);

    /**
     *  This method is used by sendRegularUpdateMessage() and sendTriggeredUpdateMessage()
     *  Link-local address of the interface is used as the source address in the message.
     *  @param msg [in] message to be send
     *  @param addr [in] destination address
     *  @param port [in] destination port
     *  @param enabledInterfaceIndex [in] from which RIPng interface should be message sent
     *  @param globalSourceAddress [in] a global-unicast address should be used as the source address of the message (or: source address is left for lower layers)
     *  @see sendRegularUpdateMessage()
     *  @see sendTriggeredUpdateMessage()
     */
    void sendMessage(RIPngMessage *msg, IPv6Address &addr, int port, unsigned long enabledInterfaceIndex, bool globalSourceAddress);

    /**
     *  Sends request for all routes to RIPng address and port
     */
    void sendAllRoutesRequest();

    /**
     *  Clear Route Change Flag for all Routing Table Entries.
     */
    void clearRouteChangeFlags();

    /**
     * Fills in the RIPngRTE vector with Routing Table Entries from the RIPng routing table
     * this method is used for creating RIPng (update) messages
     * @param rtes [out] vector to be filled in
     * @param interfaceId [in] if this parameter is not NULL, then split horizon (with poison reverse) is used based on the interface parameters
     *                         otherwise all RTEs are included
     * @param onlyChanged [in] if this parameter is true, then only routes with routeChangeFlag are included
     */
    void getRTEs(std::vector<RIPngRTE> &rtes, RIPng::Interface *interface, bool onlyChanged = false);

    /**
     *  Creates and returns new RTE base on the routingTableEntry
     *  @param routingTableEntry [in] Routing Table Entry
     *  @return created RIPngRTE object
     */
    RIPngRTE makeRTEFromRoutingTableEntry(RIPng::RoutingTableEntry *routingTableEntry);

  public:
    //-- INPUT PROCESSING
    /**
     *  Handle a RIPng message
     *  @param msg [in] message to process
     */
    void handleRIPngMessage(RIPngMessage *msg);

  protected:
    //-- RESPONSE PROCESSING
    /**
     * Handle a RIPng response message
     * @param response [in] a response to process
     * @param srcRIPngIntInd [in] index of the RIPng interface which "received message"
     * @param srcAddr [in] source address of the response
     * @see processMessage()
     */
    void handleResponse(RIPngMessage *response, int srcRIPngIntInd, IPv6Address &srcAddr);

    /**
     *  Checks for response message validity as is described in RFC 2080
     *  @param responese [in] response message to check
     *  @return true if the response is valid, false otherwise
     */
    bool checkMessageValidity(RIPngMessage *response);

    /**
     *  Process all the RTEs in the response message
     *  @param response [in] the response message
     *  @param srcRIPngIntInd [in] index of the RIPng interface which "received message"
     *  @param sourceAddr [in] an IPv6 address of the source of the RTEs
     */
    void processRTEs(RIPngMessage *response, int srcRIPngIntInd, IPv6Address &sourceAddr);

    /**
     *  Process single RTE from the response message
     *  @param rte [in] the RTE to process
     *  @param srcRIPngIntInd [in] index of the RIPng interface which "received message"
     *  @param sourceAddr [in] an IPv6 address of the source of the RTE
     */
    void processRTE(RIPngRTE &rte, int srcRIPngIntInd, IPv6Address &sourceAddr);

    /**
     *  Checks for the rte validity as is described in RFC 2080
     *  @param rte [in] rte to check
     *  @param sourceAddr [in] rte originator
     *  @return true if the rte is valid, false otherwise
     */
    bool checkAndLogRTE(RIPngRTE &rte, IPv6Address &sourceAddr);

    //-- REQUEST PROCESSING
    /**
     * Handle a RIPng request message
     * @param response [in] a request to process
     * @param srcPort [in] source port of the request
     * @param srcAddr [in] source address of the request
     * @param destAddr [in] destination address of the request
     * @param ripngIntInd [in] ripng interface index of the interface on which the request was received
     * @see processMessage()
     */
    void handleRequest(RIPngMessage *request, int srcPort, IPv6Address &srcAddr, IPv6Address &destAddr, unsigned long ripngIntInd);

  public:
    //-- TIMERS
    /**
     *  Handles self-messages (timers)
     *  @param msg [in] RIPngTimer message
     */
    void handleTimer(RIPngTimer *msg);

  protected:
    /**
     *  Handles regular update timer, sends updates messages and schedules next reg. update
     */
    void handleRegularUpdateTimer();

    /**
     *  Handles triggered update timer, sends triggered update message, if the message is "in the queue"
     */
    void handleTriggeredUpdateTimer();

    /**
     *  Starts Route Deletion Process for the route with the given timer (called when the RTE timer expires).
     *  @param timer [in] Route Deletion Process will be started for the route with this timer
     */
    void startRouteDeletionProcess(RIPngTimer *timer);

    /**
     *  Starts Route Deletion Process for the route.
     *  This method do not stop route timer!
     *  @param timer [in] Route Deletion Process will be started for this route.
     */
    void startRouteDeletionProcess(RIPng::RoutingTableEntry *routingTableEntry);

    /**
     *  Deletes route from RIPng routing table, called when the RTE GCTimer expires.
     *  @param timer [in] Route which should be deleted is given by this timer (also garbage collection timer)
     */
    void deleteRoute(RIPngTimer *timer);

  public:
    //-- NOTIFICATION
    virtual void handleNotification(int category, const cObject *details);
};

#endif /* RIPNGPROCESS_H_ */
