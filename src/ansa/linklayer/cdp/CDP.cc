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

#include "ansa/linklayer/cdp/CDP.h"
#include "inet/common/NotifierConsts.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/NotifierConsts.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"

#include <algorithm>
#include <string>
#include <iostream>

#define SSTR( x ) static_cast< std::ostringstream & >(( std::ostringstream() << std::hex << x ) ).str()
#define TLV_TYPE_AND_LENGHT_SIZE 4

namespace inet {


Define_Module(CDP);

CDP::CDP() {
    // TODO Auto-generated constructor stub

}

CDP::~CDP() {
    // TODO Auto-generated destructor stub
    for (auto it=interfaceUpdateTimer.begin(); it!=interfaceUpdateTimer.end(); ++it)
    {
        cancelAndDelete(it->second);
        interfaceUpdateTimer.erase(it);
    }

    for (auto it = table.begin() ; it != table.end(); ++it)
        cancelAndDelete((*it)->getHoldTimeTimer());

    table.clear();
}


void CDP::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        WATCH_VECTOR(table);

        containingModule = findContainingNode(this);
        if (!containingModule)
            throw cRuntimeError("Containing @networkNode module not found");
        ift = check_and_cast<IInterfaceTable *>(containingModule->getSubmodule("interfaceTable"));

        getCapabilities(containingModule->getProperties()->get("capabilities"));

        updateTime = par("timer").doubleValue();
        holdTime = par("holdTime");
        version = par("version");
        odr = par("odr");

        CDPTimer *startupTimer = new CDPTimer();
        startupTimer->setTimerType(CDPStartUp);
        scheduleAt(simTime(), startupTimer);

        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(containingModule->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
    }
}

void CDP::deleteInterfacesUpdateTimer()
{
    for (auto it=interfaceUpdateTimer.begin(); it!=interfaceUpdateTimer.end(); ++it)
    {
        cancelAndDelete(it->second);
        interfaceUpdateTimer.erase(it);
    }
}

void CDP::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG)
{
    Enter_Method_Silent();
    EV << "INTERFACE CHANGED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!§\n";

    InterfaceEntry *changedIE = check_and_cast<const InterfaceEntryChangeDetails *>(obj)->getInterfaceEntry();
    int id = changedIE->getNodeOutputGateId();

    if(changedIE->isUp() && containingModule->gate(id)->isPathOK())
    {
        EV << "Interface go up, id:" << id <<  "\n";
        CDPTimer *updateTimer = new CDPTimer();
        updateTimer->setTimerType(CDPUpdateTime);
        updateTimer->setIndex(id);
        interfaceUpdateTimer.insert(std::pair<int,CDPTimer *>(id, updateTimer));
        sendUpdate(id);
        scheduleAt(simTime() + updateTime, updateTimer);
    }
    else
    {
        EV << "Interface go down, id: " << id << "\n";
        std::map<int,CDPTimer *>::iterator it = interfaceUpdateTimer.find(id);
        if(it != interfaceUpdateTimer.end())
        {
            cancelAndDelete(it->second);
            interfaceUpdateTimer.erase(it);
        }
    }
}

bool CDP::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_LINK_LAYER)
            isOperational = true;
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_LINK_LAYER)
            stop();
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if ((NodeCrashOperation::Stage)stage == NodeCrashOperation::STAGE_CRASH)
            stop();
    }
    else
        throw cRuntimeError("Unsupported operation '%s'", operation->getClassName());

    return true;
}

void CDP::stop()
{
    isOperational = false;
    deleteInterfacesUpdateTimer();
    //table.clear();        //TODO:smazat?
}

void CDP::getCapabilities(cProperty *property)
{
    if(property != NULL)
    {
        for(int i=0; i < property->getNumValues(""); i++)
        {
            capabilities.push_back(property->getValue("", i));
        }
    }
}

void CDP::scheduleALL()
{
    CDPTimer *updateTimer;
    int index;
    for(int i=0; i < containingModule->gate("ethg$o", 0)->getVectorSize(); i++)
    {
        index = containingModule->gate("ethg$o", i)->getIndex();
        if(ift->getInterface(index)->isUp() && containingModule->gate("ethg$o", i)->isPathOK())
        {
            updateTimer = new CDPTimer();
            updateTimer->setTimerType(CDPUpdateTime);
            updateTimer->setIndex(containingModule->gate("ethg$o", i)->getId());
            interfaceUpdateTimer.insert(std::pair<int,CDPTimer *>(containingModule->gate("ethg$o", i)->getId(), updateTimer));
            scheduleAt(simTime()+1, updateTimer);
        }
    }
}

