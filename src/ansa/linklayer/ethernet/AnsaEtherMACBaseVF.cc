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

#include "AnsaEtherMACBaseVF.h"

#include "EtherFrame_m.h"
#include "InterfaceEntry.h"
#include "AnsaInterfaceEntry.h"
#include "InterfaceTableAccess.h"
#include "opp_utils.h"

void AnsaEtherMACBaseVF::initialize()
{

    physInGate = gate("phys$i");
    physOutGate = gate("phys$o");
    upperLayerInGate = gate("upperLayerIn");
    transmissionChannel = NULL;
    interfaceEntry = NULL;
    curTxFrame = NULL;

    initializeFlags();

    initializeMACAddress();
    initializeQueueModule();
    initializeStatistics();

    registerInterface(); // needs MAC address

    readChannelParameters(true);

    lastTxFinishTime = -1.0; // not equals with current simtime.

    // initialize self messages
    endTxMsg = new cMessage("EndTransmission", ENDTRANSMISSION);
    endIFGMsg = new cMessage("EndIFG", ENDIFG);
    endPauseMsg = new cMessage("EndPause", ENDPAUSE);

    // initialize states
    transmitState = TX_IDLE_STATE;
    receiveState = RX_IDLE_STATE;
    WATCH(transmitState);
    WATCH(receiveState);

    // initialize pause
    pauseUnitsRequested = 0;
    WATCH(pauseUnitsRequested);

    subscribe(POST_MODEL_CHANGE, this);
}


void AnsaEtherMACBaseVF::registerInterface()
{
    interfaceEntry = new AnsaInterfaceEntry(this);

    // interface name: NIC module's name without special characters ([])
    interfaceEntry->setName(OPP_Global::stripnonalnum(getParentModule()->getFullName()).c_str());

    // generate a link-layer address to be used as interface token for IPv6
    interfaceEntry->setMACAddress(address);
    interfaceEntry->setInterfaceToken(address.formInterfaceIdentifier());
    //InterfaceToken token(0, simulation.getUniqueNumber(), 64);
    //interfaceEntry->setInterfaceToken(token);

    // MTU: typical values are 576 (Internet de facto), 1500 (Ethernet-friendly),
    // 4000 (on some point-to-point links), 4470 (Cisco routers default, FDDI compatible)
    interfaceEntry->setMtu(par("mtu").longValue());

    // capabilities
    interfaceEntry->setMulticast(true);
    interfaceEntry->setBroadcast(true);

    // add
    IInterfaceTable *ift = InterfaceTableAccess().getIfExists();

    if (ift)
        ift->addInterface(interfaceEntry);
}

bool AnsaEtherMACBaseVF::dropFrameNotForUs(EtherFrame *frame)
{
    // Current ethernet mac implementation does not support the configuration of multicast
    // ethernet address groups. We rather accept all multicast frames (just like they were
    // broadcasts) and pass it up to the higher layer where they will be dropped
    // if not needed.
    //
    // PAUSE frames must be handled a bit differently because they are processed at
    // this level. Multicast PAUSE frames should not be processed unless they have a
    // destination of MULTICAST_PAUSE_ADDRESS. We drop all PAUSE frames that have a
    // different muticast destination. (Note: Would the multicast ethernet addressing
    // implemented, we could also process the PAUSE frames destined to any of our
    // multicast adresses)
    // All NON-PAUSE frames must be passed to the upper layer if the interface is
    // in promiscuous mode.

    if (frame->getDest().equals(address))
        return false;

    if (frame->getDest().isBroadcast())
        return false;

    bool isPause = (dynamic_cast<EtherPauseFrame *>(frame) != NULL);

    if (!isPause && (promiscuous || frame->getDest().isMulticast()))
        return false;

    if (isPause && frame->getDest().equals(MACAddress::MULTICAST_PAUSE_ADDRESS))
        return false;

    if ((dynamic_cast<AnsaInterfaceEntry *>(interfaceEntry))->hasMacAddress(frame->getDest()))
        return false;

    EV << "Frame `" << frame->getName() <<"' not destined to us, discarding\n";
    numDroppedNotForUs++;
    emit(dropPkNotForUsSignal, frame);
    delete frame;
    return true;
}
