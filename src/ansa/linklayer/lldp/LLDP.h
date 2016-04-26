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

#ifndef __LLDP_H_
#define __LLDP_H_

#include <omnetpp.h>

#include "inet/common/INETDefs.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/ILifecycle.h"

#include "ansa/linklayer/lldp/LLDPTimer_m.h"
#include "ansa/linklayer/lldp/LLDPDef.h"
#include "ansa/linklayer/lldp/tables/LLDPAgentTable.h"
#include "ansa/linklayer/lldp/tables/LLDPNeighbourTable.h"

namespace inet {

class INET_API LLDP: protected cListener, public ILifecycle, public cSimpleModule
{
  protected:
    std::string hostName;       // name of the module
    IInterfaceTable *ift;       // interface table
    cModule *containingModule;
    LLDPAgentTable lat;         // LLDP agent table
    LLDPNeighbourTable lnt;     // LLDP neighbour table
    char enCap[2];              // system capabilities
    char sysCap[2];             // enabled capabilities

    bool isOperational = false; // for lifecycle

    LLDPTimer *tickTimer;

    uint16_t msgFastTxDef;     // ticks between transmission during fast transmission
    uint8_t msgTxHoldDef;      // multiplier of msgTxInterval, to determine value of TTL
    uint16_t msgTxIntervalDef; // ticks between transmission during normal transmission periods
    uint16_t reinitDelayDef;   // delay from when adminStatus becomes ‘disabled’ until reinitialization is attempted
    uint16_t txFastInitDef;    // determines the number of LLDPDUs that are transmitted during a fast transmission period
    uint8_t txCreditMaxDef;    // the maximum value of txCredit
    AS adminStatusDef;

    std::string chassisId;

    void startLLDP();
    void startAgents();
    void stopLLDP();


    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;
    virtual void processTimer(LLDPTimer *timer);
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    void handleUpdate(LLDPUpdate *msg);
    bool frameValidation(LLDPUpdate *msg);
    std::string getNameOfTlv(short type);


  private:
    bool isInterfaceSupported(const char *name);
    AS getAdminStatusFromString(std::string par);
    void createAgent(InterfaceEntry *interface);
    std::string generateChassisId();
    void getCapabilities(cProperty *propSysCap, cProperty *propEnCap);
    int capabilitiesPosition(std::string capability);

  public:
    LLDPAgentTable* getLat() {return &lat;}
    LLDPNeighbourTable* getLnt() {return &lnt;}
    std::string getChassisId() {return chassisId;}
    cModule *getContainingModule() {return containingModule;}
    const char *getSystemCapabilites() {return sysCap;}
    const char *getEnabledCapabilites() {return enCap;}
};

} //namespace

#endif
