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

//OTAZKA: kdy se ma simulace ukoncit, kdyz nastane nejake neocekavana udalost (napriklad spatny checksum)
//OTAZKA: jak s address TLV, jelikoz tam mohou byt i nuly a tak jak priradit do char

//TODO: jak multiplehubs//ok
//TODO: kolikrat muze byt stejna adresa v routovaci tabulce//ok, 4
//TODO: pri smazani cdp zaznamu smazat i vsechny prislusne naucene routy. BLBOST
//TODO: udelat checksum //ok

//TODO: zajistit maximalni casy
//TODO: nejak poresit vypnuti linky na druhe strane. V gns3 zmizne okamzite z tabulky (TTL=0?)
//TODO: kdyz se smaze ODR routa z rt z jineho modulu, tak vyvolat nejakou vyjimku a smazat ji i z odrroutes
//TODO: administrativni vzdalenosti
//TODO: kdyz nespecifikuji nextHope (viz obrazek gns3), tak se automaticky v rt nastavi na directly connected

//INFO: defaultni routa se smaze po 180s, jesi je jedyno, tak sie niesmaze
//INFO: jesi je co posylac prez prefixes tak to posylo, jesi nima zapnone odr
//INFO: jak nima adresa na rozhrani, tak do routovaci tabulki sie prido 0.0.0.0 via rozhrani, albo pyrso polozka z addres TLV
//INFO: jak shutdown interface, tak sie smazom vsystki routy z tego rozhranio
//INFO: na rozhrani kde neni specifikovana adresa se neposila ODR TLV

//TODO: poresit casovace

#include "ansa/linklayer/cdp/CDP.h"
#include "inet/networklayer/contract/IRoute.h"
#include "inet/common/NotifierConsts.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/NotifierConsts.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/networklayer/contract/IRoutingTable.h"

#include <stdlib.h>
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
    for (auto it = neighbourTable.begin(); it != neighbourTable.end(); ++it)
        delete (*it);
    neighbourTable.clear();
    for (auto it = odrRoutes.begin(); it != odrRoutes.end(); ++it)
        if((*it) != nullptr)
            delete (*it);
    odrRoutes.clear();
}

std::string ODRRoute::info() const
{
    std::stringstream out;
    if (dest.isUnspecified())
        out << "0.0.0.0";
    else
        out << dest;

    out << "/" << prefixLength;

    out << " via ";
    if (nextHop.isUnspecified())
        out << "*";
    else
        out << nextHop;
    out << ", " <<  ie->getName();

    return out.str();
}

std::ostream& operator<<(std::ostream& os, const ODRRoute& e)
{
    os << e.info();
    return os;
}

std::ostream& operator<<(std::ostream& os, const CDPTableEntry& e)
{
    os << e.info();
    return os;
}

void CDP::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        WATCH_PTRVECTOR(neighbourTable);
        WATCH_VECTOR(cdpInterfaceTable.getInterfaces());
        WATCH_PTRVECTOR(odrRoutes);

        containingModule = findContainingNode(this);
        if (!containingModule)
            throw cRuntimeError("Containing @networkNode module not found");
        ift = check_and_cast<IInterfaceTable *>(containingModule->getSubmodule("interfaceTable"));

        getCapabilities(containingModule->getProperties()->get("capabilities"));
        cap[0] = cap[3];    //TODO: nefunguje poslat 4 bajtove capabilities (null bajty), staci 1 bajtove?

        updateTime = par("timer").doubleValue();
        holdTime = par("holdTime");
        version = par("version");
        odr = par("odr");
        routeInvalidTime = par("ODRRouteInvalidTime").doubleValue();
        routeHolddownTime = par("ODRRouteHolddownTime").doubleValue();
        routeFlushTime = par("ODRRouteFlushTime").doubleValue();
        maxDestinationPaths = par("maxDestinationPaths").doubleValue();
        defaultRouteInvalide = par("defaultRouteInvalide").doubleValue();

        CDPTimer *startupTimer = new CDPTimer();
        startupTimer->setTimerType(StartUp);
        scheduleAt(simTime(), startupTimer);
        startCDP();

        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(containingModule->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;

        if(containingModule->getSubmodule("routingTable") != nullptr)
        {
            sendIpPrefixes = true;
            //rt4 = check_and_cast<IPv4RoutingTable*>(containingModule->getSubmodule("routingTable")->getSubmodule("ipv4"));
            rt = getModuleFromPar<IRoutingTable>(par("routingTableModule"), this);
        }
    }
    else if (stage == INITSTAGE_LAST) {
        containingModule->subscribe(NF_INTERFACE_STATE_CHANGED, this);
        containingModule->subscribe(NF_INTERFACE_CREATED, this);
        containingModule->subscribe(NF_INTERFACE_DELETED, this);
        containingModule->subscribe(NF_ROUTE_DELETED, this);
    }
}


