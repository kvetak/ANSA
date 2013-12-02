// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
 * @file TRILL.cc
 * @author Marcel Marek (mailto:xscrew02@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 7.3.2013
 * @brief Base class for TRILL module.
 * @detail Base class for TRILL module. Performs TRILL de/encapsulation.
 * @todo Z9
 */

#include "TRILL.h"
#include "ISIS.h"


Define_Module(TRILL);
TRILL::TRILL(){

}

TRILL::~TRILL(){

}

MACAddress TRILL::getBridgeAddress() {
    Enter_Method_Silent();
    return bridgeAddress;
}

/**
 *  Initialize TRILL module. Sets pointers to MAC and VLAN Table as well as IS-IS module
 * @param stage is configuration stage
 */
void TRILL::initialize(int stage) {

    if (stage == 0) {
        portCount = gateSize("lowerLayerOut");

        /* because ASSERT in handleIncomingFrame() */
        int i = 0;
        char msgname[16];
        sprintf(msgname, "switchMsg %d", i);
        currentMsg = new cMessage(msgname, i);

        cModule * tmp_rBMACTable = getParentModule()->getSubmodule("rBMACTable");
        rbMACTable = check_and_cast<RBMACTable *>(tmp_rBMACTable);

        cModule * tmp_rBVLANTable = getParentModule()->getSubmodule("rBVLANTable");
        vlanTable = check_and_cast<RBVLANTable *>(tmp_rBVLANTable);

        ift = InterfaceTableAccess().get();


//        cModule * tmp_stp = getParentModule()->getSubmodule("stp");
//        spanningTree = check_and_cast<Stp *>(tmp_stp);

        cModule * tmp_isis = getParentModule()->getSubmodule("isis");
        isis = check_and_cast<ISIS *>(tmp_isis);

        clnsTable = CLNSTableAccess().get();

        bridgeGroupAddress = MACAddress("01-80-C2-00-00-00");
        //TODO two modes, MAC per rBridge vs. MAC per interface
        bridgeAddress = MACAddress::generateAutoAddress();

        WATCH(bridgeAddress);
    }
    else if(stage == 1){
        /* Only default init for now */
        for(int i = 0; i < ift->getNumInterfaces(); i++){
            InterfaceEntry *ie = ift->getInterface(i);
            TRILLInterfaceData *d = new TRILLInterfaceData();
            d->setDefaults();
            ie->setTRILLInterfaceData(d);

        }
    }
}

/**
 * Handles incoming messages.
 * @param msg is incoming message
 */
void TRILL::handleMessage(cMessage *msg) {

    if (!msg->isSelfMessage()) {
        tFrameDescriptor frameDesc;
        const char * inGate = msg->getArrivalGate()->getName();

        if (strcmp(msg->getArrivalGate()->getName(), "stpIn") == 0) {
            /* Messages from STP process */
            dispatchBPDU(msg, msg->getArrivalGate()->getIndex());
            return;
        }else if (strcmp(msg->getArrivalGate()->getName(), "isisIn") == 0){
            /* Messages from ISIS module */
//            reception(frameDesc, (ISISMessage*) msg);//TODO C1 handle returned value
            dispatchTRILLControl((ISISMessage*) msg);
            return;

        } else if (strcmp(msg->getArrivalGate()->getName(), "lowerLayerIn") == 0) {
            /* Messages from network */
            bool processResult;
            if(reception(frameDesc, msg)){


                //frame received on port's allowed VLAN and successfully parsed
                //now determine the proper type
                //getTRILL_Type
                Ieee802Ctrl *ctrl;
                TRILL::FrameCategory cat;
                switch(cat = classify(frameDesc)){
                    case TRILL::TRILL_L2_CONTROL:
                        //process L2 control frame like register VLAN etc
                        //TODO B1
                        break;

                    case TRILL::TRILL_NATIVE:
                        //process non-TRILL-encapsulated
                        //processNative(frameDesc);
                        //TBD
                        processResult = processNative(frameDesc);

                        break;

                    case TRILL::TRILL_DATA:
                        //processTRILL encapsulated
                        EV << "TRILL-encapsulated data received" << std::endl;
                        processResult = processTRILLData(frameDesc);
                        break;

                    case TRILL::TRILL_CONTROL:
                        //TODO B1
//                        EV <<"Error: L2_ISIS frame shoudn't get send to TRILL (for NOW)" <<endl;
//                        send(msg->)
//                        ctrl = (Ieee802Ctrl*)msg->getControlInfo();
//                        ctrl->dup();
                        frameDesc.payload->setControlInfo(frameDesc.ctrl->dup());
//                        framed
                        send(frameDesc.payload, "isisOut", frameDesc.rPort);
                        break;
                    case TRILL::TRILL_OTHER:
                        EV <<"Warning: TRILL: Received TRILL::OTHER frame so discarding" <<endl;
                        break;
                    case TRILL::TRILL_NONE:
                        EV <<"ERROR: TRILL: Misclassified frame" <<endl;
                        break;
                }

                if(processResult){


                }else{
                    delete msg;
                    return;
                }
                delete frameDesc.payload;

            }

//            if (reception(frame, msg) == true) {
//                //EV << frame;
//                //error("BLE BLE");
//                relay(frameDesc);
//            }
//            delete frame.payload;
        }
    } else {
        // Self message signal used to indicate a frame has finished processing
        processFrame(msg);
    }
    delete msg;
}

//bool TRILL::reception(TRILL::tFrameDescriptor& frame, ISISMessage *isisMsg){
//    int rPort = isisMsg->getArrivalGate()->getIndex();
//
//    return true;
//}
/**
 * Fills frame's descriptor with information in @param msg
 * @param frame is frame's descriptor structure
 * @param msg incomming message to be parsed
 */
bool TRILL::reception(TRILL::tFrameDescriptor& frame, cMessage *msg) {
    /* ACTIVE TOPOLOGY ENFORCEMENT AFTER CLASSIFICATION AND INGRESS
     * because of learning state operation, MAC record can be learned
     * after classification and ingress, if learning is enabled on
     * that port.
     */
  //CLASSIFICATION & INGRESS CALL
    int rPort = msg->getArrivalGate()->getIndex();

    cMessage * tmp = msg;
    ASSERT(tmp);

    // Classify frame, and unpack to frame descriptor
    if (dynamic_cast<AnsaEtherFrame *>(tmp)){
        AnsaEtherFrame * taggedFrame = (AnsaEtherFrame *) tmp;
        if (ingress(frame, taggedFrame, rPort) == false) {
            return false;
        }
    } else if (dynamic_cast<EthernetIIFrame *>(tmp)) {
        EthernetIIFrame * untaggedFrame = (EthernetIIFrame *) tmp;
        ingress(frame, untaggedFrame, rPort); // discarding forbidden PortVID is in relay
    }else{
        return false;
        EV << "Warning: dropping unsupported frame in TRILL::reception";
    }

    return true;
}
//
//void TRILL::relay(TRILL::tFrameDescriptor& frame) {
//
//    // BPDU Handling
//    if (frame.dest == bridgeGroupAddress) {
//        deliverBPDU(frame);
//        return;
//    }
//
//     /* Dropping forbidden PortVID */
//    if (frame.VID == 0) {
//        return;
//    }
//
//    // BROADCAST ??
//    if (frame.dest.isBroadcast()) {
//        frame.portList = vlanTable->getPorts(frame.VID);
//        if (frame.portList.size() == 0) {
//            return;
//        }
//
//    } else {
//        RBVLANTable::tVIDPort tmpPort;
//        tmpPort.action = RBVLANTable::REMOVE;
//
//        RBMACTable::tPortList tmpPortList = rbMACTable->getPorts(frame.dest);
//
//        if (tmpPortList.size() == 0) { // not known -> bcast
//            frame.portList = vlanTable->getPorts(frame.VID);
//        } else {
//            for (unsigned int i = 0; i < tmpPortList.size(); i++) {
//                tmpPort.port = tmpPortList.at(i);
//                frame.portList.push_back(tmpPort);
//            }
//        }
//    }
//
//    //EV << frame;
//    //error("BLE BLE");
//
//
//    // LEARNING (explained in reception())
//    learn(frame);
//    // ACTIVE TOPOLOGY ENFORCEMENT (explained in reception())
////    if (spanningTree->forwarding(frame.rPort, frame.VID) == true) {
//        // EGRESS
//        egress(frame);
//        // SEND
//        dispatch(frame);
////    }
//}

bool TRILL::ingress(TRILL::tFrameDescriptor& tmp, EthernetIIFrame *frame, int rPort) {
    // Info from recepted frame
    tmp.ctrl = (Ieee802Ctrl *) frame->getControlInfo();
    if (tmp.ctrl == NULL)
    {
        tmp.ctrl = new Ieee802Ctrl();
        tmp.ctrl->setSrc(frame->getSrc());
        tmp.ctrl->setDest(frame->getDest());
        tmp.ctrl->setEtherType(frame->getEtherType());
    }
    tmp.payload = frame->decapsulate();
    tmp.name.insert(0, frame->getName());
    tmp.rPort = rPort;
    tmp.src = frame->getSrc();
    tmp.dest = frame->getDest();
    tmp.etherType = frame->getEtherType();

    // VLAN Assign
    tmp.VID = vlanTable->getVID(rPort);
    tmp.d = this->ift->getInterfaceByNetworkLayerGateIndex(rPort)->trillData();

    if (tmp.VID == 0) {
        return false;
    }


    return true;
}

bool TRILL::ingress(TRILL::tFrameDescriptor& tmp, AnsaEtherFrame *frame, int rPort) {
    // Info from recepted frame
    tmp.ctrl = (Ieee802Ctrl *)frame->getControlInfo();
    if (tmp.ctrl == NULL)
    {
        tmp.ctrl = new Ieee802Ctrl();
        tmp.ctrl->setSrc(frame->getSrc());
        tmp.ctrl->setDest(frame->getDest());
        tmp.ctrl->setEtherType(frame->getEtherType());
    }
    tmp.payload = frame->decapsulate();
    tmp.name.insert(0, frame->getName());
    tmp.rPort = rPort;
    tmp.VID = frame->getVlan();
    tmp.src = frame->getSrc();
    tmp.dest = frame->getDest();
    tmp.etherType = frame->getEtherType();
    tmp.d = this->ift->getInterfaceByNetworkLayerGateIndex(rPort)->trillData();





    // VLAN Allowed
    if (vlanTable->isAllowed(frame->getVlan(), rPort) == false) {
        return false;
    }

    return true;
}

void TRILL::egress(TRILL::tFrameDescriptor& frame) {
  // MINIMIZE OUT PORTS
  // SET TAG ACTIONS

    RBVLANTable::tVIDPortList tmp = frame.portList;
    frame.portList.clear();

    for (unsigned int i = 0; i < tmp.size(); i++) {
        if (vlanTable->isAllowed(frame.VID, tmp.at(i).port) == true && tmp.at(i).port != frame.rPort) {
            frame.portList.push_back(tmp.at(i));
        }
    }

}
/**
 * Returns false when there is no port to send it to.
 */
bool TRILL::egressNativeLocal(TRILL::tFrameDescriptor& frameDesc){
    /*
     * Probably rename it to egressNativeLocal or move switch from processNative here, because
     * we might receive native frame on port "1" and send it through that same port but
     * TRILL-encapsulated. It we handle it the same way as NativeLocalDispatch we would
     * remove such port because of the "don't send it on received port" rule. The same
     * thing might apply in the opposite direction when we receive TRILL-encapsulated frame
     * and as App Fwd send it on the same interface but in native form.
     * In conclusion this rule seems to apply only in case of receiving native frame from local
     * port and forwarding it again in native form.
     */

    //minimize out ports
    //set tag actions


    RBVLANTable::tVIDPortList tmp = frameDesc.portList;
    frameDesc.portList.clear();

    for (unsigned int i = 0; i < tmp.size(); i++) {
        TRILLInterfaceData *d = this->ift->getInterfaceByNetworkLayerGateIndex(tmp.at(i).port)->trillData();
        if (d->isEnabled(frameDesc.VID) && tmp.at(i).port != frameDesc.rPort && d->isAppointedForwarder(frameDesc.VID, this->isis->getNickname())) {
            frameDesc.portList.push_back(tmp.at(i));
        }
    }

    return !frameDesc.portList.empty();


}

bool TRILL::egressNativeMulticastRemote(TRILL::tFrameDescriptor &frameDesc){
    //determine distribution tree (this->isis->getNickname)

    //get every path with "from" or "to" this RBridge System-ID
    //actually when I'm the root it should be enough just to get all path with matching "from"
    std::vector<unsigned char *> *systemIDs = this->isis->getSystemIDsFromTreeOnlySource(this->isis->getNickname(), this->isis->getSysId());



    //for each systemID determine output interface
    //since we are the source.RBridge we don't have to deal with any multi-dest check

    for(std::vector<unsigned char *>::iterator it = systemIDs->begin(); it != systemIDs->end(); ++it){
        RBVLANTable::tVIDPort portAction;
        ISISadj * adj = this->isis->getAdjBySystemID((*it), L1_TYPE);
        if(adj == NULL){
            EV << "TRILL: Error in egressNativeMulticastRemote: can't find adjacency for systemId"<< (*it) <<std::endl;
            continue;
        }
        portAction.port = adj->gateIndex;
//        portAction.action = this->vlanTable->getTag()//vlanId will be determined in dispatch (designated Vlan)
        frameDesc.portList.push_back(portAction);
    }

    return !frameDesc.portList.empty();
}

bool TRILL::egressTRILLDataMultiDestRemote(TRILL::tFrameDescriptor &frameDesc){

    TRILLFrame *trillFrame = static_cast<TRILLFrame *>(frameDesc.payload);

    std::vector<unsigned char *> *systemIDs = this->isis->getSystemIDsFromTreeOnlySource(trillFrame->getEgressRBNickname(), this->isis->getSysId());


    for(std::vector<unsigned char *>::iterator it = systemIDs->begin(); it != systemIDs->end(); ++it){
        RBVLANTable::tVIDPort portAction;
        ISISadj * adj = this->isis->getAdjBySystemID((*it), L1_TYPE);
        if(adj == NULL){
            EV << "TRILL: Error in egressNativeMulticastRemote: can't find adjacency for systemId"<< (*it) <<std::endl;
            continue;
        }

        //don't send it on received interface
        if(frameDesc.rPort == adj->gateIndex){
            continue;
        }
        portAction.port = adj->gateIndex;
//        portAction.action = this->vlanTable->getTag()//vlanId will be determined in dispatch (designated Vlan)
        frameDesc.portList.push_back(portAction);
    }

    return !frameDesc.portList.empty();

}

bool TRILL::egressTRILLDataMultiDestNative(TRILL::tFrameDescriptor &innerFrameDesc){
    innerFrameDesc.portList = vlanTable->getPorts(innerFrameDesc.VID);

    for(RBVLANTable::tVIDPortList::iterator it = innerFrameDesc.portList.begin(); it != innerFrameDesc.portList.end();){
        TRILLInterfaceData *d = ift->getInterfaceByNetworkLayerGateIndex((*it).port)->trillData();
        if(!d->isAppointedForwarder(innerFrameDesc.VID, this->isis->getNickname())){
            it = innerFrameDesc.portList.erase(it);
        }else{
            ++it;
        }
    }

}

void TRILL::dispatch(TRILL::tFrameDescriptor& frame) {

    EthernetIIFrame * untaggedFrame = new EthernetIIFrame(frame.name.c_str());
    AnsaEtherFrame * taggedFrame = new AnsaEtherFrame(frame.name.c_str());

    taggedFrame->setKind(frame.payload->getKind());
    taggedFrame->setSrc(frame.src);
    taggedFrame->setDest(frame.dest);
    taggedFrame->setByteLength(ETHER_MAC_FRAME_BYTES);
    taggedFrame->setVlan(frame.VID);
    taggedFrame->setEtherType(frame.etherType);

    taggedFrame->encapsulate(frame.payload->dup());
    if (taggedFrame->getByteLength() < MIN_ETHERNET_FRAME_BYTES) {
        taggedFrame->setByteLength(MIN_ETHERNET_FRAME_BYTES); // "padding"
    }

    untaggedFrame->setKind(frame.payload->getKind());
    untaggedFrame->setSrc(frame.src);
    untaggedFrame->setDest(frame.dest);
    untaggedFrame->setByteLength(ETHER_MAC_FRAME_BYTES);
    untaggedFrame->setEtherType(frame.etherType);

    untaggedFrame->encapsulate(frame.payload->dup());
    if (untaggedFrame->getByteLength() < MIN_ETHERNET_FRAME_BYTES) {
        untaggedFrame->setByteLength(MIN_ETHERNET_FRAME_BYTES); // "padding"
    }
    RBVLANTable::tVIDPortList::iterator it;
    for (it = frame.portList.begin(); it != frame.portList.end(); it++) {
        if (it->port >= portCount) {
            continue;
        }
//        if (spanningTree->forwarding(it->port, frame.VID) == false) {
//            continue;
//        }
        if (it->action == RBVLANTable::INCLUDE) {
            send(taggedFrame->dup(), "lowerLayerOut", it->port);
        } else {
            send(untaggedFrame->dup(), "lowerLayerOut", it->port);
        }
    }

    delete taggedFrame;
    delete untaggedFrame;
    return;
}
//bool TRILL::dispatchTRILLHello(TRILLHelloPacket* trillHello){
//
//
//
//    return true;
//}
/*
 * This method is for passing messages from IS-IS module to lowerLayer module
 * with encapsulating it in Ethernet frame.
 * @param isisMsg is received message from ISIS module. It may be Hello, LSP, CSNP, ..
 */
bool TRILL::dispatchTRILLControl(ISISMessage* isisMsg){
    EthernetIIFrame* untaggedFrame = new EthernetIIFrame(isisMsg->getName());
    AnsaEtherFrame * taggedFrame = new AnsaEtherFrame(isisMsg->getName());

    taggedFrame->setKind(isisMsg->getKind());
//     taggedFrame->setSrc(frame.src); will be set by underlying module
//     taggedFrame->setDest(frame.dest);
    taggedFrame->setByteLength(ETHER_MAC_FRAME_BYTES);
    taggedFrame->setVlan(TRILL_DEFAULT_VLAN); //TODO A! VLAN_ID should get dynamically. Also more than one Hello could be send on interface.
    taggedFrame->setEtherType(ETHERTYPE_L2_ISIS);

    taggedFrame->encapsulate(isisMsg->dup());
    if (taggedFrame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
    {
        taggedFrame->setByteLength(MIN_ETHERNET_FRAME_BYTES); // "padding"
    }

    untaggedFrame->setKind(isisMsg->getKind());
//     untaggedFrame->setSrc(frame.src);
//     untaggedFrame->setDest(frame.dest);
    untaggedFrame->setByteLength(ETHER_MAC_FRAME_BYTES);
    untaggedFrame->setEtherType(ETHERTYPE_L2_ISIS);

    untaggedFrame->encapsulate(isisMsg->dup());
    if (untaggedFrame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
    {
        untaggedFrame->setByteLength(MIN_ETHERNET_FRAME_BYTES); // "padding"
    }

    MACAddress ma;
    switch (isisMsg->getType())
        {

        case TRILL_HELLO:
        case L1_LSP:
        case L1_CSNP:
        case L1_PSNP:
        case MTU_PROBE_PDU:
        case MTU_ACK_PDU:
            ma.setAddress(ALL_IS_IS_RBRIDGES);

        }
    taggedFrame->setDest(ma);
    untaggedFrame->setDest(ma);

    Ieee802Ctrl *ctrl = new Ieee802Ctrl();
    ctrl->setEtherType(ETHERTYPE_L2_ISIS);
    ctrl->setDsap(SAP_CLNS);
    ctrl->setSsap(SAP_CLNS);
    ctrl->setDest(MACAddress(ALL_IS_IS_RBRIDGES)); //ALL-IS-IS-RBridges
//

    taggedFrame->setControlInfo(ctrl->dup());
    untaggedFrame->setControlInfo(ctrl->dup());

    RBVLANTable::tVIDPortList::iterator it;
//         for (it = frame.portList.begin(); it != frame.portList.end(); it++) {
//             if (it->port >= portCount) {//TODO A! this needs to go to egress// do not send on received port
//                 continue;
//             }
    //        if (spanningTree->forwarding(it->port, frame.VID) == false) {
    //            continue;
    //        }

    //TODO A1 This below is a mess. Get VLAN ID and port/gate index properly.

    int port = isisMsg->getArrivalGate()->getIndex();

    RBVLANTable::tTagAction action = this->vlanTable->getTag(TRILL_DEFAULT_VLAN, port);

    if (action == RBVLANTable::INCLUDE)
    {
        send(taggedFrame, "lowerLayerOut", port);
    }
    else
    {
        send(untaggedFrame->dup(), "lowerLayerOut", port);
    }
//         }

//    delete taggedFrame;
    delete untaggedFrame;
    delete ctrl;
    return true;


}

/* NEW */
bool TRILL::dispatchNativeLocalPort(TRILL::tFrameDescriptor& frame) {

    EthernetIIFrame * untaggedFrame = new EthernetIIFrame(frame.name.c_str());
    AnsaEtherFrame * taggedFrame = new AnsaEtherFrame(frame.name.c_str());

    taggedFrame->setKind(frame.payload->getKind());
    taggedFrame->setSrc(frame.src);
    taggedFrame->setDest(frame.dest);
    taggedFrame->setByteLength(ETHER_MAC_FRAME_BYTES);
    taggedFrame->setVlan(frame.VID);
    taggedFrame->setEtherType(frame.etherType);

    taggedFrame->encapsulate(frame.payload->dup());
    if (taggedFrame->getByteLength() < MIN_ETHERNET_FRAME_BYTES) {
        taggedFrame->setByteLength(MIN_ETHERNET_FRAME_BYTES); // "padding"
    }

    untaggedFrame->setKind(frame.payload->getKind());
    untaggedFrame->setSrc(frame.src);
    untaggedFrame->setDest(frame.dest);
    untaggedFrame->setByteLength(ETHER_MAC_FRAME_BYTES);
    untaggedFrame->setEtherType(frame.etherType);

    untaggedFrame->encapsulate(frame.payload->dup());
    if (untaggedFrame->getByteLength() < MIN_ETHERNET_FRAME_BYTES) {
        untaggedFrame->setByteLength(MIN_ETHERNET_FRAME_BYTES); // "padding"
    }
    RBVLANTable::tVIDPortList::iterator it;
    for (it = frame.portList.begin(); it != frame.portList.end(); it++) {
        if (it->port >= portCount) {//TODO A! this needs to go to egress// do not send on received port
            continue;
        }
//        if (spanningTree->forwarding(it->port, frame.VID) == false) {
//            continue;
//        }
        if (it->action == RBVLANTable::INCLUDE) {
            send(taggedFrame->dup(), "lowerLayerOut", it->port);
        } else {
            send(untaggedFrame->dup(), "lowerLayerOut", it->port);
        }
    }

    delete taggedFrame;
    delete untaggedFrame;
    return true;
}

bool TRILL::dispatchNativeRemote(tFrameDescriptor &frameDesc){

    AnsaEtherFrame * innerFrame = new AnsaEtherFrame(frameDesc.name.c_str());

    innerFrame->setKind(frameDesc.payload->getKind());
    innerFrame->setSrc(frameDesc.src);
    innerFrame->setDest(frameDesc.dest);
    innerFrame->setByteLength(ETHER_MAC_FRAME_BYTES);
    innerFrame->setVlan(frameDesc.VID);
    innerFrame->setEtherType(frameDesc.etherType);
    //TODO A! comment the section below or not?
    innerFrame->encapsulate(frameDesc.payload->dup());
    if (innerFrame->getByteLength() < MIN_ETHERNET_FRAME_BYTES) {
        innerFrame->setByteLength(MIN_ETHERNET_FRAME_BYTES); // "padding"
    }


    TRILLFrame *trillFrame = new TRILLFrame(frameDesc.name.c_str());
    trillFrame->setEthertype(ETHERTYPE_TRILL);
    trillFrame->setVersion(0);
    trillFrame->setReserved(0);
    trillFrame->setMultiDest(false);
    trillFrame->setOpLength(0);
    trillFrame->setHopCount(3);// TODO A1 fix
    trillFrame->setEgressRBNickname(frameDesc.record.ingressNickname); //yes ingressNickname is correct
    trillFrame->setIngressRBNickname(this->isis->getNickname());
    //TODO A1 set options
    trillFrame->encapsulate(innerFrame->dup());


    /* TODO A0
     * because we don't have nickname ->systemID mapping, we have to go the hard way
     * ALSO move it to egressNativeRemote
     */

    unsigned char *tmpSysId = new unsigned char[ISIS_SYSTEM_ID];
    memcpy(tmpSysId, this->isis->sysId, ISIS_SYSTEM_ID);
    tmpSysId[ISIS_SYSTEM_ID - 2] = frameDesc.record.ingressNickname >> 8;
    tmpSysId[ISIS_SYSTEM_ID - 1] = frameDesc.record.ingressNickname & 0xFF;

    RBVLANTable::tVIDPort egressPort;
    unsigned char *nextHopSysId;
    nextHopSysId = this->clnsTable->getNextHopSystemIDBySystemID(tmpSysId);
    if (nextHopSysId == NULL){
        return false;
    }
    egressPort.port = clnsTable->getGateIndexBySystemID(tmpSysId);
    frameDesc.portList.push_back(egressPort);
    ISISadj *adj = this->isis->getAdjBySystemID(nextHopSysId, L1_TYPE, egressPort.port);
    if(egressPort.port < 0 || adj == NULL){
        EV <<"dispatchNativeRemote: NOOOOOOOOOOOOOOOOOOOOOOOOOOO!";
        return false;
    }

    /*
     * End of horribly wrong solution
     */

    EthernetIIFrame * untaggedFrame = new EthernetIIFrame(frameDesc.name.c_str());
    AnsaEtherFrame * taggedFrame = new AnsaEtherFrame(frameDesc.name.c_str());

    taggedFrame->setKind(frameDesc.payload->getKind());
//    taggedFrame->setSrc(frameDesc.src); ////will be set by underlying MAC module
    taggedFrame->setDest(adj->mac);//TODO A! this should be MAC of the nextHop
    taggedFrame->setByteLength(ETHER_MAC_FRAME_BYTES);
//    taggedFrame->setVlan(frameDesc.VID);//vlanId is set below during send
    taggedFrame->setEtherType(ETHERTYPE_TRILL);

    taggedFrame->encapsulate(trillFrame->dup());
    if (taggedFrame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
    {
        taggedFrame->setByteLength(MIN_ETHERNET_FRAME_BYTES); // "padding"
    }

    untaggedFrame->setKind(frameDesc.payload->getKind());
//    untaggedFrame->setSrc(frameDesc.src);//will be set by underlying MAC module
    untaggedFrame->setDest(adj->mac);
    untaggedFrame->setByteLength(ETHER_MAC_FRAME_BYTES);
    untaggedFrame->setEtherType(ETHERTYPE_TRILL);

    untaggedFrame->encapsulate(trillFrame->dup());
    if (untaggedFrame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
    {
        untaggedFrame->setByteLength(MIN_ETHERNET_FRAME_BYTES); // "padding"
    }


    RBVLANTable::tVIDPortList::iterator it;
    for (it = frameDesc.portList.begin(); it != frameDesc.portList.end(); it++) {
        if (it->port >= portCount) {
            continue;
        }
//        if (spanningTree->forwarding(it->port, frameDesc.VID) == false) {
//            continue;
//        }


        TRILLInterfaceData *d = this->ift->getInterfaceByNetworkLayerGateIndex(it->port)->trillData();
        it->action = vlanTable->getTag(d->getDesigVlan(), it->port);
        if (it->action == RBVLANTable::INCLUDE) {

            taggedFrame->setVlan(d->getDesigVlan());
            send(taggedFrame->dup(), "lowerLayerOut", it->port);
        } else {
            send(untaggedFrame->dup(), "lowerLayerOut", it->port);
        }
    }

    delete innerFrame;
    delete trillFrame;
    delete untaggedFrame;
    delete taggedFrame;
    return true;
}

bool TRILL::dispatchTRILLDataUnicastRemote(tFrameDescriptor &frameDesc){

    TRILLFrame *trillFrame = static_cast<TRILLFrame *>(frameDesc.payload);


    /* TODO A0
     * because we don't have nickname ->systemID mapping, we have to go the hard way
     * ALSO move it to egressNativeRemote
     */

    unsigned char *tmpSysId = new unsigned char[ISIS_SYSTEM_ID];
    memcpy(tmpSysId, this->isis->sysId, ISIS_SYSTEM_ID);
    tmpSysId[ISIS_SYSTEM_ID - 2] = trillFrame->getEgressRBNickname() >> 8;
    tmpSysId[ISIS_SYSTEM_ID - 1] = trillFrame->getEgressRBNickname() & 0xFF;

    RBVLANTable::tVIDPort egressPort;
    unsigned char *nextHopSysId;
    nextHopSysId = this->clnsTable->getNextHopSystemIDBySystemID(tmpSysId);
    egressPort.port = clnsTable->getGateIndexBySystemID(tmpSysId);
    frameDesc.portList.push_back(egressPort);
    ISISadj *adj = this->isis->getAdjBySystemID(nextHopSysId, L1_TYPE, egressPort.port);
    if(egressPort.port < 0 || adj == NULL){
        EV <<"Error: dispatchTRILLDataUnicastRemote: NOOOOOOOOOOOOOOOOOOOOOOOOOOO!";
        return false;
    }

    /*
     * End of horribly wrong solution
     */

    EthernetIIFrame * untaggedFrame = new EthernetIIFrame(frameDesc.name.c_str());
    AnsaEtherFrame * taggedFrame = new AnsaEtherFrame(frameDesc.name.c_str());

    taggedFrame->setKind(frameDesc.payload->getKind());
//    taggedFrame->setSrc(frameDesc.src); ////will be set by underlying MAC module
    taggedFrame->setDest(adj->mac);//TODO A! this should be MAC of the nextHop
    taggedFrame->setByteLength(ETHER_MAC_FRAME_BYTES);
//    taggedFrame->setVlan(frameDesc.VID);//vlanId is set below during send
    taggedFrame->setEtherType(ETHERTYPE_TRILL);

    taggedFrame->encapsulate(trillFrame->dup());
    if (taggedFrame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
    {
        taggedFrame->setByteLength(MIN_ETHERNET_FRAME_BYTES); // "padding"
    }

    untaggedFrame->setKind(frameDesc.payload->getKind());
//    untaggedFrame->setSrc(frameDesc.src);//will be set by underlying MAC module
    untaggedFrame->setDest(MACAddress(adj->mac));
    untaggedFrame->setByteLength(ETHER_MAC_FRAME_BYTES);
    untaggedFrame->setEtherType(ETHERTYPE_TRILL);

    untaggedFrame->encapsulate(trillFrame->dup());
    if (untaggedFrame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
    {
        untaggedFrame->setByteLength(MIN_ETHERNET_FRAME_BYTES); // "padding"
    }


    RBVLANTable::tVIDPortList::iterator it;
    for (it = frameDesc.portList.begin(); it != frameDesc.portList.end(); it++) {
        if (it->port >= portCount) {
            continue;
        }
//        if (spanningTree->forwarding(it->port, frameDesc.VID) == false) {
//            continue;
//        }


        TRILLInterfaceData *d = this->ift->getInterfaceByNetworkLayerGateIndex(it->port)->trillData();
        it->action = vlanTable->getTag(d->getDesigVlan(), it->port);
        if (it->action == RBVLANTable::INCLUDE) {

            taggedFrame->setVlan(d->getDesigVlan());
            send(taggedFrame->dup(), "lowerLayerOut", it->port);
        } else {
            send(untaggedFrame->dup(), "lowerLayerOut", it->port);
        }
    }


//    delete trillFrame;
    delete untaggedFrame;
    delete taggedFrame;
    return true;
}



bool TRILL::dispatchNativeMultiDestRemote(tFrameDescriptor &frameDesc){

//    EthernetIIFrame * untaggedFrame = new EthernetIIFrame(frameDesc.name.c_str());
    AnsaEtherFrame * innerFrame = new AnsaEtherFrame(frameDesc.name.c_str());

    innerFrame->setKind(frameDesc.payload->getKind());
    innerFrame->setSrc(frameDesc.src);
    innerFrame->setDest(frameDesc.dest);
    innerFrame->setByteLength(ETHER_MAC_FRAME_BYTES);
    innerFrame->setVlan(frameDesc.VID);
    innerFrame->setEtherType(frameDesc.etherType);
    //TODO A! comment the section below or not?
    innerFrame->encapsulate(frameDesc.payload->dup());
    if (innerFrame->getByteLength() < MIN_ETHERNET_FRAME_BYTES) {
        innerFrame->setByteLength(MIN_ETHERNET_FRAME_BYTES); // "padding"
    }


    TRILLFrame *trillFrame = new TRILLFrame(frameDesc.name.c_str());
    trillFrame->setEthertype(frameDesc.etherType);
    trillFrame->setVersion(0);
    trillFrame->setReserved(0);
    trillFrame->setMultiDest(true);
    trillFrame->setOpLength(0);
    trillFrame->setHopCount(3);// TODO A1 fix
    trillFrame->setEgressRBNickname(this->isis->getNickname()); //TODO A1 check/change
    trillFrame->setIngressRBNickname(this->isis->getNickname());
    //TODO A1 set options
    trillFrame->encapsulate(innerFrame->dup());


    EthernetIIFrame * untaggedFrame = new EthernetIIFrame(frameDesc.name.c_str());
    AnsaEtherFrame * taggedFrame = new AnsaEtherFrame(frameDesc.name.c_str());

    taggedFrame->setKind(frameDesc.payload->getKind());
//    taggedFrame->setSrc(frameDesc.src); ////will be set by underlying MAC module
    taggedFrame->setDest(MACAddress(ALL_RBRIDGES));
    taggedFrame->setByteLength(ETHER_MAC_FRAME_BYTES);
//    taggedFrame->setVlan(frameDesc.VID);//vlanId is set below during send
    taggedFrame->setEtherType(ETHERTYPE_TRILL);

    taggedFrame->encapsulate(trillFrame->dup());
    if (taggedFrame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
    {
        taggedFrame->setByteLength(MIN_ETHERNET_FRAME_BYTES); // "padding"
    }

    untaggedFrame->setKind(frameDesc.payload->getKind());
//    untaggedFrame->setSrc(frameDesc.src);//will be set by underlying MAC module
    untaggedFrame->setDest(MACAddress(ALL_RBRIDGES));
    untaggedFrame->setByteLength(ETHER_MAC_FRAME_BYTES);
    untaggedFrame->setEtherType(ETHERTYPE_TRILL);

    untaggedFrame->encapsulate(trillFrame->dup());
    if (untaggedFrame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
    {
        untaggedFrame->setByteLength(MIN_ETHERNET_FRAME_BYTES); // "padding"
    }


    RBVLANTable::tVIDPortList::iterator it;
    for (it = frameDesc.portList.begin(); it != frameDesc.portList.end(); it++) {
        if (it->port >= portCount) {
            continue;
        }
//        if (spanningTree->forwarding(it->port, frameDesc.VID) == false) {
//            continue;
//        }


        TRILLInterfaceData *d = this->ift->getInterfaceByNetworkLayerGateIndex(it->port)->trillData();
        it->action = vlanTable->getTag(d->getDesigVlan(), it->port);
        if (it->action == RBVLANTable::INCLUDE) {

            taggedFrame->setVlan(d->getDesigVlan());
            send(taggedFrame->dup(), "lowerLayerOut", it->port);
        } else {
            send(untaggedFrame->dup(), "lowerLayerOut", it->port);
        }
    }

    delete innerFrame;
    delete trillFrame;
    delete untaggedFrame;
    delete taggedFrame;
    return true;
}

bool TRILL::dispatchTRILLDataMultiDestRemote(tFrameDescriptor &frameDesc){

    EthernetIIFrame * untaggedFrame = new EthernetIIFrame(frameDesc.name.c_str());
    AnsaEtherFrame * taggedFrame = new AnsaEtherFrame(frameDesc.name.c_str());

    taggedFrame->setKind(frameDesc.payload->getKind());
    //    taggedFrame->setSrc(frameDesc.src); ////will be set by underlying MAC module
    taggedFrame->setDest(frameDesc.dest);
    taggedFrame->setByteLength(ETHER_MAC_FRAME_BYTES);
    //    taggedFrame->setVlan(frameDesc.VID);//vlanId is set below during send
    taggedFrame->setEtherType(frameDesc.etherType);

    taggedFrame->encapsulate(frameDesc.payload->dup());
    if (taggedFrame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
    {
        taggedFrame->setByteLength(MIN_ETHERNET_FRAME_BYTES); // "padding"
    }

    untaggedFrame->setKind(frameDesc.payload->getKind());
    //    untaggedFrame->setSrc(frameDesc.src);//will be set by underlying MAC module
    untaggedFrame->setDest(frameDesc.dest);
    untaggedFrame->setByteLength(ETHER_MAC_FRAME_BYTES);
    untaggedFrame->setEtherType(frameDesc.etherType);

    untaggedFrame->encapsulate(frameDesc.payload->dup());
    if (untaggedFrame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
    {
        untaggedFrame->setByteLength(MIN_ETHERNET_FRAME_BYTES); // "padding"
    }

    RBVLANTable::tVIDPortList::iterator it;
    for (it = frameDesc.portList.begin(); it != frameDesc.portList.end(); it++)
    {
        if (it->port >= portCount)
        {
            continue;
        }
        //        if (spanningTree->forwarding(it->port, frameDesc.VID) == false) {
        //            continue;
        //        }

        TRILLInterfaceData *d = this->ift->getInterfaceByNetworkLayerGateIndex(it->port)->trillData();
        it->action = vlanTable->getTag(d->getDesigVlan(), it->port);
        if (it->action == RBVLANTable::INCLUDE)
        {

            taggedFrame->setVlan(d->getDesigVlan());
            send(taggedFrame->dup(), "lowerLayerOut", it->port);
        }
        else
        {
            send(untaggedFrame->dup(), "lowerLayerOut", it->port);
        }
    }

//        delete innerFrame;
//        delete trillFrame;
        delete untaggedFrame;
        delete taggedFrame;
        return true;
}




/*
 * ByGate means that the input interface is identified by gateIndex
 */

bool TRILL::isAllowedByGate(int vlanID, int gateIndex){

    return vlanTable->isAllowed(vlanID, gateIndex);

}

void TRILL::learn(AnsaEtherFrame *frame){
    //remember the last parameter is not interfaceIndex, but gateId WRONG
    rbMACTable->updateNative(frame->getSrc(), frame->getVlan(), frame->getArrivalGate()->getIndex());

}

void TRILL::learn(EthernetIIFrame *frame){
    //remember the last parameter is not interfaceIndex, but gateId WRONG!!
    /*TODO instead of frame->getVlan() do something like:
     * find out if tagAction ::REMOVE is set to this port
     *  if yes -> use some default vlanId
     *  if not -> drop frame?
     */
    rbMACTable->updateNative(frame->getSrc(), 1, frame->getArrivalGate()->getIndex());

}

void TRILL::learn(TRILL::tFrameDescriptor& frame) {
    if (!frame.src.isBroadcast() && !frame.src.isMulticast()) {
      rbMACTable->updateNative(frame.src, frame.VID, frame.rPort);
    }
}

void TRILL::learnTRILLData(TRILL::tFrameDescriptor &innerFrameDesc, int ingressNickname){
    if(!innerFrameDesc.src.isMulticast()){
        rbMACTable->updateTRILLData(innerFrameDesc.src, innerFrameDesc.VID, ingressNickname);
    }
}



void TRILL::handleIncomingFrame(EthernetIIFrame *frame) {
    // If buffer not full, insert payload frame into buffer and process the frame in parallel.
    cMessage *msg = this->currentMsg;
    ASSERT(msg->getContextPointer()==NULL);
    msg->setContextPointer(frame);
    scheduleAt(simTime(), msg);
    return;
}

void TRILL::processFrame(cMessage *msg) {
    EthernetIIFrame *frame = (EthernetIIFrame *) msg->getContextPointer();
    ASSERT(frame);
    msg->setContextPointer(NULL);
    int inputport = frame->getArrivalGate()->getIndex();

    handleAndDispatchFrame(frame, inputport);
    return;
}

void TRILL::handleAndDispatchFrame(EthernetIIFrame *frame, int inputport) {
    this->rbMACTable->update(frame->getSrc(), inputport);

/*
     AnsaEtherFrame * tmp;
     tmp = this->tagMsg(frame, 1);
     newframe = this->untagMsg(tmp);
*/

    // handle broadcast frames first
    if (frame->getDest().isBroadcast()) {
        EV<< "Broadcasting broadcast frame " << frame << endl;
        broadcastFrame(frame, inputport);
        return;
    }

    // Finds output port of destination address and sends to output port
    // if not found then broadcasts to all other ports instead
    MACTable::tPortList portList = rbMACTable->getPorts(frame->getDest());

    if (portList.size() == 0) {
        EV << "Dest address " << frame->getDest() << " unknown, broadcasting frame " << frame << endl;
        broadcastFrame(frame, inputport);
    } else {
        for (unsigned int i = 0; i < portList.size(); i++) {
            if (inputport != portList.at(i)) {
                EV << "Sending frame " << frame << " with dest address " << frame->getDest() << " to port " << portList.at(i) << endl;
                send(frame->dup(), "lowerLayerOut", portList.at(i));
            }
        }
        delete frame;
    }

    return;
}

void TRILL::broadcastFrame(EthernetIIFrame *frame, int inputport) {
    for (int i = 0; i < this->portCount; ++i) {
        if (i != inputport) {
            send((EthernetIIFrame*) frame->dup(), "lowerLayerOut", i);
        }
    }
    delete frame;

    return;
}

void TRILL::sinkMsg(cMessage *msg) {
    send(msg, "toSink");
    return;
}


void TRILL::sinkDupMsg(cMessage *msg) {
    send(msg->dup(), "toSink");
    return;
}

void TRILL::dispatchBPDU(cMessage *msg, int port) {
    if (port >= this->portCount || port < 0) {
        return;
    }

    EthernetIIFrame * untaggedFrame = new EthernetIIFrame(msg->getName());

    untaggedFrame->setKind(msg->getKind());
    untaggedFrame->setSrc(bridgeAddress);
    untaggedFrame->setDest(bridgeGroupAddress);
    untaggedFrame->setByteLength(ETHER_MAC_FRAME_BYTES);
    untaggedFrame->setEtherType(-1);

    untaggedFrame->encapsulate((cPacket *)msg);
    if (untaggedFrame->getByteLength() < MIN_ETHERNET_FRAME_BYTES) {
        untaggedFrame->setByteLength(MIN_ETHERNET_FRAME_BYTES); // "padding"
    }


    send((EthernetIIFrame*) untaggedFrame, "lowerLayerOut", port);
}

void TRILL::deliverBPDU(TRILL::tFrameDescriptor& frame) {
    send(frame.payload->dup(), "stpOut", frame.rPort);
}

void TRILL::finish() {
    cancelAndDelete(this->currentMsg);
    return;
}
/*
 void TRILL::tagMsg(int _vlan) {

 EthernetIIFrame * tmp = check_and_cast<EthernetIIFrame *> (this->currentMsg);

 cPacket * payload = tmp->decapsulate();
 AnsaEtherFrame * frame = new AnsaEtherFrame(payload->getName());

 frame->setSrc(tmp->getSrc());  // if blank, will be filled in by MAC
 frame->setDest(tmp->getDest());
 frame->setByteLength(ETHER_MAC_FRAME_BYTES);
 frame->setVlan(_vlan);

 frame->encapsulate(tmp);
 if (frame->getByteLength() < MIN_ETHERNET_FRAME) {
 frame->setByteLength(MIN_ETHERNET_FRAME);  // "padding"
 }
 this->currentMsg = frame;
 return;
 }*/


AnsaEtherFrame * TRILL::tagMsg(EthernetIIFrame * _frame, int _vlan) {

    EthernetIIFrame * tmp = _frame;

    cPacket * payload = tmp->decapsulate();
    AnsaEtherFrame * frame = new AnsaEtherFrame(payload->getName());

    frame->setSrc(tmp->getSrc());
    frame->setDest(tmp->getDest());
    frame->setByteLength(ETHER_MAC_FRAME_BYTES);
    frame->setVlan(_vlan);
    frame->setEtherType(tmp->getEtherType());

    frame->encapsulate(payload);
    if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES) {
        frame->setByteLength(MIN_ETHERNET_FRAME_BYTES); // "padding"
    }

    return frame;
}

EthernetIIFrame * TRILL::untagMsg(AnsaEtherFrame * _frame) {

    AnsaEtherFrame * tmp = _frame;

    cPacket * payload = tmp->decapsulate();
    EthernetIIFrame * frame = new EthernetIIFrame(payload->getName());

    frame->setSrc(tmp->getSrc()); // if blank, will be filled in by MAC
    frame->setDest(tmp->getDest());
    frame->setByteLength(ETHER_MAC_FRAME_BYTES);
    frame->setEtherType(tmp->getEtherType());

    frame->encapsulate(payload);
    if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES) {
        frame->setByteLength(MIN_ETHERNET_FRAME_BYTES); // "padding"
    }
    return frame;
}
/*

 void TRILL::untagMsg() {

 AnsaEtherFrame * tmp = check_and_cast<AnsaEtherFrame *> (this->currentMsg);

 cPacket * payload = tmp->decapsulate();
 EthernetIIFrame * frame = new EthernetIIFrame(payload->getName());

 frame->setSrc(tmp->getSrc());  // if blank, will be filled in by MAC
 frame->setDest(tmp->getDest());
 frame->setByteLength(ETHER_MAC_FRAME_BYTES);

 frame->encapsulate(tmp);
 if (frame->getByteLength() < MIN_ETHERNET_FRAME) {
 frame->setByteLength(MIN_ETHERNET_FRAME);  // "padding"
 }
 this->currentMsg = frame;
 return;
 }

*/
/* NEW */
TRILL::FrameCategory TRILL::classify(tFrameDescriptor &frameDesc){

    if(frameDesc.dest.compareTo(MACAddress("01-80-C2-00-00-00")) >=0 && frameDesc.dest.compareTo(MACAddress("01-80-C2-00-00-21")) <=0){
        frameDesc.category = TRILL_L2_CONTROL;
        return TRILL::TRILL_L2_CONTROL;
    }else if (frameDesc.etherType != ETHERTYPE_L2_ISIS && frameDesc.etherType != ETHERTYPE_TRILL &&
            (frameDesc.dest.compareTo(MACAddress("01-80-C2-00-00-40")) < 0 || frameDesc.dest.compareTo(MACAddress("01-80-C2-00-00-4F")) > 0)){
        frameDesc.category = TRILL_NATIVE;
        return TRILL::TRILL_NATIVE;
    }else if(frameDesc.etherType == ETHERTYPE_TRILL){
        frameDesc.category = TRILL_DATA;
        return TRILL::TRILL_DATA;
    }else if(frameDesc.etherType == ETHERTYPE_L2_ISIS){
        frameDesc.category = TRILL_CONTROL;
        return TRILL::TRILL_CONTROL;
    }else if (frameDesc.dest.compareTo(MACAddress(ALL_IS_IS_RBRIDGES)) > 0 && frameDesc.dest.compareTo(MACAddress("01-80-C2-00-00-4F")) <= 0){
        frameDesc.category = TRILL_OTHER;
        return TRILL::TRILL_OTHER; //these will get discarded
    }else{
        frameDesc.category = TRILL_NONE;
        return TRILL::TRILL_NONE;
    }

}


bool TRILL::processNative(tFrameDescriptor &frameDesc){

    if(!isNativeAllowed(frameDesc)){
        return false;
    }

    if(!frameDesc.dest.isBroadcast() && !frameDesc.dest.isMulticast()){
        //TODO A0 unicast RFC 6325 4.6.1.1

//    rbMACTable->get
    frameDesc.record = rbMACTable->getESTRecordByESTKey(std::make_pair(MACAddress(frameDesc.dest), frameDesc.VID));


    switch(frameDesc.record.type){
        case RBMACTable::EST_LOCAL_PROCESS:
            //process localy
            //i guess send it back to rBridgeSplitter?
            //TODO A2
            break;

        case RBMACTable::EST_LOCAL_PORT:
            //TODO multiple ports? (guess not, that's what MULTICAST is for)
            if(frameDesc.record.portList.at(0) == frameDesc.rPort){
                //RFC 6325 4.6.1.1.2.
                //frame received on the same link that it should get sent to so discard
                return false;

            }else{
                //TODO move to egress()?
                frameDesc.portList.clear();
//                RBVLANTable::s_vid_port vid_port = RBVLANTable::s_vid_port();
                RBVLANTable::tVIDPort vid_port;
                //TODO always remove tag for native frames?
                vid_port.action = RBVLANTable::REMOVE;
                vid_port.port = frameDesc.record.portList.at(0);
                frameDesc.portList.push_back(vid_port);

                return dispatchNativeLocalPort(frameDesc);
//                return true;
            }
            break;

        case RBMACTable::EST_RBRIDGE:
            //TODO A!
            dispatchNativeRemote(frameDesc);
            return true;
            break;

        case RBMACTable::EST_EMPTY:
            //not found ->treat as ... broadcast i guess?

            //TODO A! to be continued
//            frameDesc.portList.clear();
//            frameDesc.portList = vlanTable->getPorts(frameDesc.VID);
            //egressNative
//            return dispatchNativeLocalPort(frameDesc);
            return this->processNativeMultiDest(frameDesc);
            break;
        default:
            EV << "TRILL: ERROR: ProcessNative unrecognized type" <<endl;
    }
    }else{
        //egress and dispatch are called within processNativeMultiDest
        return this->processNativeMultiDest(frameDesc);
    }

    return true;
}
/*
 * Process native (non-TRILL encapsulated) frame with multi-destination Outer.MacDA.
 */
bool TRILL::processNativeMultiDest(tFrameDescriptor &frameDesc){
    /*
     * I don't plan to fully complete this method within my master's thesis, so it will remain in
     * to be continued... state until further notice ;).
     */

    //send it to all local ports in native form on which i am app fwd (app fwd check can be in egress method)
    frameDesc.portList.clear(); //clear output ports (obviously)
    frameDesc.portList = this->vlanTable->getPorts(frameDesc.VID); //set it to
    //minimize output ports
    this->egressNativeLocal(frameDesc);
    dispatchNativeLocalPort(frameDesc);


    frameDesc.portList.clear();
    //send it to all ports with RBridge Adjacency as TRILL-encapsulated
    egressNativeMulticastRemote(frameDesc);
    return dispatchNativeMultiDestRemote(frameDesc);

}

bool TRILL::processTRILLData(tFrameDescriptor &frameDesc){

    //RFC 6325 4.6.2. 1.
    //skipping L2_ISIS frames are handled earlier

    //RFC 6325 4.6.2. 2.
    if(frameDesc.dest.compareTo(MACAddress("01-80-C2-00-00-40")) >= 0 && frameDesc.dest.compareTo(MACAddress("01-80-C2-00-00-4F")) <= 0 && frameDesc.dest.compareTo(MACAddress(ALL_RBRIDGES)) != 0){
        //discard
        return false;
    }
    //RFC 6325 4.6.2. 3.
    if(!frameDesc.dest.isMulticast() && frameDesc.dest.compareTo(frameDesc.d->getInterfaceEntry()->getMacAddress()) != 0){
        //discard
        return false;
    }

    //RFC 6325 4.6.2 4. ?? how could this frame NOT-have TRILL ethertype?
    //skipping

    TRILLFrame *trillFrame = static_cast<TRILLFrame *> (frameDesc.payload);
    //RFC 6325 4.6.2 5.
    if(trillFrame->getVersion() != TRILL_VERSION){
        return false;
    }
    // 6.
    if(trillFrame->getHopCount() == 0){
        return false;
    }

    // 7.
    if((frameDesc.dest.isMulticast() && !trillFrame->getMultiDest()) || (!frameDesc.dest.isMulticast() && trillFrame->getMultiDest() ) ){
        return false;

    }

    // 8.
    if(this->isis->getAdjByMAC(frameDesc.src, L1_TYPE) == NULL){
        return false;
    }

    // .9
    //we do not (yet) implement ESADI so skipping and processing proceeds as TRILL Data Frames :D


    //RFC 6325 4.6.2.3.
    if(!trillFrame->getMultiDest()){
        //RFC 6325 4.6.2.4
        return processTRILLDataUnicast(frameDesc);
    }
    else{
        //RFC 6325 4.6.2.5
        return processTRILLDataMultiDest(frameDesc);
    }

    return true;

}

bool TRILL::processTRILLDataUnicast(tFrameDescriptor &frameDesc){

    //check egress Nickname. if it is unknown or reserved -> discard
    tFrameDescriptor innerFrameDesc;
    TRILLFrame *trillFrame = static_cast<TRILLFrame *>(frameDesc.payload);
    AnsaEtherFrame *innerFrame = static_cast<AnsaEtherFrame *>((frameDesc.payload->dup())->decapsulate());

    this->ingress(innerFrameDesc, innerFrame, frameDesc.rPort);
    this->learnTRILLData(innerFrameDesc, trillFrame->getIngressRBNickname());
    bool result;
    if(trillFrame->getEgressRBNickname() == this->isis->getNickname()){
        //destination RBridge
//        AnsaEtherFrame *innerFrame = static_cast<AnsaEtherFrame *>((frameDesc.payload->dup())->decapsulate());
        if(innerFrame->getDest().isMulticast()){
            //discard
            return false;
        }

        //TODO A2 if the Inner.MacDA is local address -> local processing

        if(innerFrameDesc.VID == 0x0 || innerFrameDesc.VID == 0xFFF){
            //discard
            return false;
        }


        innerFrameDesc.record = rbMACTable->getESTRecordByESTKey(std::make_pair(MACAddress(innerFrameDesc.dest), innerFrameDesc.VID));

        switch (innerFrameDesc.record.type)
            {
            case RBMACTable::EST_LOCAL_PROCESS:
                //process localy
                //i guess send it back to rBridgeSplitter?
                //TODO A2
                break;

            case RBMACTable::EST_LOCAL_PORT:
                //TODO multiple ports? (guess not, that's what MULTICAST is for)

                    //TODO move to egress()?
                    innerFrameDesc.portList.clear();
//                    RBVLANTable::s_vid_port vid_port = RBVLANTable::s_vid_port();
//                    RBVLANTable::s_vid_port vid_port = RBVLANTable::s_vid_port();
                    RBVLANTable::tVIDPort vid_port;
                    //TODO always remove tag for native frames?
                    vid_port.action = RBVLANTable::REMOVE;
                    vid_port.port = innerFrameDesc.record.portList.at(0);
                    innerFrameDesc.portList.push_back(vid_port);

                    result = dispatchNativeLocalPort(innerFrameDesc);
                    delete innerFrame;
                    delete innerFrameDesc.payload;
                    return result;
                    //                return true;

                break;

            case RBMACTable::EST_RBRIDGE:
                /*
                 * This should never happen. When the frame innerFrame is unicast and this is the
                 * destination RBridge, then it should be localy delivered.
                 * This situation could mean that the end-station has relocated and therefore,
                 * it's record should be deleted and when running ESADI, immediately initiate sending updating LSP.
                 */
                EV<< "TRILL: ERROR: ProcessTRILLDataUnicast" <<endl;
                //TODO A! change to dispatchNativeMulti (should be send on all links where this RBridge is App Fwd)

                result = dispatchNativeRemote(innerFrameDesc);
                delete innerFrame;
                delete innerFrameDesc.payload;
                return result;
                break;

            case RBMACTable::EST_EMPTY:
                //not found ->treat as ... broadcast i guess?

                //TODO A! to be continued
                //            innerFrameDesc.portList.clear();
                //            innerFrameDesc.portList = vlanTable->getPorts(innerFrameDesc.VID);
                //egressNative
                //            return dispatchNativeLocalPort(innerFrameDesc);
                innerFrameDesc.portList.clear();//clear output ports (obviously)
                innerFrameDesc.portList = this->vlanTable->getPorts(innerFrameDesc.VID);//set it to
                //minimize output ports
                this->egressNativeLocal(innerFrameDesc);
                result = dispatchNativeLocalPort(innerFrameDesc);
                delete innerFrame;
                delete innerFrameDesc.payload;
                return result;


            break;
                default:
                EV << "TRILL: ERROR: ProcessNative unrecognized type" <<endl;
                delete innerFrame;
                delete innerFrameDesc.payload;

                return false;
            }

        }
        else
        {
            //transit RBridge
            trillFrame->decHopCount();
            //egressTRILLDataUnicastRemote
            result = this->dispatchTRILLDataUnicastRemote(frameDesc);
            delete innerFrame;
            delete innerFrameDesc.payload;

                            return result;
        }



}


bool TRILL::processTRILLDataMultiDest(tFrameDescriptor &frameDesc){

    TRILLFrame *trillFrame = static_cast<TRILLFrame *> (frameDesc.payload);

    //RFC 6325 4.6.2.5.
    //examine ingress/egress nicknames, if iether is unknown or reserved -> discard frame

    //get adjacencies/paths for Outer.MacSA and if it doesn't belong to specified tree -> discard
    ISISadj *adj = this->isis->getAdjByMAC(frameDesc.src, L1_TYPE, frameDesc.rPort);
    if(adj == NULL){
        //discard -> received from somebody with whom we don't have adjacency
        return false;
    }

    //get paths from nickname's tree
    std::vector<unsigned char *> *systemIDs = this->isis->getSystemIDsFromTree(trillFrame->getEgressRBNickname(), this->isis->getSysId());

    bool found = false;
    for(std::vector<unsigned char *>::iterator it = systemIDs->begin(); it != systemIDs->end(); ++it){
        ISISadj *tmpAdj = this->isis->getAdjBySystemID((*it), L1_TYPE);
        if(tmpAdj == adj){
            //adjacency belong to specified tree so remove it from destinations and mark it as found
            found = true;
            systemIDs->erase(it);
            break;

        }
    }

    if(!found){
        return false;
    }



    //Reverse path Forwarding check on both nicknames

    //skipping additional check more info in RFC 6325 section 4.5.2.
    tFrameDescriptor innerFrameDesc;

    AnsaEtherFrame *innerFrame = static_cast<AnsaEtherFrame *>((frameDesc.payload->dup())->decapsulate());

    this->ingress(innerFrameDesc, innerFrame, frameDesc.rPort);

    if(innerFrameDesc.VID == 0x0 || innerFrameDesc.VID == 0xFFF){
        //discard
        delete innerFrame;
        delete innerFrameDesc.payload;
        return false;
    }

    //if I'm App Fwd for innerFrameDesc.VID, then learn Inner.MacSA and Inner.VLAN ID (same rules apply - unicast, ...)
    //send in native form so i guess dispatchNativeLocal()?


    for(int i = 0; i < ift->getNumInterfaces(); i++){
        TRILLInterfaceData *d = ift->getInterface(i)->trillData();
        if(d->isAppointedForwarder(innerFrameDesc.VID, this->isis->getNickname())){
            //learn

                this->learnTRILLData(innerFrameDesc, trillFrame->getIngressRBNickname());


//            innerFrameDesc.portList.push_back(ift->getInterface(i)->getNetworkLayerGateIndex());
            break;
        }
    }

    this->egressTRILLDataMultiDestNative(innerFrameDesc);

    this->dispatchNativeLocalPort(innerFrameDesc);
    //delete copy of innerFrame
    delete innerFrame; //the original innerFrame is still in frameDesc.payload (TRILL encapsulated)
    delete innerFrameDesc.payload;


    //hop count decreased
    trillFrame->decHopCount();

    //from set of adjacencies/paths delete the source one and send it all others

//    frameDesc.systemIDs = systemIDs;

    this->egressTRILLDataMultiDestRemote(frameDesc);

    this->dispatchTRILLDataMultiDestRemote(frameDesc);
    /* dispatchTRILLDataMultiDest{
     *   encap frameDesc.payload
     *   set dst, ethertype ...
     *   for(port in portList){
     *
     *     setVland
     *     send
     *    }
     *  }
     */


    return true;
}

bool TRILL::isNativeAllowed(tFrameDescriptor &frameDesc){

    TRILLInterfaceData *d = (ift->getInterfaceByNetworkLayerGateIndex(frameDesc.rPort))->trillData();
    //RFC 6325 4.6.1
    if(d->isDisabled() || d->isTrunk() || d->isP2p()){
        //frame should be discarded
        return false;
    }

    if(d->isAppointedForwarder(frameDesc.VID, this->isis->getNickname()) && !d->isInhibited()){
        this->learn(frameDesc);
        return true;
    }
    return false;
}
