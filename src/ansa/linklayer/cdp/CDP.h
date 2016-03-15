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

#include "ansa/linklayer/cdp/CDPUpdate_m.h"
#include "ansa/linklayer/cdp/CDPTimer_m.h"
#include "ansa/linklayer/cdp/CDPTableEntry.h"


namespace inet {

class CDP: public cSimpleModule, public ILifecycle, public cListener {
public:
    CDP();
    virtual ~CDP();
    void scheduleALL();
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG) override;
    void stop();

protected:
    simtime_t updateTime;
    int holdTime;
    std::string hostName;
    int version;
    std::vector<std::string> capabilities;
    IInterfaceTable *ift;
    cModule *containingModule;

    bool isOperational = false;    // for lifecycle
    std::vector<CDPTableEntry*> table;
    std::map <int, CDPTimer *> interfaceUpdateTimer;      //interface update timer
    std::vector<int> odrSpokePropagationInt;
    bool odr;

    virtual void initialize(int stage);
    void getCapabilities(cProperty *property);
    int capabilitiesPosition(std::string capability);
    void deleteInterfacesUpdateTimer();
    bool hasRoutingProtocol();

    //handling with messages
    virtual void handleMessage(cMessage *msg);
    void handleTimer(CDPTimer *msg);
    void handleUpdate(CDPUpdate *msg);
    void sendUpdate(int interface);

    //neighbour table
    CDPTableEntry *newTableEntry(CDPUpdate *msg);
    CDPTableEntry *findEntryInTable(std::string name, int port);
    void deleteEntryInTable(CDPTableEntry *entry);

    //TLV
    uint16_t getTlvSize(CDPTLV *tlv);
    void createTlv(CDPUpdate *msg, int interface);
    void setTlvDeviceId(CDPUpdate *msg, int pos);
    void setTlvPortId(CDPUpdate *msg, int pos, int index);
    void setTlvVersion(CDPUpdate *msg, int pos);
    void setTlvCapabilities(CDPUpdate *msg, int pos);
    void setTlvDuplex(CDPUpdate *msg, int pos, int index);
    void setTlvODR(CDPUpdate *msg, int pos, int index);
    void setTlvIpPrefix(CDPUpdate *msg, int pos);
};

} /* namespace inet */

#endif /* CDP_H_ */
