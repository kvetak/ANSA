//
// Copyright (C) 2009 - today, Brno University of Technology, Czech Republic
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
* @file CDPMain.cc
* @authors Tomas Rajca, Vladimir Vesely (ivesely@fit.vutbr.cz)
* @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
*/

#include "ansa/linklayer/cdp/CDPMain.h"
#include "inet/common/Simsignals.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/contract/IRoutingTable.h"

#include "ansa/linklayer/cdp/CDPDeviceConfigurator.h"

namespace inet {

Define_Module(CDPMain);

CDPMain::CDPMain() {
    stat.txV1 = stat.txV2 = stat.rxV1 = stat.rxV2 = 0;
    stat.checksumErr = stat.headerErr = 0;
}

CDPMain::~CDPMain() {
    containingModule->unsubscribe(interfaceStateChangedSignal, this);
    containingModule->unsubscribe(interfaceCreatedSignal, this);
    containingModule->unsubscribe(interfaceDeletedSignal, this);
    containingModule->unsubscribe(routeDeletedSignal, this);
}

std::string Statistics::info() const
{
    std::stringstream string;
    string << "Output:" << txV1+txV2 << ", Input:" << rxV1+rxV2;
    string << ", ChckErr:" << checksumErr << ", HdrSyn:" << headerErr << endl;
    return string.str();
}

std::string CDPMain::printStats() const
{
    std::stringstream string;

    string << "Total packets output: " << stat.txV1+stat.txV2 << ", Input: " << stat.rxV1+stat.rxV2 << endl;
    string << "Header syntax: " << stat.headerErr << ", Checksum error: " << stat.checksumErr << endl;
    string << "CDP version 1 advertisements output: " << stat.txV1 << ", Input: " << stat.rxV1 << endl;
    string << "CDP version 2 advertisements output: " << stat.txV2 << ", Input: " << stat.rxV2 << endl;

    return string.str();
}

std::ostream& operator<<(std::ostream& os, const Statistics& e)
{
    os << e.info();
    return os;
}

void CDPMain::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        containingModule = findContainingNode(this);
        if (!containingModule)
            throw cRuntimeError("Containing @networkNode module not found");
        ift = check_and_cast<IInterfaceTable *>(containingModule->getSubmodule("interfaceTable"));

        getCapabilities(containingModule->getProperties()->get("capabilities"));

        updateTime = par("timer").doubleValue();
        holdTime = par("holdTime").doubleValue();
        version = par("version");
        odr = par("odr");
        routeInvalidTime = par("ODRRouteInvalidTime").doubleValue();
        routeHolddownTime = par("ODRRouteHolddownTime").doubleValue();
        routeFlushTime = par("ODRRouteFlushTime").doubleValue();
        maxDestinationPaths = par("maxDestinationPaths").doubleValue();
        defaultRouteInvalide = par("defaultRouteInvalide").doubleValue();
        validateVariable();

        cnt = getModuleFromPar<CDPNeighbourTable>(par("cdpNeighbourTableModule"), this);
        cit = getModuleFromPar<CDPInterfaceTable>(par("cdpInterfaceTableModule"), this);
        ort = getModuleFromPar<CDPODRRouteTable>(par("cdpODRRouteTableModule"), this);
        if(containingModule->getSubmodule("routingTable") != nullptr)
            rt = getModuleFromPar<IRoutingTable>(par("routingTableModule"), this);

        if(odr)
        {
            WATCH(routeInvalidTime);
            WATCH(routeHolddownTime);
            WATCH(routeFlushTime);
        }
        WATCH_PTRVECTOR(cnt->getNeighbours());
        WATCH_PTRVECTOR(cit->getInterfaces());
        WATCH_PTRVECTOR(ort->getRoutes());
        WATCH(stat);
        WATCH(isOperational);
    }
    if (stage == INITSTAGE_LAST) {
        containingModule->subscribe(interfaceStateChangedSignal, this);
        containingModule->subscribe(interfaceCreatedSignal, this);
        containingModule->subscribe(interfaceDeletedSignal, this);
        containingModule->subscribe(routeDeletedSignal, this);

        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(containingModule->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;

        startCDP();

        CDPDeviceConfigurator *conf = new CDPDeviceConfigurator(par("deviceId"),par("deviceType"),par("configFile"), ift);
        conf->loadCDPConfig(this);

        for (auto it = cit->getInterfaces().begin(); it != cit->getInterfaces().end(); ++it)
            if((*it)->isCDPEnabled() == false)
                cancelEvent((*it)->getUpdateTimer());
    }
}


