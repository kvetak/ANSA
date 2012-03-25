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

#ifndef ANSAOSPFAREA6_H_
#define ANSAOSPFAREA6_H_

#include "ansaLsa6.h"
#include "ansaOspfCommon6.h"
#include "ansaOspfInterface6.h"
#include "ansaOspfRoutingTableEntry6.h"

namespace AnsaOspf6 {

class Router;

class Area : public cPolymorphic{
private:
   AreaID                                                      areaID;
   std::map<IPv6AddressPrefix, bool, IPv6AddressPrefix_Less>   advertiseAddressPrefixes;
   std::vector<IPv6AddressPrefix>                              areaAddressPrefixes;
   std::vector<Interface*>                                     associatedInterfaces;
   std::vector<HostRouteParameters>                            hostRoutes;
   std::map<LinkStateID, RouterLsa*>                           routerLSAsByID;
   std::vector<RouterLsa*>                                     routerLSAs;
   std::map<LinkStateID, NetworkLsa*>                          networkLSAsByID;
   std::vector<NetworkLsa*>                                    networkLSAs;
   std::map<LinkStateID, InterAreaPrefixLsa*>                  interAreaPrefixLSAsByID;
   std::vector<InterAreaPrefixLsa*>                            interAreaPrefixLSAs;
   std::map<LinkStateID, InterAreaRouterLsa*>                  interAreaRouterLSAsByID;
   std::vector<InterAreaRouterLsa*>                            interAreaRouterLSAs;
   std::map<LinkStateID, IntraAreaPrefixLsa*>                  intraAreaPrefixLSAsByID;
   std::vector<IntraAreaPrefixLsa*>                            intraAreaPrefixLSAs;
   bool                                                        transitCapability;
   bool                                                        externalRoutingCapability;
   Metric                                                      stubDefaultCost;
   RouterLsa*                                                  spfTreeRoot;
                                                               // FIXME is it still RouterLSA, or
                                                               // some new LSA with IPv6 prefixes?

   Router*                                                     parentRouter;

public:
            Area(AreaID id = BackboneAreaID);
   virtual ~Area(void);

   void                SetAreaID                       (AreaID id)                                     { areaID = id; }
   AreaID              GetAreaID                       (void) const                                    { return areaID; }
   void                AddAddressPrefix                (IPv6AddressPrefix prefix, bool advertise)      { areaAddressPrefixes.push_back(prefix); advertiseAddressPrefixes[prefix] = advertise; }
   unsigned int        GetAddressPrefixCount           (void) const                                    { return areaAddressPrefixes.size(); }
   IPv6AddressPrefix   GetAddressPrefix                (unsigned int index) const                      { return areaAddressPrefixes[index]; }
   void                AddHostRoute                    (HostRouteParameters& hostRouteParameters)      { hostRoutes.push_back(hostRouteParameters); }
   void                SetTransitCapability            (bool transit)                                  { transitCapability = transit; }
   bool                GetTransitCapability            (void) const                                    { return transitCapability; }
   void                SetExternalRoutingCapability    (bool flooded)                                  { externalRoutingCapability = flooded; }
   bool                GetExternalRoutingCapability    (void) const                                    { return externalRoutingCapability; }
   void                SetSPFTreeRoot                  (RouterLsa* root)                               { spfTreeRoot = root; }
   RouterLsa*          GetSPFTreeRoot                  (void)                                          { return spfTreeRoot; }
   const RouterLsa*    GetSPFTreeRoot                  (void) const                                    { return spfTreeRoot; }

   void                SetRouter                       (Router* router)                                { parentRouter = router; }
   Router*             GetRouter                       (void)                                          { return parentRouter; }
   const Router*       GetRouter                       (void) const                                    { return parentRouter; }

   unsigned long              GetRouterLSACount             (void) const               { return routerLSAs.size(); }
   RouterLsa*                 GetRouterLSA                  (unsigned long i)          { return routerLSAs[i]; }
   const RouterLsa*           GetRouterLSA                  (unsigned long i) const    { return routerLSAs[i]; }
   unsigned long              GetNetworkLSACount            (void) const               { return networkLSAs.size(); }
   NetworkLsa*                GetNetworkLSA                 (unsigned long i)          { return networkLSAs[i]; }
   const NetworkLsa*          GetNetworkLSA                 (unsigned long i) const    { return networkLSAs[i]; }
   unsigned long              GetInterAreaPrefixLSACount    (void) const               { return interAreaPrefixLSAs.size(); }
   InterAreaPrefixLsa*        GetInterAreaPrefixLSA         (unsigned long i)          { return interAreaPrefixLSAs[i]; }
   const InterAreaPrefixLsa*  GetInterAreaPrefixLSA         (unsigned long i) const    { return interAreaPrefixLSAs[i]; }
   unsigned long              GetInterAreaRouterLSACount    (void) const               { return interAreaRouterLSAs.size(); }
   InterAreaRouterLsa*        GetInterAreaRouterLSA         (unsigned long i)          { return interAreaRouterLSAs[i]; }
   const InterAreaRouterLsa*  GetInterAreaRouterLSA         (unsigned long i) const    { return interAreaRouterLSAs[i]; }
   unsigned long              GetIntraAreaPrefixLSACount    (void) const               { return intraAreaPrefixLSAs.size(); }
   IntraAreaPrefixLsa*        GetIntraAreaPrefixLSA         (unsigned long i)          { return intraAreaPrefixLSAs[i]; }
   const IntraAreaPrefixLsa*  GetIntraAreaPrefixLSA         (unsigned long i) const    { return intraAreaPrefixLSAs[i]; }

