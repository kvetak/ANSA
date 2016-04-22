/*
 * GLBPVirtualRouter.cc
 *
 *  Created on: 18.4. 2016
 *      Author: Jan Holusa
 */
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

#include "GLBPVirtualRouter.h"
#include <iostream>
#include <sstream>
#include <iomanip>

#include "inet/networklayer/ipv4/IPv4InterfaceData.h"

namespace inet {

Define_Module(GLBPVirtualRouter);

GLBPVirtualRouter::GLBPVirtualRouter() {
}

void GLBPVirtualRouter::initialize(int stage)
{

    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        hostname = par("deviceId").stdstringValue();
        containingModule = getContainingNode(this);

        //setup HSRP parameters
        glbpState = INIT;
        glbpUdpPort = (int)par("glbpUdpPort");
        glbpMulticast = new L3Address(par("glbpMulticastAddress"));
        glbpGroup = (int)par("vrid");
        virtualIP = new IPv4Address(par("virtualIP").stringValue());
        priority = (int)par("priority");
        preempt = (bool)par("preempt");
        setVirtualMAC(1);

        WATCH(glbpState);

        printf("PARAMS:\ngroup: %d; prio: %d, preempt: %d\n", glbpGroup, (int)priority, preempt);

        //setup Timers
        helloTime = par("hellotime");
        holdTime = par("holdtime");
        redirect = par("redirect");
        timeout = par("timeout");
        hellotimer = new cMessage("helloTimer");
        standbytimer = new cMessage("standbyTimer");
        activetimer = new cMessage("activeTimer");
        initmessage = new cMessage("startHSRP");

        //get neccessary OMNET parameters
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        arp = getModuleFromPar<ARP>(par("arp"), this);
        ie = dynamic_cast<AnsaInterfaceEntry *>(ift->getInterfaceById((int) par("interface")));

        //max nuber of vf is 4
        vfVector.reserve(4);

        //TODO Premistit
        //add another VF to the interface
        vfVector[0] = new GLBPVirtualForwarder();
        vfVector[0]->addIPAddress(*virtualIP);
        vfVector[0]->setMACAddress(*virtualMAC);
        ie->addVirtualForwarder(vfVector[0]);

        //get socket ready
        socket = new UDPSocket();
        socket->setOutputGate(gate("udpOut"));
        socket->setReuseAddress(true);
        socket->bind(glbpUdpPort);

        //subscribe to notifications
//        containingModule->subscribe(NF_INTERFACE_CREATED, this);
//        containingModule->subscribe(NF_INTERFACE_DELETED, this);
        containingModule->subscribe(NF_INTERFACE_STATE_CHANGED, this);

        //start GLBP
        scheduleAt(simTime() , initmessage);
    }

}

