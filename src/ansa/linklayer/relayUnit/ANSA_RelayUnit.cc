// Copyright (C) 2013 OpenSim Ltd.
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
// Author: Benjamin Martin Seregi


// TODO rename/change "cdpIn" etc gates to upperLayer more importantly - use protocol tags to

#include "ANSA_RelayUnit.h"

#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/linklayer/configurator/Ieee8021dInterfaceData.h"
#include "inet/common/ModuleAccess.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ethernet/EtherEncap.h"

namespace inet {

Define_Module(ANSA_RelayUnit);

ANSA_RelayUnit::ANSA_RelayUnit()
{
}

void ANSA_RelayUnit::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        // statistics
        numDispatchedUpdateFrames = numDispatchedNonUpdateFrames = numDeliveredUpdatesToCDP = 0;
        numReceivedUpdatesFromCDP = numReceivedNetworkFrames = numDroppedFrames = 0;
        numDeliveredUpdatesToLLDP = numReceivedUpdatesFromLLDP = 0;

        isSTPAware = par("hasStp");// true;
        isCDPAware = par("hasCDP"); //true;    // if the cdpIn is not connected then the switch is CDP unaware
        isLLDPAware = par("hasLLDP"); //true;    // if the cdpIn is not connected then the switch is CDP unaware
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        registerService(Protocol::ethernetMac, nullptr, gate("ifIn"));
        registerProtocol(Protocol::ethernetMac, gate("ifOut"), nullptr);

        //TODO FIX
        if(isSTPAware) {
            registerAddress(MacAddress::STP_MULTICAST_ADDRESS);
        }
    }
    else if (stage == INITSTAGE_LINK_LAYER_2) {
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;

        macTable = check_and_cast<IMacAddressTable *>(getModuleByPath(par("macTablePath")));
        ifTable = check_and_cast<IInterfaceTable *>(getModuleByPath(par("interfaceTablePath")));

        if (isOperational) {
            ie = chooseInterface();

            if (ie)
            {
                bridgeAddress = ie->getMacAddress(); // get the bridge's MAC address
                registerAddress(bridgeAddress);
            }
            else
            {
                throw cRuntimeError("No non-loopback interface found!");
            }
        }


        WATCH(bridgeAddress);
        WATCH(numReceivedNetworkFrames);
        WATCH(numDroppedFrames);
        WATCH(numReceivedUpdatesFromCDP);
        WATCH(numDeliveredUpdatesToCDP);
        WATCH(numReceivedUpdatesFromLLDP);
        WATCH(numDeliveredUpdatesToLLDP);
        WATCH(numDispatchedNonUpdateFrames);
    }

}

void ANSA_RelayUnit::registerAddress(MacAddress mac)
{
    registerAddresses(mac, mac);

}


void ANSA_RelayUnit::registerAddresses(MacAddress startMac, MacAddress endMac)
{
    registeredMacAddresses.insert(MacAddressPair(startMac, endMac));

}

void ANSA_RelayUnit::handleMessage(cMessage *msg)
{
    if (!isOperational) {
        EV_ERROR << "Message '" << msg << "' arrived when module status is down, dropped it." << endl;
        delete msg;
        return;
    }

    if (!msg->isSelfMessage()) {

        if (strcmp(msg->getArrivalGate()->getName(), "upperLayerIn") == 0)
        {
            handleAndDispatchFrameFromHL(check_and_cast<Packet *>(msg));
        }
        // messages from network
        else if (strcmp(msg->getArrivalGate()->getName(), "ifIn") == 0) {
            numReceivedNetworkFrames++;
            EV_INFO << "Received " << msg << " from network." << endl;
            Packet *packet = check_and_cast<Packet *>(msg);
            handleAndDispatchFrame(packet);
        }
    }
    else
        throw cRuntimeError("This module doesn't handle self-messages!");
}

void ANSA_RelayUnit::handleAndDispatchFrameFromHL(Packet *packet)
{
    const auto& frame = packet->peekAtFront<EthernetMacHeader>();

    InterfaceReq* interfaceReq = packet->findTag<InterfaceReq>();
    int interfaceId =
            interfaceReq == nullptr ? -1 : interfaceReq->getInterfaceId();

    if (interfaceId != -1) {
        dispatch(packet, interfaceId);
    } else if (frame->getDest().isBroadcast()) {    // broadcast address
        broadcast(packet);
    } else {
        int outInterfaceId = macTable->getPortForAddress(frame->getDest());
        // Not known -> broadcast
        if (outInterfaceId == -1) {
            EV_DETAIL << "Destination address = " << frame->getDest()
                             << " unknown, broadcasting frame " << frame
                             << endl;
            broadcast(packet);
        } else {

            dispatch(packet, outInterfaceId);

        }
    }
}

void ANSA_RelayUnit::broadcast(Packet *packet)
{
    EV_DETAIL << "Broadcast frame " << packet << endl;


    InterfaceInd* interfaceInd = packet->findTag<InterfaceInd>();
    int inputInterfaceId = interfaceInd == nullptr ? -1 : interfaceInd->getInterfaceId();

    int numPorts = ifTable->getNumInterfaces();
    for (int i = 0; i < numPorts; i++) {
        InterfaceEntry *ie = ifTable->getInterface(i);
        if (ie->isLoopback() || !ie->isBroadcast())
            continue;
        if (ie->getInterfaceId() != inputInterfaceId) {
            dispatch(packet->dup(), ie->getInterfaceId());
        }
    }
    delete packet;
}

