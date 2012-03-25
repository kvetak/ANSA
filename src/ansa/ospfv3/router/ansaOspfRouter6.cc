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

#include "ansaOspfRouter6.h"

/**
 * Constructor.
 * Initializes internal variables, adds a MessageHandler and starts the Database Age timer.
 */
AnsaOspf6::Router::Router(ProcessID pid, RouterID id, cSimpleModule* containingModule) :
      routerID(id),
      processID(pid){

   messageHandler = new AnsaOspf6::MessageHandler(this, containingModule);
   /*
   ageTimer = new OspfTimer6;
   ageTimer->setTimerKind(DatabaseAgeTimer);
   ageTimer->setContextPointer(this);
   ageTimer->setName("AnsaOspf6::Router::DatabaseAgeTimer");
   messageHandler->StartTimer(ageTimer, 1.0);
   */
}

/**
 * Destructor.
 * Clears all LSA lists and kills the Database Age timer.
 */
AnsaOspf6::Router::~Router(void) {
   long areaCount = areas.size();
   for (long i = 0; i < areaCount; i++){
      delete areas[i];
   }
   long lsaCount = asExternalLSAs.size();
   for (long j = 0; j < lsaCount; j++){
      delete asExternalLSAs[j];
   }
   long routeCount = routingTable.size();
   for (long k = 0; k < routeCount; k++){
      delete routingTable[k];
   }

   /* TODO
   messageHandler->ClearTimer(ageTimer);
   delete ageTimer;
   */
   delete messageHandler;
}


/**
 * Adds a new Area to the Area list.
 * @param area [in] The Area to add.
 */
void AnsaOspf6::Router::AddArea(AnsaOspf6::Area* area) {
   area->SetRouter(this);
   areasByID[area->GetAreaID()] = area;
   areas.push_back(area);
}


/**
 * Returns the pointer to the Area identified by the input areaID, if it's on the Area list,
 * NULL otherwise.
 * @param areaID [in] The Area identifier.
 */
AnsaOspf6::Area* AnsaOspf6::Router::GetArea(AnsaOspf6::AreaID areaID) {
   std::map<AnsaOspf6::AreaID, AnsaOspf6::Area*>::iterator areaIt = areasByID.find(areaID);
   if (areaIt != areasByID.end()){
      return (areaIt->second);
   }else{
      return NULL;
   }
}

AnsaOspf6::Area* AnsaOspf6::Router::GetAreaByIndex(int index) {
   if (index < areas.size()){
      return areas[index];
   }else{
      return NULL;
   }
}



/**
 * Returns the pointer of the physical Interface identified by the input interface index,
 * NULL if the Router doesn't have such an interface.
 * @param ifIndex [in] The interface index to look for.
 */
AnsaOspf6::Interface* AnsaOspf6::Router::GetNonVirtualInterface(unsigned char ifIndex) {
   long areaCount = areas.size();
   for (long i = 0; i < areaCount; i++){
      AnsaOspf6::Interface* intf = areas[i]->GetInterface(ifIndex);
      if (intf != NULL){
         return intf;
      }
   }
   return NULL;
}


/**
 * Installs a new LSA into the Router database.
 * Checks the input LSA's type and installs it into either the selected Area's database,
 * or if it's an AS External LSA then into the Router's common asExternalLSAs list.
 * @param lsa    [in] The LSA to install. It will be copied into the database.
 * @param areaID [in] Identifies the input Router, Network and Summary LSA's Area.
 * @return True if the routing table needs to be updated, false otherwise.
 */
bool AnsaOspf6::Router::InstallLSA(OspfLsa6* lsa, AnsaOspf6::AreaID areaID) {
   switch (lsa->getHeader().getLsType()){
      case RouterLsaType: {
         std::map<AnsaOspf6::AreaID, AnsaOspf6::Area*>::iterator areaIt = areasByID.find(areaID);
         if (areaIt != areasByID.end()){
            OspfRouterLsa6* ospfRouterLSA = check_and_cast<OspfRouterLsa6*> (lsa);
            return areaIt->second->InstallRouterLSA(ospfRouterLSA);
         }
      }
         break;
      case NetworkLsaType: {
         std::map<AnsaOspf6::AreaID, AnsaOspf6::Area*>::iterator areaIt = areasByID.find(areaID);
         if (areaIt != areasByID.end()){
            OspfNetworkLsa6* ospfNetworkLSA = check_and_cast<OspfNetworkLsa6*> (lsa);
            return areaIt->second->InstallNetworkLSA(ospfNetworkLSA);
         }
      }

      case InterAreaPrefixLsaType: {
         std::map<AnsaOspf6::AreaID, AnsaOspf6::Area*>::iterator areaIt = areasByID.find(areaID);
         if (areaIt != areasByID.end()){
            OspfInterAreaPrefixLsa6* ospfInterAreaPrefixLSA = check_and_cast<OspfInterAreaPrefixLsa6*> (lsa);
            return areaIt->second->InstallInterAreaPrefixLSA(ospfInterAreaPrefixLSA);
         }
      }

      case InterAreaRouterLsaType: {
         std::map<AnsaOspf6::AreaID, AnsaOspf6::Area*>::iterator areaIt = areasByID.find(areaID);
         if (areaIt != areasByID.end()){
            OspfInterAreaRouterLsa6* ospfInterAreaRouterLSA = check_and_cast<OspfInterAreaRouterLsa6*> (lsa);
            return areaIt->second->InstallInterAreaRouterLSA(ospfInterAreaRouterLSA);
         }
      }

      case AsExternalLsaType: {
         OspfAsExternalLsa6* ospfASExternalLSA = check_and_cast<OspfAsExternalLsa6*> (lsa);
         return InstallASExternalLSA(ospfASExternalLSA);
      }

      case LinkLsaType: {
         // TODO
      }

      case IntraAreaPrefixLsaType: {
         std::map<AnsaOspf6::AreaID, AnsaOspf6::Area*>::iterator areaIt = areasByID.find(areaID);
         if (areaIt != areasByID.end()){
            OspfIntraAreaPrefixLsa6* ospfIntraAreaPrefixLSA = check_and_cast<OspfIntraAreaPrefixLsa6*> (lsa);
            return areaIt->second->InstallIntraAreaPrefixLSA(ospfIntraAreaPrefixLSA);
         }
      }
   }
   return false;
}