///************************ CDP OPERATION ****************************///

void CDP::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG)
{
    Enter_Method_Silent();

    IRoute *route;
    InterfaceEntry *interface;

    if(signalID == NF_INTERFACE_CREATED)
    {
        interface = check_and_cast<const InterfaceEntryChangeDetails *>(obj)->getInterfaceEntry();
        activateInterface(interface);
    }
    else if(signalID == NF_INTERFACE_DELETED)
    {
        interface = check_and_cast<const InterfaceEntryChangeDetails *>(obj)->getInterfaceEntry();
        deactivateInterface(interface);
    }
    else if(signalID == NF_INTERFACE_STATE_CHANGED)
    {
        interface = check_and_cast<const InterfaceEntryChangeDetails *>(obj)->getInterfaceEntry();
        int outputGateid = interface->getNodeOutputGateId();

        if(interface->isUp() && containingModule->gate(outputGateid)->isPathOK())
            activateInterface(interface);
        else
            deactivateInterface(interface);
    }
    else if (signalID == NF_ROUTE_DELETED) {
        // remove references to the deleted route and invalidate the RIP route
        route = const_cast<IRoute *>(check_and_cast<const IRoute *>(obj));
        if (route->getSource() != this) {
            for (auto it = odrRoutes.begin(); it != odrRoutes.end(); ++it)
                if ((*it)->getRoute() == route) {
                    (*it)->setRoute(nullptr);
                    invalidateRoute(*it);
                }
        }
    }
}

bool CDP::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_LINK_LAYER) {
            isOperational = true;
            startCDP();
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_ROUTING_PROTOCOLS){
            for (auto it = odrRoutes.begin(); it != odrRoutes.end(); ++it)
                invalidateRoute(*it);
            // send updates to neighbors
            for (auto it = cdpInterfaceTable.getInterfaces().begin(); it != cdpInterfaceTable.getInterfaces().end(); ++it)
                sendUpdate((*it)->getInterfaceId(), true);

            stopCDP();
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if ((NodeCrashOperation::Stage)stage == NodeCrashOperation::STAGE_CRASH)
            stopCDP();
    }
    else
        throw cRuntimeError("Unsupported operation '%s'", operation->getClassName());

    return true;
}

/**
 * Stop CDP
 */
void CDP::stopCDP()
{
    isOperational = false;

    deleteTimers();

    cdpInterfaceTable.removeInterfaces();
    neighbourTable.clear();
    for (auto it = odrRoutes.begin(); it != odrRoutes.end(); ++it)
        delete (*it);
}

/**
 * Cancel and delete all timers from module.
 * Cdp update and holdtime timer. Odr timers (invalide, holdtime, flush).
 */
void CDP::deleteTimers()
{
    for (auto it = cdpInterfaceTable.getInterfaces().begin(); it != cdpInterfaceTable.getInterfaces().end(); ++it)
        if((*it) != nullptr)
            cancelAndDelete((*it)->getUpdateTimer());
    for (auto it = neighbourTable.begin(); it != neighbourTable.end(); ++it)
        if((*it) != nullptr)
            cancelAndDelete((*it)->getHoldTimeTimer());
    for (auto it = odrRoutes.begin(); it != odrRoutes.end(); ++it)
    {
        if((*it) == nullptr)
            continue;
        cancelAndDelete((*it)->getODRInvalideTime());
        cancelAndDelete((*it)->getODRHoldTime());
        cancelAndDelete((*it)->getODRFlush());
    }
}

/**
 * Starts CDP
 */
void CDP::startCDP()
{
    InterfaceEntry *interface;

    EV << ift->getNumInterfaces() << "\n";
    for(int i=0; i < ift->getNumInterfaces(); i++)
    {
        if(isInterfaceSupported(ift->getInterface(i)->getInterfaceModule()->getName()))
        {
            interface = ift->getInterface(i);
            activateInterface(interface);
        }
    }
}

/**
 * Activate interface. Start update timer.
 */
