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

#include "ansa/linklayer/lldp/LLDPAgentTable.h"

namespace inet {

std::string LLDPAgent::info() const
{
    std::stringstream string;

    string << getIfaceName();
    string << ":";
    if(adminStatus == enabledRxTx)
        string << "RxTx";
    else if(adminStatus == enabledRxOnly)
        string << "Rx";
    else if(adminStatus == enabledTxOnly)
        string << "Tx";
    else if(adminStatus == disabled)
        string << "dis";
    string << " txCredit:" << (uint32_t)txCredit;
    string << " txFast:" << (uint32_t)txFast;
    string << " msgFastTx:" << msgFastTx;
    string << " msgTxHold:" << (uint32_t)msgTxHold;
    string << " msgTxInterval:" << msgTxInterval;
    string << " reinitDelay:" << reinitDelay;
    string << " txFastInit:" << txFastInit;
    string << " txCreditMax:" << (uint32_t)txCreditMax;
    return string.str();
}

LLDPAgent::LLDPAgent(
        InterfaceEntry *iface,
        cModule *cm,
        uint8_t msgFastTxDef,
        uint8_t msgTxHoldDef,
        uint16_t msgTxIntervalDef,
        uint16_t reinitDelayDef,
        uint16_t txFastInitDef,
        uint8_t txCreditMaxDef,
        AS adminStatusDef)
{
    interface = iface;
    containingModule = cm;

    txTTR = new LLDPTimer();
    txTTR->setTimerType(TTR);
    txTTR->setContextPointer(this);
    txShutdownWhile = new LLDPTimer();
    txShutdownWhile->setTimerType(ShutdownWhile);
    txShutdownWhile->setContextPointer(this);

    txCredit = txCreditMaxDef;
    txCreditMax = txCreditMaxDef;
    txFast = 0;
    msgFastTx = msgFastTxDef;
    msgTxHold = msgTxHoldDef;
    msgTxInterval = msgTxIntervalDef;
    reinitDelay = reinitDelayDef;
    txFastInit = txFastInitDef;
    adminStatus = adminStatusDef;

    txTTROwner = dynamic_cast<cSimpleModule *>(txTTR->getOwner());
}

LLDPAgent::~LLDPAgent()
{
    //if is scheduled, get his sender module, otherwise get owner module
    cSimpleModule *owner = dynamic_cast<cSimpleModule *>((txTTR->isScheduled()) ? txTTR->getSenderModule() : txTTR->getOwner());
    if(owner != NULL)
    {// owner is cSimpleModule object -> can call his methods
        owner->cancelAndDelete(txTTR);
        txTTR = NULL;
    }
    //if is scheduled, get his sender module, otherwise get owner module
    owner = dynamic_cast<cSimpleModule *>((txShutdownWhile->isScheduled()) ? txShutdownWhile->getSenderModule() : txShutdownWhile->getOwner());
    if(owner != NULL)
    {// owner is cSimpleModule object -> can call his methods
        owner->cancelAndDelete(txShutdownWhile);
        txShutdownWhile = NULL;
    }
}

/**
 * increment credit value, up to the maximum value specified by the txCreditMax
 * tx_tick state in transmit timer state machine
 */
void LLDPAgent::txAddCredit()
{
    if(txCredit < txCreditMax)
        txCredit++;
}

/**
 * decrement by 1 value specified in parameter
 *
 * @param   var     variable
 */
void LLDPAgent::dec(uint16_t var)
{
    if(var > 0)
        var--;
}

void LLDPAgent::startAgent()
{
    if(adminStatus == enabledRxTx || adminStatus == enabledTxOnly)
        txSchedule();
}

void LLDPAgent::txSchedule()
{
    if(adminStatus != enabledRxTx && adminStatus != enabledTxOnly)
        return;

    uint8_t delay;
    if(txFast > 0)
        delay = msgFastTx;
    else
        delay = msgTxInterval;

    txTTROwner->scheduleAt(simTime() + delay, txTTR);
}

/**
 * Create update frame
 *
 * @return  created update frame
 */
LLDPUpdate *LLDPAgent::constrUpdateLLDPDU()
{
    int txTTL = std::min(65535, (msgTxInterval * msgTxHold)+1);

    //TODO:
    return nullptr;
}

/**
 * Create update frame
 *
 * @return  created update frame
 */
LLDPUpdate *LLDPAgent::constrShutdownLLDPDU()
{
    LLDPUpdate *update = new LLDPUpdate();
    int txTTL = 0;
    int count = 0;
    update->setTlvArraySize(4);

    setTlvChassisId(update, count++);      //chassis-id
    setTlvPortId(update, count++);         //port-id
    setTlvTtl(update, count++, txTTL);       //ttl

    //TODO:
    return update;
}

/**
 * Send update
 *
 * @param   update  update frame to be send
 */
void LLDPAgent::txFrame(LLDPUpdate *update)
{
    if(!interface->isUp() || adminStatus == disabled)
    {// trying send message on DOWN iface -> cancel
        EV_ERROR << "Trying send message on DOWN or PASSIVE interface - sending canceled" << endl;
        delete update;
        return;
    }


    if (!containingModule->gate(interface->getNodeOutputGateId())->isPathOK())
    {
        EV_ERROR << "Interface not connected, discarding packet\n";
        delete update;
        return;
    }

    //TODO: send
    Ieee802Ctrl *controlInfo = new Ieee802Ctrl();
    controlInfo->setDestinationAddress(MACAddress("01-80-c2-00-00-0e"));
    controlInfo->setInterfaceId(interface->getInterfaceId());
    update->setControlInfo(controlInfo);

    EV << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
    txTTROwner->send(update, "ifOut", interface->getNetworkLayerGateIndex());
}

/**
 * Create update frame, send it and decrement credit
 */
void LLDPAgent::txInfoFrame()
{
    if(txCredit > 0)
    {
        //LLDPUpdate *update = mibConstrInfoLLDPDU();
        LLDPUpdate *update = constrShutdownLLDPDU();
        txFrame(update);
        dec(txCredit);
        dec(txFast);

        txSchedule();
    }
    else
    {
        //TODO: jakove zpozdeni?
    }
}

/**
 * Create shutdown frame and send it
 */
void LLDPAgent::txShutdownFrame()
{
    LLDPUpdate *update = constrUpdateLLDPDU();
    txFrame(update);
    //TODO: reinit
}

void LLDPAgent::txFastStart()
{
    if(txFast == 0)
        txFast = txFastInit;

    txInfoFrame();
}



///************************* START OF TLV *****************************///


void LLDPAgent::setTlvChassisId(LLDPUpdate *update, int pos)
{
    LLDPTLV *tlv = new LLDPTLV();
    tlv->setType(LLDPTLV_CHASSIS_ID);
    tlv->setValue("chassis");
    tlv->setLength(5);
    update->setTlv(pos, tlv[0]);
}

void LLDPAgent::setTlvPortId(LLDPUpdate *update, int pos)
{
    LLDPTLV *tlv = new LLDPTLV();
    tlv->setType(LLDPTLV_PORT_ID);
    tlv->setValue("port");
    tlv->setLength(4);
    update->setTlv(pos, tlv[0]);
}

void LLDPAgent::setTlvTtl(LLDPUpdate *update, int pos, int ttl)
{
    LLDPTLV *tlv = new LLDPTLV();
    tlv->setType(LLDPTLV_TTL);
    tlv->setValue("0");
    tlv->setLength(5);
    update->setTlv(pos, tlv[0]);
}

///************************** END OF TLV ******************************///


///************************ LLDP AGENT TABLE ****************************///

LLDPAgent * LLDPAgentTable::findAgentById(const int ifaceId)
{
    std::vector<LLDPAgent *>::iterator it;

    for (it = agents.begin(); it != agents.end(); ++it)
    {// through all agents search for same interfaceId
        if((*it)->getInterfaceId() == ifaceId)
        {// found same
            return (*it);
        }
    }

    return NULL;
}


void LLDPAgentTable::startAgents()
{
    std::vector<LLDPAgent *>::iterator it;

    for (it = agents.begin(); it != agents.end(); ++it)
    {// through all agents
        (*it)->startAgent();
    }
}


LLDPAgent * LLDPAgentTable::addAgent(LLDPAgent * iface)
{
    if(findAgentById(iface->getInterfaceId()) != NULL)
    {// agent already in table
        throw cRuntimeError("Adding to LLDPAgentTable interface, which is already in it - id %d", iface);
    }

    agents.push_back(iface);

    return iface;
}

/**
 * Remove agent
 *
 * @param   agent   agent to delete
 *
 */
void LLDPAgentTable::removeAgent(LLDPAgent * agent)
{
    std::vector<LLDPAgent *>::iterator it;

    for (it = agents.begin(); it != agents.end();)
    {// through all interfaces
        if((*it) == agent)
        {// found same
            delete (*it);
            it = agents.erase(it);
            return;
        }
        else
        {// do not delete -> get next
            ++it;
        }
    }
}

/**
 * Removes agent
 *
 * @param   ifaceId interface ID of agent to delete
 */
void LLDPAgentTable::removeAgent(int ifaceId)
{
    std::vector<LLDPAgent *>::iterator it;

    for (it = agents.begin(); it != agents.end();)
    {// through all agents
        if((*it)->getInterfaceId() == ifaceId)
        {// found same
            delete (*it);
            it = agents.erase(it);
            return;
        }        else
        {// do not delete -> get next
            ++it;
        }
    }
}

LLDPAgentTable::~LLDPAgentTable()
{
    std::vector<LLDPAgent *>::iterator it;

    for (it = agents.begin(); it != agents.end(); ++it)
    {// through all agents
        delete (*it);
    }
    agents.clear();
}


std::string LLDPAgentTable::printStats()
{
    std::stringstream string;
    std::vector<LLDPAgent *>::iterator it;

    for (it = agents.begin(); it != agents.end(); ++it)
    {// through all agents
        if((*it)->getAdminStatus() != AS::disabled)
        {
            string << (*it)->getIfaceName() << " interface statistics:" << endl;
/*
            string << "Received " << (*it)->rxStat.str();
            if((*it)->rxStat.tlv[tlvT::UPDATE].getCount() > 0) string << "Update avg. size: " << ((*it)->rxStat.tlv[tlvT::ROUTERID].getSum() + (*it)->rxStat.tlv[tlvT::NEXTHOP].getSum() + (*it)->rxStat.tlv[tlvT::UPDATE].getSum()) / static_cast<double>((*it)->rxStat.tlv[tlvT::UPDATE].getCount()) << " B/TLV" << endl;

            string << endl << "Transmitted " << (*it)->txStat.str();
            if((*it)->txStat.tlv[tlvT::UPDATE].getCount() > 0) string << "Update avg. size: " << ((*it)->txStat.tlv[tlvT::ROUTERID].getSum() + (*it)->txStat.tlv[tlvT::NEXTHOP].getSum() + (*it)->txStat.tlv[tlvT::UPDATE].getSum()) / static_cast<double>((*it)->txStat.tlv[tlvT::UPDATE].getCount()) << " B/TLV" << endl;
            */
        }
    }
    return string.str();
}


} /* namespace inet */