void GLBPVirtualRouter::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()){
        if (msg == initmessage) {
            initState();
            return;
        }
        switch(glbpState){
            case LISTEN:
                //f,g
                if (msg == activetimer){
                    scheduleTimer(activetimer);
                    scheduleTimer(standbytimer);
                    DebugStateMachine(LISTEN, SPEAK);
                    glbpState = SPEAK;
                    scheduleTimer(hellotimer);
                    sendMessage(HELLO);
                }else if ( msg == standbytimer ){
                    scheduleTimer(standbytimer);
                    DebugStateMachine(LISTEN, SPEAK);
                    glbpState = SPEAK;
                    scheduleTimer(hellotimer);
                    sendMessage(HELLO);
                }else if (msg == hellotimer){
                    scheduleTimer(hellotimer);
                    sendMessage(HELLO);
                }
                break;
            case SPEAK:
                if (msg == standbytimer){//f
                    if (standbytimer->isScheduled())
                        cancelEvent(standbytimer);
                    DebugStateMachine(SPEAK, STANDBY);
                    glbpState = STANDBY;
                    sendMessage(HELLO);
                    scheduleTimer(hellotimer);
                }else
                if (msg == activetimer){
                    if (activetimer->isScheduled())
                        cancelEvent(activetimer);
                    DebugStateMachine(SPEAK, ACTIVE);
                    glbpState = ACTIVE;
                    sendMessage(HELLO);//TODO
                    scheduleTimer(hellotimer);
                }else
                if (msg == hellotimer){
                    sendMessage(HELLO);//TODO
                    scheduleTimer(hellotimer);
                }
                break;
            case STANDBY:
                if (msg == activetimer){//g
                    if (standbytimer->isScheduled())
                        cancelEvent(standbytimer);
                    if (activetimer->isScheduled())
                        cancelEvent(activetimer);
                    DebugStateMachine(STANDBY, ACTIVE);
                    glbpState = ACTIVE;
//                    vf->setEnable();//TODO
                    sendMessage(HELLO);//TODO
                    scheduleTimer(hellotimer);
                }
                if (msg == hellotimer){
                    sendMessage(HELLO);//TODO
                    scheduleTimer(hellotimer);
                }
                break;
            case ACTIVE:
                if (msg == hellotimer){
                    sendMessage(HELLO); //TODO
                    scheduleTimer(hellotimer);
                }
                if (msg == standbytimer){
                    DebugUnknown(STANDBY);
                }
                break;
        }
    }
    else
    {
        GLBPMessage *GLBPm = dynamic_cast<GLBPMessage *>(msg);
        DebugGetMessage(GLBPm);
        //TODO check jestli je tam hello
        //VG Zpracovani ---> pocita s Hello casti
        switch(glbpState){
            case LISTEN:
                handleMessageListen(GLBPm);
                break;
            case SPEAK:
                handleMessageSpeak(GLBPm);
                break;
            case STANDBY:
                handleMessageStandby(GLBPm);
                break;
            case ACTIVE:
                handleMessageActive(GLBPm);
                break;
        }//end switch
        delete GLBPm;
    }//end if self msg
}

/**
 * Msg handlers
 */

void GLBPVirtualRouter::handleMessageListen(GLBPMessage *msg){
    //TODO resign receive
    GLBPHello *hello = check_and_cast<GLBPHello *>(msg->findOptionByType(HELLO,0));

    switch(hello->getVgState()){
    case SPEAK:
        if (!isHigherPriorityThan(msg)){
            scheduleTimer(standbytimer);//FIXME TODO mozna tady nema byt
//        scheduleTimer(activetimer);//FIXME TODO mozna tady nema byt
        }
        break;
    case STANDBY://j
        //lower
        if (isHigherPriorityThan(msg)){
            DebugStateMachine(LISTEN,SPEAK);
            glbpState = SPEAK;
            scheduleTimer(standbytimer);
            sendMessage(HELLO);
            scheduleTimer(hellotimer);
        }
        break;
    case ACTIVE:
        //lower
        if (isHigherPriorityThan(msg)){//l
            if (preempt){
                DebugStateMachine(LISTEN, SPEAK);
                glbpState = SPEAK;
                scheduleTimer(activetimer);
                scheduleTimer(standbytimer);
                sendMessage(HELLO);
                scheduleTimer(hellotimer);
            } else{//TODO KDYZ NENI PREEMPT TAK PREPLANUJ TIMER"":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
                scheduleTimer(activetimer);
            }
        }
        else
        {//higher - k
            //TODO Learn params
            scheduleTimer(activetimer);
        }
        break;
    }
}

