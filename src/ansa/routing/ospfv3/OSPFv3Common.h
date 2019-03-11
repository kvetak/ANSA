#ifndef __ANSA_OSPFV3ROUTING_H_
#define __ANSA_OSPFV3ROUTING_H_

#include <omnetpp.h>
#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "ansa/routing/ospfv3/OSPFv3Packet_m.h"

namespace inet{
#define DEFAULT_IPV4_INSTANCE   64
#define DEFAULT_IPV6_INSTANCE   0
#define DEFAULT_ROUTER_PRIORITY 1
#define DEFAULT_HELLO_INTERVAL  10
#define DEFAULT_DEAD_INTERVAL   40
#define IPV4INSTANCE            1
#define IPV6INSTANCE            2
#define VIRTUAL_LINK_TTL        32
#define IPV6_DATAGRAM_LENGTH    65535
#define OSPFV3_HEADER_LENGTH    16
#define OSPFV3_DD_HEADER_LENGTH 12
#define OSPFV3_LSA_HEADER_LENGTH  20
#define OSPFV3_LSR_LENGTH       12
#define MAX_AGE                 3600
#define MAX_AGE_DIFF            900
#define MIN_LS_ARRIVAL          1
#define MIN_LS_INTERVAL         5
#define INITIAL_SEQUENCE_NUMBER 0              //TODO: -2147483647
#define MAX_SEQUENCE_NUMBER     2147483647
#define MAX_SPF_WAIT_TIME       10000
#define MIN_SPF_WAIT_TIME       10000
#define REFERENCE_BANDWIDTH     100

#define LS_REFRESH_TIME         1800
#define CHECK_AGE               300
#define LS_INFINITY             16777215

#define OSPFV3_ROUTER_LSA_HEADER_LENGTH 4
#define OSPFV3_ROUTER_LSA_BODY_LENGTH 16

#define OSPFV3_NETWORK_LSA_HEADER_LENGTH 4
#define OSPFV3_NETWORK_LSA_ATTACHED_LENGTH 4

#define OSPFV3_INTER_AREA_PREFIX_LSA_HEADER_LENGTH 4
#define OSPFV3_INTER_AREA_PREFIX_LSA_BODY_LENGTH 24

#define OSPFV3_INTRA_AREA_PREFIX_LSA_HEADER_LENGTH 12
#define OSPFV3_INTRA_AREA_PREFIX_LSA_PREFIX_LENGTH 20

#define OSPFV3_LINK_LSA_BODY_LENGTH 24
#define OSPFV3_LINK_LSA_PREFIX_LENGTH 20 //this is just temporary - the prefix now has fixed size

#define ROUTER_LSA_FUNCTION_CODE 1

const IPv4Address NULL_ROUTERID(0, 0, 0, 0);
const IPv4Address BACKBONE_AREAID(0, 0, 0, 0);
const IPv4Address NULL_LINKSTATEID(0, 0, 0, 0);
const IPv4Address NULL_IPV4ADDRESS(0, 0, 0, 0);


typedef IPv4Address AreaID;
typedef unsigned int Metric;


struct IPv6AddressRange
{
    IPv6Address prefix;
    short prefixLength;
    IPv6AddressRange() : prefix(), prefixLength() {}
    IPv6AddressRange(IPv6Address prefixPar, int prefixLengthPar) : prefix(prefixPar), prefixLength(prefixLengthPar) {}

    bool operator<(const IPv6AddressRange& other) const
    {
        return (prefixLength > other.prefixLength) || ((prefixLength == other.prefixLength) && (prefix < other.prefix));
    }

    bool operator==(const IPv6AddressRange& other) const
    {
        return (prefix == other.prefix) && (prefixLength == other.prefixLength);
    }

    bool contains(const IPv6Address& other) const // ma to robit to, ze vezme other a zisti, ci patri pod adresu siete ,respektive ci maju prefix a other rovnaku siet (cize prefix s dlzkou prefixlen musi byt rovnaky ako other s dlzkou prefixlen)
    {
        return (prefix.getPrefix(prefixLength) == other.getPrefix(prefixLength));
    }

    bool contains(const IPv6AddressRange& other) const
    {

        return prefix.getPrefix(prefixLength) == other.prefix.getPrefix(prefixLength)&& (prefixLength <= other.prefixLength);
    }

