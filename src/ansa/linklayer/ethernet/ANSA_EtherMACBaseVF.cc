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

#include "ANSA_EtherMACBaseVF.h"

#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "ansa/networklayer/common/ANSA_InterfaceEntry.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/INETUtils.h"
namespace inet{
void ANSA_EtherMACBaseVF::initialize(int stage)
{
    connectionColoring = par("connectionColoring");

    MACBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        physInGate = gate("phys$i");
        physOutGate = gate("phys$o");
        upperLayerInGate = gate("upperLayerIn");
        transmissionChannel = nullptr;
        curTxFrame = nullptr;

        initializeFlags();

        initializeMACAddress();
        initializeStatistics();

        lastTxFinishTime = -1.0;    // not equals with current simtime.

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
    else if (stage == INITSTAGE_LINK_LAYER) {
        registerInterface();    // needs MAC address    //FIXME why not called in MACBase::initialize()?
        initializeQueueModule();
        readChannelParameters(true);
    }
}

void ANSA_EtherMACBaseVF::registerInterface()
{
    interfaceEntry = new ANSA_InterfaceEntry(this);

    // interface name: NIC module's name without special characters ([])
    interfaceEntry->setName(utils::stripnonalnum(getParentModule()->getFullName()).c_str());

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
    cModule *containingModule = findContainingNode(this);
    IInterfaceTable *ift = dynamic_cast<IInterfaceTable *>(containingModule->getSubmodule("interfaceTable"));

    if (ift)
        ift->addInterface(interfaceEntry);
}


}//namespace inet