   bool                ContainsAddress             (IPv6Address address) const;
   bool                HasAddressPrefix            (IPv6AddressPrefix prefix) const;
   IPv6AddressPrefix   GetContainingAddressRange   (IPv6AddressPrefix prefix, bool* advertise = NULL) const;
   void                AddInterface                (Interface* intf);
   unsigned int        GetNumberOfInterfaces       (void)                              { return associatedInterfaces.size(); }
   Interface*          GetInterfaceByIndex         (unsigned int index);
   Interface*          GetInterface                (unsigned char ifIndex);
   Interface*          GetInterface                (IPv6Address address);
   bool                HasVirtualLink              (AreaID withTransitArea) const;
   Interface*          FindVirtualLink             (RouterID routerID);


   bool                       InstallRouterLSA                (OspfRouterLsa6* lsa);
   bool                       InstallNetworkLSA               (OspfNetworkLsa6* lsa);
   bool                       InstallInterAreaPrefixLSA       (OspfInterAreaPrefixLsa6* lsa);
   bool                       InstallInterAreaRouterLSA       (OspfInterAreaRouterLsa6* lsa);
   bool                       InstallIntraAreaPrefixLSA       (OspfIntraAreaPrefixLsa6* lsa);
   RouterLsa*                 FindRouterLSA                   (LinkStateID linkStateID);
   const RouterLsa*           FindRouterLSA                   (LinkStateID linkStateID) const;
   NetworkLsa*                FindNetworkLSA                  (LinkStateID linkStateID);
   const NetworkLsa*          FindNetworkLSA                  (LinkStateID linkStateID) const;
   InterAreaPrefixLsa*        FindInterAreaPrefixLSA          (LinkStateID linkStateID);
   const InterAreaPrefixLsa*  FindInterAreaPrefixLSA          (LinkStateID linkStateID) const;
   InterAreaRouterLsa*        FindInterAreaRouterLSA          (LinkStateID linkStateID);
   const InterAreaRouterLsa*  FindInterAreaRouterLSA          (LinkStateID linkStateID) const;
   IntraAreaPrefixLsa*        FindIntraAreaPrefixLSA          (LinkStateID linkStateID);
   const IntraAreaPrefixLsa*  FindIntraAreaPrefixLSA          (LinkStateID linkStateID) const;
   void                       AgeDatabase                     (void);
   void                       RemoveParentFromRoutingInfo     (OspfLsa6* parent);
   bool                       HasAnyNeighborInStates          (int states) const;
   void                       RemoveFromAllRetransmissionLists(LsaKeyType6 lsaKey);
   bool                       IsOnAnyRetransmissionList       (LsaKeyType6 lsaKey) const;
   bool                       FloodLSA                        (OspfLsa6* lsa, Interface* intf = NULL, Neighbor* neighbor = NULL);
   bool                       IsLocalAddress                  (IPv6Address address) const;
   RouterLsa*                 OriginateRouterLSA              (void);
   NetworkLsa*                OriginateNetworkLSA             (const Interface* intf);
   InterAreaPrefixLsa*        OriginateInterAreaPrefixLSA     (void); // FIXME
   InterAreaRouterLsa*        OriginateInterAreaRouterLSA     (void); // FIXME
   IntraAreaPrefixLsa*        OriginateIntraAreaPrefixLSA     (void); // FIXME
   void                       CalculateShortestPathTree       (std::vector<RoutingTableEntry*>& newRoutingTable);
   void                       CalculateInterAreaRoutes        (std::vector<RoutingTableEntry*>& newRoutingTable);

   // FIXME
   // void                       ReCheckSummaryLSAs                  (std::vector<RoutingTableEntry*>& newRoutingTable);

   void        info(char* buffer);
   std::string detailedInfo(void) const;

private:

};

}

inline std::ostream& operator<<(std::ostream& ostr, AnsaOspf6::Area& area) {
   ostr << area.detailedInfo();
   return ostr;
}

#endif /* ANSAOSPFAREA6_H_ */
