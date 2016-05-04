/*
 * HSRPVirtualRouter.cc
 *
 *  Created on: 24. 3. 2016
 *      Author: Jan Holusa
 */

#include "HSRPVirtualRouter.h"
#include <iostream>
#include <sstream>
#include <iomanip>

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/arp/ipv4/ARPPacket_m.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"

#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/NotifierConsts.h"


namespace inet {

Define_Module(HSRPVirtualRouter);

HSRPVirtualRouter::HSRPVirtualRouter() {
//    vf = NULL;
//    arp = NULL;
}

/**
 * Startup initializacion of HSRPVirtualRouter
 */
void HSRPVirtualRouter::initialize(int stage)
{

    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
//        hostname = par("deviceId").stdstringValue();
        containingModule = getContainingNode(this);
        hostname = std::string(containingModule->getName(), strlen(containingModule->getName()));
        //setup HSRP parameters
        hsrpUdpPort = HSRP_UDP_PORT;
        hsrpMulticast = new L3Address(HSRP_MULTICAST_ADDRESS.c_str());
        hsrpGroup = (int)par("group");
        virtualIP = new IPv4Address(par("virtualIP").stringValue());
        priority = (int)par("priority");
        preempt = (bool)par("preempt");
        hsrpState = DISABLED;
        setVirtualMAC();

        printf("PARAMS:\ngroup: %d; prio: %d, preempt: %d\n", hsrpGroup, (int)priority, preempt);

        //setup Timers
        helloTime = par("hellotime");
        holdTime = par("holdtime");
        hellotimer = new cMessage("helloTimer");
        standbytimer = new cMessage("standbyTimer");
        activetimer = new cMessage("activeTimer");
        initmessage = new cMessage("startHSRP");

        //get neccessary OMNET parameters
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        arp = getModuleFromPar<ARP>(par("arp"), this);
        ie = dynamic_cast<AnsaInterfaceEntry *>(ift->getInterfaceById((int) par("interface")));

        //add another VF to the interface
        vf = new VirtualForwarder();
        vf->addIPAddress(*virtualIP);
        vf->setMACAddress(*virtualMAC);
        ie->addVirtualForwarder(vf);

        //get socket ready
        socket = new UDPSocket();
        socket->setOutputGate(gate("udpOut"));

        //subscribe to notifications
//        containingModule->subscribe(NF_INTERFACE_CREATED, this);
//        containingModule->subscribe(NF_INTERFACE_DELETED, this);
        containingModule->subscribe(NF_INTERFACE_STATE_CHANGED, this);

        //start HSRP
        scheduleAt(simTime() , initmessage);
    }

}

/**
 * Omnet++ function for handeling incoming messages
 */
