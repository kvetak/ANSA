//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
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
* @file LLDPMain.h
* @author Tomas Rajca
* @author Vladimir Vesely (ivesely@fit.vutbr.cz)
* @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
*/

#ifndef __LLDPMAIN_H_
#define __LLDPMAIN_H_

#include <omnetpp.h>

#include "inet/common/INETDefs.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/ILifecycle.h"

#include "ansa/linklayer/lldp/LLDPTimer_m.h"
#include "ansa/linklayer/lldp/tables/LLDPAgentTable.h"
#include "ansa/linklayer/lldp/tables/LLDPNeighbourTable.h"

//#define CREDIT            // uncomment to enable credit system

namespace inet {

class INET_API LLDPMain: public cSimpleModule, protected cListener, public ILifecycle
{
  protected:
    std::string hostName;       // name of the module
    cModule *containingModule;
    IInterfaceTable *ift;        // interface table
    LLDPAgentTable *lat;         // LLDP agent table
    LLDPNeighbourTable *lnt;     // LLDP neighbour table

    char enCap[2];              // system capabilities
    char sysCap[2];             // enabled capabilities
    bool isOperational = false; // for lifecycle
    LLDPTimer *tickTimer;

    uint16_t msgFastTxDef;     // ticks between transmission during fast transmission
    uint8_t msgTxHoldDef;      // multiplier of msgTxInterval, to determine value of TTL
    uint16_t msgTxIntervalDef; // ticks between transmission during normal transmission periods
    uint16_t reinitDelayDef;   // delay from when adminStatus becomes disabled until reinitialization is attempted
    uint16_t txFastInitDef;    // determines the number of LLDPDUs that are transmitted during a fast transmission period
    uint8_t txCreditMaxDef;    // the maximum value of txCredit
    AS adminStatusDef;

    std::string chassisId;      // device chassis id

    virtual ~LLDPMain();
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void finish() override;

    /*
     * Start LLDP process on device
     */
    void startLLDP();
    /*
     * Stop LLDP process on device
     */
    void stopLLDP();

    /**
     * Handle timers (self-messages)
     *
     * @param   msg     message
     */
    void processTimer(LLDPTimer *timer);
    /**
     * Handle received update
     * Frame is validated and send to appropriate agent
     *
     * @param   msg     message
     */
    void handleUpdate(LLDPUpdate *msg);

    /*
     * Validate LLDP frame. Returns false if frame is invalid
     */
    bool frameValidation(LLDPUpdate *msg, LLDPAgent *agent);

    /*
     * Return string name of TLV type
     */
    std::string getNameOfTlv(short type);


  private:
    /**
     * Check if interface is supported. Only ethernet and ppp.
     */
    bool isInterfaceSupported(const char *name);
    AS getAdminStatusFromString(std::string par);
    void createAgent(InterfaceEntry *interface);
    /**
     * Return mac address from first interface in ift with set mac address
     *
     * @return  chassis ID
     */
    std::string generateChassisId();
    /**
     * Set capability vector
     *
     * @param   property    module capability
     */
    void getCapabilities(cProperty *propSysCap, cProperty *propEnCap);
    /**
     * Get position of capability in vector
     *
     * @param   capability  name of capability
     * @return  position
     */
    int capabilitiesPosition(std::string capability);

  public:
    //getters
    LLDPAgentTable* getLat() {return lat;}
    LLDPNeighbourTable* getLnt() {return lnt;}
    std::string getChassisId() {return chassisId;}
    cModule *getContainingModule() {return containingModule;}
    const char *getSystemCapabilites() {return sysCap;}
    const char *getEnabledCapabilites() {return enCap;}
};

} //namespace

#endif
