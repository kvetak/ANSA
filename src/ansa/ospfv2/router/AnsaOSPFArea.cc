
#include "AnsaOSPFArea.h"
#include "AnsaOSPFRouter.h"
#include <memory.h>

AnsaOSPF::Area::Area(AnsaOSPF::AreaID id) :
    areaID(id),
    transitCapability(false),
    externalRoutingCapability(true),
    stubDefaultCost(1),
    spfTreeRoot(NULL),
    parentRouter(NULL)
{
}

AnsaOSPF::Area::~Area(void)
{
    int interfaceNum = associatedInterfaces.size();
    for (int i = 0; i < interfaceNum; i++) {
        delete(associatedInterfaces[i]);
    }
    long lsaCount = routerLSAs.size();
    for (long j = 0; j < lsaCount; j++) {
        delete routerLSAs[j];
    }
    routerLSAs.clear();
    lsaCount = networkLSAs.size();
    for (long k = 0; k < lsaCount; k++) {
        delete networkLSAs[k];
    }
    networkLSAs.clear();
    lsaCount = summaryLSAs.size();
    for (long m = 0; m < lsaCount; m++) {
        delete summaryLSAs[m];
    }
    summaryLSAs.clear();
}

void AnsaOSPF::Area::AddInterface(AnsaOSPF::Interface* intf)
{
    intf->SetArea(this);
    associatedInterfaces.push_back(intf);
}

void AnsaOSPF::Area::info(char *buffer)
{
    std::stringstream out;
    char areaString[16];
    out << "areaID: " << AddressStringFromULong(areaString, 16, areaID);
    strcpy(buffer, out.str().c_str());
}

std::string AnsaOSPF::Area::detailedInfo(void) const
{
    std::stringstream out;
    char addressString[16];
    int i;
    out << "\n    areaID: " << AddressStringFromULong(addressString, 16, areaID) << ", ";
    out << "transitCapability: " << (transitCapability ? "true" : "false") << ", ";
    out << "externalRoutingCapability: " << (externalRoutingCapability ? "true" : "false") << ", ";
    out << "stubDefaultCost: " << stubDefaultCost << "\n";
    int addressRangeNum = areaAddressRanges.size();
    for (i = 0; i < addressRangeNum; i++) {
        out << "    addressRanges[" << i << "]: ";
        out << AddressStringFromIPv4Address(addressString, 16, areaAddressRanges[i].address);
        out << "/" << AddressStringFromIPv4Address(addressString, 16, areaAddressRanges[i].mask) << "\n";
    }
    int interfaceNum = associatedInterfaces.size();
    for (i = 0; i < interfaceNum; i++) {
        out << "    interface[" << i << "]: addressRange: ";
        out << AddressStringFromIPv4Address(addressString, 16, associatedInterfaces[i]->GetAddressRange().address);
        out << "/" << AddressStringFromIPv4Address(addressString, 16, associatedInterfaces[i]->GetAddressRange().mask) << "\n";
    }

    out << "\n";
    out << "    Database:\n";
    out << "      RouterLSAs:\n";
    long lsaCount = routerLSAs.size();
    for (i = 0; i < lsaCount; i++) {
        out << "        " << *routerLSAs[i] << "\n";
    }
    out << "      NetworkLSAs:\n";
    lsaCount = networkLSAs.size();
    for (i = 0; i < lsaCount; i++) {
        out << "        " << *networkLSAs[i] << "\n";
    }
    out << "      SummaryLSAs:\n";
    lsaCount = summaryLSAs.size();
    for (i = 0; i < lsaCount; i++) {
        out << "        " << *summaryLSAs[i] << "\n";
    }

    out << "--------------------------------------------------------------------------------";

    return out.str();
}

bool AnsaOSPF::Area::ContainsAddress(AnsaOSPF::IPv4Address address) const
{
    int addressRangeNum = areaAddressRanges.size();
    for (int i = 0; i < addressRangeNum; i++) {
        if ((areaAddressRanges[i].address & areaAddressRanges[i].mask) == (address & areaAddressRanges[i].mask)) {
            return true;
        }
    }
    return false;
}

bool AnsaOSPF::Area::HasAddressRange(AnsaOSPF::IPv4AddressRange addressRange) const
{
    int addressRangeNum = areaAddressRanges.size();
    for (int i = 0; i < addressRangeNum; i++) {
        if ((areaAddressRanges[i].address == addressRange.address) &&
            (areaAddressRanges[i].mask == addressRange.mask))
        {
            return true;
        }
    }
    return false;
}

AnsaOSPF::IPv4AddressRange AnsaOSPF::Area::GetContainingAddressRange(AnsaOSPF::IPv4AddressRange addressRange, bool* advertise /*= NULL*/) const
{
    int addressRangeNum = areaAddressRanges.size();
    for (int i = 0; i < addressRangeNum; i++) {
        if ((areaAddressRanges[i].address & areaAddressRanges[i].mask) == (addressRange.address & areaAddressRanges[i].mask)) {
            if (advertise != NULL) {
                std::map<AnsaOSPF::IPv4AddressRange, bool, AnsaOSPF::IPv4AddressRange_Less>::const_iterator rangeIt = advertiseAddressRanges.find(areaAddressRanges[i]);
                if (rangeIt != advertiseAddressRanges.end()) {
                    *advertise = rangeIt->second;
                } else {
                    *advertise = true;
                }
            }
            return areaAddressRanges[i];
        }
    }
    if (advertise != NULL) {
        *advertise =  false;
    }
    return NullIPv4AddressRange;
}

AnsaOSPF::Interface*  AnsaOSPF::Area::GetInterface(unsigned char ifIndex)
{
    int interfaceNum = associatedInterfaces.size();
    for (int i = 0; i < interfaceNum; i++) {
        if ((associatedInterfaces[i]->GetType() != AnsaOSPF::Interface::Virtual) &&
            (associatedInterfaces[i]->GetIfIndex() == ifIndex))
        {
            return associatedInterfaces[i];
        }
    }
    return NULL;
}

AnsaOSPF::Interface*  AnsaOSPF::Area::GetInterface(AnsaOSPF::IPv4Address address)
{
    int interfaceNum = associatedInterfaces.size();
    for (int i = 0; i < interfaceNum; i++) {
        if ((associatedInterfaces[i]->GetType() != AnsaOSPF::Interface::Virtual) &&
            (associatedInterfaces[i]->GetAddressRange().address == address))
        {
            return associatedInterfaces[i];
        }
    }
    return NULL;
}

bool AnsaOSPF::Area::HasVirtualLink(AnsaOSPF::AreaID withTransitArea) const
{
    if ((areaID != AnsaOSPF::BackboneAreaID) || (withTransitArea == AnsaOSPF::BackboneAreaID)) {
        return false;
    }

    int interfaceNum = associatedInterfaces.size();
    for (int i = 0; i < interfaceNum; i++) {
        if ((associatedInterfaces[i]->GetType() == AnsaOSPF::Interface::Virtual) &&
            (associatedInterfaces[i]->GetTransitAreaID() == withTransitArea))
        {
            return true;
        }
    }
    return false;
}


AnsaOSPF::Interface*  AnsaOSPF::Area::FindVirtualLink(AnsaOSPF::RouterID routerID)
{
    int interfaceNum = associatedInterfaces.size();
    for (int i = 0; i < interfaceNum; i++) {
        if ((associatedInterfaces[i]->GetType() == AnsaOSPF::Interface::Virtual) &&
            (associatedInterfaces[i]->GetNeighborByID(routerID) != NULL))
        {
            return associatedInterfaces[i];
        }
    }
    return NULL;
}

bool AnsaOSPF::Area::InstallRouterLSA(OSPFRouterLSA* lsa)
{
    AnsaOSPF::LinkStateID linkStateID = lsa->getHeader().getLinkStateID();
    std::map<AnsaOSPF::LinkStateID, AnsaOSPF::RouterLSA*>::iterator lsaIt = routerLSAsByID.find(linkStateID);
    if (lsaIt != routerLSAsByID.end()) {
        AnsaOSPF::LSAKeyType lsaKey;

        lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
        lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter().getInt();

        RemoveFromAllRetransmissionLists(lsaKey);
        return lsaIt->second->Update(lsa);
    } else {
        AnsaOSPF::RouterLSA* lsaCopy = new AnsaOSPF::RouterLSA(*lsa);
        routerLSAsByID[linkStateID] = lsaCopy;
        routerLSAs.push_back(lsaCopy);
        return true;
    }
}

bool AnsaOSPF::Area::InstallNetworkLSA(OSPFNetworkLSA* lsa)
{
    AnsaOSPF::LinkStateID linkStateID = lsa->getHeader().getLinkStateID();
    std::map<AnsaOSPF::LinkStateID, AnsaOSPF::NetworkLSA*>::iterator lsaIt = networkLSAsByID.find(linkStateID);
    if (lsaIt != networkLSAsByID.end()) {
        AnsaOSPF::LSAKeyType lsaKey;

        lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
        lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter().getInt();

        RemoveFromAllRetransmissionLists(lsaKey);
        return lsaIt->second->Update(lsa);
    } else {
        AnsaOSPF::NetworkLSA* lsaCopy = new AnsaOSPF::NetworkLSA(*lsa);
        networkLSAsByID[linkStateID] = lsaCopy;
        networkLSAs.push_back(lsaCopy);
        return true;
    }
}

bool AnsaOSPF::Area::InstallSummaryLSA(OSPFSummaryLSA* lsa)
{
    AnsaOSPF::LSAKeyType lsaKey;

    lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
    lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter().getInt();

    std::map<AnsaOSPF::LSAKeyType, AnsaOSPF::SummaryLSA*, AnsaOSPF::LSAKeyType_Less>::iterator lsaIt = summaryLSAsByID.find(lsaKey);
    if (lsaIt != summaryLSAsByID.end()) {
        AnsaOSPF::LSAKeyType lsaKey;

        lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
        lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter().getInt();

        RemoveFromAllRetransmissionLists(lsaKey);
        return lsaIt->second->Update(lsa);
    } else {
        AnsaOSPF::SummaryLSA* lsaCopy = new AnsaOSPF::SummaryLSA(*lsa);
        summaryLSAsByID[lsaKey] = lsaCopy;
        summaryLSAs.push_back(lsaCopy);
        return true;
    }
}

AnsaOSPF::RouterLSA* AnsaOSPF::Area::FindRouterLSA(AnsaOSPF::LinkStateID linkStateID)
{
    std::map<AnsaOSPF::LinkStateID, AnsaOSPF::RouterLSA*>::iterator lsaIt = routerLSAsByID.find(linkStateID);
    if (lsaIt != routerLSAsByID.end()) {
        return lsaIt->second;
    } else {
        return NULL;
    }
}

const AnsaOSPF::RouterLSA* AnsaOSPF::Area::FindRouterLSA(AnsaOSPF::LinkStateID linkStateID) const
{
    std::map<AnsaOSPF::LinkStateID, AnsaOSPF::RouterLSA*>::const_iterator lsaIt = routerLSAsByID.find(linkStateID);
    if (lsaIt != routerLSAsByID.end()) {
        return lsaIt->second;
    } else {
        return NULL;
    }
}

AnsaOSPF::NetworkLSA* AnsaOSPF::Area::FindNetworkLSA(AnsaOSPF::LinkStateID linkStateID)
{
    std::map<AnsaOSPF::LinkStateID, AnsaOSPF::NetworkLSA*>::iterator lsaIt = networkLSAsByID.find(linkStateID);
    if (lsaIt != networkLSAsByID.end()) {
        return lsaIt->second;
    } else {
        return NULL;
    }
}

const AnsaOSPF::NetworkLSA* AnsaOSPF::Area::FindNetworkLSA(AnsaOSPF::LinkStateID linkStateID) const
{
    std::map<AnsaOSPF::LinkStateID, AnsaOSPF::NetworkLSA*>::const_iterator lsaIt = networkLSAsByID.find(linkStateID);
    if (lsaIt != networkLSAsByID.end()) {
        return lsaIt->second;
    } else {
        return NULL;
    }
}

AnsaOSPF::SummaryLSA* AnsaOSPF::Area::FindSummaryLSA(AnsaOSPF::LSAKeyType lsaKey)
{
    std::map<AnsaOSPF::LSAKeyType, AnsaOSPF::SummaryLSA*, AnsaOSPF::LSAKeyType_Less>::iterator lsaIt = summaryLSAsByID.find(lsaKey);
    if (lsaIt != summaryLSAsByID.end()) {
        return lsaIt->second;
    } else {
        return NULL;
    }
}

const AnsaOSPF::SummaryLSA* AnsaOSPF::Area::FindSummaryLSA(AnsaOSPF::LSAKeyType lsaKey) const
{
    std::map<AnsaOSPF::LSAKeyType, AnsaOSPF::SummaryLSA*, AnsaOSPF::LSAKeyType_Less>::const_iterator lsaIt = summaryLSAsByID.find(lsaKey);
    if (lsaIt != summaryLSAsByID.end()) {
        return lsaIt->second;
    } else {
        return NULL;
    }
}

void AnsaOSPF::Area::RemoveParentFromRoutingInfo(OSPFLSA* parent)
{
    long            lsaCount            = routerLSAs.size();
    long            i;
    
    for (i = 0; i < lsaCount; i++) 
    {
      if(routerLSAs[i] != NULL)
      {
        AnsaOSPF::RouterLSA* routerLSA     = routerLSAs[i];
        AnsaOSPF::RoutingInfo* routingInfo = check_and_cast<AnsaOSPF::RoutingInfo*> (routerLSA);
        
        if(routingInfo->GetParent() == parent)
          routingInfo->SetParent(NULL);
      }
    }
    
    lsaCount = networkLSAs.size();
    
    for (i = 0; i < lsaCount; i++) 
    {
      if(networkLSAs[i] != NULL)
      {
        AnsaOSPF::NetworkLSA* networkLSA     = networkLSAs[i];
        AnsaOSPF::RoutingInfo* routingInfo = check_and_cast<AnsaOSPF::RoutingInfo*> (networkLSA);
        
        if(routingInfo->GetParent() == parent)
          routingInfo->SetParent(NULL);
      }
    }
    
    lsaCount = summaryLSAs.size();
    
    for (i = 0; i < lsaCount; i++) 
    {
      if(summaryLSAs[i] != NULL)
      {
        AnsaOSPF::SummaryLSA* summaryLSA     = summaryLSAs[i];
        AnsaOSPF::RoutingInfo* routingInfo = check_and_cast<AnsaOSPF::RoutingInfo*> (summaryLSA);
        
        if(routingInfo->GetParent() == parent)
          routingInfo->SetParent(NULL);
      }
    }
}

