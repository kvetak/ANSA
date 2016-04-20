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
        glbpUdpPort = GLBP_UDP_PORT;
        glbpMulticast = new L3Address(GLBP_MULTICAST_ADDRESS.c_str());
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
                if (msg == activetimer || msg == standbytimer){
                    DebugStateMachine(LISTEN, SPEAK);
                    glbpState = SPEAK;
                    scheduleTimer(activetimer);
                    scheduleTimer(standbytimer);
                    scheduleTimer(hellotimer);
                }
                break;
            case SPEAK:
                if (msg == standbytimer){//f
                    if (standbytimer->isScheduled())
                        cancelEvent(standbytimer);
                    DebugStateMachine(SPEAK, STANDBY);
                    glbpState = STANDBY;
                }else
                if (msg == activetimer){
                    scheduleTimer(activetimer);
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
                    sendMessage(HELLO);//TODO
                    DebugStateMachine(STANDBY, ACTIVE);
                    glbpState = ACTIVE;
                    vf->setEnable();
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
                break;
        }
    }
    else
    {
        GLBPHello *GLBPm = dynamic_cast<GLBPHello *>(msg);

        DebugGetMessage(GLBPm);

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

void GLBPVirtualRouter::handleMessageListen(GLBPHello *msg){
    //TODO resign receive
    switch(msg->getVgState()){
    case STANDBY://j
        //lower
        if (isHigherPriorityThan(msg)){
            DebugStateMachine(LISTEN,SPEAK);
            glbpState = SPEAK;
            scheduleTimer(standbytimer);
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

void GLBPVirtualRouter::handleMessageSpeak(GLBPHello *msg){
    //TODO RESIGN
    switch(msg->getVgState()){
        case SPEAK:
            //h
            if (!isHigherPriorityThan(msg)){
                scheduleTimer(standbytimer);
                if (hellotimer->isScheduled())
                    cancelEvent(hellotimer);
                DebugStateMachine(SPEAK, LISTEN);
                glbpState = LISTEN;
            }
            break;
        case STANDBY:
            //i
            if (!isHigherPriorityThan(msg)){
                scheduleTimer(standbytimer);
                if (hellotimer->isScheduled())
                    cancelEvent(hellotimer);
                DebugStateMachine(SPEAK, LISTEN);
                glbpState = LISTEN;
            }
            else
            {//j
                if (standbytimer->isScheduled())
                    cancelEvent(standbytimer);
                DebugStateMachine(SPEAK, STANDBY);
                glbpState = STANDBY;
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

void GLBPVirtualRouter::handleMessageStandby(GLBPHello *msg){
    //TODO RESIGN RECEIVE
    switch(msg->getVgState()){
        case SPEAK:
            if (!isHigherPriorityThan(msg)){
                scheduleTimer(standbytimer);
                if (hellotimer->isScheduled())
                    cancelEvent(hellotimer);
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
                    vf->setEnable();
                }
            }
            break;
    }//end switch
}

void GLBPVirtualRouter::handleMessageActive(GLBPHello *msg){
    switch(msg->getVgState()){
        case STANDBY:
            scheduleTimer(standbytimer);
            break;
        case ACTIVE:
            if (!isHigherPriorityThan(msg)){//k
                scheduleTimer(activetimer);
                scheduleTimer(standbytimer);
                DebugStateMachine(ACTIVE, SPEAK);
                vf->setDisable();
                glbpState = SPEAK;
            }
            break;
    }
}

void GLBPVirtualRouter::receiveSignal(cComponent*, int, cObject*){

}

void GLBPVirtualRouter::initState(){
    vf->setDisable();
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
    scheduleTimer(activetimer);
    scheduleTimer(standbytimer);
}

void GLBPVirtualRouter::sendMessage(TYPE type)
{
    DebugSendMessage(type);
    GLBPHello *packet = generateMessage(type);
    packet->setBitLength(GLBP_HELLO_SIZE);

    UDPSocket::SendOptions options;
    options.outInterfaceId = ie->getInterfaceId();
    options.srcAddr = ie->ipv4Data()->getIPAddress();

    socket->setTimeToLive(1);
    socket->sendTo(packet, *glbpMulticast, glbpUdpPort, &options);
}

GLBPHello *GLBPVirtualRouter::generateMessage(TYPE type)
{ //todo udelat obecne ne jen pro hello
    GLBPHello* msg = new GLBPHello("GLBPHello");
    msg->setGroup(glbpGroup);
    //todo owner id
    msg->setType(type);
    msg->setLength(GLBP_HELLO_SIZE);
    msg->setVgState(glbpState);
    msg->setPriority(priority);
    msg->setHelloint(par("hellotime"));
    msg->setHoldint(par("holdtime"));
    msg->setRedirect(600);//TODO
    msg->setTimeout(700);//TODO
    if (virtualIP != nullptr)
        msg->setAddress(*(virtualIP));

    return msg;
}

void GLBPVirtualRouter::scheduleTimer(cMessage *msg)
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

bool GLBPVirtualRouter::isHigherPriorityThan(GLBPHello *GLBPm){
    UDPDataIndication *udpInfo = check_and_cast<UDPDataIndication *>(GLBPm->getControlInfo());
    if (GLBPm->getPriority() < priority){
        return true;
    }else if (GLBPm->getPriority() > priority){
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

void GLBPVirtualRouter::DebugGetMessage(GLBPHello *msg){
    std::string type = intToMessageType(msg->getType());
    std::string state = intToGlbpState(msg->getVgState());
    UDPDataIndication *udpInfo = check_and_cast<UDPDataIndication *>(msg->getControlInfo());
    std::string ipFrom =  udpInfo->getSrcAddr().toIPv4().str(false);

    EV<<hostname<<" # "<<ie->getName()<<" Grp "<<(int)msg->getGroup()<<" "<<type<<" in "<< ipFrom <<" "<<state << " pri "<<(int)msg->getPriority()<<" vIP "<<msg->getAddress().str(false)<<endl;
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

GLBPVirtualRouter::~GLBPVirtualRouter() {
    cancelAndDelete(hellotimer);
    cancelAndDelete(standbytimer);
    cancelAndDelete(activetimer);
    cancelAndDelete(initmessage);
//    containingModule->unsubscribe(NF_INTERFACE_STATE_CHANGED, this);
}

} /* namespace inet */
