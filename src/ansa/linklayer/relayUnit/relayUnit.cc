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

#include "ansa/linklayer/relayUnit/relayUnit.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/linklayer/configurator/Ieee8021dInterfaceData.h"
#include "inet/common/ModuleAccess.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ethernet/EtherEncap.h"

namespace inet {

Define_Module(relayUnit);

relayUnit::relayUnit()
{
}

void relayUnit::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        // statistics
        numDispatchedUpdateFrames = numDispatchedNonUpdateFrames = numDeliveredUpdatesToCDP = 0;
        numReceivedUpdatesFromCDP = numReceivedNetworkFrames = numDroppedFrames = 0;
        numDeliveredUpdatesToLLDP = numReceivedUpdatesFromLLDP = 0;
    }
    else if (stage == INITSTAGE_LINK_LAYER_2) {
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;

        macTable = check_and_cast<IMacAddressTable *>(getModuleByPath(par("macTablePath")));
        ifTable = check_and_cast<IInterfaceTable *>(getModuleByPath(par("interfaceTablePath")));

        if (isOperational) {
            ie = chooseInterface();

            if (ie)
                bridgeAddress = ie->getMacAddress(); // get the bridge's MAC address
            else
                throw cRuntimeError("No non-loopback interface found!");
        }

        isCDPAware = true;    // if the cdpIn is not connected then the switch is CDP unaware
        isLLDPAware = true;    // if the cdpIn is not connected then the switch is CDP unaware

        WATCH(bridgeAddress);
        WATCH(numReceivedNetworkFrames);
        WATCH(numDroppedFrames);
        WATCH(numReceivedUpdatesFromCDP);
        WATCH(numDeliveredUpdatesToCDP);
        WATCH(numReceivedUpdatesFromLLDP);
        WATCH(numDeliveredUpdatesToLLDP);
        WATCH(numDispatchedNonUpdateFrames);
    }
    bridgeGroupCDPAddress = MacAddress("01-00-0c-cc-cc-cc");
    bridgeGroupLLDPAddress = MacAddress("01-80-c2-00-00-0e");
}

void relayUnit::handleMessage(cMessage *msg)
{
    if (!isOperational) {
        EV_ERROR << "Message '" << msg << "' arrived when module status is down, dropped it." << endl;
        delete msg;
        return;
    }

    if (!msg->isSelfMessage()) {
        // messages from CDP process
        if (strcmp(msg->getArrivalGate()->getName(), "cdpIn") == 0) {
            numReceivedUpdatesFromCDP++;
            EV_INFO << "Received " << msg << " from CDP module." << endl;
            Packet *packet = check_and_cast<Packet *>(msg);
            dispatchCDPUpdate(packet);
        }
        // messages from LLDP process
        else if (strcmp(msg->getArrivalGate()->getName(), "lldpIn") == 0) {
            numReceivedUpdatesFromLLDP++;
            EV_INFO << "Received " << msg << " from LLDP module." << endl;
            Packet *packet = check_and_cast<Packet *>(msg);
            dispatchLLDPUpdate(packet);
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

void relayUnit::broadcast(Packet *packet)
{
    EV_DETAIL << "Broadcast frame " << packet << endl;

    int inputInterfaceId = packet->getTag<InterfaceInd>()->getInterfaceId();

    int numPorts = ifTable->getNumInterfaces();
    for (int i = 0; i < numPorts; i++) {
        InterfaceEntry *ie = ifTable->getInterface(i);
        if (ie->isLoopback() || !ie->isBroadcast())
            continue;
        if (ie->getInterfaceId() != inputInterfaceId && (!isCDPAware) && (!isLLDPAware)) {
            dispatch(packet->dup(), ie->getInterfaceId());
        }
    }
    delete packet;
}

void relayUnit::handleAndDispatchFrame(Packet *packet)
{
    const auto& frame = packet->peekAtFront<EthernetMacHeader>();
    int arrivalInterfaceId = packet->getTag<InterfaceInd>()->getInterfaceId();
    InterfaceEntry *arrivalInterface = ifTable->getInterfaceById(arrivalInterfaceId);
    //Ieee8021dInterfaceData *arrivalPortData = getPortInterfaceData(arrivalGate);

    learn(frame->getSrc(), arrivalInterfaceId);

    // BPDU Handling
    if ( frame->getDest() == bridgeGroupCDPAddress) {
        EV_DETAIL << "Deliver update to the CDP module" << endl;
        deliverCDPUpdate(packet);    // deliver to the CDP module
    }
    else if ( frame->getDest() == bridgeGroupLLDPAddress) {
        EV_DETAIL << "Deliver update to the LLDP module" << endl;
        deliverLLDPUpdate(packet);    // deliver to the LLDP module
    }
    else if (!isCDPAware && !isLLDPAware) {
        EV_INFO << "The arrival port is not forwarding! Discarding it!" << endl;
        numDroppedFrames++;
        delete packet;
    }
    else if (frame->getDest().isBroadcast()) {    // broadcast address
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

                if (!isCDPAware && !isLLDPAware)
                    dispatch(packet, outInterfaceId);
                else {
                    EV_INFO << "Output port " << outInterfaceId << " is not forwarding. Discarding!" << endl;
                    numDroppedFrames++;
                    delete packet;
                }
            }
            else {
                EV_DETAIL << "Output port is same as input port, " << frame->getFullName() << " destination = " << frame->getDest() << ", discarding frame " << frame << endl;
                numDroppedFrames++;
                delete packet;
            }
        }
    }
}

void relayUnit::dispatch(Packet *packet, unsigned int portNum)
{
    const auto& frame = packet->peekAtFront<EthernetMacHeader>();
    EV_INFO << "Sending frame " << packet << " on output port " << portNum << "." << endl;

    EV_INFO << "Sending " << packet << " with destination = " << frame->getDest() << ", port = " << portNum << endl;

    numDispatchedNonUpdateFrames++;
    auto oldPacketProtocolTag = packet->removeTag<PacketProtocolTag>();
    packet->clearTags();
    auto newPacketProtocolTag = packet->addTag<PacketProtocolTag>();
    *newPacketProtocolTag = *oldPacketProtocolTag;
    delete oldPacketProtocolTag;
    packet->addTag<InterfaceReq>()->setInterfaceId(portNum);
    send(packet, "ifOut");
}

void relayUnit::learn(MacAddress srcAddr, int arrivalInterfaceId)
{
    if (!isCDPAware && !isLLDPAware)
        macTable->updateTableWithAddress(arrivalInterfaceId, srcAddr);
}

void relayUnit::dispatchLLDPUpdate(Packet *packet)
{
    const auto& lldpUpdate = packet->peekAtFront<LLDPUpdate>(); // verify packet type
    (void)lldpUpdate;       // unused variable

    // TODO: use LLCFrame       // see the inet::Ieee8021dRelay::dispatchBPDU()
    const auto& header = makeShared<EthernetMacHeader>();
    unsigned int portNum = packet->getTag<InterfaceReq>()->getInterfaceId();
    MacAddress destAddress = packet->getTag<MacAddressReq>()->getDestAddress();
    const Protocol *protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    ASSERT(protocol == &Protocol::lldp);
    int ethType = ProtocolGroup::ethertype.getProtocolNumber(protocol);

    header->setSrc(bridgeAddress);
    header->setDest(destAddress);
    header->setTypeOrLength(ethType);
    packet->insertAtFront(header);
    EtherEncap::addPaddingAndFcs(packet, FCS_DECLARED_CORRECT); //TODO add/use fcsMode parameter
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);

    EV_INFO << "Sending LLDP packet " << packet << " with destination = " << header->getDest() << ", interface = " << portNum << endl;
    numDispatchedUpdateFrames++;
    send(packet, "ifOut");
}

