#include "ansa/routing/ospfv3/interface/OSPFv3InterfaceState.h"
#include "ansa/routing/ospfv3/interface/OSPFv3InterfaceStateDR.h"
#include "ansa/routing/ospfv3/interface/OSPFv3InterfaceStateDROther.h"
#include "ansa/routing/ospfv3/interface/OSPFv3InterfaceStateBackup.h"

namespace inet{
void OSPFv3InterfaceState::changeState(OSPFv3Interface *interface, OSPFv3InterfaceState *newState, OSPFv3InterfaceState *currentState)
{
    OSPFv3Interface::OSPFv3InterfaceFAState oldState = currentState->getState();
    OSPFv3Interface::OSPFv3InterfaceFAState nextState = newState->getState();
    OSPFv3Interface::OSPFv3InterfaceType intfType = interface->getType();
    bool shouldRebuildRoutingTable = false;

    interface->changeState(currentState, newState);

    //change of state -> new router LSA needs to be generated

    if (oldState == OSPFv3Interface::INTERFACE_STATE_WAITING)
    {
        OSPFv3RouterLSA* routerLSA;
        int routerLSAcount = interface->getArea()->getRouterLSACount();
        for(int i=0; i< routerLSAcount; i++) {
            routerLSA = interface->getArea()->getRouterLSA(i);
            OSPFv3LSAHeader &header = routerLSA->getHeader();
            if (header.getAdvertisingRouter() == interface->getArea()->getInstance()->getProcess()->getRouterID()) {
                EV_DEBUG << "Changing state -> deleting the old router LSA\n";
                interface->getArea()->deleteRouterLSA(i);
            }
        }

        EV_DEBUG << "Changing state -> new Router LSA\n";
        interface->getArea()->originateRouterLSA();

        if(nextState == OSPFv3Interface::INTERFACE_STATE_BACKUP ||
           nextState == OSPFv3Interface::INTERFACE_STATE_DESIGNATED ||
           nextState == OSPFv3Interface::INTERFACE_STATE_DROTHER) {
            interface->setTransitNetInt(true);//this interface is in broadcast network
        }

        interface->getArea()->installIntraAreaPrefixLSA(interface->getArea()->originateIntraAreaPrefixLSA());

        //OSPFv3RouterLSA *routerLSA = intf->getArea()->getRouterLSA(intf->getArea()->getRouter()->getRouterID());

//        if (routerLSA != nullptr) {
//            long sequenceNumber = routerLSA->getHeader().getLsSequenceNumber();
//            if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
//                routerLSA->getHeader().setLsAge(MAX_AGE);
//                intf->getArea()->floodLSA(routerLSA);
//                routerLSA->incrementInstallTime();
//            }
//            else {
//                RouterLSA *newLSA = intf->getArea()->originateRouterLSA();
//
//                newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
//                shouldRebuildRoutingTable |= routerLSA->update(newLSA);
//                delete newLSA;
//
//                intf->getArea()->floodLSA(routerLSA);
//            }
//        }
//        else {    // (lsa == nullptr) -> This must be the first time any interface is up...
//            RouterLSA *newLSA = intf->getArea()->originateRouterLSA();
//
//            shouldRebuildRoutingTable |= intf->getArea()->installRouterLSA(newLSA);
//
//            routerLSA = intf->getArea()->findRouterLSA(intf->getArea()->getRouter()->getRouterID());
//
//            intf->getArea()->setSPFTreeRoot(routerLSA);
//            intf->getArea()->floodLSA(newLSA);
//            delete newLSA;
//        }
    }

    if (nextState == OSPFv3Interface::INTERFACE_STATE_DESIGNATED) {
        interface->getArea()->originateNetworkLSA(interface);
//        NetworkLSA *newLSA = interface->getArea()->originateNetworkLSA(intf);
//        if (newLSA != nullptr) {
//            shouldRebuildRoutingTable |= interface->getArea()->installNetworkLSA(newLSA);
//
//            interface->getArea()->floodLSA(newLSA);
//            delete newLSA;
//        }
//        else {    // no neighbors on the network -> old NetworkLSA must be flushed
//            NetworkLSA *oldLSA = interface->getArea()->findNetworkLSA(intf->getAddressRange().address);
//
//            if (oldLSA != nullptr) {
//                oldLSA->getHeader().setLsAge(MAX_AGE);
//                intf->getArea()->floodLSA(oldLSA);
//                oldLSA->incrementInstallTime();
//            }
//        }
    }

    if (oldState == OSPFv3Interface::INTERFACE_STATE_DESIGNATED) {
//        NetworkLSA *networkLSA = interface->getArea()->findNetworkLSA(intf->getAddressRange().address);
//
//        if (networkLSA != nullptr) {
//            networkLSA->getHeader().setLsAge(MAX_AGE);
//            intf->getArea()->floodLSA(networkLSA);
//            networkLSA->incrementInstallTime();
//        }
    }

    if (shouldRebuildRoutingTable) {
//        intf->getArea()->getRouter()->rebuildRoutingTable();
    }
}

void OSPFv3InterfaceState::calculateDesignatedRouter(OSPFv3Interface *intf){
    IPv4Address routerID = intf->getArea()->getInstance()->getProcess()->getRouterID();
    IPv4Address currentDesignatedRouter = intf->getDesignatedID();
    IPv4Address currentBackupRouter = intf->getBackupID();
    EV_DEBUG << "Calculating the designated router, currentDesignated:" << currentDesignatedRouter << ", current backup: " << currentBackupRouter << "\n";

    unsigned int neighborCount = intf->getNeighborCount();
    unsigned char repeatCount = 0;
    unsigned int i;

    IPv6Address declaredBackupIP;
    int declaredBackupPriority;
    IPv4Address declaredBackupID;
    bool backupDeclared;

    IPv6Address declaredDesignatedRouterIP;
    int declaredDesignatedRouterPriority;
    IPv4Address declaredDesignatedRouterID;
    bool designatedRouterDeclared;

    do {
        EV_DEBUG <<  "ELECTION DO in router " << routerID << "\n";
        // calculating backup designated router
        declaredBackupIP = IPv6Address::UNSPECIFIED_ADDRESS;
        declaredBackupPriority = 0;
        declaredBackupID = NULL_IPV4ADDRESS;
        backupDeclared = false;

        IPv4Address highestRouter = NULL_IPV4ADDRESS;
        IPv6Address highestRouterIP = IPv6Address::UNSPECIFIED_ADDRESS;
        int highestPriority = 0;

        for (i = 0; i < neighborCount; i++) {

            OSPFv3Neighbor *neighbor = intf->getNeighbor(i);
            int neighborPriority = neighbor->getNeighborPriority();

            if (neighbor->getState() < OSPFv3Neighbor::TWOWAY_STATE) {
                continue;
            }
            if (neighborPriority == 0) {
                continue;
            }

            IPv4Address neighborID = neighbor->getNeighborID();
            IPv4Address neighborsDesignatedRouterID = neighbor->getNeighborsDR();
            IPv4Address neighborsBackupDesignatedRouterID = neighbor->getNeighborsBackup();

            EV_DEBUG << "Neighbors DR: " << neighborsDesignatedRouterID << ", neighbors backup: " << neighborsBackupDesignatedRouterID << "\n";
            if (neighborsDesignatedRouterID != neighborID) {
                EV_DEBUG << "Router " << routerID << " trying backup on neighbor " << neighborID << "\n";
                if (neighborsBackupDesignatedRouterID == neighborID) {
                    if ((neighborPriority > declaredBackupPriority) ||
                            ((neighborPriority == declaredBackupPriority) &&
                                    (neighborID > declaredBackupID)))
                    {
                        declaredBackupID = neighborsBackupDesignatedRouterID;
                        declaredBackupPriority = neighborPriority;
                        declaredBackupIP = neighbor->getNeighborIP();
                        backupDeclared = true;
                    }
                }
                if (!backupDeclared) {
                    if ((neighborPriority > highestPriority) ||
                            ((neighborPriority == highestPriority) &&
                                    (neighborID > highestRouter)))
                    {
                        highestRouter = neighborID;
                        highestRouterIP = neighbor->getNeighborIP();
                        highestPriority = neighborPriority;
                    }
                }
            }
            EV_DEBUG << "Router " << routerID << " declared backup is " << declaredBackupID << "\n";
        }

        // also include the router itself in the calculations
        if (intf->routerPriority > 0) {
            if (currentDesignatedRouter != routerID) {
                if (currentBackupRouter == routerID) {
                    if ((intf->routerPriority > declaredBackupPriority) ||
                            ((intf->routerPriority == declaredBackupPriority) &&
                                    (routerID > declaredBackupID)))
                    {
                        declaredBackupID = routerID;
                        declaredBackupIP = intf->getInterfaceIP();
                        declaredBackupPriority = intf->getRouterPriority();
                        backupDeclared = true;
                    }
                }
                if (!backupDeclared) {
                    if ((intf->routerPriority > highestPriority) ||
                            ((intf->routerPriority == highestPriority) &&
                                    (routerID > highestRouter)))
                    {
                        declaredBackupID = routerID;
                        declaredBackupIP = intf->getInterfaceIP();
                        declaredBackupPriority = intf->getRouterPriority();
                        backupDeclared = true;
                    }
                    else {
                        declaredBackupID = highestRouter;
                        declaredBackupPriority = highestPriority;
                        backupDeclared = true;
                    }
                }
            }
        }
        EV_DEBUG << "Router " << routerID << " declared backup after backup round is " << declaredBackupID << "\n";
        // calculating designated router
        declaredDesignatedRouterID = NULL_IPV4ADDRESS;
        declaredDesignatedRouterPriority = 0;
        designatedRouterDeclared = false;

        for (i = 0; i < neighborCount; i++) {
            OSPFv3Neighbor *neighbor = intf->getNeighbor(i);
            unsigned short neighborPriority = neighbor->getNeighborPriority();

            if (neighbor->getState() < OSPFv3Neighbor::TWOWAY_STATE) {
                continue;
            }
            if (neighborPriority == 0) {
                continue;
            }

            IPv4Address neighborID = neighbor->getNeighborID();
            IPv4Address neighborsDesignatedRouterID = neighbor->getNeighborsDR();
            IPv4Address neighborsBackupDesignatedRouterID = neighbor->getNeighborsBackup();

            if (neighborsDesignatedRouterID == neighborID) {
                if ((neighborPriority > declaredDesignatedRouterPriority) ||
                        ((neighborPriority == declaredDesignatedRouterPriority) &&
                                (neighborID > declaredDesignatedRouterID)))
                {
//                    declaredDesignatedRouterID = neighborsDesignatedRouterID;
                    declaredDesignatedRouterPriority = neighborPriority;
                    declaredDesignatedRouterID = neighborID;
                    designatedRouterDeclared = true;
                }
            }
        }
        EV_DEBUG << "Router " << routerID << " declared DR after neighbors is " << declaredDesignatedRouterID << "\n";
        // also include the router itself in the calculations
        if (intf->routerPriority > 0) {
            if (currentDesignatedRouter == routerID) {
                if ((intf->routerPriority > declaredDesignatedRouterPriority) ||
                        ((intf->routerPriority == declaredDesignatedRouterPriority) &&
                                (routerID > declaredDesignatedRouterID)))
                {
                    declaredDesignatedRouterID = routerID;
                    declaredDesignatedRouterIP = intf->getInterfaceIP();
                    declaredDesignatedRouterPriority = intf->getRouterPriority();
                    designatedRouterDeclared = true;
                }
            }
        }
        if (!designatedRouterDeclared) {
            declaredDesignatedRouterID = declaredBackupID;
            declaredDesignatedRouterPriority = declaredBackupPriority;
            designatedRouterDeclared = true;
        }
        EV_DEBUG << "Router " << routerID << " declared DR after a round is " << declaredDesignatedRouterID << "\n";
        // if the router is any kind of DR or is no longer one of them, then repeat
        if (
                (
                        (declaredDesignatedRouterID != NULL_IPV4ADDRESS) &&
                        ((      //if this router is not the DR but it was before
                                (currentDesignatedRouter == routerID) &&
                                (declaredDesignatedRouterID != routerID)
                        ) ||
                                (//if this router was not the DR but now it is
                                        (currentDesignatedRouter != routerID) &&
                                        (declaredDesignatedRouterID == routerID)
                                ))
                ) ||
                (
                        (declaredBackupID != NULL_IPV4ADDRESS) &&
                        ((
                                (currentBackupRouter == routerID) &&
                                (declaredBackupID != routerID)
                        ) ||
                                (
                                        (currentBackupRouter != routerID) &&
                                        (declaredBackupID == routerID)
                                ))
                )
        )
        {
            currentDesignatedRouter = declaredDesignatedRouterID;
            currentBackupRouter = declaredBackupID;
            repeatCount++;
        }
        else {
            repeatCount += 2;
        }
    } while (repeatCount < 2);

    IPv4Address routersOldDesignatedRouterID = intf->getDesignatedID();
    IPv4Address routersOldBackupID = intf->getBackupID();

    intf->setDesignatedID(declaredDesignatedRouterID);
    intf->setBackupID(declaredBackupID);

    bool wasBackupDesignatedRouter = (routersOldBackupID == routerID);
    bool wasDesignatedRouter = (routersOldDesignatedRouterID == routerID);
    bool wasOther = (intf->getState() == OSPFv3Interface::INTERFACE_STATE_DROTHER);
    bool wasWaiting = (!wasBackupDesignatedRouter && !wasDesignatedRouter && !wasOther);
    bool isBackupDesignatedRouter = (declaredBackupID == routerID);
    bool isDesignatedRouter = (declaredDesignatedRouterID == routerID);
    bool isOther = (!isBackupDesignatedRouter && !isDesignatedRouter);

    if (wasBackupDesignatedRouter) {
        if (isDesignatedRouter) {
            changeState(intf, new OSPFv3InterfaceStateDR, this);
        }
        if (isOther) {
            changeState(intf, new OSPFv3InterfaceStateDROther, this);
        }
    }
    if (wasDesignatedRouter) {
        if (isBackupDesignatedRouter) {
            changeState(intf, new OSPFv3InterfaceStateBackup, this);
        }
        if (isOther) {
            changeState(intf, new OSPFv3InterfaceStateDROther, this);
        }
    }
    if (wasOther) {
        if (isDesignatedRouter) {
            changeState(intf, new OSPFv3InterfaceStateDR, this);
        }
        if (isBackupDesignatedRouter) {
            changeState(intf, new OSPFv3InterfaceStateBackup, this);
        }
    }
    if (wasWaiting) {
        if (isDesignatedRouter) {
            changeState(intf, new OSPFv3InterfaceStateDR, this);
        }
        if (isBackupDesignatedRouter) {
            changeState(intf, new OSPFv3InterfaceStateBackup, this);
        }
        if (isOther) {
            changeState(intf, new OSPFv3InterfaceStateDROther, this);
        }
    }

    for (i = 0; i < neighborCount; i++) {
        if ((intf->interfaceType == OSPFv3Interface::NBMA_TYPE) &&
                ((!wasBackupDesignatedRouter && isBackupDesignatedRouter) ||
                        (!wasDesignatedRouter && isDesignatedRouter)))
        {
            if (intf->getNeighbor(i)->getNeighborPriority() == 0) {
                intf->getNeighbor(i)->processEvent(OSPFv3Neighbor::START);
            }
        }
        if ((declaredDesignatedRouterID != routersOldDesignatedRouterID) ||
                (declaredBackupID != routersOldBackupID))
        {
            if (intf->getNeighbor(i)->getState() >= OSPFv3Neighbor::TWOWAY_STATE) {
                intf->getNeighbor(i)->processEvent(OSPFv3Neighbor::IS_ADJACENCY_OK);
            }
        }
    }
}
}//namespace inet
