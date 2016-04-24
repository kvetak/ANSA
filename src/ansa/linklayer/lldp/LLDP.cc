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

//DOTAZ: jde nejak zrusit upozornovani na nejake udalosti? (Tick zpravy)

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

        getCapabilities(containingModule->getProperties()->get("capabilities"), containingModule->getProperties()->get("capabilitiesEnabled"));

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

        chassisId = generateChassisId();
        startLLDP();

        LLDPDeviceConfigurator *conf = new LLDPDeviceConfigurator(par("deviceId"),par("deviceType"),par("configFile"), ift);
        conf->loadLLDPConfig(this);

        lat.startAgents();
    }
}

/**
 * Return mac address from first interface in ift with set mac address
 *
 * @return  chassis ID
 */
std::string LLDP::generateChassisId()
{
   for(int i=0; i < ift->getNumInterfaces(); i++)
        if(ift->getInterface(i)->getMacAddress().getInt() != 0)
            return ift->getInterface(i)->getMacAddress().str();

    std::string s;
    return s;       //TODO co udelat
}

/**
 * Set capability vector
 *
 * @param   property    module capability
 */
void LLDP::getCapabilities(cProperty *propSysCap, cProperty *propEnCap)
{
    int capabilityPos;
    enCap[0]=enCap[1]=sysCap[2]=sysCap[3]=0;

    if(propSysCap != NULL)
    {
        for(int i=0; i < propSysCap->getNumValues(""); i++)
        {
            capabilityPos = capabilitiesPosition(propSysCap->getValue("", i));
            if(capabilityPos != -1)
                sysCap[1-capabilityPos/8] |= 1 << capabilityPos%8;
        }
    }

    if(propEnCap != NULL)
    {
        for(int i=0; i < propEnCap->getNumValues(""); i++)
        {
            capabilityPos = capabilitiesPosition(propEnCap->getValue("", i));
            if(capabilityPos != -1)
                enCap[1-capabilityPos/8] |= 1 << capabilityPos%8;
        }
    }
}

/**
 * Get position of capability in vector
 *
 * @param   capability  name of capability
 *
 * @return  position
 */
int LLDP::capabilitiesPosition(std::string capability)
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
    LLDPAgent *agent = new LLDPAgent(interface, this, msgFastTxDef, msgTxHoldDef, msgTxIntervalDef,
                                reinitDelayDef, txFastInitDef, txCreditMaxDef, adminStatusDef);

    lat.addAgent(agent);
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
void LLDP::processTimer(LLDPTimer *msg)
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
        processTimer(check_and_cast<LLDPTimer*> (msg));
    }
    else if(dynamic_cast<LLDPUpdate *>(msg))
    {
        handleUpdate(check_and_cast<LLDPUpdate*> (msg));
        delete msg;
    }
    else
    {
        //error("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }
}

/**
 * Handle received update
 *
 * @param   msg     message
 */
void LLDP::handleUpdate(LLDPUpdate *msg)
{
    if(!frameValidation(msg))
        return;

    LLDPAgent *agent = lat.findAgentByMsap(msg->getMsap());

    // shutdown packet
    if(msg->getTtl() == 0)
    {
        //cnt.removeNeighbour(neighbour);
        EV_INFO << "Neighbour " << " go down. Delete from table" << endl;
        return;
    }

    //neighbourUpdate(msg, neighbour);
}


bool LLDP::frameValidation(LLDPUpdate *msg)
{
    int countPortDes, countSystemName, countSystemDes, countCap;
    int count[128];
    short type;
    memset(count, 0, sizeof(count));

    for(unsigned int i=0; i < msg->getOptionArraySize(); i++)
    {
        TLVOptionBase *option = &msg->getOption(i);
        type = msg->getOption(i).getType();
        switch(type)
        {
            case LLDPTLV_CHASSIS_ID: {
                if(i != 0)
                {
                    EV_WARN << "An LLDPDU shall contain exactly one Chassis ID TLV and be the first TLV in LLDPDU. Frame dropped" << endl;
                    return false;
                }
                break;
            }

            case LLDPTLV_PORT_ID: {
                if(i != 1)
                {
                    EV_WARN << "An LLDPDU shall contain exactly one Port ID TLV and be the second TLV in LLDPDU. Frame dropped" << endl;
                    return false;
                }
                break;
            }

            case LLDPTLV_TTL: {
                if(i != 2)
                {
                    EV_WARN << "An LLDPDU shall contain exactly one TTL TLV and be the third TLV in LLDPDU. Frame dropped" << endl;
                    return false;
                }
                break;
            }

            case LLDPTLV_PORT_DES: {
                if(countPortDes > 0)
                {
                    EV_WARN << "An LLDPDU should contain one Port Description TLV. TLV deleted" << endl;
                    msg->getOptions().remove(option);
                    i--;
                }
                countPortDes++;
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
        }
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
            }
        }
    }

    return true;
}

std::string LLDP::getNameOfTlv(short type)
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
