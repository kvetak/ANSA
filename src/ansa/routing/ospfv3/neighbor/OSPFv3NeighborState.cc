//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#include "ansa/routing/ospfv3/neighbor/OSPFv3NeighborState.h"
#include "ansa/routing/ospfv3/neighbor/OSPFv3Neighbor.h"
//#include "ansa/routing/ospfv3/process/OSPFv3Area.h"
//#include "ansa/routing/ospfv3/interface/OSPFv3Interface.h"
//#include "ansa/routing/ospfv3/process/OSPFv3Process.h"

namespace inet {

void OSPFv3NeighborState::changeState(OSPFv3Neighbor *neighbor, OSPFv3NeighborState *newState, OSPFv3NeighborState *currentState)
{//TODO
    OSPFv3Neighbor::OSPFv3NeighborStateType oldState = currentState->getState();
    OSPFv3Neighbor::OSPFv3NeighborStateType nextState = newState->getState();
    bool shouldRebuildRoutingTable = false;

    EV_DEBUG << "Changing neighbor state from " << currentState->getNeighborStateString() << " to " << newState->getNeighborStateString() << "\n";

    neighbor->changeState(newState, currentState);

//    OSPFv3RouterLSA* routerLSA = neighbor->getInterface()->getArea()->originateRouterLSA();
//    if ((oldState == OSPFv3Neighbor::FULL_STATE) || (nextState == OSPFv3Neighbor::FULL_STATE)) {
//        IPv4Address routerID = neighbor->getContainingInterface()->getContainingArea()->getContainingInstance()->getContainingProcess()->getRouterID();
        //OSPFv3RouterLSA *routerLSA = neighbor->getInterface()->getArea()->findRouterLSA(routerID); //TODO findRouterLSA

//        if (routerLSA != nullptr) {
//            long sequenceNumber = routerLSA->getHeader().getLsSequenceNumber();
//            if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
//                routerLSA->getHeader().setLsAge(MAX_AGE);
//                neighbor->getInterface()->getArea()->floodLSA(routerLSA);
//                routerLSA->incrementInstallTime();
//            }
//            else {
//                RouterLSA *newLSA = neighbor->getInterface()->getArea()->originateRouterLSA();
//
//                newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
//                shouldRebuildRoutingTable |= routerLSA->update(newLSA);
//                delete newLSA;
//
//                neighbor->getInterface()->getArea()->floodLSA(routerLSA);
//            }
//        }
//
//        if (neighbor->getInterface()->getState() == Interface::DESIGNATED_ROUTER_STATE) {
//            NetworkLSA *networkLSA = neighbor->getInterface()->getArea()->findNetworkLSA(neighbor->getInterface()->getAddressRange().address);
//
//            if (networkLSA != nullptr) {
//                long sequenceNumber = networkLSA->getHeader().getLsSequenceNumber();
//                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
//                    networkLSA->getHeader().setLsAge(MAX_AGE);
//                    neighbor->getInterface()->getArea()->floodLSA(networkLSA);
//                    networkLSA->incrementInstallTime();
//                }
//                else {
//                    NetworkLSA *newLSA = neighbor->getInterface()->getArea()->originateNetworkLSA(neighbor->getInterface());
//
//                    if (newLSA != nullptr) {
//                        newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
//                        shouldRebuildRoutingTable |= networkLSA->update(newLSA);
//                        delete newLSA;
//                    }
//                    else {    // no neighbors on the network -> old NetworkLSA must be flushed
//                        networkLSA->getHeader().setLsAge(MAX_AGE);
//                        networkLSA->incrementInstallTime();
//                    }
//
//                    neighbor->getInterface()->getArea()->floodLSA(networkLSA);
//                }
//            }
//        }
//    }
//
//    if (shouldRebuildRoutingTable) {
//        neighbor->getInterface()->getArea()->getRouter()->rebuildRoutingTable();
//    }
}

} // namespace inet
