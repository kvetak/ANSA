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

#include "ansa/linklayer/lldp/LLDP.h"
#include "inet/common/NotifierConsts.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/NotifierConsts.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/networklayer/contract/IRoutingTable.h"

#include "ansa/linklayer/lldp/LLDPDeviceConfigurator.h"

//TODO: handleParameterChange(const char *parname)
//TODO: jak resit zmeny promennych
//TODO: jak s vypnutim posilani pouze na jednom rozhrani. Resit? Vyresene pomoci device configuratoru
//TODO: kdyz prijde zprava na vypnute rozhrani spadne to. FIXNUL jsem v inet/linklayer/base/MACBase.cc handleMessageWhenDown()
//TODO: nemam jak simulovat v gns3

//TODO: maximalni velikost paketu a deleni. Resit?

namespace inet {

Define_Module(LLDP);

void LLDP::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        containingModule = findContainingNode(this);
        if (!containingModule)
            throw cRuntimeError("Containing @networkNode module not found");
        ift = check_and_cast<IInterfaceTable *>(containingModule->getSubmodule("interfaceTable"));

        msgFastTxDef = par("msgFastTx");
        msgTxHoldDef = par("msgTxHold");
        msgTxIntervalDef = par("msgTxInterval");
        reinitDelayDef = par("reinitDelay");
        txCreditMaxDef = par("txCreditMax");
        txFastInitDef = par("txFastInit");
        adminStatusDef = getAdminStatusFromString(par("adminStatus"));

        WATCH_PTRVECTOR(lat.getAgents());
    }
    if (stage == INITSTAGE_LAST) {
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(containingModule->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;

        startLLDP();

        LLDPDeviceConfigurator *conf = new LLDPDeviceConfigurator(par("deviceId"),par("deviceType"),par("configFile"), ift);
        conf->loadLLDPConfig(this);

        lat.startAgents();
    }
}

bool LLDP::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_LINK_LAYER) {
            startLLDP();
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_LINK_LAYER){
            //TODO: jak udelat odeslani TTL=0 pred vypnutim
            /*for (auto it = odrRoutes.begin(); it != odrRoutes.end(); ++it)
                invalidateRoute(*it);*/
            // send updates to neighbors
            /*for (auto it = cdpInterfaceTable.getInterfaces().begin(); it != cdpInterfaceTable.getInterfaces().end(); ++it)
                sendUpdate((*it)->getInterfaceId(), true);*/

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


void LLDP::startLLDP()
{
    for(int i=0; i < ift->getNumInterfaces(); i++)
    {
        if(isInterfaceSupported(ift->getInterface(i)->getInterfaceModule()->getName()))
        {
            createAgent(ift->getInterface(i));
        }
    }

    tickTimer = new LLDPTimer();
    tickTimer->setTimerType(Tick);
    scheduleAt(simTime() + 1, tickTimer);
}

void LLDP::stopLLDP()
{
    cancelAndDelete(tickTimer);
}

void LLDP::createAgent(InterfaceEntry *interface)
{
    LLDPAgent *agent = new LLDPAgent(interface, containingModule, msgFastTxDef, msgTxHoldDef, msgTxIntervalDef,
                                reinitDelayDef, txFastInitDef, txCreditMaxDef, adminStatusDef);

    lat.addAgent(agent);
}


void LLDP::handleParameterChange(const char* name)
{
    if (name && !strcmp(name, "txCreditMax")) {
        txCreditMaxDef = par(name);
    }
    else if (name && !strcmp(name, "msgFastTx")) {
        msgFastTxDef = par(name);
    }
    else if (name && !strcmp(name, "msgTxInterval")) {
        msgTxIntervalDef = par(name);
    }
    else if (name && !strcmp(name, "reinitDelay")) {
        reinitDelayDef = par(name);
    }
    else if (name && !strcmp(name, "adminStatusDefault")) {
        adminStatusDef = getAdminStatusFromString(par(name));
    }
    else if (name && !strcmp(name, "reinitDelay")) {
        reinitDelayDef = par(name);
    }
}

/**
 * Check if interface is supported. Only ethernet and ppp.
 */
bool LLDP::isInterfaceSupported(const char *name)
{
    if(strcmp(name, "eth") == 0 || strcmp(name, "ppp") == 0)
        return true;
    else
        return false;
}

/**
 * Handle timers (self-messages)
 *
 * @param   msg     message
 */
void LLDP::handleTimer(LLDPTimer *msg)
{
    std::vector<LLDPAgent *>::iterator it;
    LLDPAgent *agent;

    switch(msg->getTimerType())
    {
        case LLDPTimerType::Tick:
            EV << "TICK!!!!!!!!!!!!!!!!!!!!!!! \n";
            for (it = lat.getAgents().begin(); it != lat.getAgents().end(); ++it)
            {// through all agents
                (*it)->txAddCredit();
            }
            scheduleAt(simTime() + 1, msg);
            break;
        case LLDPTimerType::TTR:
            EV << "TTR!!!!!!!!!!!!!!!!!!!!!!! \n";
            agent = check_and_cast<LLDPAgent *>(static_cast<cObject *>(msg->getContextPointer()));
            agent->txInfoFrame();
            break;
        default:
            EV << "UNKNOWN TIMER!!!!!!!!!!!!!!!!!!!!!!! \n";
            delete msg;
            break;
    }
}

void LLDP::handleMessage(cMessage *msg)
{
    if (!isOperational) {
        EV << "Message '" << msg << "' arrived when module status is down, dropped it\n";
        delete msg;
    }
    if (msg->isSelfMessage())
    {
        handleTimer(check_and_cast<LLDPTimer*> (msg));
    }
    else
    {
        //error("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }
}

AS LLDP::getAdminStatusFromString(std::string par)
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

} //namespace