void CDP::activateInterface(InterfaceEntry *interface)
{
    if(!interface->isUp() || !containingModule->gate(interface->getNodeOutputGateId())->isPathOK())
        return;

    CDPTimer *updateTimer = new CDPTimer();
    updateTimer->setTimerType(UpdateTime);
    updateTimer->setInterfaceId(interface->getInterfaceId());
    scheduleAt(simTime()+1, updateTimer);

    CDPInterface *cdpInterface = new CDPInterface(interface, updateTimer);
    cdpInterfaceTable.addInterface(cdpInterface);

    EV << "Interface " << interface->getName() << " go up, id:" << interface->getInterfaceId() <<  "\n";
}

/**
 * Deactivate interface. Cancel and delete update timer
 * and delete all routes learned from this interface.
 */
void CDP::deactivateInterface(InterfaceEntry *interface)
{
    CDPInterface *cdpInterface = cdpInterfaceTable.findInterfaceById(interface->getInterfaceId());
    if(cdpInterface != nullptr)
    {
        cancelAndDelete(cdpInterface->getUpdateTimer());
        cdpInterfaceTable.removeInterface(cdpInterface->getInterfaceId());

        for (auto it = odrRoutes.begin(); it != odrRoutes.end(); ++it)
        {
            if((*it)->getInterface()->getInterfaceId() == cdpInterface->getInterfaceId())
            {
                purgeRoute(*it);
            }
        }
        EV << "Interface " << interface->getName() << " go down, id: " << interface->getInterfaceId() << "\n";
    }
}

/**
 * Check if interface is supported. Only ethernet and ppp.
 */
bool CDP::isInterfaceSupported(const char *name)
{
    if(strcmp(name, "eth") == 0 || strcmp(name, "ppp") == 0)
        return true;
    else
        return false;
}

///************************ END OF CDP OPERATION ****************************///


///************************ END OF CDP OPERATION ****************************///

/**
 * Delete entry from neighbour table
 *
 * @param   entry   entry to be deleted
 */
void CDP::deleteEntryInTable(CDPTableEntry *entry)
{
    for (auto it = neighbourTable.begin() ; it != neighbourTable.end(); ++it)
    {
        if((*it) == entry)
        {
            delete(*it);
            neighbourTable.erase(it);
            EV << "DELETE ENTRY!!!!!!!!!!!!!!!!!!!!!!!! \n";
            return;
        }
    }
}

/**
 * Get cdp table entry from neighbour table by name and port
 *
 * @param   name    name of module
 * @param   port    port number
 *
 * @return  cdp table entry
 */
CDPTableEntry *CDP::findEntryInTable(std::string name, int port)
{
    for (auto it = neighbourTable.begin() ; it != neighbourTable.end(); ++it)
        if((*it)->getName() == name && (*it)->getPortReceive() == port)
            return *it;

    return NULL;
}

/**
 * Get interface entry by port number
 *
 * @param   portNum     port number
 *
 * @return  interface entry
 */
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

/**
 * Update cdp entry
 *
 * @param   msg     message
 * @param   entry   from which cdp neighbour update came
 */
