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
* @file LLDPAgentTable.h
* @author Tomas Rajca
*/

#ifndef LLDPAGENT_H_
#define LLDPAGENT_H_

#include <omnetpp.h>
#include "inet/common/INETDefs.h"

#include "ansa/linklayer/lldp/LLDPTimer_m.h"
#include "ansa/linklayer/lldp/LLDPUpdate.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"

#define OUI_IEEE_802_3 4623

namespace inet {
class LLDPMain;
class LLDPNeighbourTable;

/**
 * Structure holding statistical information in each LLDP agent.
 */
struct LLDPStatistics
{
    uint32_t ageoutsTotal = 0;          // how many times neighbor’s information has been deleted
    uint32_t framesDiscardedTotal = 0;  // all LLDPDUs received and then discarded
    uint32_t framesInErrorsTotal = 0;   // LLDPDUs received with one or more detectable errors
    uint32_t framesInTotal = 0;         // LLDP frames received
    uint32_t framesOutTotal = 0;        // LLDP frames transmitted by this instance of the transmit state machine
    uint32_t tlvsDiscardedTotal = 0;    // TLVs received and then discarded for any reason
    uint32_t tlvsUnrecognizedTotal = 0; // TLVs received on the port that are not recognized by the LLDP local
    uint32_t lldpduLengthErrors = 0;    // LLPDU length restriction errors detected

    std::string info() const;
};

enum AS
{
    enabledRxTx = 0,         // is enabled for reception and transmission of LLDPDUs
    enabledTxOnly = 1,       // is enabled for transmission of LLDPDUs only
    enabledRxOnly = 2,       // is enabled for reception of LLDPDUs only
    disabled = 3             // is disabled for both reception and transmission
};


/**
 * Class holding information about a management address TLV.
 */
class LLDPManAdd: public LLDPOptionManAdd
{
        bool updated;

    public:
        bool getUpdated() {return updated;}
        void setUpdated(bool u) {updated = u;}
};

/**
 * Class holding information about a management address TLVs.
 */
class LLDPManAddTab
{
    protected:
        std::vector<LLDPManAdd *> manAddresses;

    public:
        void setAllUnchanged();
        std::vector<LLDPManAdd *> getManAdd() {return manAddresses;}
        LLDPManAdd * findManAdd(uint8_t type);
        void addManAdd(LLDPManAdd *manAdd);
        void removeManAdd(LLDPManAdd *manAdd);
        /*
         * Remove management informations with variable updated=true
         */
        void removeNotUpdated();
};

/**
 * Class holding information about a LLDP agent.
 */
class INET_API LLDPAgent: public cObject
{
    friend class LLDPAgentTable;

protected:
    InterfaceEntry *interface;      // physical network interface
    LLDPTimer *txTTR;               // timer is used to determine the next LLDPDU transmission is due
    LLDPTimer *txShutdownWhile;     // remaining until LLDP re-initialization can occur
    LLDPMain *core;
    LLDPNeighbourTable *lnt;
    bool reinitDelaySet;

    uint16_t msgFastTx;     // ticks between transmission during fast transmission
    uint8_t msgTxHold;      // multiplier of msgTxInterval, to determine value of TTL
    uint16_t msgTxInterval; // ticks between transmission during normal transmission periods
    uint16_t reinitDelay;   // delay from when adminStatus becomes disabled until reinitialization is attempted
    uint8_t txFast;         // down counter of transmissions to be made during a fast transmission period
    uint16_t txFastInit;    // determines the number of LLDPDUs that are transmitted during a fast transmission period
    uint8_t txCreditMax;    // the maximum value of txCredit
    uint8_t txCredit;       // number of consecutive LLDPDUs that can be transmitted at any time
    AS adminStatus;         // indicates whether or not the LLDP agent is enabled

    cSimpleModule *txTTROwner;
    LLDPStatistics st;

    /**
     * Capabilities char convert to string describing each capability
     *
     * @param   cap     capabilities
     * @return string of capabilities separated by spaces
     */
    std::string capabilitiesConvert(char cap1, char cap2);
    void setInterface(InterfaceEntry *i) {interface = i;}

    void setTlvChassisId(LLDPUpdate *msg);
    void setTlvPortId(LLDPUpdate *msg);
    void setTlvTtl(LLDPUpdate *msg, bool shutDown);
    void setTlvEndOf(LLDPUpdate *msg);
    void setTlvPortDes(LLDPUpdate *msg);
    void setTlvSystemName(LLDPUpdate *msg);
    void setTlvSystemDes(LLDPUpdate *msg);
    void setTlvCap(LLDPUpdate *msg);
    void setTlvManAdd(LLDPUpdate *msg);
    void setTlvManAddSpec(LLDPUpdate *msg, std::string add);
    void setTlvOrgSpec(LLDPUpdate *msg, LLDPOptionOrgSpec *tlv);
    void setTlvMtu(LLDPUpdate *msg);

