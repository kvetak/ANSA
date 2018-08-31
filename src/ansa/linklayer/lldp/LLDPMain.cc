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
* @file LLDPMain.cc
* @author Tomas Rajca
* @author Vladimir Vesely (ivesely@fit.vutbr.cz)
* @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
*/

#include "ansa/linklayer/lldp/LLDPMain.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"

#include "ansa/linklayer/lldp/LLDPDeviceConfigurator.h"

namespace inet {

Define_Module(LLDPMain);

LLDPMain::~LLDPMain()
{
    containingModule->unsubscribe(interfaceStateChangedSignal, this);
    containingModule->unsubscribe(interfaceCreatedSignal, this);
    containingModule->unsubscribe(interfaceDeletedSignal, this);

    cancelAndDelete(tickTimer);
}

void LLDPMain::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        containingModule = findContainingNode(this);
        if (!containingModule)
            throw cRuntimeError("Containing module not found");

        getCapabilities(containingModule->getProperties()->get("capabilities"), containingModule->getProperties()->get("capabilitiesEnabled"));

        msgFastTxDef = par("msgFastTx");
        msgTxHoldDef = par("msgTxHold");
        msgTxIntervalDef = par("msgTxInterval");
        reinitDelayDef = par("reinitDelay");
        txCreditMaxDef = par("txCreditMax");
        txFastInitDef = par("txFastInit");
        adminStatusDef = getAdminStatusFromString(par("adminStatus"));

        tickTimer = new LLDPTimer();
        tickTimer->setTimerType(Tick);

        ift = check_and_cast<IInterfaceTable *>(containingModule->getSubmodule("interfaceTable"));
        lat = getModuleFromPar<LLDPAgentTable>(par("lldpAgentTableModule"), this);
        lnt = getModuleFromPar<LLDPNeighbourTable>(par("lldpNeighbourTableModule"), this);

        WATCH_PTRVECTOR(lat->getAgents());
        WATCH_PTRVECTOR(lnt->getNeighbours());
    }
    if (stage == INITSTAGE_LAST) {
        containingModule->subscribe(interfaceStateChangedSignal, this);
        containingModule->subscribe(interfaceCreatedSignal, this);
        containingModule->subscribe(interfaceDeletedSignal, this);

        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(containingModule->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;

        chassisId = generateChassisId();
        startLLDP();

        LLDPDeviceConfigurator *conf = new LLDPDeviceConfigurator(par("deviceId"),par("deviceType"),par("configFile"), ift);
        conf->loadLLDPConfig(this);

        for (auto & agent : lat->getAgents())
            if(agent->getAdminStatus() == disabled || agent->getAdminStatus() == enabledRxOnly)
                cancelEvent(agent->getTxTTRTimer());
    }
}

std::string LLDPMain::generateChassisId()
{
   for(int i=0; i < ift->getNumInterfaces(); i++)
        if(ift->getInterface(i)->getMacAddress().getInt() != 0)
            return ift->getInterface(i)->getMacAddress().str();

    std::string s = containingModule->getFullName();
    return s;
}

void LLDPMain::getCapabilities(cProperty *propSysCap, cProperty *propEnCap)
{
    int capabilityPos;
    enCap[0] = 0;
    enCap[1] = 0;
    sysCap[0] = 0;
    sysCap[1] = 0;

    if(propSysCap != nullptr)
    {
        for(int i=0; i < propSysCap->getNumValues(""); i++)
        {
            capabilityPos = capabilitiesPosition(propSysCap->getValue("", i));
            if(capabilityPos != -1)
                sysCap[1-capabilityPos/8] |= 1 << capabilityPos%8;
        }
    }

    if(propEnCap != nullptr)
    {
        for(int i=0; i < propEnCap->getNumValues(""); i++)
        {
            capabilityPos = capabilitiesPosition(propEnCap->getValue("", i));
            if(capabilityPos != -1)
                enCap[1-capabilityPos/8] |= 1 << capabilityPos%8;
        }
    }
}

