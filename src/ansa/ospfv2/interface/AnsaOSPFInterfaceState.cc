#include "AnsaOSPFInterfaceState.h"
#include "AnsaOSPFInterface.h"
#include "AnsaOSPFArea.h"
#include "AnsaOSPFRouter.h"
#include "AnsaOSPFInterfaceStateDesignatedRouter.h"
#include "AnsaOSPFInterfaceStateNotDesignatedRouter.h"
#include "AnsaOSPFInterfaceStateBackup.h"
#include <map>

void AnsaOSPF::InterfaceState::ChangeState(AnsaOSPF::Interface* intf, AnsaOSPF::InterfaceState* newState, AnsaOSPF::InterfaceState* currentState)
{
    AnsaOSPF::Interface::InterfaceStateType oldState            = currentState->GetState();
    AnsaOSPF::Interface::InterfaceStateType nextState           = newState->GetState();
    AnsaOSPF::Interface::OSPFInterfaceType  intfType            = intf->GetType();
    bool                                rebuildRoutingTable = false;

    intf->ChangeState(newState, currentState);

    if ((oldState == AnsaOSPF::Interface::DownState) ||
        (nextState == AnsaOSPF::Interface::DownState) ||
        (oldState == AnsaOSPF::Interface::LoopbackState) ||
        (nextState == AnsaOSPF::Interface::LoopbackState) ||
        (oldState == AnsaOSPF::Interface::DesignatedRouterState) ||
        (nextState == AnsaOSPF::Interface::DesignatedRouterState) ||
        ((intfType == AnsaOSPF::Interface::PointToPoint) &&
         ((oldState == AnsaOSPF::Interface::PointToPointState) ||
          (nextState == AnsaOSPF::Interface::PointToPointState))) ||
        (((intfType == AnsaOSPF::Interface::Broadcast) ||
          (intfType == AnsaOSPF::Interface::NBMA)) &&
         ((oldState == AnsaOSPF::Interface::WaitingState) ||
          (nextState == AnsaOSPF::Interface::WaitingState))))
    {
        AnsaOSPF::RouterLSA* routerLSA = intf->GetArea()->FindRouterLSA(intf->GetArea()->GetRouter()->GetRouterID());

        if (routerLSA != NULL) {
            long sequenceNumber = routerLSA->getHeader().getLsSequenceNumber();
            if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                routerLSA->getHeader().setLsAge(MAX_AGE);
                intf->GetArea()->FloodLSA(routerLSA);
                routerLSA->IncrementInstallTime();
            } else {
                AnsaOSPF::RouterLSA* newLSA = intf->GetArea()->OriginateRouterLSA();

                newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                newLSA->getHeader().setLsChecksum(0);    // TODO: calculate correct LS checksum
                rebuildRoutingTable |= routerLSA->Update(newLSA);
                delete newLSA;

                intf->GetArea()->FloodLSA(routerLSA);
            }
        } else {  // (lsa == NULL) -> This must be the first time any interface is up...
            AnsaOSPF::RouterLSA* newLSA = intf->GetArea()->OriginateRouterLSA();

            rebuildRoutingTable |= intf->GetArea()->InstallRouterLSA(newLSA);

            routerLSA = intf->GetArea()->FindRouterLSA(intf->GetArea()->GetRouter()->GetRouterID());

            intf->GetArea()->SetSPFTreeRoot(routerLSA);
            intf->GetArea()->FloodLSA(newLSA);
            delete newLSA;
        }
    }

    if (nextState == AnsaOSPF::Interface::DesignatedRouterState) {
        AnsaOSPF::NetworkLSA* newLSA = intf->GetArea()->OriginateNetworkLSA(intf);
        if (newLSA != NULL) {
            rebuildRoutingTable |= intf->GetArea()->InstallNetworkLSA(newLSA);

            intf->GetArea()->FloodLSA(newLSA);
            delete newLSA;
        } else {    // no neighbors on the network -> old NetworkLSA must be flushed
            AnsaOSPF::NetworkLSA* oldLSA = intf->GetArea()->FindNetworkLSA(ULongFromIPv4Address(intf->GetAddressRange().address));

            if (oldLSA != NULL) {
                oldLSA->getHeader().setLsAge(MAX_AGE);
                intf->GetArea()->FloodLSA(oldLSA);
                oldLSA->IncrementInstallTime();
            }
        }
    }

    if (oldState == AnsaOSPF::Interface::DesignatedRouterState) {
        AnsaOSPF::NetworkLSA* networkLSA = intf->GetArea()->FindNetworkLSA(ULongFromIPv4Address(intf->GetAddressRange().address));

        if (networkLSA != NULL) {
            networkLSA->getHeader().setLsAge(MAX_AGE);
            intf->GetArea()->FloodLSA(networkLSA);
            networkLSA->IncrementInstallTime();
        }
    }

    if (rebuildRoutingTable) {
        intf->GetArea()->GetRouter()->RebuildRoutingTable();
    }
}

