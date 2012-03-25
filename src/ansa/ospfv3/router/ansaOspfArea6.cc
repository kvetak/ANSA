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

#include "ansaOspfArea6.h"
#include "ansaOspfRouter6.h"


AnsaOspf6::Area::Area(AnsaOspf6::AreaID id) :
      areaID(id),
      transitCapability(false),
      externalRoutingCapability(true),
      stubDefaultCost(1),
      spfTreeRoot(NULL),
      parentRouter(NULL){

   //
}

AnsaOspf6::Area::~Area(void) {
   int interfaceNum = associatedInterfaces.size();
   for (int i = 0; i < interfaceNum; i++){
      delete (associatedInterfaces[i]);
   }


   long lsaCount = routerLSAs.size();
   for (long j = 0; j < lsaCount; j++){
      delete routerLSAs[j];
   }
   routerLSAs.clear();

   lsaCount = networkLSAs.size();
   for (long k = 0; k < lsaCount; k++){
      delete networkLSAs[k];
   }
   networkLSAs.clear();

   lsaCount = interAreaPrefixLSAs.size();
   for (long k = 0; k < lsaCount; k++){
      delete interAreaPrefixLSAs[k];
   }
   interAreaPrefixLSAs.clear();

   lsaCount = interAreaRouterLSAs.size();
   for (long k = 0; k < lsaCount; k++){
      delete interAreaRouterLSAs[k];
   }
   interAreaRouterLSAs.clear();

   lsaCount = intraAreaPrefixLSAs.size();
   for (long k = 0; k < lsaCount; k++){
      delete intraAreaPrefixLSAs[k];
   }
   intraAreaPrefixLSAs.clear();
}

void AnsaOspf6::Area::AddInterface(AnsaOspf6::Interface* intf) {
   intf->SetArea(this);
   associatedInterfaces.push_back(intf);
}

Interface* AnsaOspf6::Area::GetInterfaceByIndex(unsigned int index) {
   if (index < associatedInterfaces.size()){
      return associatedInterfaces[index];
   }else{
      return NULL;
   }
}

void AnsaOspf6::Area::info(char *buffer) {
   std::stringstream out;
   char areaString[16];
   out << "areaID: " << areaID;
   strcpy(buffer, out.str().c_str());
}