/**
 * Rebuilds the routing table from scratch(based on the LSA database).
 * @sa RFC2328 Section 16.
 */
void AnsaOspf6::Router::RebuildRoutingTable(void){

   /*
    unsigned long                         areaCount       = areas.size();
    bool                                  hasTransitAreas = false;
    std::vector<AnsaOspf6::RoutingTableEntry*> newTable;
    unsigned long                         i;

    EV << "Rebuilding routing table:\n";

    for (i = 0; i < areaCount; i++) {
        areas[i]->CalculateShortestPathTree(newTable);
        if (areas[i]->GetTransitCapability()) {
            hasTransitAreas = true;
        }
    }
    if (areaCount > 1) {
        AnsaOspf6::Area* backbone = GetArea(AnsaOspf6::BackboneAreaID);
        if (backbone != NULL) {
            backbone->CalculateInterAreaRoutes(newTable);
        }
    } else {
        if (areaCount == 1) {
            areas[0]->CalculateInterAreaRoutes(newTable);
        }
    }
    if (hasTransitAreas) {
        for (i = 0; i < areaCount; i++) {
            if (areas[i]->GetTransitCapability()) {
                areas[i]->ReCheckSummaryLSAs(newTable);
            }
        }
    }
    CalculateASExternalRoutes(newTable);

    // backup the routing table
    unsigned long                         routeCount = routingTable.size();
    std::vector<AnsaOspf6::RoutingTableEntry*> oldTable;

    oldTable.assign(routingTable.begin(), routingTable.end());
    routingTable.clear();
    routingTable.assign(newTable.begin(), newTable.end());

    RoutingTableAccess         routingTableAccess;
    std::vector<const IPRoute*> eraseEntries;
    IRoutingTable*              simRoutingTable    = routingTableAccess.get();
    unsigned long              routingEntryNumber = simRoutingTable->getNumRoutes();
    // remove entries from the IP routing table inserted by the OSPF module
    for (i = 0; i < routingEntryNumber; i++) {
        const IPRoute *entry = simRoutingTable->getRoute(i);
        const AnsaOspf6::RoutingTableEntry* ospfEntry = dynamic_cast<const AnsaOspf6::RoutingTableEntry*>(entry);
        if (ospfEntry != NULL) {
            eraseEntries.push_back(entry);
        }
    }

    unsigned int eraseCount = eraseEntries.size();
    for (i = 0; i < eraseCount; i++) {
        simRoutingTable->deleteRoute(eraseEntries[i]);
    }

    // add the new routing entries
    routeCount = routingTable.size();
    for (i = 0; i < routeCount; i++) {
        if (routingTable[i]->GetDestinationType() == AnsaOspf6::RoutingTableEntry::NetworkDestination) {
            simRoutingTable->addRoute(new AnsaOspf6::RoutingTableEntry(*(routingTable[i])));
        }
    }

    NotifyAboutRoutingTableChanges(oldTable);

    routeCount = oldTable.size();
    for (i = 0; i < routeCount; i++) {
        delete(oldTable[i]);
    }

    EV << "Routing table was rebuilt.\n"
       << "Results:\n";

    routeCount = routingTable.size();
    for (i = 0; i < routeCount; i++) {
        EV << *routingTable[i]
           << "\n";
    }
    */
}