uint16_t CDP::getTlvSize(CDPTLV *tlv)
{
    return sizeof(tlv->getLength()) + TLV_TYPE_AND_LENGHT_SIZE;
}

void CDP::setTlvDeviceId(CDPUpdate *msg, int pos)
{
    CDPTLV *tlv = new CDPTLV();
    tlv->setType(TLV_DEVICE_ID);
    tlv->setValue(containingModule->getFullName());
    tlv->setLength(getTlvSize(tlv));
    msg->setTlv(pos, tlv[0]);
}

void CDP::setTlvPortId(CDPUpdate *msg, int pos, int index)
{
    CDPTLV *tlv = new CDPTLV();
    tlv->setType(TLV_PORT_ID);
    tlv->setValue(ift->getInterfaceByNodeOutputGateId(index)->getFullName());
    tlv->setLength(getTlvSize(tlv)-1);
    msg->setTlv(pos, tlv[0]);
}

void CDP::setTlvVersion(CDPUpdate *msg, int pos)
{
    CDPTLV *tlv = new CDPTLV();
    tlv->setType(TLV_VERSION);

    //TODO: upravit na verzi SW
    tlv->setValue(SSTR(1).c_str());
    tlv->setLength(getTlvSize(tlv)-1);
    msg->setTlv(pos, tlv[0]);
}

int CDP::capabilitiesPosition(std::string capability)
{
    if(capability.compare("Router") == 0)
        return 0;
    else if(capability.compare("Transparent Bridge") == 0)
        return 1;
    else if(capability.compare("Source Route Bridge") == 0)
        return 2;
    else if(capability.compare("Switch") == 0)
        return 3;
    else if(capability.compare("Host") == 0)
        return 4;
    else if(capability.compare("IGMP capable") == 0)
        return 5;
    else if(capability.compare("Repeater") == 0)
        return 6;
    else
        return -1;
}

void CDP::setTlvCapabilities(CDPUpdate *msg, int pos)
{
    char cap[4];
    int capabilityPos;
    CDPTLV *tlv = new CDPTLV();
    cap[0]=cap[1]=cap[2]=cap[3]=0;

    for (std::vector<std::string>::iterator it = capabilities.begin(); it != capabilities.end(); ++it)
    {
        capabilityPos = capabilitiesPosition(*it);
        if(capabilityPos == -1)
            continue;
        cap[3-capabilityPos/8] |= 1 << capabilityPos%8;
    }
    tlv->setType(TLV_CAPABILITIES);
    tlv->setValue(cap);
    tlv->setLength(getTlvSize(tlv));
    msg->setTlv(pos, tlv[0]);
}

void CDP::setTlvDuplex(CDPUpdate *msg, int pos, int index)
{
    CDPTLV *tlv = new CDPTLV();
    tlv->setType(TLV_DUPLEX);
    if(ift->getInterfaceByNodeOutputGateId(index)->getInterfaceModule()->getSubmodule("mac")->par("duplexMode"))
        tlv->setValue("1");
    else
        tlv->setValue("0");
    tlv->setLength(getTlvSize(tlv)-1);
    msg->setTlv(pos, tlv[0]);
}

void CDP::setTlvODR(CDPUpdate *msg, int pos, int index)
{
    CDPTLV *tlv = new CDPTLV();
    tlv->setType(TLV_ODR);
    tlv->setValue(ift->getInterfaceByNodeOutputGateId(index)->getNetworkAddress().str().c_str());
    tlv->setLength(getTlvSize(tlv)-1);
    msg->setTlv(pos, tlv[0]);
}