void GLBPVirtualRouter::handleMessageSpeak(GLBPMessage *msg){
    //TODO RESIGN
    GLBPHello *hello = check_and_cast<GLBPHello *>(msg->findOptionByType(HELLO,0));

    switch(hello->getVgState()){//TODO msg->getVgState()){
        case SPEAK:
            //h
            if (!isHigherPriorityThan(msg)){
                scheduleTimer(standbytimer);//TODO mozna i activetimer
                scheduleTimer(hellotimer);
                DebugStateMachine(SPEAK, LISTEN);
                glbpState = LISTEN;

            }
            break;
        case STANDBY:
            //i
            if (!isHigherPriorityThan(msg)){
                scheduleTimer(standbytimer);
                scheduleTimer(hellotimer);
                DebugStateMachine(SPEAK, LISTEN);
                glbpState = LISTEN;
            }
            else
            {//j
                if (standbytimer->isScheduled())
                    cancelEvent(standbytimer);
                DebugStateMachine(SPEAK, STANDBY);
                glbpState = STANDBY;
                sendMessage(HELLO);
                scheduleTimer(hellotimer);
            }
            break;
        case ACTIVE:
            if (isHigherPriorityThan(msg)){
                if (preempt){ //l
                    if (activetimer->isScheduled())
                        cancelEvent(activetimer);
                    if (standbytimer->isScheduled())
                        cancelEvent(standbytimer);
                    DebugStateMachine(SPEAK, ACTIVE);
                    glbpState = ACTIVE;
                    sendMessage(HELLO);
                    scheduleTimer(hellotimer);
                }else{
                    scheduleTimer(activetimer);
                }
            }
            else
            {//k
                //TODO LEARN PARAMS
                scheduleTimer(activetimer);
            }
            break;
    }//switch
}

void GLBPVirtualRouter::handleMessageStandby(GLBPMessage *msg){
    //TODO RESIGN RECEIVE
    GLBPHello *hello = check_and_cast<GLBPHello *>(msg->findOptionByType(HELLO,0));

    switch(hello->getVgState()){// TODOmsg->getVgState()){
        case SPEAK:
            if (!isHigherPriorityThan(msg)){
                scheduleTimer(standbytimer);
                scheduleTimer(hellotimer);
                DebugStateMachine(STANDBY, LISTEN);
                glbpState = LISTEN;
            }
            break;
        case ACTIVE:
            //l
            if (!isHigherPriorityThan(msg)){
                //TODO Learn params
                scheduleTimer(activetimer);
            }
            else
            {//k
                if (preempt){
                    if (activetimer->isScheduled())
                        cancelEvent(activetimer);
                    if (standbytimer->isScheduled())
                        cancelEvent(standbytimer);
                    DebugStateMachine(STANDBY, ACTIVE);
                    glbpState = ACTIVE;
//                    vf->setEnable();//TODO
                    sendMessage(HELLO);
                    scheduleTimer(hellotimer);
                }else{
                    scheduleTimer(activetimer);
                }
            }
            break;
    }//end switch
}

void GLBPVirtualRouter::handleMessageActive(GLBPMessage *msg)
{
    GLBPHello *hello = check_and_cast<GLBPHello *>(msg->findOptionByType(HELLO,0));

    switch(hello->getVgState()){//msg->getVgState()){
        case STANDBY:
            scheduleTimer(standbytimer);
            break;
        case ACTIVE:
            if (!isHigherPriorityThan(msg)){//k
                scheduleTimer(activetimer);
                scheduleTimer(standbytimer);
                DebugStateMachine(ACTIVE, SPEAK);
//                vf->setDisable();//TODO
                glbpState = SPEAK;
                sendMessage(HELLO);
                scheduleTimer(hellotimer);
            }
            break;
    }
}

