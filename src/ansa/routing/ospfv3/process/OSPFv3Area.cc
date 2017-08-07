#include "ansa/routing/ospfv3/process/OSPFv3Area.h"

namespace inet{

OSPFv3Area::OSPFv3Area(IPv4Address areaID, OSPFv3Instance* parent, OSPFv3AreaType type)
{
    this->areaID=areaID;
    this->containingInstance=parent;
    this->externalRoutingCapability = true;
    this->areaType = type;
//    EV_DEBUG << "Area Type : " << type << endl;

    WATCH_PTRVECTOR(this->interfaceList);
}

OSPFv3Area::~OSPFv3Area()
{
}

bool OSPFv3Area::hasInterface(std::string interfaceName)
{
    std::map<std::string, OSPFv3Interface*>::iterator interfaceIt = this->interfaceByName.find(interfaceName);
    if(interfaceIt == this->interfaceByName.end())
        return false;

    return true;
}//hasArea

OSPFv3Interface* OSPFv3Area::getInterfaceById(int id)
{
    std::map<int, OSPFv3Interface*>::iterator interfaceIt = this->interfaceById.find(id);
    if(interfaceIt == this->interfaceById.end())
        return nullptr;

    return interfaceIt->second;
}//getInterfaceById

OSPFv3Interface* OSPFv3Area::getInterfaceByIndex(int id)
{
    std::map<int, OSPFv3Interface*>::iterator interfaceIt = this->interfaceByIndex.find(id);
    if(interfaceIt == this->interfaceByIndex.end())
        return nullptr;

    return interfaceIt->second;
}//getInterfaceByIndex

void OSPFv3Area::addInterface(OSPFv3Interface* newInterface)
{
    this->interfaceList.push_back(newInterface);
    this->interfaceByName[newInterface->getIntName()]=newInterface;
    this->interfaceById[newInterface->getInterfaceId()]=newInterface;
}//addArea

void OSPFv3Area::init()
{
    for(auto it=this->interfaceList.begin(); it!=this->interfaceList.end(); it++)
    {
        (*it)->setInterfaceIndex(this->getInstance()->getUniqueId());
        this->interfaceByIndex[(*it)->getInterfaceIndex()]=(*it);
        (*it)->processEvent(OSPFv3Interface::INTERFACE_UP_EVENT);
    }
    this->setSpfTreeRoot(this->originateRouterLSA());

    OSPFv3IntraAreaPrefixLSA* prefixLsa = this->originateIntraAreaPrefixLSA();
    EV_DEBUG << "Creating InterAreaPrefixLSA from IntraAreaPrefixLSA\n";
    this->originateInterAreaPrefixLSA(prefixLsa, this);
    this->installIntraAreaPrefixLSA(prefixLsa);

    if((this->getAreaType() == OSPFv3AreaType::STUB) && (this->getInstance()->getAreaCount()>1))
        this->originateDefaultInterAreaPrefixLSA(this);
}

OSPFv3Interface* OSPFv3Area::findVirtualLink(IPv4Address routerID)
{
    int interfaceNum = this->interfaceList.size();
    for (int i = 0; i < interfaceNum; i++) {
        if ((interfaceList.at(i)->getType() == OSPFv3Interface::VIRTUAL_TYPE) &&
            (interfaceList.at(i)->getNeighborById(routerID) != nullptr))
        {
            return interfaceList.at(i);
        }
    }
    return nullptr;
}

OSPFv3LSAHeader* OSPFv3Area::findLSA(LSAKeyType lsaKey)
{
//    EV_DEBUG << "FIND LSA:\n";

    switch (lsaKey.LSType) {
    case ROUTER_LSA: {
//        EV_DEBUG << "looking for lsa type ROUTER\n";
        OSPFv3RouterLSA* lsa = this->getRouterLSAbyKey(lsaKey);
        if(lsa == nullptr) {
//            EV_DEBUG << "FIND LSA - nullptr returned\n";
            return nullptr;
        }
        else {
            OSPFv3LSAHeader* lsaHeader = &(lsa->getHeader());
//            EV_DEBUG << "FIND LSA - header returned\n";
            return lsaHeader;
        }
    }
    break;

    case NETWORK_LSA: {
//        EV_DEBUG << "looking for lsa type NETWORK\n";
        OSPFv3RouterLSA* lsa = this->getRouterLSAbyKey(lsaKey);
                if(lsa == nullptr) {
//                    EV_DEBUG << "FIND LSA - nullptr returned\n";
                    return nullptr;
                }
                else {
                    OSPFv3LSAHeader* lsaHeader = &(lsa->getHeader());
//                    EV_DEBUG << "FIND LSA - header returned\n";
                    return lsaHeader;
                }
        //return this->getNetworkLSAbyId(lsaKey.linkStateID);
    }
    break;

//    case SUMMARYLSA_NETWORKS_TYPE:
//    case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE: {
//        auto areaIt = areasByID.find(areaID);
//        if (areaIt != areasByID.end()) {
//            return areaIt->second->findSummaryLSA(lsaKey);
//        }
//    }
//    break;
//
//    case AS_EXTERNAL_LSA_TYPE: {
//        return findASExternalLSA(lsaKey);
//    }
//    break;
//
    default:
        //ASSERT(false);
        break;
    }
    return nullptr;
}

IPv4Address OSPFv3Area::getNewRouterLinkStateID()
{
    IPv4Address currIP = this->routerLsID;
    int newIP = currIP.getInt()+1;
    this->routerLsID = IPv4Address(newIP);
    return currIP;
}

OSPFv3RouterLSA* OSPFv3Area::originateRouterLSA()
{
    OSPFv3RouterLSA *routerLSA = new OSPFv3RouterLSA();
    OSPFv3LSAHeader& lsaHeader = routerLSA->getHeader();
    long interfaceCount = this->interfaceList.size();
    OSPFv3Options lsOptions;
    memset(&lsOptions, 0, sizeof(OSPFv3Options));

    //First set the LSA Header
    lsaHeader.setLsaAge((int)simTime().dbl());
    //The LSA Type is 0x2001
    lsaHeader.setLsaType(ROUTER_LSA);

    lsaHeader.setLinkStateID(this->getNewRouterLinkStateID());
    lsaHeader.setAdvertisingRouter(this->getInstance()->getProcess()->getRouterID());
    lsaHeader.setLsaSequenceNumber(this->getCurrentRouterSequence());
    this->incrementRouterSequence();

    if(this->getInstance()->getAreaCount()>1)
        routerLSA->setBBit(true);
    //TODO - set options

    for(int i=0; i<interfaceCount; i++)
    {
        OSPFv3Interface* intf = this->interfaceList.at(i);
        int neighborCount = intf->getNeighborCount();
        for(int j=0; j<neighborCount; j++)
        {
            OSPFv3RouterLSABody routerLSABody;
            memset(&routerLSABody, 0, sizeof(OSPFv3RouterLSABody));

            switch(intf->getType())
            {
            case OSPFv3Interface::POINTTOPOINT_TYPE:
                routerLSABody.type=POINT_TO_POINT;
                break;

            case OSPFv3Interface::BROADCAST_TYPE:
                routerLSABody.type=TRANSIT_NETWORK;
                break;

            case OSPFv3Interface::VIRTUAL_TYPE:
                routerLSABody.type=VIRTUAL_LINK;
                break;
            }

            routerLSABody.interfaceID = intf->getInterfaceIndex();
            routerLSABody.metric = 1;//TODO - correct this

            OSPFv3Neighbor* neighbor = intf->getNeighbor(j);
            routerLSABody.neighborInterfaceID = neighbor->getNeighborInterfaceID();
            routerLSABody.neighborRouterID = neighbor->getNeighborID();

            routerLSA->setRoutersArraySize(j+1);
            routerLSA->setRouters(j, routerLSABody);
        }
    }

    this->installRouterLSA(routerLSA);

    //originate Intra-Area-Prefix LSA along with any Router LSA
//    this->installIntraAreaPrefixLSA(this->originateIntraAreaPrefixLSA());


//    routerLSA->setB_AreaBorderRouter(parentRouter->getAreaCount() > 1);
//    routerLSA->setE_ASBoundaryRouter((externalRoutingCapability && parentRouter->getASBoundaryRouter()) ? true : false);
//    Area *backbone = parentRouter->getAreaByID(BACKBONE_AREAID);
//    routerLSA->setV_VirtualLinkEndpoint((backbone == nullptr) ? false : backbone->hasVirtualLink(areaID));
//
//    routerLSA->setNumberOfLinks(0);
//    routerLSA->setLinksArraySize(0);
//    for (i = 0; i < interfaceCount; i++) {
//        Interface *intf = associatedInterfaces[i];
//
//        if (intf->getState() == Interface::DOWN_STATE) {
//            continue;
//        }
//        if ((intf->getState() == Interface::LOOPBACK_STATE) &&
//            ((intf->getType() != Interface::POINTTOPOINT) ||
//             (intf->getAddressRange().address != NULL_IPV4ADDRESS)))
//        {
//            Link stubLink;
//            stubLink.setType(STUB_LINK);
//            stubLink.setLinkID(intf->getAddressRange().address);
//            stubLink.setLinkData(0xFFFFFFFF);
//            stubLink.setLinkCost(0);
//            stubLink.setNumberOfTOS(0);
//            stubLink.setTosDataArraySize(0);
//
//            unsigned short linkIndex = routerLSA->getLinksArraySize();
//            routerLSA->setLinksArraySize(linkIndex + 1);
//            routerLSA->setNumberOfLinks(linkIndex + 1);
//            routerLSA->setLinks(linkIndex, stubLink);
//        }
//        if (intf->getState() > Interface::LOOPBACK_STATE) {
//            switch (intf->getType()) {
//                case Interface::POINTTOPOINT: {
//                    Neighbor *neighbor = (intf->getNeighborCount() > 0) ? intf->getNeighbor(0) : nullptr;
//                    if (neighbor != nullptr) {
//                        if (neighbor->getState() == Neighbor::FULL_STATE) {
//                            Link link;
//                            link.setType(POINTTOPOINT_LINK);
//                            link.setLinkID(IPv4Address(neighbor->getNeighborID()));
//                            if (intf->getAddressRange().address != NULL_IPV4ADDRESS) {
//                                link.setLinkData(intf->getAddressRange().address.getInt());
//                            }
//                            else {
//                                link.setLinkData(intf->getIfIndex());
//                            }
//                            link.setLinkCost(intf->getOutputCost());
//                            link.setNumberOfTOS(0);
//                            link.setTosDataArraySize(0);
//
//                            unsigned short linkIndex = routerLSA->getLinksArraySize();
//                            routerLSA->setLinksArraySize(linkIndex + 1);
//                            routerLSA->setNumberOfLinks(linkIndex + 1);
//                            routerLSA->setLinks(linkIndex, link);
//                        }
//                        if (intf->getState() == Interface::POINTTOPOINT_STATE) {
//                            if (neighbor->getAddress() != NULL_IPV4ADDRESS) {
//                                Link stubLink;
//                                stubLink.setType(STUB_LINK);
//                                stubLink.setLinkID(neighbor->getAddress());
//                                stubLink.setLinkData(0xFFFFFFFF);
//                                stubLink.setLinkCost(intf->getOutputCost());
//                                stubLink.setNumberOfTOS(0);
//                                stubLink.setTosDataArraySize(0);
//
//                                unsigned short linkIndex = routerLSA->getLinksArraySize();
//                                routerLSA->setLinksArraySize(linkIndex + 1);
//                                routerLSA->setNumberOfLinks(linkIndex + 1);
//                                routerLSA->setLinks(linkIndex, stubLink);
//                            }
//                            else {
//                                if (intf->getAddressRange().mask.getInt() != 0xFFFFFFFF) {
//                                    Link stubLink;
//                                    stubLink.setType(STUB_LINK);
//                                    stubLink.setLinkID(intf->getAddressRange().address
//                                            & intf->getAddressRange().mask);
//                                    stubLink.setLinkData(intf->getAddressRange().mask.getInt());
//                                    stubLink.setLinkCost(intf->getOutputCost());
//                                    stubLink.setNumberOfTOS(0);
//                                    stubLink.setTosDataArraySize(0);
//
//                                    unsigned short linkIndex = routerLSA->getLinksArraySize();
//                                    routerLSA->setLinksArraySize(linkIndex + 1);
//                                    routerLSA->setNumberOfLinks(linkIndex + 1);
//                                    routerLSA->setLinks(linkIndex, stubLink);
//                                }
//                            }
//                        }
//                    }
//                }
//                break;
//
//                case Interface::BROADCAST:
//                case Interface::NBMA: {
//                    if (intf->getState() == Interface::WAITING_STATE) {
//                        Link stubLink;
//                        stubLink.setType(STUB_LINK);
//                        stubLink.setLinkID(intf->getAddressRange().address
//                                & intf->getAddressRange().mask);
//                        stubLink.setLinkData(intf->getAddressRange().mask.getInt());
//                        stubLink.setLinkCost(intf->getOutputCost());
//                        stubLink.setNumberOfTOS(0);
//                        stubLink.setTosDataArraySize(0);
//
//                        unsigned short linkIndex = routerLSA->getLinksArraySize();
//                        routerLSA->setLinksArraySize(linkIndex + 1);
//                        routerLSA->setNumberOfLinks(linkIndex + 1);
//                        routerLSA->setLinks(linkIndex, stubLink);
//                    }
//                    else {
//                        Neighbor *dRouter = intf->getNeighborByAddress(intf->getDesignatedRouter().ipInterfaceAddress);
//                        if (((dRouter != nullptr) && (dRouter->getState() == Neighbor::FULL_STATE)) ||
//                            ((intf->getDesignatedRouter().routerID == parentRouter->getRouterID()) &&
//                             (intf->hasAnyNeighborInStates(Neighbor::FULL_STATE))))
//                        {
//                            Link link;
//                            link.setType(TRANSIT_LINK);
//                            link.setLinkID(intf->getDesignatedRouter().ipInterfaceAddress);
//                            link.setLinkData(intf->getAddressRange().address.getInt());
//                            link.setLinkCost(intf->getOutputCost());
//                            link.setNumberOfTOS(0);
//                            link.setTosDataArraySize(0);
//
//                            unsigned short linkIndex = routerLSA->getLinksArraySize();
//                            routerLSA->setLinksArraySize(linkIndex + 1);
//                            routerLSA->setNumberOfLinks(linkIndex + 1);
//                            routerLSA->setLinks(linkIndex, link);
//                        }
//                        else {
//                            Link stubLink;
//                            stubLink.setType(STUB_LINK);
//                            stubLink.setLinkID(intf->getAddressRange().address
//                                    & intf->getAddressRange().mask);
//                            stubLink.setLinkData(intf->getAddressRange().mask.getInt());
//                            stubLink.setLinkCost(intf->getOutputCost());
//                            stubLink.setNumberOfTOS(0);
//                            stubLink.setTosDataArraySize(0);
//
//                            unsigned short linkIndex = routerLSA->getLinksArraySize();
//                            routerLSA->setLinksArraySize(linkIndex + 1);
//                            routerLSA->setNumberOfLinks(linkIndex + 1);
//                            routerLSA->setLinks(linkIndex, stubLink);
//                        }
//                    }
//                }
//                break;
//
//                case Interface::VIRTUAL: {
//                    Neighbor *neighbor = (intf->getNeighborCount() > 0) ? intf->getNeighbor(0) : nullptr;
//                    if ((neighbor != nullptr) && (neighbor->getState() == Neighbor::FULL_STATE)) {
//                        Link link;
//                        link.setType(VIRTUAL_LINK);
//                        link.setLinkID(IPv4Address(neighbor->getNeighborID()));
//                        link.setLinkData(intf->getAddressRange().address.getInt());
//                        link.setLinkCost(intf->getOutputCost());
//                        link.setNumberOfTOS(0);
//                        link.setTosDataArraySize(0);
//
//                        unsigned short linkIndex = routerLSA->getLinksArraySize();
//                        routerLSA->setLinksArraySize(linkIndex + 1);
//                        routerLSA->setNumberOfLinks(linkIndex + 1);
//                        routerLSA->setLinks(linkIndex, link);
//                    }
//                }
//                break;
//
//                case Interface::POINTTOMULTIPOINT: {
//                    Link stubLink;
//                    stubLink.setType(STUB_LINK);
//                    stubLink.setLinkID(intf->getAddressRange().address);
//                    stubLink.setLinkData(0xFFFFFFFF);
//                    stubLink.setLinkCost(0);
//                    stubLink.setNumberOfTOS(0);
//                    stubLink.setTosDataArraySize(0);
//
//                    unsigned short linkIndex = routerLSA->getLinksArraySize();
//                    routerLSA->setLinksArraySize(linkIndex + 1);
//                    routerLSA->setNumberOfLinks(linkIndex + 1);
//                    routerLSA->setLinks(linkIndex, stubLink);
//
//                    long neighborCount = intf->getNeighborCount();
//                    for (long i = 0; i < neighborCount; i++) {
//                        Neighbor *neighbor = intf->getNeighbor(i);
//                        if (neighbor->getState() == Neighbor::FULL_STATE) {
//                            Link link;
//                            link.setType(POINTTOPOINT_LINK);
//                            link.setLinkID(IPv4Address(neighbor->getNeighborID()));
//                            link.setLinkData(intf->getAddressRange().address.getInt());
//                            link.setLinkCost(intf->getOutputCost());
//                            link.setNumberOfTOS(0);
//                            link.setTosDataArraySize(0);
//
//                            unsigned short linkIndex = routerLSA->getLinksArraySize();
//                            routerLSA->setLinksArraySize(linkIndex + 1);
//                            routerLSA->setNumberOfLinks(linkIndex + 1);
//                            routerLSA->setLinks(linkIndex, stubLink);
//                        }
//                    }
//                }
//                break;
//
//                default:
//                    break;
//            }
//        }
//    }
//
//    long hostRouteCount = hostRoutes.size();
//    for (i = 0; i < hostRouteCount; i++) {
//        Link stubLink;
//        stubLink.setType(STUB_LINK);
//        stubLink.setLinkID(hostRoutes[i].address);
//        stubLink.setLinkData(0xFFFFFFFF);
//        stubLink.setLinkCost(hostRoutes[i].linkCost);
//        stubLink.setNumberOfTOS(0);
//        stubLink.setTosDataArraySize(0);
//
//        unsigned short linkIndex = routerLSA->getLinksArraySize();
//        routerLSA->setLinksArraySize(linkIndex + 1);
//        routerLSA->setNumberOfLinks(linkIndex + 1);
//        routerLSA->setLinks(linkIndex, stubLink);
//    }
//
//    routerLSA->setSource(LSATrackingInfo::ORIGINATED);

    return routerLSA;
}//originateRouterLSA


OSPFv3RouterLSA* OSPFv3Area::getRouterLSAbyKey(LSAKeyType LSAKey)
{
//    EV_DEBUG << "GET ROUTER LSA BY KEY  \n";
    for (auto it=this->routerLSAList.begin(); it!=this->routerLSAList.end(); it++)
    {
//        EV_DEBUG << "FOR, routerLSAList size: " << this->routerLSAList.size() << "\n";
        if(((*it)->getHeader().getAdvertisingRouter() == LSAKey.advertisingRouter) && (*it)->getHeader().getLinkStateID() == LSAKey.linkStateID) {
            return (*it);
        }
    }

    return nullptr;
}//getRouterLSAByKey


bool OSPFv3Area::installRouterLSA(OSPFv3RouterLSA *lsa)
{
    LSAKeyType lsaKey;
    lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
    lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();
    lsaKey.LSType = lsa->getHeader().getLsaType();

    OSPFv3RouterLSA* lsaInDatabase = (OSPFv3RouterLSA*)this->getLSAbyKey(lsaKey);
    if (lsaInDatabase != nullptr) {
        this->removeFromAllRetransmissionLists(lsaKey);
        return this->updateRouterLSA(lsaInDatabase, lsa);
    }
    else {
        OSPFv3RouterLSA* lsaCopy = new OSPFv3RouterLSA(*lsa);
        this->routerLSAList.push_back(lsaCopy);
        return true;
    }
}//installRouterLSA


bool OSPFv3Area::updateRouterLSA(OSPFv3RouterLSA* currentLsa, OSPFv3RouterLSA* newLsa)
{
    bool different = routerLSADiffersFrom(currentLsa, newLsa);
    (*currentLsa) = (*newLsa);
    currentLsa->getHeader().setLsaAge(0);//reset the age
    if (different) {
//        clearNextHops();//TODO
        return true;
    }
    else {
        return false;
    }
}//updateRouterLSA


bool OSPFv3Area::routerLSADiffersFrom(OSPFv3RouterLSA* currentLsa, OSPFv3RouterLSA* newLsa)
{
    const OSPFv3LSAHeader& thisHeader = currentLsa->getHeader();
    const OSPFv3LSAHeader& lsaHeader = newLsa->getHeader();
    bool differentHeader = (((thisHeader.getLsaAge() == MAX_AGE) && (lsaHeader.getLsaAge() != MAX_AGE)) ||
                            ((thisHeader.getLsaAge() != MAX_AGE) && (lsaHeader.getLsaAge() == MAX_AGE)) ||
                            (thisHeader.getLsaLength() != lsaHeader.getLsaLength()));
    bool differentBody = false;

    if (!differentHeader) {
        differentBody = ((currentLsa->getNtBit() != newLsa->getNtBit()) ||
                         (currentLsa->getEBit() != newLsa->getEBit()) ||
                         (currentLsa->getBBit() != newLsa->getBBit()) ||
                         (currentLsa->getVBit() != newLsa->getVBit()) ||
                         (currentLsa->getXBit() != newLsa->getXBit()) ||
                         (currentLsa->getRoutersArraySize() != newLsa->getRoutersArraySize()));

        if (!differentBody) {
            unsigned int routersCount = currentLsa->getRoutersArraySize();
            for (unsigned int i = 0; i < routersCount; i++) {
                auto thisRouter = currentLsa->getRouters(i);
                auto lsaRouter = newLsa->getRouters(i);
                bool differentLink = ((thisRouter.type != lsaRouter.type) ||
                                      (thisRouter.metric != lsaRouter.metric) ||
                                      (thisRouter.interfaceID != lsaRouter.interfaceID) ||
                                      (thisRouter.neighborInterfaceID != lsaRouter.neighborInterfaceID) ||
                                      (thisRouter.neighborRouterID != lsaRouter.neighborRouterID));

                if (differentLink) {
                    differentBody = true;
                    break;
                }
            }
        }
    }

    return differentHeader || differentBody;
}//routerLSADiffersFrom

void OSPFv3Area::deleteRouterLSA(int index) {
    OSPFv3RouterLSA *delRouter = this->routerLSAList.at(index);
    OSPFv3LSAHeader &routerHeader = delRouter->getHeader();

    int prefixCount = this->intraAreaPrefixLSAList.size();
    for(int i=0; i<prefixCount; i++) {
       OSPFv3IntraAreaPrefixLSA* lsa = this->intraAreaPrefixLSAList.at(i);

       if (lsa->getReferencedAdvRtr() == routerHeader.getAdvertisingRouter() &&
               lsa->getReferencedLSID() == routerHeader.getLinkStateID() &&
               lsa->getReferencedLSType() == ROUTER_LSA) {
           this->intraAreaPrefixLSAList.erase(this->intraAreaPrefixLSAList.begin()+i);
           EV_DEBUG << "Deleting IntraAreaPrefixLSA\n";
           break;
       }
    }



    this->routerLSAList.erase(this->routerLSAList.begin()+index);
}

bool OSPFv3Area::floodLSA(OSPFv3LSA* lsa, OSPFv3Interface* interface, OSPFv3Neighbor* neighbor)
{
//    EV_DEBUG << "Flooding from Area to all interfaces\n";
    bool floodedBackOut = false;
    long interfaceCount = this->interfaceList.size();

    for (long i = 0; i < interfaceCount; i++) {
        if (interfaceList.at(i)->floodLSA(lsa, interface, neighbor)) {
            floodedBackOut = true;
        }
    }

    return floodedBackOut;
}//floodLSA


void OSPFv3Area::removeFromAllRetransmissionLists(LSAKeyType lsaKey)
{
    long interfaceCount = this->interfaceList.size();
    for (long i = 0; i < interfaceCount; i++) {
        this->interfaceList.at(i)->removeFromAllRetransmissionLists(lsaKey);
    }
}

//------------------------------------- Network LSA --------------------------------------//
OSPFv3NetworkLSA* OSPFv3Area::originateNetworkLSA(OSPFv3Interface* interface)
{
    OSPFv3NetworkLSA* networkLsa = new OSPFv3NetworkLSA();
    OSPFv3LSAHeader& lsaHeader = networkLsa->getHeader();
    OSPFv3Options lsOptions;
    memset(&lsOptions, 0, sizeof(OSPFv3Options));

    //First set the LSA Header
    lsaHeader.setLsaAge((int)simTime().dbl());
    //The LSA Type is 0x2002
    lsaHeader.setLsaType(NETWORK_LSA);

    lsaHeader.setLinkStateID(IPv4Address(interface->getInterfaceIndex()));
    lsaHeader.setAdvertisingRouter(this->getInstance()->getProcess()->getRouterID());
    lsaHeader.setLsaSequenceNumber(this->getCurrentNetworkSequence());

    uint16_t packetLength = OSPFV3_LSA_HEADER_LENGTH + 4;//4 for options field

    //now the body
    networkLsa->setOspfOptions(lsOptions);

    int attachedCount = interface->getNeighborCount();//+1 for this router
    if(attachedCount >= 2){
        networkLsa->setAttachedRouterArraySize(attachedCount+1);
        for(int i=0; i<attachedCount; i++){
            OSPFv3Neighbor* neighbor = interface->getNeighbor(i);
            networkLsa->setAttachedRouter(i, neighbor->getNeighborID());
            packetLength+=4;
        }

        networkLsa->setAttachedRouter(attachedCount, this->getInstance()->getProcess()->getRouterID());
        packetLength+=4;
    }

    lsaHeader.setLsaLength(packetLength);
    this->networkLSAList.push_back(networkLsa);
    return networkLsa;
}//originateNetworkLSA

OSPFv3IntraAreaPrefixLSA* OSPFv3Area::getNetIntraAreaPrefixLSA(L3Address prefix, int prefixLen)
{
    int intraPrefCnt = this->getIntraAreaPrefixLSACount();
    for(int i=0; i<intraPrefCnt; i++){
        OSPFv3IntraAreaPrefixLSA* intraLsa = this->getIntraAreaPrefixLSA(i);
        if(intraLsa->getReferencedLSType() == NETWORK_LSA){
            L3Address intraPrefix = intraLsa->getPrefixes(0).addressPrefix;
            int intraPrefLen = intraLsa->getPrefixes(0).prefixLen;
            if(prefix.getPrefix(prefixLen) == intraPrefix.getPrefix(intraPrefLen))
                return intraLsa;
        }
    }
}

bool OSPFv3Area::installNetworkLSA(OSPFv3NetworkLSA *lsa)
{
    LSAKeyType lsaKey;
    lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
    lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();
    lsaKey.LSType = lsa->getHeader().getLsaType();

    OSPFv3NetworkLSA* lsaInDatabase = (OSPFv3NetworkLSA*)this->getLSAbyKey(lsaKey);
    if (lsaInDatabase != nullptr) {
        this->removeFromAllRetransmissionLists(lsaKey);
        return this->updateNetworkLSA(lsaInDatabase, lsa);
    }
    else {
        OSPFv3NetworkLSA* lsaCopy = new OSPFv3NetworkLSA(*lsa);
        this->networkLSAList.push_back(lsaCopy);
        return true;
    }
}//installNetworkLSA


bool OSPFv3Area::updateNetworkLSA(OSPFv3NetworkLSA* currentLsa, OSPFv3NetworkLSA* newLsa)
{
    bool different = networkLSADiffersFrom(currentLsa, newLsa);
    (*currentLsa) = (*newLsa);
    currentLsa->getHeader().setLsaAge(0);//reset the age
    if (different) {
//        clearNextHops();//TODO
        return true;
    }
    else {
        return false;
    }
}//updateNetworkLSA


bool OSPFv3Area::networkLSADiffersFrom(OSPFv3NetworkLSA* currentLsa, OSPFv3NetworkLSA* newLsa)
{
    const OSPFv3LSAHeader& thisHeader = currentLsa->getHeader();
    const OSPFv3LSAHeader& lsaHeader = newLsa->getHeader();
    bool differentHeader = (((thisHeader.getLsaAge() == MAX_AGE) && (lsaHeader.getLsaAge() != MAX_AGE)) ||
                            ((thisHeader.getLsaAge() != MAX_AGE) && (lsaHeader.getLsaAge() == MAX_AGE)) ||
                            (thisHeader.getLsaLength() != lsaHeader.getLsaLength()));
    bool differentBody = false;

    if (!differentHeader) {
        differentBody = (currentLsa->getOspfOptions() != newLsa->getOspfOptions());


        if (!differentBody) {
            unsigned int attachedCount = currentLsa->getAttachedRouterArraySize();
            for (unsigned int i = 0; i < attachedCount; i++) {
                bool differentLink = (currentLsa->getAttachedRouter(i)!=newLsa->getAttachedRouter(i));

                if (differentLink) {
                    differentBody = true;
                    break;
                }
            }
        }
    }

    return differentHeader || differentBody;
}//networkLSADiffersFrom

IPv4Address OSPFv3Area::getNewNetworkLinkStateID()
{
    IPv4Address currIP = this->networkLsID;
    int newIP = currIP.getInt()+1;
    this->networkLsID = IPv4Address(newIP);
    return currIP;
}//getNewNetworkLinkStateID

//----------------------------------------- Inter-Area-Prefix LSA ------------------------------------------//
void OSPFv3Area::originateInterAreaPrefixLSA(OSPFv3IntraAreaPrefixLSA* lsa, OSPFv3Area* fromArea)
{
    int packetLength = OSPFV3_LSA_HEADER_LENGTH+OSPFV3_INTER_AREA_PREFIX_LSA_HEADER_LENGTH;
    int prefixCount = 0;

    //Only one Inter-Area-Prefix LSA for an area so only one header will suffice
    OSPFv3InterAreaPrefixLSA* newLsa = new OSPFv3InterAreaPrefixLSA();
    OSPFv3LSAHeader& newHeader = newLsa->getHeader();
    newHeader.setLsaAge((int)simTime().dbl());
    newHeader.setLsaType(INTER_AREA_PREFIX_LSA);
    newHeader.setLinkStateID(this->getNewInterAreaPrefixLinkStateID());
    newHeader.setAdvertisingRouter(this->getInstance()->getProcess()->getRouterID());
    newHeader.setLsaSequenceNumber(this->interAreaPrefixLSASequenceNumber++);

    OSPFv3LSAPrefix& prefix = lsa->getPrefixes(0);//TODO - this here takes only one prefix, need to make one new LSA for each prefix
    newLsa->setDnBit(prefix.dnBit);
    newLsa->setLaBit(prefix.laBit);
    newLsa->setNuBit(prefix.nuBit);
    newLsa->setPBit(prefix.pBit);
    newLsa->setXBit(prefix.xBit);
    newLsa->setMetric(prefix.metric);
    newLsa->setPrefixLen(64);//TODO - correct pref length
    newLsa->setPrefix(prefix.addressPrefix);

//    EV_DEBUG << "Setting Address Prefix in InterAreaPrefixLSA to " << prefix.addressPrefix << endl;

    for(int i = 0; i < this->getInstance()->getAreaCount(); i++)
    {
        OSPFv3Area* area = this->getInstance()->getArea(i);
        if(area->getAreaID() == fromArea->getAreaID())
            continue;

        area->installInterAreaPrefixLSA(newLsa);
    }
    //TODO - length!!!
}

void OSPFv3Area::originateInterAreaPrefixLSA(OSPFv3LSA* prefLsa, OSPFv3Area* fromArea)
{
    for(int i = 0; i < this->getInstance()->getAreaCount(); i++)
    {
        OSPFv3Area* area = this->getInstance()->getArea(i);
        if(area->getAreaID() == fromArea->getAreaID())
            continue;

        OSPFv3InterAreaPrefixLSA *lsa = check_and_cast<OSPFv3InterAreaPrefixLSA *>(prefLsa);
        int packetLength = OSPFV3_LSA_HEADER_LENGTH+OSPFV3_INTER_AREA_PREFIX_LSA_HEADER_LENGTH;
        int prefixCount = 0;

        //Only one Inter-Area-Prefix LSA for an area so only one header will suffice
        OSPFv3InterAreaPrefixLSA* newLsa = new OSPFv3InterAreaPrefixLSA();
        OSPFv3LSAHeader& newHeader = newLsa->getHeader();
        newHeader.setLsaAge(0);
        newHeader.setLsaType(INTER_AREA_PREFIX_LSA);
        newHeader.setLinkStateID(area->getNewInterAreaPrefixLinkStateID());
        newHeader.setAdvertisingRouter(this->getInstance()->getProcess()->getRouterID());
        newHeader.setLsaSequenceNumber(area->getCurrentInterAreaPrefixSequence());
        area->incrementInterAreaPrefixSequence();

        newLsa->setDnBit(lsa->getDnBit());
        newLsa->setLaBit(lsa->getLaBit());
        newLsa->setNuBit(lsa->getNuBit());
        newLsa->setPBit(lsa->getPBit());
        newLsa->setXBit(lsa->getXBit());
        newLsa->setMetric(lsa->getMetric());
        newLsa->setPrefixLen(64);//TODO - correct pref length
        newLsa->setPrefix(lsa->getPrefix());

        area->installInterAreaPrefixLSA(newLsa);
    }
    //TODO - length!!!
}

void OSPFv3Area::originateDefaultInterAreaPrefixLSA(OSPFv3Area* toArea)
{
    int packetLength = OSPFV3_LSA_HEADER_LENGTH+OSPFV3_INTER_AREA_PREFIX_LSA_HEADER_LENGTH;
    int prefixCount = 0;

    //Only one Inter-Area-Prefix LSA for an area so only one header will suffice
    OSPFv3InterAreaPrefixLSA* newLsa = new OSPFv3InterAreaPrefixLSA();
    OSPFv3LSAHeader& newHeader = newLsa->getHeader();
    newHeader.setLsaAge(0);
    newHeader.setLsaType(INTER_AREA_PREFIX_LSA);
    newHeader.setLinkStateID(toArea->getNewInterAreaPrefixLinkStateID());
    newHeader.setAdvertisingRouter(this->getInstance()->getProcess()->getRouterID());
    newHeader.setLsaSequenceNumber(toArea->getCurrentInterAreaPrefixSequence());
    toArea->incrementInterAreaPrefixSequence();

//    OSPFv3LSAPrefix& prefix = lsa->getPrefixes(0);//TODO - this here takes only one prefix, need to make one new LSA for each prefix
    newLsa->setDnBit(false);
    newLsa->setLaBit(false);
    newLsa->setNuBit(false);
    newLsa->setPBit(false);
    newLsa->setXBit(false);
    newLsa->setMetric(1);
    newLsa->setPrefixLen(0);

    if(this->getInstance()->getAddressFamily() == IPV4INSTANCE) {
        IPv4Address defaultPref = IPv4Address("0.0.0.0");
        newLsa->setPrefix(defaultPref);
    }
    else{
        IPv6Address defaultPref = IPv6Address("::");
        newLsa->setPrefix(defaultPref);
    }

//    EV_DEBUG << "Setting Address Prefix in InterAreaPrefixLSA to " << prefix.addressPrefix << endl;
    toArea->installInterAreaPrefixLSA(newLsa);
    //TODO - length!!!
}

bool OSPFv3Area::installInterAreaPrefixLSA(OSPFv3InterAreaPrefixLSA* lsa)
{
    OSPFv3LSAHeader &header = lsa->getHeader();

    EV_DEBUG << "Installing Inter-Area-Prefix LSA:\nLink State ID: " << header.getLinkStateID() << "\nAdvertising router: " << header.getAdvertisingRouter();
    EV_DEBUG << "\nLS Seq Number: " << header.getLsaSequenceNumber() << endl;

    EV_DEBUG << "Prefix Address: " << lsa->getPrefix();
    EV_DEBUG << "\nPrefix Length: " << lsa->getPrefixLen();
    if(lsa->getDnBit())
        EV_DEBUG << "DN ";
    if(lsa->getLaBit())
        EV_DEBUG << "LA ";
    if(lsa->getNuBit())
        EV_DEBUG << "NU ";
    if(lsa->getPBit())
        EV_DEBUG << "P ";
    if(lsa->getXBit())
        EV_DEBUG << "X ";

    EV_DEBUG << ", Metric: " << lsa->getMetric() << "\n";

    LSAKeyType lsaKey;
    lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
    lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();
    lsaKey.LSType = lsa->getHeader().getLsaType();

    OSPFv3InterAreaPrefixLSA* lsaInDatabase = (OSPFv3InterAreaPrefixLSA*)this->getLSAbyKey(lsaKey);
    if (lsaInDatabase != nullptr) {
        this->removeFromAllRetransmissionLists(lsaKey);
        EV_DEBUG << "Only updating\n";
        return this->updateInterAreaPrefixLSA(lsaInDatabase, lsa);
    }
    else {
        OSPFv3InterAreaPrefixLSA* lsaCopy = new OSPFv3InterAreaPrefixLSA(*lsa);
        this->interAreaPrefixLSAList.push_back(lsaCopy);
        EV_DEBUG << "creating new one\n";
        return true;
    }
}

bool OSPFv3Area::updateInterAreaPrefixLSA(OSPFv3InterAreaPrefixLSA* currentLsa, OSPFv3InterAreaPrefixLSA* newLsa)
{
    bool different = interAreaPrefixLSADiffersFrom(currentLsa, newLsa);
    (*currentLsa) = (*newLsa);
    currentLsa->getHeader().setLsaAge(0);//reset the age
    if (different) {
        //        clearNextHops();//TODO
        return true;
    }
    else {
        return false;
    }
}

bool OSPFv3Area::interAreaPrefixLSADiffersFrom(OSPFv3InterAreaPrefixLSA* currentLsa, OSPFv3InterAreaPrefixLSA* newLsa)
{
    const OSPFv3LSAHeader& thisHeader = currentLsa->getHeader();
    const OSPFv3LSAHeader& lsaHeader = newLsa->getHeader();
    bool differentHeader = (((thisHeader.getLsaAge() == MAX_AGE) && (lsaHeader.getLsaAge() != MAX_AGE)) ||
            ((thisHeader.getLsaAge() != MAX_AGE) && (lsaHeader.getLsaAge() == MAX_AGE)) ||
            (thisHeader.getLsaLength() != lsaHeader.getLsaLength()));
    bool differentBody = false;

    if (!differentHeader) {
        differentBody = ((currentLsa->getMetric() != newLsa->getMetric()) ||
                (currentLsa->getPrefix() != newLsa->getPrefix()) ||
                (currentLsa->getPrefixLen() != newLsa->getPrefixLen()) ||
                (currentLsa->getDnBit() != newLsa->getDnBit()) ||
                (currentLsa->getLaBit() != newLsa->getLaBit()) ||
                (currentLsa->getMetric() != newLsa->getMetric()) ||
                (currentLsa->getNuBit() != newLsa->getNuBit()) ||
                (currentLsa->getPBit() != newLsa->getPBit()) ||
                (currentLsa->getPrefixLen() != newLsa->getPrefixLen()) ||
                (currentLsa->getXBit() != newLsa->getXBit()));
    }

    return differentHeader || differentBody;
}

IPv4Address OSPFv3Area::getNewInterAreaPrefixLinkStateID()
{
    IPv4Address currIP = this->interAreaPrefixLsID;
    int newIP = currIP.getInt()+1;
    this->interAreaPrefixLsID = IPv4Address(newIP);
    return currIP;
}


//----------------------------------------- Intra-Area-Prefix LSA ------------------------------------------//
OSPFv3IntraAreaPrefixLSA* OSPFv3Area::originateIntraAreaPrefixLSA()
{
    int packetLength = OSPFV3_LSA_HEADER_LENGTH+OSPFV3_INTRA_AREA_PREFIX_LSA_HEADER_LENGTH;
    int prefixCount = 0;

    //Only one Inter-Area-Prefix LSA for an area so only one header will suffice
    OSPFv3IntraAreaPrefixLSA* newLsa = new OSPFv3IntraAreaPrefixLSA();
    OSPFv3LSAHeader& newHeader = newLsa->getHeader();
    newHeader.setLsaAge((int)simTime().dbl());
    newHeader.setLsaType(INTRA_AREA_PREFIX_LSA);
    newHeader.setLinkStateID(this->getNewIntraAreaPrefixLinkStateID());
    newHeader.setAdvertisingRouter(this->getInstance()->getProcess()->getRouterID());
    newHeader.setLsaSequenceNumber(this->intraAreaPrefixLSASequenceNumber++);

    //for each Router LSA there is a corresponding Inter-Area-Prefix LSA
    for(auto it=this->routerLSAList.begin(); it!=this->routerLSAList.end(); it++) {
        OSPFv3LSAHeader &routerHeader = (*it)->getHeader();
        if(routerHeader.getAdvertisingRouter()!=this->getInstance()->getProcess()->getRouterID())
            continue;
        else {
            newLsa->setReferencedLSType(ROUTER_LSA);
            newLsa->setReferencedLSID(routerHeader.getLinkStateID());
            newLsa->setReferencedAdvRtr(routerHeader.getAdvertisingRouter());
        }
    }

    int currentPrefix = 0;
    for(auto it=this->interfaceList.begin(); it!=this->interfaceList.end(); it++) {
        if((*it)->getTransitNetInt()==false) {//if this interface is not reported as transit, all its addresses belong to the prefix
            InterfaceEntry *ie = this->getInstance()->getProcess()->ift->getInterfaceByName((*it)->getIntName().c_str());
            IPv6InterfaceData* ipv6int = ie->ipv6Data();
            int ipv6Count = ipv6int->getNumAddresses();
            currentPrefix++;
            for(int i=0; i<ipv6Count; i++) {
                IPv6Address ipv6 = ipv6int->getAddress(i);
                if(ipv6.isGlobal()) {//Only all the global prefixes belong to the Intra-Area-Prefix LSA
                    OSPFv3LSAPrefix *prefix = new OSPFv3LSAPrefix();
                    prefix->prefixLen=64;//TODO - this will never work until the prefix can be gathered from IPv6Address
                    prefix->metric=1;//TODO - check
                    prefix->addressPrefix=ipv6;

                    newLsa->setPrefixesArraySize(currentPrefix);
                    newLsa->setPrefixes(currentPrefix-1, *prefix);
                    prefixCount++;
                }
            }
        }
    }

    //TODO - length!!!
    newLsa->setNumPrefixes(prefixCount);
    return newLsa;
}//originateIntraAreaPrefixLSA

OSPFv3IntraAreaPrefixLSA* OSPFv3Area::originateNetIntraAreaPrefixLSA(OSPFv3NetworkLSA* networkLSA, OSPFv3Interface* interface)
{
    OSPFv3LSAHeader &header = networkLSA->getHeader();

    OSPFv3IntraAreaPrefixLSA* newLsa = new OSPFv3IntraAreaPrefixLSA();
    OSPFv3LSAHeader& newHeader = newLsa->getHeader();
    newHeader.setLsaAge((int)simTime().dbl());
    newHeader.setLsaType(INTRA_AREA_PREFIX_LSA);
    newHeader.setLinkStateID(this->getNewNetIntraAreaPrefixLinkStateID());
    newHeader.setAdvertisingRouter(this->getInstance()->getProcess()->getRouterID());
    newHeader.setLsaSequenceNumber(this->netIntraAreaPrefixLSASequenceNumber++);

    newLsa->setReferencedLSType(NETWORK_LSA);
    newLsa->setReferencedLSID(header.getLinkStateID());
    newLsa->setReferencedAdvRtr(header.getAdvertisingRouter());

    InterfaceEntry *ie = this->getInstance()->getProcess()->ift->getInterfaceByName(interface->getIntName().c_str());
    IPv6InterfaceData* ipv6int = ie->ipv6Data();
    int ipv6Count = ipv6int->getNumAddresses();
    int currentPrefix = 1;
    int prefixCount = 0;
    for(int i=0; i<ipv6Count; i++) {
        IPv6Address ipv6 = ipv6int->getAddress(i);
        if(ipv6.isGlobal()) {//Only all the global prefixes belong to the Intra-Area-Prefix LSA
            OSPFv3LSAPrefix *prefix = new OSPFv3LSAPrefix();
            prefix->prefixLen=64;//TODO - this will never work until the prefix can be gathered from IPv6Address
            prefix->metric=0;//TODO - check
            prefix->addressPrefix=ipv6.getPrefix(64);

            newLsa->setPrefixesArraySize(currentPrefix);
            newLsa->setPrefixes(currentPrefix-1, *prefix);
            prefixCount++;
            currentPrefix++;
        }
    }

    newLsa->setNumPrefixes(prefixCount);

    int intraPrefCnt = this->getIntraAreaPrefixLSACount();
    for(int i=0; i<intraPrefCnt; i++){
        OSPFv3IntraAreaPrefixLSA* pref = this->getIntraAreaPrefixLSA(i);
        if((pref->getReferencedAdvRtr() == this->getInstance()->getProcess()->getRouterID()) &&
                pref->getReferencedLSType() == ROUTER_LSA){
            this->intraAreaPrefixLSAList.erase(this->intraAreaPrefixLSAList.begin()+i);
        }
    }

    return newLsa;
}

bool OSPFv3Area::installIntraAreaPrefixLSA(OSPFv3IntraAreaPrefixLSA *lsa)
{
    OSPFv3LSAHeader &header = lsa->getHeader();

    EV_DEBUG << "Installing Intra-Area-Prefix LSA:\nLink State ID: " << header.getLinkStateID() << "\nAdvertising router: " << header.getAdvertisingRouter();
    EV_DEBUG << "\nLS Seq Number: " << header.getLsaSequenceNumber() << "\nReferenced LSA Type: " << lsa->getReferencedLSType();
    EV_DEBUG << "\nReferenced Link State ID: " << lsa->getReferencedLSID();
    EV_DEBUG << "\nReferenced Advertising Router: " << lsa->getReferencedAdvRtr();
    EV_DEBUG << "\nNumber of Prefixes: " << lsa->getNumPrefixes() << " and " << lsa->getPrefixesArraySize() << "\n";

    for(int i = 0; i<lsa->getNumPrefixes(); i++) {
        OSPFv3LSAPrefix &prefix = lsa->getPrefixes(i);
        EV_DEBUG << "Prefix Address: " << prefix.addressPrefix;
        EV_DEBUG << "\nPrefix Length: " << prefix.prefixLen;
        if(prefix.dnBit)
            EV_DEBUG << "DN ";
        if(prefix.laBit)
            EV_DEBUG << "LA ";
        if(prefix.nuBit)
            EV_DEBUG << "NU ";
        if(prefix.pBit)
            EV_DEBUG << "P ";
        if(prefix.xBit)
            EV_DEBUG << "X ";

        EV_DEBUG << ", Metric: " << prefix.metric << "\n";
    }

    LSAKeyType lsaKey;
    lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
    lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();
    lsaKey.LSType = lsa->getHeader().getLsaType();

    if(lsa->getReferencedLSType() == NETWORK_LSA){
        EV_DEBUG << "Received NETWORK_LSA prefix: " << lsa->getPrefixes(0).addressPrefix << endl;
        int intraPrefCnt = this->getIntraAreaPrefixLSACount();
        for(int i=0; i<intraPrefCnt; i++){
            OSPFv3IntraAreaPrefixLSA* pref = this->getIntraAreaPrefixLSA(i);
            if(pref->getReferencedLSType() == ROUTER_LSA) {
                EV_DEBUG << "Examining IntraAreaPrefix LSA - " << pref->getPrefixes(0).addressPrefix << endl;
                IPv6Address routerPref = lsa->getPrefixes(0).addressPrefix.toIPv6();
                IPv6Address netPref = pref->getPrefixes(0).addressPrefix.toIPv6();
                if(routerPref.getPrefix(64) == netPref.getPrefix(64)) {
                    EV_DEBUG << "THE SAAAAAMEEEEE DELETIIIIIING\n";
                    this->intraAreaPrefixLSAList.erase(this->intraAreaPrefixLSAList.begin()+i);
                    return false;
                }
            }
        }

    }

    if(lsa->getReferencedLSType() == ROUTER_LSA){
        EV_DEBUG << "Received ROUTER Intra prefix: " << lsa->getPrefixes(0).addressPrefix << endl;
        int intraPrefCnt = this->getIntraAreaPrefixLSACount();
        for(int i=0; i<intraPrefCnt; i++){
            OSPFv3IntraAreaPrefixLSA* pref = this->getIntraAreaPrefixLSA(i);
            if(pref->getReferencedLSType() == NETWORK_LSA) {
                EV_DEBUG << "Examining IntraAreaPrefix LSA - " << pref->getPrefixes(0).addressPrefix << endl;
                IPv6Address routerPref = lsa->getPrefixes(0).addressPrefix.toIPv6();
                IPv6Address netPref = pref->getPrefixes(0).addressPrefix.toIPv6();
                if(routerPref.getPrefix(64) == netPref.getPrefix(64)) {
                    EV_DEBUG << "THE SAAAAAMEEEEE DELETIIIIIING\n";
                    return false;
                }
            }
        }
    }

    OSPFv3IntraAreaPrefixLSA* lsaInDatabase = (OSPFv3IntraAreaPrefixLSA*)this->getLSAbyKey(lsaKey);
    if (lsaInDatabase != nullptr) {
        this->removeFromAllRetransmissionLists(lsaKey);
        EV_DEBUG << "Only updating\n";
        return this->updateIntraAreaPrefixLSA(lsaInDatabase, lsa);
    }
    else {
        OSPFv3IntraAreaPrefixLSA* lsaCopy = new OSPFv3IntraAreaPrefixLSA(*lsa);
        this->intraAreaPrefixLSAList.push_back(lsaCopy);
        EV_DEBUG << "creating new one\n";
        return true;
    }
}//installIntraAreaPrefixLSA


bool OSPFv3Area::updateIntraAreaPrefixLSA(OSPFv3IntraAreaPrefixLSA* currentLsa, OSPFv3IntraAreaPrefixLSA* newLsa)
{
    bool different = intraAreaPrefixLSADiffersFrom(currentLsa, newLsa);
    (*currentLsa) = (*newLsa);
    currentLsa->getHeader().setLsaAge(0);//reset the age
    if (different) {
//        clearNextHops();//TODO
        return true;
    }
    else {
        return false;
    }
}//updateIntraAreaPrefixLSA


bool OSPFv3Area::intraAreaPrefixLSADiffersFrom(OSPFv3IntraAreaPrefixLSA* currentLsa, OSPFv3IntraAreaPrefixLSA* newLsa)
{
    const OSPFv3LSAHeader& thisHeader = currentLsa->getHeader();
    const OSPFv3LSAHeader& lsaHeader = newLsa->getHeader();
    bool differentHeader = (((thisHeader.getLsaAge() == MAX_AGE) && (lsaHeader.getLsaAge() != MAX_AGE)) ||
                            ((thisHeader.getLsaAge() != MAX_AGE) && (lsaHeader.getLsaAge() == MAX_AGE)) ||
                            (thisHeader.getLsaLength() != lsaHeader.getLsaLength()));
    bool differentBody = false;

    if (!differentHeader) {
        differentBody = ((currentLsa->getNumPrefixes() != newLsa->getNumPrefixes()) ||
                         (currentLsa->getReferencedLSType() != newLsa->getReferencedLSType()) ||
                         (currentLsa->getReferencedLSID() != newLsa->getReferencedLSID()) ||
                         (currentLsa->getReferencedAdvRtr() != newLsa->getReferencedAdvRtr()));


        if (!differentBody) {
            unsigned int referenceCount = currentLsa->getNumPrefixes();
            for (unsigned int i = 0; i < referenceCount; i++) {
                OSPFv3LSAPrefix currentPrefix = currentLsa->getPrefixes(i);
                OSPFv3LSAPrefix newPrefix = newLsa->getPrefixes(i);
                bool differentLink = ((currentPrefix.addressPrefix != newPrefix.addressPrefix) ||
                                      (currentPrefix.dnBit != newPrefix.dnBit) ||
                                      (currentPrefix.laBit != newPrefix.laBit) ||
                                      (currentPrefix.metric != newPrefix.metric) ||
                                      (currentPrefix.nuBit != newPrefix.nuBit) ||
                                      (currentPrefix.pBit != newPrefix.pBit) ||
                                      (currentPrefix.prefixLen != newPrefix.prefixLen) ||
                                      (currentPrefix.xBit != newPrefix.xBit));

                if (differentLink) {
                    differentBody = true;
                    break;
                }
            }
        }
    }

    return differentHeader || differentBody;
}//intraAreaPrefixLSADiffersFrom

IPv4Address OSPFv3Area::getNewIntraAreaPrefixLinkStateID()
{
    IPv4Address currIP = this->intraAreaPrefixLsID;
    int newIP = currIP.getInt()+1;
    this->intraAreaPrefixLsID = IPv4Address(newIP);
    return currIP;
}//getNewIntraAreaPrefixStateID

IPv4Address OSPFv3Area::getNewNetIntraAreaPrefixLinkStateID()
{
    IPv4Address currIP = this->netIntraAreaPrefixLsID;
    int newIP = currIP.getInt()+1;
    this->netIntraAreaPrefixLsID = IPv4Address(newIP);
    return currIP;
}//getNewNetIntraAreaPrefixStateID


OSPFv3LSA* OSPFv3Area::getLSAbyKey(LSAKeyType LSAKey)
{
    switch(LSAKey.LSType){
    case ROUTER_LSA:
        for (auto it=this->routerLSAList.begin(); it!=this->routerLSAList.end(); it++)
        {
            if(((*it)->getHeader().getAdvertisingRouter() == LSAKey.advertisingRouter) && (*it)->getHeader().getLinkStateID() == LSAKey.linkStateID) {
                return (*it);
            }
        }

        break;

    case NETWORK_LSA:
        for (auto it=this->networkLSAList.begin(); it!=this->networkLSAList.end(); it++)
        {
            if(((*it)->getHeader().getAdvertisingRouter() == LSAKey.advertisingRouter) && (*it)->getHeader().getLinkStateID() == LSAKey.linkStateID) {
                return (*it);
            }
        }

        break;

    case INTER_AREA_PREFIX_LSA:
        for (auto it=this->interAreaPrefixLSAList.begin(); it!=this->interAreaPrefixLSAList.end(); it++)
        {
            if(((*it)->getHeader().getAdvertisingRouter() == LSAKey.advertisingRouter) && (*it)->getHeader().getLinkStateID() == LSAKey.linkStateID) {
                return (*it);
            }
        }

        break;

    case INTER_AREA_ROUTER_LSA:
        for (auto it=this->routerLSAList.begin(); it!=this->routerLSAList.end(); it++)
        {
            if(((*it)->getHeader().getAdvertisingRouter() == LSAKey.advertisingRouter) && (*it)->getHeader().getLinkStateID() == LSAKey.linkStateID) {
                return (*it);
            }
        }

        break;

    case NSSA_LSA:
        for (auto it=this->routerLSAList.begin(); it!=this->routerLSAList.end(); it++)
        {
            if(((*it)->getHeader().getAdvertisingRouter() == LSAKey.advertisingRouter) && (*it)->getHeader().getLinkStateID() == LSAKey.linkStateID) {
                return (*it);
            }
        }

        break;

    case INTRA_AREA_PREFIX_LSA:
        for (auto it=this->intraAreaPrefixLSAList.begin(); it!=this->intraAreaPrefixLSAList.end(); it++)
        {
            if(((*it)->getHeader().getAdvertisingRouter() == LSAKey.advertisingRouter) && (*it)->getHeader().getLinkStateID() == LSAKey.linkStateID) {
                return (*it);
            }
        }

        break;

    case LINK_LSA:
        for (auto it=this->interfaceList.begin(); it!=this->interfaceList.end(); it++)
        {
            OSPFv3LinkLSA* lsa = (*it)->getLinkLSAbyKey(LSAKey);
            if(lsa != nullptr)
                return lsa;
        }

        break;
    }
//TODO - link lsa from interface
    //TOFO - as external from top data structure


    return nullptr;
}

void OSPFv3Area::calculateShortestPathTree(std::vector<OSPFv3RoutingTableEntry* > newTable)
{
//    EV_DEBUG << "Calculating SPF Tree for area " << this->getAreaID() << "\n";
    /*1)Initialize the algorithm’s data structures. Clear the list
        of candidate vertices. Initialize the shortest-path tree to
        only the root (which is the router doing the calculation).
        Set Area A’s TransitCapability to FALSE*/

    std::vector<OSPFv3SPFVertex*> candidates;
    std::vector<OSPFv3SPFVertex*> treeVertices;
    OSPFv3SPFVertex* justAddedVertex;
    VertexID rootID;
    OSPFv3SPFVertex* rootVertex;
    IPv4Address routerID = this->getInstance()->getProcess()->getRouterID();
    bool transitCapability = false;
    bool finished = false;

    rootID.routerID = routerID;
    rootVertex = new OSPFv3SPFVertex(ROUTER_VERTEX, rootID);
    rootVertex->setSource(ORIGINATED);

    for(auto it=this->routerLSAList.begin(); it != this->routerLSAList.end(); it++){
        if ((*it)->getHeader().getAdvertisingRouter()==routerID) {
            rootVertex->setVertexLSA((*it));
//            EV_DEBUG << "ROOT:\n";
//            rootVertex->vertexInfo();
            break;
        }
    }

    justAddedVertex = rootVertex;

//    EV_DEBUG << "SPFTREE:";
    /*2)Call the vertex just added to the tree vertex V. Examine
        the LSA associated with vertex V. This is a lookup in the
        Area A’s link state database based on the Vertex ID. If
        this is a router-LSA, and bit V of the router-LSA (see
        Section A.4.2) is set, set Area A’s TransitCapability to
        TRUE. In any case, each link described by the LSA gives the
        cost to an adjacent vertex. For each described link, (say
        it joins vertex V to vertex W):*/
    do{
        if(justAddedVertex->getVertexType() == ROUTER_VERTEX){
//            EV_DEBUG << "\t\tCurrent Vertex - ROUTER\n";
            OSPFv3RouterLSA* currentLSA = justAddedVertex->getVertexLSA()->routerLSA;
            if(currentLSA->getVBit() == true)
                transitCapability = true;

            unsigned int routerCount = currentLSA->getRoutersArraySize();
            for(unsigned int i = 0; i<routerCount; i++){
//                EV_DEBUG << "\t\tExamining Router body\n";
                OSPFv3RouterLSABody& router = currentLSA->getRouters(i);
                short int routerType = router.type;
                OSPFv3SPFVertex* joiningVertex;
                VertexType joiningVertexType;

                /*2b)W is a transit vertex (router or transit
                     network). Look up the vertex W’s LSA (router-LSA or
                     network-LSA) in Area A’s link state database. If the
                     LSA does not exist, or its LS age is equal to MaxAge, or
                     it does not have a link back to vertex V, examine the
                     next link in V’s LSA.*/
                if(routerType == TRANSIT_NETWORK) {
//                    EV_DEBUG << "\t\tRouter is transit\n";
                    OSPFv3NetworkLSA* joiningLSA=nullptr;
//                    EV_DEBUG << "\t\tInterface id is " << router.interfaceID << "\n";
                    OSPFv3Interface* intf = this->getInterfaceByIndex(router.interfaceID);
                    IPv4Address designatedRtr = intf->getDesignatedID();

                    for(auto it=this->networkLSAList.begin(); it!=this->networkLSAList.end(); it++){
                        if((*it)->getHeader().getAdvertisingRouter() == designatedRtr) {
//                            EV_DEBUG << "\t\tFound Network LSA originated by " << designatedRtr << "\n";
//                            EV_DEBUG << "\t\tDesignated Rtr Interface ID is " << intf->getDesignatedIntID() << "\n";
                            joiningLSA = (*it);
                            break;
                        }
                    }

                    if(joiningLSA == nullptr)
                        continue;

                    joiningVertexType = NETWORK_VERTEX;
                    VertexID vertID;
                    //TODO - here I need not only Router ID but also its interface ID
                    //TODO - V6 and R bits check
                    vertID.routerID = designatedRtr;
                    vertID.interfaceID = 0;
                    joiningVertex = new OSPFv3SPFVertex(joiningVertexType, vertID);
                    joiningVertex->setVertexLSA(joiningLSA);
                }
                else{
//                    EV_DEBUG << "\t\tRouter is Router";
                    OSPFv3RouterLSA* joiningLSA=nullptr;
                    for(auto it=this->routerLSAList.begin(); it!=this->routerLSAList.end(); it++){
                        if((*it)->getHeader().getAdvertisingRouter()==router.neighborRouterID){
                            joiningLSA = (*it);
//                            EV_DEBUG << "\t\tFound Router LSA originated by " << (*it)->getHeader().getAdvertisingRouter() << "\n";
                            break;
                        }
                    }

                    if(joiningLSA == nullptr)
                        continue;

                    joiningVertexType = ROUTER_VERTEX;
                    VertexID vertID;
                    vertID.routerID = router.neighborRouterID;
                    joiningVertex = new OSPFv3SPFVertex(joiningVertexType, vertID);
                    joiningVertex->setVertexLSA(joiningLSA);
                }

                unsigned int treeSize = treeVertices.size();
                bool alreadyOnTree = false;

                for(int i=0; i<treeSize; i++){
                    if(treeVertices.at(i)->getVertexID()==joiningVertex->getVertexID()){
                        alreadyOnTree = true;
                        break;
                    }
                }

                /*2c)If vertex W is already on the shortest-path tree,
                     examine the next link in the LSA.*/
                if(alreadyOnTree) {
//                    EV_DEBUG << "\t\tAlready on the tree... skippnig\n";
                    continue;
                }
//                else
//                    EV_DEBUG << "\t\tTotaly new to the tree\n";

                /*2d)Calculate the link state cost D of the resulting path
                     from the root to vertex W. D is equal to the sum of the
                     link state cost of the (already calculated) shortest
                     path to vertex V and the advertised cost of the link
                     between vertices V and W. If D is:*/
                unsigned long linkStateCost = justAddedVertex->getDistance() + router.metric;
                unsigned int candidateCount = candidates.size();
                OSPFv3SPFVertex *candidate = nullptr;

                for (int j = 0; j < candidateCount; j++) {
                    if (candidates.at(j)->getVertexID() == joiningVertex->getVertexID()) {
                        candidate = candidates.at(j);
                    }
                }

                if(candidate!=nullptr){
//                    EV_DEBUG << "\t\tcandidate found on candidate list\n";
                    unsigned long candidateDistance = candidate->getDistance();

                    /*  Greater than the value that already appears for
                        vertex W on the candidate list, then examine the
                        next link.*/
                    if (linkStateCost > candidateDistance) {
//                        EV_DEBUG << "\t\tlink state cost greater than candidateDistance\n";
                        continue;
                    }
                    /*  Less than the value that appears for vertex W on the
                        candidate list, or if W does not yet appear on the
                        candidate list, then set the entry for W on the
                        candidate list to indicate a distance of D from the
                        root. Also calculate the list of next hops that
                        result from using the advertised link, setting the
                        next hop values for W accordingly.It takes
                        as input the destination (W) and its parent (V)*/
                    if (linkStateCost < candidateDistance) {
//                        EV_DEBUG << "\t\tlink state cost is less than that of candidate\n";
                        candidate->setDistance(linkStateCost);
                        candidate->clearNextHops();
                    }

                    /*  Equal to the value that appears for vertex W on the
                        candidate list, calculate the set of next hops that
                        result from using the advertised link. Input to
                        this calculation is the destination (W), and its
                        parent (V). This set of hops should be added to the
                        next hop values that appear for W on the candidate
                        list.*/
//                    std::vector<NextHop> *newNextHops = calculateNextHops(joiningVertex, justAddedVertex);    // (destination, parent)
//                    unsigned int nextHopCount = newNextHops->size();
//                    for (int k = 0; k < nextHopCount; k++) {
//                        candidate->addNextHop((*newNextHops)[k]);
//                    }
//                    delete newNextHops;
                }
                else {
//                    EV_DEBUG << "\t\tcandidate not found on the tree - adding\n";
                    joiningVertex->setDistance(linkStateCost);
                    //std::vector<NextHop> *newNextHops = calculateNextHops(joiningVertex, justAddedVertex);    // (destination, parent)
                    //unsigned int nextHopCount = newNextHops->size();
//                    for (int k = 0; k < nextHopCount; k++) {
//                        joiningVertex->addNextHop((*newNextHops)[k]);
//                    }
//                    delete newNextHops;
                    joiningVertex->setParent(justAddedVertex);

                    candidates.push_back(joiningVertex);
//                    EV_DEBUG << "\t\tcandidates size: " << candidates.size();
                }
            }//for
        }

        if(justAddedVertex->getVertexType() == NETWORK_VERTEX){
//            EV_DEBUG << "\t\tCurrent Vertex - Network\n";
            unsigned int routerCount = justAddedVertex->getVertexLSA()->networkLSA->getAttachedRouterArraySize();

            for(int i=0; i<routerCount;i++){
                OSPFv3SPFVertex* joiningVertex;
                VertexType joiningVertexType;
                IPv4Address routerID = justAddedVertex->getVertexLSA()->networkLSA->getAttachedRouter(i);
                OSPFv3RouterLSA* joiningLSA = nullptr;
                for(auto it=this->routerLSAList.begin(); it<this->routerLSAList.end(); it++){
                    if((*it)->getHeader().getAdvertisingRouter()==routerID) {
                        joiningLSA = (*it);
                    }
                }

                if(joiningLSA == nullptr)
                    continue;

                joiningVertexType = ROUTER_VERTEX;
                VertexID vertID;
                vertID.routerID = routerID;
                joiningVertex = new OSPFv3SPFVertex(joiningVertexType, vertID);
                joiningVertex->setVertexLSA(joiningLSA);

                unsigned int treeSize = treeVertices.size();
                bool alreadyOnTree = false;

                for (int j = 0; j < treeSize; j++) {
                    if (treeVertices.at(i)->getVertexID()==joiningVertex->getVertexID()) {
                        alreadyOnTree = true;
                        break;
                    }
                }
                if (alreadyOnTree) {    // (2) (c)
                    continue;
                }

                unsigned long linkStateCost = justAddedVertex->getDistance();
                unsigned int candidateCount = candidates.size();
                OSPFv3SPFVertex *candidate = nullptr;

                for (int j = 0; j < candidateCount; j++) {
                    if (candidates.at(j)->getVertexID() == joiningVertex->getVertexID()) {
                        candidate = candidates.at(j);
                    }
                }

                if(candidate!=nullptr){
//                    EV_DEBUG << "\t\tcandidate found on candidate list\n";
                    unsigned long candidateDistance = candidate->getDistance();

                    if (linkStateCost > candidateDistance) {
//                        EV_DEBUG << "\t\tlink state cost greater than candidateDistance\n";
                        continue;
                    }
                    if (linkStateCost < candidateDistance) {
//                        EV_DEBUG << "\t\tlink state cost is less than that of candidate\n";
                        candidate->setDistance(linkStateCost);
                        candidate->clearNextHops();
                    }

                    //                    std::vector<NextHop> *newNextHops = calculateNextHops(joiningVertex, justAddedVertex);    // (destination, parent)
                    //                    unsigned int nextHopCount = newNextHops->size();
                    //                    for (int k = 0; k < nextHopCount; k++) {
                    //                        candidate->addNextHop((*newNextHops)[k]);
                    //                    }
                    //                    delete newNextHops;
                }
                else {
//                    EV_DEBUG << "\t\tcandidate not found on the tree - adding\n";
                    joiningVertex->setDistance(linkStateCost);
                    //std::vector<NextHop> *newNextHops = calculateNextHops(joiningVertex, justAddedVertex);    // (destination, parent)
                    //unsigned int nextHopCount = newNextHops->size();
                    //                    for (int k = 0; k < nextHopCount; k++) {
                    //                        joiningVertex->addNextHop((*newNextHops)[k]);
                    //                    }
                    //                    delete newNextHops;
                    joiningVertex->setParent(justAddedVertex);

                    candidates.push_back(joiningVertex);
//                    EV_DEBUG << "\t\tcandidates size: " << candidates.size();
                }
            }//for
        }
        /*3)If at this step the candidate list is empty, the shortest-
            path tree (of transit vertices) has been completely built
            and this stage of the procedure terminates.*/
        if(candidates.empty())
            finished=true;
        else{
//            unsigned int candidateCount = candidateVertices.size();
//            unsigned long minDistance = LS_INFINITY;
//            OSPFLSA *closestVertex = candidateVertices[0];
//
//            for (i = 0; i < candidateCount; i++) {
//                RoutingInfo *routingInfo = check_and_cast<RoutingInfo *>(candidateVertices[i]);
//                unsigned long currentDistance = routingInfo->getDistance();
//
//                if (currentDistance < minDistance) {
//                    closestVertex = candidateVertices[i];
//                    minDistance = currentDistance;
//                }
//                else {
//                    if (currentDistance == minDistance) {
//                        if ((closestVertex->getHeader().getLsType() == ROUTERLSA_TYPE) &&
//                                (candidateVertices[i]->getHeader().getLsType() == NETWORKLSA_TYPE))
//                        {
//                            closestVertex = candidateVertices[i];
//                        }
//                    }
//                }
//            }
//
//            treeVertices.push_back(closestVertex);
//
//            for (auto it = candidateVertices.begin(); it != candidateVertices.end(); it++) {
//                if ((*it) == closestVertex) {
//                    candidateVertices.erase(it);
//                    break;
//                }
//            }
//
//            if (closestVertex->getHeader().getLsType() == ROUTERLSA_TYPE) {
//                RouterLSA *routerLSA = check_and_cast<RouterLSA *>(closestVertex);
//                if (routerLSA->getB_AreaBorderRouter() || routerLSA->getE_ASBoundaryRouter()) {
//                    RoutingTableEntry *entry = new RoutingTableEntry(ift);
//                    RouterID destinationID = routerLSA->getHeader().getLinkStateID();
//                    unsigned int nextHopCount = routerLSA->getNextHopCount();
//                    RoutingTableEntry::RoutingDestinationType destinationType = RoutingTableEntry::NETWORK_DESTINATION;
//
//                    entry->setDestination(destinationID);
//                    entry->setLinkStateOrigin(routerLSA);
//                    entry->setArea(areaID);
//                    entry->setPathType(RoutingTableEntry::INTRAAREA);
//                    entry->setCost(routerLSA->getDistance());
//                    if (routerLSA->getB_AreaBorderRouter()) {
//                        destinationType |= RoutingTableEntry::AREA_BORDER_ROUTER_DESTINATION;
//                    }
//                    if (routerLSA->getE_ASBoundaryRouter()) {
//                        destinationType |= RoutingTableEntry::AS_BOUNDARY_ROUTER_DESTINATION;
//                    }
//                    entry->setDestinationType(destinationType);
//                    entry->setOptionalCapabilities(routerLSA->getHeader().getLsOptions());
//                    for (i = 0; i < nextHopCount; i++) {
//                        entry->addNextHop(routerLSA->getNextHop(i));
//                    }
//
//                    newRoutingTable.push_back(entry);
//
//                    Area *backbone;
//                    if (areaID != BACKBONE_AREAID) {
//                        backbone = parentRouter->getAreaByID(BACKBONE_AREAID);
//                    }
//                    else {
//                        backbone = this;
//                    }
//                    if (backbone != nullptr) {
//                        Interface *virtualIntf = backbone->findVirtualLink(destinationID);
//                        if ((virtualIntf != nullptr) && (virtualIntf->getTransitAreaID() == areaID)) {
//                            IPv4AddressRange range;
//                            range.address = getInterface(routerLSA->getNextHop(0).ifIndex)->getAddressRange().address;
//                            range.mask = IPv4Address::ALLONES_ADDRESS;
//                            virtualIntf->setAddressRange(range);
//                            virtualIntf->setIfIndex(ift, routerLSA->getNextHop(0).ifIndex);
//                            virtualIntf->setOutputCost(routerLSA->getDistance());
//                            Neighbor *virtualNeighbor = virtualIntf->getNeighbor(0);
//                            if (virtualNeighbor != nullptr) {
//                                unsigned int linkCount = routerLSA->getLinksArraySize();
//                                RouterLSA *toRouterLSA = dynamic_cast<RouterLSA *>(justAddedVertex);
//                                if (toRouterLSA != nullptr) {
//                                    for (i = 0; i < linkCount; i++) {
//                                        Link& link = routerLSA->getLinks(i);
//
//                                        if ((link.getType() == POINTTOPOINT_LINK) &&
//                                                (link.getLinkID() == toRouterLSA->getHeader().getLinkStateID()) &&
//                                                (virtualIntf->getState() < Interface::WAITING_STATE))
//                                        {
//                                            virtualNeighbor->setAddress(IPv4Address(link.getLinkData()));
//                                            virtualIntf->processEvent(Interface::INTERFACE_UP);
//                                            break;
//                                        }
//                                    }
//                                }
//                                else {
//                                    NetworkLSA *toNetworkLSA = dynamic_cast<NetworkLSA *>(justAddedVertex);
//                                    if (toNetworkLSA != nullptr) {
//                                        for (i = 0; i < linkCount; i++) {
//                                            Link& link = routerLSA->getLinks(i);
//
//                                            if ((link.getType() == TRANSIT_LINK) &&
//                                                    (link.getLinkID() == toNetworkLSA->getHeader().getLinkStateID()) &&
//                                                    (virtualIntf->getState() < Interface::WAITING_STATE))
//                                            {
//                                                virtualNeighbor->setAddress(IPv4Address(link.getLinkData()));
//                                                virtualIntf->processEvent(Interface::INTERFACE_UP);
//                                                break;
//                                            }
//                                        }
//                                    }
//                                }//else
//                            }//(virtualNeighbor != nullptr)
//                        }//if ((virtualIntf != nullptr) && (virtualIntf->getTransitAreaID() == areaID))
//                    }//if(backbone != nullptr) {
//                }//if (routerLSA->getB_AreaBorderRouter() || routerLSA->getE_ASBoundaryRouter())
//            }//if (closestVertex->getHeader().getLsType() == ROUTERLSA_TYPE)
//
//            if (closestVertex->getHeader().getLsType() == NETWORKLSA_TYPE) {
//                NetworkLSA *networkLSA = check_and_cast<NetworkLSA *>(closestVertex);
//                IPv4Address destinationID = (networkLSA->getHeader().getLinkStateID() & networkLSA->getNetworkMask());
//                unsigned int nextHopCount = networkLSA->getNextHopCount();
//                bool overWrite = false;
//                RoutingTableEntry *entry = nullptr;
//                unsigned long routeCount = newRoutingTable.size();
//                IPv4Address longestMatch(0u);
//
//                for (i = 0; i < routeCount; i++) {
//                    if (newRoutingTable[i]->getDestinationType() == RoutingTableEntry::NETWORK_DESTINATION) {
//                        RoutingTableEntry *routingEntry = newRoutingTable[i];
//                        IPv4Address entryAddress = routingEntry->getDestination();
//                        IPv4Address entryMask = routingEntry->getNetmask();
//
//                        if ((entryAddress & entryMask) == (destinationID & entryMask)) {
//                            if ((destinationID & entryMask) > longestMatch) {
//                                longestMatch = (destinationID & entryMask);
//                                entry = routingEntry;
//                            }
//                        }
//                    }
//                }
//                if (entry != nullptr) {
//                    const OSPFLSA *entryOrigin = entry->getLinkStateOrigin();
//                    if ((entry->getCost() != networkLSA->getDistance()) ||
//                            (entryOrigin->getHeader().getLinkStateID() >= networkLSA->getHeader().getLinkStateID()))
//                    {
//                        overWrite = true;
//                    }
//                }
//
//                if ((entry == nullptr) || (overWrite)) {
//                    if (entry == nullptr) {
//                        entry = new RoutingTableEntry(ift);
//                    }
//
//                    entry->setDestination(IPv4Address(destinationID));
//                    entry->setNetmask(networkLSA->getNetworkMask());
//                    entry->setLinkStateOrigin(networkLSA);
//                    entry->setArea(areaID);
//                    entry->setPathType(RoutingTableEntry::INTRAAREA);
//                    entry->setCost(networkLSA->getDistance());
//                    entry->setDestinationType(RoutingTableEntry::NETWORK_DESTINATION);
//                    entry->setOptionalCapabilities(networkLSA->getHeader().getLsOptions());
//                    for (i = 0; i < nextHopCount; i++) {
//                        entry->addNextHop(networkLSA->getNextHop(i));
//                    }
//
//                    if (!overWrite) {
//                        newRoutingTable.push_back(entry);
//                    }
//                }
//            }
//
//            justAddedVertex = closestVertex;
        }//else


        finished = true;
    }while(!finished);





     /*3)Otherwise,
         choose the vertex belonging to the candidate list that is
         closest to the root, and add it to the shortest-path tree
         (removing it from the candidate list in the process). Note
         that when there is a choice of vertices closest to the root,
         network vertices must be chosen before router vertices in
         order to necessarily find all equal-cost paths*/

    /*4)Possibly modify the routing table. For those routing table
        entries modified, the associated area will be set to Area A,
        the path type will be set to intra-area, and the cost will
        be set to the newly discovered shortest path’s calculated
        distance.*/
//    EV_DEBUG << "SPFTree calculation finished\n";
}

void OSPFv3Area::calculateInterAreaRoutes(std::vector<OSPFv3RoutingTableEntry* > newTable)
{
//    EV_DEBUG << "Calculating Inter-Area Routes for Backbone\n";
}

void OSPFv3Area::recheckSummaryLSAs(std::vector<OSPFv3RoutingTableEntry* > newTable)
{
//    EV_DEBUG << "Rechecking Summary LSA\n";
}

std::vector<NextHop> *OSPFv3Area::calculateNextHops(OSPFv3SPFVertex* destination, OSPFv3SPFVertex *parent) const
{
//    EV_DEBUG << "Calculating Next Hops\n";
}

std::vector<NextHop> *OSPFv3Area::calculateNextHops(OSPFv3LSA *destination, OSPFv3LSA *parent) const
{
//    EV_DEBUG << "Calculating Next Hops\n";
}

void OSPFv3Area::debugDump()
{
    for(auto it=this->interfaceList.begin(); it!=this->interfaceList.end(); it++)
        EV_DEBUG << "\t\tinterface id: " << (*it)->getIntName() << "\n";

}//debugDump

std::string OSPFv3Area::detailedInfo() const
{//TODO - adjust so that it pnly prints LSAs that are there
    std::stringstream out;

    out << "OSPFv3 1 address-family ";
    if(this->getInstance()->getAddressFamily()==IPV4INSTANCE)
        out << "ipv4 (router-id ";
    else
        out << "ipv6 (router-id ";

    out << this->getInstance()->getProcess()->getRouterID();
    out << ")\n\n";

    if(this->routerLSAList.size()>0) {
        out << "Router Link States (Area " << this->getAreaID().str(false) << ")\n" ;
        out << "ADV Router\tAge\tSeq#\t\tFragment ID\tLink count\tBits\n";
        for(auto it=this->routerLSAList.begin(); it!=this->routerLSAList.end(); it++) {
            OSPFv3LSAHeader& header = (*it)->getHeader();
            bool bitsEmpty = true;
            out << header.getAdvertisingRouter()<<"\t";
            out << (int)simTime().dbl() - header.getLsaAge() <<"\t0x8000000"<<header.getLsaSequenceNumber()<<"\t0\t\t1\t\t";
            if((*it)->getNtBit()) {
                out << "Nt ";
                bitsEmpty = false;
            }
            if((*it)->getXBit()){
                out << "x ";
                bitsEmpty = false;
            }
            if((*it)->getVBit()){
                out << "V ";
                bitsEmpty = false;
            }
            if((*it)->getEBit()){
                out << "E ";
                bitsEmpty = false;
            }
            if((*it)->getBBit()){
                out << "B";
                bitsEmpty = false;
            }
            if(bitsEmpty)
                out << "None";//TODO Link count and Bits

            out << endl;
        }
    }

    if(this->networkLSAList.size()>0) {
        out << "\nNet Link States (Area " << this->getAreaID().str(false) << ")\n" ;
        out << "ADV Router\tAge\tSeq#\t\tLink State ID\tRtr count\n";
        for(auto it=this->networkLSAList.begin(); it!=this->networkLSAList.end(); it++) {
            OSPFv3LSAHeader& header = (*it)->getHeader();
            out << header.getAdvertisingRouter()<<"\t";
            out << (int)simTime().dbl() - header.getLsaAge()<<"\t0x8000000"<<header.getLsaSequenceNumber()<<"\t"<<header.getLinkStateID().str(false)<<"\t\t" << (*it)->getAttachedRouterArraySize() << "\n";
        }
    }

    if(this->interAreaPrefixLSAList.size()>0) {
        out << "\nInter Area Prefix Link States (Area " << this->getAreaID().str(false) << ")\n" ;
        out << "ADV Router\tAge\tSeq#\t\tPrefix\n";
        for(auto it = this->interAreaPrefixLSAList.begin(); it != this->interAreaPrefixLSAList.end(); it++){
            OSPFv3LSAHeader& header = (*it)->getHeader();
            out << header.getAdvertisingRouter()<<"\t";
            out << (int)simTime().dbl() - header.getLsaAge()<<"\t0x8000000"<<header.getLsaSequenceNumber()<<"\t";

            L3Address addrPref = (*it)->getPrefix();
            if(this->getInstance()->getAddressFamily()==IPV4INSTANCE) {
                IPv4Address ipv4addr = addrPref.toIPv4();
                ipv4addr = ipv4addr.getNetwork();
                out << ipv4addr << "/" << (*it)->getPrefixLen() << endl;
            }
            else if(this->getInstance()->getAddressFamily()==IPV6INSTANCE) {
                IPv6Address ipv6addr = addrPref.toIPv6();
                ipv6addr = ipv6addr.getPrefix((*it)->getPrefixLen());
                if(ipv6addr == IPv6Address::UNSPECIFIED_ADDRESS)
                    out << "::/0" << endl; //TODO - this needs to be changed to the actual length
                else
                    out << ipv6addr << "/64" << endl;
            }
        }
    }

    out << "\nLink (Type-8) Link States (Area " << this->getAreaID().str(false) << ")\n" ;
    out << "ADV Router\tAge\tSeq#\t\tLink State ID\tInterface\n";
    for(auto it=this->interfaceList.begin(); it!=this->interfaceList.end(); it++) {
        int linkLSACount = (*it)->getLinkLSACount();
        for(int i = 0; i<linkLSACount; i++) {
            OSPFv3LSAHeader& header = (*it)->getLinkLSA(i)->getHeader();
            out << header.getAdvertisingRouter()<<"\t";
            out << (int)simTime().dbl() - header.getLsaAge()<<"\t0x8000000"<<header.getLsaSequenceNumber()<<"\t"<<header.getLinkStateID().str(false)<<"\t\t"<< (*it)->getIntName() << "\n";
        }
    }

    if(this->intraAreaPrefixLSAList.size()>0) {
        out << "\nIntra Area Prefix Link States (Area" << this->getAreaID().str(false) << ")\n" ;
        out << "ADV Router\tAge\tSeq#\t\tLink ID\t\tRef-lstype\tRef-LSID\n";
        for(auto it=this->intraAreaPrefixLSAList.begin(); it!=this->intraAreaPrefixLSAList.end(); it++) {
            OSPFv3LSAHeader& header = (*it)->getHeader();
            out << header.getAdvertisingRouter() << "\t";
            out << (int)simTime().dbl() - header.getLsaAge() << "\t0x8000000" << header.getLsaSequenceNumber() << "\t" << header.getLinkStateID().str(false) << "\t\t0x200" << (*it)->getReferencedLSType() << "\t\t" << (*it)->getReferencedLSID().str(false)<<"\n";
        }
    }

    return out.str();

}//detailedInfo
}//namespace inet
