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

#include "HSRP.h"
#include <iostream>
#include <iomanip>

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/arp/ipv4/ARPPacket_m.h"


namespace inet {

Define_Module(HSRP);


HSRP::HSRP() {
    // TODO Auto-generated constructor stub

}

/**
 * Startup initializacion of HSRP
 */
void HSRP::initialize(int stage)
{

    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        hsrpUdpPort = 1985;
        socket = new UDPSocket(); //UDP socket used for sending messages
        HsrpMulticast = new L3Address(par("multicastIPv4")); //HSRP multicast address
        hellotimer = new cMessage("helloTimer");
        standbytimer = new cMessage("standbyTimer");
        activetimer = new cMessage("activeTimer");
        initmessage = new cMessage("startHSRP");
        HSRPgroup = (int)par("group").doubleValue();
        if (strcmp(par("virtualIP").stringValue(), "") != 0){
            virtualIP = new IPv4Address(par("virtualIP").stringValue());
        }
        scheduleAt(simTime() , initmessage);
        setVirtualMAC();
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this); //usable interfaces of tihs router
        containingModule = findContainingNode(this);
        arp = dynamic_cast<ARP *>(containingModule->getSubmodule("networkLayer")->getSubmodule("ipv4")->getSubmodule("arp"));
    }
}

/**
 * Omnet++ function for handeling incoming messages
 */