void relayUnit::dispatchCDPUpdate(Packet *packet)
{
    const auto& cdpUpdate = packet->peekAtFront<CDPUpdate>(); // verify packet type
    (void)cdpUpdate;       // unused variable

    const auto& header = makeShared<EthernetMacHeader>();
    unsigned int portNum = packet->getTag<InterfaceReq>()->getInterfaceId();
    MacAddress destAddress = packet->getTag<MacAddressReq>()->getDestAddress();
    const Protocol *protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    ASSERT(protocol == &Protocol::cdp);
    int ethType = ProtocolGroup::ethertype.getProtocolNumber(protocol);

    header->setSrc(bridgeAddress);
    header->setDest(destAddress);
    header->setTypeOrLength(ethType);
    packet->insertAtFront(header);
    EtherEncap::addPaddingAndFcs(packet, FCS_DECLARED_CORRECT); //TODO add/use fcsMode parameter
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);

    EV_INFO << "Sending CDP packet " << packet << " with destination = " << header->getDest() << ", port = " << portNum << endl;
    numDispatchedUpdateFrames++;
    send(packet, "ifOut");
}

void relayUnit::deliverLLDPUpdate(Packet *packet)
{
    auto eth = EtherEncap::decapsulateMacHeader(packet);
    ASSERT(isEth2Header(*eth));    //TODO use LLC header
    //ASSERT(isIeee8023Header(*eth));

    const auto& lldpUpdate = packet->peekAtFront<LLDPUpdate>();

    EV_INFO << "Sending LLDP frame " << lldpUpdate << " to the LLDP module" << endl;
    numDeliveredUpdatesToLLDP++;
    send(packet, "lldpOut");
}

void relayUnit::deliverCDPUpdate(Packet *packet)
{
    auto eth = EtherEncap::decapsulateMacHeader(packet);
    ASSERT(isEth2Header(*eth));    //TODO use LLC header
    //ASSERT(isIeee8023Header(*eth));

    const auto& cdpUpdate = packet->peekAtFront<CDPUpdate>();

    EV_INFO << "Sending CDP frame " << cdpUpdate << " to the CDP module" << endl;
    numDeliveredUpdatesToCDP++;
    send(packet, "cdpOut");
}

void relayUnit::start()
{
    isOperational = true;

    ie = chooseInterface();
    if (ie)
        bridgeAddress = ie->getMacAddress(); // get the bridge's MAC address
    else
        throw cRuntimeError("No non-loopback interface found!");

    macTable->clearTable();
}

void relayUnit::stop()
{
    isOperational = false;

    macTable->clearTable();
    ie = nullptr;
}

InterfaceEntry *relayUnit::chooseInterface()
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

void relayUnit::finish()
{
    recordScalar("number of received BPDUs from STP module", numReceivedUpdatesFromCDP);
    recordScalar("number of received frames from network (including BPDUs)", numReceivedNetworkFrames);
    recordScalar("number of dropped frames (including BPDUs)", numDroppedFrames);
    recordScalar("number of delivered BPDUs to the STP module", numDeliveredUpdatesToCDP);
    recordScalar("number of dispatched BPDU frames to the network", numDispatchedUpdateFrames);
    recordScalar("number of dispatched non-BDPU frames to the network", numDispatchedNonUpdateFrames);
}

bool relayUnit::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
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

