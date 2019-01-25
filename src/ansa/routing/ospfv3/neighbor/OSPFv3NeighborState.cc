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

    // NEROZUMIEM , to akoze vytvorim cez neigbora jeho RouterLSA  a potom naplanujem
    // cez jeho intf floodLSA ?
    if ((oldState == OSPFv3Neighbor::FULL_STATE) || (nextState == OSPFv3Neighbor::FULL_STATE)) {
        IPv4Address routerID = neighbor->getInterface()->getArea()->getInstance()->getProcess()->getRouterID();
//        IPv4Address routerID = neighbor->getContainingInterface()->getContainingArea()->getContainingInstance()->getContainingProcess()->getRouterID();
        RouterLSA *routerLSA = neighbor->getInterface()->getArea()->findRouterLSA(routerID);

        if (routerLSA != nullptr) {
            long sequenceNumber = routerLSA->getHeader().getLsaSequenceNumber();
            if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                routerLSA->getHeader().setLsaAge(MAX_AGE);
                neighbor->getInterface()->getArea()->floodLSA(routerLSA);
                routerLSA->incrementInstallTime();
            }
            else {
                RouterLSA *newLSA = neighbor->getInterface()->getArea()->originateRouterLSA();

                newLSA->getHeader().setLsaSequenceNumber(sequenceNumber + 1);
//                shouldRebuildRoutingTable |= routerLSA->update(newLSA);
                shouldRebuildRoutingTable |= neighbor->getInterface()->getArea()->updateRouterLSA(routerLSA, newLSA);
                if (shouldRebuildRoutingTable)
                    neighbor->getInterface()->getArea()->setSpfTreeRoot(routerLSA);
                delete newLSA;

                neighbor->getInterface()->getArea()->floodLSA(routerLSA);
            }
        }

        // FIXME: Sposobuje, za v sieti 2 routerov v stave kedy prejdu do FULLstate a podmienka je true
        // ma DR NetworkLSAList.size() = 2 , napriek tomu, ze v DB ma NetworkLSA len jedno
        if (neighbor->getInterface()->getState() == OSPFv3Interface::INTERFACE_STATE_DESIGNATED) {
            // Link State ID for NETWORK LSA is Interface Index - FALSE
            // LG - JE TO INTERFACE ID. NIE INDEX !
            NetworkLSA *networkLSA = neighbor->getInterface()->getArea()->findNetworkLSAByLSID(
                    IPv4Address(neighbor->getInterface()->getInterfaceId()));

            if (networkLSA != nullptr) {
                long sequenceNumber = networkLSA->getHeader().getLsaSequenceNumber();
                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                    networkLSA->getHeader().setLsaAge(MAX_AGE);
                    neighbor->getInterface()->getArea()->floodLSA(networkLSA);
                    networkLSA->incrementInstallTime();
                }
                else {
                    NetworkLSA *newLSA = neighbor->getInterface()->getArea()->originateNetworkLSA(neighbor->getInterface());

                    if (newLSA != nullptr) {
                        newLSA->getHeader().setLsaSequenceNumber(sequenceNumber + 1);
                        shouldRebuildRoutingTable |= neighbor->getInterface()->getArea()->updateNetworkLSA(networkLSA, newLSA);
                        delete newLSA;
                    }
                    else {    // no neighbors on the network -> old NetworkLSA must be flushed
                        networkLSA->getHeader().setLsaAge(MAX_AGE);
                        networkLSA->incrementInstallTime();
                    }
                    std::cout << "neighbor router ID = " << neighbor->getInterface()->getArea()->getInstance()->getProcess()->getRouterID() << endl;
                                std::cout << "neighbor NetworkLSA getCount = " << neighbor->getInterface()->getArea()->getNetworkLSACount() << endl;

                    neighbor->getInterface()->getArea()->floodLSA(networkLSA);
                }
            }
        }
    }

    if (shouldRebuildRoutingTable) {
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->rebuildRoutingTable();
    }
}

} // namespace inet