void HSRPVirtualRouter::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {//self state change
//        std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"]Got SELF message:"<<std::endl;
//        std::cout<<msg->getFullName()<<std::endl;
//        fflush(stdout);
        if (msg == initmessage && hsrpState == DISABLED){
            hsrpState = INIT;
            DebugStateMachine(DISABLED, INIT);
            vf->setDisable();
//            std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"]Init state;"<<std::endl;
//            fflush(stdout);
            initState();
            return;
        }
        switch(hsrpState){
            case LISTEN:
                if (msg == standbytimer){
                    hsrpState = SPEAK;
//                    std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"]Speak state;"<<std::endl;
//                    fflush(stdout);
                    DebugStateMachine(LISTEN, SPEAK);
                    scheduleTimer(standbytimer);
                    scheduleTimer(hellotimer);
                }
                else if (msg == activetimer){
                    hsrpState = SPEAK;
//                    std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"]Speak state;"<<std::endl;
//                    fflush(stdout);
                    DebugStateMachine(LISTEN, SPEAK);
                    scheduleTimer(standbytimer);
                    scheduleTimer(activetimer);
                    scheduleTimer(hellotimer);
                }
                break;

            case SPEAK:
                if (msg == standbytimer){
                    if (standbytimer->isScheduled())
                        cancelEvent(standbytimer);
                    hsrpState = STANDBY;
                    DebugStateMachine(SPEAK, STANDBY);
//                    std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"]Standby state;"<<std::endl;
//                    fflush(stdout);
                }
                else if(msg == hellotimer)
                {
                    sendMessage(HELLO);//FIXME- je to good?
                    scheduleTimer(hellotimer);
                }else if (msg == activetimer){
                    scheduleTimer(activetimer);
                }
                break;

            case STANDBY:
                if (msg == activetimer){
                    if (standbytimer->isScheduled())
                        cancelEvent(standbytimer);
                    if (activetimer->isScheduled())
                        cancelEvent(activetimer);
                    hsrpState = ACTIVE;
                    DebugStateMachine(STANDBY, ACTIVE);
                    vf->setEnable();
//                    std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"]ACtive state;"<<std::endl;
//                    fflush(stdout);
                    sendMessage(HELLO);
                    arp->sendARPGratuitous(ie,*(virtualMAC), *(virtualIP), ARP_REPLY);
//                    printf("iid:%d, virmac:", ift->getInterfaceByName("eth0")->getInterfaceId());
//                    std::cout<<virtualMAC->str()<<"virtip"<<virtualIP->str()<<std::endl;
//                    fflush(stdout);
                }
                else if (msg == hellotimer){
                    sendMessage(HELLO);
                    scheduleTimer(hellotimer);
                }
                break;

            case ACTIVE:
                if (msg == hellotimer){
                    sendMessage(HELLO);
                    scheduleTimer(hellotimer);
                }
                break;
            default:
//                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"]"<<intToHsrpState(hsrpState)<<" default-;"<<std::endl;
//                fflush(stdout);
                break;
        }
    }
    else
    {//got message from another router
        HSRPMessage *HSRPm = dynamic_cast<HSRPMessage *>(msg);

        //FIXME Zkontroluj sli spada do moji skupiny a zajima me...?
        if ((double)HSRPm->getGroup() != par("group").doubleValue()){
            EV_DEBUG << "Wrong message with GID:" <<HSRPm->getGroup() << "\n";
            return;
        }

//        std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"]Got messag!!!!!!!!!!!"<<std::endl;
//        fflush(stdout);
        DebugGetMessage(HSRPm);

        switch(hsrpState){
            case LEARN:
//                std::cout<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"]hndl msg LEARN:"<<std::endl;
//                fflush(stdout);
                handleMessageLearn(HSRPm);
                break;
            case LISTEN:
//                std::cout<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"] hndl msg LISTEN"<<std::endl;
//                fflush(stdout);
                handleMessageListen(HSRPm);
                break;
            case SPEAK:
//                std::cout<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"] SPEAK state"<<std::endl;
//                fflush(stdout);
                handleMessageSpeak(HSRPm);
                break;
            case STANDBY:
//                std::cout<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"] STANDBY state"<<std::endl;
//                fflush(stdout);
                handleMessageStandby(HSRPm); break;
            case ACTIVE:
//                std::cout<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"] ACTIVE state"<<std::endl;
//                fflush(stdout);
                handleMessageActive(HSRPm); break;
            default: return;
        }
        delete HSRPm;
    }
} //end handleMessage

/*
 * State machine
 */

/*
 * initState()
 * If HSRP is functionally seted up->change self state to LISTEN
 * if it is NOT -> change self state to LEARN
 * and send HELLO self-message in hellotime
 */
void HSRPVirtualRouter::initState(){
//    std::cout<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"] INIT state"<<std::endl;

    vf->setDisable();
    // Check virtualIP
    if (strcmp(virtualIP->str(false).c_str(), "0.0.0.0") != 0)
    { //virtual ip is already set
        DebugStateMachine(INIT, LISTEN);
        listenState();
    }
    else
    { //virtual ip is not set
        DebugStateMachine(INIT, LEARN);
        learnState();
    }
}

/*
 * learnState()
 *
 */
void HSRPVirtualRouter::learnState()
{
    hsrpState = LEARN;
//    std::cout<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"] LEARN state"<<std::endl;
//    fflush(stdout);
    scheduleTimer(activetimer);
    scheduleTimer(standbytimer);
    //TODO v pripade vypnuti rozhrani -> prechod do INIT a vypnuti timeru... je mozne vypnuti rozhrani za behu?
}

/*
 * listenState()
 * Router is not Active neither Standby. Just listening
 * all kind of msgs from Active and Standby.
 */
void HSRPVirtualRouter::listenState(){
    //kontrola
    //vyprseni timeru! ->eventualni prechody do speak
    //nebo active
    hsrpState = LISTEN;
//    std::cout<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"] LISTEN state"<<std::endl;
//    fflush(stdout);

    //StartTimers
    scheduleTimer(activetimer);
    scheduleTimer(standbytimer);
}

void HSRPVirtualRouter::speakState(){
    //TODO vypne se rozhranni -> INIT a stop timers

}
/*
 * State machine END
 */

