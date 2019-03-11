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

        WATCH_PTRVECTOR(this->processesModules);
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
            std::map<std::string, std::pair<int,int>>::iterator it = this->interfaceToProcess.find(ieName);
            //Is there a process with this interface??
            if(it!=this->interfaceToProcess.end()){
                this->send(msg, "processOut", it->second.first);//first is always there

                if(it->second.second!=-1) {
                    cMessage* copy = msg->dup();
                    copy->setControlInfo(msg->getControlInfo()->dup());
                    this->send(copy, "processOut", it->second.second);
                }
            }
            else {
                delete msg;
            }
        }
    }
}

// routingConfig and intConfig are filled through omnetpp.ini of an example
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

    cXMLElementList intList = intConfig->getChildren(); // from <interfaces> take interface one by one into intList
    for(auto it=intList.begin(); it!=intList.end(); it++)
    {//TODO - check whether the interface exists on the router
        const char* intName = (*it)->getAttribute("name");

        cXMLElementList processElements = (*it)->getElementsByTagName("Process");
        if(processElements.size()>2) // only two process per interface are permitted
            EV_DEBUG <<"More than two processes are configured for interface " << intName << "\n";

        int processCount = processElements.size();
        for(int i=0; i<processCount; i++) {
            const char* processID = processElements.at(i)->getAttribute("id");
            std::map<std::string, int>::iterator procIt;
            procIt = this->processInVector.find(processID);
            int processPosition = procIt->second;

            //find interface in interfaceToProcess, if this is the first time, set first int to process position
            //and second int to -1
            std::map<std::string, std::pair<int, int>>::iterator intProc;
            intProc = this->interfaceToProcess.find(intName);
            std::pair<int, int> procInts;
            if(intProc == this->interfaceToProcess.end()) {
                EV_DEBUG << "Process " << processID << " is assigned to interface " << intName << " as the first process. Its position in vector is" << processPosition << "\n";
                procInts.first = processPosition;
                procInts.second = -1;
            }
            else {
                procInts.second = processPosition;
                EV_DEBUG << "Process " << processID << " is assigned to interface " << intName << " as the second process. The first was number " << procInts.first << " in the vector\n";
            }

            this->interfaceToProcess[intName]=procInts;

//            EV_DEBUG << processPosition << endl;
            this->processesModules.at(processPosition)->activateProcess();


            std::string procName = "process"+std::string(processID);
            EV_DEBUG << "Adding process " << processID << " to ProcessToInterface\n";
            this->processToInterface[(char*)procName.c_str()]=(char*)intName;
        }
        //register all interfaces to MCAST
        for (auto it=processToInterface.begin(); it!=processToInterface.end(); it++) {
//            EV_DEBUG << "FOR \n";
            InterfaceEntry* ie = ift->getInterfaceByName(intName);
            IPv6InterfaceData *ipv6int = ie->ipv6Data();
//            std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl; LG
//            std::cout << "ie = " << endl;
//            std::cout << "getNumAddresses() = " << ie->ipv6Data()->getNumAddresses() << endl;
//            for (int i = 0; i < ie->ipv6Data()->getNumAddresses(); i++)
//            {
//                std::cout << "getAddress(i) = " << ie->ipv6Data()->getAddress(i) << endl;
////                std::cout << "getAdvPrefix(i)" << ie->ipv6Data()->getAdvPrefix(i).prefixLength<< endl;
////                std::cout << "getAdvPrefix(i)" << ie->ipv6Data()->getAdvPrefix(i).prefix << endl;
//            }

            ipv6int->joinMulticastGroup(IPv6Address::ALL_OSPF_ROUTERS_MCAST);//TODO - join only once
            ipv6int->assignAddress(IPv6Address::ALL_OSPF_ROUTERS_MCAST, false, 0, 0);
        }
    }

    EV_DEBUG << "Interface to Process: \n";
    for(auto it=this->interfaceToProcess.begin(); it!=this->interfaceToProcess.end(); it++)
        EV_DEBUG << "\tinterface " << (*it).first << " mapped to process(es) " << (*it).second.first << ", " <<
        (*it).second.second << endl;

    EV_DEBUG << "Process to Interface: \n";
    for(auto it=this->processToInterface.begin(); it!=this->processToInterface.end(); it++)
        EV_DEBUG << "\tprocess " << (*it).first << " is mapped to " << (*it).second << endl;
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
    std::istringstream ss(processID);
    int processIdNum;
    ss>>processIdNum;
    for(auto it=this->processesModules.begin(); it!=this->processesModules.end(); it++){
        if((*it)->getProcessID()==processIdNum) {
            //two processes with same ID are defined
            throw cRuntimeError("Duplicate process found, no new process created");
            return;
        }
    }

    EV_DEBUG << "New process " << processID << " with routerID " << routerID << " created\n";

    this->processInVector.insert(std::make_pair(processID,gateIndex));//[*processID]=gateCount;
    std::string processFullName = "process" + processID;

    OSPFv3Process* newProcessModule = (OSPFv3Process*)newProcessType->create(processFullName.c_str(), this->routingModule);


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
