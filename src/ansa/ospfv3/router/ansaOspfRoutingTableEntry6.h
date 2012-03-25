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

#ifndef ANSAOSPFROUTINGTABLEENTRY6_H_
#define ANSAOSPFROUTINGTABLEENTRY6_H_

#include "RoutingTable6.h"
#include "AnsaInterfaceTableAccess.h"
#include "ansaOspfCommon6.h"

namespace AnsaOspf6 {

class RoutingTableEntry : public IPv6Route {
public:
    enum RoutingPathType {
        IntraArea     = 0,
        InterArea     = 1,
        Type1External = 2,
        Type2External = 3
    };

    typedef unsigned char RoutingDestinationType;

    // destinationType bitfield values
    static const unsigned char NetworkDestination = 0;
    static const unsigned char AreaBorderRouterDestination = 1;
    static const unsigned char ASBoundaryRouterDestination = 2;

private:
    RoutingDestinationType  destinationType;
    OspfOptions6            optionalCapabilities;
    AreaID                  area;
    RoutingPathType         pathType;
    Metric                  cost;
    Metric                  type2Cost;
    const OspfLsa6*         linkStateOrigin;
    std::vector<NextHop>    nextHops;

public:
             RoutingTableEntry   (void);
             RoutingTableEntry   (const RoutingTableEntry& entry);
    virtual ~RoutingTableEntry   (void) {}

    bool    operator== (const RoutingTableEntry& entry) const;
    bool    operator!= (const RoutingTableEntry& entry) const { return (!((*this) == entry)); }

    void                    SetDestinationType      (RoutingDestinationType type)   { destinationType = type; }
    RoutingDestinationType  GetDestinationType      (void) const                    { return destinationType; }
    void                    SetDestinationPrefix    (IPv6Address destID)            { _destPrefix = destID; }
    IPv6Address             GetDestinationPrefix    (void) const                    { return _destPrefix; }
    void                    SetPrefixLength         (short prefixLen)               { _length = prefixLen; }
    short                   GetPrefixLength         (void) const                    { return _length; }
    void                    SetOptionalCapabilities (OspfOptions6 options)          { optionalCapabilities = options; }
    OspfOptions6            GetOptionalCapabilities (void) const                    { return optionalCapabilities; }
    void                    SetArea                 (AreaID source)                 { area = source; }
    AreaID                  GetArea                 (void) const                    { return area; }
    void                    SetPathType             (RoutingPathType type);
    RoutingPathType         GetPathType             (void) const                    { return pathType; }
    void                    SetCost                 (Metric pathCost);
    Metric                  GetCost                 (void) const                    { return cost; }
    void                    SetType2Cost            (Metric pathCost);
    Metric                  GetType2Cost            (void) const                    { return type2Cost; }
    void                    SetLinkStateOrigin      (const OspfLsa6* lsa)           { linkStateOrigin = lsa; }
    const OspfLsa6*         GetLinkStateOrigin      (void) const                    { return linkStateOrigin; }
    void                    AddNextHop              (NextHop hop);
    void                    ClearNextHops           (void)                          { nextHops.clear(); }
    unsigned int            GetNextHopCount         (void) const                    { return nextHops.size(); }
    NextHop                 GetNextHop              (unsigned int index) const      { return nextHops[index]; }
};

}

inline AnsaOspf6::RoutingTableEntry::RoutingTableEntry(void) :
      IPv6Route(IPv6Address::UNSPECIFIED_ADDRESS, 0, IPv6Route::ROUTING_PROT),
      destinationType(AnsaOspf6::RoutingTableEntry::NetworkDestination),
      area(AnsaOspf6::BackboneAreaID),
      pathType(AnsaOspf6::RoutingTableEntry::IntraArea),
      type2Cost(0),
      linkStateOrigin(NULL) {

   memset(&optionalCapabilities, 0, sizeof(OspfOptions6));
}

inline AnsaOspf6::RoutingTableEntry::RoutingTableEntry(const RoutingTableEntry& entry) :
      IPv6Route(entry._destPrefix, entry._length, entry._src),
      destinationType(entry.destinationType),
      optionalCapabilities(entry.optionalCapabilities),
      area(entry.area),
      pathType(entry.pathType),
      cost(entry.cost),
      type2Cost(entry.type2Cost),
      linkStateOrigin(entry.linkStateOrigin),
      nextHops(entry.nextHops) {

   _nextHop = entry._nextHop;
   _interfaceID = entry._interfaceID;
   _metric = entry._metric;
}