void AnsaOSPF::Area::AgeDatabase(void)
{
    long            lsaCount            = routerLSAs.size();
    bool            rebuildRoutingTable = false;
    long            i;

    IPAddress routerId(GetRouter()->GetRouterID());
    EV << routerId.str();


    for (i = 0; i < lsaCount; i++) {
        unsigned short   lsAge          = routerLSAs[i]->getHeader().getLsAge();
        bool             selfOriginated = (routerLSAs[i]->getHeader().getAdvertisingRouter().getInt() == parentRouter->GetRouterID());
        bool             unreachable    = parentRouter->IsDestinationUnreachable(routerLSAs[i]);
        AnsaOSPF::RouterLSA* lsa            = routerLSAs[i];
        

        char addressString[16];
        unsigned int linkCount = lsa->getLinksArraySize();
        for (unsigned int j = 0; j < linkCount; j++) {
            const Link& link = lsa->getLinks(j);
            EV << "    ID="
               << AddressStringFromULong(addressString, sizeof(addressString), link.getLinkID().getInt())
               << ",";
            EV << " data="
               << AddressStringFromULong(addressString, sizeof(addressString), link.getLinkData())
               << ", type=";
            switch (link.getType()) {
                case PointToPointLink:  EV << "PointToPoint";   break;
                case TransitLink:       EV << "Transit";        break;
                case StubLink:          EV << "Stub";           break;
                case VirtualLink:       EV << "Virtual";        break;
                default:                EV << "Unknown";        break;
            }
            EV << ", cost="
               << link.getLinkCost()
               << "\n";
        }

        if ((selfOriginated && (lsAge < (LS_REFRESH_TIME - 1))) || (!selfOriginated && (lsAge < (MAX_AGE - 1)))) {
            lsa->getHeader().setLsAge(lsAge + 1);
            if ((lsAge + 1) % CHECK_AGE == 0) {
                if (!lsa->ValidateLSChecksum()) {
                    EV << "Invalid LS checksum. Memory error detected!\n";
                }
            }
            lsa->IncrementInstallTime();
        }
        if (selfOriginated && (lsAge == (LS_REFRESH_TIME - 1))) {
            if (unreachable) {
                lsa->getHeader().setLsAge(MAX_AGE);
                FloodLSA(lsa);
                lsa->IncrementInstallTime();
            } else {
                long sequenceNumber = lsa->getHeader().getLsSequenceNumber();
                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                    lsa->getHeader().setLsAge(MAX_AGE);
                    FloodLSA(lsa);
                    lsa->IncrementInstallTime();
                } else {
                    AnsaOSPF::RouterLSA* newLSA = OriginateRouterLSA();

                    newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                    newLSA->getHeader().setLsChecksum(0);    // TODO: calculate correct LS checksum
                    rebuildRoutingTable |= lsa->Update(newLSA);
                    delete newLSA;

                    FloodLSA(lsa);
                }
            }
        }
        if (!selfOriginated && (lsAge == MAX_AGE - 1)) {
            lsa->getHeader().setLsAge(MAX_AGE);
            FloodLSA(lsa);
            lsa->IncrementInstallTime();
        }
        if (lsAge == MAX_AGE) {
            AnsaOSPF::LSAKeyType lsaKey;

            lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
            lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter().getInt();

            if (!IsOnAnyRetransmissionList(lsaKey) &&
                !HasAnyNeighborInStates(AnsaOSPF::Neighbor::ExchangeState | AnsaOSPF::Neighbor::LoadingState))
            {
                if (!selfOriginated || unreachable) {
                    routerLSAsByID.erase(lsa->getHeader().getLinkStateID());
                    delete lsa;
                    routerLSAs[i] = NULL;
                    rebuildRoutingTable = true;
                } else {
                    AnsaOSPF::RouterLSA* newLSA              = OriginateRouterLSA();
                    long             sequenceNumber      = lsa->getHeader().getLsSequenceNumber();

                    newLSA->getHeader().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
                    newLSA->getHeader().setLsChecksum(0);    // TODO: calculate correct LS checksum
                    rebuildRoutingTable |= lsa->Update(newLSA);
                    delete newLSA;

                    FloodLSA(lsa);
                }
            }
        }
    }
    

    std::vector<RouterLSA*>::iterator routerIt = routerLSAs.begin();
    while (routerIt != routerLSAs.end()) {
        if ((*routerIt) == NULL) {
            routerIt = routerLSAs.erase(routerIt);
        } else {
            routerIt++;
        }
    }

    lsaCount = networkLSAs.size();
    for (i = 0; i < lsaCount; i++) {
        unsigned short    lsAge          = networkLSAs[i]->getHeader().getLsAge();
        bool              unreachable    = parentRouter->IsDestinationUnreachable(networkLSAs[i]);
        AnsaOSPF::NetworkLSA* lsa            = networkLSAs[i];
        AnsaOSPF::Interface*  localIntf      = GetInterface(IPv4AddressFromULong(lsa->getHeader().getLinkStateID()));
        bool              selfOriginated = false;
        

        if ((localIntf != NULL) &&
            (localIntf->GetState() == AnsaOSPF::Interface::DesignatedRouterState) &&
            (localIntf->GetNeighborCount() > 0) &&
            (localIntf->HasAnyNeighborInStates(AnsaOSPF::Neighbor::FullState)))
        {
            selfOriginated = true;
        }

        if ((selfOriginated && (lsAge < (LS_REFRESH_TIME - 1))) || (!selfOriginated && (lsAge < (MAX_AGE - 1)))) {
            lsa->getHeader().setLsAge(lsAge + 1);
            if ((lsAge + 1) % CHECK_AGE == 0) {
                if (!lsa->ValidateLSChecksum()) {
                    EV << "Invalid LS checksum. Memory error detected!\n";
                }
            }
            lsa->IncrementInstallTime();
        }
        if (selfOriginated && (lsAge == (LS_REFRESH_TIME - 1))) {
            if (unreachable) {
                lsa->getHeader().setLsAge(MAX_AGE);
                FloodLSA(lsa);
                lsa->IncrementInstallTime();
            } else {
                long sequenceNumber = lsa->getHeader().getLsSequenceNumber();
                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                    lsa->getHeader().setLsAge(MAX_AGE);
                    FloodLSA(lsa);
                    lsa->IncrementInstallTime();
                } else {
                    AnsaOSPF::NetworkLSA* newLSA = OriginateNetworkLSA(localIntf);

                    if (newLSA != NULL) {
                        newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                        newLSA->getHeader().setLsChecksum(0);    // TODO: calculate correct LS checksum
                        rebuildRoutingTable |= lsa->Update(newLSA);
                        delete newLSA;
                    } else {    // no neighbors on the network -> old NetworkLSA must be flushed
                        lsa->getHeader().setLsAge(MAX_AGE);
                        lsa->IncrementInstallTime();
                    }

                    FloodLSA(lsa);
                }
            }
        }
        if (!selfOriginated && (lsAge == MAX_AGE - 1)) {
            lsa->getHeader().setLsAge(MAX_AGE);
            FloodLSA(lsa);
            lsa->IncrementInstallTime();
        }
        if (lsAge == MAX_AGE) {
            AnsaOSPF::LSAKeyType lsaKey;

            lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
            lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter().getInt();

            if (!IsOnAnyRetransmissionList(lsaKey) &&
                !HasAnyNeighborInStates(AnsaOSPF::Neighbor::ExchangeState | AnsaOSPF::Neighbor::LoadingState))
            {
                if (!selfOriginated || unreachable) {
                    networkLSAsByID.erase(lsa->getHeader().getLinkStateID());
                    RemoveParentFromRoutingInfo(check_and_cast<OSPFLSA*> (lsa));
                    delete lsa;
                    networkLSAs[i] = NULL;
                    rebuildRoutingTable = true;
                } else {
                    AnsaOSPF::NetworkLSA* newLSA              = OriginateNetworkLSA(localIntf);
                    long              sequenceNumber      = lsa->getHeader().getLsSequenceNumber();

                    if (newLSA != NULL) {
                        newLSA->getHeader().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
                        newLSA->getHeader().setLsChecksum(0);    // TODO: calculate correct LS checksum
                        rebuildRoutingTable |= lsa->Update(newLSA);
                        delete newLSA;

                        FloodLSA(lsa);
                    } else {    // no neighbors on the network -> old NetworkLSA must be deleted
                        delete networkLSAs[i];
                    }
                }
            }
        }
    }
    std::vector<NetworkLSA*>::iterator networkIt = networkLSAs.begin();
    while (networkIt != networkLSAs.end()) {
        if ((*networkIt) == NULL) {
            networkIt = networkLSAs.erase(networkIt);
        } else {
            networkIt++;
        }
    }

    lsaCount = summaryLSAs.size();
    for (i = 0; i < lsaCount; i++) {
        unsigned short    lsAge          = summaryLSAs[i]->getHeader().getLsAge();
        bool              selfOriginated = (summaryLSAs[i]->getHeader().getAdvertisingRouter().getInt() == parentRouter->GetRouterID());
        bool              unreachable    = parentRouter->IsDestinationUnreachable(summaryLSAs[i]);
        AnsaOSPF::SummaryLSA* lsa            = summaryLSAs[i];
        

        if ((selfOriginated && (lsAge < (LS_REFRESH_TIME - 1))) || (!selfOriginated && (lsAge < (MAX_AGE - 1)))) {
            lsa->getHeader().setLsAge(lsAge + 1);
            if ((lsAge + 1) % CHECK_AGE == 0) {
                if (!lsa->ValidateLSChecksum()) {
                    EV << "Invalid LS checksum. Memory error detected!\n";
                }
            }
            lsa->IncrementInstallTime();
        }
        if (selfOriginated && (lsAge == (LS_REFRESH_TIME - 1))) {
            if (unreachable) {
                lsa->getHeader().setLsAge(MAX_AGE);
                FloodLSA(lsa);
                lsa->IncrementInstallTime();
            } else {
                long sequenceNumber = lsa->getHeader().getLsSequenceNumber();
                if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                    lsa->getHeader().setLsAge(MAX_AGE);
                    FloodLSA(lsa);
                    lsa->IncrementInstallTime();
                } else {
                    AnsaOSPF::SummaryLSA* newLSA = OriginateSummaryLSA(lsa);

                    if (newLSA != NULL) {
                        newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
                        newLSA->getHeader().setLsChecksum(0);    // TODO: calculate correct LS checksum
                        rebuildRoutingTable |= lsa->Update(newLSA);
                        delete newLSA;

                        FloodLSA(lsa);
                    } else {
                        lsa->getHeader().setLsAge(MAX_AGE);
                        FloodLSA(lsa);
                        lsa->IncrementInstallTime();
                    }
                }
            }
        }
        if (!selfOriginated && (lsAge == MAX_AGE - 1)) {
            lsa->getHeader().setLsAge(MAX_AGE);
            FloodLSA(lsa);
            lsa->IncrementInstallTime();
        }
        if (lsAge == MAX_AGE) {
            AnsaOSPF::LSAKeyType lsaKey;

            lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
            lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter().getInt();

            if (!IsOnAnyRetransmissionList(lsaKey) &&
                !HasAnyNeighborInStates(AnsaOSPF::Neighbor::ExchangeState | AnsaOSPF::Neighbor::LoadingState))
            {
                if (!selfOriginated || unreachable) {
                    summaryLSAsByID.erase(lsaKey);
                    delete lsa;
                    summaryLSAs[i] = NULL;
                    rebuildRoutingTable = true;
                } else {
                    AnsaOSPF::SummaryLSA* newLSA = OriginateSummaryLSA(lsa);
                    if (newLSA != NULL) {
                        long sequenceNumber = lsa->getHeader().getLsSequenceNumber();

                        newLSA->getHeader().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
                        newLSA->getHeader().setLsChecksum(0);    // TODO: calculate correct LS checksum
                        rebuildRoutingTable |= lsa->Update(newLSA);
                        delete newLSA;

                        FloodLSA(lsa);
                    } else {
                        summaryLSAsByID.erase(lsaKey);
                        delete lsa;
                        summaryLSAs[i] = NULL;
                        rebuildRoutingTable = true;
                    }
                }
            }
        }
    }

    std::vector<SummaryLSA*>::iterator summaryIt = summaryLSAs.begin();
    while (summaryIt != summaryLSAs.end()) {
        if ((*summaryIt) == NULL) {
            summaryIt = summaryLSAs.erase(summaryIt);
        } else {
            summaryIt++;
        }
    }

    long interfaceCount = associatedInterfaces.size();
    for (long m = 0; m < interfaceCount; m++) {
        associatedInterfaces[m]->AgeTransmittedLSALists();
    }

    if (rebuildRoutingTable) {
        parentRouter->RebuildRoutingTable();
    }
}

bool AnsaOSPF::Area::HasAnyNeighborInStates(int states) const
{
    long interfaceCount = associatedInterfaces.size();
    for (long i = 0; i < interfaceCount; i++) {
        if (associatedInterfaces[i]->HasAnyNeighborInStates(states)) {
            return true;
        }
    }
    return false;
}

void AnsaOSPF::Area::RemoveFromAllRetransmissionLists(AnsaOSPF::LSAKeyType lsaKey)
{
    long interfaceCount = associatedInterfaces.size();
    for (long i = 0; i < interfaceCount; i++) {
        associatedInterfaces[i]->RemoveFromAllRetransmissionLists(lsaKey);
    }
}

bool AnsaOSPF::Area::IsOnAnyRetransmissionList(AnsaOSPF::LSAKeyType lsaKey) const
{
    long interfaceCount = associatedInterfaces.size();
    for (long i = 0; i < interfaceCount; i++) {
        if (associatedInterfaces[i]->IsOnAnyRetransmissionList(lsaKey)) {
            return true;
        }
    }
    return false;
}