void HSRPVirtualRouter::handleMessageLearn(HSRPMessage *msg)
{
//    zkontroluj zda msg obsahuje IPv4 adresu - sli jo, tak si ji uloz jako virtual
//    a prejdi do stavu listen
//    msg->getAddress();
    if (msg->getAddress().isUnspecified()){
        std::cout<<"IP4 is NOT set"<<std::endl;
        fflush(stdout);
    }else{
        //h,g - receipe Hello of any priority from ACTIVE router
        if (msg->getState() == ACTIVE){
//            std::cout<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"] got- "<<msg->getAddress().str(true)<<std::endl;
//            fflush(stdout);
//            IPv4Address *a = new IPv4Address();
            virtualIP->set(msg->getAddress().str().c_str());
            DebugStateMachine(LEARN, LISTEN);
            listenState();
        }
    }
}

void HSRPVirtualRouter::handleMessageStandby(HSRPMessage *msg)
{
//    std::cout<<"handle msg Standby"<<std::endl;

    switch(msg->getState()){
        case ACTIVE:
            //h - lower priority from Active
//            if (msg->getPriority() < priority){
            if (isHigherPriorityThan(msg)) {
                if (preempt){
                    if (activetimer->isScheduled())
                        cancelEvent(activetimer);
                    hsrpState = ACTIVE;
                    DebugStateMachine(STANDBY, ACTIVE);
                    vf->setEnable();
//                    std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"]Active state;"<<std::endl;
//                    fflush(stdout);
                    scheduleTimer(standbytimer);
                    sendMessage(COUP);
                    sendMessage(HELLO);
//                    const InterfaceEntry *destInterface = ift->getInterfaceByName("eth0");
                    arp->sendARPGratuitous(ie,*(virtualMAC), *(virtualIP), ARP_REPLY);
                }else{
                    scheduleTimer(activetimer);
                }
                break;
            }
            //g - higher priority from Active
            else if (!isHigherPriorityThan(msg)){
                learnTimers(msg);
                scheduleTimer(activetimer);
            }
            //i - resign from Active
            if (msg->getOp_code() == RESIGN){
                if (activetimer->isScheduled())
                    cancelEvent(activetimer);
                hsrpState = ACTIVE;
                DebugStateMachine(STANDBY, ACTIVE);
                vf->setEnable();
//                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"]Active state;"<<std::endl;
//                fflush(stdout);
                sendMessage(HELLO);
//                const InterfaceEntry *destInterface = ift->getInterfaceByName("eth0");
                arp->sendARPGratuitous(ie,*(virtualMAC), *(virtualIP), ARP_REPLY);
            }
            break;
        case SPEAK:
            //f - higher prio from Speak
            if (!isHigherPriorityThan(msg)){
                hsrpState = LISTEN;
                DebugStateMachine(STANDBY, LISTEN);
//                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"]Listen state;"<<std::endl;
//                fflush(stdout);
                scheduleTimer(standbytimer);
            }
            break;
        case STANDBY:
            //k - higher priority from Standby
            if (!isHigherPriorityThan(msg)){
                hsrpState = LISTEN;
                DebugStateMachine(STANDBY, LISTEN);
//                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"]Listen state;"<<std::endl;
//                fflush(stdout);
                scheduleTimer(standbytimer);
            }
            break;
    }
}

/*
 * handleMessageSpeak:
 * Router is sending Hello msg and is
 * attending Active and Standby Router election
 */
void HSRPVirtualRouter::handleMessageSpeak(HSRPMessage *msg)
{
//    std::cout<<"handle msg Speak"<<std::endl;
    switch(msg->getState()){
        case SPEAK:
            //f - higher prio from Speak
            if (!isHigherPriorityThan(msg)){
                hsrpState = LISTEN;
                DebugStateMachine(SPEAK, LISTEN);
//                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"]Listen state;"<<std::endl;
//                fflush(stdout);
                scheduleTimer(standbytimer);
            }
            break;
        case STANDBY:
            //l - lower prio from Standby
            if (isHigherPriorityThan(msg)){
                hsrpState = STANDBY;
                DebugStateMachine(SPEAK, STANDBY);
//                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"]Standby state;"<<std::endl;
//                fflush(stdout);
                if (standbytimer->isScheduled())
                    cancelEvent(standbytimer);
            }
            else{//k - higher prio from Standby
                hsrpState = LISTEN;
                DebugStateMachine(SPEAK, LISTEN);
//                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"]Listen state;"<<std::endl;
//                fflush(stdout);
                scheduleTimer(standbytimer);
            }
            break;
        case ACTIVE:
            //h - lower prio from Active
            if (isHigherPriorityThan(msg)){
                if (preempt){
                    if (activetimer->isScheduled())
                        cancelEvent(activetimer);
                    hsrpState = ACTIVE;
                    DebugStateMachine(SPEAK, ACTIVE);
                    vf->setEnable();
//                    std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"]Active state;"<<std::endl;
//                    fflush(stdout);
                    scheduleTimer(standbytimer);
                    sendMessage(COUP);
                    sendMessage(HELLO);
//                    const InterfaceEntry *destInterface = ift->getInterfaceByName("eth0");
                    arp->sendARPGratuitous(ie,*(virtualMAC), *(virtualIP), ARP_REPLY);
                }else{
                    scheduleTimer(activetimer);
                }
            }
            else{ //g - higher prio from Active
                learnTimers(msg);
                scheduleTimer(activetimer);
            }
            break;

    }
}