void CDP::setTlvIpPrefix(CDPUpdate *msg, int pos)
{
    CDPTLV *tlv = new CDPTLV();
    tlv->setType(TLV_IP_PREFIX);
    std::string prefixes, prefix, networkMask;
    int mask, count;
    for(int i=0; i < ift->getNumInterfaces(); i++)
    {
        if(ift->getInterface(i)->ipv4Data() != nullptr &&
                !ift->getInterface(i)->ipv4Data()->getIPAddress().isUnspecified())
        {
            prefix = SSTR(ift->getInterface(i)->ipv4Data()->getIPAddress().getInt());
            if(prefix.size() < 8) prefix = "0" + prefix;
            EV << prefix << " ";
            mask = ift->getInterface(i)->ipv4Data()->getNetmask().getInt();
            for(count = 0; mask; mask &= mask - 1) count++;
            networkMask = SSTR(count);
            if(count < 16) networkMask = "0" + networkMask;
            EV << networkMask << "\n";

            prefixes.append(prefix);
            prefixes.append(networkMask);
        }

    }
    EV << prefixes.c_str() << " " << sizeof(prefixes.c_str()) << " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
    tlv->setValue(prefixes.c_str());
    EV << getTlvSize(tlv)-1 << "\n";
    tlv->setLength(getTlvSize(tlv)-1);
    msg->setTlv(pos, tlv[0]);
}

void CDP::createTlv(CDPUpdate *msg, int index)
{
    int count = 5;
    msg->setTlvArraySize(count);

    setTlvDeviceId(msg, 0); //device-id
    setTlvPortId(msg, 1, index); //port-id
    setTlvVersion(msg, 2); //version
    setTlvCapabilities(msg, 3); //capabilities
    setTlvDuplex(msg, 4, index); //full duplex
    if(odr)
    {
        EV << "neeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\n";
        count++;
        msg->setTlvArraySize(count);
        setTlvODR(msg, 5, index); //odr hub
    }

    if(std::find(odrSpokePropagationInt.begin(), odrSpokePropagationInt.end(), containingModule->gate(index)->getIndex())
                != odrSpokePropagationInt.end()
                && !hasRoutingProtocol())
    {
        count++;
        msg->setTlvArraySize(count);
        setTlvIpPrefix(msg, 4); //full duplex
        EV << "posilani adres tadyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n";
    }
    else
    {
        EV << index << " "<< "\n";
        EV << "ne posilani adres tadyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n";
    }
}

bool CDP::hasRoutingProtocol()
{
    bool result = false;
    cProperties *properties = containingModule->getProperties();
    result |= properties->getAsBool("hasOSPF");
    result |= properties->getAsBool("hasRIP");
    result |= properties->getAsBool("hasBGP");
    result |= properties->getAsBool("hasPIM");
    result |= properties->getAsBool("hasEIGRP");
    result |= properties->getAsBool("hasBABEL");
    result |= properties->getAsBool("hasLISP");
    return result;
}

void CDP::sendUpdate(int index)
{
    CDPUpdate *msg = new CDPUpdate();
    msg->setTtl(holdTime);
    createTlv(msg, index);

    Ieee802Ctrl *controlInfo = new Ieee802Ctrl();
    controlInfo->setDest(MACAddress("01-00-0c-cc-cc-cc"));
    controlInfo->setInterfaceId(index);
    msg->setControlInfo(controlInfo);

    send(msg, "ifOut", containingModule->gate(index)->getIndex());
}

void CDP::deleteEntryInTable(CDPTableEntry *entry)
{
    for (auto it = table.begin() ; it != table.end(); ++it)
    {
        if((*it) == entry)
        {
            delete(*it);
            table.erase(it);
            EV << "DELETE ENTRY!!!!!!!!!!!!!!!!!!!!!!!! \n";
            break;
        }
    }
}

CDPTableEntry *CDP::findEntryInTable(std::string name, int port)
{
    for (auto it = table.begin() ; it != table.end(); ++it)
        if((*it)->getName() == name && (*it)->getPortReceive() == port)
            return *it;

    return NULL;
}