bool AnsaOSPF::Area::FloodLSA(OSPFLSA* lsa, AnsaOSPF::Interface* intf, AnsaOSPF::Neighbor* neighbor)
{
    bool floodedBackOut  = false;
    long interfaceCount = associatedInterfaces.size();

    for (long i = 0; i < interfaceCount; i++) {
        if (associatedInterfaces[i]->FloodLSA(lsa, intf, neighbor)) {
            floodedBackOut = true;
        }
    }

    return floodedBackOut;
}

bool AnsaOSPF::Area::IsLocalAddress(AnsaOSPF::IPv4Address address) const
{
    long interfaceCount = associatedInterfaces.size();
    for (long i = 0; i < interfaceCount; i++) {
        if (associatedInterfaces[i]->GetAddressRange().address == address) {
            return true;
        }
    }
    return false;
}

AnsaOSPF::RouterLSA* AnsaOSPF::Area::OriginateRouterLSA(void)
{
    AnsaOSPF::RouterLSA* routerLSA      = new AnsaOSPF::RouterLSA;
    OSPFLSAHeader&   lsaHeader      = routerLSA->getHeader();
    long             interfaceCount = associatedInterfaces.size();
    OSPFOptions      lsOptions;
    long             i;

    lsaHeader.setLsAge(0);
    memset(&lsOptions, 0, sizeof(OSPFOptions));
    lsOptions.E_ExternalRoutingCapability = externalRoutingCapability;
    lsaHeader.setLsOptions(lsOptions);
    lsaHeader.setLsType(RouterLSAType);
    lsaHeader.setLinkStateID(parentRouter->GetRouterID());
    lsaHeader.setAdvertisingRouter(parentRouter->GetRouterID());
    lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

    routerLSA->setB_AreaBorderRouter(parentRouter->GetAreaCount() > 1);
    routerLSA->setE_ASBoundaryRouter((externalRoutingCapability && parentRouter->GetASBoundaryRouter()) ? true : false);
    AnsaOSPF::Area* backbone = parentRouter->GetArea(AnsaOSPF::BackboneAreaID);
    routerLSA->setV_VirtualLinkEndpoint((backbone == NULL) ? false : backbone->HasVirtualLink(areaID));

    routerLSA->setNumberOfLinks(0);
    routerLSA->setLinksArraySize(0);
    for (i = 0; i < interfaceCount; i++) {
        AnsaOSPF::Interface* intf = associatedInterfaces[i];

        if (intf->GetState() == AnsaOSPF::Interface::DownState) {
            continue;
        }
        if ((intf->GetState() == AnsaOSPF::Interface::LoopbackState) &&
            ((intf->GetType() != AnsaOSPF::Interface::PointToPoint) ||
             (intf->GetAddressRange().address != AnsaOSPF::NullIPv4Address)))
        {
            Link stubLink;
            stubLink.setType(StubLink);
            stubLink.setLinkID(ULongFromIPv4Address(intf->GetAddressRange().address));
            stubLink.setLinkData(0xFFFFFFFF);
            stubLink.setLinkCost(0);
            stubLink.setNumberOfTOS(0);
            stubLink.setTosDataArraySize(0);

            unsigned short linkIndex = routerLSA->getLinksArraySize();
            routerLSA->setLinksArraySize(linkIndex + 1);
            routerLSA->setNumberOfLinks(linkIndex + 1);
            routerLSA->setLinks(linkIndex, stubLink);
        }
        if (intf->GetState() > AnsaOSPF::Interface::LoopbackState) {
            switch (intf->GetType()) {
                case AnsaOSPF::Interface::PointToPoint:
                    {
                        AnsaOSPF::Neighbor* neighbor = (intf->GetNeighborCount() > 0) ? intf->GetNeighbor(0) : NULL;
                        if (neighbor != NULL) {
                            if (neighbor->GetState() == AnsaOSPF::Neighbor::FullState) {
                                Link link;
                                link.setType(PointToPointLink);
                                link.setLinkID(neighbor->GetNeighborID());
                                if (intf->GetAddressRange().address != AnsaOSPF::NullIPv4Address) {
                                    link.setLinkData(ULongFromIPv4Address(intf->GetAddressRange().address));
                                } else {
                                    link.setLinkData(intf->GetIfIndex());
                                }
                                link.setLinkCost(intf->GetOutputCost());
                                link.setNumberOfTOS(0);
                                link.setTosDataArraySize(0);

                                unsigned short linkIndex = routerLSA->getLinksArraySize();
                                routerLSA->setLinksArraySize(linkIndex + 1);
                                routerLSA->setNumberOfLinks(linkIndex + 1);
                                routerLSA->setLinks(linkIndex, link);
                            }
                            if (intf->GetState() == AnsaOSPF::Interface::PointToPointState) {
                                if (neighbor->GetAddress() != AnsaOSPF::NullIPv4Address) {
                                    Link stubLink;
                                    stubLink.setType(StubLink);
                                    stubLink.setLinkID(ULongFromIPv4Address(neighbor->GetAddress()));
                                    stubLink.setLinkData(ULongFromIPv4Address(intf->GetAddressRange().mask));
                                    stubLink.setLinkCost(intf->GetOutputCost());
                                    stubLink.setNumberOfTOS(0);
                                    stubLink.setTosDataArraySize(0);

                                    unsigned short linkIndex = routerLSA->getLinksArraySize();
                                    routerLSA->setLinksArraySize(linkIndex + 1);
                                    routerLSA->setNumberOfLinks(linkIndex + 1);
                                    routerLSA->setLinks(linkIndex, stubLink);
                                } else {
                                    if (ULongFromIPv4Address(intf->GetAddressRange().mask) != 0xFFFFFFFF) {
                                        Link stubLink;
                                        stubLink.setType(StubLink);
                                        stubLink.setLinkID(ULongFromIPv4Address(intf->GetAddressRange().address &
                                                                                  intf->GetAddressRange().mask));
                                        stubLink.setLinkData(ULongFromIPv4Address(intf->GetAddressRange().mask));
                                        stubLink.setLinkCost(intf->GetOutputCost());
                                        stubLink.setNumberOfTOS(0);
                                        stubLink.setTosDataArraySize(0);

                                        unsigned short linkIndex = routerLSA->getLinksArraySize();
                                        routerLSA->setLinksArraySize(linkIndex + 1);
                                        routerLSA->setNumberOfLinks(linkIndex + 1);
                                        routerLSA->setLinks(linkIndex, stubLink);
                                    }
                                }
                            }
                        }
                    }
                    break;
                case AnsaOSPF::Interface::Broadcast:
                case AnsaOSPF::Interface::NBMA:
                    {
                        if (intf->GetState() == AnsaOSPF::Interface::WaitingState) {
                            Link stubLink;
                            stubLink.setType(StubLink);
                            stubLink.setLinkID(ULongFromIPv4Address(intf->GetAddressRange().address &
                                                                      intf->GetAddressRange().mask));
                            stubLink.setLinkData(ULongFromIPv4Address(intf->GetAddressRange().mask));
                            stubLink.setLinkCost(intf->GetOutputCost());
                            stubLink.setNumberOfTOS(0);
                            stubLink.setTosDataArraySize(0);

                            unsigned short linkIndex = routerLSA->getLinksArraySize();
                            routerLSA->setLinksArraySize(linkIndex + 1);
                            routerLSA->setNumberOfLinks(linkIndex + 1);
                            routerLSA->setLinks(linkIndex, stubLink);
                        } else {
                            AnsaOSPF::Neighbor* dRouter = intf->GetNeighborByAddress(intf->GetDesignatedRouter().ipInterfaceAddress);
                            if (((dRouter != NULL) && (dRouter->GetState() == AnsaOSPF::Neighbor::FullState)) ||
                                ((intf->GetDesignatedRouter().routerID == parentRouter->GetRouterID()) &&
                                 (intf->HasAnyNeighborInStates(AnsaOSPF::Neighbor::FullState))))
                            {
                                Link link;
                                link.setType(TransitLink);
                                link.setLinkID(ULongFromIPv4Address(intf->GetDesignatedRouter().ipInterfaceAddress));
                                link.setLinkData(ULongFromIPv4Address(intf->GetAddressRange().address));
                                link.setLinkCost(intf->GetOutputCost());
                                link.setNumberOfTOS(0);
                                link.setTosDataArraySize(0);

                                unsigned short linkIndex = routerLSA->getLinksArraySize();
                                routerLSA->setLinksArraySize(linkIndex + 1);
                                routerLSA->setNumberOfLinks(linkIndex + 1);
                                routerLSA->setLinks(linkIndex, link);
                            } else {
                                Link stubLink;
                                stubLink.setType(StubLink);
                                stubLink.setLinkID(ULongFromIPv4Address(intf->GetAddressRange().address &
                                                                          intf->GetAddressRange().mask));
                                stubLink.setLinkData(ULongFromIPv4Address(intf->GetAddressRange().mask));
                                stubLink.setLinkCost(intf->GetOutputCost());
                                stubLink.setNumberOfTOS(0);
                                stubLink.setTosDataArraySize(0);

                                unsigned short linkIndex = routerLSA->getLinksArraySize();
                                routerLSA->setLinksArraySize(linkIndex + 1);
                                routerLSA->setNumberOfLinks(linkIndex + 1);
                                routerLSA->setLinks(linkIndex, stubLink);
                            }
                        }
                    }
                    break;
                case AnsaOSPF::Interface::Virtual:
                    {
                        AnsaOSPF::Neighbor* neighbor = (intf->GetNeighborCount() > 0) ? intf->GetNeighbor(0) : NULL;
                        if ((neighbor != NULL) && (neighbor->GetState() == AnsaOSPF::Neighbor::FullState)) {
                            Link link;
                            link.setType(VirtualLink);
                            link.setLinkID(neighbor->GetNeighborID());
                            link.setLinkData(ULongFromIPv4Address(intf->GetAddressRange().address));
                            link.setLinkCost(intf->GetOutputCost());
                            link.setNumberOfTOS(0);
                            link.setTosDataArraySize(0);

                            unsigned short linkIndex = routerLSA->getLinksArraySize();
                            routerLSA->setLinksArraySize(linkIndex + 1);
                            routerLSA->setNumberOfLinks(linkIndex + 1);
                            routerLSA->setLinks(linkIndex, link);
                        }
                    }
                    break;
                case AnsaOSPF::Interface::PointToMultiPoint:
                    {
                        Link stubLink;
                        stubLink.setType(StubLink);
                        stubLink.setLinkID(ULongFromIPv4Address(intf->GetAddressRange().address));
                        stubLink.setLinkData(0xFFFFFFFF);
                        stubLink.setLinkCost(0);
                        stubLink.setNumberOfTOS(0);
                        stubLink.setTosDataArraySize(0);

                        unsigned short linkIndex = routerLSA->getLinksArraySize();
                        routerLSA->setLinksArraySize(linkIndex + 1);
                        routerLSA->setNumberOfLinks(linkIndex + 1);
                        routerLSA->setLinks(linkIndex, stubLink);

                        long neighborCount = intf->GetNeighborCount();
                        for (long i = 0; i < neighborCount; i++) {
                            AnsaOSPF::Neighbor* neighbor = intf->GetNeighbor(i);
                            if (neighbor->GetState() == AnsaOSPF::Neighbor::FullState) {
                                Link link;
                                link.setType(PointToPointLink);
                                link.setLinkID(neighbor->GetNeighborID());
                                link.setLinkData(ULongFromIPv4Address(intf->GetAddressRange().address));
                                link.setLinkCost(intf->GetOutputCost());
                                link.setNumberOfTOS(0);
                                link.setTosDataArraySize(0);

                                unsigned short linkIndex = routerLSA->getLinksArraySize();
                                routerLSA->setLinksArraySize(linkIndex + 1);
                                routerLSA->setNumberOfLinks(linkIndex + 1);
                                routerLSA->setLinks(linkIndex, stubLink);
                            }
                        }
                    }
                    break;
                default: break;
            }
        }
    }

    long hostRouteCount = hostRoutes.size();
    for (i = 0; i < hostRouteCount; i++) {
        Link stubLink;
        stubLink.setType(StubLink);
        stubLink.setLinkID(ULongFromIPv4Address(hostRoutes[i].address));
        stubLink.setLinkData(0xFFFFFFFF);
        stubLink.setLinkCost(hostRoutes[i].linkCost);
        stubLink.setNumberOfTOS(0);
        stubLink.setTosDataArraySize(0);

        unsigned short linkIndex = routerLSA->getLinksArraySize();
        routerLSA->setLinksArraySize(linkIndex + 1);
        routerLSA->setNumberOfLinks(linkIndex + 1);
        routerLSA->setLinks(linkIndex, stubLink);
    }

    lsaHeader.setLsChecksum(0);    // TODO: calculate correct LS checksum

    routerLSA->SetSource(AnsaOSPF::LSATrackingInfo::Originated);

    return routerLSA;
}

AnsaOSPF::NetworkLSA* AnsaOSPF::Area::OriginateNetworkLSA(const AnsaOSPF::Interface* intf)
{
    if (intf->HasAnyNeighborInStates(AnsaOSPF::Neighbor::FullState)) {
        AnsaOSPF::NetworkLSA* networkLSA      = new AnsaOSPF::NetworkLSA;
        OSPFLSAHeader&   lsaHeader        = networkLSA->getHeader();
        long             neighborCount    = intf->GetNeighborCount();
        OSPFOptions      lsOptions;

        lsaHeader.setLsAge(0);
        memset(&lsOptions, 0, sizeof(OSPFOptions));
        lsOptions.E_ExternalRoutingCapability = externalRoutingCapability;
        lsaHeader.setLsOptions(lsOptions);
        lsaHeader.setLsType(NetworkLSAType);
        lsaHeader.setLinkStateID(ULongFromIPv4Address(intf->GetAddressRange().address));
        lsaHeader.setAdvertisingRouter(parentRouter->GetRouterID());
        lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

        networkLSA->setNetworkMask(ULongFromIPv4Address(intf->GetAddressRange().mask));

        for (long j = 0; j < neighborCount; j++) {
            const AnsaOSPF::Neighbor* neighbor = intf->GetNeighbor(j);
            if (neighbor->GetState() == AnsaOSPF::Neighbor::FullState) {
                unsigned short netIndex = networkLSA->getAttachedRoutersArraySize();
                networkLSA->setAttachedRoutersArraySize(netIndex + 1);
                networkLSA->setAttachedRouters(netIndex, neighbor->GetNeighborID());
            }
        }
        unsigned short netIndex = networkLSA->getAttachedRoutersArraySize();
        networkLSA->setAttachedRoutersArraySize(netIndex + 1);
        networkLSA->setAttachedRouters(netIndex, parentRouter->GetRouterID());

        lsaHeader.setLsChecksum(0);    // TODO: calculate correct LS checksum

        return networkLSA;
    } else {
        return NULL;
    }
}