void AnsaOSPF::InterfaceState::CalculateDesignatedRouter(AnsaOSPF::Interface* intf)
{
    AnsaOSPF::RouterID           routerID                = intf->parentArea->GetRouter()->GetRouterID();
    AnsaOSPF::DesignatedRouterID currentDesignatedRouter = intf->designatedRouter;
    AnsaOSPF::DesignatedRouterID currentBackupRouter     = intf->backupDesignatedRouter;

    unsigned int             neighborCount           = intf->neighboringRouters.size();
    unsigned char            repeatCount             = 0;
    unsigned int             i;

    AnsaOSPF::DesignatedRouterID declaredBackup;
    unsigned char            declaredBackupPriority;
    AnsaOSPF::RouterID           declaredBackupID;
    bool                     backupDeclared;

    AnsaOSPF::DesignatedRouterID declaredDesignatedRouter;
    unsigned char            declaredDesignatedRouterPriority;
    AnsaOSPF::RouterID           declaredDesignatedRouterID;
    bool                     designatedRouterDeclared;

    do {
        // calculating backup designated router
        declaredBackup = AnsaOSPF::NullDesignatedRouterID;
        declaredBackupPriority = 0;
        declaredBackupID = AnsaOSPF::NullRouterID;
        backupDeclared = false;

        AnsaOSPF::DesignatedRouterID highestRouter                 = AnsaOSPF::NullDesignatedRouterID;
        unsigned char            highestPriority               = 0;
        AnsaOSPF::RouterID           highestID                     = AnsaOSPF::NullRouterID;

        for (i = 0; i < neighborCount; i++) {
            AnsaOSPF::Neighbor* neighbor         = intf->neighboringRouters[i];
            unsigned char   neighborPriority = neighbor->GetPriority();

            if (neighbor->GetState() < AnsaOSPF::Neighbor::TwoWayState) {
                continue;
            }
            if (neighborPriority == 0) {
                continue;
            }

            AnsaOSPF::RouterID           neighborID                      = neighbor->GetNeighborID();
            AnsaOSPF::DesignatedRouterID neighborsDesignatedRouter       = neighbor->GetDesignatedRouter();
            AnsaOSPF::DesignatedRouterID neighborsBackupDesignatedRouter = neighbor->GetBackupDesignatedRouter();

            if (neighborsDesignatedRouter.routerID != neighborID) {
                if (neighborsBackupDesignatedRouter.routerID == neighborID) {
                    if ((neighborPriority > declaredBackupPriority) ||
                        ((neighborPriority == declaredBackupPriority) &&
                         (neighborID > declaredBackupID)))
                    {
                        declaredBackup = neighborsBackupDesignatedRouter;
                        declaredBackupPriority = neighborPriority;
                        declaredBackupID = neighborID;
                        backupDeclared = true;
                    }
                }
                if (!backupDeclared) {
                    if ((neighborPriority > highestPriority) ||
                        ((neighborPriority == highestPriority) &&
                         (neighborID > highestID)))
                    {
                        highestRouter.routerID = neighborID;
                        highestRouter.ipInterfaceAddress = neighbor->GetAddress();
                        highestPriority = neighborPriority;
                        highestID = neighborID;
                    }
                }
            }
        }
        // also include the router itself in the calculations
        if (intf->routerPriority > 0) {
            if (currentDesignatedRouter.routerID != routerID) {
                if (currentBackupRouter.routerID == routerID) {
                    if ((intf->routerPriority > declaredBackupPriority) ||
                        ((intf->routerPriority == declaredBackupPriority) &&
                         (routerID > declaredBackupID)))
                    {
                        declaredBackup.routerID = routerID;
                        declaredBackup.ipInterfaceAddress = intf->interfaceAddressRange.address;
                        declaredBackupPriority = intf->routerPriority;
                        declaredBackupID = routerID;
                        backupDeclared = true;
                    }

                }
                if (!backupDeclared) {
                    if ((intf->routerPriority > highestPriority) ||
                        ((intf->routerPriority == highestPriority) &&
                         (routerID > highestID)))
                    {
                        declaredBackup.routerID = routerID;
                        declaredBackup.ipInterfaceAddress = intf->interfaceAddressRange.address;
                        declaredBackupPriority = intf->routerPriority;
                        declaredBackupID = routerID;
                        backupDeclared = true;
                    } else {
                        declaredBackup = highestRouter;
                        declaredBackupPriority = highestPriority;
                        declaredBackupID = highestID;
                        backupDeclared = true;
                    }
                }
            }
        }

        // calculating backup designated router
        declaredDesignatedRouter = AnsaOSPF::NullDesignatedRouterID;
        declaredDesignatedRouterPriority = 0;
        declaredDesignatedRouterID = AnsaOSPF::NullRouterID;
        designatedRouterDeclared = false;

        for (i = 0; i < neighborCount; i++) {
            AnsaOSPF::Neighbor* neighbor         = intf->neighboringRouters[i];
            unsigned char   neighborPriority = neighbor->GetPriority();

            if (neighbor->GetState() < AnsaOSPF::Neighbor::TwoWayState) {
                continue;
            }
            if (neighborPriority == 0) {
                continue;
            }

            AnsaOSPF::RouterID           neighborID                      = neighbor->GetNeighborID();
            AnsaOSPF::DesignatedRouterID neighborsDesignatedRouter       = neighbor->GetDesignatedRouter();
            AnsaOSPF::DesignatedRouterID neighborsBackupDesignatedRouter = neighbor->GetBackupDesignatedRouter();

            if (neighborsDesignatedRouter.routerID == neighborID) {
                if ((neighborPriority > declaredDesignatedRouterPriority) ||
                    ((neighborPriority == declaredDesignatedRouterPriority) &&
                     (neighborID > declaredDesignatedRouterID)))
                {
                    declaredDesignatedRouter = neighborsDesignatedRouter;
                    declaredDesignatedRouterPriority = neighborPriority;
                    declaredDesignatedRouterID = neighborID;
                    designatedRouterDeclared = true;
                }
            }
        }
        // also include the router itself in the calculations
        if (intf->routerPriority > 0) {
            if (currentDesignatedRouter.routerID == routerID) {
                if ((intf->routerPriority > declaredDesignatedRouterPriority) ||
                    ((intf->routerPriority == declaredDesignatedRouterPriority) &&
                     (routerID > declaredDesignatedRouterID)))
                {
                    declaredDesignatedRouter.routerID = routerID;
                    declaredDesignatedRouter.ipInterfaceAddress = intf->interfaceAddressRange.address;
                    declaredDesignatedRouterPriority = intf->routerPriority;
                    declaredDesignatedRouterID = routerID;
                    designatedRouterDeclared = true;
                }

            }
        }
        if (!designatedRouterDeclared) {
            declaredDesignatedRouter = declaredBackup;
            declaredDesignatedRouterPriority = declaredBackupPriority;
            declaredDesignatedRouterID = declaredBackupID;
            designatedRouterDeclared = true;
        }

        // if the router is any kind of DR or is no longer one of them, then repeat
        if (((declaredDesignatedRouter.routerID != AnsaOSPF::NullRouterID) &&
             ((currentDesignatedRouter.routerID == routerID) &&
              (declaredDesignatedRouter.routerID != routerID)) ||
             ((currentDesignatedRouter.routerID != routerID) &&
              (declaredDesignatedRouter.routerID == routerID))) ||
            ((declaredBackup.routerID != AnsaOSPF::NullRouterID) &&
             ((currentBackupRouter.routerID == routerID) &&
              (declaredBackup.routerID != routerID)) ||
             ((currentBackupRouter.routerID != routerID) &&
              (declaredBackup.routerID == routerID))))
        {
            currentDesignatedRouter = declaredDesignatedRouter;
            currentBackupRouter = declaredBackup;
            repeatCount++;
        } else {
            repeatCount += 2;
        }

    } while (repeatCount < 2);

    AnsaOSPF::RouterID routersOldDesignatedRouterID = intf->designatedRouter.routerID;
    AnsaOSPF::RouterID routersOldBackupID           = intf->backupDesignatedRouter.routerID;

    intf->designatedRouter = declaredDesignatedRouter;
    intf->backupDesignatedRouter = declaredBackup;

    bool wasBackupDesignatedRouter = (routersOldBackupID == routerID);
    bool wasDesignatedRouter       = (routersOldDesignatedRouterID == routerID);
    bool wasOther                  = (intf->GetState() == AnsaOSPF::Interface::NotDesignatedRouterState);
    bool wasWaiting                = (!wasBackupDesignatedRouter && !wasDesignatedRouter && !wasOther);
    bool isBackupDesignatedRouter  = (declaredBackup.routerID == routerID);
    bool isDesignatedRouter        = (declaredDesignatedRouter.routerID == routerID);
    bool isOther                   = (!isBackupDesignatedRouter && !isDesignatedRouter);

    if (wasBackupDesignatedRouter) {
        if (isDesignatedRouter) {
            ChangeState(intf, new AnsaOSPF::InterfaceStateDesignatedRouter, this);
        }
        if (isOther) {
            ChangeState(intf, new AnsaOSPF::InterfaceStateNotDesignatedRouter, this);
        }
    }
    if (wasDesignatedRouter) {
        if (isBackupDesignatedRouter) {
            ChangeState(intf, new AnsaOSPF::InterfaceStateBackup, this);
        }
        if (isOther) {
            ChangeState(intf, new AnsaOSPF::InterfaceStateNotDesignatedRouter, this);
        }
    }
    if (wasOther) {
        if (isDesignatedRouter) {
            ChangeState(intf, new AnsaOSPF::InterfaceStateDesignatedRouter, this);
        }
        if (isBackupDesignatedRouter) {
            ChangeState(intf, new AnsaOSPF::InterfaceStateBackup, this);
        }
    }
    if (wasWaiting) {
        if (isDesignatedRouter) {
            ChangeState(intf, new AnsaOSPF::InterfaceStateDesignatedRouter, this);
        }
        if (isBackupDesignatedRouter) {
            ChangeState(intf, new AnsaOSPF::InterfaceStateBackup, this);
        }
        if (isOther) {
            ChangeState(intf, new AnsaOSPF::InterfaceStateNotDesignatedRouter, this);
        }
    }

    for (i = 0; i < neighborCount; i++) {
        if ((intf->interfaceType == AnsaOSPF::Interface::NBMA) &&
            ((!wasBackupDesignatedRouter && isBackupDesignatedRouter) ||
             (!wasDesignatedRouter && isDesignatedRouter)))
        {
            if (intf->neighboringRouters[i]->GetPriority() == 0) {
                intf->neighboringRouters[i]->ProcessEvent(AnsaOSPF::Neighbor::Start);
            }
        }
        if ((declaredDesignatedRouter.routerID != routersOldDesignatedRouterID) ||
            (declaredBackup.routerID != routersOldBackupID))
        {
            if (intf->neighboringRouters[i]->GetState() >= AnsaOSPF::Neighbor::TwoWayState) {
                intf->neighboringRouters[i]->ProcessEvent(AnsaOSPF::Neighbor::IsAdjacencyOK);
            }
        }
    }
}
