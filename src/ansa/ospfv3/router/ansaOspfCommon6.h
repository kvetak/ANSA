/*
 * ansaOSPFCommon6.h
 *
 *  Created on: 2.4.2011
 *      Author: TDLmarek
 */

#ifndef ANSAOSPFCOMMON6_H_
#define ANSAOSPFCOMMON6_H_

#include "IPv6Address.h"
#include "IPAddress.h"

#define OSPF_MAX_PROCESSID    65535
#define OSPF_MAX_INSTANCEID   255
#define OSPF_MAX_COST         65535
#define OSPF_MAX_PRIORITY     255

#define LS_REFRESH_TIME                     1800
#define MIN_LS_INTERVAL                     5
#define MIN_LS_ARRIVAL                      1
#define MAX_AGE                             3600
#define CHECK_AGE                           300
#define MAX_AGE_DIFF                        900
#define LS_INFINITY                         16777215
#define DEFAULT_DESTINATION_ADDRESS         0
#define INITIAL_SEQUENCE_NUMBER             -2147483647
#define MAX_SEQUENCE_NUMBER                 2147483647

#define VIRTUAL_LINK_TTL                    32

namespace AnsaOspf6 {

typedef uint32 RouterID;
typedef uint32 DesignatedRouterID;
typedef uint32 ProcessID;
typedef uint32 AreaID;
typedef uint32 IfaceID;
typedef uint8  InstanceID;
typedef uint16 Metric;
typedef uint32 LinkStateID;


class IPv6Address_Less : public std::binary_function <IPv6Address, IPv6Address, bool> {
   public:
      bool operator() (IPv6Address leftAddress, IPv6Address rightAddress) const;
};

// replaces IPv4AddressRange
struct IPv6AddressPrefix {
   IPv6Address address;
   int prefixLen;
};

class IPv6AddressPrefix_Less : public std::binary_function <IPv6AddressPrefix, IPv6AddressPrefix, bool> {
   public:
      bool operator() (IPv6AddressPrefix leftAddressRange, IPv6AddressPrefix rightAddressRange) const;
};

struct HostRouteParameters {
   unsigned char ifIndex;
   IPv6Address   address;
   Metric        linkCost;
};

struct LsaKeyType6 {
    LinkStateID linkStateID;
    RouterID    advertisingRouter;
};


class LsaKeyType6_Less: public std::binary_function<LsaKeyType6, LsaKeyType6, bool> {
   public:
      bool operator()(LsaKeyType6 leftKey, LsaKeyType6 rightKey) const;
};

const RouterID    NullRouterID = 0;
const AreaID      BackboneAreaID = 0;
const LinkStateID NullLinkStateID = 0;

// http://tools.ietf.org/html/rfc5340#appendix-A.1
const IPv6Address AllSPFRouters  = "FF02::5";
const IPv6Address AllDRouters    = "FF02::6";
const IPv6AddressPrefix NullAddressPrefix = {IPv6Address::UNSPECIFIED_ADDRESS, 128};

const DesignatedRouterID NullDesignatedRouterID = NullRouterID;
}

inline std::ostream& operator<<(std::ostream& os, AnsaOspf6::InstanceID instanceId){
   os << (int) instanceId;
   return os;
}

inline bool AnsaOspf6::IPv6Address_Less::operator() (IPv6Address leftAddress, IPv6Address rightAddress) const {
   return (leftAddress < rightAddress);
}

inline bool operator==(AnsaOspf6::IPv6AddressPrefix left, AnsaOspf6::IPv6AddressPrefix right) {
   return ( (left.address == right.address) &&
            (left.prefixLen == right.prefixLen));
}

inline bool operator!=(AnsaOspf6::IPv6AddressPrefix left, AnsaOspf6::IPv6AddressPrefix right) {
   return (!(left==right));
}

inline bool AnsaOspf6::IPv6AddressPrefix_Less::operator() (AnsaOspf6::IPv6AddressPrefix leftPrefix, AnsaOspf6::IPv6AddressPrefix rightPrefix) const {
   return ( (leftPrefix.address < rightPrefix.address)
         || (  (leftPrefix.address == rightPrefix.address)
            && (leftPrefix.prefixLen < rightPrefix.prefixLen)));
}

inline bool AnsaOspf6::LsaKeyType6_Less::operator() (AnsaOspf6::LsaKeyType6 leftKey, AnsaOspf6::LsaKeyType6 rightKey) const{
    return (   (leftKey.linkStateID < rightKey.linkStateID)
            || (  (leftKey.linkStateID == rightKey.linkStateID)
               && (leftKey.advertisingRouter < rightKey.advertisingRouter)));
}

#endif /* ANSAOSPFCOMMON6_H_ */