/**
 * Returns a link state ID for the input destination.
 * If this router hasn't originated a Summary LSA for the input destination then
 * the function returs the destination address as link state ID. If it has originated
 * a Summary LSA for the input destination then the function checks which LSA would
 * contain the longer netmask. If the two masks are equal then this means thet we're
 * updating an LSA already in the database, so the function returns the destination
 * address as link state ID. If the input destination netmask is longer then the
 * one already in the database, then the returned link state ID is the input
 * destination address ORed together with the inverse of the input destination mask.
 * If the input destination netmask is shorter, then the Summary LSA already in the
 * database has to be replaced by the current destination. In this case the
 * lsaToReoriginate parameter is filled with a copy of the Summary LSA in the database
 * with it's mask replaced by the destination mask and the cost replaced by the input
 * destination cost; the returned link state ID is the input destination address ORed
 * together with the inverse of the mask stored in the Summary LSA in the database.
 * This means that if the lsaToReoriginate parameter is not NULL on return then another
 * lookup in the database is needed with the same LSAKey as used here(input
 * destination address and the router's own routerID) and the resulting Summary LSA's
 * link state ID should be changed to the one returned by this function.
 */
AnsaOSPF::LinkStateID AnsaOSPF::Area::GetUniqueLinkStateID(AnsaOSPF::IPv4AddressRange destination,
                                                    AnsaOSPF::Metric destinationCost,
                                                    AnsaOSPF::SummaryLSA*& lsaToReoriginate) const
{
    if (lsaToReoriginate != NULL) {
        delete lsaToReoriginate;
        lsaToReoriginate = NULL;
    }

    AnsaOSPF::LSAKeyType lsaKey;

    lsaKey.linkStateID = ULongFromIPv4Address(destination.address);
    lsaKey.advertisingRouter = parentRouter->GetRouterID();

    const AnsaOSPF::SummaryLSA* foundLSA = FindSummaryLSA(lsaKey);

    if (foundLSA == NULL) {
        return lsaKey.linkStateID;
    } else {
        AnsaOSPF::IPv4Address existingMask = IPv4AddressFromULong(foundLSA->getNetworkMask().getInt());

        if (destination.mask == existingMask) {
            return lsaKey.linkStateID;
        } else {
            if (destination.mask >= existingMask) {
                return (lsaKey.linkStateID | (~(ULongFromIPv4Address(destination.mask))));
            } else {
                AnsaOSPF::SummaryLSA* summaryLSA = new AnsaOSPF::SummaryLSA(*foundLSA);

                long sequenceNumber = summaryLSA->getHeader().getLsSequenceNumber();

                summaryLSA->getHeader().setLsAge(0);
                summaryLSA->getHeader().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
                summaryLSA->setNetworkMask(ULongFromIPv4Address(destination.mask));
                summaryLSA->setRouteCost(destinationCost);
                summaryLSA->getHeader().setLsChecksum(0);    // TODO: calculate correct LS checksum

                lsaToReoriginate = summaryLSA;

                return (lsaKey.linkStateID | (~(ULongFromIPv4Address(existingMask))));
            }
        }
    }
}

AnsaOSPF::SummaryLSA* AnsaOSPF::Area::OriginateSummaryLSA(const AnsaOSPF::RoutingTableEntry* entry,
                                                   const std::map<AnsaOSPF::LSAKeyType, bool, AnsaOSPF::LSAKeyType_Less>& originatedLSAs,
                                                   AnsaOSPF::SummaryLSA*& lsaToReoriginate)
{
    if (((entry->GetDestinationType() & AnsaOSPF::RoutingTableEntry::AreaBorderRouterDestination) != 0) ||
        (entry->GetPathType() == AnsaOSPF::RoutingTableEntry::Type1External) ||
        (entry->GetPathType() == AnsaOSPF::RoutingTableEntry::Type2External) ||
        (entry->GetArea() == areaID))
    {
        return NULL;
    }

    bool         allNextHopsInThisArea = true;
    unsigned int nextHopCount          = entry->GetNextHopCount();

    for (unsigned int i = 0; i < nextHopCount; i++) {
        AnsaOSPF::Interface* nextHopInterface = parentRouter->GetNonVirtualInterface(entry->GetNextHop(i).ifIndex);
        if ((nextHopInterface != NULL) && (nextHopInterface->GetAreaID() != areaID)) {
            allNextHopsInThisArea = false;
            break;
        }
    }
    if ((allNextHopsInThisArea) || (entry->GetCost() >= LS_INFINITY)){
        return NULL;
    }

    if ((entry->GetDestinationType() & AnsaOSPF::RoutingTableEntry::ASBoundaryRouterDestination) != 0) {
        AnsaOSPF::RoutingTableEntry* preferredEntry = parentRouter->GetPreferredEntry(*(entry->GetLinkStateOrigin()), false);
        if ((preferredEntry != NULL) && (*preferredEntry == *entry) && (externalRoutingCapability)) {
            AnsaOSPF::SummaryLSA* summaryLSA    = new AnsaOSPF::SummaryLSA;
            OSPFLSAHeader&    lsaHeader     = summaryLSA->getHeader();
            OSPFOptions       lsOptions;

            lsaHeader.setLsAge(0);
            memset(&lsOptions, 0, sizeof(OSPFOptions));
            lsOptions.E_ExternalRoutingCapability = externalRoutingCapability;
            lsaHeader.setLsOptions(lsOptions);
            lsaHeader.setLsType(SummaryLSA_ASBoundaryRoutersType);
            lsaHeader.setLinkStateID(entry->GetDestinationID().getInt());
            lsaHeader.setAdvertisingRouter(parentRouter->GetRouterID());
            lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

            summaryLSA->setNetworkMask(entry->GetAddressMask());
            summaryLSA->setRouteCost(entry->GetCost());
            summaryLSA->setTosDataArraySize(0);

            lsaHeader.setLsChecksum(0);    // TODO: calculate correct LS checksum

            summaryLSA->SetSource(AnsaOSPF::LSATrackingInfo::Originated);

            return summaryLSA;
        }
    } else {    // entry->GetDestinationType() == AnsaOSPF::RoutingTableEntry::NetworkDestination
        if (entry->GetPathType() == AnsaOSPF::RoutingTableEntry::InterArea) {
            AnsaOSPF::IPv4AddressRange destinationRange;

            destinationRange.address = IPv4AddressFromULong(entry->GetDestinationID().getInt());
            destinationRange.mask = IPv4AddressFromULong(entry->GetAddressMask().getInt());

            AnsaOSPF::LinkStateID newLinkStateID = GetUniqueLinkStateID(destinationRange, entry->GetCost(), lsaToReoriginate);

            if (lsaToReoriginate != NULL) {
                AnsaOSPF::LSAKeyType lsaKey;

                lsaKey.linkStateID = entry->GetDestinationID().getInt();
                lsaKey.advertisingRouter = parentRouter->GetRouterID();

                std::map<AnsaOSPF::LSAKeyType, AnsaOSPF::SummaryLSA*, AnsaOSPF::LSAKeyType_Less>::iterator lsaIt = summaryLSAsByID.find(lsaKey);
                if (lsaIt == summaryLSAsByID.end()) {
                    delete(lsaToReoriginate);
                    lsaToReoriginate = NULL;
                    return NULL;
                } else {
                    AnsaOSPF::SummaryLSA* summaryLSA = new AnsaOSPF::SummaryLSA(*(lsaIt->second));
                    OSPFLSAHeader&    lsaHeader  = summaryLSA->getHeader();

                    lsaHeader.setLsAge(0);
                    lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);
                    lsaHeader.setLinkStateID(newLinkStateID);
                    lsaHeader.setLsChecksum(0);    // TODO: calculate correct LS checksum

                    return summaryLSA;
                }
            } else {
                AnsaOSPF::SummaryLSA* summaryLSA    = new AnsaOSPF::SummaryLSA;
                OSPFLSAHeader&    lsaHeader     = summaryLSA->getHeader();
                OSPFOptions       lsOptions;

                lsaHeader.setLsAge(0);
                memset(&lsOptions, 0, sizeof(OSPFOptions));
                lsOptions.E_ExternalRoutingCapability = externalRoutingCapability;
                lsaHeader.setLsOptions(lsOptions);
                lsaHeader.setLsType(SummaryLSA_NetworksType);
                lsaHeader.setLinkStateID(newLinkStateID);
                lsaHeader.setAdvertisingRouter(parentRouter->GetRouterID());
                lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

                summaryLSA->setNetworkMask(entry->GetAddressMask());
                summaryLSA->setRouteCost(entry->GetCost());
                summaryLSA->setTosDataArraySize(0);

                lsaHeader.setLsChecksum(0);    // TODO: calculate correct LS checksum

                summaryLSA->SetSource(AnsaOSPF::LSATrackingInfo::Originated);

                return summaryLSA;
            }
        } else {    // entry->GetPathType() == AnsaOSPF::RoutingTableEntry::IntraArea
            AnsaOSPF::IPv4AddressRange destinationAddressRange;

            destinationAddressRange.address = IPv4AddressFromULong(entry->GetDestinationID().getInt());
            destinationAddressRange.mask = IPv4AddressFromULong(entry->GetAddressMask().getInt());

            bool doAdvertise = false;
            AnsaOSPF::IPv4AddressRange containingAddressRange = parentRouter->GetContainingAddressRange(destinationAddressRange, &doAdvertise);
            if (((entry->GetArea() == AnsaOSPF::BackboneAreaID) &&         // the backbone's configured ranges should be ignored
                 (transitCapability)) ||                                // when originating Summary LSAs into transit areas
                (containingAddressRange == AnsaOSPF::NullIPv4AddressRange))
            {
                AnsaOSPF::LinkStateID newLinkStateID = GetUniqueLinkStateID(destinationAddressRange, entry->GetCost(), lsaToReoriginate);

                if (lsaToReoriginate != NULL) {
                    AnsaOSPF::LSAKeyType lsaKey;

                    lsaKey.linkStateID = entry->GetDestinationID().getInt();
                    lsaKey.advertisingRouter = parentRouter->GetRouterID();

                    std::map<AnsaOSPF::LSAKeyType, AnsaOSPF::SummaryLSA*, AnsaOSPF::LSAKeyType_Less>::iterator lsaIt = summaryLSAsByID.find(lsaKey);
                    if (lsaIt == summaryLSAsByID.end()) {
                        delete(lsaToReoriginate);
                        lsaToReoriginate = NULL;
                        return NULL;
                    } else {
                        AnsaOSPF::SummaryLSA* summaryLSA = new AnsaOSPF::SummaryLSA(*(lsaIt->second));
                        OSPFLSAHeader&    lsaHeader  = summaryLSA->getHeader();

                        lsaHeader.setLsAge(0);
                        lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);
                        lsaHeader.setLinkStateID(newLinkStateID);
                        lsaHeader.setLsChecksum(0);    // TODO: calculate correct LS checksum

                        return summaryLSA;
                    }
                } else {
                    AnsaOSPF::SummaryLSA* summaryLSA    = new AnsaOSPF::SummaryLSA;
                    OSPFLSAHeader&    lsaHeader     = summaryLSA->getHeader();
                    OSPFOptions       lsOptions;

                    lsaHeader.setLsAge(0);
                    memset(&lsOptions, 0, sizeof(OSPFOptions));
                    lsOptions.E_ExternalRoutingCapability = externalRoutingCapability;
                    lsaHeader.setLsOptions(lsOptions);
                    lsaHeader.setLsType(SummaryLSA_NetworksType);
                    lsaHeader.setLinkStateID(newLinkStateID);
                    lsaHeader.setAdvertisingRouter(parentRouter->GetRouterID());
                    lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

                    summaryLSA->setNetworkMask(entry->GetAddressMask());
                    summaryLSA->setRouteCost(entry->GetCost());
                    summaryLSA->setTosDataArraySize(0);

                    lsaHeader.setLsChecksum(0);    // TODO: calculate correct LS checksum

                    summaryLSA->SetSource(AnsaOSPF::LSATrackingInfo::Originated);

                    return summaryLSA;
                }
            } else {
                if (doAdvertise) {
                    Metric        maxRangeCost = 0;
                    unsigned long entryCount   = parentRouter->GetRoutingTableEntryCount();

                    for (unsigned long i = 0; i < entryCount; i++) {
                        const AnsaOSPF::RoutingTableEntry* routingEntry = parentRouter->GetRoutingTableEntry(i);

                        if ((routingEntry->GetDestinationType() == AnsaOSPF::RoutingTableEntry::NetworkDestination) &&
                            (routingEntry->GetPathType() == AnsaOSPF::RoutingTableEntry::IntraArea) &&
                            ((routingEntry->GetDestinationID().getInt() & routingEntry->GetAddressMask().getInt() & ULongFromIPv4Address(containingAddressRange.mask)) ==
                             ULongFromIPv4Address(containingAddressRange.address & containingAddressRange.mask)) &&
                            (routingEntry->GetCost() > maxRangeCost))
                        {
                            maxRangeCost = routingEntry->GetCost();
                        }
                    }

                    AnsaOSPF::LinkStateID newLinkStateID = GetUniqueLinkStateID(containingAddressRange, maxRangeCost, lsaToReoriginate);
                    AnsaOSPF::LSAKeyType  lsaKey;

                    if (lsaToReoriginate != NULL) {
                        lsaKey.linkStateID = lsaToReoriginate->getHeader().getLinkStateID();
                        lsaKey.advertisingRouter = parentRouter->GetRouterID();

                        std::map<AnsaOSPF::LSAKeyType, bool, AnsaOSPF::LSAKeyType_Less>::const_iterator originatedIt = originatedLSAs.find(lsaKey);
                        if (originatedIt != originatedLSAs.end()) {
                            delete(lsaToReoriginate);
                            lsaToReoriginate = NULL;
                            return NULL;
                        }

                        lsaKey.linkStateID = entry->GetDestinationID().getInt();
                        lsaKey.advertisingRouter = parentRouter->GetRouterID();

                        std::map<AnsaOSPF::LSAKeyType, AnsaOSPF::SummaryLSA*, AnsaOSPF::LSAKeyType_Less>::iterator lsaIt = summaryLSAsByID.find(lsaKey);
                        if (lsaIt == summaryLSAsByID.end()) {
                            delete(lsaToReoriginate);
                            lsaToReoriginate = NULL;
                            return NULL;
                        }

                        AnsaOSPF::SummaryLSA* summaryLSA = new AnsaOSPF::SummaryLSA(*(lsaIt->second));
                        OSPFLSAHeader&    lsaHeader  = summaryLSA->getHeader();

                        lsaHeader.setLsAge(0);
                        lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);
                        lsaHeader.setLinkStateID(newLinkStateID);
                        lsaHeader.setLsChecksum(0);    // TODO: calculate correct LS checksum

                        return summaryLSA;
                    } else {
                        lsaKey.linkStateID = newLinkStateID;
                        lsaKey.advertisingRouter = parentRouter->GetRouterID();

                        std::map<AnsaOSPF::LSAKeyType, bool, AnsaOSPF::LSAKeyType_Less>::const_iterator originatedIt = originatedLSAs.find(lsaKey);
                        if (originatedIt != originatedLSAs.end()) {
                            return NULL;
                        }

                        AnsaOSPF::SummaryLSA* summaryLSA    = new AnsaOSPF::SummaryLSA;
                        OSPFLSAHeader&    lsaHeader     = summaryLSA->getHeader();
                        OSPFOptions       lsOptions;

                        lsaHeader.setLsAge(0);
                        memset(&lsOptions, 0, sizeof(OSPFOptions));
                        lsOptions.E_ExternalRoutingCapability = externalRoutingCapability;
                        lsaHeader.setLsOptions(lsOptions);
                        lsaHeader.setLsType(SummaryLSA_NetworksType);
                        lsaHeader.setLinkStateID(newLinkStateID);
                        lsaHeader.setAdvertisingRouter(parentRouter->GetRouterID());
                        lsaHeader.setLsSequenceNumber(INITIAL_SEQUENCE_NUMBER);

                        summaryLSA->setNetworkMask(entry->GetAddressMask());
                        summaryLSA->setRouteCost(entry->GetCost());
                        summaryLSA->setTosDataArraySize(0);

                        lsaHeader.setLsChecksum(0);    // TODO: calculate correct LS checksum

                        summaryLSA->SetSource(AnsaOSPF::LSATrackingInfo::Originated);

                        return summaryLSA;
                    }
                }
            }
        }
    }

    return NULL;
}