    bool containsRange(const IPv6Address& otherAddress, const int otherMask) const
    {
        return prefix.getPrefix(prefixLength) == otherAddress.getPrefix(prefixLength) && (prefixLength <= otherMask);
    }

    bool containedByRange(const IPv6Address& otherAddress, const int otherMask) const
    {
        return prefix.getPrefix(otherMask) == otherAddress.getPrefix(otherMask) && (otherMask <= prefixLength);
    }

    bool operator!=(IPv6AddressRange other) const
    {
        return !operator==(other);
    }

    std::string str() const;
};

inline std::string IPv6AddressRange::str() const
{
    std::string str(prefix.str());
    str += "/";
    str += prefixLength;
    return str;
}

const IPv6AddressRange NULL_IPV6ADDRESSRANGE(IPv6Address(0, 0, 0, 0), 0);

inline bool isSameNetwork(IPv6Address address1, int prefixLen1, IPv6Address address2, int prefixLen2)
{
    return (prefixLen1 == prefixLen2) && (address1.getPrefix(prefixLen1) == address2.getPrefix(prefixLen2) );
}

//-------------------------------------------------------------------------------------
// Address range for IPv4
struct IPv4AddressRange
{
    IPv4Address address;
    IPv4Address mask;
    IPv4AddressRange() : address(), mask() {}
    IPv4AddressRange(IPv4Address addressPar, IPv4Address maskPar) : address(addressPar), mask(maskPar) {}

    bool operator<(const IPv4AddressRange& other) const
    {
        return (mask > other.mask) || ((mask == other.mask) && (address < other.address));
    }

    bool operator==(const IPv4AddressRange& other) const
    {
        return (address == other.address) && (mask == other.mask);
    }

    bool contains(const IPv4Address& other) const
    {
        return IPv4Address::maskedAddrAreEqual(address, other, mask);
    }

    bool contains(const IPv4AddressRange& other) const
    {
        return IPv4Address::maskedAddrAreEqual(address, other.address, mask) && (mask <= other.mask);
    }

    bool containsRange(const IPv4Address& otherAddress, const IPv4Address& otherMask) const
    {
        return IPv4Address::maskedAddrAreEqual(address, otherAddress, mask) && (mask <= otherMask);
    }

    bool containedByRange(const IPv4Address& otherAddress, const IPv4Address& otherMask) const
    {
        return IPv4Address::maskedAddrAreEqual(otherAddress, address, otherMask) && (otherMask <= mask);
    }

    bool operator!=(IPv4AddressRange other) const
    {
        return !operator==(other);
    }