void HSRP::handleMessage(cMessage *msg)
{
//    const_simtime_t t = 3;  //TODO just some time
    HSRPMessage *HSRPm = dynamic_cast<HSRPMessage *>(msg);

    if (msg->isSelfMessage())
    {//self state change
        std::cout<<"\n"<<par("deviceId").stringValue()<<"]Got SELF message:"<<std::endl;
        std::cout<<msg->getFullName()<<std::endl;
        fflush(stdout);
        if (msg == initmessage){
            HsrpState = INIT;
            std::cout<<"\n"<<par("deviceId").stringValue()<<"]Init state;"<<std::endl;
            fflush(stdout);
            initState();
            return;
        }
        switch(HsrpState){
            case LISTEN:
                if (msg == standbytimer){
                    HsrpState = SPEAK;
                    std::cout<<"\n"<<par("deviceId").stringValue()<<"]Speak state;"<<std::endl;
                    fflush(stdout);
                    scheduleTimer(standbytimer);
                    scheduleTimer(hellotimer);
                }
                else if (msg == activetimer){
                    HsrpState = SPEAK;
                    std::cout<<"\n"<<par("deviceId").stringValue()<<"]Speak state;"<<std::endl;
                    fflush(stdout);
                    scheduleTimer(standbytimer);
                    scheduleTimer(activetimer);
                    scheduleTimer(hellotimer);
                }
                break;

            case SPEAK:
                if (msg == standbytimer){
                    HsrpState = STANDBY;
                    std::cout<<"\n"<<par("deviceId").stringValue()<<"]Standby state;"<<std::endl;
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
                    //FIXME zrusit zrusit timery CD
                    HsrpState = ACTIVE;
                    std::cout<<"\n"<<par("deviceId").stringValue()<<"]ACtive state;"<<std::endl;
                    fflush(stdout);
                    sendMessage(HELLO);
                    const InterfaceEntry *destInterface = ift->getInterfaceByName("eth0"); //TODO
                    printf("hereIgo\n");
                    printf("id:%d",(destInterface->getInterfaceId()));
                    arp->sendARPGratuitous(destInterface,*(virtualMAC), *(virtualIP), ARP_REQUEST);//TODO send gratious ARP
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
                std::cout<<"\n"<<par("deviceId").stringValue()<<"]default-;"<<std::endl;
                fflush(stdout);
                break;
        }
    }
    else
    {//got message from another router

        //FIXME Zkontroluj sli spada do moji skupiny a zajima me...?
        if ((double)HSRPm->getGroup() != par("group").doubleValue()){
            EV_DEBUG << "Wrong message with GID:" <<HSRPm->getGroup() << "\n";
            return;
        }

        std::cout<<"\n"<<par("deviceId").stringValue()<<"]Got messag!!!!!!!!!!!"<<std::endl;
        fflush(stdout);
        switch(HsrpState){
            case LEARN:
                std::cout<<par("deviceId").stringValue()<<"]hndl msg LEARN:"<<std::endl;
                fflush(stdout);
                handleMessageLearn(HSRPm);
                break;
            case LISTEN:
                std::cout<<par("deviceId").stringValue()<<"] hndl msg LISTEN"<<std::endl;
                fflush(stdout);
                handleMessageListen(HSRPm);
                break;
            case SPEAK:
                std::cout<<par("deviceId").stringValue()<<"] SPEAK state"<<std::endl;
                fflush(stdout);
                handleMessageSpeak(HSRPm);
                break;
            case STANDBY:
                std::cout<<par("deviceId").getName()<<"] STANDBY state"<<std::endl;
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
void HSRP::initState(){
    std::cout<<par("deviceId").stringValue()<<"] INIT state"<<std::endl;

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
void HSRP::learnState()
{
    HsrpState = LEARN;
    std::cout<<par("deviceId").stringValue()<<"] LEARN state"<<std::endl;
    fflush(stdout);
    //TODO v pripade vypnuti rozhrani -> prechod do INIT a vypnuti timeru... je mozne vypnuti rozhrani za behu?
}

/*
 * listenState()
 * Router is not Active neither Standby. Just listening
 * all kind of msgs from Active and Standby.
 */
void HSRP::listenState(){
    //kontrola
    //vyprseni timeru! ->eventualni prechody do speak
    //nebo active
    HsrpState = LISTEN;
    std::cout<<par("deviceId").stringValue()<<"] LISTEN state"<<std::endl;
    fflush(stdout);

    //StartTimers
    scheduleTimer(activetimer);
    scheduleTimer(standbytimer);
    //TODO learn params
}

void HSRP::speakState(){
    //TODO vypne se rozhranni -> INIT a stop timers

}
/*
 * State machine END
 */

void HSRP::handleMessageLearn(HSRPMessage *msg)
{
//    zkontroluj zda msg obsahuje IPv4 adresu - sli jo, tak si ji uloz jako virtual
//    a prejdi do stavu listen
//    msg->getAddress();
    if (msg->getAddress().isUnspecified()){
        std::cout<<"IP4 is NOT set"<<std::endl;
        fflush(stdout);
    }else{
        if (msg->getState() == ACTIVE){ //TODO podle RFC pouze ACTIVE... zjistit GNS jestli ne i STANDBY
            std::cout<<par("deviceId").stringValue()<<"] got- "<<msg->getAddress().str(true)<<std::endl;
            fflush(stdout);
            IPv4Address *a = new IPv4Address(msg->getAddress());
            virtualIP = a;
            fflush(stdout);
            listenState();
        }
    }
}

void HSRP::handleMessageStandby(HSRPMessage *msg)
{
    std::cout<<"handle msg Standby"<<std::endl;

    switch(msg->getState()){
        case ACTIVE:
            if (msg->getPriority() < par("priority").doubleValue()){ // TODO if preempt enable
                HsrpState = ACTIVE;
                std::cout<<"\n"<<par("deviceId").stringValue()<<"]Active state;"<<std::endl;
                fflush(stdout);
                scheduleTimer(standbytimer);
                //TODO:
                //send coup m.
                sendMessage(HELLO);
                //TODO send gratious arp
                const InterfaceEntry *destInterface = ift->getInterfaceByName("eth0"); //TODO
                arp->sendARPGratuitous(destInterface,*(virtualMAC), *(virtualIP), ARP_REQUEST);//TODOX
                break;
            }
            else if (msg->getPriority() > par("priority").doubleValue()){
                //TODO learn params
                scheduleTimer(activetimer);
            }
            if (msg->getOp_code() == RESIGN){
                HsrpState = ACTIVE;
                std::cout<<"\n"<<par("deviceId").stringValue()<<"]Active state;"<<std::endl;
                fflush(stdout);
                cancelEvent(activetimer);
                sendMessage(HELLO);
                //TODO send gratious ARP
            }
            break;
        case SPEAK:
            if (msg->getPriority() > par("priority").doubleValue()){
                HsrpState = LISTEN;
                std::cout<<"\n"<<par("deviceId").stringValue()<<"]Listen state;"<<std::endl;
                fflush(stdout);
                scheduleTimer(standbytimer);
            }
            break;
        case STANDBY:
            if (msg->getPriority() > par("priority").doubleValue()){
                HsrpState = LISTEN;
                std::cout<<"\n"<<par("deviceId").stringValue()<<"]Listen state;"<<std::endl;
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
void HSRP::handleMessageSpeak(HSRPMessage *msg)
{
    std::cout<<"handle msg Speak"<<std::endl;
    switch(msg->getState()){
        case SPEAK:
            if (msg->getPriority() > par("priority").doubleValue()){
                HsrpState = LISTEN;
                std::cout<<"\n"<<par("deviceId").stringValue()<<"]Listen state;"<<std::endl;
                fflush(stdout);
                scheduleTimer(standbytimer);
            }
            break;
        case STANDBY:
            if (msg->getPriority() < par("priority").doubleValue()){
                HsrpState = STANDBY;
                std::cout<<"\n"<<par("deviceId").stringValue()<<"]Standby state;"<<std::endl;
                fflush(stdout);
                if (standbytimer->isScheduled())
                    cancelEvent(standbytimer);
            }
            else
            {
                HsrpState = LISTEN;
                std::cout<<"\n"<<par("deviceId").stringValue()<<"]Listen state;"<<std::endl;
                fflush(stdout);
                scheduleTimer(standbytimer);
            }
            break;
        case ACTIVE: //TODO just if preemption is set
            if (msg->getPriority() < par("priority").doubleValue()){
                HsrpState = ACTIVE;
                std::cout<<"\n"<<par("deviceId").stringValue()<<"]Active state;"<<std::endl;
                fflush(stdout);
                scheduleTimer(standbytimer);
                //TODO send coup message to active
                //TODO send hello message with current state atd
                //TODO gratious ARP? WTF?
            }
            break;

    }
}

void HSRP::handleMessageActive(HSRPMessage *msg)
{
    std::cout<<"handle msg Active"<<std::endl;
    fflush(stdout);
    return;
}

/*
 * handleMessageListen:
 *
 */
void HSRP::handleMessageListen(HSRPMessage *msg)
{
//    Prijal resign zpravu od active -> SPEAK
//    Prijal hello s nizkou prio od standby-> SPEAK
//    Prijal Hello s malou prio od Active -> ACTIVE
    std::cout<<"handle msg Listen"<<std::endl;
    switch (msg->getState()){
        case ACTIVE:
            if (msg->getOp_code() == RESIGN){
                HsrpState = SPEAK;
                std::cout<<"\n"<<par("deviceId").stringValue()<<"]Speak state;"<<std::endl;
                fflush(stdout);
            }
            if (msg->getPriority() < par("priority").doubleValue()){ //TODO Jen kdyz je nastavena preempce!!
                HsrpState = ACTIVE;
                std::cout<<"\n"<<par("deviceId").stringValue()<<"]Active state;"<<std::endl;
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
                std::cout<<"\n"<<par("deviceId").stringValue()<<"]Speak state;"<<std::endl;
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

void HSRP::setVirtualMAC()
{
    virtualMAC = new MACAddress("00-00-0C-07-AC-00");
    virtualMAC->setAddressByte(5, HSRPgroup);
    EV_DEBUG<<"routerID:"<<par("deviceId").str()<<"vMAC:"<<virtualMAC->str()<<"\n";
}

void HSRP::bindMulticast(){
    //bind to multicast for listening of other HSRP routers
    socket->setOutputGate(gate("udpOut"));
    socket->bind(hsrpUdpPort);
    socket->joinMulticastGroup(*HsrpMulticast,ift->getInterfaceByName("eth0")->getInterfaceId());//TODO for now just specific Interface
}


void HSRP::scheduleTimer(cMessage *msg)
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

HSRPMessage *HSRP::generateMessage(OP_CODE opCode)
{
    HSRPMessage* msg = new HSRPMessage("HSRPMessage");
    msg->setVersion(par("version"));
    msg->setOp_code(opCode);
    msg->setState(HsrpState);
    msg->setHellotime(par("hellotime"));
    msg->setHoldtime(par("holdtime"));
    msg->setPriority(par("priority"));
    msg->setGroup(par("group"));
    if (virtualIP != nullptr)
        msg->setAddress(*(virtualIP));
    return msg;
}

void HSRP::sendMessage(OP_CODE opCode)
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

HSRP::~HSRP() {
    // TODO Auto-generated destructor stub
    printf("destrukce\n");
}

} /* namespace inet */