AnsaOSPF::SummaryLSA* AnsaOSPF::Area::OriginateSummaryLSA(const AnsaOSPF::SummaryLSA* summaryLSA)
{
    const std::map<AnsaOSPF::LSAKeyType, bool, AnsaOSPF::LSAKeyType_Less> emptyMap;
    AnsaOSPF::SummaryLSA*                                             dontReoriginate = NULL;

    const OSPFLSAHeader& lsaHeader   = summaryLSA->getHeader();
    unsigned long   entryCount = parentRouter->GetRoutingTableEntryCount();

    for (unsigned long i = 0; i < entryCount; i++) {
        const AnsaOSPF::RoutingTableEntry* entry = parentRouter->GetRoutingTableEntry(i);

        if ((lsaHeader.getLsType() == SummaryLSA_ASBoundaryRoutersType) &&
            ((((entry->GetDestinationType() & AnsaOSPF::RoutingTableEntry::AreaBorderRouterDestination) != 0) ||
              ((entry->GetDestinationType() & AnsaOSPF::RoutingTableEntry::ASBoundaryRouterDestination) != 0)) &&
             ((entry->GetDestinationID().getInt() == lsaHeader.getLinkStateID()) &&
              (entry->GetAddressMask() == summaryLSA->getNetworkMask()))))
        {
            AnsaOSPF::SummaryLSA* returnLSA = OriginateSummaryLSA(entry, emptyMap, dontReoriginate);
            if (dontReoriginate != NULL) {
                delete dontReoriginate;
            }
            return returnLSA;
        }

        unsigned long lsaMask = summaryLSA->getNetworkMask().getInt();
        

        if ((lsaHeader.getLsType() == SummaryLSA_NetworksType) &&
            (entry->GetDestinationType() == AnsaOSPF::RoutingTableEntry::NetworkDestination) &&
            (entry->GetAddressMask().getInt() == lsaMask) &&
            ((entry->GetDestinationID().getInt() & lsaMask) == (lsaHeader.getLinkStateID() & lsaMask)))
        {
            AnsaOSPF::SummaryLSA* returnLSA = OriginateSummaryLSA(entry, emptyMap, dontReoriginate);
            if (dontReoriginate != NULL) {
                delete dontReoriginate;
            }
            return returnLSA;
        }
    }
    return NULL;
}

