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
#include "inet/networklayer/contract/IRoute.h"
#include "inet/common/NotifierConsts.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/NotifierConsts.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/networklayer/contract/IRoutingTable.h"

#include <algorithm>
#include <string>
#include <iostream>

#define SSTR( x ) static_cast< std::ostringstream & >(( std::ostringstream() << std::hex << x ) ).str()
#define TLV_TYPE_AND_LENGHT_SIZE 4
#define IPV4_ADDRESS_STRING_LENGTH 8
#define NETMASK_STRING_LENGTH 2

namespace inet {


Define_Module(CDP);

CDP::CDP() {
    // TODO Auto-generated constructor stub

}

CDP::~CDP() {

    //delete timers
    deleteTimers();

    //delete tables
    cdpInterfaceTable.removeInterfaces();
    table.clear();
}

void CDP::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        WATCH_VECTOR(table);
        WATCH_VECTOR(cdpInterfaceTable.getInterfaces());

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

        if(containingModule->getSubmodule("routingTable") != nullptr)
        {
            sendIpPrefixes = true;
            //rt4 = check_and_cast<IPv4RoutingTable*>(containingModule->getSubmodule("routingTable")->getSubmodule("ipv4"));
            if(odr)
                rt = getModuleFromPar<IRoutingTable>(par("routingTableModule"), this);
        }
    }
}


///************************ CDP OPERATION ****************************///

void CDP::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG)
{
    Enter_Method_Silent();

    InterfaceEntry *interface = check_and_cast<const InterfaceEntryChangeDetails *>(obj)->getInterfaceEntry();
    int outputGateid = interface->getNodeOutputGateId();

    if(interface->isUp() && containingModule->gate(outputGateid)->isPathOK())
        activateInterface(interface);
    else
        deactivateInterface(interface);
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
    cdpInterfaceTable.removeInterfaces();
    table.clear();
    deleteTimers();
}

void CDP::deleteTimers()
{
    for (auto it = cdpInterfaceTable.getInterfaces().begin(); it != cdpInterfaceTable.getInterfaces().end(); ++it)
        cancelAndDelete((*it)->getUpdateTimer());
    for (auto it = table.begin() ; it != table.end(); ++it)
        cancelAndDelete((*it)->getHoldTimeTimer());
}

void CDP::scheduleALL()
{
    InterfaceEntry *interface;

    for(int i=0; i < ift->getNumInterfaces(); i++)
    {
        if(isInterfaceSupported(ift->getInterface(i)->getInterfaceModule()->getName()))
        {
            interface = ift->getInterface(i);
            activateInterface(interface);
        }
    }
}

void CDP::activateInterface(InterfaceEntry *interface)
{
    if(!interface->isUp() || !containingModule->gate(interface->getNodeOutputGateId())->isPathOK())
        return;

    CDPTimer *updateTimer = new CDPTimer();
    updateTimer->setTimerType(CDPUpdateTime);
    updateTimer->setInterfaceId(interface->getInterfaceId());
    scheduleAt(simTime()+1, updateTimer);

    CDPInterface *cdpInterface = new CDPInterface(interface, updateTimer);
    cdpInterfaceTable.addInterface(cdpInterface);

    EV << "Interface " << interface->getName() << " go up, id:" << interface->getInterfaceId() <<  "\n";
}

void CDP::deactivateInterface(InterfaceEntry *interface)
{
    CDPInterface *cdpInterface = cdpInterfaceTable.findInterfaceById(interface->getInterfaceId());
    if(cdpInterface != nullptr)
    {
        cancelAndDelete(cdpInterface->getUpdateTimer());
        cdpInterfaceTable.removeInterface(cdpInterface->getInterfaceId());
        EV << "Interface " << interface->getName() << " go down, id: " << interface->getInterfaceId() << "\n";
    }
}

bool CDP::isInterfaceSupported(const char *name)
{
    if(strcmp(name, "eth") == 0 || strcmp(name, "ppp") == 0)
        return true;
    else
        return false;
}

///************************ END OF CDP OPERATION ****************************///


///************************ END OF CDP OPERATION ****************************///

