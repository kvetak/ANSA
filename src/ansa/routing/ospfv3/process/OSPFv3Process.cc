#include "ansa/routing/ospfv3/process/OSPFv3Process.h"

namespace inet{

Define_Module(OSPFv3Process);

OSPFv3Process::OSPFv3Process()
{
//    this->processID = processID;
}

OSPFv3Process::~OSPFv3Process()
{
}

void OSPFv3Process::initialize(int stage){

    if(stage == INITSTAGE_ROUTING_PROTOCOLS){
        this->containingModule=findContainingNode(this);
        ift = check_and_cast<IInterfaceTable* >(containingModule->getSubmodule("interfaceTable"));
        rt = check_and_cast<IPv6RoutingTable* >(containingModule->getSubmodule("routingTable")->getSubmodule("ipv6"));

        this->routerID = IPv4Address(par("routerID").stringValue());
        this->processID = (int)par("processID");
        this->parseConfig(par("interfaceConfig"));

        cMessage* init = new cMessage();
        init->setKind(INIT_PROCESS);
        scheduleAt(simTime(), init);
        WATCH_PTRVECTOR(this->instances);
        WATCH_PTRVECTOR(this->routingTable);

        ageTimer = new cMessage();
        ageTimer->setKind(DATABASE_AGE_TIMER);
        ageTimer->setContextPointer(this);
        ageTimer->setName("OSPFv3Process::DatabaseAgeTimer");
    //    std::cout << "ageTimer->getOwner = " << ageTimer->getOwner()->getName() << endl;
        this->setTimer(ageTimer, 1.0);
    }
}

void OSPFv3Process::handleMessage(cMessage* msg)
{
    if(msg->isSelfMessage())
    {
        this->handleTimer(msg);
    }
    else
    {
        OSPFv3Packet* packet = check_and_cast<OSPFv3Packet*>(msg);
        if(packet->getRouterID()==this->getRouterID()) //is it this router who originated the message?
            delete msg;
        else {
            IPv6ControlInfo *ctlInfo = dynamic_cast<IPv6ControlInfo*>(packet->getControlInfo());
            if(ctlInfo!=nullptr) {
                OSPFv3Instance* instance = this->getInstanceById(packet->getInstanceID());
                if(instance == nullptr){//Is there an instance with this number?
                    EV_DEBUG << "Instance with this ID not found, dropping\n";
                    //delete msg;//TODO - some warning??
                }
                else {
                    instance->processPacket(packet);
                }
            }
            else {
                delete msg;
            }
        }
    }
}//handleMessage

void OSPFv3Process::parseConfig(cXMLElement* interfaceConfig)
{
    EV_DEBUG << "Parsing config on process " << this->processID << endl;
    //Take each interface
    cXMLElementList intList = interfaceConfig->getElementsByTagName("Interface");
    for(auto interfaceIt=intList.begin(); interfaceIt!=intList.end(); interfaceIt++)
    {
        const char* interfaceName = (*interfaceIt)->getAttribute("name");
        const char* routerPriority = nullptr;
        const char* helloInterval = nullptr;
        const char* deadInterval = nullptr;
        const char* interfaceCost = nullptr;
        const char* interfaceType = nullptr;
        OSPFv3Interface::OSPFv3InterfaceType interfaceTypeNum;
        bool passiveInterface = false;


        cXMLElementList process = (*interfaceIt)->getElementsByTagName("Process");
        if(process.size()>2)
            throw cRuntimeError("More than two processes configured for interface %s", (*interfaceIt)->getAttribute("name"));

        //Check whether it belongs to this process
        int processCount = process.size();
        for(int i = 0; i < processCount; i++){
            int procId = atoi(process.at(i)->getAttribute("id"));
            if(procId != this->processID)
                continue;

            EV_DEBUG << "Creating new interface "  << interfaceName << " in process " << procId << endl;

            //Parsing instances
            cXMLElementList instList = process.at(i)->getElementsByTagName("Instance");
            for(auto instIt=instList.begin(); instIt!=instList.end(); instIt++)
            {
                const char* instId = (*instIt)->getAttribute("instanceID");
                const char* addressFamily = (*instIt)->getAttribute("AF");

                //Get the router priority for this interface and instance
                cXMLElementList interfaceOptions = (*instIt)->getElementsByTagName("RouterPriority");
                if(interfaceOptions.size()>1)
                    throw cRuntimeError("Multiple router priority is configured for interface %s", interfaceName);

                if(interfaceOptions.size()!=0)
                    routerPriority = interfaceOptions.at(0)->getNodeValue();

                //get the hello interval for this interface and instance
                interfaceOptions = (*instIt)->getElementsByTagName("HelloInterval");
                if(interfaceOptions.size()>1)
                    throw cRuntimeError("Multiple HelloInterval value is configured for interface %s", interfaceName);

                if(interfaceOptions.size()!=0)
                    helloInterval = interfaceOptions.at(0)->getNodeValue();

                //get the dead interval for this interface and instance
                interfaceOptions = (*instIt)->getElementsByTagName("DeadInterval");
                if(interfaceOptions.size()>1)
                    throw cRuntimeError("Multiple DeadInterval value is configured for interface %s", interfaceName);

                if(interfaceOptions.size()!=0)
                    deadInterval = interfaceOptions.at(0)->getNodeValue();

                //get the interface cost for this interface and instance
                interfaceOptions = (*instIt)->getElementsByTagName("InterfaceCost");
                if(interfaceOptions.size()>1)
                    throw cRuntimeError("Multiple InterfaceCost value is configured for interface %s", interfaceName);

                if(interfaceOptions.size()!=0)
                    interfaceCost = interfaceOptions.at(0)->getNodeValue();

                //get the interface cost for this interface and instance
                interfaceOptions = (*instIt)->getElementsByTagName("InterfaceType");
                if(interfaceOptions.size()>1)
                    throw cRuntimeError("Multiple InterfaceType value is configured for interface %s", interfaceName);

                if(interfaceOptions.size()!=0){
                    interfaceType = interfaceOptions.at(0)->getNodeValue();
                    if(strcmp(interfaceType, "Broadcast")==0)
                        interfaceTypeNum = OSPFv3Interface::BROADCAST_TYPE;
                    else if(strcmp(interfaceType, "PointToPoint")==0)
                        interfaceTypeNum = OSPFv3Interface::POINTTOPOINT_TYPE;
                    else if(strcmp(interfaceType, "NBMA")==0)
                        interfaceTypeNum = OSPFv3Interface::NBMA_TYPE;
                    else if(strcmp(interfaceType, "PointToMultipoint")==0)
                        interfaceTypeNum = OSPFv3Interface::POINTTOMULTIPOINT_TYPE;
                    else if(strcmp(interfaceType, "Virtual")==0)
                        interfaceTypeNum = OSPFv3Interface::VIRTUAL_TYPE;
                    else
                        interfaceTypeNum = OSPFv3Interface::UNKNOWN_TYPE;
                }
                else
                    throw cRuntimeError("Interface type needs to be specified for interface %s", interfaceName);

                //find out whether the interface is passive
                interfaceOptions = (*instIt)->getElementsByTagName("PassiveInterface");
                if(interfaceOptions.size()>1)
                    throw cRuntimeError("Multiple PassiveInterface value is configured for interface %s", interfaceName);

                if(interfaceOptions.size()!=0){
                    if(strcmp(interfaceOptions.at(0)->getNodeValue(), "True")==0)
                        passiveInterface = true;
                }


                int instIdNum;

                if(instId==nullptr) {
                    EV_DEBUG << "Address Family " << addressFamily << endl;
                    if(strcmp(addressFamily, "IPv4")==0) {
                        EV_DEBUG << "IPv4 instance\n";
                        instIdNum = DEFAULT_IPV4_INSTANCE;
                    }
                    else if(strcmp(addressFamily, "IPv6")==0) {
                        EV_DEBUG << "IPv6 instance\n";
                        instIdNum = DEFAULT_IPV6_INSTANCE;
                    }
                    else
                        throw cRuntimeError("Unknown address family in process %d", this->getProcessID());
                }
                else
                    instIdNum = atoi(instId);

                //TODO - check range of instance ID

                //check for multiple definition of one instance
                OSPFv3Instance* instance = this->getInstanceById(instIdNum);
                //if(instance != nullptr)
                // throw cRuntimeError("Multiple OSPFv3 instance with the same instance ID configured for process %d on interface %s", this->getProcessID(), interfaceName);

                if(instance == nullptr) {
                    if(strcmp(addressFamily, "IPv4")==0)
                        instance = new OSPFv3Instance(instIdNum, this, IPV4INSTANCE);
                    else
                        instance = new OSPFv3Instance(instIdNum, this, IPV6INSTANCE);

                    EV_DEBUG << "Adding instance " << instIdNum << " to process " << this->processID << endl;
                    this->addInstance(instance);
                }

                //TODO - multiarea configuration??
                cXMLElementList areasList = (*instIt)->getElementsByTagName("Area");
                for(auto areasIt=areasList.begin(); areasIt!=areasList.end(); areasIt++)
                {
                    const char* areaId = (*areasIt)->getNodeValue();
                    IPv4Address areaIP = IPv4Address(areaId);
                    const char* areaType = (*areasIt)->getAttribute("type");
                    OSPFv3AreaType type = NORMAL;

                    if(areaType != nullptr){
                        if(strcmp(areaType, "stub") == 0)
                            type = STUB;
                        else if(strcmp(areaType, "nssa") == 0)
                            type = NSSA;
                    }

                    const char* summary = (*areasIt)->getAttribute("summary");

                    if(summary != nullptr){
                        if(strcmp(summary, "no") == 0) {
                            if(type == STUB)
                                type = TOTALLY_STUBBY;
                            else if(type == NSSA)
                                type = NSSA_TOTALLY_STUB;
                        }
                    }

                    //insert area if it's not already there and assign this interface
                    OSPFv3Area* area;
                    if(!(instance->hasArea(areaIP))) {
                        area = new OSPFv3Area(areaIP, instance, type);
                        instance->addArea(area);
                    }
                    else
                        area = instance->getAreaById(areaIP);

                    if(!area->hasInterface(std::string(interfaceName))) {
                        OSPFv3Interface* newInterface = new OSPFv3Interface(interfaceName, this->containingModule, this, interfaceTypeNum, passiveInterface);
                        if(helloInterval!=nullptr)
                            newInterface->setHelloInterval(atoi(helloInterval));

                        if(deadInterval!=nullptr)
                            newInterface->setDeadInterval(atoi(deadInterval));

                        if(interfaceCost!=nullptr)
                            newInterface->setInterfaceCost(atoi(interfaceCost));

                        if(routerPriority!=nullptr) {
                            int rtrPrio = atoi(routerPriority);
                            if(rtrPrio < 0 || rtrPrio > 255)
                                throw cRuntimeError("Router priority out of range on interface %s", interfaceName);

                            newInterface->setRouterPriority(rtrPrio);
                        }

                        newInterface->setArea(area);
                        std::cout << "I am " << this->getOwner()->getOwner()->getName() << " on int " << newInterface->getInterfaceIP() << " with area " << area->getAreaID() << endl;
                        area->addInterface(newInterface);
                    }
                }
            }
        }
    }
}//parseConfig


void OSPFv3Process::ageDatabase()
{
    bool shouldRebuildRoutingTable = false;

    long instanceCount = instances.size();
    for (long i = 0; i < instanceCount; i++)
    {
        long areaCount = instances[i]->getAreaCount();

        for (long j = 0; j < areaCount; j++)
        {
            instances[i]->getArea(j)->ageDatabase();
        }

//        messageHandler->startTimer(ageTimer, 1.0);

        if (shouldRebuildRoutingTable) {
            rebuildRoutingTable();
        }
    }
} // ageDatabase

void OSPFv3Process::handleTimer(cMessage* msg)
{
    switch(msg->getKind())
    {
        case INIT_PROCESS:
            for(auto it=this->instances.begin(); it!=this->instances.end(); it++)
                (*it)->init();

            this->debugDump();
        break;

        case HELLO_TIMER:
        {
            OSPFv3Interface* interface;
            if(!(interface=reinterpret_cast<OSPFv3Interface*>(msg->getContextPointer())))
            {
                //TODO - error
            }
            else {
                EV_DEBUG << "Process received msg, sending event HELLO_TIMER_EVENT\n";
                interface->processEvent(OSPFv3Interface::HELLO_TIMER_EVENT);
            }
        }
        break;

        case WAIT_TIMER:
        {
            OSPFv3Interface* interface;
            if(!(interface=reinterpret_cast<OSPFv3Interface*>(msg->getContextPointer())))
            {
                //TODO - error
            }
            else {
                EV_DEBUG << "Process received msg, sending event WAIT_TIMER_EVENT\n";
                interface->processEvent(OSPFv3Interface::WAIT_TIMER_EVENT);
            }
        }
        break;

        case ACKNOWLEDGEMENT_TIMER: {
            OSPFv3Interface *intf;
            if (!(intf = reinterpret_cast<OSPFv3Interface *>(msg->getContextPointer()))) {
                // should not reach this point
                EV_INFO << "Discarding invalid InterfaceAcknowledgementTimer.\n";
                delete msg;
            }
            else {
//                printEvent("Acknowledgement Timer expired", intf);
                intf->processEvent(OSPFv3Interface::ACKNOWLEDGEMENT_TIMER_EVENT);
            }
        }
        break;

        case NEIGHBOR_INACTIVITY_TIMER: {
            OSPFv3Neighbor *neighbor;
            if (!(neighbor = reinterpret_cast<OSPFv3Neighbor *>(msg->getContextPointer()))) {
                // should not reach this point
                EV_INFO << "Discarding invalid NeighborInactivityTimer.\n";
                delete msg;
            }
            else {
//                printEvent("Inactivity Timer expired", neighbor->getInterface(), neighbor);
//                neighbor->processEvent(OSPFv3Neighbor::INACTIVITY_TIMER);
                OSPFv3Interface* intf = neighbor->getInterface();
                int neighborCnt = intf->getNeighborCount();
                for(int i=0; i<neighborCnt; i++){
                    OSPFv3Neighbor* currNei = intf->getNeighbor(i);
                    if(currNei->getNeighborID() == neighbor->getNeighborID()){
                        intf->removeNeighborByID(neighbor->getNeighborID());
                        break;
                    }
                }

                intf->processEvent(OSPFv3Interface::NEIGHBOR_CHANGE_EVENT);
            }
        }
        break;

        case NEIGHBOR_POLL_TIMER: {
            OSPFv3Neighbor *neighbor;
            if (!(neighbor = reinterpret_cast<OSPFv3Neighbor *>(msg->getContextPointer()))) {
                // should not reach this point
                EV_INFO << "Discarding invalid NeighborInactivityTimer.\n";
                delete msg;
            }
            else {
//                printEvent("Poll Timer expired", neighbor->getInterface(), neighbor);
                neighbor->processEvent(OSPFv3Neighbor::POLL_TIMER);
            }
        }
        break;

        case NEIGHBOR_DD_RETRANSMISSION_TIMER: {
            OSPFv3Neighbor *neighbor;
            if (!(neighbor = reinterpret_cast<OSPFv3Neighbor *>(msg->getContextPointer()))) {
                // should not reach this point
                EV_INFO << "Discarding invalid NeighborDDRetransmissionTimer.\n";
                delete msg;
            }
            else {
//                printEvent("Database Description Retransmission Timer expired", neighbor->getInterface(), neighbor);
                neighbor->processEvent(OSPFv3Neighbor::DD_RETRANSMISSION_TIMER);
            }
        }
        break;

        case NEIGHBOR_UPDATE_RETRANSMISSION_TIMER: {
            OSPFv3Neighbor *neighbor;
            if (!(neighbor = reinterpret_cast<OSPFv3Neighbor *>(msg->getContextPointer()))) {
                // should not reach this point
                EV_INFO << "Discarding invalid NeighborUpdateRetransmissionTimer.\n";
                delete msg;
            }
            else {
//                printEvent("Update Retransmission Timer expired", neighbor->getInterface(), neighbor);
                neighbor->processEvent(OSPFv3Neighbor::UPDATE_RETRANSMISSION_TIMER);
            }
        }
        break;

        case NEIGHBOR_REQUEST_RETRANSMISSION_TIMER: {
            OSPFv3Neighbor *neighbor;
            if (!(neighbor = reinterpret_cast<OSPFv3Neighbor *>(msg->getContextPointer()))) {
                // should not reach this point
                EV_INFO << "Discarding invalid NeighborRequestRetransmissionTimer.\n";
                delete msg;
            }
            else {
//                printEvent("Request Retransmission Timer expired", neighbor->getInterface(), neighbor);
                neighbor->processEvent(OSPFv3Neighbor::REQUEST_RETRANSMISSION_TIMER);
            }
        }
        break;

        case DATABASE_AGE_TIMER: {
            EV_DEBUG << "Ageing the database\n";
            this->setTimer(ageTimer, 1.0);
            this->ageDatabase();
        }
        break;


        default:
            break;
    }
}

void OSPFv3Process::setTimer(cMessage* msg, double delay = 0)
{
    scheduleAt(simTime()+delay, msg);
}

void OSPFv3Process::activateProcess()
{
    Enter_Method_Silent();
    this->isActive=true;
    cMessage* init = new cMessage();
    init->setKind(HELLO_TIMER);
    scheduleAt(simTime(), init);
}//activateProcess

void OSPFv3Process::debugDump()
{
    EV_DEBUG << "$$$$$$$$ Process " << this->getProcessID() << "\n";
    for(auto it=this->instances.begin(); it!=this->instances.end(); it++)
        (*it)->debugDump();
}//debugDump

OSPFv3Instance* OSPFv3Process::getInstanceById(int instanceId)
{
    std::map<int, OSPFv3Instance*>::iterator instIt = this->instancesById.find(instanceId);
    if(instIt == this->instancesById.end())
        return nullptr;

    return instIt->second;
}

void OSPFv3Process::addInstance(OSPFv3Instance* newInstance)
{
    OSPFv3Instance* check = this->getInstanceById(newInstance->getInstanceID());
    if(check==nullptr){
        this->instances.push_back(newInstance);
        this->instancesById[newInstance->getInstanceID()]=newInstance;
    }
}

void OSPFv3Process::sendPacket(OSPFv3Packet *packet, IPv6Address destination, const char* ifName, short hopLimit)
{
    InterfaceEntry *ie = this->ift->getInterfaceByName(ifName);

    IPv6InterfaceData *ipv6int = ie->ipv6Data();
    IPv6ControlInfo *ipv6ControlInfo = new IPv6ControlInfo();
    ipv6ControlInfo->setProtocol(IP_PROT_OSPF);
    ipv6ControlInfo->setHopLimit(hopLimit);
    //ipv6ControlInfo->setSourceAddress(ipv6int->getLinkLocalAddress());
    ipv6ControlInfo->setSourceAddress(ipv6int->getLinkLocalAddress());
    ipv6ControlInfo->setDestinationAddress(destination);
    ipv6ControlInfo->setInterfaceId(ie->getInterfaceId());

    packet->setByteLength(packet->getByteLength()+54);//This is for the IPv6 Header and Ethernet Header

    packet->setControlInfo(ipv6ControlInfo);
    this->send(packet, "splitterOut");
}//sendPacket


OSPFv3LSA* OSPFv3Process::findLSA(LSAKeyType lsaKey, IPv4Address areaID, int instanceID)
{
    OSPFv3Instance* instance = this->getInstanceById(instanceID);
    OSPFv3Area* area = instance->getAreaById(areaID);
    return area->getLSAbyKey(lsaKey);
    /*
    switch(lsaKey.LSType) {
        case ROUTER_LSA:
        {
            OSPFv3Instance* instance = this->getInstanceById(instanceID);
            OSPFv3Area* area = instance->getAreaById(areaID);
            return area->getLSAbyKey(lsaKey);

            break;
        }

        case NETWORK_LSA:
        {
            OSPFv3Instance* instance = this->getInstanceById(instanceID);
            OSPFv3Area* area = instance->getAreaById(areaID);
            return area->getLSAbyKey(lsaKey);

            break;
        }

        case INTER_AREA_PREFIX_LSA:
            break;

        case INTER_AREA_ROUTER_LSA:
            break;

        case AS_EXTERNAL_LSA:
            break;

        case DEPRECATED:
            break;

        case NSSA_LSA:
            break;

        case LINK_LSA:
        {
            OSPFv3Instance* instance = this->getInstanceById(instanceID);
            OSPFv3Area* area = instance->getAreaById(areaID);
            return area->getLSAbyKey(lsaKey);
            break;
        }

        case INTRA_AREA_PREFIX_LSA:

            break;
    }*/
}

bool OSPFv3Process::floodLSA(OSPFv3LSA* lsa, IPv4Address areaID, OSPFv3Interface* interface, OSPFv3Neighbor* neighbor)
{
    EV_DEBUG << "Flooding LSA from router " << lsa->getHeader().getAdvertisingRouter() << " with ID " << lsa->getHeader().getLinkStateID() << "\n";
    bool floodedBackOut = false;

    if (lsa != nullptr) {
        OSPFv3Instance* instance = interface->getArea()->getInstance();
        if (lsa->getHeader().getLsaType() == AS_EXTERNAL_LSA) {
            long areaCount = instance->getAreaCount();
            for (long i = 0; i < areaCount; i++) {
                OSPFv3Area* area = instance->getArea(i);
                if (area->getExternalRoutingCapability()) {
                    if (area->floodLSA(lsa, interface, neighbor)) {
                        floodedBackOut = true;
                    }
                }
            }
        }
        else {
            OSPFv3Area* area = instance->getAreaById(areaID);
            if (area != nullptr) {
                floodedBackOut = area->floodLSA(lsa, interface, neighbor);
            }
        }
    }

    return floodedBackOut;
}

bool OSPFv3Process::installLSA(OSPFv3LSA *lsa, int instanceID, IPv4Address areaID    /*= BACKBONE_AREAID*/, OSPFv3Interface* intf)
{
    EV_DEBUG << "OSPFv3Process::installLSA\n";
    switch (lsa->getHeader().getLsaType()) {
        case ROUTER_LSA: {
            OSPFv3Instance* instance = this->getInstanceById(instanceID);
            OSPFv3Area* area = instance->getAreaById(areaID);
            if (area!=nullptr) {
                OSPFv3RouterLSA *ospfRouterLSA = check_and_cast<OSPFv3RouterLSA *>(lsa);
                return area->installRouterLSA(ospfRouterLSA);
            }
        }
        break;

        case NETWORK_LSA: {
            OSPFv3Instance* instance = this->getInstanceById(instanceID);
            OSPFv3Area* area = instance->getAreaById(areaID);
            if (area!=nullptr) {
                OSPFv3NetworkLSA *ospfNetworkLSA = check_and_cast<OSPFv3NetworkLSA *>(lsa);
                return area->installNetworkLSA(ospfNetworkLSA);
            }
        }
        break;

        case INTER_AREA_PREFIX_LSA: {
            OSPFv3Instance* instance = this->getInstanceById(instanceID);
            OSPFv3Area* area = instance->getAreaById(areaID);
            if (area!=nullptr) {
                OSPFv3InterAreaPrefixLSA *ospfInterAreaLSA = check_and_cast<OSPFv3InterAreaPrefixLSA *>(lsa);
                return area->installInterAreaPrefixLSA(ospfInterAreaLSA);
            }
        }
        break;

//        case SUMMARYLSA_NETWORKS_TYPE:
//        case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE: {
//            auto areaIt = areasByID.find(areaID);
//            if (areaIt != areasByID.end()) {
//                OSPFSummaryLSA *ospfSummaryLSA = check_and_cast<OSPFSummaryLSA *>(lsa);
//                return areaIt->second->installSummaryLSA(ospfSummaryLSA);
//            }
//        }
//        break;
//
//        case AS_EXTERNAL_LSA_TYPE: {
//            OSPFASExternalLSA *ospfASExternalLSA = check_and_cast<OSPFASExternalLSA *>(lsa);
//            return installASExternalLSA(ospfASExternalLSA);
//        }
//        break;
//
        case LINK_LSA: {
            OSPFv3Instance* instance = this->getInstanceById(instanceID);
            OSPFv3Area* area = instance->getAreaById(areaID);
            if (area!=nullptr) {
                OSPFv3LinkLSA *ospfLinkLSA = check_and_cast<OSPFv3LinkLSA *>(lsa);
                return intf->installLinkLSA(ospfLinkLSA);
            }
        }
        break;

        case INTRA_AREA_PREFIX_LSA: {
            OSPFv3Instance* instance = this->getInstanceById(instanceID);
            OSPFv3Area* area = instance->getAreaById(areaID);
            if(area!=nullptr) {
                OSPFv3IntraAreaPrefixLSA* intraLSA = check_and_cast<OSPFv3IntraAreaPrefixLSA *>(lsa);
                return area->installIntraAreaPrefixLSA(intraLSA);
            }
        }
        break;
//        default:
//            ASSERT(false);
//            break;
    }
    return false;
}

void OSPFv3Process::calculateASExternalRoutes(std::vector<OSPFv3RoutingTableEntry* > newTable)
{
    EV_DEBUG << "Calculating AS External Routes\n";
}

void OSPFv3Process::rebuildRoutingTable()
{
    unsigned long instanceCount = this->instances.size();
    std::vector<OSPFv3RoutingTableEntry *> newTable;

    for(unsigned int k=0; k<instanceCount; k++) {
        OSPFv3Instance* currInst = this->instances.at(k);
        unsigned long areaCount = currInst->getAreaCount();
        bool hasTransitAreas = false;

        unsigned long i;

        EV_INFO << "Rebuilding routing table for instance " << this->instances.at(k)->getInstanceID() << ":\n";

        //2)Intra area routes are calculated using SPF algo
        for (i = 0; i < areaCount; i++) {
            currInst->getArea(i)->calculateShortestPathTree(newTable);
            if (currInst->getArea(i)->getTransitCapability()) {
                hasTransitAreas = true;
            }
        }
        //3)Inter-area routes are calculated by examining summary-LSAs (on backbone only)
        if (areaCount > 1) {
            OSPFv3Area *backbone = currInst->getAreaById(BACKBONE_AREAID);
            if (backbone != nullptr) {
                backbone->calculateInterAreaRoutes(newTable);
            }
        }
        else {
            if (areaCount == 1) {
                currInst->getArea(0)->calculateInterAreaRoutes(newTable);
            }
        }

        //4)On BDR - Transit area LSAs(summary) are examined - find better paths then in 2) and 3)
        if (hasTransitAreas) {
            for (i = 0; i < areaCount; i++) {
                if (currInst->getArea(i)->getTransitCapability()) {
                    currInst->getArea(i)->recheckSummaryLSAs(newTable);
                }
            }
        }

        //5) Routes to external destinations are calculated
        calculateASExternalRoutes(newTable);

        // backup the routing table
        unsigned long routeCount = routingTable.size();
        std::vector<OSPFv3RoutingTableEntry *> oldTable;

        oldTable.assign(routingTable.begin(), routingTable.end());
        routingTable.clear();
        routingTable.assign(newTable.begin(), newTable.end());

        std::vector<IPv6Route *> eraseEntries;
        unsigned long routingEntryNumber = rt->getNumRoutes();
        // remove entries from the IPv4 routing table inserted by the OSPF module
        for (i = 0; i < routingEntryNumber; i++) {
            IPv6Route *entry = rt->getRoute(i);
            OSPFv3RoutingTableEntry *ospfEntry = dynamic_cast<OSPFv3RoutingTableEntry *>(entry);
            if (ospfEntry != nullptr) {
                eraseEntries.push_back(entry);
            }
        }

        unsigned int eraseCount = eraseEntries.size();
        for (i = 0; i < eraseCount; i++) {
            rt->deleteRoute(eraseEntries[i]);
        }

        // add the new routing entries
        routeCount = routingTable.size();
        for (i = 0; i < routeCount; i++) {
//            if (routingTable[i]->getDestinationType() == OSPFv3RoutingTableEntry::NETWORK_DESTINATION) {
//                rt->addRoute(new RoutingTableEntry(*(routingTable[i])));
//            }
        }

        //notifyAboutRoutingTableChanges(oldTable);

        routeCount = oldTable.size();
        for (i = 0; i < routeCount; i++) {
            delete (oldTable[i]);
        }

        EV_INFO << "Routing table was rebuilt.\n"
                << "Results:\n";

        routeCount = routingTable.size();
        for (i = 0; i < routeCount; i++) {
            EV_INFO << *routingTable[i] << "\n";
        }
    }
}
}//namespace inet