    std::string str() const;
};

inline std::string IPv4AddressRange::str() const
{
    std::string str(address.str(false));
    str += "/";
    str += mask.str(false);
    return str;
}

const IPv4AddressRange NULL_IPV4ADDRESSRANGE(IPv4Address(0, 0, 0, 0), IPv4Address(0, 0, 0, 0));

inline IPv4Address operator&(IPv4Address address, IPv4Address mask)
{
    IPv4Address maskedAddress;
    maskedAddress.set(address.getInt() & mask.getInt());
    return maskedAddress;
}

inline IPv4Address operator|(IPv4Address address, IPv4Address match)
{
    IPv4Address matchAddress;
    matchAddress.set(address.getInt() | match.getInt());
    return matchAddress;
}

inline bool isSameNetwork(IPv4Address address1, IPv4Address mask1, IPv4Address address2, IPv4Address mask2)
{
    return (mask1 == mask2) && ((address1 & mask1) == (address2 & mask2));
}

//individual LSAs are identified by a combination
//of their LS type, Link State ID, and Advertising Router fields
struct LSAKeyType
{
    uint16_t LSType;
    IPv4Address linkStateID;
    IPv4Address advertisingRouter;
};

//Things needed for SPF Tree Vertices
struct NextHop
{
    int ifIndex;
    //IPv6Address hopAddress;
    L3Address hopAddress;
    IPv4Address advertisingRouter;      // Router ID
};

struct VertexID {
    int interfaceID=-1; //Needed only for Network Vertex
    IPv4Address routerID;
};

inline bool operator==(const VertexID& leftID, const VertexID& rightID)
{
    return ((leftID.interfaceID == rightID.interfaceID)
            && (leftID.routerID == rightID.routerID));
}

enum InstallSource {
    ORIGINATED = 0,
    FLOODED = 1
};

struct VertexLSA {
    OSPFv3RouterLSA* routerLSA;
    OSPFv3NetworkLSA* networkLSA;
};

inline bool operator==(const NextHop& leftHop, const NextHop& rightHop)
{
    return (leftHop.ifIndex == rightHop.ifIndex) &&
           (leftHop.hopAddress == rightHop.hopAddress) &&
           (leftHop.advertisingRouter == rightHop.advertisingRouter);
}

inline bool operator!=(const NextHop& leftHop, const NextHop& rightHop)
{
    return !(leftHop == rightHop);
}

struct OSPFv3DDPacketID
{
    OSPFv3DDOptions ddOptions;
    OSPFv3Options options;
    unsigned long sequenceNumber;
};

inline bool operator != (OSPFv3DDOptions leftID, OSPFv3DDOptions rightID)
{
    return ((leftID.iBit != rightID.iBit) ||
            (leftID.mBit != rightID.mBit) ||
            (leftID.msBit != rightID.msBit));
}

inline bool operator == (OSPFv3Options leftID, OSPFv3Options rightID)
{
    return ((leftID.dcBit == rightID.dcBit) ||
            (leftID.eBit == rightID.eBit) ||
            (leftID.nBit == rightID.nBit) ||
            (leftID.rBit == rightID.rBit) ||
            (leftID.v6Bit == rightID.v6Bit) ||
            (leftID.xBit == rightID.xBit));
}

inline bool operator != (OSPFv3Options leftID, OSPFv3Options rightID)
{
    return !(leftID==rightID);
}

inline bool operator !=(OSPFv3DDPacketID leftID, OSPFv3DDPacketID rightID)
{
    return ((leftID.ddOptions.iBit != rightID.ddOptions.iBit) ||
       (leftID.ddOptions.mBit != rightID.ddOptions.mBit) ||
       (leftID.ddOptions.msBit != rightID.ddOptions.msBit) ||
       (leftID.options.dcBit != rightID.options.dcBit) ||
       (leftID.options.eBit != rightID.options.eBit) ||
       (leftID.options.nBit != rightID.options.nBit) ||
       (leftID.options.rBit != rightID.options.rBit) ||
       (leftID.options.v6Bit != rightID.options.v6Bit) ||
       (leftID.options.xBit != rightID.options.xBit) ||
       (leftID.sequenceNumber != rightID.sequenceNumber));
}

inline bool operator<(OSPFv3LSAHeader& leftLSA, OSPFv3LSAHeader& rightLSA)
{
    long leftSequenceNumber = leftLSA.getLsaSequenceNumber();
    long rightSequenceNumber = rightLSA.getLsaSequenceNumber();

    if (leftSequenceNumber < rightSequenceNumber) {
        return true;
    }
    if (leftSequenceNumber == rightSequenceNumber) {
        unsigned short leftAge = leftLSA.getLsaAge();
        unsigned short rightAge = rightLSA.getLsaAge();

        if ((leftAge != MAX_AGE) && (rightAge == MAX_AGE)) {
            return true;
        }
        if ((leftAge == MAX_AGE) && (rightAge != MAX_AGE)) {
            return false;
        }
        if ((abs(leftAge - rightAge) > MAX_AGE_DIFF) && (leftAge > rightAge)) {
            return true;
        }
    }
    return false;
}

inline bool operator==(OSPFv3LSAHeader& leftLSA, OSPFv3LSAHeader& rightLSA)
{
    long leftSequenceNumber = leftLSA.getLsaSequenceNumber();
    long rightSequenceNumber = rightLSA.getLsaSequenceNumber();
    unsigned short leftAge = leftLSA.getLsaAge();
    unsigned short rightAge = rightLSA.getLsaAge();

    if ((leftSequenceNumber == rightSequenceNumber) &&
        (((leftAge == MAX_AGE) && (rightAge == MAX_AGE)) ||
         (((leftAge != MAX_AGE) && (rightAge != MAX_AGE)) &&
          (abs(leftAge - rightAge) <= MAX_AGE_DIFF))))
    {
        return true;
    }
    else {
        return false;
    }
}
//Packets

}//namespace inet

#endif
