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

#ifndef LLDPAGENT_H_
#define LLDPAGENT_H_

#include <omnetpp.h>
#include "inet/common/INETDefs.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/common/ModuleAccess.h"

//#include "ansa/linklayer/lldp/LLDP.h"
#include "ansa/linklayer/lldp/LLDPTimer_m.h"
#include "ansa/linklayer/lldp/LLDPUpdate.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "ansa/linklayer/lldp/LLDPDef.h"


#include "inet/networklayer/contract/IRoute.h"
#include "inet/common/NotifierConsts.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/NotifierConsts.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/networklayer/contract/IRoutingTable.h"

namespace inet {

class LLDP;

class INET_API LLDPAgent: public cObject
{
protected:
    InterfaceEntry *interface;      // physical network interface
    LLDPTimer *txTTR;               // timer is used to determine the next LLDPDU transmission is due
    LLDPTimer *txShutdownWhile;     // remaining until LLDP re-initialization can occur
    //LLDPSystemInfo *systemInfo;
    cModule *containingModule;
    LLDP *core;
    const char* msap = nullptr;     // MSAP (contatenation of chassis ID and port ID)


    uint16_t msgFastTx;     // ticks between transmission during fast transmission
    uint8_t msgTxHold;      // multiplier of msgTxInterval, to determine value of TTL
    uint16_t msgTxInterval; // ticks between transmission during normal transmission periods
    uint16_t reinitDelay;   // delay from when adminStatus becomes ‘disabled’ until reinitialization is attempted
    uint8_t txFast;         // down counter of transmissions to be made during a fast transmission period
    uint16_t txFastInit;    // determines the number of LLDPDUs that are transmitted during a fast transmission period
    uint8_t txCreditMax;    // the maximum value of txCredit
    uint8_t txCredit;               // number of consecutive LLDPDUs that can be transmitted at any time
    AS adminStatus;  // indicates whether or not the LLDP agent is enabled

    cSimpleModule *txTTROwner;

public:
    LLDPAgent(
            InterfaceEntry *iface,
            LLDP *c,
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

    int getInterfaceId() {return (interface) ? interface->getInterfaceId() : -1;}
    const char *getIfaceName() const {return (interface) ? interface->getName() : "-";}
    const char *getMsap() {return msap;}

    InterfaceEntry *getInterface() {return interface;}
    void setInterface(InterfaceEntry *i) {interface = i;}

    LLDPTimer* getTxTTRTimer() {return txTTR;}
    LLDPTimer* getTxShutdownWhile() {return txShutdownWhile;}
    AS getAdminStatus() {return adminStatus;}

    void setTxCredit(uint8_t t) {txCredit = t;}
    void setTxCreditMax(uint8_t t) {txCreditMax = t;}
    void setMsgFastTx(uint16_t m) {msgFastTx = m;}
    void setMsgTxHold(uint8_t m) {msgTxHold = m;}
    void setMsgTxInterval(uint16_t m) {msgTxInterval = m;}
    void setReinitDelay(uint16_t r) {reinitDelay = r;}
    void setTxFastInit(uint16_t t) {txFastInit = t;}
    void setAdminStatus(AS a) {adminStatus = a;}

    void txAddCredit();                     //nechat
    void txInitializeLLDP();                //
    LLDPUpdate *constrUpdateLLDPDU();      //nechat
    LLDPUpdate *constrShutdownLLDPDU();  //nechat
    void txFrame(LLDPUpdate *update);       //pres
    void txSchedule();                      //nechar a prejmenovat
    void dec(uint16_t txCredit);            //nechat
    void startAgent();

    void txInfoFrame();
    void txShutdownFrame();
    void txFastStart();


    void setTlvChassisId(LLDPUpdate *msg);
    void setTlvPortId(LLDPUpdate *msg);
    void setTlvTtl(LLDPUpdate *msg);
    void setTlvEndOf(LLDPUpdate *msg);
    void setTlvPortDes(LLDPUpdate *msg);
    void setTlvSystemName(LLDPUpdate *msg);
    void setTlvSystemDes(LLDPUpdate *msg);
    void setTlvCap(LLDPUpdate *msg);
    void setTlvManAdd(LLDPUpdate *msg);
    void setTlvManAddSpec(LLDPUpdate *msg, std::string add);
    void setTlvOrgSpec(LLDPUpdate *msg, LLDPOptionOrgSpec *tlv);

};

class LLDPAgentTable
{
  protected:
    std::vector<LLDPAgent *> agents;

  public:
    virtual ~LLDPAgentTable();

    std::vector<LLDPAgent *>& getAgents() {return agents;}
    LLDPAgent * findAgentById(const int ifaceId);
    LLDPAgent * findAgentByMsap(const char* m);
    LLDPAgent * addAgent(LLDPAgent * agent);
    void removeAgent(LLDPAgent * agent);
    void removeAgent(int ifaceId);
    std::string printStats();
    void startAgents();

};
} /* namespace inet */

#endif /* LLDPAGENT_H_ */
