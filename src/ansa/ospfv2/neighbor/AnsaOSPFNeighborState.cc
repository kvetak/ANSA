#include "AnsaOSPFNeighborState.h"
#include "AnsaOSPFInterface.h"
#include "AnsaOSPFArea.h"
#include "AnsaOSPFRouter.h"
#include <fstream>

void AnsaOSPF::NeighborState::ChangeState(AnsaOSPF::Neighbor* neighbor, AnsaOSPF::NeighborState* newState, AnsaOSPF::NeighborState* currentState)
{

    AnsaOSPF::Neighbor::NeighborStateType   oldState            = currentState->GetState();
    AnsaOSPF::Neighbor::NeighborStateType   nextState           = newState->GetState();
    bool                                rebuildRoutingTable = false;

    neighbor->ChangeState(newState, currentState);

    if (((oldState == AnsaOSPF::Neighbor::FullState) || (nextState == AnsaOSPF::Neighbor::FullState)) && !(neighbor->GetInterface()->IsGoingDown())) {
        AnsaOSPF::RouterID   routerID  = neighbor->GetInterface()->GetArea()->GetRouter()->GetRouterID();
        AnsaOSPF::RouterLSA* routerLSA = neighbor->GetInterface()->GetArea()->FindRouterLSA(routerID);

        if (routerLSA != NULL) {
            long sequenceNumber = routerLSA->getHeader().getLsSequenceNumber();
            if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                routerLSA->getHeader().setLsAge(MAX_AGE);
                neighbor->GetInterface()->GetArea()->FloodLSA(routerLSA);
                routerLSA->IncrementInstallTime();
            } else {
                AnsaOSPF::RouterLSA* newLSA = neighbor->GetInterface()->GetArea()->OriginateRouterLSA();

                newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                newLSA->getHeader().setLsChecksum(0);    // TODO: calculate correct LS checksum
                rebuildRoutingTable |= routerLSA->Update(newLSA);
                delete newLSA;

                neighbor->GetInterface()->GetArea()->FloodLSA(routerLSA);
            }
        }

        if (neighbor->GetInterface()->GetState() == AnsaOSPF::Interface::DesignatedRouterState) {
            AnsaOSPF::NetworkLSA* networkLSA = neighbor->GetInterface()->GetArea()->FindNetworkLSA(ULongFromIPv4Address(neighbor->GetInterface()->GetAddressRange().address));

            if (networkLSA != NULL) {
                long sequenceNumber = networkLSA->getHeader().getLsSequenceNumber();
                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                    networkLSA->getHeader().setLsAge(MAX_AGE);
                    neighbor->GetInterface()->GetArea()->FloodLSA(networkLSA);
                    networkLSA->IncrementInstallTime();
                } else {
                    AnsaOSPF::NetworkLSA* newLSA = neighbor->GetInterface()->GetArea()->OriginateNetworkLSA(neighbor->GetInterface());

                    if (newLSA != NULL) {
                        newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                        newLSA->getHeader().setLsChecksum(0);    // TODO: calculate correct LS checksum
                        rebuildRoutingTable |= networkLSA->Update(newLSA);
                        delete newLSA;
                    } else {    // no neighbors on the network -> old NetworkLSA must be flushed
                        networkLSA->getHeader().setLsAge(MAX_AGE);
                        networkLSA->IncrementInstallTime();
                    }

                    neighbor->GetInterface()->GetArea()->FloodLSA(networkLSA);
                }
            }
        }
    }

    if (rebuildRoutingTable) {
        neighbor->GetInterface()->GetArea()->GetRouter()->RebuildRoutingTable();
    }
}
