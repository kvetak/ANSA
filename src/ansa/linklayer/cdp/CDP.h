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

struct AddressTLV
{
    char protocolType;
    char protocolLength;
    char protocol;
    uint16_t length;
    uint32_t address;
};

struct ODRRoute : public cObject
{
    private:
      IRoute *route = nullptr;    // the route in the host routing table that is associated with this route, may be nullptr if deleted
      L3Address dest;    // destination of the route
      int prefixLength = 0;    // prefix length of the destination
      L3Address nextHop;    // next hop of the route
      InterfaceEntry *ie = nullptr;    // outgoing interface of the route
      bool invalide = false;    // true if the route has changed since the update
      bool noUpdates = false;    // true if the route has changed since the update
      simtime_t lastUpdateTime;    // time of the last change, only for RTE routes
      CDPTimer *ODRInvalideTime;
      CDPTimer *ODRHoldTime;
      CDPTimer *ODRFlush;

    public:
      ODRRoute(IRoute *route);
      ODRRoute(L3Address des, int pre, L3Address nex, InterfaceEntry *i);
      virtual std::string info() const override;

      IRoute *getRoute() const { return route; }
      L3Address getDestination() const { return dest; }
      int getPrefixLength() const { return prefixLength; }
      L3Address getNextHop() const { return nextHop; }
      InterfaceEntry *getInterface() const { return ie; }
      bool isInvalide() const { return invalide; }
      bool isNoUpdates() const { return noUpdates; }
      simtime_t getLastUpdateTime() const { return lastUpdateTime; }
      CDPTimer *getODRInvalideTime() const { return ODRInvalideTime; }
      CDPTimer *getODRHoldTime() const { return ODRHoldTime; }
      CDPTimer *getODRFlush() const { return ODRFlush; }

      void setRoute(IRoute *route) { this->route = route; }
      void setDestination(const L3Address& dest) { this->dest = dest; }
      void setPrefixLength(int prefixLength) { this->prefixLength = prefixLength; }
      void setNextHop(const L3Address& nextHop) { this->nextHop = nextHop; route->setNextHop(nextHop); }
      void setInterface(InterfaceEntry *ie) { this->ie = ie; route->setInterface(ie); }
      void setInvalide(bool invalide) { this->invalide = invalide; }
      void setNoUpdates(bool noUpdates) { this->noUpdates = noUpdates; }
      void setLastUpdateTime(simtime_t time) { lastUpdateTime = time; }
      void setODRInvalideTime(CDPTimer *time) { ODRInvalideTime = time; }
      void setODRHoldTime(CDPTimer *time) { ODRHoldTime = time; }
      void setODRFlush(CDPTimer *time) { ODRFlush = time; }
};

class INET_API CDP: public cSimpleModule, public ILifecycle, public cListener {
public:
    CDP();
    virtual ~CDP();

protected:
    std::string hostName;       // name of the module
    int version;                // cdp version
    IInterfaceTable *ift;       // interface table
    cModule *containingModule;

    bool isOperational = false;             // for lifecycle
    typedef std::vector<CDPTableEntry *> CDPTableEntryVector;
    CDPTableEntryVector neighbourTable;
    CDPInterfaceTable cdpInterfaceTable;
    char cap[4];

    simtime_t holdTime;             // cdp entry is deleted after this period of time is elapsed
    simtime_t updateTime;           // time between regular updates
    simtime_t routeFlushTime;       // routes are deleted after this period of time is elapsed
    simtime_t routeHolddownTime;    // how long routes will not be updated
    simtime_t routeInvalidTime;     // learned routes becomes invalid if no update received in this period of time
    simtime_t defaultRouteInvalide; // default routes are deleted after this period of time is elapsed

    IRoutingTable *rt = nullptr;    // routing table from which routes are imported and to which learned routes are added
    typedef std::vector<ODRRoute *> RouteVector;
    RouteVector odrRoutes;          // all learned routes
    int maxDestinationPaths;        // max number of the same routes in routing table
    bool sendIpPrefixes;            // capability of sending ip prefixes
    bool odr;                       // odr routing

    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG) override;

    virtual void startCDP();
    virtual void stopCDP();

    //handling with messages
    virtual void handleTimer(CDPTimer *msg);
    virtual void handleUpdate(CDPUpdate *msg);
    virtual void sendUpdate(int interface, bool shutDown);
    virtual void entryUpdate(CDPUpdate *msg, CDPTableEntry *entry);


    virtual void addRoute(const L3Address& dest, int prefixLength, const InterfaceEntry *ie, const L3Address& nextHop);
    virtual void purgeRoute(ODRRoute *route);
    virtual void invalidateRoute(ODRRoute *odrRoute);
    virtual void addDefaultRoute(InterfaceEntry *ie, const L3Address& nextHop);

private:
    void getCapabilities(cProperty *property);
    int capabilitiesPosition(std::string capability);
    bool hasRoutingProtocol();
    bool ipInterfaceExist(int interfaceId);
    uint16_t countChecksum(CDPUpdate *msg);
    bool isInterfaceSupported(const char *name);
    int ipOnInterface(int interfaceId);
    std::string capabilitiesConvert(const char *cap);
    InterfaceEntry *getPortInterfaceEntry(unsigned int gateId);
    void processPrefixes(CDPUpdate *msg, int tlvPosition, CDPTableEntry *entry);

    void activateInterface(InterfaceEntry *interface);
    void deactivateInterface(InterfaceEntry *interface);

    void deleteTimers();
    IRoute *addRouteToRT(const L3Address& dest, int prefixLength, const InterfaceEntry *ie, const L3Address& nextHop);
    void rescheduleODRRouteTimer(ODRRoute *odrRoute);
    ODRRoute *findRoute(const L3Address& destination, int prefixLength, const L3Address& ie);
    int countDestinationPaths(const L3Address& destination, int prefixLength);
    ODRRoute *findRoute(const IRoute *route);
    ODRRoute *existOtherDefaultRoute(ODRRoute *odrRoute);

    //neighbour table
    CDPTableEntry *findEntryInTable(std::string name, int port);
    void deleteEntryInTable(CDPTableEntry *entry);

    //TLV
    CDPTLV *getTlv(CDPUpdate *msg, CDPTLVType type);
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
    void setTlvAddress(CDPUpdate *msg, int pos, int interfaceId);
};

} /* namespace inet */

#endif /* CDP_H_ */