int LLDPMain::capabilitiesPosition(std::string capability)
{
    if(capability.compare("Other") == 0)
        return 0;
    else if(capability.compare("Repeater") == 0)
        return 1;
    else if(capability.compare("MAC Bridge") == 0)
        return 2;
    else if(capability.compare("WLAN Access Point") == 0)
        return 3;
    else if(capability.compare("Router") == 0)
        return 4;
    else if(capability.compare("Telephone") == 0)
        return 5;
    else if(capability.compare("DOCSIS cable device") == 0)
        return 6;
    else if(capability.compare("Station Only") == 0)
        return 7;
    else if(capability.compare("C-VLAN Component of a VLAN Bridge") == 0)
        return 8;
    else if(capability.compare("S-VLAN Component of a VLAN Bridge") == 0)
        return 9;
    else if(capability.compare("Two-port MAC Relay (TPMR)") == 0)
        return 10;
    else
        return -1;
}


void LLDPMain::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method_Silent();

    InterfaceEntry *interface;
    LLDPAgent *agent;

    if(signalID == interfaceCreatedSignal)
    {
        //new interface created
        interface = check_and_cast<const InterfaceEntryChangeDetails *>(obj)->getInterfaceEntry();
        createAgent(interface);
    }
    else if(signalID == interfaceDeletedSignal)
    {
        //interface deleted
        interface = check_and_cast<const InterfaceEntryChangeDetails *>(obj)->getInterfaceEntry();
        agent = lat->findAgentById(interface->getInterfaceId());
        agent->txShutdownFrame();
        delete agent;
    }
    else if(signalID == interfaceStateChangedSignal)
    {
        //interface state changed
        interface = check_and_cast<const InterfaceEntryChangeDetails *>(obj)->getInterfaceEntry();
        agent = lat->findAgentById(interface->getInterfaceId());

        if(interface->isUp())
            agent->startAgent();
        else
            agent->stopAgent();
    }
}

bool LLDPMain::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
   Enter_Method_Silent();

    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_LAST) {
            startLLDP();
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_LINK_LAYER){
            //TODO: interface already down so shutdown packet are dropped
            // send shutdown updates to neighbors
            stopLLDP();
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if ((NodeCrashOperation::Stage)stage == NodeCrashOperation::STAGE_CRASH)
            stopLLDP();
    }
    else
        throw cRuntimeError("Unsupported operation '%s'", operation->getClassName());

    return true;
}

void LLDPMain::startLLDP()
{
    for(int i=0; i < ift->getNumInterfaces(); i++)
        if(isInterfaceSupported(ift->getInterface(i)->getInterfaceModule()->getName()))
            createAgent(ift->getInterface(i));

    for (auto & agent : lat->getAgents())
        if(!agent->getTxTTRTimer()->isScheduled())
            agent->startAgent();

#ifdef CREDIT
    scheduleAt(simTime() + 1, tickTimer);
#endif
}

void LLDPMain::stopLLDP()
{
#ifdef CREDIT
    cancelEvent(tickTimer);
#endif

    for (auto & agent : lat->getAgents())
    {
        agent->txShutdownFrame();
        agent->setReinitDelaySet(true);
        scheduleAt(simTime() + agent->getReinitDelay(), agent->getTxShutdownWhile());
        agent->stopAgent();
    }
}

void LLDPMain::createAgent(InterfaceEntry *interface)
{
    LLDPAgent *agent = lat->findAgentById(interface->getInterfaceId());
    if(agent == nullptr)
    {
        LLDPAgent *agent = new LLDPAgent(interface, this, msgFastTxDef, msgTxHoldDef, msgTxIntervalDef,
                reinitDelayDef, txFastInitDef, txCreditMaxDef, adminStatusDef);

        lat->addAgent(agent);
    }
}

