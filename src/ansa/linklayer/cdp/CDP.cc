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

//TODO: nejak poresit vypnuti linky na druhe strane. V gns3 zmizne okamzite z tabulky (TTL=0?)
//TODO: pri vypnuti se posle paket s TTL0 a pouze device ID
//TODO: kdyz se smaze ODR routa z rt z jineho modulu, tak vyvolat nejakou vyjimku a smazat ji i z odrroutes // overit
//TODO: administrativni vzdalenosti
//TODO: kdyz nespecifikuji nextHope (viz obrazek gns3), tak se automaticky v rt nastavi na directly connected
//TODO: jak s address TLV, nelze zkopirovat kvuli 0 polozkam a ani prekopirovat pamet kvuli const

//TODO: handleParameterChange(const char *parname)

//TODO: kdyz se zapne a vypne rozhrani, tak se vymaze jeho nastaveni

//TODO: snap
//TODO: pridat VLAN a VTP
//TODO: CDP can discover up to 256 neighbors per port if the port is connected to a hub with 256 connections.

//INFO: defaultni routa se smaze po 180s, jesi je jedyno, tak sie niesmaze
//INFO: jesi je co posylac prez prefixes tak to posylo, jesi nima zapnone odr
//INFO: jak nima adresa na rozhrani, tak do routovaci tabulki sie prido 0.0.0.0 via rozhrani, albo pyrso polozka z addres TLV
//INFO: jak shutdown interface, tak sie smazom vsystki routy z tego rozhranio
//INFO: na rozhrani kde neni specifikovana adresa se neposila ODR TLV

#include "ansa/linklayer/cdp/CDP.h"
#include "inet/networklayer/contract/IRoute.h"
#include "inet/common/NotifierConsts.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/NotifierConsts.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/networklayer/contract/IRoutingTable.h"

#include "ansa/linklayer/cdp/CDPDeviceConfigurator.h"

#include <stdlib.h>
#include <algorithm>
#include <string>
#include <iostream>

#define SSTR( x ) static_cast< std::ostringstream & >(( std::ostringstream() << std::hex << x ) ).str()
#define TLV_TYPE_AND_LENGHT_SIZE 4
#define IPV4_ADDRESS_STRING_LENGTH 8
#define NETMASK_STRING_LENGTH 2

#define CDP_DEBUG       // print debug information

namespace inet {


Define_Module(CDP);

CDP::CDP() {
    stat.txV1 = stat.txV2 = stat.rxV1 = stat.rxV2 = 0;
    stat.checksumErr = stat.headerErr = 0;
}

CDP::~CDP() {
    //delete tables
    cit.removeInterfaces();
    cnt.removeNeighbours();
    ort.removeRoutes();

    containingModule->unsubscribe(NF_INTERFACE_STATE_CHANGED, this);
    containingModule->unsubscribe(NF_INTERFACE_CREATED, this);
    containingModule->unsubscribe(NF_INTERFACE_DELETED, this);
    containingModule->unsubscribe(NF_ROUTE_DELETED, this);
}

std::string Statistics::info() const
{
    std::stringstream string;
    string << "Output:" << txV1+txV2 << ", Input:" << rxV1+rxV2;
    string << ", ChckErr:" << checksumErr << ", HdrSyn:" << headerErr << endl;
    return string.str();
}

std::string CDP::printStats() const
{
    std::stringstream string;

    string << "Total packets output: " << stat.txV1+stat.txV2 << ", Input: " << stat.rxV1+stat.rxV2 << endl;
    string << "Header syntax: " << stat.headerErr << ", Checksum error: " << stat.checksumErr << endl;
    string << "CDP version 1 advertisements output: " << stat.txV1 << ", Input: " << stat.rxV1 << endl;
    string << "CDP version 2 advertisements output: " << stat.txV2 << ", Input: " << stat.rxV2 << endl;

    return string.str();
}

std::ostream& operator<<(std::ostream& os, const CDPNeighbour& e)
{
    os << e.info();
    return os;
}

std::ostream& operator<<(std::ostream& os, const CDPODRRoute& e)
{
    os << e.info();
    return os;
}

std::ostream& operator<<(std::ostream& os, const CDPInterface& e)
{
    os << e.info();
    return os;
}

std::ostream& operator<<(std::ostream& os, const Statistics& e)
{
    os << e.info();
    return os;
}

void CDP::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        WATCH_PTRVECTOR(cnt.getNeighbours());
        WATCH_PTRVECTOR(cit.getInterfaces());
        WATCH_PTRVECTOR(ort.getRoutes());
        WATCH(stat);
        WATCH(isOperational);

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
        validateVariable();

