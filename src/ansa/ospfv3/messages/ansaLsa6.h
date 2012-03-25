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

#ifndef ANSALSA6_H_
#define ANSALSA6_H_

#include "ansaOspfCommon6.h"
#include "ansaOspfPacket6_m.h"

namespace AnsaOspf6 {

struct NextHop {
   unsigned char ifIndex;
   IPv6Address   hopAddress;
   RouterID      advertisingRouter;
};


class RoutingInfo {
private:
   std::vector<NextHop>  nextHops;
   unsigned long         distance;
   OspfLsa6*             parent;

public:
            RoutingInfo  (void) : distance(0), parent(NULL) {}
            RoutingInfo  (const RoutingInfo& routingInfo) : nextHops(routingInfo.nextHops), distance(routingInfo.distance), parent(routingInfo.parent) {}

   virtual ~RoutingInfo(void) {}

   void            AddNextHop          (NextHop nextHop)           { nextHops.push_back(nextHop); }
   void            ClearNextHops       (void)                      { nextHops.clear(); }
   unsigned int    GetNextHopCount     (void) const                { return nextHops.size(); }
   NextHop         GetNextHop          (unsigned int index) const  { return nextHops[index]; }
   void            SetDistance         (unsigned long d)           { distance = d; }
   unsigned long   GetDistance         (void) const                { return distance; }
   void            SetParent           (OspfLsa6* p)               { parent = p; }
   OspfLsa6*       GetParent           (void) const                { return parent; }
};


class LsaTrackingInfo {
public:
   enum InstallSource {
      Originated = 0,
      Flooded = 1
   };

private:
   InstallSource   source;
   unsigned long   installTime;

public:
            LsaTrackingInfo(void) : source(Flooded), installTime(0) {}
            LsaTrackingInfo(const LsaTrackingInfo& info) : source(info.source), installTime(info.installTime) {}