bool LLDPMain::isInterfaceSupported(const char *name)
{
    if(strcmp(name, "eth") == 0 || strcmp(name, "ppp") == 0)
        return true;
    else
        return false;
}

void LLDPMain::processTimer(LLDPTimer *msg)
{
    std::vector<LLDPAgent *>::iterator it;
    LLDPAgent *agent;

    switch(msg->getTimerType())
    {
        case LLDPTimerType::Tick: {
            EV_DETAIL << "TICK message \n";
            for (it = lat->getAgents().begin(); it != lat->getAgents().end(); ++it)
                (*it)->txAddCredit();
            scheduleAt(simTime() + 1, msg);
            break;
        }

        case LLDPTimerType::TTR: {
            agent = check_and_cast<LLDPAgent *>(static_cast<cObject *>(msg->getContextPointer()));
            EV_DETAIL << "Update time on interface " << agent->getInterface()->getFullName() << endl;
            agent->txInfoFrame();
            break;
        }

        case LLDPTimerType::ShutdownWhile: {
            agent = check_and_cast<LLDPAgent *>(static_cast<cObject *>(msg->getContextPointer()));
            EV_DETAIL << "Shutdown while time on interface " << agent->getInterface()->getFullName() << endl;
            agent->setReinitDelaySet(false);
            lnt->removeNeighboursByAgent(agent);
            break;
        }

        default: {
            EV_WARN << "Self-message with unexpected message kind " << msg->getTimerType() << endl;
            delete msg;
            break;
        }
    }
}

void LLDPMain::handleMessage(cMessage *msg)
{
    if (!isOperational) {
        EV << "Message '" << msg << "' arrived when module status is down, dropped it\n";
        delete msg;
    }
    if (msg->isSelfMessage())
    {
        processTimer(check_and_cast<LLDPTimer*> (msg));
    }
    else if(dynamic_cast<LLDPUpdate *>(msg))
    {
        handleUpdate(check_and_cast<LLDPUpdate*> (msg));
        delete msg;
    }
    else
    {
        EV_WARN << "Unrecognized message " << msg->getClassName() << ", " << msg->getName() << endl;
    }
}

void LLDPMain::handleUpdate(LLDPUpdate *msg)
{
    // get agent
    int ifaceId = ift->getInterfaceByNetworkLayerGateIndex(msg->getArrivalGate()->getIndex())->getInterfaceId();
    LLDPAgent *agent = lat->findAgentById(ifaceId);
    if(agent == nullptr)
    {
        EV_ERROR << "Agent doesn't exist on interface " << ifaceId << endl;
        return;
    }

    LLDPStatistics *st = agent->getSt();
    st->framesInTotal++;

    // check if agent is enabled to receive
    if(agent->getAdminStatus() == enabledTxOnly || agent->getAdminStatus() == disabled)
        return;

    // validation
    if(!frameValidation(msg, agent))
    {
        st->framesDiscardedTotal++;
        return;
    }

    // update
    agent->neighbourUpdate(msg);
}