OspfLsa6* AnsaOspf6::Router::FindLSA(LsaType6 lsaType, AnsaOspf6::LsaKeyType6 lsaKey, AnsaOspf6::AreaID areaID) {
   /*
   switch (lsaType){
      case RouterLSAType: {
         std::map<AnsaOspf6::AreaID, AnsaOspf6::Area*>::iterator areaIt = areasByID.find(areaID);
         if (areaIt != areasByID.end()){
            return areaIt->second->FindRouterLSA(lsaKey.linkStateID);
         }
      }
         break;
      case NetworkLSAType: {
         std::map<AnsaOspf6::AreaID, AnsaOspf6::Area*>::iterator areaIt = areasByID.find(areaID);
         if (areaIt != areasByID.end()){
            return areaIt->second->FindNetworkLSA(lsaKey.linkStateID);
         }
      }
         break;
      case SummaryLSA_NetworksType:
      case SummaryLSA_ASBoundaryRoutersType: {
         std::map<AnsaOspf6::AreaID, AnsaOspf6::Area*>::iterator areaIt = areasByID.find(areaID);
         if (areaIt != areasByID.end()){
            return areaIt->second->FindSummaryLSA(lsaKey);
         }
      }
         break;
      case ASExternalLSAType: {
         return FindASExternalLSA(lsaKey);
      }
         break;
      default:
         ASSERT(false);
         break;
   }
   return NULL;
   */
}

/**
 * Ages the LSAs in the Router's database.
 * This method is called on every firing of the DatabaseAgeTimer(every second).
 * @sa RFC2328 Section 14.
 */
void AnsaOspf6::Router::AgeDatabase(void){
}


/**
 * Returns true if any Neighbor on any Interface in any of the Router's Areas is
 * in any of the input states, false otherwise.
 * @param states [in] A bitfield combination of NeighborStateType values.
 */
bool AnsaOspf6::Router::HasAnyNeighborInStates(int states) const {
   long areaCount = areas.size();
   for (long i = 0; i < areaCount; i++){
      if (areas[i]->HasAnyNeighborInStates(states)){
         return true;
      }
   }
   return false;
}

/**
 * Removes all LSAs from all Neighbor's retransmission lists which are identified by
 * the input lsaKey.
 * @param lsaKey [in] Identifies the LSAs to remove from the retransmission lists.
 */
void AnsaOspf6::Router::RemoveFromAllRetransmissionLists(AnsaOspf6::LsaKeyType6 lsaKey) {
   long areaCount = areas.size();
   for (long i = 0; i < areaCount; i++){
      areas[i]->RemoveFromAllRetransmissionLists(lsaKey);
   }
}

/**
 * Returns true if there's at least one LSA on any Neighbor's retransmission list
 * identified by the input lsaKey, false otherwise.
 * @param lsaKey [in] Identifies the LSAs to look for on the retransmission lists.
 */
bool AnsaOspf6::Router::IsOnAnyRetransmissionList(AnsaOspf6::LsaKeyType6 lsaKey) const {
   long areaCount = areas.size();
   for (long i = 0; i < areaCount; i++){
      if (areas[i]->IsOnAnyRetransmissionList(lsaKey)){
         return true;
      }
   }
   return false;
}

/**
 * Floods out the input lsa on a set of Interfaces.
 * @sa RFC2328 Section 13.3.
 * @param lsa      [in] The LSA to be flooded out.
 * @param areaID   [in] If the lsa is a Router, Network or Summary LSA, then flood it only in this Area.
 * @param intf     [in] The Interface this LSA arrived on.
 * @param neighbor [in] The Nieghbor this LSA arrived from.
 * @return True if the LSA was floooded back out on the receiving Interface, false otherwise.
 */
bool AnsaOspf6::Router::FloodLSA(OspfLsa6* lsa, AnsaOspf6::AreaID areaID, AnsaOspf6::Interface* intf, AnsaOspf6::Neighbor* neighbor){
}


bool AnsaOspf6::Router::IsDestinationUnreachable(OspfLsa6* lsa) const {

}


bool AnsaOspf6::Router::InstallASExternalLSA(OspfAsExternalLsa6* lsa){
}


/**
 * Find the AS External LSA identified by the input lsaKey in the database.
 * @param lsaKey [in] Look for the AS External LSA which is identified by this key.
 * @return The pointer to the AS External LSA if it was found, NULL otherwise.
 */
AnsaOspf6::AsExternalLsa* AnsaOspf6::Router::FindASExternalLSA(AnsaOspf6::LsaKeyType6 lsaKey) {
   std::map<AnsaOspf6::LsaKeyType6, AnsaOspf6::AsExternalLsa*, AnsaOspf6::LsaKeyType6_Less>
      ::iterator lsaIt = asExternalLSAsByID.find(lsaKey);
   if (lsaIt != asExternalLSAsByID.end()){
      return lsaIt->second;
   }else{
      return NULL;
   }
}


/**
 * Find the AS External LSA identified by the input lsaKey in the database.
 * @param lsaKey [in] Look for the AS External LSA which is identified by this key.
 * @return The const pointer to the AS External LSA if it was found, NULL otherwise.
 */
const AnsaOspf6::AsExternalLsa* AnsaOspf6::Router::FindASExternalLSA(AnsaOspf6::LsaKeyType6 lsaKey) const {
   std::map<AnsaOspf6::LsaKeyType6, AnsaOspf6::AsExternalLsa*, AnsaOspf6::LsaKeyType6_Less>
      ::const_iterator lsaIt = asExternalLSAsByID.find(lsaKey);
   if (lsaIt != asExternalLSAsByID.end()){
      return lsaIt->second;
   }else{
      return NULL;
   }
}

