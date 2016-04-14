/*
 * HSRPVirtualRouter.cc
 *
 *  Created on: 24. 3. 2016
 *      Author: Honza
 */

#include "HSRPVirtualRouter.h"
#include <iostream>
#include <sstream>
#include <iomanip>

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/arp/ipv4/ARPPacket_m.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"

namespace inet {

Define_Module(HSRPVirtualRouter);

HSRPVirtualRouter::HSRPVirtualRouter() {
    vf = NULL;
    arp = NULL;
}

/**
 * Startup initializacion of HSRPVirtualRouter
 */
void HSRPVirtualRouter::initialize(int stage)
{

    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        hostname = par("deviceId").stdstringValue();
        containingModule = getContainingNode(this);

        //setup HSRP parameters
        hsrpUdpPort = HSRP_UDP_PORT;
        HsrpMulticast = new L3Address(HSRP_MULTICAST_ADDRESS.c_str());
        HSRPgroup = (int)par("vrid");
        virtualIP = new IPv4Address(par("virtualIP").stringValue());
        priority = (int)par("priority");
        preempt = (bool)par("preempt");
        setVirtualMAC();

        printf("PARAMS:\ngroup: %d; prio: %d, preempt: %d\n", HSRPgroup, (int)priority, preempt);

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
//        std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Got SELF message:"<<std::endl;
//        std::cout<<msg->getFullName()<<std::endl;
//        fflush(stdout);
        if (msg == initmessage){
            HsrpState = INIT;
            DebugStateMachine(DISABLED, INIT);
            vf->setDisable();
//            std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Init state;"<<std::endl;
//            fflush(stdout);
            initState();
            return;
        }
        switch(HsrpState){
            case LISTEN:
                if (msg == standbytimer){
                    HsrpState = SPEAK;
//                    std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Speak state;"<<std::endl;
//                    fflush(stdout);
                    DebugStateMachine(LISTEN, SPEAK);
                    scheduleTimer(standbytimer);
                    scheduleTimer(hellotimer);
                }
                else if (msg == activetimer){
                    HsrpState = SPEAK;
//                    std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Speak state;"<<std::endl;
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
                    HsrpState = STANDBY;
                    DebugStateMachine(SPEAK, STANDBY);
//                    std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Standby state;"<<std::endl;
//                    fflush(stdout);
                }
                else if(msg == hellotimer)
                {
                    sendMessage(HELLO);//FIXME- je to good?
                    scheduleTimer(hellotimer);
                }
                break;

            case STANDBY:
                if (msg == activetimer){
                    if (standbytimer->isScheduled())
                        cancelEvent(standbytimer);
                    if (activetimer->isScheduled())
                        cancelEvent(activetimer);
                    HsrpState = ACTIVE;
                    DebugStateMachine(STANDBY, ACTIVE);
                    vf->setEnable();
//                    std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]ACtive state;"<<std::endl;
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
//                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]"<<intToHsrpState(HsrpState)<<" default-;"<<std::endl;
//                fflush(stdout);
                break;
        }
    }
    else
    {//got message from another router
        HSRPMessage *HSRPm = dynamic_cast<HSRPMessage *>(msg);

        //FIXME Zkontroluj sli spada do moji skupiny a zajima me...?
        if ((double)HSRPm->getGroup() != par("vrid").doubleValue()){
            EV_DEBUG << "Wrong message with GID:" <<HSRPm->getGroup() << "\n";
            return;
        }

//        std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Got messag!!!!!!!!!!!"<<std::endl;
//        fflush(stdout);
        DebugGetMessage(HSRPm);

        switch(HsrpState){
            case LEARN:
//                std::cout<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]hndl msg LEARN:"<<std::endl;
//                fflush(stdout);
                handleMessageLearn(HSRPm);
                break;
            case LISTEN:
//                std::cout<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"] hndl msg LISTEN"<<std::endl;
//                fflush(stdout);
                handleMessageListen(HSRPm);
                break;
            case SPEAK:
//                std::cout<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"] SPEAK state"<<std::endl;
//                fflush(stdout);
                handleMessageSpeak(HSRPm);
                break;
            case STANDBY:
//                std::cout<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"] STANDBY state"<<std::endl;
//                fflush(stdout);
                handleMessageStandby(HSRPm); break;
            case ACTIVE:
//                std::cout<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"] ACTIVE state"<<std::endl;
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
//    std::cout<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"] INIT state"<<std::endl;

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
    HsrpState = LEARN;
    std::cout<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"] LEARN state"<<std::endl;
    fflush(stdout);
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
    HsrpState = LISTEN;
//    std::cout<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"] LISTEN state"<<std::endl;
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
            std::cout<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"] got- "<<msg->getAddress().str(true)<<std::endl;
            fflush(stdout);
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
                    HsrpState = ACTIVE;
                    DebugStateMachine(STANDBY, ACTIVE);
                    vf->setEnable();
//                    std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Active state;"<<std::endl;
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
                HsrpState = ACTIVE;
                DebugStateMachine(STANDBY, ACTIVE);
                vf->setEnable();
//                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Active state;"<<std::endl;
//                fflush(stdout);
                sendMessage(HELLO);
//                const InterfaceEntry *destInterface = ift->getInterfaceByName("eth0");
                arp->sendARPGratuitous(ie,*(virtualMAC), *(virtualIP), ARP_REPLY);
            }
            break;
        case SPEAK:
            //f - higher prio from Speak
            if (!isHigherPriorityThan(msg)){
                HsrpState = LISTEN;
                DebugStateMachine(STANDBY, LISTEN);
//                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Listen state;"<<std::endl;
//                fflush(stdout);
                scheduleTimer(standbytimer);
            }
            break;
        case STANDBY:
            //k - higher priority from Standby
            if (!isHigherPriorityThan(msg)){
                HsrpState = LISTEN;
                DebugStateMachine(STANDBY, LISTEN);
//                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Listen state;"<<std::endl;
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
                HsrpState = LISTEN;
                DebugStateMachine(SPEAK, LISTEN);
//                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Listen state;"<<std::endl;
//                fflush(stdout);
                scheduleTimer(standbytimer);
            }
            break;
        case STANDBY:
            //l - lower prio from Standby
            if (isHigherPriorityThan(msg)){
                HsrpState = STANDBY;
                DebugStateMachine(SPEAK, STANDBY);
//                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Standby state;"<<std::endl;
//                fflush(stdout);
                if (standbytimer->isScheduled())
                    cancelEvent(standbytimer);
            }
            else{//k - higher prio from Standby
                HsrpState = LISTEN;
                DebugStateMachine(SPEAK, LISTEN);
//                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Listen state;"<<std::endl;
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
                    HsrpState = ACTIVE;
                    DebugStateMachine(SPEAK, ACTIVE);
                    vf->setEnable();
//                    std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Active state;"<<std::endl;
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
                HsrpState = SPEAK;
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
            HsrpState = SPEAK;
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
                HsrpState = SPEAK;
                DebugStateMachine(LISTEN, SPEAK);
                sendMessage(HELLO);//TODO-fakt ji posila rovnou?
//                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Speak state;"<<std::endl;
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
                    HsrpState = ACTIVE;
                    DebugStateMachine(LISTEN, ACTIVE);
                    vf->setEnable();
//                    std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Active state;"<<std::endl;
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
                HsrpState = SPEAK;//TODO poslat rovnou HELLO?
                DebugStateMachine(LISTEN, SPEAK);
//                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Speak state;"<<std::endl;
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

void HSRPVirtualRouter::parseConfig(cXMLElement *config)
{

}

void HSRPVirtualRouter::setVirtualMAC()
{
    virtualMAC = new MACAddress("00-00-0C-07-AC-00");
    virtualMAC->setAddressByte(5, HSRPgroup);
    EV_DEBUG<<"routerID:"<<par("deviceId").str()<<"vMAC:"<<virtualMAC->str()<<"\n";
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
    msg->setState(HsrpState);
    msg->setHellotime(par("hellotime"));
    msg->setHoldtime(par("holdtime"));
    msg->setPriority(par("priority"));
    msg->setGroup(HSRPgroup);
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
    socket->sendTo(packet, *HsrpMulticast, hsrpUdpPort, &options);
}

void HSRPVirtualRouter::learnTimers(HSRPMessage *msg)
{
    //learn hello and holdhold timer from ACTIVE router HELLO message
    if (msg->getHellotime() != helloTime){
        helloTime = msg->getHellotime();
        std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Changing helloTime;"<<std::endl;
        fflush(stdout);
    }
    if (msg->getHoldtime() != holdTime){
        holdTime = msg->getHoldtime();
        std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Changing holdTime;"<<std::endl;
        fflush(stdout);
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

void HSRPVirtualRouter::interfaceDown(){
    switch(HsrpState){
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

    DebugStateMachine(HsrpState, INIT);
    HsrpState = INIT;
    vf->setDisable();
}

void HSRPVirtualRouter::interfaceUp(){
    initState();
}

/***
 * Debug Messages
 */
void HSRPVirtualRouter::DebugStateMachine(int from, int to){
    std::string fromText, toText;
    fromText = intToHsrpState(from);
    toText = intToHsrpState(to);
    EV<<hostname<<" # "<<ie->getName()<<" Grp "<<HSRPgroup<<" "<<fromText<<" -> "<<toText<<endl;
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
    std::string state = intToHsrpState(HsrpState);

    EV<<hostname<<" # "<<ie->getName()<<" Grp "<<HSRPgroup<<" "<<type<<" out "<< ie->ipv4Data()->getIPAddress().str(false) <<" "<<state << " pri "<<priority<<" vIP "<<virtualIP->str(false)<<endl;
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