   void            SetSource               (InstallSource installSource)   { source = installSource; }
   InstallSource   GetSource               (void) const                    { return source; }
   void            IncrementInstallTime    (void)                          { installTime++; }
   void            ResetInstallTime        (void)                          { installTime = 0; }
   unsigned long   GetInstallTime          (void) const                    { return installTime; }
};


class RouterLsa : public OspfRouterLsa6,
                  public RoutingInfo,
                  public LsaTrackingInfo
{
public:
            RouterLsa (void) : OspfRouterLsa6(), RoutingInfo(), LsaTrackingInfo() {}
            RouterLsa (const OspfRouterLsa6& lsa) : OspfRouterLsa6(lsa), RoutingInfo(), LsaTrackingInfo() {}
            RouterLsa (const RouterLsa& lsa) : OspfRouterLsa6(lsa), RoutingInfo(lsa), LsaTrackingInfo(lsa) {}
   virtual ~RouterLsa (void) {}

   bool     Update       (const OspfRouterLsa6* lsa);
   bool     DiffersFrom  (const OspfRouterLsa6* routerLsa) const;
};


class NetworkLsa : public OspfNetworkLsa6,
                   public RoutingInfo,
                   public LsaTrackingInfo
{
public:
            NetworkLsa (void) : OspfNetworkLsa6(), RoutingInfo(), LsaTrackingInfo() {}
            NetworkLsa (const OspfNetworkLsa6& lsa) : OspfNetworkLsa6(lsa), RoutingInfo(), LsaTrackingInfo() {}
            NetworkLsa (const NetworkLsa& lsa) : OspfNetworkLsa6(lsa), RoutingInfo(lsa), LsaTrackingInfo(lsa) {}
   virtual ~NetworkLsa (void) {}

   bool     Update      (const OspfNetworkLsa6* lsa);
   bool     DiffersFrom (const OspfNetworkLsa6* networkLsa) const;
};


class InterAreaPrefixLsa : public OspfInterAreaPrefixLsa6,
                           public RoutingInfo,
                           public LsaTrackingInfo
{
public:
            InterAreaPrefixLsa (void) : OspfInterAreaPrefixLsa6(), RoutingInfo(), LsaTrackingInfo() {}
            InterAreaPrefixLsa (const OspfInterAreaPrefixLsa6& lsa) : OspfInterAreaPrefixLsa6(lsa), RoutingInfo(), LsaTrackingInfo() {}
            InterAreaPrefixLsa (const InterAreaPrefixLsa& lsa) : OspfInterAreaPrefixLsa6(lsa), RoutingInfo(lsa), LsaTrackingInfo(lsa) {}
   virtual ~InterAreaPrefixLsa (void) {}

   bool     Update      (const OspfInterAreaPrefixLsa6* lsa);
   bool     DiffersFrom (const OspfInterAreaPrefixLsa6* interAreaPrefixLsa) const;
};


class InterAreaRouterLsa : public OspfInterAreaRouterLsa6,
                           public RoutingInfo,
                           public LsaTrackingInfo
{
public:
            InterAreaRouterLsa (void) : OspfInterAreaRouterLsa6(), RoutingInfo(), LsaTrackingInfo() {}
            InterAreaRouterLsa (const OspfInterAreaRouterLsa6& lsa) : OspfInterAreaRouterLsa6(lsa), RoutingInfo(), LsaTrackingInfo() {}
            InterAreaRouterLsa (const InterAreaRouterLsa& lsa) : OspfInterAreaRouterLsa6(lsa), RoutingInfo(lsa), LsaTrackingInfo(lsa) {}
   virtual ~InterAreaRouterLsa (void) {}

   bool     Update      (const OspfInterAreaRouterLsa6* lsa);
   bool     DiffersFrom (const OspfInterAreaRouterLsa6* interAreaRouterLsa) const;
};


class AsExternalLsa : public OspfAsExternalLsa6,
                      public RoutingInfo,
                      public LsaTrackingInfo
{
public:
            AsExternalLsa (void) : OspfAsExternalLsa6(), RoutingInfo(), LsaTrackingInfo() {}
            AsExternalLsa (const OspfAsExternalLsa6& lsa) : OspfAsExternalLsa6(lsa), RoutingInfo(), LsaTrackingInfo() {}
            AsExternalLsa (const AsExternalLsa& lsa) : OspfAsExternalLsa6(lsa), RoutingInfo(lsa), LsaTrackingInfo(lsa) {}
   virtual ~AsExternalLsa (void) {}

   bool     Update       (const OspfAsExternalLsa6* lsa);
   bool     DiffersFrom  (const OspfAsExternalLsa6* asExternalLsa) const;
};


class LinkLsa : public OspfLinkLsa6,
                public RoutingInfo,
                public LsaTrackingInfo
{
public:
            LinkLsa (void) : OspfLinkLsa6(), RoutingInfo(), LsaTrackingInfo() {}
            LinkLsa (const OspfLinkLsa6& lsa) : OspfLinkLsa6(lsa), RoutingInfo(), LsaTrackingInfo() {}
            LinkLsa (const LinkLsa& lsa) : OspfLinkLsa6(lsa), RoutingInfo(lsa), LsaTrackingInfo(lsa) {}
   virtual ~LinkLsa (void) {}

   bool     Update       (const OspfLinkLsa6* lsa);
   bool     DiffersFrom  (const OspfLinkLsa6* linkLsa) const;
};


class IntraAreaPrefixLsa : public OspfIntraAreaPrefixLsa6,
                           public RoutingInfo,
                           public LsaTrackingInfo
{
public:
            IntraAreaPrefixLsa (void) : OspfIntraAreaPrefixLsa6(), RoutingInfo(), LsaTrackingInfo() {}
            IntraAreaPrefixLsa (const OspfIntraAreaPrefixLsa6& lsa) : OspfIntraAreaPrefixLsa6(lsa), RoutingInfo(), LsaTrackingInfo() {}
            IntraAreaPrefixLsa (const IntraAreaPrefixLsa& lsa) : OspfIntraAreaPrefixLsa6(lsa), RoutingInfo(lsa), LsaTrackingInfo(lsa) {}
   virtual ~IntraAreaPrefixLsa (void) {}

   bool     Update       (const OspfIntraAreaPrefixLsa6* lsa);
   bool     DiffersFrom  (const OspfIntraAreaPrefixLsa6* intraAreaPrefixLsa) const;
};

}

/**
 * Returns true if leftLSA is older than rightLSA.
 */
inline bool operator<(const OspfLsaHeader6& leftLSA, const OspfLsaHeader6& rightLSA) {
   long leftSequenceNumber = leftLSA.getLsSequenceNumber();
   long rightSequenceNumber = rightLSA.getLsSequenceNumber();

   if (leftSequenceNumber < rightSequenceNumber){
      return true;
   }
   if (leftSequenceNumber == rightSequenceNumber){
      unsigned short leftAge = leftLSA.getLsAge();
      unsigned short rightAge = rightLSA.getLsAge();

      if ((leftAge != MAX_AGE) && (rightAge == MAX_AGE)){
         return true;
      }
      if ((abs(leftAge - rightAge) > MAX_AGE_DIFF) && (leftAge > rightAge)){
         return true;
      }
   }
   return false;
}

