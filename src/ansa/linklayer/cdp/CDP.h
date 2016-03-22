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

#ifndef CDP_H_
#define CDP_H_

#include <omnetpp.h>

#include "inet/common/INETDefs.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/networklayer/ipv4/IPv4RoutingTable.h"
#include "inet/networklayer/contract/IRoute.h"
#include "inet/common/serializer/TCPIPchecksum.h"

#include "ansa/linklayer/cdp/CDPUpdate_m.h"
#include "ansa/linklayer/cdp/CDPTimer_m.h"
#include "ansa/linklayer/cdp/CDPTableEntry.h"
#include "ansa/linklayer/cdp/CDPInterface.h"

#include "inet/common/serializer/SerializerBase.h"
#include "inet/common/serializer/Buffer.h"



namespace inet {


struct ODRRoute : public cObject
{
    private:
      IRoute *route = nullptr;    // the route in the host routing table that is associated with this route, may be nullptr if deleted
      L3Address dest;    // destination of the route
      int prefixLength = 0;    // prefix length of the destination
      L3Address nextHop;    // next hop of the route
      InterfaceEntry *ie = nullptr;    // outgoing interface of the route
      L3Address from;    // only for RTE routes
      int ad = 160;    // the metric of this route, or infinite (16) if invalid
      uint16 tag = 0;    // route tag, only for REDISTRIBUTE routes
      bool changed = false;    // true if the route has changed since the update
      simtime_t lastUpdateTime;    // time of the last change, only for RTE routes

    public:
      ODRRoute(IRoute *route, uint16 tag);

      IRoute *getRoute() const { return route; }
      L3Address getDestination() const { return dest; }
      int getPrefixLength() const { return prefixLength; }
      L3Address getNextHop() const { return nextHop; }
      InterfaceEntry *getInterface() const { return ie; }
      L3Address getFrom() const { return from; }
      bool isChanged() const { return changed; }
      simtime_t getLastUpdateTime() const { return lastUpdateTime; }

      void setRoute(IRoute *route) { this->route = route; }
      void setDestination(const L3Address& dest) { this->dest = dest; }
      void setPrefixLength(int prefixLength) { this->prefixLength = prefixLength; }
      void setNextHop(const L3Address& nextHop) { this->nextHop = nextHop; route->setNextHop(nextHop); }
      void setInterface(InterfaceEntry *ie) { this->ie = ie; route->setInterface(ie); }
      void setRouteTag(uint16 routeTag) { this->tag = routeTag; }
      void setFrom(const L3Address& from) { this->from = from; }
      void setChanged(bool changed) { this->changed = changed; }
      void setLastUpdateTime(simtime_t time) { lastUpdateTime = time; }
};

class CDP: public cSimpleModule, public ILifecycle, public cListener {
public:
    CDP();
    virtual ~CDP();
    void scheduleALL();
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG) override;
    void stop();
    void deleteTimers();

protected:
    simtime_t updateTime;
    int holdTime;
    std::string hostName;
    int version;
    IInterfaceTable *ift;
    cModule *containingModule;
    bool sendIpPrefixes;

    bool isOperational = false;    // for lifecycle
    std::vector<CDPTableEntry*> table;
    std::map <int, CDPTimer *> interfaceUpdateTimer;      //interface update timer
    CDPInterfaceTable cdpInterfaceTable;
    bool odr;
    IPv4RoutingTable *rt4 = nullptr;
    char cap[4];

    simtime_t routeFlushTime;    // time between regular updates
    simtime_t routeHolddowndTime;    // learned routes becomes invalid if no update received in this period of time
    simtime_t routeInvalidTime;    // invalid routes are deleted after this period of time is elapsed
    //simtime_t shutdownTime;    // time of shutdown processing

    IRoutingTable *rt = nullptr;    // routing table from which routes are imported and to which learned routes are added
    typedef std::vector<ODRRoute *> RouteVector;
    RouteVector odrRoutes;    // all advertised routes (imported or learned)*/


    virtual void initialize(int stage);
    void getCapabilities(cProperty *property);
    int capabilitiesPosition(std::string capability);
    bool hasRoutingProtocol();
    bool parseIPAddress(const char *text, unsigned char tobytes[]);
    bool ipInterfaceExist(int interfaceId);
    uint16_t checksum(CDPUpdate *msg);
    void activateInterface(InterfaceEntry *interface);
    void deactivateInterface(InterfaceEntry *interface);
    bool isInterfaceSupported(const char *name);

    InterfaceEntry *getPortInterfaceEntry(unsigned int gateId);

    //handling with messages
    virtual void handleMessage(cMessage *msg);
    void handleTimer(CDPTimer *msg);
    void handleUpdate(CDPUpdate *msg);
    void sendUpdate(int interface);

    //neighbour table
    CDPTableEntry *newTableEntry(CDPUpdate *msg);
    void updateTableEntry(CDPUpdate *msg, CDPTableEntry *entry);
    CDPTableEntry *findEntryInTable(std::string name, int port);
    void deleteEntryInTable(CDPTableEntry *entry);

    //TLV
    uint16_t getTlvSize(CDPTLV *tlv);
    void createTlv(CDPUpdate *msg, int interface);
    void setTlvDeviceId(CDPUpdate *msg, int pos);
    void setTlvPortId(CDPUpdate *msg, int pos, int interfaceId);
    void setTlvPlatform(CDPUpdate *msg, int pos);
    void setTlvVersion(CDPUpdate *msg, int pos);
    void setTlvCapabilities(CDPUpdate *msg, int pos);
    void setTlvDuplex(CDPUpdate *msg, int pos, int interfaceId);
    void setTlvODR(CDPUpdate *msg, int pos, int interfaceId);
    void setTlvIpPrefix(CDPUpdate *msg, int pos, int interfaceId);

private:
    IRoute *addRoute(const L3Address& dest, int prefixLength, const InterfaceEntry *ie, const L3Address& nextHop, int metric);
    void deleteRoute(IRoute *route);
    virtual void invalidateRoutes(const InterfaceEntry *ie);

protected:
    virtual void addRoute(const L3Address& dest, int prefixLength, const InterfaceEntry *ie, const L3Address& nextHop, int metric, uint16 routeTag, const L3Address& from);
    //virtual void triggerUpdate();
    virtual ODRRoute *checkRouteIsExpired(ODRRoute *route);
    virtual void invalidateRoute(ODRRoute *route);
    virtual void purgeRoute(ODRRoute *route);

};

} /* namespace inet */

#endif /* CDP_H_ */