void HSRPVirtualRouter::handleMessageActive(HSRPMessage *msg)
{
//    std::cout<<"handle msg Active"<<std::endl;
//    fflush(stdout);
    switch (msg->getState()){
        case ACTIVE:
            //g - higher priority from Active
            if (!isHigherPriorityThan(msg)){
                hsrpState = SPEAK;
                DebugStateMachine(ACTIVE, SPEAK);
                vf->setDisable();
                scheduleTimer(activetimer);
                scheduleTimer(standbytimer);
            }
            else{//h - lower priority from Active
                sendMessage(COUP);
            }
            break;
        case STANDBY:
            //k,l - higher/lower prio from standby
            scheduleTimer(standbytimer);
    }

    //j - higher priority COUP from some router
    if (msg->getOp_code() == COUP){
        if (!isHigherPriorityThan(msg)){
            hsrpState = SPEAK;
            DebugStateMachine(ACTIVE, SPEAK);
            vf->setDisable();
            scheduleTimer(activetimer);
            scheduleTimer(standbytimer);
            sendMessage(RESIGN);
        }
    }
}

/*
 * handleMessageListen:
 *
 */
void HSRPVirtualRouter::handleMessageListen(HSRPMessage *msg)
{
//    Prijal resign zpravu od active -> SPEAK
//    Prijal hello s nizkou prio od standby-> SPEAK
//    Prijal Hello s malou prio od Active -> ACTIVE
//    std::cout<<"handle msg Listen"<<std::endl;
    switch (msg->getState()){
        case ACTIVE:
            if (msg->getOp_code() == RESIGN){
                hsrpState = SPEAK;
                DebugStateMachine(LISTEN, SPEAK);
                sendMessage(HELLO);//TODO-fakt ji posila rovnou?
//                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"]Speak state;"<<std::endl;
//                fflush(stdout);
                scheduleTimer(activetimer);
                scheduleTimer(standbytimer);
                scheduleTimer(hellotimer);
            }
            //h - lower prio from Active
            if (isHigherPriorityThan(msg)){
                if (preempt){
                    if (activetimer->isScheduled())
                        cancelEvent(activetimer);
                    hsrpState = ACTIVE;
                    DebugStateMachine(LISTEN, ACTIVE);
                    vf->setEnable();
//                    std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"]Active state;"<<std::endl;
//                    fflush(stdout);
                    scheduleTimer(standbytimer);
                    sendMessage(COUP);
                    sendMessage(HELLO);
                    arp->sendARPGratuitous(ie,*(virtualMAC), *(virtualIP), ARP_REPLY);
                    scheduleTimer(hellotimer);
                }else{
                    scheduleTimer(activetimer);
                }
            }
            else// g - higher prio from Active
            {
                scheduleTimer(activetimer);
                learnTimers(msg);
            }
            break;
        case STANDBY:
            //l - receipt Hello of lower prio from Standby
            if (isHigherPriorityThan(msg)){
                hsrpState = SPEAK;//TODO poslat rovnou HELLO?
                DebugStateMachine(LISTEN, SPEAK);
//                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"]Speak state;"<<std::endl;
//                fflush(stdout);
                scheduleTimer(standbytimer);
                scheduleTimer(hellotimer);
            }
            else//k - receipt Hello of higher prio from Standby
            {
                scheduleTimer(standbytimer);
            }
            break;
    }
}
/***************************************************************************************************/
/*
 * Other usefull functions
 *
 */

//void HSRPVirtualRouter::parseConfig(cXMLElement *config)
//{
//
//}