bool LLDPMain::frameValidation(LLDPUpdate *msg, LLDPAgent *agent)
{
    int count[128];
    short type;
    memset(count, 0, sizeof(count));
    LLDPStatistics *st = agent->getSt();
    unsigned int i;
    bool endOf = false;

    for(i=0; i < msg->getOptionArraySize(); i++)
    {
        TlvOptionBase *option = &msg->getOption(i);
        type = msg->getOption(i).getType();

        // validation of option position/occurrence
        switch(type)
        {
            case LLDPTLV_END_OF: {
                endOf = true;
                break;
            }

            case LLDPTLV_CHASSIS_ID: {
                if(i != 0)
                {
                    EV_WARN << "An LLDPDU shall contain exactly one " << getNameOfTlv(type) << " TLV and be the first TLV in LLDPDU. Frame dropped" << endl;
                    return false;
                }
                break;
            }

            case LLDPTLV_PORT_ID: {
                if(i != 1)
                {
                    EV_WARN << "An LLDPDU shall contain exactly one " << getNameOfTlv(type) << " TLV and be the second TLV in LLDPDU. Frame dropped" << endl;
                    return false;
                }
                break;
            }

            case LLDPTLV_TTL: {
                if(i != 2)
                {
                    EV_WARN << "An LLDPDU shall contain exactly one " << getNameOfTlv(type) << " TLV and be the third TLV in LLDPDU. Frame dropped" << endl;
                    return false;
                }
                break;
            }

            case LLDPTLV_PORT_DES: {
                if(count[type] > 0)
                {
                    EV_WARN << "An LLDPDU should contain one " << getNameOfTlv(type) << " TLV. TLV deleted" << endl;
                    msg->getOptions().remove(option);
                    i--;
                    st->tlvsDiscardedTotal++;
                }
                count[type]++;
                break;
            }

            case LLDPTLV_SYSTEM_NAME:
            case LLDPTLV_SYSTEM_DES:
            case LLDPTLV_SYSTEM_CAP:
            case LLDPTLV_MAN_ADD: {
                if(count[type] > 0)
                {
                    EV_WARN << "An LLDPDU shall not contain more than one " << getNameOfTlv(type) << " TLV. Frame dropped" << endl;
                    return false;
                }
                count[type]++;
                break;
            }

            case LLDPTLV_ORG_SPEC: {
                break;
            }

            default: {
                st->tlvsUnrecognizedTotal++;
            }
        }
        // validation of option length
        if(msg->getOptionLength(option) != option->getLength())
        {
            if(type == LLDPTLV_CHASSIS_ID || type == LLDPTLV_PORT_ID || type == LLDPTLV_TTL)
            {//mandatory TLV couldn't be deleted
                EV_WARN << "TLV length of " << getNameOfTlv(type) << " TLV is different than actually length. Frame dropped" << endl;
                return false;
            }
            else
            {
                EV_WARN << "TLV length of " << getNameOfTlv(type) << " TLV is different than actually length. TLV deleted" << endl;
                msg->getOptions().remove(option);
                i--;
                st->tlvsDiscardedTotal++;
            }
            st->lldpduLengthErrors++;
        }

        if(endOf)       // LLDPTLV_END_OF
            break;
    }

    // remove options after End Of TLV
    for(; i < msg->getOptionArraySize(); i++)
    {
        TlvOptionBase *option = &msg->getOption(i);
        msg->getOptions().remove(option);
        st->tlvsDiscardedTotal++;
    }

    return true;
}

std::string LLDPMain::getNameOfTlv(short type)
{
    std::string s;

    if(type == LLDPTLV_END_OF)
        s = "End Of LLDPDU";
    else if(type == LLDPTLV_CHASSIS_ID)
        s = "Chassis ID";
    else if(type == LLDPTLV_PORT_ID)
        s = "Port ID";
    else if(type == LLDPTLV_TTL)
        s = "Time To Live";
    else if(type == LLDPTLV_PORT_DES)
        s = "Port Description";
    else if(type == LLDPTLV_SYSTEM_NAME)
        s = "System Name";
    else if(type == LLDPTLV_SYSTEM_DES)
        s = "System Description";
    else if(type == LLDPTLV_SYSTEM_CAP)
        s = "System Capabilities";
    else if(type == LLDPTLV_MAN_ADD)
        s = "Management Address";
    else if(type == LLDPTLV_ORG_SPEC)
        s = "Organizationally Specific";

    return s;
}

AS LLDPMain::getAdminStatusFromString(std::string par)
{
    if(par == "enabledRxTx")
        return enabledRxTx;
    else if(par == "enabledTxOnly")
        return enabledTxOnly;
    else if(par == "enabledRxOnly")
        return enabledRxOnly;
    else if(par == "disabled")
        return disabled;
    else
        return enabledRxTx;
}

void LLDPMain::finish()
{
    EV_INFO << lat->printStats();
}

} //namespace
