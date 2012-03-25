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

#ifndef ANSAOSPFROUTER6_H_
#define ANSAOSPFROUTER6_H_

#include "IPAddress.h"

#include "ansaMessageHandler6.h"
#include "ansaOspfArea6.h"
#include "ansaOspfCommon6.h"
#include "ansaOspfStat6.h"

namespace AnsaOspf6 {

class Router {
private:
   RouterID                                                    routerID;
   ProcessID                                                   processID;
   std::map<AreaID, Area*>                                     areasByID;
   std::vector<Area*>                                          areas;
   std::map<LsaKeyType6, AsExternalLsa*, LsaKeyType6_Less>     asExternalLSAsByID;
   std::vector<AsExternalLsa*>                                 asExternalLSAs;
   std::map<IPv6Address, OspfAsExternalLsa6, IPv6Address_Less> externalRoutes;
   OspfTimer6*                                                 ageTimer;
   std::vector<RoutingTableEntry*>                             routingTable;
   MessageHandler*                                             messageHandler;
public:
   Stat                                                        stat;

public:
            Router(ProcessID pid, RouterID id, cSimpleModule* containingModule);
   virtual ~Router(void);

   void              SetRouterID             (RouterID id)     { routerID = id; }
   RouterID          GetRouterID             (void) const      { return routerID; }
   void              SetProcessID            (ProcessID pid)   { processID = pid; }
   ProcessID         GetProcessID            (void) const      { return processID; }
   unsigned long     GetAreaCount            (void) const      { return areas.size(); }

   MessageHandler*   GetMessageHandler       (void)            { return messageHandler; }

   unsigned long              GetASExternalLSACount      (void) const               { return asExternalLSAs.size(); }
   AsExternalLsa*             GetASExternalLSA           (unsigned long i)          { return asExternalLSAs[i]; }
   const AsExternalLsa*       GetASExternalLSA           (unsigned long i) const    { return asExternalLSAs[i]; }
   bool                       GetASBoundaryRouter        (void) const               { return (externalRoutes.size() > 0); }
   unsigned long              GetRoutingTableEntryCount  (void) const               { return routingTable.size(); }
   RoutingTableEntry*         GetRoutingTableEntry       (unsigned long i)          { return routingTable[i]; }
   const RoutingTableEntry*   GetRoutingTableEntry       (unsigned long i) const    { return routingTable[i]; }
   void                       AddRoutingTableEntry       (RoutingTableEntry* entry) { routingTable.push_back(entry); }

   void              AddArea                 (Area* area);
   Area*             GetArea                 (AreaID areaID);
   Area*             GetAreaByIndex          (int index);
   Area*             GetArea                 (IPv6Address address);
   Interface*        GetNonVirtualInterface  (unsigned char ifIndex);

   bool              InstallLSA                       (OspfLsa6* lsa, AreaID areaID = BackboneAreaID);
   OspfLsa6*         FindLSA                          (LsaType6 lsaType, LsaKeyType6 lsaKey, AreaID areaID);
   void              AgeDatabase                      (void);
   bool              HasAnyNeighborInStates           (int states) const;
   void              RemoveFromAllRetransmissionLists (LsaKeyType6 lsaKey);
   bool              IsOnAnyRetransmissionList        (LsaKeyType6 lsaKey) const;
   bool              FloodLSA                         (OspfLsa6* lsa, AreaID areaID = BackboneAreaID, Interface* intf = NULL, Neighbor* neighbor = NULL);

   bool              IsDestinationUnreachable         (OspfLsa6* lsa) const;

   void              RebuildRoutingTable     (void);

private:
   bool                 InstallASExternalLSA (OspfAsExternalLsa6* lsa);
   AsExternalLsa*       FindASExternalLSA    (LsaKeyType6 lsaKey);
   const AsExternalLsa* FindASExternalLSA    (LsaKeyType6 lsaKey) const;
};

}


inline std::ostream& operator<<(std::ostream& os, AnsaOspf6::Router& r){
   // TODO: sem pøijde zhruba obsah metody add watches
   // ale taky bych mohl prozkoumat, jak funguje to rozklikávání do dalších
   // oken jako je tøeba v modulu interface table

   os << "Router-ID: " << IPAddress(r.GetRouterID());

   int areaCount = r.GetAreaCount();
   for (int i = 0; i < areaCount; i++){
      os << endl << "Interfaces in area " << r.GetAreaByIndex(i)->GetAreaID() << ":";
      int ifaces = r.GetAreaByIndex(i)->GetNumberOfInterfaces();
      for (int j = 0; j < ifaces; j++){
         AnsaOspf6::Interface *iface = r.GetAreaByIndex(i)->GetInterfaceByIndex(j);
         os << endl << "- " << iface->GetIfName();
         os << " (" << iface->GetTypeString(iface->GetType()) << ")";
         //os << ", inst: " << iface->GetInstanceID();
         os << ", cost: " << iface->GetOutputCost();
         os << ", pri: " << iface->GetRouterPriority();
         os << ", hello: " << iface->GetHelloInterval() << "s";
         os << ", dead:" << iface->GetRouterDeadInterval() << "s";

         os << endl << "  * state: " << iface->GetStateString(iface->GetState());
         os << ", DR: " << IPAddress(iface->GetDesignatedRouter());
         os << ", BDR: " << IPAddress(iface->GetBackupDesignatedRouter());

         os << endl << "  * " << "prefixes: ";
         int prefixes = iface->GetAddressPrefixCount();
         for (int k = 0; k < prefixes; k++){
            if (k != 0){
               os << ", ";
            }
            os << iface->GetAddressPrefix(k).address << "/" << iface->GetAddressPrefix(k).prefixLen;
         }

         for (int l = 0; l < iface->GetNeighborCount(); l++){
            os << endl << "  * neighbor ";
            os << IPAddress(iface->GetNeighbor(l)->GetNeighborID());
            os << " (" << iface->GetNeighbor(l)->GetStateString(iface->GetNeighbor(l)->GetState()) << ")";
            os << ", pri: " << iface->GetNeighbor(l)->GetPriority();
            if (iface->GetNeighbor(l)->GetDesignatedRouter() != 0){
               os << endl << "    DR: " << IPAddress(iface->GetNeighbor(l)->GetDesignatedRouter());
            }

            if (iface->GetNeighbor(l)->GetBackupDesignatedRouter() != 0){
               os << ", BDR:" << IPAddress(iface->GetNeighbor(l)->GetBackupDesignatedRouter());
            }
         }
      }
   }


   return os;
}



#endif /* ANSAOSPFROUTER6_H_ */
