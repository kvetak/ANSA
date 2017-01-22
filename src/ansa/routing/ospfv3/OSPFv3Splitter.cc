#include "ansa/routing/ospfv3/OSPFv3Splitter.h"

namespace inet{

Define_Module(OSPFv3Splitter);

OSPFv3Splitter::OSPFv3Splitter()
{

}

OSPFv3Splitter::~OSPFv3Splitter()
{

}

void OSPFv3Splitter::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if(stage == INITSTAGE_ROUTING_PROTOCOLS)
    {
        containingModule=findContainingNode(this);
        routingModule=this->getParentModule();

        ift = check_and_cast<IInterfaceTable *>(containingModule->getSubmodule("interfaceTable"));
        IPSocket ipSocket(gate("ipOut"));
        ipSocket.registerProtocol(IP_PROT_OSPF);

        this->parseConfig(par("ospfv3RoutingConfig"), par("ospfv3IntConfig"));
    }
}

void OSPFv3Splitter::handleMessage(cMessage* msg)
{
    if(msg->isSelfMessage()) {
        EV_DEBUG <<"Self message received by Splitter\n";
        delete msg;
    }
    else{
        if(strcmp(msg->getArrivalGate()->getBaseName(),"processIn")==0){
            this->send(msg, "ipOut");//A message from one of the processes
        }
        else if(strcmp(msg->getArrivalGate()->getBaseName(),"ipIn")==0){
            IPv6ControlInfo *ctlInfo = dynamic_cast<IPv6ControlInfo*>(msg->getControlInfo());

            if(ctlInfo==nullptr){
                delete msg;
                return;
            }

            OSPFv3Packet* packet = dynamic_cast<OSPFv3Packet*>(msg);

            if(packet==nullptr){
                delete msg;
                return;
            }

            InterfaceEntry* ie = this->ift->getInterfaceById(ctlInfo->getInterfaceId());
            char* ieName = (char*)ie->getName();
            std::map<std::string, int>::iterator it = this->interfaceToProcess.find(ieName);
            //Is there a process with this interface??
            if(it!=this->interfaceToProcess.end()){
                int outNum = it->second;
                this->send(msg, "processOut", outNum);
            }
            else {
                delete msg;
            }
        }
    }
}

void OSPFv3Splitter::parseConfig(cXMLElement* routingConfig, cXMLElement* intConfig)
{
    if(routingConfig==nullptr)
        throw cRuntimeError("Routing configuration not found");

    cXMLElementList processList = routingConfig->getElementsByTagName("Process");

    int splitterGateVector=processList.size();
    this->setGateSize("processOut", splitterGateVector);
    this->setGateSize("processIn", splitterGateVector);

    int gateCount=0;
    for(auto it=processList.begin(); it!=processList.end(); it++)
        this->addNewProcess((*it), intConfig, gateCount++);

    if(intConfig==nullptr)
                EV_DEBUG << "Configuration of interfaces was not found in config.xml";

    cXMLElementList intList = intConfig->getChildren();
    for(auto it=intList.begin(); it!=intList.end(); it++)
    {//TODO - check whether the interface exists on the router
        const char* intName = (*it)->getAttribute("name");

        cXMLElementList processElements = (*it)->getElementsByTagName("Process");
        if(processElements.size()>1)
            EV_DEBUG <<"More than one process is configured for interface " << intName << "\n";

        const char* processID = processElements.at(0)->getAttribute("id");
        std::map<std::string, int>::iterator procIt;
        procIt = this->processInVector.find(processID);
        int processPosition = procIt->second;
        this->interfaceToProcess[intName]=processPosition;
        this->processesModules.at(processPosition)->activateProcess();

        std::string procName = "process"+std::string(processID);
        this->processToInterface[(char*)procName.c_str()]=(char*)intName;

        //register all interfaces to MCAST
        for (auto it=processToInterface.begin(); it!=processToInterface.end(); it++) {
            EV_DEBUG << "FOR \n";
            InterfaceEntry* ie = ift->getInterfaceByName(intName);
            IPv6InterfaceData *ipv6int = ie->ipv6Data();
            ipv6int->joinMulticastGroup(IPv6Address::ALL_OSPF_ROUTERS_MCAST);//TODO - join only once
            ipv6int->assignAddress(IPv6Address::ALL_OSPF_ROUTERS_MCAST, false, 0, 0);
        }
    }
}//parseConfig

void OSPFv3Splitter::addNewProcess(cXMLElement* process, cXMLElement* interfaces, int gateIndex)
{
    cModuleType* newProcessType = cModuleType::find("ansa.routing.ospfv3.process.OSPFv3Process");
    if(newProcessType==nullptr)
        throw cRuntimeError("OSPFv3Routing: OSPFv3Process module was not found");

    std::string processID = std::string(process->getAttribute("id"));
    cXMLElementList idList = process->getElementsByTagName("RouterID");
    if(idList.size()>1)
        throw cRuntimeError("More than one routerID was configured for process %s", processID.c_str());

    std::string routerID = std::string(idList.at(0)->getNodeValue());//TODO - if no routerID choose the loopback or interface
    this->processInVector.insert(std::make_pair(processID,gateIndex));//[*processID]=gateCount;
    std::string processFullName = "process" + processID;

    OSPFv3Process* newProcessModule = (OSPFv3Process*)newProcessType->create(processFullName.c_str(), this->routingModule);
    std::istringstream ss(processID);
    int processIdNum;
    ss>>processIdNum;

    newProcessModule->par("processID")=processIdNum;
    newProcessModule->par("routerID")=routerID;
    newProcessModule->par("interfaceConfig")=interfaces;
    newProcessModule->finalizeParameters();
    //newProcessModule->callInitialize(INITSTAGE_ROUTING_PROTOCOLS);
//    newProcessModule->buildInside();

    this->gate("processOut", gateIndex)->connectTo(newProcessModule->gate("splitterIn"));
    newProcessModule->gate("splitterOut")->connectTo(this->gate("processIn", gateIndex));

    newProcessModule->scheduleStart(simTime());
    this->processesModules.push_back(newProcessModule);
}//addNewProcess
}//namespace inet