CDPTableEntry *CDP::newTableEntry(CDPUpdate *msg)
{
    CDPTableEntry *entry = new CDPTableEntry();

    for(unsigned int i=0; i < msg->getTlvArraySize(); i++)
    {
        switch(msg->getTlv(i).getType())
        {
            case TLV_DEVICE_ID:
                entry->setName(msg->getTlv(i).getValue());
                break;
            case TLV_PORT_ID:
                entry->setPortSend(atoi(msg->getTlv(i).getValue()));
                break;
            case TLV_DUPLEX:
                entry->setFullDuplex(msg->getTlv(i).getValue());
                break;
            case TLV_CAPABILITIES:
                entry->setCapabilities(msg->getTlv(i).getValue());
                break;
            case TLV_VERSION:
                entry->setVersion(msg->getTlv(i).getValue());
                break;
            case TLV_PLATFORM:
                entry->setPlatform(msg->getTlv(i).getValue());
                break;
            case TLV_ODR:
                EV << "ZAPNOUT POSILANI ADRES!!!!!\n";
                if(std::find(odrSpokePropagationInt.begin(), odrSpokePropagationInt.end(), msg->getArrivalGateId())
                            == odrSpokePropagationInt.end())
                {
                    odrSpokePropagationInt.push_back(containingModule->gate("ethg$o", msg->getArrivalGate()->getIndex())->getIndex());
                }
                break;
            case TLV_IP_PREFIX:
                EV << "PRIJMUTI ADRES!!!!!!!!!!\n";
                if(odr)
                {
                    //for
                }
                break;
        }
    }

    EV << "vlozeni " << containingModule->gate("ethg$o", msg->getArrivalGate()->getIndex())->getIndex() << "\n";
    //EV << "vlozeni " << msg->getArrivalGate()->getIndex() << "\n";

    //ift->getInterfaceByNodeOutputGateId(index)->getInterfaceModule()->getSubmodule("mac")->par("duplexMode")


    entry->setLastUpdated(simTime());
    entry->setPortReceive(msg->getArrivalGateId());

    CDPTimer *holdTimeTimer = new CDPTimer();
    holdTimeTimer->setTimerType(CDPHoldTime);
    holdTimeTimer->setContextPointer(entry);
    scheduleAt(simTime() + (simtime_t)msg->getTtl(), holdTimeTimer);
    entry->setHoldTimeTimer(holdTimeTimer);

    return entry;
}

void CDP::handleTimer(CDPTimer *msg)
{
    if(msg->getTimerType() == CDPUpdateTime )
    {
        EV << msg->getIndex() << " UPDATETIME!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n";
        sendUpdate(msg->getIndex());
        scheduleAt(simTime() + updateTime, msg);
    }
    else if(msg->getTimerType() == CDPHoldTime)
    {
        EV << "HOLDTIME!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n";
        CDPTableEntry *entry = reinterpret_cast<CDPTableEntry *>(msg->getContextPointer());
        deleteEntryInTable(entry);
        delete msg;
    }
    else if(msg->getTimerType() == CDPStartUp)
    {
        EV << "START UP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n";
        containingModule->subscribe(NF_INTERFACE_STATE_CHANGED, this);
        scheduleALL();
        delete msg;
    }
    else
    {
        EV << "UNKNOWN TIMER!!!!!!!!!!!!!!!!!!!!!!! \n";
    }
}

void CDP::handleUpdate(CDPUpdate *msg)
{
    if(msg->getTlvArraySize() == 0)
        return;
    CDPTableEntry *entry = findEntryInTable(msg->getTlv(0).getValue(), msg->getArrivalGateId());

    if(entry != NULL)
    {
        EV << "UPDATE ENTRY" << msg->getTlv(0).getValue() << " " << msg->getArrivalGateId() << "\n";
        entry->setLastUpdated(simTime());

        //reschedule holdtime
        if (entry->getHoldTimeTimer()->isScheduled())
            cancelEvent(entry->getHoldTimeTimer());
        scheduleAt(simTime() + (simtime_t)msg->getTtl(), entry->getHoldTimeTimer());
    }
    else
    {
        EV << "NEW ENTRY" << msg->getTlv(0).getValue() << " " << msg->getArrivalGateId() << "\n";

        entry = newTableEntry(msg);
        table.push_back(entry);
    }
}

void CDP::handleMessage(cMessage *msg)
{
    if (!isOperational) {
        EV << "Message '" << msg << "' arrived when module status is down, dropped it\n";
        delete msg;
    }
    else if (msg->isSelfMessage())
    {
        handleTimer(check_and_cast<CDPTimer*> (msg));
    }
    else if(dynamic_cast<CDPUpdate *>(msg))
    {
        EV << "Packet arrived \n";
        handleUpdate(check_and_cast<CDPUpdate*> (msg));
        delete msg;
    }
    else
    {
        error("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }
}

} /* namespace inet */