void HSRPVirtualRouter::setVirtualMAC()
{
    virtualMAC = new MACAddress("00-00-0C-07-AC-00");
    virtualMAC->setAddressByte(5, hsrpGroup);
//    EV_DEBUG<<"routerID:"<<par("deviceId").str()<<"vMAC:"<<virtualMAC->str()<<"\n";
}

void HSRPVirtualRouter::scheduleTimer(cMessage *msg)
{
    if (msg->isScheduled()){
        cancelEvent(msg);
    }
    if (msg == activetimer)
        scheduleAt(simTime() + holdTime, activetimer);
    if (msg == standbytimer)
        scheduleAt(simTime() + holdTime, standbytimer);
    if (msg == hellotimer)
        scheduleAt(simTime() + helloTime, hellotimer);
}

HSRPMessage *HSRPVirtualRouter::generateMessage(OP_CODE opCode)
{
    HSRPMessage* msg = new HSRPMessage("HSRPMessage");
    msg->setVersion(par("version"));
    msg->setOp_code(opCode);
    msg->setState(hsrpState);
    msg->setHellotime(par("hellotime"));
    msg->setHoldtime(par("holdtime"));
    msg->setPriority(par("priority"));
    msg->setGroup(hsrpGroup);
    if (virtualIP != nullptr)
        msg->setAddress(*(virtualIP));

    return msg;
}

void HSRPVirtualRouter::sendMessage(OP_CODE opCode)
{
    DebugSendMessage(opCode);
    HSRPMessage *packet = generateMessage(opCode);
    packet->setBitLength(HSRP_HEADER_SIZE);

    UDPSocket::SendOptions options;
    options.outInterfaceId = ie->getInterfaceId();
    options.srcAddr = ie->ipv4Data()->getIPAddress();

    socket->setTimeToLive(1);
    socket->sendTo(packet, *hsrpMulticast, hsrpUdpPort, &options);
}

void HSRPVirtualRouter::learnTimers(HSRPMessage *msg)
{
    //learn hello and holdhold timer from ACTIVE router HELLO message
    if (msg->getHellotime() != helloTime){
        helloTime = msg->getHellotime();
//        std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"]Changing helloTime;"<<std::endl;
//        fflush(stdout);
    }
    if (msg->getHoldtime() != holdTime){
        holdTime = msg->getHoldtime();
//        std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<hsrpGroup<<"]Changing holdTime;"<<std::endl;
//        fflush(stdout);
    }
}

bool HSRPVirtualRouter::isHigherPriorityThan(HSRPMessage *HSRPm){
    UDPDataIndication *udpInfo = check_and_cast<UDPDataIndication *>(HSRPm->getControlInfo());
    if (HSRPm->getPriority() < priority){
        return true;
    }else if (HSRPm->getPriority() > priority){
        return false;
    }else{// ==
//        std::cout<<"ie IP:"<<ie->ipv4Data()->getIPAddress().str(false)<<"recv mess IP:"<<ci->getSrcAddr().str()<<std::endl;
//        fflush(stdout);
        if (ie->ipv4Data()->getIPAddress() > udpInfo->getSrcAddr().toIPv4() ){
            return true;
        }else{
            return false;
        }
    }
}

HSRPVirtualRouter::~HSRPVirtualRouter() {
//    socket.close();

    cancelAndDelete(hellotimer);
    cancelAndDelete(standbytimer);
    cancelAndDelete(activetimer);
    cancelAndDelete(initmessage);

    // unsubscribe to notifications
//    containingModule->unsubscribe(NF_INTERFACE_CREATED, this);
//    containingModule->unsubscribe(NF_INTERFACE_DELETED, this);
    containingModule->unsubscribe(NF_INTERFACE_STATE_CHANGED, this);
}

/***
 * Signal Handler
 */
/**
 * Listen on interface/route changes and update private data structures.
 */
void HSRPVirtualRouter::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG)
{
    Enter_Method_Silent("HSRPVirtualRouter::receiveChangeNotification(%s)", notificationCategoryName(signalID));

    const AnsaInterfaceEntry *ief;
    const InterfaceEntryChangeDetails *change;

    if (signalID == NF_INTERFACE_STATE_CHANGED) {

        change = check_and_cast<const InterfaceEntryChangeDetails *>(obj);
        ief = check_and_cast<const AnsaInterfaceEntry *>(change->getInterfaceEntry());

        //Is it my interface?
        if (ief->getInterfaceId() == ie->getInterfaceId()){

            //is it change to UP or to DOWN?
            if (ie->isUp()){
                //do Enable VF
                EV<<hostname<<" # "<<ie->getName()<<" Interface up."<<endl;
                interfaceUp();
            }else{
                //do disable VF
                EV<<hostname<<" # "<<ie->getName()<<" Interface down."<<endl;
                interfaceDown();
            }

        }
    }
    else
        throw cRuntimeError("Unexpected signal: %s", getSignalName(signalID));
}

