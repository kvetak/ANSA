// Copyright (C) 2012 - 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
 * @file RBridgeSplitter.cc
 * @author Marcel Marek (mailto:xscrew02@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 10.2.2013
 * @brief
 * @detail
 * @todo
 */

#include "RBridgeSplitter.h"

Define_Module(RBridgeSplitter);
//RBridgeSplitter::RBridgeSplitter()
//{
//    // TODO Auto-generated constructor stub
//
//}
//
//RBridgeSplitter::~RBridgeSplitter()
//{
//    // TODO Auto-generated destructor stub
//}

void RBridgeSplitter::initialize(int stage){

    if(stage == 3){
        trillModule = TRILLAccess().get();

        isisModule = ISISAccess().get();

        this->vlanTableModule = ModuleAccess<RBVLANTable>("rBVLANTable").get();

//        this->portTableModule = ModuleAccess<RBPortTable>("rBVPortTable").get();

        this->ift = InterfaceTableAccess().get();
    }
}

void RBridgeSplitter::handleMessage(cMessage *msg){

    cGate* gate = msg->getArrivalGate();
    std::string gateName = gate->getBaseName();
    int gateIndex = gate->getIndex();

    // packet coming from network layer modules within the router
    if (gateName == "isisIn")
    {
        int vlanId = vlanTableModule->getVID(gateIndex);

        if (vlanTableModule->getTag(vlanId, gateIndex) == RBVLANTable::INCLUDE)
        {
            InterfaceEntry *ie = ift->getInterfaceByNetworkLayerGateIndex(gateIndex);
            Ieee802Ctrl *ctrl = (Ieee802Ctrl *) msg->getControlInfo();
            msg->removeControlInfo();
            ctrl->setEtherType(ETHERTYPE_L2_ISIS);
            ctrl->setDest(MACAddress(ALL_IS_IS_RBRIDGES)); //ALL-IS-IS-RBridges
            ctrl->setSrc(ie->getMacAddress());

            AnsaEtherFrame *frame = new AnsaEtherFrame(msg->getName());
            frame->setKind(msg->getKind());
            frame->setSrc(ie->getMacAddress());
            frame->setDest(MACAddress(ALL_IS_IS_RBRIDGES));
            frame->setByteLength(ETHER_MAC_FRAME_BYTES);
            frame->setVlan(vlanId);
            frame->setEtherType(ETHERTYPE_L2_ISIS);
            frame->encapsulate((ISISMessage *) msg);
            if(frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES) {
                frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);
            }
            frame->setControlInfo(ctrl->dup());








            this->send(frame, "lowerLayerOut", gateIndex);


        }
        else
        {

            //set ethertype L2_ISIS

            ((Ieee802Ctrl *)msg->getControlInfo())->setEtherType(ETHERTYPE_L2_ISIS);
            ((Ieee802Ctrl *)msg->getControlInfo())->setDest(MACAddress(ALL_IS_IS_RBRIDGES));
            // send packet to out interface
            this->send(msg, "lowerLayerOut", gateIndex);


        }

    // packet coming from in interfaces
    }else if(gateName == "upperLayerIn" || gateName == "trillIn"){
        this->send(msg, "lowerLayerOut", gateIndex);
    }
    else
    {
        if(dynamic_cast<AnsaEtherFrame *>(msg)){
//            EthernetIIFrame * frame = (EthernetIIFrame *) msg;
            AnsaEtherFrame * frame = (AnsaEtherFrame *)msg;
            //if src unicast
              //trillModule->learn(msg);

            //check integrity, ...

            //if ethertype == L2_ISIS AND Outer.MacDA == ALL-IS-IS-RBridges
            if(frame->getEtherType() == ETHERTYPE_L2_ISIS){
                if(trillModule->isAllowedByGate(frame->getVlan(), frame->getArrivalGate()->getIndex())){
                                trillModule->learn(frame);
                                Ieee802Ctrl *ctrl = (Ieee802Ctrl *) frame->getControlInfo();
                                if(ctrl == NULL){
                                    ctrl= new Ieee802Ctrl();
                                    ctrl->setSrc(frame->getSrc());
                                    ctrl->setDest(frame->getDest());
                                    ctrl->setEtherType(frame->getEtherType());

                                }
                                cPacket *packet = frame->decapsulate()->dup();
                                packet->setControlInfo(ctrl->dup());
                                this->send(packet, "isisOut", gateIndex);
                }else{
                    EV <<" Warning L2_ISIS frame with not-allowed vlan tag" << endl;
                    delete msg;
                }
                return;
            }
            //
            else{
                this->send(msg,"trillOut", gateIndex);
            }
        }
        //
        else if (dynamic_cast<EthernetIIFrame *>(msg))
        {
            EthernetIIFrame * frame = (EthernetIIFrame *) msg;
//                        AnsaEtherFrame * frame = (AnsaEtherFrame *)msg;
            //if src unicast
            //trillModule->learn(msg);

            //check integrity, ...

            //if ethertype == L2_ISIS AND Outer.MacDA == ALL-IS-IS-RBridges
            if (frame->getEtherType() == ETHERTYPE_L2_ISIS)
            {
//                            if(trillModule->isAllowedByGate(frame->getVlan(), frame->getArrivalGateId())){
                trillModule->learn(frame);
                this->send(msg, "isisOut", gateIndex);
//                            }else{
//                                EV <<" Warning L2_ISIS frame with not-allowed vlan tag" << endl;
//                                delete msg;
//                            }
                return;
            }
            //
            else
            {
                this->send(msg, "trillOut", gateIndex);
            }
        }
        else
        {
            EV << "Warning: received unsupported frame type" << endl;
        }












//        if (dynamic_cast<ISISMessage *>(msg))
//        {
//            this->send(msg, "isisOut", gateIndex);
//            return;
//        }


        // IPv6 datagram, send it to networkLayer6 via ipv6Out
//        if (dynamic_cast<IPv6Datagram *>(msg))
//        {
//            this->send(msg, "ipv6Out", gateIndex);
//        }
//        else
//        {
//            if(dynamic_cast<ISISMessage *>(msg))
//            {
//                this->send(msg, "isisOut", gateIndex);
//            }
//
//            // other (IPv4), send it to networkLayer via ipv4Out
//            else
//            {
//                this->send(msg, "ipv4Out", gateIndex);
//            }
//        }
    }
}