inline void AnsaOspf6::RoutingTableEntry::SetPathType(RoutingPathType type) {
   pathType = type;
   // FIXME: this is a hack. But the correct way to do it is to implement a separate IRoutingTable module for OSPF...
   if (pathType == AnsaOspf6::RoutingTableEntry::Type2External){
      _metric = cost + type2Cost * 1000;
   }else{
      _metric = cost;
   }
}

inline void AnsaOspf6::RoutingTableEntry::SetCost(Metric pathCost) {
   cost = pathCost;
   // FIXME: this is a hack. But the correct way to do it is to implement a separate IRoutingTable module for OSPF...
   if (pathType == AnsaOspf6::RoutingTableEntry::Type2External){
      _metric = cost + type2Cost * 1000;
   }else{
      _metric = cost;
   }
}

inline void AnsaOspf6::RoutingTableEntry::SetType2Cost(Metric pathCost) {
   type2Cost = pathCost;
   // FIXME: this is a hack. But the correct way to do it is to implement a separate IRoutingTable module for OSPF...
   if (pathType == AnsaOspf6::RoutingTableEntry::Type2External){
      _metric = cost + type2Cost * 1000;
   }else{
      _metric = cost;
   }
}

inline void AnsaOspf6::RoutingTableEntry::AddNextHop(AnsaOspf6::NextHop hop) {
   if (nextHops.size() == 0){
      InterfaceEntry* routingInterface = AnsaInterfaceTableAccess().get()->getInterfaceById(hop.ifIndex);
      _interfaceID = routingInterface->getInterfaceId();
      _nextHop = hop.hopAddress;
   }
   nextHops.push_back(hop);
}

inline bool AnsaOspf6::RoutingTableEntry::operator==(const RoutingTableEntry& entry) const {
   unsigned int hopCount = nextHops.size();
   unsigned int i = 0;

   if (hopCount != entry.nextHops.size()){
      return false;
   }
   for (i = 0; i < hopCount; i++){
      if ((nextHops[i] != entry.nextHops[i])){
         return false;
      }
   }

   return ( (destinationType == entry.destinationType)
         && (_destPrefix == entry._destPrefix)
         && (_length == entry._length)
         && (optionalCapabilities == entry.optionalCapabilities)
         && (area == entry.area)
         && (pathType == entry.pathType)
         && (cost == entry.cost)
         && (type2Cost == entry.type2Cost)
         && (linkStateOrigin == entry.linkStateOrigin));
}

inline std::ostream& operator<<(std::ostream& out, const AnsaOspf6::RoutingTableEntry& entry) {
   out << "Destination: "
         << entry.GetDestinationPrefix().str()
         << "/"
         << entry.GetPrefixLength()
         << " (";

   if (entry.GetDestinationType() == AnsaOspf6::RoutingTableEntry::NetworkDestination) {
      out << "Network";
   } else {
      if ((entry.GetDestinationType()
            & AnsaOspf6::RoutingTableEntry::AreaBorderRouterDestination) != 0) {
         out << "AreaBorderRouter";
      }
      if ((entry.GetDestinationType()
            & ( AnsaOspf6::RoutingTableEntry::ASBoundaryRouterDestination
              | AnsaOspf6::RoutingTableEntry::AreaBorderRouterDestination)) != 0) {
         out << "+";
      }
      if ((entry.GetDestinationType()
            & AnsaOspf6::RoutingTableEntry::ASBoundaryRouterDestination) != 0) {
         out << "ASBoundaryRouter";
      }
   }

   out << "), Area: "
         << entry.GetArea()
         << ", PathType: ";

    switch (entry.GetPathType()){
      case AnsaOspf6::RoutingTableEntry::IntraArea:
         out << "IntraArea";
         break;
      case AnsaOspf6::RoutingTableEntry::InterArea:
         out << "InterArea";
         break;
      case AnsaOspf6::RoutingTableEntry::Type1External:
         out << "Type1External";
         break;
      case AnsaOspf6::RoutingTableEntry::Type2External:
         out << "Type2External";
         break;
      default:
         out << "Unknown";
         break;
   }

   out << ", Cost: "
         << entry.GetCost()
         << ", Type2Cost: "
         << entry.GetType2Cost()
         << ", Origin: [";

   PrintLsaHeader6(entry.GetLinkStateOrigin()->getHeader(), out);
   out << "], NextHops: ";

   unsigned int hopCount = entry.GetNextHopCount();
   for (unsigned int i = 0; i < hopCount; i++) {
     out << entry.GetNextHop(i).hopAddress << " ";
   }

   return out;
}

#endif /* ANSAOSPFROUTINGTABLEENTRY6_H_ */