        if(containingModule->getSubmodule("routingTable") != nullptr)
        {
            sendIpPrefixes = true;
            rt = getModuleFromPar<IRoutingTable>(par("routingTableModule"), this);
        }
    }
    if (stage == INITSTAGE_LAST) {
        containingModule->subscribe(NF_INTERFACE_STATE_CHANGED, this);
        containingModule->subscribe(NF_INTERFACE_CREATED, this);
        containingModule->subscribe(NF_INTERFACE_DELETED, this);
        containingModule->subscribe(NF_ROUTE_DELETED, this);

        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(containingModule->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;

        startCDP();

        CDPDeviceConfigurator *conf = new CDPDeviceConfigurator(par("deviceId"),par("deviceType"),par("configFile"), ift);
        conf->loadCDPConfig(this);

        for (auto it = cit.getInterfaces().begin(); it != cit.getInterfaces().end(); ++it)
        {// through all interfaces
            if((*it)->getEnabled() == false)
                cancelEvent((*it)->getUpdateTimer());
        }
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
        //new interface created
        interface = check_and_cast<const InterfaceEntryChangeDetails *>(obj)->getInterfaceEntry();
        activateInterface(interface);
    }
    else if(signalID == NF_INTERFACE_DELETED)
    {
        //interface deleted
        interface = check_and_cast<const InterfaceEntryChangeDetails *>(obj)->getInterfaceEntry();
        deactivateInterface(interface, true);
    }
    else if(signalID == NF_INTERFACE_STATE_CHANGED)
    {
        //interface state changed
        interface = check_and_cast<const InterfaceEntryChangeDetails *>(obj)->getInterfaceEntry();

        if(interface->isUp())
            activateInterface(interface);
        else
            deactivateInterface(interface, false);
    }
    else if (signalID == NF_ROUTE_DELETED) {
        // remove references to the deleted route and invalidate the RIP route
        route = const_cast<IRoute *>(check_and_cast<const IRoute *>(obj));
        if (route->getSource() != this) {
            for (auto it = ort.getRoutes().begin(); it != ort.getRoutes().end(); ++it)
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
            startCDP();
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_LINK_LAYER){
            //TODO: jak udelat odeslani TTL=0 pred vypnutim
            // send updates to neighbors
            //for (auto it = it.getInterfaces().begin(); it != it.getInterfaces().end(); ++it)
              //  sendUpdate((*it)->getInterfaceId(), true);
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

    cit.removeInterfaces();    //TODO: mazat? kdyz smazu tak smazu i puvodni nastaveni. Jinak pada na TODO 22
    cnt.removeNeighbours();
    ort.removeRoutes();
}

/**
 * Starts CDP
 */
void CDP::startCDP()
{
    InterfaceEntry *interface;
    isOperational = true;

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
    if(!isOperational || interface == nullptr || !interface->isUp())
        return;

    CDPInterface *cdpInterface = cit.findInterfaceById(interface->getInterfaceId());
    if(cdpInterface == nullptr)
    {
        cdpInterface = new CDPInterface(interface);
        cit.addInterface(cdpInterface);
    }

    scheduleAt(simTime(), cdpInterface->getUpdateTimer());  //TODO 22 owner chyba

    EV_INFO << "Interface " << interface->getName() << " go up, id:" << interface->getInterfaceId() <<  endl;
}

/**
 * Deactivate interface. Purge all routes learned from this interface.
 * Neighbours learned from this interface are still in neighbour table (until holdtime expire)
 *
 * @param   interface
 * @param   removeFromTable     if remove from interface table
 */
void CDP::deactivateInterface(InterfaceEntry *interface, bool removeFromTable)
{
    CDPInterface *cdpInterface = cit.findInterfaceById(interface->getInterfaceId());
    if(cdpInterface != nullptr)
    {
        if(removeFromTable)
            cit.removeInterface(cdpInterface->getInterfaceId());

        if(cdpInterface->getUpdateTimer()->isScheduled())
            cancelEvent(cdpInterface->getUpdateTimer());

        for (auto it = ort.getRoutes().begin(); it != ort.getRoutes().end(); ++it)
        {
            if((*it)->getInterface()->getInterfaceId() == cdpInterface->getInterfaceId())
            {
                purgeRoute(*it);
            }
        }
        EV_INFO << "Interface " << interface->getName() << " go down, id: " << interface->getInterfaceId() << endl;
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

/**
 * Validate variables get from module
 */
void CDP::validateVariable()
{
    if(updateTime < 5 || updateTime > 254)
        throw cRuntimeError("Bad value for CDP update time (<5, 254>). The default is 180.");
    if(holdTime < 10 || holdTime > 255)
        throw cRuntimeError("Bad value for CDP hold time (<10, 255>). The default is 180.");
    if(version != 1 && version != 2)
        throw cRuntimeError("Bad value for CDP versiom (1 or 2). The default is 2.");
    if(routeInvalidTime.dbl() < 1.0 || routeInvalidTime.dbl() > 4294967295.0)
        throw cRuntimeError("Bad value for ODR invalid time (<1, 4294967295>)");
    if(routeHolddownTime.dbl() < 0.0 || routeHolddownTime.dbl() > 4294967295)
        throw cRuntimeError("Bad value for ODR holddown time (<0, 4294967295>)");
    if(routeFlushTime.dbl() < 1.0 || routeFlushTime.dbl() > 4294967295.0)
        throw cRuntimeError("Bad value for ODR flush time (<1, 4294967295>)");
    if(maxDestinationPaths < 1)
        throw cRuntimeError("Bad value for ODR max destination paths (<1, ->)");
}

void CDP::finish()
{
#ifdef CDP_DEBUG
    EV_INFO << printStats();
#endif
}


///************************ END OF CDP OPERATION ****************************///


///************************ END OF CDP OPERATION ****************************///

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
 * Update cdp neighbour
 *
 * @param   msg     message
 * @param   entry   from which cdp neighbour update came
 */
void CDP::neighbourUpdate(CDPUpdate *msg, CDPNeighbour *neighbour)
{
    std::string prefix;
    bool newNeighbour = false;
    IPv4Address ipAddress, nextHopeIp;
    L3Address l3Address, defaultRoute, nextHope;
    CDPODRRoute *odrRoute;
    int vers;

    if(countChecksum(msg) != msg->getChecksum())
    {
        EV_ERROR << "Bad checksum. Dropped packet.\n";
        stat.checksumErr++;
        return;
    }

    if(msg->getVersion() == '1')
        stat.rxV1++;
    else if(msg->getVersion() == '2')
        stat.rxV2++;
    else
    {
        EV_ERROR << "Bad checksum. Dropped packet.\n";
        stat.headerErr++;
        return;
    }

    if(msg->getTtl() == 0)
    {
        cnt.removeNeighbour(neighbour);
        EV_INFO << "Neighbour " << msg->getTlv(0).getValue() << " go down. Delete from table" << endl;
        return;
    }

    //if neighbour is not in cdp table
    if(neighbour == nullptr)
    {
        //EV << "NEW ENTRY " << ((CDPTLVString &)msg->getTlv(0)).getValue() << " " << msg->getArrivalGateId() << "\n";
        EV_INFO << "New neighbour " << msg->getTlv(0).getValue() << " " << msg->getArrivalGateId() << endl;

        newNeighbour = true;
        neighbour = new CDPNeighbour();
        neighbour->setInterface(ift->getInterfaceByNetworkLayerGateIndex(msg->getArrivalGate()->getIndex()));
        neighbour->setPortReceive(msg->getArrivalGateId());

        //schedule holdtime
        scheduleAt(simTime() + (simtime_t)msg->getTtl(), neighbour->getHoldtimeTimer());
        neighbour->setTtl((simtime_t)msg->getTtl());

        cnt.addNeighbour(neighbour);
    }
    else
    {
        //EV << "UPDATE ENTRY" << ((CDPTLVString &)msg->getTlv(0)).getValue() << " " << msg->getArrivalGateId() << "\n";
        EV_INFO << "Update neighbour" << msg->getTlv(0).getValue() << " " << msg->getArrivalGateId() << endl;

        //reschedule holdtime
        if (neighbour->getHoldtimeTimer()->isScheduled())
            cancelEvent(neighbour->getHoldtimeTimer());
        scheduleAt(simTime() + (simtime_t)msg->getTtl(), neighbour->getHoldtimeTimer());

        neighbour->setTtl((simtime_t)msg->getTtl());
    }

    //EV << msg->getArrivalGate()->getIndex() << ift->getInterfaceByNetworkLayerGateIndex(msg->getArrivalGate()->getIndex())->getFullName() << "\n";

    //iterate through all message tlv
    neighbour->setLastUpdated(simTime());
    for(unsigned int i=0; i < msg->getTlvArraySize(); i++)
    {
        switch(msg->getTlv(i).getType())
        {
            case CDPTLV_DEV_ID:
                if(newNeighbour)
                    //entry->setName(((CDPTLVString &)msg->getTlv(i)).getValue());
                    neighbour->setName(msg->getTlv(i).getValue());
                break;
            case CDPTLV_PORT_ID:
                //entry->setPortSend(((CDPTLVString &)msg->getTlv(i)).getValue());
                neighbour->setPortSend(msg->getTlv(i).getValue());
                break;
            case CDPTLV_DUPLEX:
                //neighbour->setFullDuplex(((CDPTLVString &)msg->getTlv(i)).getValue());
                neighbour->setFullDuplex(msg->getTlv(i).getValue());
                break;
            case CDPTLV_CAP:
                //neighbour->setCapabilities(capabilitiesConvert(((CDPTLVString &)msg->getTlv(i)).getValue()));
                neighbour->setCapabilities(capabilitiesConvert(msg->getTlv(i).getValue()));
                break;
            case CDPTLV_VERSION:
                //neighbour->setVersion(((CDPTLVString &)msg->getTlv(i)).getValue());
                if(msg->getTlv(i).getValue()[0] == '1')
                    vers = 1;
                else if(msg->getTlv(i).getValue()[0] == '2')
                    vers = 2;
                else
                {
                    vers = 2;
                    EV_ERROR << "Bad version number. Set default version (2).\n";
                }

                neighbour->setVersion(vers);
                break;
            case CDPTLV_ADDRESS:
                //prefix.assign(((CDPTLVString &)msg->getTlv(i)).getValue(), 0, strlen(((CDPTLVString &)msg->getTlv(i)).getValue()));
                prefix.assign(msg->getTlv(i).getValue(), 0, strlen(msg->getTlv(i).getValue()));
                ipAddress.set(stoi (prefix,nullptr,16));
                l3Address.set(ipAddress);
                neighbour->setAddress(&l3Address);
                break;
            case CDPTLV_PLATFORM:
                //neighbour->setPlatform(((CDPTLVString &)msg->getTlv(i)).getValue());
                neighbour->setPlatform(msg->getTlv(i).getValue());
                break;
            case CDPTLV_ODR:
                //TODO: co kdyz je odr zapnute
                defaultRoute.set(IPv4Address());

                //ipAddress.set(((CDPTLVString &)msg->getTlv(i)).getValue());
                ipAddress.set(msg->getTlv(i).getValue());
                l3Address.set(ipAddress);

                odrRoute = ort.findRoute(defaultRoute, 0, l3Address);
                if(odrRoute != nullptr)
                {
                    if (odrRoute->getODRInvalideTime()->isScheduled())
                        cancelEvent(odrRoute->getODRInvalideTime());
                    scheduleAt(simTime() + defaultRouteInvalide, odrRoute->getODRInvalideTime());
                }
                else
                {
                    addDefaultRoute(neighbour->getInterface(), l3Address);
                }
                break;
            case CDPTLV_IP_PREF:
                processPrefixes(msg, i, neighbour);
                break;
        }
    }
}

/**
 * Process all prefixes from update message
 *
 * @param   msg             message
 * @param   tlvPosition     positioin of prefix tlv in message
 * @param   neighbour       from which cdp neighbour update came
 */
void CDP::processPrefixes(CDPUpdate *msg, int tlvPosition, CDPNeighbour *neighbour)
{
    if(!odr)
        return;

    //const char *prefixes = ((CDPTLVString &)msg->getTlv(tlvPosition)).getValue();
    const char *prefixes = msg->getTlv(tlvPosition).getValue();
    int begin = 0, prefixLength;
    uint32 address;
    std::string prefix;
    IPv4Address nextHopeIp;
    L3Address nextHope;
    CDPODRRoute *odrRoute;

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

        CDPTLV *tlv = getTlv(msg, CDPTLV_ADDRESS);
        if(tlv != nullptr)
        {
            prefix.assign(tlv->getValue(), 0, strlen(tlv->getValue()));
            //prefix.assign(tlv->getValue(), 0, strlen(tlv->getValue()));
            nextHopeIp.set(stoi (prefix,nullptr,16));
            nextHope.set(nextHopeIp);
        }
        else
        {
            nextHopeIp.set((uint32)0);
            nextHope.set(nextHopeIp);
        }


        //TODO: ktere routy se nemaji updatovat, kdyz je nastaveny holdtime (vsechny se stejnou siti nebo i ze stejne adresy?)
        odrRoute = ort.findRoute(dest.getPrefix(prefixLength), prefixLength, nextHope);

        if(odrRoute == nullptr)
        {
            if(ort.countDestinationPaths(dest.getPrefix(prefixLength), prefixLength) < maxDestinationPaths)
                addRoute(dest, prefixLength, neighbour->getInterface(), nextHope);
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
    if(version == 1)
        msg->setVersion('1');
    else
        msg->setVersion('2');
    msg->setChecksum(countChecksum(msg));

    Ieee802Ctrl *controlInfo = new Ieee802Ctrl();
    controlInfo->setDest(MACAddress("01-00-0c-cc-cc-cc"));
    controlInfo->setInterfaceId(interfaceId);
    msg->setControlInfo(controlInfo);

    if(version == 1)
        stat.txV1++;
    else
        stat.txV2++;
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

    //CDPTLVString *deviceIdTlv = getTlv(msg, CDPTLVType::TLV_DEVICE_ID);
    //CDPTableEntry *entry = findEntryInTable(deviceIdTlv->getValue(), msg->getArrivalGateId());
    CDPNeighbour *neighbour = cnt.findNeighbour(msg->getTlv(0).getValue(), msg->getArrivalGateId());

    neighbourUpdate(msg, neighbour);
}

/**
 * Handle timers (self-messages)
 *
 * @param   msg     message
 */
void CDP::handleTimer(CDPTimer *msg)
{
    CDPNeighbour *neighbour;
    CDPODRRoute *odrRoute;
    CDPInterface *interface;
    simtime_t delay;

    switch(msg->getTimerType())
    {
        case UpdateTime:
            interface = reinterpret_cast<CDPInterface *>(msg->getContextPointer());

            //if interface is already shutdown
            if(interface->getEnabled())
            {
                EV_DETAIL << "Update time on interface " << interface->getInterface()->getFullName() << endl;
                sendUpdate(interface->getInterfaceId(), false);

                delay = updateTime;
                if(interface->getFastStart() > 0)
                {
                    delay = 1;
                    interface->decFastStart();
                }
                scheduleAt(simTime() + delay, msg);
            }
            else
            {
                EV_DETAIL << "CDP don't run on interface " << interface->getInterface()->getFullName() << endl;
            }
            break;
        case Holdtime:
            neighbour = reinterpret_cast<CDPNeighbour *>(msg->getContextPointer());
            EV_DETAIL << "Holdtime on neighbour with device ID: " << neighbour->getName() << endl;

            cnt.removeNeighbour(neighbour);
            break;
        case ODRInvalideTime:
            odrRoute = reinterpret_cast<CDPODRRoute *>(msg->getContextPointer());

            //if this is not default route
            if(odrRoute->getRoute() != nullptr && !odrRoute->getRoute()->getDestinationAsGeneric().isUnspecified())
            {
                EV_DETAIL << "ODR invalide time on route with destination " << odrRoute->getDestination().str() <<
                        " and next hope " << odrRoute->getNextHop().str() << endl;

                invalidateRoute(odrRoute);
                if (odrRoute->getODRHolddown()->isScheduled())
                    cancelEvent(odrRoute->getODRHolddown());
                scheduleAt(simTime() + routeHolddownTime, odrRoute->getODRHolddown());
            }
            else
            {
                EV_DETAIL << "ODR invalide time on default route with destination " << odrRoute->getDestination().str() << endl;

                // backup default route is deleted
                if(odrRoute->getRoute() != rt->getDefaultRoute())
                {
                    purgeRoute(odrRoute);
                }
                else
                {
                    // in case, that other default route exist, the route is deleted and other default
                    // route is set to main default route
                    CDPODRRoute *dR = ort.existOtherDefaultRoute(odrRoute);
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
        case ODRHolddown:
            odrRoute = reinterpret_cast<CDPODRRoute *>(msg->getContextPointer());
            EV_DETAIL << "ODR hold time on route with destination " << odrRoute->getDestination().str() <<
                        " and next hope " << odrRoute->getNextHop().str() << endl;

            odrRoute->setNoUpdates(false);
            break;
        case ODRFlush:
            odrRoute = reinterpret_cast<CDPODRRoute *>(msg->getContextPointer());
            EV_DETAIL << "ODR flush on route with destination " << odrRoute->getDestination().str() <<
                        " and next hope " << odrRoute->getNextHop().str() << endl;

            purgeRoute(odrRoute);
            break;
        default:
            throw cRuntimeError("Self-message with unexpected message kind %d", msg->getKind());

            delete msg;
            break;
    }
}

void CDP::handleMessage(cMessage *msg)
{
    if (!isOperational) {
        EV_WARN << "Message '" << msg << "' arrived when module status is down, dropped it." << endl;
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
        throw cRuntimeError("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }
}
///************************ END OF MESSAGE HANDLING ****************************///


///************************ ROUTING ****************************///

//TODO: porovnavat dle destination IP addres nebo interfacu


/**
 * invalidate the route. Delete from routing table and set invalid in odr routing table
 *
 * @param   odrRoute    route to invalidate
 */
void CDP::invalidateRoute(CDPODRRoute *odrRoute)
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
void CDP::purgeRoute(CDPODRRoute *odrRoute)
{
    IRoute *route = odrRoute->getRoute();
    if (route) {
        odrRoute->setRoute(nullptr);
        rt->deleteRoute(route);
    }

    ort.removeRoute(odrRoute);
}


/**
 * Reschedule invalide and flush timer from specific odr route.
 * Holdtime timer is just cancel if is scheduled
 *
 * @param   odrRoute    odr route which timers should be rescheduled
 */
void CDP::rescheduleODRRouteTimer(CDPODRRoute *odrRoute)
{
    //reschedule invalide
    if (odrRoute->getODRInvalideTime()->isScheduled())
        cancelEvent(odrRoute->getODRInvalideTime());
    scheduleAt(simTime() + routeInvalidTime, odrRoute->getODRInvalideTime());

    //reschedule holdtime
    if (odrRoute->getODRHolddown()->isScheduled())
        cancelEvent(odrRoute->getODRHolddown());

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
    CDPODRRoute *defaultRoute = ort.findRoute(rt->getDefaultRoute());

    if(defaultRoute == nullptr || defaultRoute->isInvalide())
    {
        if(defaultRoute != nullptr && defaultRoute->isInvalide())
            purgeRoute(defaultRoute);

        IRoute *route = addRouteToRT(defaultGateway, 0, ie, nextHope);

        defaultRoute = new CDPODRRoute(route);
        defaultRoute->setLastUpdateTime(simTime());
        ort.addRoute(defaultRoute);

        scheduleAt(simTime() + defaultRouteInvalide, defaultRoute->getODRInvalideTime());
    }
    else if(!defaultRoute->isInvalide())
    {
        defaultRoute = new CDPODRRoute(defaultGateway, 0, nextHope, ie);
        defaultRoute->setLastUpdateTime(simTime());
        ort.addRoute(defaultRoute);

        scheduleAt(simTime() + defaultRouteInvalide, defaultRoute->getODRInvalideTime());
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
    EV_INFO << "Add route to " << dest << "/" << prefixLength << ": "
             << "nextHop=" << nextHope << " metric=" << 1 << endl;

    IRoute *route = addRouteToRT(dest, prefixLength, ie, nextHope);

    CDPODRRoute *odrRoute = new CDPODRRoute(route);
    odrRoute->setLastUpdateTime(simTime());

    rescheduleODRRouteTimer(odrRoute);

    ort.addRoute(odrRoute);
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
    tlv->setType(CDPTLV_DEV_ID);
    tlv->setValue(containingModule->getFullName());
    tlv->setLength(getTlvSize(tlv));
    msg->setTlv(pos, tlv[0]);
}

void CDP::setTlvPortId(CDPUpdate *msg, int pos, int interfaceId)
{
    CDPTLV *tlv = new CDPTLV();
    tlv->setType(CDPTLV_PORT_ID);
    tlv->setValue(ift->getInterfaceById(interfaceId)->getFullName());
    tlv->setLength(getTlvSize(tlv));
    msg->setTlv(pos, tlv[0]);
}

void CDP::setTlvPlatform(CDPUpdate *msg, int pos)
{
    CDPTLV *tlv = new CDPTLV();
    tlv->setType(CDPTLV_PLATFORM);
    tlv->setValue(containingModule->getComponentType()->getName());
    tlv->setLength(getTlvSize(tlv));
    msg->setTlv(pos, tlv[0]);
}

void CDP::setTlvVersion(CDPUpdate *msg, int pos)
{
    CDPTLV *tlv = new CDPTLV();
    tlv->setType(CDPTLV_VERSION);
    tlv->setValue("1.0");
    tlv->setLength(getTlvSize(tlv));
    msg->setTlv(pos, tlv[0]);
}

void CDP::setTlvCapabilities(CDPUpdate *msg, int pos)
{
    CDPTLV *tlv = new CDPTLV();
    tlv->setType(CDPTLV_CAP);
    tlv->setValue(&cap[3]);

    tlv->setLength(4 + TLV_TYPE_AND_LENGHT_SIZE);
    msg->setTlv(pos, tlv[0]);
}

void CDP::setTlvDuplex(CDPUpdate *msg, int pos, int interfaceId)
{
    CDPTLV *tlv = new CDPTLV();
    tlv->setType(CDPTLV_DUPLEX);

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
    tlv->setType(CDPTLV_ODR);
    tlv->setValue(ift->getInterfaceById(interfaceId)->getNetworkAddress().str().c_str());
    tlv->setLength(getTlvSize(tlv));
    msg->setTlv(pos, tlv[0]);
}

void CDP::setTlvIpPrefix(CDPUpdate *msg, int pos, int interfaceId)
{
    CDPTLV *tlv = new CDPTLV();
    tlv->setType(CDPTLV_IP_PREF);
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
    CDPTLV *tlv;

    a += msg->getVersion();
    a += msg->getTtl();
    for(unsigned int i=0; i < msg->getTlvArraySize(); i++)
    {
        a += msg->getTlv(i).getType();
        a += msg->getTlv(i).getLength();

        //tlv = &((CDPTLVString &)msg->getTlv(i));
        //a += tlv->getValue();
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
    tlv->setType(CDPTLV_ADDRESS);
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
    msg->setTlvArraySize(CDPTLVType::CDPTLVCOUNT);

    setTlvDeviceId(msg, count++);            //device-id
    setTlvPortId(msg, count++, interfaceId); //port-id
    setTlvVersion(msg, count++);             //version
    setTlvCapabilities(msg, count++);        //capabilities
    setTlvPlatform(msg, count++);            //platform

    if(version == 2 &&
       ift->getInterfaceById(interfaceId)->getInterfaceModule()->getSubmodule("mac") != nullptr)
        setTlvDuplex(msg, count++, interfaceId);    //full duplex

    if(ipOnInterface(interfaceId) != 0)
        setTlvAddress(msg, count++, interfaceId);   //address

    if(odr)
        setTlvODR(msg, count++, interfaceId);       //odr hub
    else if(sendIpPrefixes && ipInterfaceExist(interfaceId) && !hasRoutingProtocol())  //ip prefix
        setTlvIpPrefix(msg, count++, interfaceId);  //prefixes

    msg->setTlvArraySize(count);
}

///************************ END OF TLV ****************************///



} /* namespace inet */


