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
    int headerErr;        // the number of CDP advertisements with bad headers

    std::string info() const;
};

class INET_API CDP: public cSimpleModule, public ILifecycle, public cListener {
public:
    CDP();
    virtual ~CDP();

    CDPInterfaceTable *getCit() {return &cit;};
    void setRouteInvalidTime(simtime_t t) {this->routeInvalidTime = t;}
    void setRouteHolddownTime(simtime_t t) {this->routeHolddownTime = t;}
    void setRouteFlushTime(simtime_t t) {this->routeFlushTime = t;}

protected:
    int version;                // cdp version
    IInterfaceTable *ift;       // interface table
    cModule *containingModule;

    bool isOperational = false;    // for lifecycle
    CDPNeighbourTable cnt;          // cdp neighbour table
    CDPInterfaceTable cit;          // cdp interface table
    CDPODRRouteTable ort;           // cdp odr route table
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

    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void finish() override;

    void startCDP();
    void stopCDP();

    //handling with messages
    void handleTimer(CDPTimer *msg);
    void handleUpdate(CDPUpdate *msg);
    void sendUpdate(int interface, bool shutDown);
    void neighbourUpdate(CDPUpdate *msg, CDPNeighbour *neighbour);

    void addRoute(const L3Address& dest, int prefixLength, const InterfaceEntry *ie, const L3Address& nextHop);
    void purgeRoute(CDPODRRoute *odrRoute);
    void invalidateRoute(CDPODRRoute *odrRoute);
    void addDefaultRoute(InterfaceEntry *ie, const L3Address& nextHop);

    std::string printStats() const;
private:
    void validateVariable();
    void getCapabilities(cProperty *property);
    int capabilitiesPosition(std::string capability);
    bool hasRoutingProtocol();
    bool ipInterfaceExist(int interfaceId);
    bool isInterfaceSupported(const char *name);
    int ipOnInterface(int interfaceId);
    std::string capabilitiesConvert(char cap1, char cap2, char cap3, char cap4);
    InterfaceEntry *getPortInterfaceEntry(unsigned int gateId);
    void processPrefixes(CDPUpdate *msg, int tlvPosition, CDPNeighbour *neighbour);

    void activateInterface(InterfaceEntry *interface);
    void deactivateInterface(InterfaceEntry *interface, bool removeFromTable);

    IRoute *addRouteToRT(const L3Address& dest, int prefixLength, const InterfaceEntry *ie, const L3Address& nextHop);
    void rescheduleODRRouteTimer(CDPODRRoute *odrRoute);
    CDPODRRoute *existOtherDefaultRoute(CDPODRRoute *odrRoute);

    //TLV
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