void CDP::entryUpdate(CDPUpdate *msg, CDPTableEntry *entry)
{
    std::string prefix;
    bool newEntry = false;
    IPv4Address ipAddress, nextHopeIp;
    L3Address l3Address, defaultRoute, nextHope;
    ODRRoute *odrRoute;

    if(countChecksum(msg) != msg->getChecksum())
    {
        EV << "spatny checksum!!!!!!!!!!!!!!!!!!!!!!\n";
        return;
    }

    if(msg->getTtl() == 0)
    {

    }

    //if neighbour is not in cdp table
    if(entry == nullptr)
    {
        EV << "NEW ENTRY " << msg->getTlv(0).getValue() << " " << msg->getArrivalGateId() << "\n";

        newEntry = true;
        entry = new CDPTableEntry();
        entry->setInterface(ift->getInterfaceByNetworkLayerGateIndex(msg->getArrivalGate()->getIndex()));
        entry->setPortReceive(msg->getArrivalGateId());

        //schedule holdtime
        CDPTimer *holdTimeTimer = new CDPTimer();
        holdTimeTimer->setTimerType(HoldTime);
        holdTimeTimer->setContextPointer(entry);
        scheduleAt(simTime() + (simtime_t)msg->getTtl(), holdTimeTimer);
        entry->setHoldTimeTimer(holdTimeTimer);
        entry->setTtl((simtime_t)msg->getTtl());

        neighbourTable.push_back(entry);
    }
    else
    {
        EV << "UPDATE ENTRY" << msg->getTlv(0).getValue() << " " << msg->getArrivalGateId() << "\n";

        //reschedule holdtime
        if (entry->getHoldTimeTimer()->isScheduled())
            cancelEvent(entry->getHoldTimeTimer());
        scheduleAt(simTime() + (simtime_t)msg->getTtl(), entry->getHoldTimeTimer());

        entry->setTtl((simtime_t)msg->getTtl());
    }

    //EV << msg->getArrivalGate()->getIndex() << ift->getInterfaceByNetworkLayerGateIndex(msg->getArrivalGate()->getIndex())->getFullName() << "\n";

    //iterate through all message tlv
    entry->setLastUpdated(simTime());
    for(unsigned int i=0; i < msg->getTlvArraySize(); i++)
    {
        switch(msg->getTlv(i).getType())
        {
            case TLV_DEVICE_ID:
                if(newEntry)
                    entry->setName(msg->getTlv(i).getValue());
                break;
            case TLV_PORT_ID:
                entry->setPortSend(msg->getTlv(i).getValue());
                break;
            case TLV_DUPLEX:
                entry->setFullDuplex(msg->getTlv(i).getValue());
                break;
            case TLV_CAPABILITIES:
                entry->setCapabilities(capabilitiesConvert(msg->getTlv(i).getValue()));
                break;
            case TLV_VERSION:
                entry->setVersion(msg->getTlv(i).getValue());
                break;
            case TLV_ADDRESS:
                prefix.assign(msg->getTlv(i).getValue(), 0, strlen(msg->getTlv(i).getValue()));
                ipAddress.set(stoi (prefix,nullptr,16));
                l3Address.set(ipAddress);
                entry->setAddress(&l3Address);
                break;
            case TLV_PLATFORM:
                entry->setPlatform(msg->getTlv(i).getValue());
                break;
            case TLV_ODR:
                //TODO: co kdyz je odr zapnute
                defaultRoute.set(IPv4Address());

                ipAddress.set(msg->getTlv(i).getValue());
                l3Address.set(ipAddress);

                odrRoute = findRoute(defaultRoute, 0, l3Address);
                if(odrRoute != nullptr)
                {
                    if (odrRoute->getODRInvalideTime()->isScheduled())
                        cancelEvent(odrRoute->getODRInvalideTime());
                    scheduleAt(simTime() + defaultRouteInvalide, odrRoute->getODRInvalideTime());
                }
                else
                {
                    addDefaultRoute(entry->getInterface(), l3Address);
                }
                break;
            case TLV_IP_PREFIX:
                processPrefixes(msg, i, entry);
                break;
        }
    }
}

/**
 * Process all prefixes from update message
 *
 * @param   msg             message
 * @param   tlvPosition     positioin of prefix tlv in message
 * @param   entry           from which cdp neighbour update came
 */
void CDP::processPrefixes(CDPUpdate *msg, int tlvPosition, CDPTableEntry *entry)
{
    if(!odr)
        return;

    const char *prefixes = msg->getTlv(tlvPosition).getValue();
    int begin = 0, prefixLength;
    uint32 address;
    std::string prefix;
    IPv4Address nextHopeIp;
    L3Address nextHope;
    ODRRoute *odrRoute;

    for(unsigned int i = begin; i < strlen(prefixes);
            i+=IPV4_ADDRESS_STRING_LENGTH + NETMASK_STRING_LENGTH)
    {
        prefix.assign(prefixes, begin, IPV4_ADDRESS_STRING_LENGTH);
        address = (uint32) stoi (prefix,nullptr,16);
        begin += IPV4_ADDRESS_STRING_LENGTH;
        prefix.assign(prefixes, begin, NETMASK_STRING_LENGTH);
        prefixLength = stoi (prefix,nullptr,16);
        begin += NETMASK_STRING_LENGTH;

        IPv4Address destIpAddress(address);
        L3Address dest(destIpAddress);

        CDPTLV *tlv = getTlv(msg, TLV_ADDRESS);
        if(tlv != nullptr)
        {
            prefix.assign(tlv->getValue(), 0, strlen(tlv->getValue()));
            nextHopeIp.set(stoi (prefix,nullptr,16));
            nextHope.set(nextHopeIp);
        }
        else
        {
            nextHopeIp.set((uint32)0);
            nextHope.set(nextHopeIp);
        }


        //TODO: ktere routy se nemaji updatovat, kdyz je nastaveny holdtime (vsechny se stejnou siti nebo i ze stejne adresy?)
        odrRoute = findRoute(dest.getPrefix(prefixLength), prefixLength, nextHope);

        if(odrRoute == nullptr)
        {
            if(countDestinationPaths(dest.getPrefix(prefixLength), prefixLength) < maxDestinationPaths)
                addRoute(dest, prefixLength, entry->getInterface(), nextHope);
        }
        else
        {
            if(!odrRoute->isNoUpdates())
            {
                odrRoute->setInvalide(false);
                rescheduleODRRouteTimer(odrRoute);
                odrRoute->setLastUpdateTime(simTime());
            }
        }

    }
    //rt->printRoutingTable();
}

