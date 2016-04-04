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


namespace inet {

Define_Module(HSRPVirtualRouter);

HSRPVirtualRouter::HSRPVirtualRouter() {
    // TODO Auto-generated constructor stub

}

/**
 * Startup initializacion of HSRPVirtualRouter
 */
void HSRPVirtualRouter::initialize(int stage)
{

    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_ROUTING_PROTOCOLS) { //TODO nevim sli to je treba v tomhle modulu
        hsrpUdpPort = 1985;
        socket = new UDPSocket(); //UDP socket used for sending messages
        HsrpMulticast = new L3Address(par("multicastIPv4")); //HSRP multicast address
        hellotimer = new cMessage("helloTimer");
        standbytimer = new cMessage("standbyTimer");
        activetimer = new cMessage("activeTimer");
        initmessage = new cMessage("startHSRP");
        HSRPgroup = (int)par("vrid").doubleValue();
        if (strcmp(par("virtualIP").stringValue(), "") != 0){
            virtualIP = new IPv4Address(par("virtualIP").stringValue());
        }
        scheduleAt(simTime() , initmessage);
        setVirtualMAC();
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this); //usable interfaces of tihs router
//        containingModule = findContainingNode(this);
        arp = getModuleFromPar<ARP>(par("arp"), this);//dynamic_cast<ARP *>(containingModule->getSubmodule("networkLayer")->getSubmodule("ipv4")->getSubmodule("arp"));
    }
}

/**
 * Omnet++ function for handeling incoming messages
 */
void HSRPVirtualRouter::handleMessage(cMessage *msg)
{
//    const_simtime_t t = 3;  //TODO just some time
    HSRPMessage *HSRPm = dynamic_cast<HSRPMessage *>(msg);

    if (msg->isSelfMessage())
    {//self state change
        std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Got SELF message:"<<std::endl;
        std::cout<<msg->getFullName()<<std::endl;
        fflush(stdout);
        if (msg == initmessage){
            HsrpState = INIT;
            std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Init state;"<<std::endl;
            fflush(stdout);
            initState();
            return;
        }
        switch(HsrpState){
            case LISTEN:
                if (msg == standbytimer){
                    HsrpState = SPEAK;
                    std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Speak state;"<<std::endl;
                    fflush(stdout);
                    scheduleTimer(standbytimer);
                    scheduleTimer(hellotimer);
                }
                else if (msg == activetimer){
                    HsrpState = SPEAK;
                    std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Speak state;"<<std::endl;
                    fflush(stdout);
                    scheduleTimer(standbytimer);
                    scheduleTimer(activetimer);
                    scheduleTimer(hellotimer);
                }
                break;

            case SPEAK:
                if (msg == standbytimer){
                    HsrpState = STANDBY;
                    std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Standby state;"<<std::endl;
                    fflush(stdout);
                    cancelEvent(standbytimer);
                }
                else if(msg == hellotimer)
                {
                    sendMessage(HELLO);//FIXME- je to good?
                    scheduleTimer(hellotimer);
                }
                break;

            case STANDBY:
                if (msg == activetimer){
                    //FIXME zrusit zrusit timery CD--zkontrolovat
                    cancelEvent(standbytimer);
                    cancelEvent(activetimer);
                    HsrpState = ACTIVE;
                    std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]ACtive state;"<<std::endl;
                    fflush(stdout);
                    sendMessage(HELLO);
                    const InterfaceEntry *destInterface = ift->getInterfaceByName("eth0"); //TODO
                    printf("id:%d",(destInterface->getInterfaceId()));
                    arp->sendARPGratuitous(destInterface,*(virtualMAC), *(virtualIP), ARP_REPLY);//TODO send gratious ARP
                    printf("iid:%d, virmac:", ift->getInterfaceByName("eth0")->getInterfaceId());
                    std::cout<<virtualMAC->str()<<"virtip"<<virtualIP->str()<<std::endl;
                    fflush(stdout);
                }
                else if (msg == hellotimer){
                    sendMessage(HELLO);
                    scheduleTimer(hellotimer);
                }
                break;

            case ACTIVE:
                break;
            default:
                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]default-;"<<std::endl;
                fflush(stdout);
                break;
        }
    }
    else
    {//got message from another router

        //FIXME Zkontroluj sli spada do moji skupiny a zajima me...?
        if ((double)HSRPm->getGroup() != par("vrid").doubleValue()){
            EV_DEBUG << "Wrong message with GID:" <<HSRPm->getGroup() << "\n";
            return;
        }

        std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Got messag!!!!!!!!!!!"<<std::endl;
        fflush(stdout);
        switch(HsrpState){
            case LEARN:
                std::cout<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]hndl msg LEARN:"<<std::endl;
                fflush(stdout);
                handleMessageLearn(HSRPm);
                break;
            case LISTEN:
                std::cout<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"] hndl msg LISTEN"<<std::endl;
                fflush(stdout);
                handleMessageListen(HSRPm);
                break;
            case SPEAK:
                std::cout<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"] SPEAK state"<<std::endl;
                fflush(stdout);
                handleMessageSpeak(HSRPm);
                break;
            case STANDBY:
                std::cout<<par("deviceId").getName()<<"]"<<HSRPgroup<<"] STANDBY state"<<std::endl;
                fflush(stdout);
                handleMessageStandby(HSRPm); break;
            case ACTIVE: handleMessageActive(HSRPm); break;
            default: return;
        }
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
    std::cout<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"] INIT state"<<std::endl;

    bindMulticast();

    // Check virtualIP
    if (strcmp(par("virtualIP").stringValue(), "") != 0)
    { //virtual ip is already set
        listenState();
    }
    else
    { //virtual ip is not set
        learnState();
        //TODO event A, B - spusteni obou timeru -- proc?
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
    std::cout<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"] LISTEN state"<<std::endl;
    fflush(stdout);

    //StartTimers
    scheduleTimer(activetimer);
    scheduleTimer(standbytimer);
    //TODO learn params
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
        if (msg->getState() == ACTIVE){ //TODO podle RFC pouze ACTIVE... zjistit GNS jestli ne i STANDBY
            std::cout<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"] got- "<<msg->getAddress().str(true)<<std::endl;
            fflush(stdout);
            IPv4Address *a = new IPv4Address(msg->getAddress());
            virtualIP = a;
            fflush(stdout);
            listenState();
        }
    }
}

