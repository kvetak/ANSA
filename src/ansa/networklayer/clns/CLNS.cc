// Copyright (C) 2012 - 2016 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
 * @file CLNS.cc
 * @author Marcel Marek (mailto:imarek@fit.vutbr.cz)
 * @date 5.8.2016
 */

#include "ansa/networklayer/clns/CLNS.h"
#include "ansa/networklayer/clns/CLNSRoutingTable.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/common/ModuleAccess.h"
//#include "ansa/networklayer/isis/ISISMessage_m.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
//#include "inet/networklayer/contract/clns/ClnsControlInfo.h"

namespace inet {

Define_Module(CLNS);

void CLNS::initialize(int stage)
{
  if (stage == INITSTAGE_LOCAL) {
    QueueBase::initialize();

    ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    rt = getModuleFromPar<CLNSRoutingTable>(par("routingTableModule"), this);


    transportInGateBaseId = gateBaseId("transportIn");
    queueOutGateBaseId = gateBaseId("queueOut");


    registerService(Protocol::clns, gate("transportIn"), gate("queueIn"));
    registerProtocol(Protocol::clns, gate("queueOut"), gate("transportOut"));

  }
  else if (stage == INITSTAGE_LAST) {
    isUp = true;
  }
}

void CLNS::handleMessage(cMessage *msg)
{
    //TODO This was replaces by the protocol register?
//  if (dynamic_cast<RegisterTransportProtocolCommand *>(msg)) {
//    RegisterTransportProtocolCommand *command = check_and_cast<RegisterTransportProtocolCommand *>(msg);
//    mapping.addProtocolMapping(command->getProtocol(), msg->getArrivalGate()->getIndex());
//    delete msg;
//  }
//  else
      if (!msg->isSelfMessage() && msg->getArrivalGate()->isName("arpIn"))
    endService(PK(msg));
  else
    QueueBase::handleMessage(msg);

}

void CLNS::endService(cPacket *packet)
{
  if (!isUp) {
    EV_ERROR << "CLNS is down -- discarding message\n";
    delete packet;
    return;
  }
  if (packet->getArrivalGate()->isName("transportIn")) { //TODO packet->getArrivalGate()->getBaseId() == transportInGateBaseId
    handlePacketFromHL(check_and_cast<Packet*>(packet));
  }
//  else if (packet->getArrivalGate() == arpInGate) {
//    handlePacketFromARP(packet);
//  }
  else {    // from network
    EV_INFO << "Received " << packet << " from network.\n";
    const InterfaceEntry *fromIE = getSourceInterfaceFrom(check_and_cast<Packet*>(packet));
    if (dynamic_cast<Packet *>(packet))
      handleIncomingISISMessage(check_and_cast<Packet*>(packet), fromIE);
//    else if (dynamic_cast<IPv4Datagram *>(packet))
//      handleIncomingDatagram((IPv4Datagram *) packet, fromIE);
    else
      throw cRuntimeError(packet, "Unexpected packet type");
  }

  if (hasGUI())
    updateDisplayString();
}

void CLNS::handleIncomingISISMessage(Packet* packet, const InterfaceEntry* fromIE)
{
    send(packet, "transportOut");
//  int protocol = 1234;
//  Ieee802Ctrl* ctrl = static_cast<Ieee802Ctrl*>(packet->getControlInfo());
//  ctrl->setInterfaceId(fromIE->getInterfaceId());



//  int gateindex = mapping.findOutputGateForProtocol(protocol);
      // check if the transportOut port are connected, otherwise discard the packet
//  if (gateindex >= 0) {
//    cGate *outGate = gate("transportOut", gateindex);
//    if (outGate->isPathOK()) {
//      send(packet, outGate);
//      numLocalDeliver++;
//      return;
//    }
//  }
//
//  EV_ERROR << "Transport protocol ID=" << protocol << " not connected, discarding packet\n";

}

const InterfaceEntry *CLNS::getSourceInterfaceFrom(Packet *packet)
{
    int interfaceId = packet->getTag<InterfaceInd>()->getInterfaceId();
    return ift->getInterfaceById(interfaceId);
//    cGate *g = packet->getArrivalGate();
//    return g ? ift->getInterfaceByNetworkLayerGateIndex(g->getIndex()) : nullptr;
}


void CLNS::handlePacketFromHL(Packet *packet)
{
    EV_INFO << "Received " << packet << " from upper layer.\n";


    //TODO Fix it properly, use Ieee802SapTag with SAP_CLNS (see EtherFrameWithLlc)
    send(packet, queueOutGateBaseId);

//    // if no interface exists, do not send datagram
//    if (ift->getNumInterfaces() == 0) {
//        EV_ERROR << "No interfaces exist, dropping packet\n";
//        numDropped++;
//        delete packet;
//        return;
//    }
//
//    if(dynamic_cast<ISISMessage*>(packet)){
//      CLNSControlInfo* info = static_cast<CLNSControlInfo*>(packet->removeControlInfo());
//      int id = info->getInterfaceId();
//      InterfaceEntry *ie = ift->getInterfaceById(id);
//      if(ie != nullptr){
//        Ieee802Ctrl* ctrl = new Ieee802Ctrl();
//        ctrl->setSrc(info->getSrc());
//        ctrl->setDest(info->getDest());
//        ctrl->setSsap(SAP_CLNS);
//        ctrl->setDsap(SAP_CLNS);
//        ctrl->setInterfaceId(id);
//        packet->setControlInfo(ctrl);
//        send(packet, queueOutGateBaseId + ie->getNetworkLayerGateIndex());
//      }
//      delete info;
//    }
}

void CLNS::handleRegisterService(const Protocol& protocol, cGate *out, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterService");
}

void CLNS::handleRegisterProtocol(const Protocol& protocol, cGate *in, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterProtocol");
    if (in->isName("transportIn"))
            upperProtocols.insert(&protocol);
}

void CLNS::updateDisplayString()
{
//    char buf[80] = "";
//    if (numForwarded > 0)
//        sprintf(buf + strlen(buf), "fwd:%d ", numForwarded);
//    if (numLocalDeliver > 0)
//        sprintf(buf + strlen(buf), "up:%d ", numLocalDeliver);
//    if (numMulticast > 0)
//        sprintf(buf + strlen(buf), "mcast:%d ", numMulticast);
//    if (numDropped > 0)
//        sprintf(buf + strlen(buf), "DROP:%d ", numDropped);
//    if (numUnroutable > 0)
//        sprintf(buf + strlen(buf), "UNROUTABLE:%d ", numUnroutable);
//    getDisplayString().setTagArg("t", 0, buf);
}

ClnsAddress CLNS::getKAddress(unsigned int k) const {

    if(k< localAddresses.size())
    {
        return localAddresses[k];
    }
}

} //namespace