/**
 * Set capability vector
 *
 * @param   property    module capability
 */
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

/**
 * Get position of capability in vector
 *
 * @param   capability  name of capability
 *
 * @return  position
 */
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

/**
 * capabilities char convert to string describing each capability with one letter
 *
 * @param   cap     capabilities
 *
 * @return string of capabilities separated by spaces
 */
std::string CDP::capabilitiesConvert(const char *cap)
{
    std::string out;
    char capLabel[32] = {'R', 'T', 'B', 'S', 'H', 'I', 'r'};

    for(int i=0; i < 8; i++)
        if(cap[0] & (1 << i%8))
            out += " " + capLabel[i];

    if(out.length() > 0)    //delete first space
        out.erase(0, 1);
    return out;
}


///************************ MESSAGE HANDLING ****************************///
/**
 * Send update on interface
 *
 * @param   interfaceId     ID of outgoing interface
 */
void CDP::sendUpdate(int interfaceId, bool shutDown)
{
    CDPUpdate *msg = new CDPUpdate();
    if(!shutDown)
    {
        createTlv(msg, interfaceId);
        msg->setTtl((int)holdTime.dbl());
    }
    else
        msg->setTtl(0);
    msg->setChecksum(countChecksum(msg));

    Ieee802Ctrl *controlInfo = new Ieee802Ctrl();
    controlInfo->setDest(MACAddress("01-00-0c-cc-cc-cc"));
    controlInfo->setInterfaceId(interfaceId);
    msg->setControlInfo(controlInfo);

    send(msg, "ifOut", ift->getInterfaceById(interfaceId)->getNetworkLayerGateIndex());
}

/**
 * Handle received update
 *
 * @param   msg     message
 */
void CDP::handleUpdate(CDPUpdate *msg)
{
    if(msg->getTlvArraySize() == 0  )
        return;
    CDPTableEntry *entry = findEntryInTable(msg->getTlv(0).getValue(), msg->getArrivalGateId());

    entryUpdate(msg, entry);
}

/**
 * Handle timers (self-messages)
 *
 * @param   msg     message
 */