void ANSA_RelayUnit::handleAndDispatchFrame(Packet *packet)
{
    const auto& frame = packet->peekAtFront<EthernetMacHeader>();
    int arrivalInterfaceId = packet->getTag<InterfaceInd>()->getInterfaceId();
    InterfaceEntry *arrivalInterface = ifTable->getInterfaceById(arrivalInterfaceId);
    Ieee8021dInterfaceData *arrivalPortData = arrivalInterface->ieee8021dData();


    learn(frame->getSrc(), arrivalInterfaceId);


    // BPDU Handling
    if (isSTPAware
               && (frame->getDest() == MacAddress::STP_MULTICAST_ADDRESS || frame->getDest() == bridgeAddress)
               && arrivalPortData->getRole() != Ieee8021dInterfaceData::DISABLED) { // removed "&& isBpdu(packet, frame)"
           EV_DETAIL << "Deliver BPDU to the STP/RSTP module" << endl;
           sendUp(packet);
    } else if (in_range(registeredMacAddresses, frame->getDest())) {
        // destination MAC address is registered, send it up
        sendUp(packet);

    } else if (frame->getDest().isBroadcast()) {    // broadcast address
        broadcast(packet);
    }
    else {
        int outInterfaceId = macTable->getPortForAddress(frame->getDest());
        // Not known -> broadcast
        if (outInterfaceId == -1) {
            EV_DETAIL << "Destination address = " << frame->getDest() << " unknown, broadcasting frame " << frame << endl;
            broadcast(packet);
        }
        else {
            if (outInterfaceId != arrivalInterfaceId) {
                //Ieee8021dInterfaceData *outPortData = getPortInterfaceData(outInterfaceId);

//                if (!isCDPAware && !isLLDPAware)
                    dispatch(packet, outInterfaceId);
//                else {
//                    EV_INFO << "Output port " << outInterfaceId << " is not forwarding. Discarding!" << endl;
//                    numDroppedFrames++;
//                    delete packet;
//                }
            }
            else {
                EV_DETAIL << "Output port is same as input port, " << frame->getFullName() << " destination = " << frame->getDest() << ", discarding frame " << frame << endl;
                numDroppedFrames++;
                delete packet;
            }
        }
    }
}

void ANSA_RelayUnit::dispatch(Packet *packet, unsigned int interfaceId)
{
    const auto& frame = packet->peekAtFront<EthernetMacHeader>();
    EV_INFO << "Sending frame " << packet << " on output port " << interfaceId << "." << endl;

    EV_INFO << "Sending " << packet << " with destination = " << frame->getDest() << ", port = " << interfaceId << endl;

    numDispatchedNonUpdateFrames++;
    auto oldPacketProtocolTag = packet->removeTag<PacketProtocolTag>();
    packet->clearTags();
    auto newPacketProtocolTag = packet->addTag<PacketProtocolTag>();
    *newPacketProtocolTag = *oldPacketProtocolTag;
    delete oldPacketProtocolTag;
    packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(interfaceId);
    send(packet, "ifOut");
}

void ANSA_RelayUnit::learn(MacAddress srcAddr, int arrivalInterfaceId)
{
    if (!isCDPAware && !isLLDPAware)
        macTable->updateTableWithAddress(arrivalInterfaceId, srcAddr);
}

void ANSA_RelayUnit::sendUp(Packet *packet)
{

    send(packet, "upperLayerOut");
}

void ANSA_RelayUnit::sendDown(Packet *packet){

    send(packet, "ifOut");
}

void ANSA_RelayUnit::start()
{
    isOperational = true;

    ie = chooseInterface();
    if (ie)
        bridgeAddress = ie->getMacAddress(); // get the bridge's MAC address
    else
        throw cRuntimeError("No non-loopback interface found!");

    macTable->clearTable();
}

void ANSA_RelayUnit::stop()
{
    isOperational = false;

    macTable->clearTable();
    ie = nullptr;
}

InterfaceEntry *ANSA_RelayUnit::chooseInterface()
{
    // TODO: Currently, we assume that the first non-loopback interface is an Ethernet interface
    //       since relays work on EtherSwitches.
    //       NOTE that, we don't check if the returning interface is an Ethernet interface!
    IInterfaceTable *ift = check_and_cast<IInterfaceTable *>(getModuleByPath(par("interfaceTablePath")));

    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        InterfaceEntry *current = ift->getInterface(i);
        if (!current->isLoopback())
            return current;
    }

    return nullptr;
}

void ANSA_RelayUnit::finish()
{
    recordScalar("number of received BPDUs from STP module", numReceivedUpdatesFromCDP);
    recordScalar("number of received frames from network (including BPDUs)", numReceivedNetworkFrames);
    recordScalar("number of dropped frames (including BPDUs)", numDroppedFrames);
    recordScalar("number of delivered BPDUs to the STP module", numDeliveredUpdatesToCDP);
    recordScalar("number of dispatched BPDU frames to the network", numDispatchedUpdateFrames);
    recordScalar("number of dispatched non-BDPU frames to the network", numDispatchedNonUpdateFrames);
}

bool ANSA_RelayUnit::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_LINK_LAYER) {
            start();
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_LINK_LAYER) {
            stop();
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if ((NodeCrashOperation::Stage)stage == NodeCrashOperation::STAGE_CRASH) {
            stop();
        }
    }
    else {
        throw cRuntimeError("Unsupported operation '%s'", operation->getClassName());
    }

    return true;
}

} // namespace inet