std::string AnsaOspf6::Area::detailedInfo(void) const {
   std::stringstream out;
   char addressString[16];
   int i;
   out << "\n    areaID: " << areaID << ", ";
   out << "transitCapability: " << (transitCapability ? "true" : "false") << ", ";
   out << "externalRoutingCapability: " << (externalRoutingCapability ? "true" : "false") << ", ";
   out << "stubDefaultCost: " << stubDefaultCost << "\n";
   int addressPrefixNum = areaAddressPrefixes.size();
   for (i = 0; i < addressPrefixNum; i++) {
      out << "    addressPrefix[" << i << "]: ";
      out << areaAddressPrefixes[i].address;
      out << "/" << areaAddressPrefixes[i].prefixLen << "\n";
   }
   int interfaceNum = associatedInterfaces.size();
   for (i = 0; i < interfaceNum; i++) {
      out << "    interface[" << i << "]: link-local address: ";
      out << associatedInterfaces[i]->GetInterfaceAddress() << "\n";
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

   out << "      InterAreaPrefixLSAs:\n";
   lsaCount = interAreaPrefixLSAs.size();
   for (i = 0; i < lsaCount; i++) {
      out << "        " << *interAreaPrefixLSAs[i] << "\n";
   }

   out << "      InterAreaRouterLSAs:\n";
   lsaCount = interAreaRouterLSAs.size();
   for (i = 0; i < lsaCount; i++) {
      out << "        " << *interAreaRouterLSAs[i] << "\n";
   }

   out << "      IntraAreaPrefixLSAs:\n";
   lsaCount = intraAreaPrefixLSAs.size();
   for (i = 0; i < lsaCount; i++) {
      out << "        " << *intraAreaPrefixLSAs[i] << "\n";
   }

   out << "--------------------------------------------------------------------------------";

   return out.str();
}

bool AnsaOspf6::Area::ContainsAddress(IPv6Address address) const {
   int addressPrefixesNum = areaAddressPrefixes.size();
   for (int i = 0; i < addressPrefixesNum; i++){
      if (address.matches(areaAddressPrefixes[i].address, areaAddressPrefixes[i].prefixLen)){
         return true;
      }
   }
   return false;
}

bool AnsaOspf6::Area::HasAddressPrefix(AnsaOspf6::IPv6AddressPrefix addressPrefix) const {
   int addressPrefixesNum = areaAddressPrefixes.size();
   for (int i = 0; i < addressPrefixesNum; i++){
      if (areaAddressPrefixes[i] == addressPrefix){
         return true;
      }
   }
   return false;
}

AnsaOspf6::IPv6AddressPrefix AnsaOspf6::Area::GetContainingAddressRange(AnsaOspf6::IPv6AddressPrefix addressPrefix, bool* advertise) const {
   int addressPrefixesNum = areaAddressPrefixes.size();
   for (int i = 0; i < addressPrefixesNum; i++){
      if ((addressPrefix == areaAddressPrefixes[i]) && (advertise != NULL)){
         std::map<AnsaOspf6::IPv6AddressPrefix, bool, AnsaOspf6::IPv6AddressPrefix_Less>::const_iterator prefixIt;
         prefixIt = advertiseAddressPrefixes.find(areaAddressPrefixes[i]);
         if (prefixIt != advertiseAddressPrefixes.end()){
            *advertise = prefixIt->second;
         }else{
            *advertise = true;
         }
         return areaAddressPrefixes[i];
      }
   }
   if (advertise != NULL){
      *advertise = false;
   }
   return AnsaOspf6::NullAddressPrefix;
}

AnsaOspf6::Interface* AnsaOspf6::Area::GetInterface(unsigned char ifIndex) {
   int interfaceNum = associatedInterfaces.size();
   for (int i = 0; i < interfaceNum; i++){
      if ((associatedInterfaces[i]->GetType() != AnsaOspf6::Interface::Virtual)
            && (associatedInterfaces[i]->GetIfIndex() == ifIndex)){
         return associatedInterfaces[i];
      }
   }
   return NULL;
}

AnsaOspf6::Interface* AnsaOspf6::Area::GetInterface(IPv6Address address) {
   int interfaceNum = associatedInterfaces.size();
   for (int i = 0; i < interfaceNum; i++){
      if (  (associatedInterfaces[i]->GetType() != AnsaOspf6::Interface::Virtual)
         && (associatedInterfaces[i]->GetInterfaceAddress() == address)){
         return associatedInterfaces[i];
      }
   }
   return NULL;
}

bool AnsaOspf6::Area::HasVirtualLink(AnsaOspf6::AreaID withTransitArea) const {
   if ((areaID != AnsaOspf6::BackboneAreaID) || (withTransitArea == AnsaOspf6::BackboneAreaID)){
      return false;
   }

   int interfaceNum = associatedInterfaces.size();
   for (int i = 0; i < interfaceNum; i++){
      if (  (associatedInterfaces[i]->GetType() == AnsaOspf6::Interface::Virtual)
         && (associatedInterfaces[i]->GetTransitAreaID() == withTransitArea)){
         return true;
      }
   }
   return false;
}

AnsaOspf6::Interface* AnsaOspf6::Area::FindVirtualLink(AnsaOspf6::RouterID routerID) {
   int interfaceNum = associatedInterfaces.size();
   for (int i = 0; i < interfaceNum; i++){
      if (  (associatedInterfaces[i]->GetType() == AnsaOspf6::Interface::Virtual)
         && (associatedInterfaces[i]->GetNeighborByID(routerID) != NULL)){
         return associatedInterfaces[i];
      }
   }
   return NULL;
}


bool AnsaOspf6::Area::InstallRouterLSA(OspfRouterLsa6* lsa){
   AnsaOspf6::LinkStateID linkStateID = lsa->getHeader().getLinkStateID();
   std::map<AnsaOspf6::LinkStateID, AnsaOspf6::RouterLsa*>::iterator lsaIt = routerLSAsByID.find(linkStateID);
   if (lsaIt != routerLSAsByID.end()){
      AnsaOspf6::LsaKeyType6 lsaKey;

      lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
      lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

      RemoveFromAllRetransmissionLists(lsaKey);
      return lsaIt->second->Update(lsa);
   } else {
      AnsaOspf6::RouterLsa* lsaCopy = new AnsaOspf6::RouterLsa(*lsa);
      routerLSAsByID[linkStateID] = lsaCopy;
      routerLSAs.push_back(lsaCopy);
      return true;
   }
}


bool AnsaOspf6::Area::InstallNetworkLSA(OspfNetworkLsa6* lsa){
    AnsaOspf6::LinkStateID linkStateID = lsa->getHeader().getLinkStateID();
    std::map<AnsaOspf6::LinkStateID, AnsaOspf6::NetworkLsa*>::iterator lsaIt = networkLSAsByID.find(linkStateID);
    if (lsaIt != networkLSAsByID.end()) {
        AnsaOspf6::LsaKeyType6 lsaKey;

        lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
        lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

        RemoveFromAllRetransmissionLists(lsaKey);
        return lsaIt->second->Update(lsa);
    } else {
        AnsaOspf6::NetworkLsa* lsaCopy = new AnsaOspf6::NetworkLsa(*lsa);
        networkLSAsByID[linkStateID] = lsaCopy;
        networkLSAs.push_back(lsaCopy);
        return true;
    }
}

bool AnsaOspf6::Area::InstallInterAreaPrefixLSA(OspfInterAreaPrefixLsa6* lsa){
   AnsaOspf6::LinkStateID linkStateID = lsa->getHeader().getLinkStateID();
   std::map<AnsaOspf6::LinkStateID, AnsaOspf6::InterAreaPrefixLsa*>::iterator lsaIt = interAreaPrefixLSAsByID.find(linkStateID);
   if (lsaIt != interAreaPrefixLSAsByID.end()) {
       AnsaOspf6::LsaKeyType6 lsaKey;

       lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
       lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

       RemoveFromAllRetransmissionLists(lsaKey);
       return lsaIt->second->Update(lsa);
   } else {
       AnsaOspf6::InterAreaPrefixLsa* lsaCopy = new AnsaOspf6::InterAreaPrefixLsa(*lsa);
       interAreaPrefixLSAsByID[linkStateID] = lsaCopy;
       interAreaPrefixLSAs.push_back(lsaCopy);
       return true;
   }
}

bool AnsaOspf6::Area::InstallInterAreaRouterLSA(OspfInterAreaRouterLsa6* lsa){
   AnsaOspf6::LinkStateID linkStateID = lsa->getHeader().getLinkStateID();
   std::map<AnsaOspf6::LinkStateID, AnsaOspf6::InterAreaRouterLsa*>::iterator lsaIt = interAreaRouterLSAsByID.find(linkStateID);
   if (lsaIt != interAreaRouterLSAsByID.end()) {
       AnsaOspf6::LsaKeyType6 lsaKey;

       lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
       lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

       RemoveFromAllRetransmissionLists(lsaKey);
       return lsaIt->second->Update(lsa);
   } else {
       AnsaOspf6::InterAreaRouterLsa* lsaCopy = new AnsaOspf6::InterAreaRouterLsa(*lsa);
       interAreaRouterLSAsByID[linkStateID] = lsaCopy;
       interAreaRouterLSAs.push_back(lsaCopy);
       return true;
   }
}

bool AnsaOspf6::Area::InstallIntraAreaPrefixLSA(OspfIntraAreaPrefixLsa6* lsa){
   AnsaOspf6::LinkStateID linkStateID = lsa->getHeader().getLinkStateID();
   std::map<AnsaOspf6::LinkStateID, AnsaOspf6::IntraAreaPrefixLsa*>::iterator lsaIt = intraAreaPrefixLSAsByID.find(linkStateID);
   if (lsaIt != intraAreaPrefixLSAsByID.end()) {
       AnsaOspf6::LsaKeyType6 lsaKey;

       lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
       lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

       RemoveFromAllRetransmissionLists(lsaKey);
       return lsaIt->second->Update(lsa);
   } else {
       AnsaOspf6::IntraAreaPrefixLsa* lsaCopy = new AnsaOspf6::IntraAreaPrefixLsa(*lsa);
       intraAreaPrefixLSAsByID[linkStateID] = lsaCopy;
       intraAreaPrefixLSAs.push_back(lsaCopy);
       return true;
   }
}


AnsaOspf6::RouterLsa* AnsaOspf6::Area::FindRouterLSA(AnsaOspf6::LinkStateID linkStateID){
   std::map<AnsaOspf6::LinkStateID, AnsaOspf6::RouterLsa*>::iterator lsaIt = routerLSAsByID.find(linkStateID);
   if (lsaIt != routerLSAsByID.end()){
      return lsaIt->second;
   }else{
      return NULL;
   }
}

const AnsaOspf6::RouterLsa* AnsaOspf6::Area::FindRouterLSA(AnsaOspf6::LinkStateID linkStateID) const {
   return FindRouterLSA(linkStateID);
}

AnsaOspf6::NetworkLsa* AnsaOspf6::Area::FindNetworkLSA(AnsaOspf6::LinkStateID linkStateID){
   std::map<AnsaOspf6::LinkStateID, AnsaOspf6::NetworkLsa*>::iterator lsaIt = networkLSAsByID.find(linkStateID);
   if (lsaIt != networkLSAsByID.end()) {
      return lsaIt->second;
   } else {
      return NULL;
   }
}

const AnsaOspf6::NetworkLsa* AnsaOspf6::Area::FindNetworkLSA(AnsaOspf6::LinkStateID linkStateID) const {
   return FindNetworkLSA(linkStateID);
}

AnsaOspf6::InterAreaPrefixLsa* AnsaOspf6::Area::FindInterAreaPrefixLSA(AnsaOspf6::LinkStateID linkStateID){
   std::map<AnsaOspf6::LinkStateID, AnsaOspf6::InterAreaPrefixLsa*>::iterator lsaIt = interAreaPrefixLSAsByID.find(linkStateID);
   if (lsaIt != interAreaPrefixLSAsByID.end()) {
      return lsaIt->second;
   } else {
      return NULL;
   }
}

const AnsaOspf6::InterAreaPrefixLsa* AnsaOspf6::Area::FindInterAreaPrefixLSA(AnsaOspf6::LinkStateID linkStateID) const {
   return FindInterAreaPrefixLSA(linkStateID);
}

AnsaOspf6::InterAreaRouterLsa* AnsaOspf6::Area::FindInterAreaRouterLSA(AnsaOspf6::LinkStateID linkStateID){
   std::map<AnsaOspf6::LinkStateID, AnsaOspf6::InterAreaRouterLsa*>::iterator lsaIt = interAreaRouterLSAsByID.find(linkStateID);
   if (lsaIt != interAreaRouterLSAsByID.end()) {
      return lsaIt->second;
   } else {
      return NULL;
   }
}

const AnsaOspf6::InterAreaRouterLsa* AnsaOspf6::Area::FindInterAreaRouterLSA(AnsaOspf6::LinkStateID linkStateID) const {
   return FindInterAreaRouterLSA(linkStateID);
}

AnsaOspf6::IntraAreaPrefixLsa* AnsaOspf6::Area::FindIntraAreaPrefixLSA(AnsaOspf6::LinkStateID linkStateID){
   std::map<AnsaOspf6::LinkStateID, AnsaOspf6::IntraAreaPrefixLsa*>::iterator lsaIt = intraAreaPrefixLSAsByID.find(linkStateID);
   if (lsaIt != intraAreaPrefixLSAsByID.end()) {
      return lsaIt->second;
   } else {
      return NULL;
   }
}

const AnsaOspf6::IntraAreaPrefixLsa* AnsaOspf6::Area::FindIntraAreaPrefixLSA(AnsaOspf6::LinkStateID linkStateID) const {
   return FindIntraAreaPrefixLSA(linkStateID);
}


void AnsaOspf6::Area::RemoveParentFromRoutingInfo(OspfLsa6* parent) {

   long lsaCount = routerLSAs.size();
   long i;

   for (i = 0; i < lsaCount; i++){
      if (routerLSAs[i] != NULL){
         AnsaOspf6::RouterLsa* routerLSA = routerLSAs[i];
         AnsaOspf6::RoutingInfo* routingInfo = check_and_cast<AnsaOspf6::RoutingInfo*> (routerLSA);

         if (routingInfo->GetParent() == parent)
            routingInfo->SetParent(NULL);
      }
   }

   lsaCount = networkLSAs.size();

   for (i = 0; i < lsaCount; i++){
      if (networkLSAs[i] != NULL){
         AnsaOspf6::NetworkLsa* networkLSA = networkLSAs[i];
         AnsaOspf6::RoutingInfo* routingInfo = check_and_cast<AnsaOspf6::RoutingInfo*> (networkLSA);

         if (routingInfo->GetParent() == parent)
            routingInfo->SetParent(NULL);
      }
   }

   lsaCount = interAreaPrefixLSAs.size();

   for (i = 0; i < lsaCount; i++){
      if (interAreaPrefixLSAs[i] != NULL){
         AnsaOspf6::InterAreaPrefixLsa* interAreaPrefixLSA = interAreaPrefixLSAs[i];
         AnsaOspf6::RoutingInfo* routingInfo = check_and_cast<AnsaOspf6::RoutingInfo*> (interAreaPrefixLSA);

         if (routingInfo->GetParent() == parent)
            routingInfo->SetParent(NULL);
      }
   }

   lsaCount = interAreaRouterLSAs.size();

   for (i = 0; i < lsaCount; i++){
      if (interAreaRouterLSAs[i] != NULL){
         AnsaOspf6::InterAreaRouterLsa* interAreaRouterLSA = interAreaRouterLSAs[i];
         AnsaOspf6::RoutingInfo* routingInfo = check_and_cast<AnsaOspf6::RoutingInfo*> (interAreaRouterLSA);

         if (routingInfo->GetParent() == parent)
            routingInfo->SetParent(NULL);
      }
   }

   lsaCount = intraAreaPrefixLSAs.size();

   for (i = 0; i < lsaCount; i++){
      if (intraAreaPrefixLSAs[i] != NULL){
         AnsaOspf6::IntraAreaPrefixLsa* intraAreaPrefixLSA = intraAreaPrefixLSAs[i];
         AnsaOspf6::RoutingInfo* routingInfo = check_and_cast<AnsaOspf6::RoutingInfo*> (intraAreaPrefixLSA);

         if (routingInfo->GetParent() == parent)
            routingInfo->SetParent(NULL);
      }
   }
}


void AnsaOspf6::Area::AgeDatabase(void) {
   /*
   bool rebuildRoutingTable = false;
   long i;

   RouterID routerId = GetRouter()->GetRouterID();
   EV << IPAddress(routerId);

   long lsaCount = routerLSAs.size();
   for (i = 0; i < lsaCount; i++){
      unsigned short lsAge = routerLSAs[i]->getHeader().getLsAge();
      bool selfOriginated = (routerLSAs[i]->getHeader().getAdvertisingRouter() == parentRouter->GetRouterID());
      bool unreachable = parentRouter->IsDestinationUnreachable(routerLSAs[i]);
      AnsaOspf6::RouterLsa* lsa = routerLSAs[i];

      unsigned int linkCount = lsa->getLinksArraySize();
      for (unsigned int j = 0; j < linkCount; j++) {
         const Link6& link = lsa->getLinks(j);
         printLsaLink(link, ev.getOStream());
        }

        if (   (selfOriginated && (lsAge < (LS_REFRESH_TIME - 1)))
            || (!selfOriginated && (lsAge < (MAX_AGE - 1)))) {
            lsa->getHeader().setLsAge(lsAge + 1);
            lsa->IncrementInstallTime();
        }

        if (selfOriginated && (lsAge == (LS_REFRESH_TIME - 1))){
           if (unreachable) {
              lsa->getHeader().setLsAge(MAX_AGE);
              FloodLSA(lsa);
              lsa->IncrementInstallTime();
           }else{
              long sequenceNumber = lsa->getHeader().getLsSequenceNumber();
              if (sequenceNumber == MAX_SEQUENCE_NUMBER) {
                 lsa->getHeader().setLsAge(MAX_AGE);
                 FloodLSA(lsa);
                 lsa->IncrementInstallTime();
              }else{
                 AnsaOspf6::RouterLsa* newLSA = OriginateRouterLSA();

                 newLSA->getHeader().setLsSequenceNumber(sequenceNumber + 1);
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
           AnsaOspf6::LsaKeyType6 lsaKey;

           lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
           lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter();

           if (   !IsOnAnyRetransmissionList(lsaKey)
              &&  !HasAnyNeighborInStates(AnsaOspf6::Neighbor::ExchangeState | AnsaOspf6::Neighbor::LoadingState)){

              if (!selfOriginated || unreachable) {
                 routerLSAsByID.erase(lsa->getHeader().getLinkStateID());
                 delete lsa;
                 routerLSAs[i] = NULL;
                 rebuildRoutingTable = true;
              }else{
                 AnsaOspf6::RouterLsa* newLSA = OriginateRouterLSA();
                 long sequenceNumber = lsa->getHeader().getLsSequenceNumber();

                 newLSA->getHeader().setLsSequenceNumber((sequenceNumber == MAX_SEQUENCE_NUMBER) ? INITIAL_SEQUENCE_NUMBER : sequenceNumber + 1);
                 rebuildRoutingTable |= lsa->Update(newLSA);
                 delete newLSA;

                 FloodLSA(lsa);
              }
           }
        }
   }


    std::vector<RouterLsa*>::iterator routerIt = routerLSAs.begin();
    while (routerIt != routerLSAs.end()) {
       if ((*routerIt) == NULL) {
          routerIt = routerLSAs.erase(routerIt);
       }else{
          routerIt++;
       }
    }

    lsaCount = networkLSAs.size();
    for (i = 0; i < lsaCount; i++) {
        unsigned short    lsAge          = networkLSAs[i]->getHeader().getLsAge();
        bool              unreachable    = parentRouter->IsDestinationUnreachable(networkLSAs[i]);
        AnsaOspf6::NetworkLSA* lsa            = networkLSAs[i];
        AnsaOspf6::Interface*  localIntf      = GetInterface(IPv4AddressFromULong(lsa->getHeader().getLinkStateID()));
        bool              selfOriginated = false;


        if ((localIntf != NULL) &&
            (localIntf->GetState() == AnsaOspf6::Interface::DesignatedRouterState) &&
            (localIntf->GetNeighborCount() > 0) &&
            (localIntf->HasAnyNeighborInStates(AnsaOspf6::Neighbor::FullState)))
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
                    AnsaOspf6::NetworkLSA* newLSA = OriginateNetworkLSA(localIntf);

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
            AnsaOspf6::LSAKeyType lsaKey;

            lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
            lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter().getInt();

            if (!IsOnAnyRetransmissionList(lsaKey) &&
                !HasAnyNeighborInStates(AnsaOspf6::Neighbor::ExchangeState | AnsaOspf6::Neighbor::LoadingState))
            {
                if (!selfOriginated || unreachable) {
                    networkLSAsByID.erase(lsa->getHeader().getLinkStateID());
                    RemoveParentFromRoutingInfo(check_and_cast<OSPFLSA*> (lsa));
                    delete lsa;
                    networkLSAs[i] = NULL;
                    rebuildRoutingTable = true;
                } else {
                    AnsaOspf6::NetworkLSA* newLSA              = OriginateNetworkLSA(localIntf);
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
        AnsaOspf6::SummaryLSA* lsa            = summaryLSAs[i];


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
                    AnsaOspf6::SummaryLSA* newLSA = OriginateSummaryLSA(lsa);

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
            AnsaOspf6::LSAKeyType lsaKey;

            lsaKey.linkStateID = lsa->getHeader().getLinkStateID();
            lsaKey.advertisingRouter = lsa->getHeader().getAdvertisingRouter().getInt();

            if (!IsOnAnyRetransmissionList(lsaKey) &&
                !HasAnyNeighborInStates(AnsaOspf6::Neighbor::ExchangeState | AnsaOspf6::Neighbor::LoadingState))
            {
                if (!selfOriginated || unreachable) {
                    summaryLSAsByID.erase(lsaKey);
                    delete lsa;
                    summaryLSAs[i] = NULL;
                    rebuildRoutingTable = true;
                } else {
                    AnsaOspf6::SummaryLSA* newLSA = OriginateSummaryLSA(lsa);
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
    */
}


bool AnsaOspf6::Area::HasAnyNeighborInStates(int states) const {
   long interfaceCount = associatedInterfaces.size();
   for (long i = 0; i < interfaceCount; i++){
      if (associatedInterfaces[i]->HasAnyNeighborInStates(states)){
         return true;
      }
   }
   return false;
}

void AnsaOspf6::Area::RemoveFromAllRetransmissionLists(AnsaOspf6::LsaKeyType6 lsaKey) {
   long interfaceCount = associatedInterfaces.size();
   for (long i = 0; i < interfaceCount; i++){
      associatedInterfaces[i]->RemoveFromAllRetransmissionLists(lsaKey);
   }
}

bool AnsaOspf6::Area::IsOnAnyRetransmissionList(AnsaOspf6::LsaKeyType6 lsaKey) const {
   long interfaceCount = associatedInterfaces.size();
   for (long i = 0; i < interfaceCount; i++){
      if (associatedInterfaces[i]->IsOnAnyRetransmissionList(lsaKey)){
         return true;
      }
   }
   return false;
}

bool AnsaOspf6::Area::FloodLSA(OspfLsa6* lsa, AnsaOspf6::Interface* intf, AnsaOspf6::Neighbor* neighbor){
   bool floodedBackOut  = false;
   long interfaceCount = associatedInterfaces.size();

   for (long i = 0; i < interfaceCount; i++) {
      if (associatedInterfaces[i]->FloodLSA(lsa, intf, neighbor)) {
         floodedBackOut = true;
      }
   }

   return floodedBackOut;
}


AnsaOspf6::RouterLsa* AnsaOspf6::Area::OriginateRouterLSA(void){

}

AnsaOspf6::NetworkLsa* AnsaOspf6::Area::OriginateNetworkLSA(const AnsaOspf6::Interface* intf){

}