void CDP::handleTimer(CDPTimer *msg)
{
    CDPTableEntry *entry;
    ODRRoute *odrRoute;
    switch(msg->getTimerType())
    {
        case UpdateTime:
            EV << msg->getInterfaceId() << " UPDATETIME!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n";

            //if interface is already shutdown, delete message
            if(cdpInterfaceTable.findInterfaceById(msg->getInterfaceId())->getEnabled())
            {
                sendUpdate(msg->getInterfaceId(), false);
                scheduleAt(simTime() + updateTime, msg);
            }
            else
                delete msg;
            break;
        case HoldTime:
            EV << "HOLDTIME!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n";
            entry = reinterpret_cast<CDPTableEntry *>(msg->getContextPointer());
            deleteEntryInTable(entry);
            delete msg;
            break;
        case StartUp:
            EV << "START UP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n";
            startCDP();
            delete msg;
            break;
        case ODRInvalideTime:
            EV << "ODR INVALIDE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n";
            odrRoute = reinterpret_cast<ODRRoute *>(msg->getContextPointer());

            //if this is not default route
            if(odrRoute->getRoute() != nullptr && !odrRoute->getRoute()->getDestinationAsGeneric().isUnspecified())
            {
                invalidateRoute(odrRoute);
                if (odrRoute->getODRHoldTime()->isScheduled())
                    cancelEvent(odrRoute->getODRHoldTime());
                scheduleAt(simTime() + routeHolddownTime, odrRoute->getODRHoldTime());
            }
            else
            {
                EV << "DEFAULT ROUTE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n";
                // backup default route is deleted
                if(odrRoute->getRoute() != rt->getDefaultRoute())
                {
                    purgeRoute(odrRoute);
                }
                else
                {
                    // in case, that other default route exist, the route is deleted and other default
                    // route is set to main default route
                    ODRRoute *dR = existOtherDefaultRoute(odrRoute);
                    if(dR != nullptr)
                    {
                        purgeRoute(odrRoute);

                        IRoute *route = addRouteToRT(dR->getDestination(), 0, dR->getInterface(), dR->getNextHop());
                        dR->setRoute(route);
                    }
                    else
                    {
                        odrRoute->setInvalide(true);
                    }
                }
            }
            break;
        case ODRHoldTime:
            EV << "ODR HOLDTIME!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n";
            odrRoute = reinterpret_cast<ODRRoute *>(msg->getContextPointer());
            odrRoute->setNoUpdates(false);
            break;
        case ODRFlush:
            EV << "ODR FLUSH!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n";
            odrRoute = reinterpret_cast<ODRRoute *>(msg->getContextPointer());
            cancelAndDelete(odrRoute->getODRHoldTime());
            cancelAndDelete(odrRoute->getODRInvalideTime());
            cancelAndDelete(odrRoute->getODRFlush());
            purgeRoute(odrRoute);
            break;
        default:
            EV << "UNKNOWN TIMER!!!!!!!!!!!!!!!!!!!!!!! \n";
            delete msg;
            break;
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

//TODO: porovnavat dle destination IP addres nebo interfacu

/**
 * count all paths to destination
 *
 * @param   destination
 * @param   prefixLength
 *
 * @return  number of paths
 */
int CDP::countDestinationPaths(const L3Address& destination, int prefixLength)
{
    int destinationPaths = 0;
    for (auto it = odrRoutes.begin(); it != odrRoutes.end(); ++it)
        if ((*it)->getDestination() == destination && (*it)->getPrefixLength() == prefixLength
                && (*it)->getODRFlush())
            destinationPaths++;

    return destinationPaths;
}

/**
 * find odr route by destination, prefix length and next hope
 */
ODRRoute *CDP::findRoute(const L3Address& destination, int prefixLength, const L3Address& nextHope)
{
    for (auto it = odrRoutes.begin(); it != odrRoutes.end(); ++it){
        EV << (*it)->getDestination() << " " << (*it)->getPrefixLength() << " " << (*it)->getNextHop() << "\n";
        EV << destination << " " << prefixLength << " " << nextHope << "\n";
        if ((*it)->getDestination() == destination && (*it)->getPrefixLength() == prefixLength
                && (*it)->getNextHop() == nextHope)
            return *it;
    }
    return nullptr;
}

/**
 * find odr route by route
 */
ODRRoute *CDP::findRoute(const IRoute *route)
{
    for (auto it = odrRoutes.begin(); it != odrRoutes.end(); ++it)
        if ((*it)->getRoute() == route)
            return *it;

    return nullptr;
}

/**
 * return other default route than that in param. In case that no other default route exist
 * return null
 *
 * @param   odrRoute    not look for this route
 *
 * @return  other       default route or null
 */
ODRRoute *CDP::existOtherDefaultRoute(ODRRoute *odrRoute)
{
    for (auto it = odrRoutes.begin(); it != odrRoutes.end(); ++it)
    {
        if ((*it)->getRoute() != nullptr && odrRoute->getRoute() != nullptr &&
                (*it)->getRoute()->getDestinationAsGeneric().isUnspecified() && (*it)->getRoute() != odrRoute->getRoute())
            return *it;
    }

    return nullptr;
}

/**
 * invalidate the route. Delete from routing table and set invalid in odr routing table
 *
 * @param   odrRoute    route to invalidate
 */
void CDP::invalidateRoute(ODRRoute *odrRoute)
{
    IRoute *route = odrRoute->getRoute();
    if (route) {
        odrRoute->setRoute(nullptr);
        rt->deleteRoute(route);
    }
    odrRoute->setInvalide(true);
    odrRoute->setNoUpdates(true);
}

/**
 * Removes the route from the routing table and the odr routing table.
 *
 * @param   odrRoute    odr route to be deleted
 */
void CDP::purgeRoute(ODRRoute *odrRoute)
{
    IRoute *route = odrRoute->getRoute();
    if (route) {
        odrRoute->setRoute(nullptr);
        rt->deleteRoute(route);
    }

    auto end = std::remove(odrRoutes.begin(), odrRoutes.end(), odrRoute);
    if (end != odrRoutes.end())
        odrRoutes.erase(end, odrRoutes.end());
    delete odrRoute;
}

ODRRoute::ODRRoute(L3Address des, int pre, L3Address nex, InterfaceEntry *i)
    : route(nullptr), invalide(false)
{
    dest = des;
    prefixLength = pre;
    nextHop = nex;
    ie = i;
}

ODRRoute::ODRRoute(IRoute *route)
    : route(route), invalide(false)
{
    ODRHoldTime = nullptr;
    ODRFlush = nullptr;
    ODRInvalideTime = nullptr;
    dest = route->getDestinationAsGeneric();
    prefixLength = route->getPrefixLength();
    nextHop = route->getNextHopAsGeneric();
    ie = route->getInterface();
}

/**
 * Reschedule invalide and flush timer from specific odr route.
 * Holdtime timer is just cancel if is scheduled
 *
 * @param   odrRoute    odr route which timers should be rescheduled
 */
void CDP::rescheduleODRRouteTimer(ODRRoute *odrRoute)
{
    //reschedule invalide
    if (odrRoute->getODRInvalideTime()->isScheduled())
        cancelEvent(odrRoute->getODRInvalideTime());
    scheduleAt(simTime() + routeInvalidTime, odrRoute->getODRInvalideTime());

    //reschedule holdtime
    if (odrRoute->getODRHoldTime()->isScheduled())
        cancelEvent(odrRoute->getODRHoldTime());

    //reschedule flush
    if (odrRoute->getODRFlush()->isScheduled())
        cancelEvent(odrRoute->getODRFlush());
    scheduleAt(simTime() + routeFlushTime, odrRoute->getODRFlush());
}

/**
 * Add default route to routing table and odr routing table.
 * If in routing table exist other default route and is marked invalid,
 * the route from routing table is deleted and replaced with route from param.
 * If in routing table exist other default route and is not marked invalid,
 * default route from param is just add to odr routing table.
 * If there is no other default route, route from param is add to be default route.
 *
 * @param   ie          outgoing interface
 * @param   nextHope    next hope address
 */
void CDP::addDefaultRoute(InterfaceEntry *ie, const L3Address& nextHope)
{
    L3Address defaultGateway;
    defaultGateway.set(IPv4Address());
    ODRRoute *defaultRoute = findRoute(rt->getDefaultRoute());

    if(defaultRoute == nullptr || defaultRoute->isInvalide())
    {
        if(defaultRoute != nullptr && defaultRoute->isInvalide())
            purgeRoute(defaultRoute);

        IRoute *route = addRouteToRT(defaultGateway, 0, ie, nextHope);

        defaultRoute = new ODRRoute(route);
        defaultRoute->setLastUpdateTime(simTime());

        CDPTimer *invalideTime = new CDPTimer();
        invalideTime->setTimerType(ODRInvalideTime);
        invalideTime->setContextPointer(defaultRoute);
        defaultRoute->setODRInvalideTime(invalideTime);
        scheduleAt(simTime() + defaultRouteInvalide, defaultRoute->getODRInvalideTime());

        odrRoutes.push_back(defaultRoute);
    }
    else if(!defaultRoute->isInvalide())
    {
        defaultRoute = new ODRRoute(defaultGateway, 0, nextHope, ie);

        CDPTimer *invalideTime = new CDPTimer();
        invalideTime->setTimerType(ODRInvalideTime);
        invalideTime->setContextPointer(defaultRoute);
        defaultRoute->setODRInvalideTime(invalideTime);
        scheduleAt(simTime() + defaultRouteInvalide, defaultRoute->getODRInvalideTime());

        odrRoutes.push_back(defaultRoute);
    }
}

/**
 * Add route to routing table and odr routing table.
 *
 * @param   dest            destination
 * @param   prefixLength    prefix length
 * @param   ie              outgoing interface
 * @param   nextHope        next hope address
 */
void CDP::addRoute(const L3Address& dest, int prefixLength, const InterfaceEntry *ie, const L3Address& nextHope)
{
    EV_DEBUG << "Add route to " << dest << "/" << prefixLength << ": "
             << "nextHop=" << nextHope << " metric=" << 1 << std::endl;

    IRoute *route = addRouteToRT(dest, prefixLength, ie, nextHope);

    ODRRoute *odrRoute = new ODRRoute(route);
    odrRoute->setLastUpdateTime(simTime());

    CDPTimer *invalideTime = new CDPTimer();
    invalideTime->setTimerType(ODRInvalideTime);
    invalideTime->setContextPointer(odrRoute);
    odrRoute->setODRInvalideTime(invalideTime);

    CDPTimer *holdTime = new CDPTimer();
    holdTime->setTimerType(ODRHoldTime);
    //holdTime->setTimerType()
    holdTime->setContextPointer(odrRoute);
    odrRoute->setODRHoldTime(holdTime);

    CDPTimer *flushTime = new CDPTimer();
    flushTime->setTimerType(ODRFlush);
    flushTime->setContextPointer(odrRoute);
    odrRoute->setODRFlush(flushTime);

    rescheduleODRRouteTimer(odrRoute);

    odrRoutes.push_back(odrRoute);
    //TODO: co to je?
    //emit(numRoutesSignal, odrRoutes.size());
}

/**
 * Add route to routing table
 *
 * @param   dest            destination
 * @param   prefixLength    prefix length
 * @param   ie              outgoing interface
 * @param   nextHope        next hope address
 *
 * @return  reference to route
 */
IRoute *CDP::addRouteToRT(const L3Address& dest, int prefixLength, const InterfaceEntry *ie, const L3Address& nextHop)
{
    IRoute *route = rt->createRoute();
    route->setSourceType(IRoute::ODR);
    route->setSource(this);
    route->setDestination(dest.getPrefix(prefixLength));
    route->setPrefixLength(prefixLength);
    route->setInterface(const_cast<InterfaceEntry *>(ie));
    route->setNextHop(nextHop);
    route->setMetric(1);
    ((IPv4Route *)route)->setAdminDist(IPv4Route::dODR);
    rt->addRoute(route);
    return route;
}


///************************ END OF ROUTING ****************************///



///************************ TLV ****************************///


/**
 * Check if there exist other routing protocols
 *
 * @return  true if another routing protocol is enabled
 */
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

/**
 * Get specific tlv from message. If tlv is not in message,
 * return nullptr
 *
 * @param   msg     message
 * @param   type    tlv type
 *
 * @return  tlv or nullptr
 */
CDPTLV *CDP::getTlv(CDPUpdate *msg, CDPTLVType type)
{
    for(unsigned int i=0; i < msg->getTlvArraySize(); i++)
    {
        if(msg->getTlv(i).getType() == type)
            return &msg->getTlv(i);
    }
    return nullptr;
}

/**
 * Get tlv size
 */
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
    tlv->setValue(&cap[3]);

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
        // add to prefixes only interfaces with specified IPv4 address, that are
        // not loopback and are not outgoing interface for this message
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

/**
 * Count the standard IP checksum for message.
 */
uint16_t CDP::countChecksum(CDPUpdate *msg)
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

/**
 * If exist any interface (except interface from param) with specified
 * IP address and that is not loopback.
 *
 * @param   interfaceId     interface id that will not be checked
 */
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

/**
 * Get ip address from interface or 0 if interface have not specified ip address
 *
 * @param   interfaceId     interface ID
 *
 * @return  ip address
 */
int CDP::ipOnInterface(int interfaceId)
{
    if(ift->getInterfaceById(interfaceId)->ipv4Data() != nullptr &&
            !ift->getInterfaceById(interfaceId)->ipv4Data()->getIPAddress().isUnspecified())
    {
        return ift->getInterfaceById(interfaceId)->ipv4Data()->getIPAddress().getInt();
    }
    else
        return 0;
}

void CDP::setTlvAddress(CDPUpdate *msg, int pos, int interfaceId)
{
    CDPTLV *tlv = new CDPTLV();
    tlv->setType(TLV_ADDRESS);
    tlv->setValue(SSTR(ipOnInterface(interfaceId)).c_str());
    tlv->setLength(getTlvSize(tlv));
    msg->setTlv(pos, tlv[0]);
}

/**
 * Create tlv's for interface
 *
 * @param   msg             message
 * @param   interfaceId     interface id
 */
void CDP::createTlv(CDPUpdate *msg, int interfaceId)
{
    int count = 0;
    msg->setTlvArraySize(5);

    setTlvDeviceId(msg, count++);            //device-id
    setTlvPortId(msg, count++, interfaceId); //port-id
    setTlvVersion(msg, count++);             //version
    setTlvCapabilities(msg, count++);        //capabilities
    setTlvPlatform(msg, count++);            //platform

    if(ift->getInterfaceById(interfaceId)->getInterfaceModule()->getSubmodule("mac") != nullptr)
    {
        msg->setTlvArraySize(++count);
        setTlvDuplex(msg, count-1, interfaceId); //full duplex
    }
    if(ipOnInterface(interfaceId) != 0)  //address
    {
        msg->setTlvArraySize(++count);
        setTlvAddress(msg, count-1, interfaceId);
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