//bool HSRPVirtualRouter::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
//{
//    Enter_Method_Silent();
//
//    if (dynamic_cast<NodeStartOperation *>(operation)) {
//        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_APPLICATION_LAYER) {
//            //start HSRP
//            scheduleAt(simTime() , initmessage);
//        }
//    }
//    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
//        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_APPLICATION_LAYER){
//            stopHSRP();
//        }
//    }
//    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
//        if ((NodeCrashOperation::Stage)stage == NodeCrashOperation::STAGE_CRASH){
//            stopHSRP();
//        }
//
//    }
//    else
//        throw cRuntimeError("Unsupported operation '%s'", operation->getClassName());
//
//    return true;
//}

void HSRPVirtualRouter::interfaceDown(){
    switch(hsrpState){
        case ACTIVE://H
            if (hellotimer->isScheduled())
                cancelEvent(standbytimer);
        default: //C,D
            if (standbytimer->isScheduled())
                cancelEvent(standbytimer);
            if (hellotimer->isScheduled())
                cancelEvent(standbytimer);
            break;
    }//switch

    DebugStateMachine(hsrpState, INIT);
    hsrpState = INIT;
    vf->setDisable();
}

void HSRPVirtualRouter::interfaceUp(){
    initState();
}

void HSRPVirtualRouter::stopHSRP(){
    if (standbytimer->isScheduled())
        cancelEvent(standbytimer);
    if (hellotimer->isScheduled())
        cancelEvent(standbytimer);
    if (activetimer->isScheduled())
        cancelEvent(activetimer);
    vf->setDisable();
    DebugStateMachine(hsrpState, DISABLED);
    hsrpState = DISABLED;
}

/***
 * Debug Messages
 */
void HSRPVirtualRouter::DebugStateMachine(int from, int to){
    std::string fromText, toText;
    fromText = intToHsrpState(from);
    toText = intToHsrpState(to);
    EV<<hostname<<" # "<<ie->getName()<<" Grp "<<hsrpGroup<<" "<<fromText<<" -> "<<toText<<endl;
}


void HSRPVirtualRouter::DebugGetMessage(HSRPMessage *msg){
    std::string type = intToMessageType(msg->getOp_code());
    std::string state = intToHsrpState(msg->getState());
    UDPDataIndication *udpInfo = check_and_cast<UDPDataIndication *>(msg->getControlInfo());
    std::string ipFrom =  udpInfo->getSrcAddr().toIPv4().str(false);

    EV<<hostname<<" # "<<ie->getName()<<" Grp "<<(int)msg->getGroup()<<" "<<type<<" in "<< ipFrom <<" "<<state << " pri "<<(int)msg->getPriority()<<" vIP "<<msg->getAddress().str(false)<<endl;
}

void HSRPVirtualRouter::DebugSendMessage(int op_code){
    std::string type = intToMessageType(op_code);
    std::string state = intToHsrpState(hsrpState);

    EV<<hostname<<" # "<<ie->getName()<<" Grp "<<hsrpGroup<<" "<<type<<" out "<< ie->ipv4Data()->getIPAddress().str(false) <<" "<<state << " pri "<<priority<<" vIP "<<virtualIP->str(false)<<endl;
}

std::string HSRPVirtualRouter::intToMessageType(int msg){
    std::string retval;
    switch(msg){
        case HELLO:
            retval = "Hello"; break;
        case COUP:
            retval = "Coup"; break;
        case RESIGN:
            retval = "Resign"; break;
    }
    return retval;
}

std::string HSRPVirtualRouter::intToHsrpState(int state){
    std::string retval;
    switch(state){
        case INIT:
            retval = "Init"; break;
        case LISTEN:
            retval = "Listen"; break;
        case LEARN:
            retval = "Learn"; break;
        case SPEAK:
            retval = "Speak"; break;
        case STANDBY:
            retval = "Standby"; break;
        case ACTIVE:
            retval = "Active"; break;
        case DISABLED:
            retval = "Disabled"; break;
    }
    return retval;
}

}//namespace inet
