//
// Copyright (C) 2009 - today, Brno University of Technology, Czech Republic
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
* @file CDPMain.h
* @authors Tomas Rajca, Vladimir Vesely (ivesely@fit.vutbr.cz)
* @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
*/

#ifndef CDP_H_
#define CDP_H_

#include <omnetpp.h>

#include "inet/common/INETDefs.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/common/ModuleAccess.h"

#include "ansa/linklayer/cdp/CDPUpdate.h"
#include "ansa/linklayer/cdp/CDPTimer_m.h"
#include "ansa/linklayer/cdp/tables/CDPNeighbourTable.h"
#include "ansa/linklayer/cdp/tables/CDPInterfaceTable.h"
#include "ansa/linklayer/cdp/tables/CDPODRRouteTable.h"

namespace inet {

struct Statistics
{
    int txV1;           // transmission statistics of CDP version 1
    int txV2;           // transmission statistics of CDP version 2
    int rxV1;           // receive statistics of CDP version 1
    int rxV2;           // receive statistics of CDP version 2
    int checksumErr;    // the number of times the checksum (verifying) operation failed
    int headerErr;      // the number of CDP advertisements with bad headers

    std::string info() const;
};

class INET_API CDPMain: public cSimpleModule, public ILifecycle, public cListener {
public:
    CDPMain();
    virtual ~CDPMain();

    CDPInterfaceTable *getCit() {return cit;};
    void setRouteInvalidTime(simtime_t t) {this->routeInvalidTime = t;}
    void setRouteHolddownTime(simtime_t t) {this->routeHolddownTime = t;}
    void setRouteFlushTime(simtime_t t) {this->routeFlushTime = t;}

protected:
    int version;                // cdp version
    IInterfaceTable *ift;       // interface table
    cModule *containingModule;

    bool isOperational = false;    // for lifecycle
    CDPNeighbourTable *cnt;          // cdp neighbour table
    CDPInterfaceTable *cit;          // cdp interface table
    CDPODRRouteTable *ort;           // cdp odr route table
    char cap[4];

    simtime_t holdTime;             // neighbour is deleted after this period of time is elapsed
    simtime_t updateTime;           // time between regular updates
    simtime_t routeFlushTime;       // routes are deleted after this period of time is elapsed
    simtime_t routeHolddownTime;    // how long routes will not be updated
    simtime_t routeInvalidTime;     // learned routes becomes invalid if no update received in this period of time
    simtime_t defaultRouteInvalide; // default routes are deleted after this period of time is elapsed

    IRoutingTable *rt = nullptr;    // routing table from which routes are imported and to which learned routes are added
    int maxDestinationPaths;        // max number of the same routes in routing table
    bool sendIpPrefixes;            // capability of sending ip prefixes
    bool odr;                       // odr routing

    Statistics stat;

    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void finish() override;

    /**
     * Starts CDP
     */
    void startCDP();
    /**
     * Stop CDP
     */
    void stopCDP();

    ////////////////////////////////////
    //      handling with messages    //
    ////////////////////////////////////
    /**
     * Handle timers (self-messages)
     *
     * @param   msg     message
     */
    void handleTimer(CDPTimer *msg);
    /**
     * Handle received update
     *
     * @param   msg     message
     */
    void handleUpdate(CDPUpdate *msg);
    /**
     * Send update on interface
     *
     * @param   interfaceId     ID of outgoing interface
     * @param   shutDown        send only device ID and set TTL to 0
     */
    void sendUpdate(int interface, bool shutDown);
    /**
     * Update cdp neighbour
     *
     * @param   msg     message
     * @param   entry   from which cdp neighbour update came
     */
    void neighbourUpdate(CDPUpdate *msg);

    /////////////////////////////
    //        routes           //
    /////////////////////////////
    /**
     * Add route to routing table and odr routing table.
     *
     * @param   dest            destination
     * @param   prefixLength    prefix length
     * @param   ie              outgoing interface
     * @param   nextHope        next hope address
     */
    void addRoute(const L3Address& dest, int prefixLength, const InterfaceEntry *ie, const L3Address& nextHop);
    /**
     * Removes the route from the routing table and the odr routing table.
     *
     * @param   odrRoute    odr route to be deleted
     */
    void purgeRoute(CDPODRRoute *odrRoute);
    /**
     * invalidate the route. Delete from routing table and set invalid in odr routing table
     *
     * @param   odrRoute    route to invalidate
     */
    void invalidateRoute(CDPODRRoute *odrRoute);
    /**
     * Add default route to routing table and odr routing table.
     * If in routing table exist other default route and is marked invalid,
     * the route from routing table is deleted and replaced with route from param.
     * If in routing table exist other default route and is not marked invalid,
     * default route from param is just add to odr routing table.
     * If there is no other default route, route from param is add to be default route.
     *
     * @param   ie          outgoing interface
     * @param   nextHope    next hope address
     */
    void addDefaultRoute(InterfaceEntry *ie, const L3Address& nextHop);