    void txInitializeLLDP();
    /**
     * Create normal LLDPDUs that provide management information about the local station
     * to that station’s neighbors.
     *
     * @return  created LLDPDUs frame
     */
    LLDPUpdate *constrUpdateLLDPDU();
    /**
     * Create a special shutdown advisory LLDPDU indicating that any information about the local
     * station that is maintained in a remote LLDP agent’s remote systems MIB is now invalid and
     * must be discarded.
     *
     * @return  created LLDPDU frame
     */
    LLDPUpdate *constrShutdownLLDPDU();
    /**
     * Send update
     *
     * @param   update  update frame to be send
     */
    void txFrame(LLDPUpdate *update);
    /*
     * Schedule next regular transmit
     *
     * @param   noDelay     if send without delay
     */
    void txSchedule(bool noDelay);
    /**
     * Decrement by 1 variable specified in parameter
     *
     * @param   var     variable
     */
    void dec(uint8_t *var);

public:
    LLDPAgent(
            InterfaceEntry *iface,
            LLDPMain *c,
            uint8_t msgFastTxDef,
            uint8_t msgTxHoldDef,
            uint16_t msgTxIntervalDef,
            uint16_t reinitDelayDef,
            uint16_t txFastInitDef,
            uint8_t txCreditMaxDef,
            AS adminStatusDef);
    virtual ~LLDPAgent();
    virtual std::string info() const;
    virtual std::string detailedInfo() const {return info();}
    friend std::ostream& operator<<(std::ostream& os, const LLDPAgent& i)
    {
        return os << i.info();
    }
    friend std::ostream& operator<<(std::ostream& os, const LLDPStatistics& e)
    {
        return os << e.info();
    }

    /*
     * Update neighbour
     */
    void neighbourUpdate(LLDPUpdate *msg);

    /*
     * Increment credit value, up to the maximum value specified by the txCreditMax
     */
    void txAddCredit();

    /*
     * Agent schedule update transmission
     */
    void startAgent();

    /*
     * Agent cancel scheduled update transmission
     */
    void stopAgent();

    /**
     * Create update frame, send it and decrement credit
     */
    void txInfoFrame();

    /**
     * Create shutdown frame and send it
     */
    void txShutdownFrame();

    /*
     * Start fast start transmition
     */
    void txFastStart();

    // getters
    LLDPStatistics *getSt() {return &st;}
    InterfaceEntry *getInterface() {return interface;}
    int getInterfaceId() {return (interface) ? interface->getInterfaceId() : -1;}
    const char *getIfaceName() const {return (interface) ? interface->getName() : "-";}
    LLDPTimer* getTxTTRTimer() {return txTTR;}
    LLDPTimer* getTxShutdownWhile() {return txShutdownWhile;}
    AS getAdminStatus() {return adminStatus;}
    uint16_t getReinitDelay() {return reinitDelay;}
    bool getReinitDelaySet() {return reinitDelaySet;}

    // setters
    void setTxCredit(uint8_t t) {txCredit = t;}
    void setTxCreditMax(uint8_t t) {txCreditMax = t;}
    void setMsgFastTx(uint16_t m) {msgFastTx = m;}
    void setMsgTxHold(uint8_t m) {msgTxHold = m;}
    void setMsgTxInterval(uint16_t m) {msgTxInterval = m;}
    void setReinitDelay(uint16_t r) {reinitDelay = r;}
    void setTxFastInit(uint16_t t) {txFastInit = t;}
    void setAdminStatus(AS a) {adminStatus = a;}
    void setReinitDelaySet(bool r) {reinitDelaySet = r;}
};

/**
 * Class holding informatation about LLDP agents.
 */
class INET_API LLDPAgentTable : public cSimpleModule
{
  protected:
    std::vector<LLDPAgent *> agents;

    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *) override;

  public:
    virtual ~LLDPAgentTable();

    std::vector<LLDPAgent *>& getAgents() {return agents;}

    /**
     * Returns the agent with the specified interface ID.
     */
    LLDPAgent * findAgentById(const int ifaceId);

    /**
     * Adds the a agent to the table. The operation might fail
     * if in the table is already agent with the same interface ID
     */
    LLDPAgent * addAgent(LLDPAgent * agent);

    /**
     * Removes a agent from the table. If the agent was
     * not found in the table then it is untouched, otherwise deleted.
     */
    void removeAgent(LLDPAgent * agent);

    /**
     * Removes a agent identified by interface ID from the table. If the agent was
     * not found in the table then it is untouched, otherwise deleted.
     */
    void removeAgent(int ifaceId);

    /*
     * Print agent statistics.
     */
    std::string printStats();

    /*
     * Start all agents.
     */
    void startAgents();

};
} /* namespace inet */

#endif /* LLDPAGENT_H_ */