///************************ CDP OPERATION ****************************///

void CDPMain::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method_Silent();

    IRoute *route;
    InterfaceEntry *interface;

    if(signalID == interfaceCreatedSignal)
    {
        //new interface created
        interface = check_and_cast<const InterfaceEntryChangeDetails *>(obj)->getInterfaceEntry();
        activateInterface(interface);
    }
    else if(signalID == interfaceDeletedSignal)
    {
        //interface deleted
        interface = check_and_cast<const InterfaceEntryChangeDetails *>(obj)->getInterfaceEntry();
        deactivateInterface(interface, true);
    }
    else if(signalID == interfaceStateChangedSignal)
    {
        //interface state changed
        interface = check_and_cast<const InterfaceEntryChangeDetails *>(obj)->getInterfaceEntry();

        if(interface->isUp())
        {
            activateInterface(interface);
            CDPInterface *cdpInterface = cit->findInterfaceById(interface->getInterfaceId());
            if(cdpInterface != nullptr)
                cdpInterface->setFastStart();
        }
        else
            deactivateInterface(interface, false);
    }
    else if (signalID == routeDeletedSignal) {
        // remove references to the deleted route and invalidate the RIP route
        route = const_cast<IRoute *>(check_and_cast<const IRoute *>(obj));
        if (route->getSource() != this) {
            for (auto it = ort->getRoutes().begin(); it != ort->getRoutes().end(); ++it)
                if ((*it)->getRoute() == route) {
                    (*it)->setRoute(nullptr);
                    invalidateRoute(*it);
                }
        }
    }
}