    std::string printStats() const;
private:
    /**
     * Validate variables get from module
     */
    void validateVariable();
    /**
     * Set capability vector
     *
     * @param   property    module capability
     */
    void getCapabilities(cProperty *property);
    /**
     * Get position of capability in vector
     *
     * @param   capability  name of capability
     * @return  position
     */
    int capabilitiesPosition(std::string capability);
    /**
     * Check if there exist other routing protocols
     *
     * @return  true if another routing protocol is enabled
     */
    bool hasRoutingProtocol();
    /**
     * If exist any interface (except interface from param) with specified
     * IP address and is not loopback.
     *
     * @param   interfaceId     interface id that will not be checked
     */
    bool ipInterfaceExist(int interfaceId);
    /**
     * Check if interface is supported. Only ethernet and ppp.
     */
    bool isInterfaceSupported(const char *name);
    /**
     * Get ip address from interface or 0 if interface have not specified ip address
     *
     * @param   interfaceId     interface ID
     * @return  ip address
     */
    Ipv4Address ipOnInterface(int interfaceId);
    /**
     * capabilities char convert to string describing each capability with one letter
     *
     * @param   cap     capabilities
     *
     * @return string of capabilities separated by spaces
     */
    std::string capabilitiesConvert(char cap1, char cap2, char cap3, char cap4);
    /**
     * Get interface entry by port number
     *
     * @param   portNum     port number
     *
     * @return  interface entry
     */
    InterfaceEntry *getPortInterfaceEntry(unsigned int gateId);
    /**
     * Process all prefixes from update message
     *
     * @param   msg             message
     * @param   tlvPosition     positioin of prefix tlv in message
     * @param   neighbour       from which cdp neighbour update came
     */
    void processPrefixes(CDPUpdate *msg, int tlvPosition, CDPNeighbour *neighbour);

    /**
     * Activate interface. Start update timer.
     */
    void activateInterface(InterfaceEntry *interface);
    /**
     * Deactivate interface. Purge all routes learned from this interface.
     * Neighbours learned from this interface are still in neighbour table (until holdtime expire)
     *
     * @param   interface
     * @param   removeFromTable     if remove from interface table
     */
    void deactivateInterface(InterfaceEntry *interface, bool removeFromTable);

    /**
     * Add route to routing table
     *
     * @param   dest            destination
     * @param   prefixLength    prefix length
     * @param   ie              outgoing interface
     * @param   nextHope        next hope address
     *
     * @return  reference to route
     */
    IRoute *addRouteToRT(const L3Address& dest, int prefixLength, const InterfaceEntry *ie, const L3Address& nextHop);
    /**
     * Reschedule invalide and flush timer from specific odr route.
     * Holdtime timer is just cancel if is scheduled
     *
     * @param   odrRoute    odr route which timers should be rescheduled
     */
    void rescheduleODRRouteTimer(CDPODRRoute *odrRoute);

    /////////////////////////////
    //          TLV            //
    /////////////////////////////
    /**
     * Create tlv's for interface
     *
     * @param   msg             message
     * @param   interfaceId     interface id
     */
    void createTlv(CDPUpdate *msg, int interface);
    void setTlvDeviceId(CDPUpdate *msg, int pos);
    void setTlvPortId(CDPUpdate *msg, int pos, int interfaceId);
    void setTlvPlatform(CDPUpdate *msg, int pos);
    void setTlvVersion(CDPUpdate *msg, int pos);
    void setTlvCapabilities(CDPUpdate *msg, int pos);
    void setTlvDuplex(CDPUpdate *msg, int pos, int interfaceId);
    void setTlvIpPrefix(CDPUpdate *msg, int pos, int interfaceId);
    void setTlvAddress(CDPUpdate *msg, int pos, int interfaceId);
    void setTlvNativeVlan(CDPUpdate *msg, int pos, int interfaceId);
    void setTlvVtp(CDPUpdate *msg, int pos, int interfaceId);
};

} /* namespace inet */

#endif /* CDP_H_ */