/**
 * Returns true if leftLSA is the same age as rightLSA.
 */
inline bool operator==(const OspfLsaHeader6& leftLSA, const OspfLsaHeader6& rightLSA) {
   long leftSequenceNumber = leftLSA.getLsSequenceNumber();
   long rightSequenceNumber = rightLSA.getLsSequenceNumber();
   unsigned short leftAge = leftLSA.getLsAge();
   unsigned short rightAge = rightLSA.getLsAge();

   if (  (leftSequenceNumber == rightSequenceNumber)
      && (
            (
                  (leftAge == MAX_AGE) && (rightAge == MAX_AGE)
            ) || (((leftAge != MAX_AGE) && (rightAge != MAX_AGE))
                  && (abs(leftAge - rightAge) <= MAX_AGE_DIFF)
            )
      )
   ){
      return true;
   }else{
      return false;
   }
}

inline bool operator==(const OspfOptions6& leftOptions, const OspfOptions6& rightOptions) {
   return ((leftOptions.V6_IPv6Routing == rightOptions.V6_IPv6Routing)
         && (leftOptions.E_ExternalRoutingCapability == rightOptions.E_ExternalRoutingCapability)
         && (leftOptions.N_NSSA == rightOptions.N_NSSA)
         && (leftOptions.R_Router == rightOptions.R_Router)
         && (leftOptions.DC_DemandCircuits == rightOptions.DC_DemandCircuits));
}

inline bool operator!=(const OspfOptions6& leftOptions, const OspfOptions6& rightOptions) {
   return (!(leftOptions == rightOptions));
}

inline bool operator==(const OspfPrefixOptions6& left, const OspfPrefixOptions6& right) {
   return ( (left.NU_NoUnicast == right.NU_NoUnicast)
         && (left.LA_LocalAddress == right.LA_LocalAddress));
}

inline bool operator!=(const OspfPrefixOptions6& left, const OspfPrefixOptions6& right) {
   return (!(left == right));
}

inline bool operator==(const OspfPrefix6& left, const OspfPrefix6& right) {
   return ( (left.getPrefixLenght() == right.getPrefixLenght())
         && (left.getPrefixOptions() == right.getPrefixOptions())
         && (left.getMetric() == right.getMetric())
         && (left.getAddressPrefix() == right.getAddressPrefix()));
}

inline bool operator!=(const OspfPrefix6& left, const OspfPrefix6& right) {
   return (!(left == right));
}

inline bool operator==(const Link6& left, const Link6& right) {
   return ( (left.getType() == right.getType())
         && (left.getMetric() == right.getMetric())
         && (left.getInterfaceID() == right.getInterfaceID())
         && (left.getNeighborInterfaceID() == right.getNeighborInterfaceID())
         && (left.getNeighborRouterID() == right.getNeighborRouterID()));
}

inline bool operator!=(const Link6& left, const Link6& right) {
   return (!(left == right));
}

inline bool operator==(const AnsaOspf6::NextHop& leftHop, const AnsaOspf6::NextHop& rightHop) {
   return ( (leftHop.ifIndex           == rightHop.ifIndex) &&
            (leftHop.hopAddress        == rightHop.hopAddress) &&
            (leftHop.advertisingRouter == rightHop.advertisingRouter));
}

inline bool operator!=(const AnsaOspf6::NextHop& leftHop, const AnsaOspf6::NextHop& rightHop) {
   return (!(leftHop == rightHop));
}

inline void PrintLsaHeader6(const OspfLsaHeader6& lsaHeader, std::ostream& o) {
   o << "LsaHeader: age=" << lsaHeader.getLsAge() << ", type=";
   switch (lsaHeader.getLsType()){
      case RouterLsaType:
         o << "RouterLsa";
         break;
      case NetworkLsaType:
         o << "NetworkLsa";
         break;
      case InterAreaPrefixLsaType:
         o << "InterAreaPrefixLsa";
         break;
      case InterAreaRouterLsaType:
         o << "InterAreaRouterLsa";
         break;
      case AsExternalLsaType:
         o << "AsExternalLsa";
         break;
      case LinkLsaType:
         o << "LinkLsa";
         break;
      case IntraAreaPrefixLsaType:
         o << "IntraAreaPrefixLsa";
         break;
      default:
         o << "UnknownLsa";
         break;
   }
   o << ", LSID="
     << lsaHeader.getLinkStateID();
   o << ", advertisingRouter="
     << IPAddress(lsaHeader.getAdvertisingRouter())
     << ", seqNumber=" << lsaHeader.getLsSequenceNumber();
   o << endl;
}