void GLBPVirtualRouter::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG)
{
    Enter_Method_Silent("GLBPVirtualRouter::receiveChangeNotification(%s)", notificationCategoryName(signalID));

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

void GLBPVirtualRouter::initState(){
//    vf->setDisable();//TODO
    // Check virtualIP
    if (strcmp(virtualIP->str(false).c_str(), "0.0.0.0") != 0)
    { //virtual ip is already set
        DebugStateMachine(INIT, LISTEN);
        listenState();
    }
    else
    { //virtual ip is not set
        DebugStateMachine(INIT, DISABLED);
        glbpState = DISABLED;
//        disabledState();
    }
}

void GLBPVirtualRouter::listenState(){
    glbpState = LISTEN;

//    if ()
    scheduleTimer(hellotimer);
    scheduleTimer(activetimer);
    scheduleTimer(standbytimer);
}

void GLBPVirtualRouter::sendMessage(GLBPType type)
{
    DebugSendMessage(type);
    GLBPMessage *packet = generateMessage();
    packet->setBitLength(GLBP_HEADER_BYTES*8);//TODO

    UDPSocket::SendOptions options;
    options.outInterfaceId = ie->getInterfaceId();
    options.srcAddr = ie->ipv4Data()->getIPAddress();

    socket->setTimeToLive(1);
    socket->sendTo(packet, *glbpMulticast, glbpUdpPort, &options);
}

GLBPMessage *GLBPVirtualRouter::generateMessage()
{ //todo udelat obecne ne jen pro hello
    GLBPMessage *msg = new GLBPMessage("GLBPMessage");

    //set glbp packet header
    msg->setGroup(glbpGroup);
    msg->setOwnerId(ie->getMacAddress());

    //set glbp HELLO TLV
    GLBPHello *helloPacket = generateHelloTLV();
    msg->addOption(helloPacket);

    GLBPRequestResponse *reqrespPacket = generateReqRespTLV(0);
    msg->addOption(reqrespPacket);

    return msg;
}

GLBPHello *GLBPVirtualRouter::generateHelloTLV(){
    GLBPHello *packet = new GLBPHello();

    //setup Hello TLV
    packet->setVgState(glbpState);
    packet->setPriority(priority);
    packet->setHelloint(helloTime);
    packet->setHoldint(holdTime);
    packet->setRedirect(redirect);
    packet->setTimeout(timeout);
    //TODO: IPv6
    packet->setAddressType(IPv4);
    packet->setAddress(*virtualIP);

    return packet;
}

GLBPRequestResponse *GLBPVirtualRouter::generateReqRespTLV(int forwarder)
{
    //TODO Pozor! Na 1 IF muze byt forwarder (napr cislo 1) z vice skupin
    GLBPRequestResponse *packet = new GLBPRequestResponse();

    //setup RequestResponse TLV
    packet->setForwarder(forwarder+1);
    packet->setVfState(vfVector[forwarder]->getState());
    packet->setPriority(vfVector[forwarder]->getPriority());
    packet->setWeight(vfVector[forwarder]->getWeight());
    packet->setMacAddress(vfVector[forwarder]->getMacAddress());

    return packet;
}

void GLBPVirtualRouter::scheduleTimer(cMessage *msg)
{
    float jitter = 0;
    if (par("jittered").boolValue()){
        jitter = (rand() % 100)*0.01; //TODO rand is not optimal
    }

    if (msg->isScheduled()){
        cancelEvent(msg);
    }
    if (msg == activetimer)
        scheduleAt(simTime() + holdTime+jitter, activetimer);
    if (msg == standbytimer)
        scheduleAt(simTime() + holdTime+jitter, standbytimer);
    if (msg == hellotimer)
        scheduleAt(simTime() + helloTime+jitter, hellotimer);
}

void GLBPVirtualRouter::setVirtualMAC(int n)
{
    //"00-07-b4-xx-xx-yy"
    //x's are 6bits of 0 followed by 10bits of group ID
    //y's reprezent number of virtual forwarder

    //set first two bits of group ID
    int minus = 0;
    if (glbpGroup < 256){
        virtualMAC = new MACAddress("00-07-b4-00-00-00");
    }
    else if ( glbpGroup < 512 ){ // glbpGroup >= 256 &&
        virtualMAC = new MACAddress("00-07-b4-01-00-00");
        minus = 256;
    }else if ( glbpGroup < 768 ){ // glbpGroup >= 512 &&
        virtualMAC = new MACAddress("00-07-b4-02-00-00");
        minus = 512;
    }else{ //horni bity oba 11
        virtualMAC = new MACAddress("00-07-b4-03-00-00");
        minus = 768;
    }

    //set another 8 bits of gid
    virtualMAC->setAddressByte(4, glbpGroup-minus);

    //set virtual forwarder number
    virtualMAC->setAddressByte(5, n);
    EV_DEBUG<<"routerID:"<<par("deviceId").str()<<"vMAC:"<<virtualMAC->str()<<"\n";
}

bool GLBPVirtualRouter::isHigherPriorityThan(GLBPMessage *GLBPm){
    UDPDataIndication *udpInfo = check_and_cast<UDPDataIndication *>(GLBPm->getControlInfo());
    GLBPHello *hello = check_and_cast<GLBPHello *>(GLBPm->findOptionByType(HELLO,0));

    if (hello->getPriority() < priority){//TODO
        return true;
    }else if (hello->getPriority() > priority){//TODO
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
/**
 * Debug Messages
 */

void GLBPVirtualRouter::DebugStateMachine(int from, int to){
    std::string fromText, toText;
    fromText = intToGlbpState(from);
    toText = intToGlbpState(to);
    EV<<hostname<<" # "<<ie->getName()<<" Grp "<<glbpGroup<<" "<<fromText<<" -> "<<toText<<endl;
}

std::string GLBPVirtualRouter::intToGlbpState(int state){
    std::string retval;
    switch(state){
        case INIT:
            retval = "Init"; break;
        case LISTEN:
            retval = "Listen"; break;
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

void GLBPVirtualRouter::DebugSendMessage(int t){
    std::string type = intToMessageType(t);
    std::string state = intToGlbpState(glbpState);

    EV<<hostname<<" # "<<ie->getName()<<" Grp "<<glbpGroup<<" "<<type<<" out "<< ie->ipv4Data()->getIPAddress().str(false) <<" "<<state << " pri "<<priority<<" vIP "<<virtualIP->str(false)<<endl;
}

void GLBPVirtualRouter::DebugGetMessage(GLBPMessage *msg){
    GLBPHello *hello = check_and_cast<GLBPHello *>(msg->findOptionByType(HELLO, 0));
    std::string type = intToMessageType(HELLO);//TODO
    std::string state = intToGlbpState(hello->getVgState());
    UDPDataIndication *udpInfo = check_and_cast<UDPDataIndication *>(msg->getControlInfo());
    std::string ipFrom =  udpInfo->getSrcAddr().toIPv4().str(false);

    EV<<hostname<<" # "<<ie->getName()<<" Grp "<<(int)msg->getGroup()<<" "<<type<<" in "<< ipFrom <<" "<<state << " pri "<<(int)hello->getPriority()<<" vIP "<<hello->getAddress().str(false)<<endl;
}

void GLBPVirtualRouter::DebugUnknown(int who){
    std::string type = intToGlbpState(who);
    EV_DEBUG<<hostname<<" # "<<ie->getName()<<" "<<glbpGroup<<" "<<type<<" router is unknown"<<endl;//TODO was, <ip stareho>
}

std::string GLBPVirtualRouter::intToMessageType(int msg){
    std::string retval;
    switch(msg){
        case HELLO:
            retval = "Hello"; break;
        case REQRESP:
            retval = "Request/Response"; break;
//        case RESIGN:
//            retval = "Resign"; break;
    }
    return retval;
}

void GLBPVirtualRouter::interfaceDown(){
    switch(glbpState){
        case ACTIVE:
            //TODO SEND RESIGN
        default:
            if (hellotimer->isScheduled())
                cancelEvent(standbytimer);
            if (standbytimer->isScheduled())
                cancelEvent(standbytimer);
            if (hellotimer->isScheduled())
                cancelEvent(standbytimer);
            break;
    }//switch

    DebugStateMachine(glbpState, INIT);
    glbpState = INIT;
//    vf->setDisable();//tODO
}

void GLBPVirtualRouter::interfaceUp(){
    initState();
}


GLBPVirtualRouter::~GLBPVirtualRouter() {
    cancelAndDelete(hellotimer);
    cancelAndDelete(standbytimer);
    cancelAndDelete(activetimer);
    cancelAndDelete(initmessage);
    containingModule->unsubscribe(NF_INTERFACE_STATE_CHANGED, this);
}

} /* namespace inet */