void HSRPVirtualRouter::handleMessageStandby(HSRPMessage *msg)
{
    std::cout<<"handle msg Standby"<<std::endl;

    switch(msg->getState()){
        case ACTIVE:
            if (msg->getPriority() < par("priority").doubleValue()){ // TODO if preempt enable
                HsrpState = ACTIVE;
                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Active state;"<<std::endl;
                fflush(stdout);
                scheduleTimer(standbytimer);
                //TODO:
                //send coup m.
                sendMessage(HELLO);
                //TODO send gratious arp
                const InterfaceEntry *destInterface = ift->getInterfaceByName("eth0"); //TODO
                arp->sendARPGratuitous(destInterface,*(virtualMAC), *(virtualIP), ARP_REPLY);//TODOX
                break;
            }
            else if (msg->getPriority() > par("priority").doubleValue()){
                //TODO learn params
                scheduleTimer(activetimer);
            }
            if (msg->getOp_code() == RESIGN){
                HsrpState = ACTIVE;
                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Active state;"<<std::endl;
                fflush(stdout);
                cancelEvent(activetimer);
                sendMessage(HELLO);
                //TODO send gratious ARP
                const InterfaceEntry *destInterface = ift->getInterfaceByName("eth0"); //TODO
                arp->sendARPGratuitous(destInterface,*(virtualMAC), *(virtualIP), ARP_REPLY);//TODO send gratious ARP
            }
            break;
        case SPEAK:
            if (msg->getPriority() > par("priority").doubleValue()){
                HsrpState = LISTEN;
                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Listen state;"<<std::endl;
                fflush(stdout);
                scheduleTimer(standbytimer);
            }
            break;
        case STANDBY:
            if (msg->getPriority() > par("priority").doubleValue()){
                HsrpState = LISTEN;
                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Listen state;"<<std::endl;
                fflush(stdout);
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
    std::cout<<"handle msg Speak"<<std::endl;
    switch(msg->getState()){
        case SPEAK:
            if (msg->getPriority() > par("priority").doubleValue()){
                HsrpState = LISTEN;
                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Listen state;"<<std::endl;
                fflush(stdout);
                scheduleTimer(standbytimer);
            }
            break;
        case STANDBY:
            if (msg->getPriority() < par("priority").doubleValue()){
                HsrpState = STANDBY;
                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Standby state;"<<std::endl;
                fflush(stdout);
                if (standbytimer->isScheduled())
                    cancelEvent(standbytimer);
            }
            else
            {
                HsrpState = LISTEN;
                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Listen state;"<<std::endl;
                fflush(stdout);
                scheduleTimer(standbytimer);
            }
            break;
        case ACTIVE: //TODO just if preemption is set
            if (msg->getPriority() < par("priority").doubleValue()){
                HsrpState = ACTIVE;
                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Active state;"<<std::endl;
                fflush(stdout);
                scheduleTimer(standbytimer);
                //TODO send coup message to active
                //TODO send hello message with current state atd
                //TODO gratious ARP? WTF?
                const InterfaceEntry *destInterface = ift->getInterfaceByName("eth0"); //TODO
                arp->sendARPGratuitous(destInterface,*(virtualMAC), *(virtualIP), ARP_REPLY);//TODO send gratious ARP
            }
            break;

    }
}

void HSRPVirtualRouter::handleMessageActive(HSRPMessage *msg)
{
    std::cout<<"handle msg Active"<<std::endl;
    fflush(stdout);
    return;
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
    std::cout<<"handle msg Listen"<<std::endl;
    switch (msg->getState()){
        case ACTIVE:
            if (msg->getOp_code() == RESIGN){
                HsrpState = SPEAK;
                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Speak state;"<<std::endl;
                fflush(stdout);
            }
            if (msg->getPriority() < par("priority").doubleValue()){ //TODO Jen kdyz je nastavena preempce!!
                HsrpState = ACTIVE;
                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Active state;"<<std::endl;
                fflush(stdout);
            }
            else
            {
                scheduleTimer(activetimer);
                //TODO E event: Learn params
            }
            break;
        case STANDBY:
            if (msg->getPriority() < par("priority").doubleValue()){
                HsrpState = SPEAK;
                std::cout<<"\n"<<par("deviceId").stringValue()<<"]"<<HSRPgroup<<"]Speak state;"<<std::endl;
                fflush(stdout);
            }
            else
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

void HSRPVirtualRouter::bindMulticast(){
    //bind to multicast for listening of other HSRP routers
    socket->setOutputGate(gate("udpOut"));
    socket->bind(hsrpUdpPort);
    socket->joinMulticastGroup(*HsrpMulticast,ift->getInterfaceByName("eth0")->getInterfaceId());//TODO for now just specific Interface
}


void HSRPVirtualRouter::scheduleTimer(cMessage *msg)
{
    if (msg->isScheduled()){
        cancelEvent(msg);
    }
    if (msg == activetimer)
        scheduleAt(simTime() + par("holdtime").doubleValue(), activetimer);
    if (msg == standbytimer)
        scheduleAt(simTime() + par("holdtime").doubleValue(), standbytimer);
    if (msg == hellotimer)
        scheduleAt(simTime() + par("hellotime").doubleValue(), hellotimer);
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
    HSRPMessage *packet = generateMessage(opCode);
    packet->setBitLength(HSRP_HEADER_SIZE);
    const InterfaceEntry *destInterface = ift->getInterfaceByName("eth0"); //TODO for now just specific Interface
    UDPSocket::SendOptions options;
    options.outInterfaceId = destInterface->getInterfaceId();
//    options.srcAddr = ift->getInterfaceByName("eth0")->ipv4Data()->getIPAddress(); //getInterfaceByName("eth0")->ipv4Data()->getIPAddress(); //TODO srcIP?? how to get
    socket->setTimeToLive(1);
    socket->sendTo(packet, *HsrpMulticast, hsrpUdpPort, &options);
}


HSRPVirtualRouter::~HSRPVirtualRouter() {
    // TODO Auto-generated destructor stub
}

}