inline void printLsaLink(const Link6& link, std::ostream& o) {
   o << "   " << link.getMetric();
   o << ", neighbor=" << IPAddress(link.getNeighborRouterID());
   o << ", neighbor iface=" << link.getNeighborInterfaceID();
   o << ", type=";
   switch (link.getType()) {
      case PointToPointLink:  o << "PointToPoint";   break;
      case TransitLink:       o << "Transit";        break;
      case VirtualLink:       o << "Virtual";        break;
      default:                o << "Unknown";        break;
   }

   o << ", iface=" << link.getInterfaceID() << endl;;
}

inline void PrintRouterLsa(const OspfRouterLsa6& lsa, std::ostream& o) {
   o << "bits=" << ((lsa.getV_VirtualLinkEndpoint()) ? "V " : "_ ")
                << ((lsa.getE_AsBoundaryRouter()) ? "E " : "_ ")
                << ((lsa.getB_AreaBorderRouter()) ? "B" : "_") << endl;

   o << "links:" << endl;
   unsigned int linkCount = lsa.getLinksArraySize();
   for (unsigned int j = 0; j < linkCount; j++){
      const Link6& link = lsa.getLinks(j);
         printLsaLink(link, o);
   }
   // TODO: format + options?
}

inline void PrintNetworkLsa(const OspfNetworkLsa6& lsa, std::ostream& o) {
   unsigned int routerCount = lsa.getAttachedRoutersArraySize();
   for (unsigned int i = 0; i < routerCount; i++) {
      o << "   " << IPAddress(lsa.getAttachedRouters(i)) << endl;
   }
   // TODO: options?
}

inline void PrintInterAreaPrefixLsa(const OspfInterAreaPrefixLsa6& lsa, std::ostream& o) {
   o << "   " << lsa.getMetric();
   o << ", " << lsa.getAddressPrefix() << "/" << lsa.getPrefixLenght() << endl;
   // TODO: prefixOptions?
}

inline void PrintInterAreaRouterLsa(const OspfInterAreaRouterLsa6& lsa, std::ostream& o) {
   o << "   " << lsa.getMetric();
   o << ", " << IPAddress(lsa.getDestinationRouterID()) << endl;
   // TODO: options?
}

inline void PrintAsExternalLsa(const OspfAsExternalLsa6& lsa, std::ostream& o) {
   o << "   " << lsa.getMetric();
   o << ", bits=" << ((lsa.getE_ExternalMetricType()) ? "E " : "_ ")
                  << ((lsa.getF_ForwardingAddress()) ? "F " : "_ ")
                  << ((lsa.getT_ExternalRouteTag()) ? "T" : "_") << endl;
   o << ", " << lsa.getAddressPrefix() << "/" << lsa.getPrefixLenght();
   if (lsa.getF_ForwardingAddress()){
      o << ", forward=" << lsa.getForwardingAddress();
   }
   if (lsa.getT_ExternalRouteTag()){
      o << ", routeTag=" << lsa.getExternalRouteTag();
   }
   o << endl;
}

inline void PrintLinkLsa(const OspfLinkLsa6& lsa, std::ostream& o) {
   o << "   " << lsa.getRtrPriority();
   o << ", " << lsa.getLinkLocalAddress() << " prefixes:" << endl;
   // TODO: options?
   unsigned int prefixCount = lsa.getPrefixesArraySize();
   for (unsigned int i = 0; i < prefixCount; i++) {
      OspfPrefix6 prefix = lsa.getPrefixes(i);
      o << "   " << prefix.getMetric() << ", "
        << prefix.getAddressPrefix() << "/" << prefix.getPrefixLenght() << endl;
   }
}

inline void PrintIntraAreaPrefixLsa(const OspfIntraAreaPrefixLsa6& lsa, std::ostream& o) {
   o << "   refType=" << lsa.getReferencedLsType();
   o << ", refLinkId=" << lsa.getReferencedLinkStateID();
   o << ", refRouter=" << IPAddress(lsa.getReferencedAdvertisingRouter()) << " prefixes:"<< endl;
   unsigned int prefixCount = lsa.getPrefixesArraySize();
   for (unsigned int i = 0; i < prefixCount; i++) {
      OspfPrefix6 prefix = lsa.getPrefixes(i);
      o << "   " << prefix.getMetric() << ", "
        << prefix.getAddressPrefix() << "/" << prefix.getPrefixLenght() << endl;
   }
}

inline std::ostream& operator<<(std::ostream& ostr, OspfLsa6& lsa) {
   PrintLsaHeader6(lsa.getHeader(), ostr);
   return ostr;
}

#endif /* ANSALSA6_H_ */