bool CDPMain::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_LAST) {
            startCDP();
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_LOCAL){
            //TODO: interface already down, packet dropped
            // send updates to neighbors
            for (auto it = cit->getInterfaces().begin(); it != cit->getInterfaces().end(); ++it)
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

void CDPMain::stopCDP()
{
    isOperational = false;

    for (auto it = cit->getInterfaces().begin(); it != cit->getInterfaces().end(); ++it)
        cancelEvent((*it)->getUpdateTimer());

    // not remove interfaces to remember configuration
    //cit->removeInterfaces();
    cnt->removeNeighbours();
    ort->removeRoutes();
}

void CDPMain::startCDP()
{
    InterfaceEntry *interface;
    isOperational = true;

    for(int i=0; i < ift->getNumInterfaces(); i++)
    {
        if(isInterfaceSupported(ift->getInterface(i)->cModule::getName()))
        {
            interface = ift->getInterface(i);
            activateInterface(interface);
        }
    }
}

void CDPMain::activateInterface(InterfaceEntry *interface)
{
    if(!isOperational || interface == nullptr || !interface->isUp())
        return;

    CDPInterface *cdpInterface = cit->findInterfaceById(interface->getInterfaceId());
    if(cdpInterface == nullptr)
    {
        cdpInterface = new CDPInterface(interface);
        cit->addInterface(cdpInterface);
    }
    cdpInterface->setFastStart();

    scheduleAt(simTime(), cdpInterface->getUpdateTimer());

    EV_INFO << "Interface " << interface->getInterfaceName() << " go up, id:" << interface->getInterfaceId() <<  endl;
}

void CDPMain::deactivateInterface(InterfaceEntry *interface, bool removeFromTable)
{
    CDPInterface *cdpInterface = cit->findInterfaceById(interface->getInterfaceId());
    if(cdpInterface != nullptr)
    {
        if(removeFromTable)
            cit->removeInterface(cdpInterface->getInterfaceId());

        if(cdpInterface->getUpdateTimer()->isScheduled())
            cancelEvent(cdpInterface->getUpdateTimer());

        for (auto it = ort->getRoutes().begin(); it != ort->getRoutes().end();)
        {
            if((*it)->getInterface()->getInterfaceId() == cdpInterface->getInterfaceId())
            {
                purgeRoute(*it);
                it = ort->getRoutes().begin();
            }
            else
                ++it;
        }
        EV_INFO << "Interface " << interface->getInterfaceName() << " go down, id: " << interface->getInterfaceId() << endl;
    }
}

bool CDPMain::isInterfaceSupported(const char *name)
{
    if(strcmp(name, "eth") == 0 || strcmp(name, "ppp") == 0)
        return true;
    else
        return false;
}

void CDPMain::validateVariable()
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

void CDPMain::finish()
{
    EV_INFO << printStats();
}


///************************ END OF CDP OPERATION ****************************///


///************************ END OF CDP OPERATION ****************************///

InterfaceEntry *CDPMain::getPortInterfaceEntry(unsigned int portNum)
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

void CDPMain::neighbourUpdate(CDPUpdate *msg)
{
    std::string prefix;
    bool newNeighbour = false;
    Ipv4Address ipAddress, nextHopeIp;
    L3Address l3Address, defaultRoute, nextHope;
    CDPODRRoute *odrRoute;

    const CDPOptionDevId *deviceIdOption = check_and_cast<const CDPOptionDevId *> (msg->findOptionByType(CDPTLV_DEV_ID, 0));
    CDPNeighbour *neighbour = cnt->findNeighbour(deviceIdOption->getValue(), msg->getArrivalGateId());
    // shutdown packet
    if(msg->getTtl() == 0)
    {
        cnt->removeNeighbour(neighbour);
        EV_INFO << "Neighbour " << neighbour->getName() << " go down. Delete from table" << endl;
        return;
    }

    // neighbour is not in cdp table
    if(neighbour == nullptr)
    {
        //can discover up to 256 neighbors per port
        if(cnt->countNeighboursOnPort(msg->getArrivalGateId()) > 256)
        {
            EV_WARN << "Can discover up to 256 neighbors per port" << endl;
            return;
        }

        newNeighbour = true;
        neighbour = new CDPNeighbour();
        neighbour->setInterface(ift->getInterfaceByNetworkLayerGateIndex(msg->getArrivalGate()->getIndex()));
        neighbour->setPortReceive(msg->getArrivalGateId());

        const CDPOptionDevId *deviceIdOption = check_and_cast<const CDPOptionDevId *> (msg->findOptionByType(CDPTLV_DEV_ID, 0));
        EV_INFO << "New neighbour " << deviceIdOption->getValue() << " on interface " << neighbour->getInterface()->getFullName() << endl;

        // schedule holdtime
        scheduleAt(simTime() + (simtime_t)msg->getTtl(), neighbour->getHoldtimeTimer());
        neighbour->setTtl((simtime_t)msg->getTtl());

        cnt->addNeighbour(neighbour);
    }
    else
    {
        EV_INFO << "Update neighbour " << neighbour->getName() << " on interface " << neighbour->getInterface()->getFullName() << endl;

        //reschedule holdtime
        if (neighbour->getHoldtimeTimer()->isScheduled())
            cancelEvent(neighbour->getHoldtimeTimer());
        scheduleAt(simTime() + (simtime_t)msg->getTtl(), neighbour->getHoldtimeTimer());

        neighbour->setTtl((simtime_t)msg->getTtl());
    }

    //iterate through all message tlv
    neighbour->setLastUpdated(simTime());
    for(unsigned int i=0; i < msg->getOptionArraySize(); i++)
    {
        const TlvOptionBase *option = msg->getOption(i);
        switch(msg->getOption(i)->getType())
        {
            case CDPTLV_DEV_ID: {
                if(newNeighbour)
                {
                    const CDPOptionDevId *opt = check_and_cast<const CDPOptionDevId *> (option);
                    neighbour->setName(opt->getValue());
                }
                break;
            }

            case CDPTLV_PORT_ID: {
                const CDPOptionPortId *opt = check_and_cast<const CDPOptionPortId *> (option);
                neighbour->setPortSend(opt->getValue());
                break;
            }

            case CDPTLV_DUPLEX: {
                if(version == 2)
                {
                    const CDPOptionDupl *opt = check_and_cast<const CDPOptionDupl *> (option);
                    neighbour->setFullDuplex(opt->getFullDuplex());
                    int ifaceId = neighbour->getInterface()->getInterfaceId();
                    if(ift->getInterfaceById(ifaceId)->getSubmodule("mac") != nullptr)
                    {
                        if(neighbour->getFullDuplex() != (bool)ift->getInterfaceById(ifaceId)->getSubmodule("mac")->par("duplexMode"))
                            EV_WARN << "Duplex mode mismatch on port " << neighbour->getInterface()->getFullName() << endl;
                    }
                }
                break;
            }

            case CDPTLV_CAP: {
                const CDPOptionCapa *opt = check_and_cast<const CDPOptionCapa *> (option);
                neighbour->setCapabilities(capabilitiesConvert(opt->getCap(0), opt->getCap(1),
                                                                opt->getCap(2), opt->getCap(3)));
                break;
            }

            case CDPTLV_VERSION: {
                const CDPOptionVersion *opt = check_and_cast<const CDPOptionVersion *> (option);
                neighbour->setVersion(opt->getValue());
                break;
            }

            case CDPTLV_ADDRESS: {
                const CDPOptionAddr *opt = check_and_cast<const CDPOptionAddr *> (option);

                if(opt->getAddresses(0).getProtocolType() == 1 &&
                        opt->getAddresses(0).getProtocol(0) == (uint8_t)204)
                {
                    prefix.assign(opt->getAddresses(0).getAddress(), 0, strlen(opt->getAddresses(0).getAddress()));
                    ipAddress.set(stoi (prefix,nullptr,16));
                    l3Address.set(ipAddress);
                    neighbour->setAddress(&l3Address);
                }
                else
                    EV_INFO << "Unknown address protocol" << endl;
                break;
            }

            case CDPTLV_PLATFORM: {
                const CDPOptionPlatform *opt = check_and_cast<const CDPOptionPlatform *> (option);
                neighbour->setPlatform(opt->getValue());
                break;
            }

            case CDPTLV_NAT_VLAN: {
                if(version == 2)
                {
                    const CDPOptionNumb *opt = check_and_cast<const CDPOptionNumb *> (option);
                    neighbour->setNativeVlan(opt->getNumber());
                    //if()
                    //EV_WARN << "Native VLAN mismatch on port " << neighbour->getInterface()->getFullName() << endl;
                }
                break;
            }

            case CDPTLV_VTP: {
                if(version == 2)
                {
                    const CDPOptionVtp *opt = check_and_cast<const CDPOptionVtp *> (option);
                    neighbour->setVtpDomain(opt->getValue());
                }
                break;
            }

            case CDPTLV_IP_PREF: {
                if(rt == nullptr)
                    break;

                if(odr)
                {
                    processPrefixes(msg, i, neighbour);
                }
                else
                {
                    if(dynamic_cast<const CDPOptionODRDef *> (option))
                    {
                        const CDPOptionODRDef *opt = check_and_cast<const CDPOptionODRDef *> (option);

                        defaultRoute.set(Ipv4Address());

                        ipAddress.set(opt->getDefaultRoute());
                        l3Address.set(ipAddress);

                        odrRoute = ort->findRoute(defaultRoute, 0, l3Address);
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
                    }
                }
                break;
            }
        }
    }
}

void CDPMain::processPrefixes(CDPUpdate *msg, int tlvPosition, CDPNeighbour *neighbour)
{
    if(!odr)
        return;

    uint32_t mask;
    Ipv4Address nextHopeIp;
    L3Address nextHope;
    CDPODRRoute *odrRoute;

    // get next hope from TLV_ADDRESS
    const TlvOptionBase *option = msg->findOptionByType(CDPTLV_ADDRESS, 0);
    if(option != nullptr)
    {
        const CDPOptionAddr *optAddress = check_and_cast<const CDPOptionAddr *> (option);

        if(optAddress->getAddresses(0).getProtocolType() == 1 &&
                        optAddress->getAddresses(0).getProtocol(0) == (uint8_t)204)
        {
            std::string address;
            address.assign(optAddress->getAddresses(0).getAddress(), 0, strlen(optAddress->getAddresses(0).getAddress()));
            nextHopeIp.set(stoi (address,nullptr,16));
        }
        else
            nextHopeIp.set((uint32)0);
    }
    else
    {
        nextHopeIp.set((uint32)0);
    }
    nextHope.set(nextHopeIp);

    // get tlv prefixes
    option = msg->getOption(tlvPosition);
    const CDPOptionPref *opt = check_and_cast<const CDPOptionPref *> (option);
    const prefixType *prefix;

    for(unsigned int i = 0; i < opt->getPrefixesArraySize(); ++i)
    {// through all prefixes
        prefix = &opt->getPrefixes(i);
        mask = (uint32_t)prefix->getMask();

        Ipv4Address destIpAddress(prefix->getNetwork());
        L3Address dest(destIpAddress);

        odrRoute = ort->findRoute(dest.getPrefix(mask), mask, nextHope);
        if(odrRoute == nullptr)
        {// add new route
            if(ort->countDestinationPaths(dest.getPrefix(mask), mask) < maxDestinationPaths)
                addRoute(dest, mask, neighbour->getInterface(), nextHope);
        }
        else
        {// update route
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

void CDPMain::getCapabilities(cProperty *property)
{
    int capabilityPos;
    cap[0]=cap[1]=cap[2]=cap[3]=0;

    if(property != nullptr)
    {
        for(int i=0; i < property->getNumValues(""); i++)
        {
            capabilityPos = capabilitiesPosition(property->getValue("", i));
            if(capabilityPos != -1)
                cap[3-capabilityPos/8] |= 1 << capabilityPos%8;
        }
    }
}

int CDPMain::capabilitiesPosition(std::string capability)
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

std::string CDPMain::capabilitiesConvert(char cap1, char cap2, char cap3, char cap4)
{
    std::string out;
    std::stringstream s;
    char capLabel[32] = {'R', 'T', 'B', 'S', 'H', 'I', 'r'};
    char capGet[4] = {cap1, cap2, cap3, cap4};

    for(int i=0; i < 8; i++)
        if(capGet[3-i/8] & (1 << i%8))
            s << " " << capLabel[i];

    out = s.str();
    if(out.length() > 0)    //delete first space
        out.erase(0, 1);
    return out;
}


///************************ MESSAGE HANDLING ****************************///

void CDPMain::sendUpdate(int interfaceId, bool shutDown)
{
    CDPUpdate *msg = new CDPUpdate();
    if(!shutDown)
    {
        createTlv(msg, interfaceId); // set all TLV
        msg->setTtl((int)holdTime.dbl());
    }
    else
    {
        setTlvDeviceId(msg, 0);     // set device ID
        msg->setTtl(0);
    }

    // set CDP version
    if(version == 1) msg->setVersion('1');
    else msg->setVersion(2);

    // count and set checksum
    msg->setChecksum(msg->countChecksum());

    // set control info
    Ieee802Ctrl *controlInfo = new Ieee802Ctrl();
    controlInfo->setDest(MacAddress("01-00-0c-cc-cc-cc"));
    controlInfo->setInterfaceId(interfaceId);
    msg->setControlInfo(controlInfo);

    // update statistics
    if(version == 1) stat.txV1++;
    else stat.txV2++;

    // set packet length
    short length = 0;
    for(unsigned int i = 0; i < msg->getOptionArraySize(); i++)
        length += msg->getOptionLength(msg->getOption(i));
    length += sizeof(length)*msg->getOptionArraySize();     // length fields in all TLV
    length += 1 + 1 + 2;               // version, ttl and checksum
    msg->setByteLength(length);

    send(msg, "ifOut", ift->getInterfaceById(interfaceId)->getNetworkLayerGateIndex());
}

void CDPMain::handleUpdate(CDPUpdate *msg)
{
    if(msg->getOptionArraySize() == 0  )
    {
        EV_INFO << "Receive packet without TLV. Dropped packet" << endl;
        return;
    }

    int ifaceId = ift->getInterfaceByNetworkLayerGateIndex(msg->getArrivalGate()->getIndex())->getInterfaceId();
    CDPInterface * interface = cit->findInterfaceById(ifaceId);
    if(interface == nullptr || !interface->isCDPEnabled())
    {
        EV_INFO << "Receive packet on interface with disabled interface. Dropped packet" << msg->getArrivalGateId() << endl;
        return;
    }

    // count and check checksum
    if(msg->countChecksum() != msg->getChecksum())
    {
        EV_ERROR << "Bad checksum. Dropped packet" << endl;
        stat.checksumErr++;
        return;
    }

    // check version and update statistics
    if(msg->getVersion() == 1)
        stat.rxV1++;
    else if(msg->getVersion() == 2)
        stat.rxV2++;
    else
    {
        EV_ERROR << "Bad version. Dropped packet" << endl;
        stat.headerErr++;
        return;
    }


    neighbourUpdate(msg);
}

void CDPMain::handleTimer(CDPTimer *msg)
{
    CDPNeighbour *neighbour;
    CDPODRRoute *odrRoute;
    CDPInterface *interface;
    simtime_t delay;

    switch(msg->getTimerType())
    {
        case CDPUpdateTime:
            interface = reinterpret_cast<CDPInterface *>(msg->getContextPointer());

            //if interface is already shutdown
            if(interface->isCDPEnabled())
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
                EV_DETAIL << "CDP not enabled interface " << interface->getInterface()->getFullName() << endl;
            }
            break;
        case CDPHoldtime:
            neighbour = reinterpret_cast<CDPNeighbour *>(msg->getContextPointer());
            EV_DETAIL << "Delete neighbour with device ID: " << neighbour->getName() << endl;

            cnt->removeNeighbour(neighbour);
            break;
        case CDPODRInvalideTime:
            odrRoute = reinterpret_cast<CDPODRRoute *>(msg->getContextPointer());

            EV_INFO << "ODR invalide time on route with destination " << odrRoute->getDestination().str() <<
                    " and next hope " << odrRoute->getNextHop().str() << endl;

            invalidateRoute(odrRoute);
            if (odrRoute->getODRHolddown()->isScheduled())
                cancelEvent(odrRoute->getODRHolddown());
            scheduleAt(simTime() + routeHolddownTime, odrRoute->getODRHolddown());
            break;
        case CDPODRHolddown:
            odrRoute = reinterpret_cast<CDPODRRoute *>(msg->getContextPointer());
            EV_INFO << "ODR exit holddown route with destination " << odrRoute->getDestination().str() <<
                        " and next hope " << odrRoute->getNextHop().str() << endl;

            odrRoute->setNoUpdates(false);
            break;
        case CDPODRFlush:
            odrRoute = reinterpret_cast<CDPODRRoute *>(msg->getContextPointer());
            EV_INFO << "ODR flush route with destination " << odrRoute->getDestination().str() <<
                        " and next hope " << odrRoute->getNextHop().str() << endl;

            purgeRoute(odrRoute);
            break;
        default:
            EV_WARN << "Self-message with unexpected message kind " << msg->getKind() << endl;
            delete msg;
            break;
    }
}

void CDPMain::handleMessage(cMessage *msg)
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
        EV_WARN << "Unrecognized message (" << msg->getClassName() << ")" <<  msg->getName() << endl;
    }
}
///************************ END OF MESSAGE HANDLING ****************************///


///************************ ROUTING ****************************///


void CDPMain::invalidateRoute(CDPODRRoute *odrRoute)
{
    IRoute *route = odrRoute->getRoute();
    if (route) {
        odrRoute->setRoute(nullptr);
        rt->deleteRoute(route);
    }
    odrRoute->setInvalide(true);
    odrRoute->setNoUpdates(true);
}

void CDPMain::purgeRoute(CDPODRRoute *odrRoute)
{
    if(odrRoute == nullptr)
        return;

    EV_INFO << "Delete route to " << odrRoute->getDestination().str() << "/" << odrRoute->getPrefixLength() << ": "
             << "nextHop=" << odrRoute->getNextHop().str() << " metric=" << 1 << endl;

    IRoute *route = odrRoute->getRoute();
    if (route) {
        odrRoute->setRoute(nullptr);
        rt->deleteRoute(route);
    }

    ort->removeRoute(odrRoute);
}

void CDPMain::rescheduleODRRouteTimer(CDPODRRoute *odrRoute)
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

void CDPMain::addDefaultRoute(InterfaceEntry *ie, const L3Address& nextHope)
{
    L3Address defaultGateway;
    defaultGateway.set(Ipv4Address());
    CDPODRRoute *defaultRoute = ort->findRoute(rt->getDefaultRoute());

    if(defaultRoute == nullptr || (defaultRoute != nullptr && defaultRoute->isInvalide()))
    {
        if(defaultRoute != nullptr && defaultRoute->isInvalide())
            purgeRoute(defaultRoute);

        IRoute *route = addRouteToRT(defaultGateway, 0, ie, nextHope);

        defaultRoute = new CDPODRRoute(route);
        defaultRoute->setLastUpdateTime(simTime());
        ort->addRoute(defaultRoute);

        scheduleAt(simTime() + defaultRouteInvalide, defaultRoute->getODRInvalideTime());
    }
    else if(!defaultRoute->isInvalide())
    {
        defaultRoute = new CDPODRRoute(defaultGateway, 0, nextHope, ie);
        defaultRoute->setLastUpdateTime(simTime());
        ort->addRoute(defaultRoute);

        scheduleAt(simTime() + defaultRouteInvalide, defaultRoute->getODRInvalideTime());
    }
}

void CDPMain::addRoute(const L3Address& dest, int prefixLength, const InterfaceEntry *ie, const L3Address& nextHope)
{
    EV_INFO << "Add route to " << dest << "/" << prefixLength << ": "
             << "nextHop=" << nextHope << " metric=" << 1 << endl;

    IRoute *route = addRouteToRT(dest, prefixLength, ie, nextHope);

    CDPODRRoute *odrRoute = new CDPODRRoute(route);
    odrRoute->setLastUpdateTime(simTime());

    rescheduleODRRouteTimer(odrRoute);

    ort->addRoute(odrRoute);
}

IRoute *CDPMain::addRouteToRT(const L3Address& dest, int prefixLength, const InterfaceEntry *ie, const L3Address& nextHop)
{
    IRoute *route = rt->createRoute();
    route->setSourceType(IRoute::ODR);
    route->setSource(this);
    route->setDestination(dest.getPrefix(prefixLength));
    route->setPrefixLength(prefixLength);
    route->setInterface(const_cast<InterfaceEntry *>(ie));
    route->setNextHop(nextHop);
    route->setMetric(1);
    ((Ipv4Route *)route)->setAdminDist(Ipv4Route::dODR);
    rt->addRoute(route);
    return route;
}


///************************ END OF ROUTING ****************************///



///************************ TLV ****************************///


bool CDPMain::hasRoutingProtocol()
{
    //TODO: check if routing protocol is really enabled
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

void CDPMain::setTlvDeviceId(CDPUpdate *msg, int pos)
{
    CDPOptionDevId *tlv = new CDPOptionDevId();
    tlv->setType(CDPTLV_DEV_ID);
    tlv->setValue(containingModule->getFullName());
    msg->setOptionLength(tlv);
    msg->addOption(tlv, pos);
}

void CDPMain::setTlvPortId(CDPUpdate *msg, int pos, int interfaceId)
{
    CDPOptionPortId *tlv = new CDPOptionPortId();
    tlv->setType(CDPTLV_PORT_ID);
    tlv->setValue(ift->getInterfaceById(interfaceId)->getFullName());
    msg->setOptionLength(tlv);
    msg->addOption(tlv, pos);
}

void CDPMain::setTlvPlatform(CDPUpdate *msg, int pos)
{
    CDPOptionPlatform *tlv = new CDPOptionPlatform();
    tlv->setType(CDPTLV_PLATFORM);
    tlv->setValue(containingModule->getComponentType()->getName());
    msg->setOptionLength(tlv);
    msg->addOption(tlv, pos);
}

void CDPMain::setTlvVersion(CDPUpdate *msg, int pos)
{
    CDPOptionVersion *tlv = new CDPOptionVersion();
    tlv->setType(CDPTLV_VERSION);
    tlv->setValue("1.0");
    msg->setOptionLength(tlv);
    msg->addOption(tlv, pos);
}

void CDPMain::setTlvCapabilities(CDPUpdate *msg, int pos)
{
    CDPOptionCapa *tlv = new CDPOptionCapa();
    tlv->setCap(0, cap[0]);
    tlv->setCap(1, cap[1]);
    tlv->setCap(2, cap[2]);
    tlv->setCap(3, cap[3]);

    msg->setOptionLength(tlv);
    msg->addOption(tlv, pos);
}

void CDPMain::setTlvDuplex(CDPUpdate *msg, int pos, int interfaceId)
{
    CDPOptionDupl *tlv = new CDPOptionDupl();
    tlv->setFullDuplex(ift->getInterfaceById(interfaceId)->getSubmodule("mac")->par("duplexMode"));
    msg->setOptionLength(tlv);
    msg->addOption(tlv, pos);
}

void CDPMain::setTlvNativeVlan(CDPUpdate *msg, int pos, int interfaceId)
{
    CDPOptionNumb *tlv = new CDPOptionNumb();
    tlv->setType(CDPTLV_NAT_VLAN);
    tlv->setNumber((uint16_t)0);        //TODO: get native vlan
    msg->setOptionLength(tlv);
    msg->addOption(tlv, pos);
}

void CDPMain::setTlvVtp(CDPUpdate *msg, int pos, int interfaceId)
{
    CDPOptionVtp *tlv = new CDPOptionVtp();
    tlv->setType(CDPTLV_VTP);
    tlv->setValue("");        //TODO: get vtp management domain name
    msg->setOptionLength(tlv);
    msg->addOption(tlv, pos);
}

void CDPMain::setTlvIpPrefix(CDPUpdate *msg, int pos, int interfaceId)
{
    if(odr)
    {
        CDPOptionODRDef *tlv = new CDPOptionODRDef();

        tlv->setDefaultRoute(ift->getInterfaceById(interfaceId)->getNetworkAddress().toIpv4().str().c_str());

        msg->setOptionLength(tlv);
        msg->addOption(tlv, pos);
    }
    else
    {
        CDPOptionPref *tlv = new CDPOptionPref();
        std::vector<prefixType *> prefixes;
        prefixType prefix;
        int mask;
        uint8_t maskShort;

        for(int i=0, count = 1; i < ift->getNumInterfaces(); ++i)
        {
            // add to prefixes only interfaces with specified IPv4 address, that are
            // not loopback and are not outgoing interface for this message
            if(ift->getInterface(i)->ipv4Data() != nullptr &&
                    !ift->getInterface(i)->ipv4Data()->getIPAddress().isUnspecified() &&
                    ift->getInterface(i)->getInterfaceId() != interfaceId &&
                    !ift->getInterface(i)->isLoopback())
            {
                mask = ift->getInterface(i)->ipv4Data()->getNetmask().getInt();
                for(maskShort = 0; mask; mask &= mask - 1) maskShort++;

                prefix.setNetwork(ift->getInterface(i)->ipv4Data()->getIPAddress().str().c_str());
                prefix.setMask(maskShort);

                tlv->setPrefixesArraySize(count);
                tlv->setPrefixes(count-1, prefix);
                count++;
            }
        }
        msg->setOptionLength(tlv);
        msg->addOption(tlv, pos);
    }
}


bool CDPMain::ipInterfaceExist(int interfaceId)
{
    for(int i=0; i < ift->getNumInterfaces(); i++)
        if(ift->getInterface(i)->ipv4Data() != nullptr &&
                !ift->getInterface(i)->ipv4Data()->getIPAddress().isUnspecified() &&
                ift->getInterface(i)->getInterfaceId() != interfaceId &&
                !ift->getInterface(i)->isLoopback())
            return true;
    return false;
}

Ipv4Address CDPMain::ipOnInterface(int interfaceId)
{
    if(ift->getInterfaceById(interfaceId)->ipv4Data() != nullptr &&
            !ift->getInterfaceById(interfaceId)->ipv4Data()->getIPAddress().isUnspecified())
    {
        return ift->getInterfaceById(interfaceId)->ipv4Data()->getIPAddress();
    }
    else
        return Ipv4Address();
}

void CDPMain::setTlvAddress(CDPUpdate *msg, int pos, int interfaceId)
{
    CDPOptionAddr *tlv = new CDPOptionAddr();
    addressType address;

    address.setLength(1);               // length of the protocol field. 1 byte for NLPID, or for 802.2 it is either 3 or 8 bytes
    address.setProtocolArraySize(address.getLength());
    address.setProtocol(0, 204);        // 0xCC (204) for IP
    address.setAddress(ipOnInterface(interfaceId).str().c_str());

    tlv->setAddressesArraySize(1);
    tlv->setAddresses(0, address);

    msg->setOptionLength(tlv);
    msg->addOption(tlv, pos);
}

void CDPMain::createTlv(CDPUpdate *msg, int interfaceId)
{
    int count = 0;

    setTlvDeviceId(msg, count++);            //device-id
    setTlvPortId(msg, count++, interfaceId); //port-id
    setTlvVersion(msg, count++);             //version
    setTlvCapabilities(msg, count++);        //capabilities
    setTlvPlatform(msg, count++);            //platform

    if(version == 2)
    {
       if(ift->getInterfaceById(interfaceId)->getSubmodule("mac") != nullptr)
           setTlvDuplex(msg, count++, interfaceId);    //full duplex
       //setTlvNativeVlan(msg, count++, interfaceId);    //TODO: yet not implemented in INET
       //setTlvVtp(msg, count++, interfaceId);           //TODO: yet not implemented in INET
    }

    if(ipOnInterface(interfaceId).getInt() != 0)
        setTlvAddress(msg, count++, interfaceId);   //address

    // exist ip interfaces and no routing protocol run (spoke)
    // odr enable (hub)
    if((ipInterfaceExist(interfaceId) != 0 && !hasRoutingProtocol()) || odr)
        setTlvIpPrefix(msg, count++, interfaceId);  //prefixes
}

///************************ END OF TLV ****************************///



} /* namespace inet */