void CDP::deleteEntryInTable(CDPTableEntry *entry)
{
    for (auto it = table.begin() ; it != table.end(); ++it)
    {
        if((*it) == entry)
        {
            delete(*it);
            table.erase(it);
            EV << "DELETE ENTRY!!!!!!!!!!!!!!!!!!!!!!!! \n";
            return;
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


InterfaceEntry *CDP::getPortInterfaceEntry(unsigned int portNum)
{
    cGate *gate = containingModule->gate("ethg$i", portNum);
    if (!gate)
    {
        gate = containingModule->gate("ppp$i", portNum);
        if (!gate)
            throw cRuntimeError("gate is nullptr");
    }
    InterfaceEntry *gateIfEntry = ift->getInterfaceByNodeInputGateId(gate->getId());
    if (!gateIfEntry)
        throw cRuntimeError("gate's Interface is nullptr");

    return gateIfEntry;
}

CDPTableEntry *CDP::newTableEntry(CDPUpdate *msg)
{
    EV << "NEW ENTRY " << msg->getTlv(0).getValue() << " " << msg->getArrivalGateId() << "\n";

    CDPTableEntry *entry = new CDPTableEntry();

    for(unsigned int i=0; i < msg->getTlvArraySize(); i++)
    {
        switch(msg->getTlv(i).getType())
        {
            case TLV_DEVICE_ID:
                entry->setName(msg->getTlv(i).getValue());
                break;
            case TLV_PORT_ID:
                entry->setPortSend(msg->getTlv(i).getValue());
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
                //TODO: default gateway?
                //interfaceId = getPortInterfaceEntry(msg->getArrivalGate()->getIndex())->getInterfaceId();
                //cdpInterfaceTable.findInterfaceById(interfaceId)->setSendODRUpdate(true);
                break;
            case TLV_IP_PREFIX:
                //EV << "PRIJMUTI ADRES!!!!!!!!!!\n";
                break;
        }
    }

    entry->setLastUpdated(simTime());
    entry->setPortReceive(msg->getArrivalGateId());

    CDPTimer *holdTimeTimer = new CDPTimer();
    holdTimeTimer->setTimerType(CDPHoldTime);
    holdTimeTimer->setContextPointer(entry);
    scheduleAt(simTime() + (simtime_t)msg->getTtl(), holdTimeTimer);
    entry->setHoldTimeTimer(holdTimeTimer);

    return entry;
}

void CDP::updateTableEntry(CDPUpdate *msg, CDPTableEntry *entry)
{
    EV << "UPDATE ENTRY" << msg->getTlv(0).getValue() << " " << msg->getArrivalGateId() << "\n";

    int begin = 0, address, netmask, interfaceId;
    std::string prefix;
    const char *prefixes;

    entry->setLastUpdated(simTime());
    for(unsigned int i=0; i < msg->getTlvArraySize(); i++)
    {
        switch(msg->getTlv(i).getType())
        {
            case TLV_IP_PREFIX:
                    prefixes = msg->getTlv(i).getValue();
                    for(unsigned int i = begin; i < strlen(prefixes);
                            i+=IPV4_ADDRESS_STRING_LENGTH + NETMASK_STRING_LENGTH)
                    {
                        prefix.assign(prefixes, begin, IPV4_ADDRESS_STRING_LENGTH);
                        address = stoi (prefix,nullptr,16);
                        begin += IPV4_ADDRESS_STRING_LENGTH;
                        prefix.assign(prefixes, begin, NETMASK_STRING_LENGTH);
                        netmask = stoi (prefix,nullptr,16);
                        begin += NETMASK_STRING_LENGTH;
                    }
                    break;
            case TLV_PORT_ID:
                entry->setPortSend(msg->getTlv(i).getValue());
                break;
            case TLV_VERSION:
                entry->setVersion(msg->getTlv(i).getValue());
                break;
            case TLV_CAPABILITIES:
                entry->setCapabilities(msg->getTlv(i).getValue());
                break;
            case TLV_PLATFORM:
                entry->setPlatform(msg->getTlv(i).getValue());
                break;
            case TLV_ODR:
                break;
        }
    }

    //reschedule holdtime
    if (entry->getHoldTimeTimer()->isScheduled())
        cancelEvent(entry->getHoldTimeTimer());
    scheduleAt(simTime() + (simtime_t)msg->getTtl(), entry->getHoldTimeTimer());
}

void CDP::getCapabilities(cProperty *property)
{
    int capabilityPos;
    cap[0]=cap[1]=cap[2]=cap[3]=0;

    if(property != NULL)
    {
        for(int i=0; i < property->getNumValues(""); i++)
        {
            capabilityPos = capabilitiesPosition(property->getValue("", i));
            if(capabilityPos != -1)
                cap[3-capabilityPos/8] |= 1 << capabilityPos%8;
        }
    }
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


///************************ MESSAGE HANDLING ****************************///

void CDP::sendUpdate(int interfaceId)
{
    CDPUpdate *msg = new CDPUpdate();
    msg->setTtl(holdTime);
    createTlv(msg, interfaceId);
    msg->setChecksum(checksum(msg));

    Ieee802Ctrl *controlInfo = new Ieee802Ctrl();
    controlInfo->setDest(MACAddress("01-00-0c-cc-cc-cc"));
    controlInfo->setInterfaceId(interfaceId);
    msg->setControlInfo(controlInfo);

    send(msg, "ifOut", ift->getInterfaceById(interfaceId)->getNetworkLayerGateIndex());
}

void CDP::handleUpdate(CDPUpdate *msg)
{
    if(msg->getTlvArraySize() == 0  )
        return;
    CDPTableEntry *entry = findEntryInTable(msg->getTlv(0).getValue(), msg->getArrivalGateId());

    if(entry != NULL)
        updateTableEntry(msg, entry);
    else
        table.push_back(newTableEntry(msg));
}

void CDP::handleTimer(CDPTimer *msg)
{
    if(msg->getTimerType() == CDPUpdateTime )
    {
        EV << msg->getInterfaceId() << " UPDATETIME!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n";
        if(cdpInterfaceTable.findInterfaceById(msg->getInterfaceId())->getEnabled())
        {
            sendUpdate(msg->getInterfaceId());
            scheduleAt(simTime() + updateTime, msg);
        }
        else
        {
            delete msg;
        }
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
        handleUpdate(check_and_cast<CDPUpdate*> (msg));
        delete msg;
    }
    else
    {
        error("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }
}

///************************ END OF MESSAGE HANDLING ****************************///


///************************ ROUTING ****************************///

void CDP::invalidateRoutes(const InterfaceEntry *ie)
{
    for (auto it = odrRoutes.begin(); it != odrRoutes.end(); ++it)
        if ((*it)->getInterface() == ie)
            invalidateRoute(*it);

}

void CDP::deleteRoute(IRoute *route)
{
    rt->deleteRoute(route);
}

void CDP::invalidateRoute(ODRRoute *odrRoute)
{
    IRoute *route = odrRoute->getRoute();
    if (route) {
        odrRoute->setRoute(nullptr);
        //deleteRoute(route);
    }
    //ripRoute->setMetric(RIP_INFINITE_METRIC);
    //ripRoute->setChanged(true);
    //triggerUpdate();
}

/**
 * Should be called regularly to handle expiry and purge of routes.
 * If the route is valid, then returns it, otherwise returns nullptr.
 */
ODRRoute *CDP::checkRouteIsExpired(ODRRoute *route)
{
    simtime_t now = simTime();
    if (now >= route->getLastUpdateTime() + routeFlushTime) {
        purgeRoute(route);
        return nullptr;
    }
    if (now >= route->getLastUpdateTime() + routeHolddowndTime) {
        //????????
        return nullptr;
    }
    if (now >= route->getLastUpdateTime() + routeInvalidTime) {
        invalidateRoute(route);
        return nullptr;
    }
    return route;
}

/**
 * Removes the route from the routing table.
 */
void CDP::purgeRoute(ODRRoute *odrRoute)
{
    IRoute *route = odrRoute->getRoute();
    if (route) {
        odrRoute->setRoute(nullptr);
        deleteRoute(route);
    }

    auto end = std::remove(odrRoutes.begin(), odrRoutes.end(), odrRoute);
    if (end != odrRoutes.end())
        odrRoutes.erase(end, odrRoutes.end());
    delete odrRoute;

    //emit(numRoutesSignal, ripRoutes.size());
}

ODRRoute::ODRRoute(IRoute *route, uint16 routeTag)
    : route(route), changed(false), lastUpdateTime(0)
{
    dest = route->getDestinationAsGeneric();
    prefixLength = route->getPrefixLength();
    nextHop = route->getNextHopAsGeneric();
    ie = route->getInterface();
    tag = routeTag;
}

void CDP::addRoute(const L3Address& dest, int prefixLength, const InterfaceEntry *ie, const L3Address& nextHop,
        int metric, uint16 routeTag, const L3Address& from)
{
    EV_DEBUG << "Add route to " << dest << "/" << prefixLength << ": "
             << "nextHop=" << nextHop << " metric=" << metric << std::endl;

    IRoute *route = addRoute(dest, prefixLength, ie, nextHop, metric);

    ODRRoute *odrRoute = new ODRRoute(route, routeTag);
    //odrRoutes->setFrom(from);
    //odrRoutes->setLastUpdateTime(simTime());
    //odrRoutes->setChanged(true);
    odrRoutes.push_back(odrRoute);
    //emit(numRoutesSignal, ripRoutes.size());
    //triggerUpdate();
}

IRoute *CDP::addRoute(const L3Address& dest, int prefixLength, const InterfaceEntry *ie, const L3Address& nextHop, int metric)
{
    IRoute *route = rt->createRoute();
    route->setSourceType(IRoute::ODR);
    route->setSource(this);
    route->setDestination(dest);
    route->setPrefixLength(prefixLength);
    route->setInterface(const_cast<InterfaceEntry *>(ie));
    route->setNextHop(nextHop);
    route->setMetric(1);
    rt->addRoute(route);
    return route;
}


///************************ END OF ROUTING ****************************///



///************************ TLV ****************************///


bool CDP::hasRoutingProtocol()
{
    //TODO: dodelat overeni zda ja zapnuty
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

uint16_t CDP::getTlvSize(CDPTLV *tlv)
{
    return strlen(tlv->getValue()) + TLV_TYPE_AND_LENGHT_SIZE;
}

void CDP::setTlvDeviceId(CDPUpdate *msg, int pos)
{
    CDPTLV *tlv = new CDPTLV();
    tlv->setType(TLV_DEVICE_ID);
    tlv->setValue(containingModule->getFullName());
    tlv->setLength(getTlvSize(tlv));
    msg->setTlv(pos, tlv[0]);
}

void CDP::setTlvPortId(CDPUpdate *msg, int pos, int interfaceId)
{
    CDPTLV *tlv = new CDPTLV();
    tlv->setType(TLV_PORT_ID);
    tlv->setValue(ift->getInterfaceById(interfaceId)->getFullName());
    tlv->setLength(getTlvSize(tlv));
    msg->setTlv(pos, tlv[0]);
}

void CDP::setTlvPlatform(CDPUpdate *msg, int pos)
{
    CDPTLV *tlv = new CDPTLV();
    tlv->setType(TLV_PLATFORM);
    tlv->setValue(containingModule->getComponentType()->getName());
    tlv->setLength(getTlvSize(tlv));
    msg->setTlv(pos, tlv[0]);
}

void CDP::setTlvVersion(CDPUpdate *msg, int pos)
{
    CDPTLV *tlv = new CDPTLV();
    tlv->setType(TLV_VERSION);
    tlv->setValue("1.0");
    tlv->setLength(getTlvSize(tlv));
    msg->setTlv(pos, tlv[0]);
}

void CDP::setTlvCapabilities(CDPUpdate *msg, int pos)
{
    CDPTLV *tlv = new CDPTLV();
    tlv->setType(TLV_CAPABILITIES);
    tlv->setValue(cap);
    tlv->setLength(4 + TLV_TYPE_AND_LENGHT_SIZE);
    msg->setTlv(pos, tlv[0]);
}

void CDP::setTlvDuplex(CDPUpdate *msg, int pos, int interfaceId)
{
    CDPTLV *tlv = new CDPTLV();
    tlv->setType(TLV_DUPLEX);

    if(ift->getInterfaceById(interfaceId)->getInterfaceModule()->getSubmodule("mac")->par("duplexMode"))
        tlv->setValue("1");
    else
        tlv->setValue("0");
    tlv->setLength(getTlvSize(tlv));
    msg->setTlv(pos, tlv[0]);
}

void CDP::setTlvODR(CDPUpdate *msg, int pos, int interfaceId)
{
    CDPTLV *tlv = new CDPTLV();
    tlv->setType(TLV_ODR);
    tlv->setValue(ift->getInterfaceById(interfaceId)->getNetworkAddress().str().c_str());
    tlv->setLength(getTlvSize(tlv));
    msg->setTlv(pos, tlv[0]);
}

void CDP::setTlvIpPrefix(CDPUpdate *msg, int pos, int interfaceId)
{
    CDPTLV *tlv = new CDPTLV();
    tlv->setType(TLV_IP_PREFIX);
    std::string prefixes, prefix, networkMask;
    int mask, count;
    for(int i=0; i < ift->getNumInterfaces(); i++)
    {
        if(ift->getInterface(i)->ipv4Data() != nullptr &&
                !ift->getInterface(i)->ipv4Data()->getIPAddress().isUnspecified() &&
                ift->getInterface(i)->getInterfaceId() != interfaceId &&
                !ift->getInterface(i)->isLoopback())
        {
            prefix = SSTR(ift->getInterface(i)->ipv4Data()->getIPAddress().getInt());
            if(prefix.size() < 8) prefix = "0" + prefix;
            mask = ift->getInterface(i)->ipv4Data()->getNetmask().getInt();
            for(count = 0; mask; mask &= mask - 1) count++;
            networkMask = SSTR(count);
            if(count < 16) networkMask = "0" + networkMask;

            prefixes.append(prefix);
            prefixes.append(networkMask);
        }

    }
    tlv->setValue(prefixes.c_str());
    tlv->setLength(getTlvSize(tlv));
    msg->setTlv(pos, tlv[0]);
}

uint16_t CDP::checksum(CDPUpdate *msg)
{
    std::string a;
    const char *serialized;

    a += msg->getVersion();
    a += msg->getTtl();
    for(unsigned int i=0; i < msg->getTlvArraySize(); i++)
    {
        a += msg->getTlv(i).getType();
        a += msg->getTlv(i).getLength();
        a += msg->getTlv(i).getValue();
    }
    serialized = a.c_str();
    int count = a.size();
    uint32_t sum = 0;

    while (count > 1) {
        sum += (serialized[0] << 8) | serialized[1];
        serialized += 2;
        if (sum & 0x80000000)
            sum = (sum & 0xFFFF) + (sum >> 16);
        count -= 2;
    }

    if (count)
        sum += *(const uint8_t *)serialized;

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return (uint16_t)sum;
}

bool CDP::parseIPAddress(const char *text, unsigned char tobytes[])
{
    if (!text)
        return false;

    if (!strcmp(text, "<unspec>")) {
        tobytes[0] = tobytes[1] = tobytes[2] = tobytes[3] = 0;
        return true;
    }

    const char *s = text;
    int i = 0;
    while (true) {
        if (*s < '0' || *s > '9')
            return false; // missing number

        // read and store number
        int num = 0;
        if ((*s >= '0' && *s <= '9') || (*s >= 'A' && *s <= 'F'))
            num = 16 * num + (*s++ - '0');
        if (num > 255)
            return false; // number too big
        tobytes[i++] = (unsigned char)num;

        if (!*s)
            break; // end of string
        if (*s != '.')
            return false; // invalid char after number
        if (i == 4)
            return false; // read 4th number and not yet EOS

        // skip '.'
        s++;
    }
    return i == 4;    // must have all 4 numbers
}

bool CDP::ipInterfaceExist(int interfaceId)
{
    for(int i=0; i < ift->getNumInterfaces(); i++)
        if(ift->getInterface(i)->ipv4Data() != nullptr &&
                !ift->getInterface(i)->ipv4Data()->getIPAddress().isUnspecified() &&
                ift->getInterface(i)->getInterfaceId() != interfaceId &&
                !ift->getInterface(i)->isLoopback())
            return true;
    return false;
}

void CDP::createTlv(CDPUpdate *msg, int interfaceId)
{
    int count = 0;
    msg->setTlvArraySize(5);

    setTlvDeviceId(msg, count++); //device-id
    setTlvPortId(msg, count++, interfaceId); //port-id
    setTlvVersion(msg, count++); //version
    setTlvCapabilities(msg, count++); //capabilities
    setTlvPlatform(msg, count++); //platform

    if(ift->getInterfaceById(interfaceId)->getInterfaceModule()->getSubmodule("mac") != nullptr)
    {
        msg->setTlvArraySize(++count);
        setTlvDuplex(msg, count-1, interfaceId); //full duplex
    }
    if(odr) //odr hub
    {
        msg->setTlvArraySize(++count);
        setTlvODR(msg, count-1, interfaceId);
    }
    else if(sendIpPrefixes && ipInterfaceExist(interfaceId) && !hasRoutingProtocol())  //ip prefix
    {
        msg->setTlvArraySize(++count);
        setTlvIpPrefix(msg, count-1, interfaceId);
    }
}

///************************ END OF TLV ****************************///

} /* namespace inet */