void AnsaOSPF::Area::CalculateShortestPathTree(std::vector<AnsaOSPF::RoutingTableEntry*>& newRoutingTable)
{
    AnsaOSPF::RouterID          routerID = parentRouter->GetRouterID();
    bool                    finished = false;
    std::vector<OSPFLSA*>   treeVertices;
    OSPFLSA*                justAddedVertex;
    std::vector<OSPFLSA*>   candidateVertices;
    unsigned long            i, j, k;
    unsigned long            lsaCount;

    if (spfTreeRoot == NULL) {
        AnsaOSPF::RouterLSA* newLSA = OriginateRouterLSA();

        InstallRouterLSA(newLSA);

        AnsaOSPF::RouterLSA* routerLSA = FindRouterLSA(routerID);

        spfTreeRoot = routerLSA;
        FloodLSA(newLSA);
        delete newLSA;
    }
    if (spfTreeRoot == NULL) {
        return;
    }

    lsaCount = routerLSAs.size();
    for (i = 0; i < lsaCount; i++) {
        routerLSAs[i]->ClearNextHops();
    }
    lsaCount = networkLSAs.size();
    for (i = 0; i < lsaCount; i++) {
        networkLSAs[i]->ClearNextHops();
    }
    spfTreeRoot->SetDistance(0);
    treeVertices.push_back(spfTreeRoot);
    justAddedVertex = spfTreeRoot;          // (1)

    do {
        LSAType vertexType = static_cast<LSAType> (justAddedVertex->getHeader().getLsType());

        if ((vertexType == RouterLSAType)) {
            AnsaOSPF::RouterLSA* routerVertex = check_and_cast<AnsaOSPF::RouterLSA*> (justAddedVertex);
            if (routerVertex->getV_VirtualLinkEndpoint()) {    // (2)
                transitCapability = true;
            }

            unsigned int linkCount = routerVertex->getLinksArraySize();
            for (i = 0; i < linkCount; i++) {
                Link&    link     = routerVertex->getLinks(i);
                LinkType linkType = static_cast<LinkType> (link.getType());
                OSPFLSA* joiningVertex;
                LSAType  joiningVertexType;

                if (linkType == StubLink) {     // (2) (a)
                    continue;
                }

                if (linkType == TransitLink) {
                    joiningVertex     = FindNetworkLSA(link.getLinkID().getInt());
                    joiningVertexType = NetworkLSAType;
                } else {
                    joiningVertex     = FindRouterLSA(link.getLinkID().getInt());
                    joiningVertexType = RouterLSAType;
                }

                if ((joiningVertex == NULL) ||
                    (joiningVertex->getHeader().getLsAge() == MAX_AGE) ||
                    (!HasLink(joiningVertex, justAddedVertex)))  // (from, to)     (2) (b)
                {
                    continue;
                }

                unsigned int treeSize      = treeVertices.size();
                bool         alreadyOnTree = false;

                for (j = 0; j < treeSize; j++) {
                    if (treeVertices[j] == joiningVertex) {
                        alreadyOnTree = true;
                        break;
                    }
                }
                if (alreadyOnTree) {    // (2) (c)
                    continue;
                }

                unsigned long linkStateCost  = routerVertex->GetDistance() + link.getLinkCost();
                unsigned int  candidateCount = candidateVertices.size();
                OSPFLSA*      candidate      = NULL;

                for (j = 0; j < candidateCount; j++) {
                    if (candidateVertices[j] == joiningVertex) {
                        candidate = candidateVertices[j];
                    }
                }
                if (candidate != NULL) {    // (2) (d)
                    AnsaOSPF::RoutingInfo* routingInfo       = check_and_cast<AnsaOSPF::RoutingInfo*> (candidate);
                    unsigned long      candidateDistance = routingInfo->GetDistance();

                    if (linkStateCost > candidateDistance) {
                        continue;
                    }
                    if (linkStateCost < candidateDistance) {
                        routingInfo->SetDistance(linkStateCost);
                        routingInfo->ClearNextHops();
                    }
                    std::vector<AnsaOSPF::NextHop>* newNextHops = CalculateNextHops(joiningVertex, justAddedVertex); // (destination, parent)
                    unsigned int nextHopCount = newNextHops->size();
                    for (k = 0; k < nextHopCount; k++) {
                        routingInfo->AddNextHop((*newNextHops)[k]);
                    }
                    delete newNextHops;
                } else {
                    if (joiningVertexType == RouterLSAType) {
                        AnsaOSPF::RouterLSA* joiningRouterVertex = check_and_cast<AnsaOSPF::RouterLSA*> (joiningVertex);
                        joiningRouterVertex->SetDistance(linkStateCost);
                        std::vector<AnsaOSPF::NextHop>* newNextHops = CalculateNextHops(joiningVertex, justAddedVertex); // (destination, parent)
                        unsigned int nextHopCount = newNextHops->size();
                        for (k = 0; k < nextHopCount; k++) {
                            joiningRouterVertex->AddNextHop((*newNextHops)[k]);
                        }
                        delete newNextHops;
                        AnsaOSPF::RoutingInfo* vertexRoutingInfo = check_and_cast<AnsaOSPF::RoutingInfo*> (joiningRouterVertex);
                        vertexRoutingInfo->SetParent(justAddedVertex);

                        candidateVertices.push_back(joiningRouterVertex);
                    } else {
                        AnsaOSPF::NetworkLSA* joiningNetworkVertex = check_and_cast<AnsaOSPF::NetworkLSA*> (joiningVertex);
                        joiningNetworkVertex->SetDistance(linkStateCost);
                        std::vector<AnsaOSPF::NextHop>* newNextHops = CalculateNextHops(joiningVertex, justAddedVertex); // (destination, parent)
                        unsigned int nextHopCount = newNextHops->size();
                        for (k = 0; k < nextHopCount; k++) {
                            joiningNetworkVertex->AddNextHop((*newNextHops)[k]);
                        }
                        delete newNextHops;
                        AnsaOSPF::RoutingInfo* vertexRoutingInfo = check_and_cast<AnsaOSPF::RoutingInfo*> (joiningNetworkVertex);
                        vertexRoutingInfo->SetParent(justAddedVertex);

                        candidateVertices.push_back(joiningNetworkVertex);
                    }
                }
            }
        }

        if ((vertexType == NetworkLSAType)) {
            AnsaOSPF::NetworkLSA* networkVertex = check_and_cast<AnsaOSPF::NetworkLSA*> (justAddedVertex);
            unsigned int      routerCount   = networkVertex->getAttachedRoutersArraySize();

            for (i = 0; i < routerCount; i++) {     // (2)
                AnsaOSPF::RouterLSA* joiningVertex = FindRouterLSA(networkVertex->getAttachedRouters(i).getInt());
                if ((joiningVertex == NULL) ||
                    (joiningVertex->getHeader().getLsAge() == MAX_AGE) ||
                    (!HasLink(joiningVertex, justAddedVertex)))  // (from, to)     (2) (b)
                {
                    continue;
                }

                unsigned int treeSize      = treeVertices.size();
                bool         alreadyOnTree = false;

                for (j = 0; j < treeSize; j++) {
                    if (treeVertices[j] == joiningVertex) {
                        alreadyOnTree = true;
                        break;
                    }
                }
                if (alreadyOnTree) {    // (2) (c)
                    continue;
                }

                unsigned long linkStateCost  = networkVertex->GetDistance();   // link cost from network to router is always 0
                unsigned int  candidateCount = candidateVertices.size();
                OSPFLSA*      candidate      = NULL;

                for (j = 0; j < candidateCount; j++) {
                    if (candidateVertices[j] == joiningVertex) {
                        candidate = candidateVertices[j];
                    }
                }
                if (candidate != NULL) {    // (2) (d)
                    AnsaOSPF::RoutingInfo* routingInfo       = check_and_cast<AnsaOSPF::RoutingInfo*> (candidate);
                    unsigned long      candidateDistance = routingInfo->GetDistance();

                    if (linkStateCost > candidateDistance) {
                        continue;
                    }
                    if (linkStateCost < candidateDistance) {
                        routingInfo->SetDistance(linkStateCost);
                        routingInfo->ClearNextHops();
                    }
                    std::vector<AnsaOSPF::NextHop>* newNextHops = CalculateNextHops(joiningVertex, justAddedVertex); // (destination, parent)
                    unsigned int nextHopCount = newNextHops->size();
                    for (k = 0; k < nextHopCount; k++) {
                        routingInfo->AddNextHop((*newNextHops)[k]);
                    }
                    delete newNextHops;
                } else {
                    joiningVertex->SetDistance(linkStateCost);
                    std::vector<AnsaOSPF::NextHop>* newNextHops = CalculateNextHops(joiningVertex, justAddedVertex); // (destination, parent)
                    unsigned int nextHopCount = newNextHops->size();
                    for (k = 0; k < nextHopCount; k++) {
                        joiningVertex->AddNextHop((*newNextHops)[k]);
                    }
                    delete newNextHops;
                    AnsaOSPF::RoutingInfo* vertexRoutingInfo = check_and_cast<AnsaOSPF::RoutingInfo*> (joiningVertex);
                    vertexRoutingInfo->SetParent(justAddedVertex);

                    candidateVertices.push_back(joiningVertex);
                }
            }
        }

        if (candidateVertices.empty()) {  // (3)
            finished = true;
        } else {
            unsigned int  candidateCount = candidateVertices.size();
            unsigned long minDistance = LS_INFINITY;
            OSPFLSA*      closestVertex = candidateVertices[0];

            for (i = 0; i < candidateCount; i++) {
                AnsaOSPF::RoutingInfo* routingInfo     = check_and_cast<AnsaOSPF::RoutingInfo*> (candidateVertices[i]);
                unsigned long      currentDistance = routingInfo->GetDistance();

                if (currentDistance < minDistance) {
                    closestVertex = candidateVertices[i];
                    minDistance = currentDistance;
                } else {
                    if (currentDistance == minDistance) {
                        if ((closestVertex->getHeader().getLsType() == RouterLSAType) &&
                            (candidateVertices[i]->getHeader().getLsType() == NetworkLSAType))
                        {
                            closestVertex = candidateVertices[i];
                        }
                    }
                }
            }

            treeVertices.push_back(closestVertex);

            for (std::vector<OSPFLSA*>::iterator it = candidateVertices.begin(); it != candidateVertices.end(); it++) {
                if ((*it) == closestVertex) {
                    candidateVertices.erase(it);
                    break;
                }
            }

            if (closestVertex->getHeader().getLsType() == RouterLSAType) {
                AnsaOSPF::RouterLSA* routerLSA = check_and_cast<AnsaOSPF::RouterLSA*> (closestVertex);
                if (routerLSA->getB_AreaBorderRouter() || routerLSA->getE_ASBoundaryRouter()) {
                    AnsaOSPF::RoutingTableEntry*                        entry           = new AnsaOSPF::RoutingTableEntry;
                    AnsaOSPF::RouterID                                  destinationID   = routerLSA->getHeader().getLinkStateID();
                    unsigned int                                    nextHopCount    = routerLSA->GetNextHopCount();
                    AnsaOSPF::RoutingTableEntry::RoutingDestinationType destinationType = AnsaOSPF::RoutingTableEntry::NetworkDestination;

                    entry->SetDestinationID(destinationID);
                    entry->SetLinkStateOrigin(routerLSA);
                    entry->SetArea(areaID);
                    entry->SetPathType(AnsaOSPF::RoutingTableEntry::IntraArea);
                    entry->SetCost(routerLSA->GetDistance());
                    if (routerLSA->getB_AreaBorderRouter()) {
                        destinationType |= AnsaOSPF::RoutingTableEntry::AreaBorderRouterDestination;
                    }
                    if (routerLSA->getE_ASBoundaryRouter()) {
                        destinationType |= AnsaOSPF::RoutingTableEntry::ASBoundaryRouterDestination;
                    }
                    entry->SetDestinationType(destinationType);
                    entry->SetOptionalCapabilities(routerLSA->getHeader().getLsOptions());
                    for (i = 0; i < nextHopCount; i++) {
                        entry->AddNextHop(routerLSA->GetNextHop(i));
                    }

                    newRoutingTable.push_back(entry);

                    AnsaOSPF::Area* backbone;
                    if (areaID != AnsaOSPF::BackboneAreaID) {
                        backbone = parentRouter->GetArea(AnsaOSPF::BackboneAreaID);
                    } else {
                        backbone = this;
                    }
                    if (backbone != NULL) {
                        AnsaOSPF::Interface* virtualIntf = backbone->FindVirtualLink(destinationID);
                        if ((virtualIntf != NULL) && (virtualIntf->GetTransitAreaID() == areaID)) {
                            AnsaOSPF::IPv4AddressRange range;
                            range.address = GetInterface(routerLSA->GetNextHop(0).ifIndex)->GetAddressRange().address;
                            range.mask    = IPv4AddressFromULong(0xFFFFFFFF);
                            virtualIntf->SetAddressRange(range);
                            virtualIntf->SetIfIndex(routerLSA->GetNextHop(0).ifIndex);
                            virtualIntf->SetOutputCost(routerLSA->GetDistance());
                            AnsaOSPF::Neighbor* virtualNeighbor = virtualIntf->GetNeighbor(0);
                            if (virtualNeighbor != NULL) {
                                unsigned int     linkCount   = routerLSA->getLinksArraySize();
                                AnsaOSPF::RouterLSA* toRouterLSA = dynamic_cast<AnsaOSPF::RouterLSA*> (justAddedVertex);
                                if (toRouterLSA != NULL) {
                                    for (i = 0; i < linkCount; i++) {
                                        Link& link = routerLSA->getLinks(i);

                                        if ((link.getType() == PointToPointLink) &&
                                            (link.getLinkID() == toRouterLSA->getHeader().getLinkStateID()) &&
                                            (virtualIntf->GetState() < AnsaOSPF::Interface::WaitingState))
                                        {
                                            virtualNeighbor->SetAddress(IPv4AddressFromULong(link.getLinkData()));
                                            virtualIntf->ProcessEvent(AnsaOSPF::Interface::InterfaceUp);
                                            break;
                                        }
                                    }
                                } else {
                                    AnsaOSPF::NetworkLSA* toNetworkLSA = dynamic_cast<AnsaOSPF::NetworkLSA*> (justAddedVertex);
                                    if (toNetworkLSA != NULL) {
                                        for (i = 0; i < linkCount; i++) {
                                            Link& link = routerLSA->getLinks(i);

                                            if ((link.getType() == TransitLink) &&
                                                (link.getLinkID() == toNetworkLSA->getHeader().getLinkStateID()) &&
                                                (virtualIntf->GetState() < AnsaOSPF::Interface::WaitingState))
                                            {
                                                virtualNeighbor->SetAddress(IPv4AddressFromULong(link.getLinkData()));
                                                virtualIntf->ProcessEvent(AnsaOSPF::Interface::InterfaceUp);
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (closestVertex->getHeader().getLsType() == NetworkLSAType) {
                AnsaOSPF::NetworkLSA*        networkLSA    = check_and_cast<AnsaOSPF::NetworkLSA*> (closestVertex);
                unsigned long            destinationID = (networkLSA->getHeader().getLinkStateID() & networkLSA->getNetworkMask().getInt());
                unsigned int             nextHopCount  = networkLSA->GetNextHopCount();
                bool                     overWrite     = false;
                AnsaOSPF::RoutingTableEntry* entry         = NULL;
                unsigned long            routeCount    = newRoutingTable.size();
                unsigned long            longestMatch  = 0;

                for (i = 0; i < routeCount; i++) {
                    if (newRoutingTable[i]->GetDestinationType() == AnsaOSPF::RoutingTableEntry::NetworkDestination) {
                        AnsaOSPF::RoutingTableEntry* routingEntry = newRoutingTable[i];
                        unsigned long            entryAddress = routingEntry->GetDestinationID().getInt();
                        unsigned long            entryMask    = routingEntry->GetAddressMask().getInt();

                        if ((entryAddress & entryMask) == (destinationID & entryMask)) {
                            if ((destinationID & entryMask) > longestMatch) {
                                longestMatch = (destinationID & entryMask);
                                entry        = routingEntry;
                            }
                        }
                    }
                }
                if (entry != NULL) {
                    const OSPFLSA* entryOrigin = entry->GetLinkStateOrigin();
                    if ((entry->GetCost() != networkLSA->GetDistance()) ||
                        (entryOrigin->getHeader().getLinkStateID() >= networkLSA->getHeader().getLinkStateID()))
                    {
                        overWrite = true;
                    }
                }

                if ((entry == NULL) || (overWrite)) {
                    if (entry == NULL) {
                        entry = new AnsaOSPF::RoutingTableEntry;
                    }

                    entry->SetDestinationID(destinationID);
                    entry->SetAddressMask(networkLSA->getNetworkMask());
                    entry->SetLinkStateOrigin(networkLSA);
                    entry->SetArea(areaID);
                    entry->SetPathType(AnsaOSPF::RoutingTableEntry::IntraArea);
                    entry->SetCost(networkLSA->GetDistance());
                    entry->SetDestinationType(AnsaOSPF::RoutingTableEntry::NetworkDestination);
                    entry->SetOptionalCapabilities(networkLSA->getHeader().getLsOptions());
                    for (i = 0; i < nextHopCount; i++) {
                        entry->AddNextHop(networkLSA->GetNextHop(i));
                    }

                    if (!overWrite) {
                        newRoutingTable.push_back(entry);
                    }
                }
            }

            justAddedVertex = closestVertex;
        }
    } while (!finished);

    unsigned int treeSize      = treeVertices.size();
    for (i = 0; i < treeSize; i++) {
        AnsaOSPF::RouterLSA* routerVertex = dynamic_cast<AnsaOSPF::RouterLSA*> (treeVertices[i]);
        if (routerVertex == NULL) {
            continue;
        }

        unsigned int linkCount = routerVertex->getLinksArraySize();
        for (j = 0; j < linkCount; j++) {
            Link&    link     = routerVertex->getLinks(j);
            if (link.getType() != StubLink) {
                continue;
            }

            unsigned long            distance      = routerVertex->GetDistance() + link.getLinkCost();
            unsigned long            destinationID = (link.getLinkID().getInt() & link.getLinkData());
            AnsaOSPF::RoutingTableEntry* entry         = NULL;
            unsigned long            routeCount    = newRoutingTable.size();
            unsigned long            longestMatch  = 0;

            for (k = 0; k < routeCount; k++) {
                if (newRoutingTable[k]->GetDestinationType() == AnsaOSPF::RoutingTableEntry::NetworkDestination) {
                    AnsaOSPF::RoutingTableEntry* routingEntry = newRoutingTable[k];
                    unsigned long            entryAddress = routingEntry->GetDestinationID().getInt();
                    unsigned long            entryMask    = routingEntry->GetAddressMask().getInt();

                    if ((entryAddress & entryMask) == (destinationID & entryMask)) {
                        if ((destinationID & entryMask) > longestMatch) {
                            longestMatch = (destinationID & entryMask);
                            entry        = routingEntry;
                        }
                    }
                }
            }

            if (entry != NULL) {
                Metric entryCost = entry->GetCost();

                if (distance > entryCost) {
                    continue;
                }
                if (distance < entryCost) {
                    //FIXME remove
                    //if(parentRouter->GetRouterID() == 0xC0A80302) {
                    //    EV << "CHEAPER STUB LINK FOUND TO " << IPAddress(destinationID).str() << "\n";
                    //}
                    entry->SetCost(distance);
                    entry->ClearNextHops();
                    entry->SetLinkStateOrigin(routerVertex);
                }
                if (distance == entryCost) {
                    // no const version from check_and_cast
                    AnsaOSPF::RouterLSA* routerOrigin = check_and_cast<AnsaOSPF::RouterLSA*> (const_cast<OSPFLSA*> (entry->GetLinkStateOrigin()));
                    if (routerOrigin->getHeader().getLinkStateID() < routerVertex->getHeader().getLinkStateID()) {
                        entry->SetLinkStateOrigin(routerVertex);
                    }
                }
                std::vector<AnsaOSPF::NextHop>* newNextHops = CalculateNextHops(link, routerVertex); // (destination, parent)
                unsigned int nextHopCount = newNextHops->size();
                for (k = 0; k < nextHopCount; k++) {
                    entry->AddNextHop((*newNextHops)[k]);
                }
                delete newNextHops;
            } else {
                //FIXME remove
                //if(parentRouter->GetRouterID() == 0xC0A80302) {
                //    EV << "STUB LINK FOUND TO " << IPAddress(destinationID).str() << "\n";
                //}
                entry = new AnsaOSPF::RoutingTableEntry;

                entry->SetDestinationID(destinationID);
                entry->SetAddressMask(link.getLinkData());
                entry->SetLinkStateOrigin(routerVertex);
                entry->SetArea(areaID);
                entry->SetPathType(AnsaOSPF::RoutingTableEntry::IntraArea);
                entry->SetCost(distance);
                entry->SetDestinationType(AnsaOSPF::RoutingTableEntry::NetworkDestination);
                entry->SetOptionalCapabilities(routerVertex->getHeader().getLsOptions());
                std::vector<AnsaOSPF::NextHop>* newNextHops = CalculateNextHops(link, routerVertex); // (destination, parent)
                unsigned int nextHopCount = newNextHops->size();
                for (k = 0; k < nextHopCount; k++) {
                    entry->AddNextHop((*newNextHops)[k]);
                }
                delete newNextHops;

                newRoutingTable.push_back(entry);
            }
        }
    }
}

std::vector<AnsaOSPF::NextHop>* AnsaOSPF::Area::CalculateNextHops(OSPFLSA* destination, OSPFLSA* parent) const
{
    std::vector<AnsaOSPF::NextHop>* hops = new std::vector<AnsaOSPF::NextHop>;
    unsigned long               i, j;

    AnsaOSPF::RouterLSA* routerLSA = dynamic_cast<AnsaOSPF::RouterLSA*> (parent);
    if (routerLSA != NULL) {
        if (routerLSA != spfTreeRoot) {
            unsigned int nextHopCount = routerLSA->GetNextHopCount();
            for (i = 0; i < nextHopCount; i++) {
                hops->push_back(routerLSA->GetNextHop(i));
            }
            return hops;
        } else {
            AnsaOSPF::RouterLSA* destinationRouterLSA = dynamic_cast<AnsaOSPF::RouterLSA*> (destination);
            if (destinationRouterLSA != NULL) {
                unsigned long interfaceNum   = associatedInterfaces.size();
                for (i = 0; i < interfaceNum; i++) {
                    AnsaOSPF::Interface::OSPFInterfaceType intfType = associatedInterfaces[i]->GetType();
                    if ((intfType == AnsaOSPF::Interface::PointToPoint) ||
                        ((intfType == AnsaOSPF::Interface::Virtual) &&
                         (associatedInterfaces[i]->GetState() > AnsaOSPF::Interface::LoopbackState)))
                    {
                        AnsaOSPF::Neighbor* ptpNeighbor = associatedInterfaces[i]->GetNeighborCount() > 0 ? associatedInterfaces[i]->GetNeighbor(0) : NULL;
                        if (ptpNeighbor != NULL) {
                            if (ptpNeighbor->GetNeighborID() == destinationRouterLSA->getHeader().getLinkStateID()) {
                                NextHop nextHop;
                                nextHop.ifIndex           = associatedInterfaces[i]->GetIfIndex();
                                nextHop.hopAddress        = ptpNeighbor->GetAddress();
                                nextHop.advertisingRouter = destinationRouterLSA->getHeader().getAdvertisingRouter().getInt();
                                hops->push_back(nextHop);
                                break;
                            }
                        }
                    }
                    if (intfType == AnsaOSPF::Interface::PointToMultiPoint) {
                        AnsaOSPF::Neighbor* ptmpNeighbor = associatedInterfaces[i]->GetNeighborByID(destinationRouterLSA->getHeader().getLinkStateID());
                        if (ptmpNeighbor != NULL) {
                            unsigned int   linkCount = destinationRouterLSA->getLinksArraySize();
                            AnsaOSPF::RouterID rootID    = parentRouter->GetRouterID();
                            for (j = 0; j < linkCount; j++) {
                                Link& link = destinationRouterLSA->getLinks(j);
                                if (link.getLinkID() == rootID) {
                                    NextHop nextHop;
                                    nextHop.ifIndex           = associatedInterfaces[i]->GetIfIndex();
                                    nextHop.hopAddress        = IPv4AddressFromULong(link.getLinkData());
                                    nextHop.advertisingRouter = destinationRouterLSA->getHeader().getAdvertisingRouter().getInt();
                                    hops->push_back(nextHop);
                                }
                            }
                            break;
                        }
                    }
                }
            } else {
                AnsaOSPF::NetworkLSA* destinationNetworkLSA = dynamic_cast<AnsaOSPF::NetworkLSA*> (destination);
                if (destinationNetworkLSA != NULL) {
                    AnsaOSPF::IPv4Address networkDesignatedRouter = IPv4AddressFromULong(destinationNetworkLSA->getHeader().getLinkStateID());
                    unsigned long     interfaceNum            = associatedInterfaces.size();
                    for (i = 0; i < interfaceNum; i++) {
                        AnsaOSPF::Interface::OSPFInterfaceType intfType = associatedInterfaces[i]->GetType();
                        if (((intfType == AnsaOSPF::Interface::Broadcast) ||
                             (intfType == AnsaOSPF::Interface::NBMA)) &&
                            (associatedInterfaces[i]->GetDesignatedRouter().ipInterfaceAddress == networkDesignatedRouter))
                        {
                            AnsaOSPF::IPv4AddressRange range = associatedInterfaces[i]->GetAddressRange();
                            NextHop                nextHop;

                            nextHop.ifIndex           = associatedInterfaces[i]->GetIfIndex();
                            nextHop.hopAddress        = (range.address & range.mask);
                            nextHop.advertisingRouter = destinationNetworkLSA->getHeader().getAdvertisingRouter().getInt();
                            hops->push_back(nextHop);
                        }
                    }
                }
            }
        }
    } else {
        AnsaOSPF::NetworkLSA* networkLSA = dynamic_cast<AnsaOSPF::NetworkLSA*> (parent);
        if (networkLSA != NULL) {
            if (networkLSA->GetParent() != spfTreeRoot) {
                unsigned int nextHopCount = networkLSA->GetNextHopCount();
                for (i = 0; i < nextHopCount; i++) {
                    hops->push_back(networkLSA->GetNextHop(i));
                }
                return hops;
            } else {
                unsigned long parentLinkStateID = parent->getHeader().getLinkStateID();

                AnsaOSPF::RouterLSA* destinationRouterLSA = dynamic_cast<AnsaOSPF::RouterLSA*> (destination);
                if (destinationRouterLSA != NULL) {
                    AnsaOSPF::RouterID destinationRouterID = destinationRouterLSA->getHeader().getLinkStateID();
                    unsigned int   linkCount           = destinationRouterLSA->getLinksArraySize();
                    for (i = 0; i < linkCount; i++) {
                        Link&   link = destinationRouterLSA->getLinks(i);
                        NextHop nextHop;

                        if (((link.getType() == TransitLink) &&
                             (link.getLinkID().getInt() == parentLinkStateID)) ||
                            ((link.getType() == StubLink) &&
                             ((link.getLinkID().getInt() & link.getLinkData()) == (parentLinkStateID & networkLSA->getNetworkMask().getInt()))))
                        {
                            unsigned long interfaceNum   = associatedInterfaces.size();
                            for (j = 0; j < interfaceNum; j++) {
                                AnsaOSPF::Interface::OSPFInterfaceType intfType = associatedInterfaces[j]->GetType();
                                if (((intfType == AnsaOSPF::Interface::Broadcast) ||
                                     (intfType == AnsaOSPF::Interface::NBMA)) &&
                                    (associatedInterfaces[j]->GetDesignatedRouter().ipInterfaceAddress == IPv4AddressFromULong(parentLinkStateID)))
                                {
                                    AnsaOSPF::Neighbor* nextHopNeighbor = associatedInterfaces[j]->GetNeighborByID(destinationRouterID);
                                    if (nextHopNeighbor != NULL) {
                                        nextHop.ifIndex           = associatedInterfaces[j]->GetIfIndex();
                                        nextHop.hopAddress        = nextHopNeighbor->GetAddress();
                                        nextHop.advertisingRouter = destinationRouterLSA->getHeader().getAdvertisingRouter().getInt();
                                        hops->push_back(nextHop);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return hops;
}

std::vector<AnsaOSPF::NextHop>* AnsaOSPF::Area::CalculateNextHops(Link& destination, OSPFLSA* parent) const
{
    std::vector<AnsaOSPF::NextHop>* hops = new std::vector<AnsaOSPF::NextHop>;
    unsigned long                i;

    AnsaOSPF::RouterLSA* routerLSA = check_and_cast<AnsaOSPF::RouterLSA*> (parent);
    if (routerLSA != spfTreeRoot) {
        unsigned int nextHopCount = routerLSA->GetNextHopCount();
        for (i = 0; i < nextHopCount; i++) {
            hops->push_back(routerLSA->GetNextHop(i));
        }
        return hops;
    } else {
        unsigned long interfaceNum = associatedInterfaces.size();
        for (i = 0; i < interfaceNum; i++) {
            AnsaOSPF::Interface::OSPFInterfaceType intfType = associatedInterfaces[i]->GetType();

            if ((intfType == AnsaOSPF::Interface::PointToPoint) ||
                ((intfType == AnsaOSPF::Interface::Virtual) &&
                 (associatedInterfaces[i]->GetState() > AnsaOSPF::Interface::LoopbackState)))
            {
                AnsaOSPF::Neighbor* neighbor = (associatedInterfaces[i]->GetNeighborCount() > 0) ? associatedInterfaces[i]->GetNeighbor(0) : NULL;
                if (neighbor != NULL) {
                    AnsaOSPF::IPv4Address neighborAddress = neighbor->GetAddress();
                    if (((neighborAddress != AnsaOSPF::NullIPv4Address) &&
                         (ULongFromIPv4Address(neighborAddress) == destination.getLinkID().getInt())) ||
                        ((neighborAddress == AnsaOSPF::NullIPv4Address) &&
                         (ULongFromIPv4Address(associatedInterfaces[i]->GetAddressRange().address) == destination.getLinkID().getInt()) &&
                         (ULongFromIPv4Address(associatedInterfaces[i]->GetAddressRange().mask) == destination.getLinkData())))
                    {
                        NextHop nextHop;
                        nextHop.ifIndex           = associatedInterfaces[i]->GetIfIndex();
                        nextHop.hopAddress        = neighborAddress;
                        nextHop.advertisingRouter = parentRouter->GetRouterID();
                        hops->push_back(nextHop);
                        break;
                    }
                }
            }
            if ((intfType == AnsaOSPF::Interface::Broadcast) ||
                (intfType == AnsaOSPF::Interface::NBMA))
            {
                if ((destination.getLinkID().getInt() == ULongFromIPv4Address(associatedInterfaces[i]->GetAddressRange().address & associatedInterfaces[i]->GetAddressRange().mask)) &&
                    (destination.getLinkData() == ULongFromIPv4Address(associatedInterfaces[i]->GetAddressRange().mask)))
                {
                    NextHop nextHop;
                    nextHop.ifIndex           = associatedInterfaces[i]->GetIfIndex();
                    nextHop.hopAddress        = IPv4AddressFromULong(destination.getLinkID().getInt());
                    nextHop.advertisingRouter = parentRouter->GetRouterID();
                    hops->push_back(nextHop);
                    break;
                }
            }
            if (intfType == AnsaOSPF::Interface::PointToMultiPoint) {
                if (destination.getType() == StubLink) {
                    if (destination.getLinkID().getInt() == ULongFromIPv4Address(associatedInterfaces[i]->GetAddressRange().address)) {
                        // The link contains the router's own interface address and a full mask,
                        // so we insert a next hop pointing to the interface itself. Kind of pointless, but
                        // not much else we could do...
                        // TODO: check what other OSPF implementations do in this situation
                        NextHop nextHop;
                        nextHop.ifIndex           = associatedInterfaces[i]->GetIfIndex();
                        nextHop.hopAddress        = associatedInterfaces[i]->GetAddressRange().address;
                        nextHop.advertisingRouter = parentRouter->GetRouterID();
                        hops->push_back(nextHop);
                        break;
                    }
                }
                if (destination.getType() == PointToPointLink) {
                    AnsaOSPF::Neighbor* neighbor = associatedInterfaces[i]->GetNeighborByID(destination.getLinkID().getInt());
                    if (neighbor != NULL) {
                        NextHop nextHop;
                        nextHop.ifIndex           = associatedInterfaces[i]->GetIfIndex();
                        nextHop.hopAddress        = neighbor->GetAddress();
                        nextHop.advertisingRouter = parentRouter->GetRouterID();
                        hops->push_back(nextHop);
                        break;
                    }
                }
            }
            // next hops for virtual links are generated later, after examining transit areas' SummaryLSAs
        }

        if (hops->size() == 0) {
            unsigned long hostRouteCount = hostRoutes.size();
            for (i = 0; i < hostRouteCount; i++) {
                if ((destination.getLinkID().getInt() == ULongFromIPv4Address(hostRoutes[i].address)) &&
                    (destination.getLinkData() == 0xFFFFFFFF))
                {
                    NextHop nextHop;
                    nextHop.ifIndex           = hostRoutes[i].ifIndex;
                    nextHop.hopAddress        = hostRoutes[i].address;
                    nextHop.advertisingRouter = parentRouter->GetRouterID();
                    hops->push_back(nextHop);
                    break;
                }
            }
        }
    }

    return hops;
}

bool AnsaOSPF::Area::HasLink(OSPFLSA* fromLSA, OSPFLSA* toLSA) const
{
    unsigned int i;

    AnsaOSPF::RouterLSA* fromRouterLSA = dynamic_cast<AnsaOSPF::RouterLSA*> (fromLSA);
    if (fromRouterLSA != NULL) {
        unsigned int     linkCount   = fromRouterLSA->getLinksArraySize();
        AnsaOSPF::RouterLSA* toRouterLSA = dynamic_cast<AnsaOSPF::RouterLSA*> (toLSA);
        if (toRouterLSA != NULL) {
            for (i = 0; i < linkCount; i++) {
                Link&    link     = fromRouterLSA->getLinks(i);
                LinkType linkType = static_cast<LinkType> (link.getType());

                if (((linkType == PointToPointLink) ||
                     (linkType == VirtualLink)) &&
                    (link.getLinkID().getInt() == toRouterLSA->getHeader().getLinkStateID()))
                {
                    return true;
                }
            }
        } else {
            AnsaOSPF::NetworkLSA* toNetworkLSA = dynamic_cast<AnsaOSPF::NetworkLSA*> (toLSA);
            if (toNetworkLSA != NULL) {
                for (i = 0; i < linkCount; i++) {
                    Link&    link     = fromRouterLSA->getLinks(i);

                    if ((link.getType() == TransitLink) &&
                        (link.getLinkID().getInt() == toNetworkLSA->getHeader().getLinkStateID()))
                    {
                        return true;
                    }
                    if ((link.getType() == StubLink) &&
                        ((link.getLinkID().getInt() & link.getLinkData()) == (toNetworkLSA->getHeader().getLinkStateID() & toNetworkLSA->getNetworkMask().getInt())))
                    {
                        return true;
                    }
                }
            }
        }
    } else {
        AnsaOSPF::NetworkLSA* fromNetworkLSA = dynamic_cast<AnsaOSPF::NetworkLSA*> (fromLSA);
        if (fromNetworkLSA != NULL) {
            unsigned int     routerCount   = fromNetworkLSA->getAttachedRoutersArraySize();
            AnsaOSPF::RouterLSA* toRouterLSA = dynamic_cast<AnsaOSPF::RouterLSA*> (toLSA);
            if (toRouterLSA != NULL) {
                for (i = 0; i < routerCount; i++) {
                    if (fromNetworkLSA->getAttachedRouters(i).getInt() == toRouterLSA->getHeader().getLinkStateID()) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

/**
 * Browse through the newRoutingTable looking for entries describing the same destination
 * as the currentLSA. If a cheaper route is found then skip this LSA(return true), else
 * note those which are of equal or worse cost than the currentCost.
 */
bool AnsaOSPF::Area::FindSameOrWorseCostRoute(const std::vector<AnsaOSPF::RoutingTableEntry*>& newRoutingTable,
                                           const AnsaOSPF::SummaryLSA&                      summaryLSA,
                                           unsigned short                               currentCost,
                                           bool&                                        destinationInRoutingTable,
                                           std::list<AnsaOSPF::RoutingTableEntry*>&         sameOrWorseCost) const
{
    destinationInRoutingTable = false;
    sameOrWorseCost.clear();

    long                   routeCount = newRoutingTable.size();
    AnsaOSPF::IPv4AddressRange destination;

    destination.address = IPv4AddressFromULong(summaryLSA.getHeader().getLinkStateID());
    destination.mask    = IPv4AddressFromULong(summaryLSA.getNetworkMask().getInt());

    for (long j = 0; j < routeCount; j++) {
        AnsaOSPF::RoutingTableEntry* routingEntry  = newRoutingTable[j];
        bool                     foundMatching = false;

        if (summaryLSA.getHeader().getLsType() == SummaryLSA_NetworksType) {
            if ((routingEntry->GetDestinationType() == AnsaOSPF::RoutingTableEntry::NetworkDestination) &&
                (ULongFromIPv4Address(destination.address & destination.mask) == routingEntry->GetDestinationID().getInt()))
            {
                foundMatching = true;
            }
        } else {
            if ((((routingEntry->GetDestinationType() & AnsaOSPF::RoutingTableEntry::AreaBorderRouterDestination) != 0) ||
                 ((routingEntry->GetDestinationType() & AnsaOSPF::RoutingTableEntry::ASBoundaryRouterDestination) != 0)) &&
                (ULongFromIPv4Address(destination.address) == routingEntry->GetDestinationID().getInt()))
            {
                foundMatching = true;
            }
        }

        if (foundMatching) {
            destinationInRoutingTable = true;

            /* If the matching entry is an IntraArea getRoute(intra-area paths are
                * always preferred to other paths of any cost), or it's a cheaper InterArea
                * route, then skip this LSA.
                */
            if ((routingEntry->GetPathType() == AnsaOSPF::RoutingTableEntry::IntraArea) ||
                ((routingEntry->GetPathType() == AnsaOSPF::RoutingTableEntry::InterArea) &&
                 (routingEntry->GetCost() < currentCost)))
            {
                return true;
            } else {
                // if it's an other InterArea path
                if ((routingEntry->GetPathType() == AnsaOSPF::RoutingTableEntry::InterArea) &&
                    (routingEntry->GetCost() >= currentCost))
                {
                    sameOrWorseCost.push_back(routingEntry);
                }   // else it's external -> same as if not in the table
            }
        }
    }
    return false;
}

/**
 * Returns a new RoutingTableEntry based on the input SummaryLSA, with the input cost
 * and the borderRouterEntry's next hops.
 */
AnsaOSPF::RoutingTableEntry* AnsaOSPF::Area::CreateRoutingTableEntryFromSummaryLSA(const AnsaOSPF::SummaryLSA&        summaryLSA,
                                                                            unsigned short                 entryCost,
                                                                            const AnsaOSPF::RoutingTableEntry& borderRouterEntry) const
{
    AnsaOSPF::IPv4AddressRange destination;

    destination.address = IPv4AddressFromULong(summaryLSA.getHeader().getLinkStateID());
    destination.mask    = IPv4AddressFromULong(summaryLSA.getNetworkMask().getInt());

    AnsaOSPF::RoutingTableEntry* newEntry = new AnsaOSPF::RoutingTableEntry;

    if (summaryLSA.getHeader().getLsType() == SummaryLSA_NetworksType) {
        newEntry->SetDestinationID(ULongFromIPv4Address(destination.address & destination.mask));
        newEntry->SetAddressMask(ULongFromIPv4Address(destination.mask));
        newEntry->SetDestinationType(AnsaOSPF::RoutingTableEntry::NetworkDestination);
    } else {
        newEntry->SetDestinationID(ULongFromIPv4Address(destination.address));
        newEntry->SetAddressMask(0xFFFFFFFF);
        newEntry->SetDestinationType(AnsaOSPF::RoutingTableEntry::ASBoundaryRouterDestination);
    }
    newEntry->SetArea(areaID);
    newEntry->SetPathType(AnsaOSPF::RoutingTableEntry::InterArea);
    newEntry->SetCost(entryCost);
    newEntry->SetOptionalCapabilities(summaryLSA.getHeader().getLsOptions());
    newEntry->SetLinkStateOrigin(&summaryLSA);

    unsigned int nextHopCount = borderRouterEntry.GetNextHopCount();
    for (unsigned int j = 0; j < nextHopCount; j++) {
        newEntry->AddNextHop(borderRouterEntry.GetNextHop(j));
    }

    return newEntry;
}

/**
 * @see RFC 2328 Section 16.2.
 * @todo This function does a lot of lookup in the input newRoutingTable.
 *       Restructuring the input vector into some kind of hash would quite
 *       probably speed up execution.
 */
void AnsaOSPF::Area::CalculateInterAreaRoutes(std::vector<AnsaOSPF::RoutingTableEntry*>& newRoutingTable)
{
    unsigned long i = 0;
    unsigned long j = 0;
    unsigned long lsaCount = summaryLSAs.size();

    for (i = 0; i < lsaCount; i++) {
        AnsaOSPF::SummaryLSA* currentLSA        = summaryLSAs[i];
        OSPFLSAHeader&    currentHeader     = currentLSA->getHeader();

        unsigned long     routeCost         = currentLSA->getRouteCost();
        unsigned short    lsAge             = currentHeader.getLsAge();
        RouterID          originatingRouter = currentHeader.getAdvertisingRouter().getInt();
        bool              selfOriginated    = (originatingRouter == parentRouter->GetRouterID());

        if ((routeCost == LS_INFINITY) || (lsAge == MAX_AGE) || (selfOriginated)) { // (1) and(2)
            continue;
        }

        char                   lsType     = currentHeader.getLsType();
        unsigned long          routeCount = newRoutingTable.size();
        AnsaOSPF::IPv4AddressRange destination;

        destination.address = IPv4AddressFromULong(currentHeader.getLinkStateID());
        destination.mask    = IPv4AddressFromULong(currentLSA->getNetworkMask().getInt());

        if ((lsType == SummaryLSA_NetworksType) && (parentRouter->HasAddressRange(destination))) { // (3)
            bool foundIntraAreaRoute = false;

            // look for an "Active" IntraArea route
            for (j = 0; j < routeCount; j++) {
                AnsaOSPF::RoutingTableEntry* routingEntry = newRoutingTable[j];

                if ((routingEntry->GetDestinationType() == AnsaOSPF::RoutingTableEntry::NetworkDestination) &&
                    (routingEntry->GetPathType() == AnsaOSPF::RoutingTableEntry::IntraArea) &&
                    ((routingEntry->GetDestinationID().getInt() &
                      routingEntry->GetAddressMask().getInt()   &
                      ULongFromIPv4Address(destination.mask)       ) == ULongFromIPv4Address(destination.address &
                                                                                               destination.mask)))
                {
                    foundIntraAreaRoute = true;
                    break;
                }
            }
            if (foundIntraAreaRoute) {
                continue;
            }
        }

        AnsaOSPF::RoutingTableEntry* borderRouterEntry = NULL;

        // The routingEntry describes a route to an other area -> look for the border router originating it
        for (j = 0; j < routeCount; j++) {     // (4) N == destination, BR == borderRouterEntry
            AnsaOSPF::RoutingTableEntry* routingEntry = newRoutingTable[j];

            if ((routingEntry->GetArea() == areaID) &&
                (((routingEntry->GetDestinationType() & AnsaOSPF::RoutingTableEntry::AreaBorderRouterDestination) != 0) ||
                 ((routingEntry->GetDestinationType() & AnsaOSPF::RoutingTableEntry::ASBoundaryRouterDestination) != 0)) &&
                (routingEntry->GetDestinationID().getInt() == originatingRouter))
            {
                borderRouterEntry = routingEntry;
                break;
            }
        }
        if (borderRouterEntry == NULL) {
            continue;
        } else {    // (5)
            /* "Else, this LSA describes an inter-area path to destination N,
             * whose cost is the distance to BR plus the cost specified in the LSA.
             * Call the cost of this inter-area path IAC."
             */
            bool                                destinationInRoutingTable = true;
            unsigned short                      currentCost               = routeCost + borderRouterEntry->GetCost();
            std::list<AnsaOSPF::RoutingTableEntry*> sameOrWorseCost;

            if (FindSameOrWorseCostRoute(newRoutingTable,
                                          *currentLSA,
                                          currentCost,
                                          destinationInRoutingTable,
                                          sameOrWorseCost))
            {
                continue;
            }

            if (destinationInRoutingTable && (sameOrWorseCost.size() > 0)) {
                AnsaOSPF::RoutingTableEntry* equalEntry = NULL;

                /* Look for an equal cost entry in the sameOrWorseCost list, and
                 * also clear the more expensive entries from the newRoutingTable.
                 */
                for (std::list<AnsaOSPF::RoutingTableEntry*>::iterator it = sameOrWorseCost.begin(); it != sameOrWorseCost.end(); it++) {
                    AnsaOSPF::RoutingTableEntry* checkedEntry = (*it);

                    if (checkedEntry->GetCost() > currentCost) {
                        for (std::vector<AnsaOSPF::RoutingTableEntry*>::iterator entryIt = newRoutingTable.begin(); entryIt != newRoutingTable.end(); entryIt++) {
                            if (checkedEntry == (*entryIt)) {
                                newRoutingTable.erase(entryIt);
                                break;
                            }
                        }
                    } else {    // EntryCost == currentCost
                        equalEntry = checkedEntry;  // should be only one - if there are more they are ignored
                    }
                }

                unsigned long nextHopCount = borderRouterEntry->GetNextHopCount();

                if (equalEntry != NULL) {
                    /* Add the next hops of the border router advertising this destination
                     * to the equal entry.
                     */
                    for (unsigned long j = 0; j < nextHopCount; j++) {
                        equalEntry->AddNextHop(borderRouterEntry->GetNextHop(j));
                    }
                } else {
                    AnsaOSPF::RoutingTableEntry* newEntry = CreateRoutingTableEntryFromSummaryLSA(*currentLSA, currentCost, *borderRouterEntry);
                    ASSERT(newEntry != NULL);
                    newRoutingTable.push_back(newEntry);
                }
            } else {
                AnsaOSPF::RoutingTableEntry* newEntry = CreateRoutingTableEntryFromSummaryLSA(*currentLSA, currentCost, *borderRouterEntry);
                ASSERT(newEntry != NULL);
                newRoutingTable.push_back(newEntry);
            }
        }
    }
}

void AnsaOSPF::Area::ReCheckSummaryLSAs(std::vector<AnsaOSPF::RoutingTableEntry*>& newRoutingTable)
{
    unsigned long i = 0;
    unsigned long j = 0;
    unsigned long lsaCount = summaryLSAs.size();

    for (i = 0; i < lsaCount; i++) {
        AnsaOSPF::SummaryLSA* currentLSA        = summaryLSAs[i];
        OSPFLSAHeader&    currentHeader     = currentLSA->getHeader();

        unsigned long     routeCost         = currentLSA->getRouteCost();
        unsigned short    lsAge             = currentHeader.getLsAge();
        RouterID          originatingRouter = currentHeader.getAdvertisingRouter().getInt();
        bool              selfOriginated    = (originatingRouter == parentRouter->GetRouterID());

        if ((routeCost == LS_INFINITY) || (lsAge == MAX_AGE) || (selfOriginated)) { // (1) and(2)
            continue;
        }

        unsigned long            routeCount       = newRoutingTable.size();
        char                     lsType           = currentHeader.getLsType();
        AnsaOSPF::RoutingTableEntry* destinationEntry = NULL;
        AnsaOSPF::IPv4AddressRange   destination;

        destination.address = IPv4AddressFromULong(currentHeader.getLinkStateID());
        destination.mask    = IPv4AddressFromULong(currentLSA->getNetworkMask().getInt());

        for (j = 0; j < routeCount; j++) {  // (3)
            AnsaOSPF::RoutingTableEntry* routingEntry  = newRoutingTable[j];
            bool                     foundMatching = false;

            if (lsType == SummaryLSA_NetworksType) {
                if ((routingEntry->GetDestinationType() == AnsaOSPF::RoutingTableEntry::NetworkDestination) &&
                    (ULongFromIPv4Address(destination.address & destination.mask) == routingEntry->GetDestinationID().getInt()))
                {
                    foundMatching = true;
                }
            } else {
                if ((((routingEntry->GetDestinationType() & AnsaOSPF::RoutingTableEntry::AreaBorderRouterDestination) != 0) ||
                     ((routingEntry->GetDestinationType() & AnsaOSPF::RoutingTableEntry::ASBoundaryRouterDestination) != 0)) &&
                    (ULongFromIPv4Address(destination.address) == routingEntry->GetDestinationID().getInt()))
                {
                    foundMatching = true;
                }
            }

            if (foundMatching) {
                AnsaOSPF::RoutingTableEntry::RoutingPathType pathType = routingEntry->GetPathType();

                if ((pathType == AnsaOSPF::RoutingTableEntry::Type1External) ||
                    (pathType == AnsaOSPF::RoutingTableEntry::Type2External) ||
                    (routingEntry->GetArea() != AnsaOSPF::BackboneAreaID))
                {
                    break;
                } else {
                    destinationEntry = routingEntry;
                    break;
                }
            }
        }
        if (destinationEntry == NULL) {
            continue;
        }

        AnsaOSPF::RoutingTableEntry* borderRouterEntry = NULL;
        unsigned short           currentCost       = routeCost;

        for (j = 0; j < routeCount; j++) {     // (4) BR == borderRouterEntry
            AnsaOSPF::RoutingTableEntry* routingEntry = newRoutingTable[j];

            if ((routingEntry->GetArea() == areaID) &&
                (((routingEntry->GetDestinationType() & AnsaOSPF::RoutingTableEntry::AreaBorderRouterDestination) != 0) ||
                 ((routingEntry->GetDestinationType() & AnsaOSPF::RoutingTableEntry::ASBoundaryRouterDestination) != 0)) &&
                (routingEntry->GetDestinationID().getInt() == originatingRouter))
            {
                borderRouterEntry = routingEntry;
                currentCost += borderRouterEntry->GetCost();
                break;
            }
        }
        if (borderRouterEntry == NULL) {
            continue;
        } else {    // (5)
            if (currentCost <= destinationEntry->GetCost()) {
                if (currentCost < destinationEntry->GetCost()) {
                    destinationEntry->ClearNextHops();
                }

                unsigned long nextHopCount = borderRouterEntry->GetNextHopCount();

                for (j = 0; j < nextHopCount; j++) {
                    destinationEntry->AddNextHop(borderRouterEntry->GetNextHop(j));
                }
            }
        }
    }
}
