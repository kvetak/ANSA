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
/**
* @file BabelFtlv.cc
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
* @brief Babel FTLVs
* @detail Includes classes represent Full TLVs for internal representation
*/

#include "ansa/applications/babel/BabelFtlv.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#include "winsock2.h"  // htonl, ntohl, ...
#else
#include <netinet/in.h>  // htonl, ntohl, ...
#endif


using namespace Babel;


//BASE FTLV
void BabelFtlv::copy(const BabelFtlv& other)
{
    type = other.type;
    sendNum = other.sendNum;
}

BabelFtlv& BabelFtlv::operator=(const BabelFtlv& other)
{
    if (this==&other) return *this;
    copy(other);
    return *this;
}

std::ostream& operator<<(std::ostream& os, const BabelFtlv& i)
{
    return os << i.str();
}

BabelFtlv::~BabelFtlv() {

}

/**
 * Length of raw TLV
 *
 * @return  Length of raw TLV in bytes
 */
int BabelFtlv::rawTlvLength() const
{
    return -1;
}

std::string BabelFtlv::str() const
{
    std::stringstream string;
    string << tlvT::toStr(type);

    if(sendNum == USE_ACK)
    {
        string << ", use ACK";
    }
    else
    {
        string << ", sendNum: " << sendNum;
    }

    return string.str();
}

//PAD1
void BabelPad1Ftlv::copy(const BabelPad1Ftlv& other)
{
    type = other.type;
    sendNum = other.sendNum;
}

BabelPad1Ftlv& BabelPad1Ftlv::operator=(const BabelPad1Ftlv& other)
{
    if (this==&other) return *this;
    copy(other);
    return *this;
}

std::ostream& operator<<(std::ostream& os, const BabelPad1Ftlv& i)
{
    return os << i.str();
}

BabelPad1Ftlv::~BabelPad1Ftlv()
{

}

/**
 * Length of raw TLV
 *
 * @return  Length of raw TLV in bytes
 */
int BabelPad1Ftlv::rawTlvLength() const
{
    return sizeof(type);
}

/**
 * Convert FTLV to raw TLV and copy to memory
 *
 * @param   dst     Destination
 *
 * @return  Number of copied bytes
 */
int BabelPad1Ftlv::copyRawTlv(char *dst) const
{
    int offset = 0;

    //set type
    dst[offset] = type;
    offset += sizeof(type);

    return offset;
}

std::string BabelPad1Ftlv::str() const
{
    std::stringstream string;

    string << BabelFtlv::str();

    return string.str();
}

//PADN
void BabelPadNFtlv::copy(const BabelPadNFtlv& other)
{
    type = other.type;
    sendNum = other.sendNum;
    n = other.n;
}

BabelPadNFtlv& BabelPadNFtlv::operator=(const BabelPadNFtlv& other)
{
    if (this==&other) return *this;
    copy(other);
    return *this;
}

std::ostream& operator<<(std::ostream& os, const BabelPadNFtlv& i)
{
    return os << i.str();
}

BabelPadNFtlv::~BabelPadNFtlv()
{

}

/**
 * Length of raw TLV
 *
 * @return  Length of raw TLV in bytes
 */
int BabelPadNFtlv::rawTlvLength() const
{
    return sizeof(type) + sizeof(uint8_t) +  (n * sizeof(uint8_t));
}

/**
 * Convert FTLV to raw TLV and copy to memory
 *
 * @param   dst     Destination
 *
 * @return  Number of copied bytes
 */
int BabelPadNFtlv::copyRawTlv(char *dst) const
{
    int offset = 0;

    //set type
    dst[offset] = type;
    offset += sizeof(type);

    //set length
    dst[offset] = n;
    offset += sizeof(uint8_t);

    //set padding
    for(uint8_t i = 0; i < n; ++i)
    {
        dst[offset] = static_cast<uint8_t>(0);
        offset += sizeof(uint8_t);
    }

    return offset;
}

std::string BabelPadNFtlv::str() const
{
    std::stringstream string;

    string << BabelFtlv::str();
    string << ", N: " << static_cast<unsigned int>(n);

    return string.str();
}

//ACKREQ
void BabelAckReqFtlv::copy(const BabelAckReqFtlv& other)
{
    type = other.type;
    sendNum = other.sendNum;
    reserved = other.reserved;
    nonce = other.nonce;
    interval = other.interval;
}

BabelAckReqFtlv& BabelAckReqFtlv::operator=(const BabelAckReqFtlv& other)
{
    if (this==&other) return *this;
    copy(other);
    return *this;
}

std::ostream& operator<<(std::ostream& os, const BabelAckReqFtlv& i)
{
    return os << i.str();
}

BabelAckReqFtlv::~BabelAckReqFtlv()
{

}

/**
 * Length of raw TLV
 *
 * @return  Length of raw TLV in bytes
 */
int BabelAckReqFtlv::rawTlvLength() const
{
    return sizeof(type) + sizeof(uint8_t) + sizeof(reserved) + sizeof(nonce) + sizeof(interval);
}

/**
 * Convert FTLV to raw TLV and copy to memory
 *
 * @param   dst     Destination
 *
 * @return  Number of copied bytes
 */
int BabelAckReqFtlv::copyRawTlv(char *dst) const
{
    int offset = 0;

    //set type
    dst[offset] = type;
    offset += sizeof(type);

    //set length
    dst[offset] = static_cast<uint8_t>(sizeof(reserved) + sizeof(nonce) + sizeof(interval));
    offset += sizeof(uint8_t);

    //copy reserved
    *reinterpret_cast<uint16_t *>(dst + offset) = htons(reserved);
    offset += sizeof(reserved);

    //copy nonce
    *reinterpret_cast<uint16_t *>(dst + offset) = htons(nonce);
    offset += sizeof(nonce);

    //copy interval
    *reinterpret_cast<uint16_t *>(dst + offset) = htons(interval);
    offset += sizeof(interval);


    return offset;
}

std::string BabelAckReqFtlv::str() const
{
    std::stringstream string;

    string << BabelFtlv::str();
    string << ", Nonce: " << nonce << ", Interval: " << interval;

    return string.str();
}



//ACK
void BabelAckFtlv::copy(const BabelAckFtlv& other)
{
    type = other.type;
    sendNum = other.sendNum;
    nonce = other.nonce;
}

BabelAckFtlv& BabelAckFtlv::operator=(const BabelAckFtlv& other)
{
    if (this==&other) return *this;
    copy(other);
    return *this;
}

std::ostream& operator<<(std::ostream& os, const BabelAckFtlv& i)
{
    return os << i.str();
}

BabelAckFtlv::~BabelAckFtlv()
{

}

/**
 * Length of raw TLV
 *
 * @return  Length of raw TLV in bytes
 */
int BabelAckFtlv::rawTlvLength() const
{
    return sizeof(type) + sizeof(uint8_t) +  sizeof(nonce);
}

/**
 * Convert FTLV to raw TLV and copy to memory
 *
 * @param   dst     Destination
 *
 * @return  Number of copied bytes
 */
int BabelAckFtlv::copyRawTlv(char *dst) const
{
    int offset = 0;

    //set type
    dst[offset] = type;
    offset += sizeof(type);

    //set length
    dst[offset] = static_cast<uint8_t>(sizeof(nonce));
    offset += sizeof(uint8_t);

    //copy nonce
    *reinterpret_cast<uint16_t *>(dst + offset) = htons(nonce);
    offset += sizeof(nonce);

    return offset;
}

std::string BabelAckFtlv::str() const
{
    std::stringstream string;

    string << BabelFtlv::str();
    string << ", Nonce: " << nonce;

    return string.str();
}


//HELLO
void BabelHelloFtlv::copy(const BabelHelloFtlv& other)
{
    type = other.type;
    sendNum = other.sendNum;
    reserved = other.reserved;
    seqno = other.seqno;
    interval = other.interval;
}

BabelHelloFtlv& BabelHelloFtlv::operator=(const BabelHelloFtlv& other)
{
    if (this==&other) return *this;
    copy(other);
    return *this;
}

std::ostream& operator<<(std::ostream& os, const BabelHelloFtlv& i)
{
    return os << i.str();
}

BabelHelloFtlv::~BabelHelloFtlv()
{

}

/**
 * Length of raw TLV
 *
 * @return  Length of raw TLV in bytes
 */
int BabelHelloFtlv::rawTlvLength() const
{
    return sizeof(type) + sizeof(uint8_t) + sizeof(reserved) + sizeof(seqno) + sizeof(interval);
}

/**
 * Convert FTLV to raw TLV and copy to memory
 *
 * @param   dst     Destination
 *
 * @return  Number of copied bytes
 */
int BabelHelloFtlv::copyRawTlv(char *dst) const
{
    int offset = 0;

    //set type
    dst[offset] = type;
    offset += sizeof(type);

    //set length
    dst[offset] = static_cast<uint8_t>(sizeof(reserved) + sizeof(seqno) + sizeof(interval));
    offset += sizeof(uint8_t);

    //copy reserved
    *reinterpret_cast<uint16_t *>(dst + offset) = htons(reserved);
    offset += sizeof(reserved);

    //copy seqno
    *reinterpret_cast<uint16_t *>(dst + offset) = htons(seqno);
    offset += sizeof(seqno);

    //copy interval
    *reinterpret_cast<uint16_t *>(dst + offset) = htons(interval);
    offset += sizeof(interval);


    return offset;
}

std::string BabelHelloFtlv::str() const
{
    std::stringstream string;

    string << BabelFtlv::str();
    string << ", SeqNo: " << seqno << ", Interval: " << interval;

    return string.str();
}

//IHU
void BabelIhuFtlv::copy(const BabelIhuFtlv& other)
{
    type = other.type;
    sendNum = other.sendNum;
    ae = other.ae;
    reserved = other.reserved;
    rxcost = other.rxcost;
    interval = other.interval;
    address = other.address;
}

BabelIhuFtlv& BabelIhuFtlv::operator=(const BabelIhuFtlv& other)
{
    if (this==&other) return *this;
    copy(other);
    return *this;
}

std::ostream& operator<<(std::ostream& os, const BabelIhuFtlv& i)
{
    return os << i.str();
}

BabelIhuFtlv::~BabelIhuFtlv()
{

}

/**
 * Length of raw TLV
 *
 * @return  Length of raw TLV in bytes
 */
int BabelIhuFtlv::rawTlvLength() const
{
    return sizeof(type) + sizeof(uint8_t) + sizeof(ae) + sizeof(reserved) + sizeof(rxcost) + sizeof(interval) + AE::maxLen(ae);
}

/**
 * Convert FTLV to raw TLV and copy to memory
 *
 * @param   dst     Destination
 *
 * @return  Number of copied bytes
 */
int BabelIhuFtlv::copyRawTlv(char *dst) const
{
    int offset = 0;

    //set type
    dst[offset] = type;
    offset += sizeof(type);

    //set length
    dst[offset] = static_cast<uint8_t>(sizeof(ae) + sizeof(reserved) + sizeof(rxcost) + sizeof(interval) + AE::maxLen(ae));
    offset += sizeof(uint8_t);

    //copy ae
    *reinterpret_cast<uint8_t *>(dst + offset) = ae;
    offset += sizeof(ae);

    //copy reserved
    *reinterpret_cast<uint8_t *>(dst + offset) = reserved;
    offset += sizeof(reserved);


    //copy rxcost
    *reinterpret_cast<uint16_t *>(dst + offset) = htons(rxcost);
    offset += sizeof(rxcost);

    //copy interval
    *reinterpret_cast<uint16_t *>(dst + offset) = htons(interval);
    offset += sizeof(interval);

    //copy address
    if(ae != AE::WILDCARD)
    {
        ASSERT(ae == static_cast<uint8_t>(getAeOfAddr(address)));

        offset += copyRawAddr(address, reinterpret_cast<char *>(dst + offset));
    }



    return offset;
}

std::string BabelIhuFtlv::str() const
{
    std::stringstream string;

    string << BabelFtlv::str();
    string << ", AE: " << AE::toStr(ae);
    string << ", rxcost: " << static_cast<unsigned int>(rxcost);
    string << ", interval: " << static_cast<unsigned int>(interval);
    string << ", address: " << address;

    return string.str();
}


//ROUTERID
void BabelRouterIdFtlv::copy(const BabelRouterIdFtlv& other)
{
    type = other.type;
    sendNum = other.sendNum;
    reserved = other.reserved;
    routerId = other.routerId;
}

BabelRouterIdFtlv& BabelRouterIdFtlv::operator=(const BabelRouterIdFtlv& other)
{
    if (this==&other) return *this;
    copy(other);
    return *this;
}

std::ostream& operator<<(std::ostream& os, const BabelRouterIdFtlv& i)
{
    return os << i.str();
}

BabelRouterIdFtlv::~BabelRouterIdFtlv()
{

}

/**
 * Length of raw TLV
 *
 * @return  Length of raw TLV in bytes
 */
int BabelRouterIdFtlv::rawTlvLength() const
{
    return sizeof(type) + sizeof(uint8_t) + sizeof(reserved) +  (2 * sizeof(uint32_t));
}

/**
 * Convert FTLV to raw TLV and copy to memory
 *
 * @param   dst     Destination
 *
 * @return  Number of copied bytes
 */
int BabelRouterIdFtlv::copyRawTlv(char *dst) const
{
    int offset = 0;

    //set type
    dst[offset] = type;
    offset += sizeof(type);

    //set length
    dst[offset] = static_cast<uint8_t>(sizeof(reserved) +  (2 * sizeof(uint32_t)));
    offset += sizeof(uint8_t);

    //copy reserved
    *reinterpret_cast<uint16_t *>(dst + offset) = htons(reserved);
    offset += sizeof(reserved);

    //copy routerid
    *reinterpret_cast<uint32_t *>(dst + offset) = htonl(routerId.getRid()[0]);
    offset += sizeof(uint32_t);
    *reinterpret_cast<uint32_t *>(dst + offset) = htonl(routerId.getRid()[1]);
    offset += sizeof(uint32_t);


    return offset;
}

std::string BabelRouterIdFtlv::str() const
{
    std::stringstream string;

    string << BabelFtlv::str();
    string << ", rid: " << routerId;

    return string.str();
}

//NEXTHOP
void BabelNextHopFtlv::copy(const BabelNextHopFtlv& other)
{
    type = other.type;
    sendNum = other.sendNum;
    ae = other.ae;
    reserved = other.reserved;
    nextHop = other.nextHop;
}

BabelNextHopFtlv& BabelNextHopFtlv::operator=(const BabelNextHopFtlv& other)
{
    if (this==&other) return *this;
    copy(other);
    return *this;
}

std::ostream& operator<<(std::ostream& os, const BabelNextHopFtlv& i)
{
    return os << i.str();
}

BabelNextHopFtlv::~BabelNextHopFtlv()
{

}

/**
 * Length of raw TLV
 *
 * @return  Length of raw TLV in bytes
 */
int BabelNextHopFtlv::rawTlvLength() const
{
    return sizeof(type) + sizeof(uint8_t) + sizeof(ae) + sizeof(reserved) + AE::maxLen(ae);
}

/**
 * Convert FTLV to raw TLV and copy to memory
 *
 * @param   dst     Destination
 *
 * @return  Number of copied bytes
 */
int BabelNextHopFtlv::copyRawTlv(char *dst) const
{
    ASSERT(ae != AE::WILDCARD); //RFC 6126, 4.4.8: AE MUST NOT be 0

    int offset = 0;

    //set type
    dst[offset] = type;
    offset += sizeof(type);

    //set length
    dst[offset] = static_cast<uint8_t>(sizeof(ae) + sizeof(reserved) + AE::maxLen(ae));
    offset += sizeof(uint8_t);

    //copy ae
    *reinterpret_cast<uint8_t *>(dst + offset) = ae;
    offset += sizeof(ae);

    //copy reserved
    *reinterpret_cast<uint8_t *>(dst + offset) = reserved;
    offset += sizeof(reserved);

    //copy nexthop
    ASSERT(ae == static_cast<uint8_t>(getAeOfAddr(nextHop)));
    offset += copyRawAddr(nextHop, reinterpret_cast<char *>(dst + offset));

    return offset;
}

std::string BabelNextHopFtlv::str() const
{
    std::stringstream string;

    string << BabelFtlv::str();
    string << ", AE: " << AE::toStr(ae);
    string << ", next-hop: " << nextHop;

    return string.str();
}


//UPDATE
void BabelUpdateFtlv::copy(const BabelUpdateFtlv& other)
{
    type = other.type;
    sendNum = other.sendNum;
    ae = other.ae;
    flags = other.flags;
    interval = other.interval;
    prefix = other.prefix;
    dist = other.dist;
    routerId = other.routerId;
    nextHop = other.nextHop;
}

BabelUpdateFtlv& BabelUpdateFtlv::operator=(const BabelUpdateFtlv& other)
{
    if (this==&other) return *this;
    copy(other);
    return *this;
}

std::ostream& operator<<(std::ostream& os, const BabelUpdateFtlv& i)
{
    return os << i.str();
}

BabelUpdateFtlv::~BabelUpdateFtlv()
{

}

/**
 * Length of raw TLV
 *
 * @return  Length of raw TLV in bytes
 */
int BabelUpdateFtlv::rawTlvLength() const
{
    return sizeof(type) + sizeof(uint8_t) + sizeof(ae) + sizeof(flags)
            + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(interval)
            + sizeof(uint16_t) + sizeof(uint16_t)
            + AE::maxLen(ae);
}

/**
 * Length of raw TLV
 *
 * @param   prevprefix  Previous prefix (used for omitting)
 *
 * @return  Length of raw TLV in bytes
 */
int BabelUpdateFtlv::rawTlvLength(const netPrefix<inet::L3Address>& prevprefix) const
{
    int prefixlen = 0;

    if(ae == AE::WILDCARD)
    {
        prefixlen = 0;
    }
    else if(ae == AE::LLIPv6)
    {
        prefixlen = prefix.lenInBytes() - (2 * sizeof(uint32_t)) - prefix.bytesToOmit(prevprefix);

        if(prefixlen < 0)
        {// llIPv6 prefix shorter than /64
            prefixlen = 0;
        }
    }
    else
    {
        prefixlen = prefix.lenInBytes() - prefix.bytesToOmit(prevprefix);
    }

    return sizeof(type) + sizeof(uint8_t) + sizeof(ae) + sizeof(flags)
            + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(interval)
            + sizeof(uint16_t) + sizeof(uint16_t)
            + prefixlen;
}

/**
 * Convert FTLV to raw TLV and copy to memory
 *
 * @param   dst     Destination
 *
 * @return  Number of copied bytes
 */
int BabelUpdateFtlv::copyRawTlv(char *dst) const
{
    return copyRawTlv(dst, NULL);
}

/**
 * Convert FTLV to raw TLV and copy to memory
 *
 * @param   dst     Destination
 * @param   prefprefix  Previous prefix (used for omitting)
 *
 * @return  Number of copied bytes
 */
int BabelUpdateFtlv::copyRawTlv(char *dst, netPrefix<inet::L3Address> *prevprefix) const
{
    int offset = 0;
    int toomit = ((prevprefix == NULL || ae == AE::WILDCARD) ? 0 : prefix.bytesToOmit(*prevprefix));

    //set type
    dst[offset] = type;
    offset += sizeof(type);

    //set length
    dst[offset] = static_cast<uint8_t>(sizeof(ae) + sizeof(flags)
            + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(interval)
            + sizeof(uint16_t) + sizeof(uint16_t)
            + ((ae == AE::WILDCARD) ? 0 : (prefix.lenInBytes() - toomit)));
    offset += sizeof(uint8_t);

    //copy ae
    *reinterpret_cast<uint8_t *>(dst + offset) = ae;
    offset += sizeof(ae);

    //copy flags and save flags pointer
    uint8_t *flagsptr = reinterpret_cast<uint8_t *>(dst + offset);
    *flagsptr = flags;
    offset += sizeof(flags);

    //save plen pointer
    uint8_t *plenptr = reinterpret_cast<uint8_t *>(dst + offset);
    offset += sizeof(uint8_t);

    //copy omitted
    *reinterpret_cast<uint8_t *>(dst + offset) = static_cast<uint8_t>(toomit);
    offset += sizeof(uint8_t);

    //copy interval
    *reinterpret_cast<uint16_t *>(dst + offset) = htons(interval);
    offset += sizeof(interval);

    //copy seqno
    *reinterpret_cast<uint16_t *>(dst + offset) = htons(dist.getSeqno());
    offset += sizeof(uint16_t);

    //copy metric
    *reinterpret_cast<uint16_t *>(dst + offset) = htons(dist.getMetric());
    offset += sizeof(uint16_t);

    //copy prefix
    if(ae == AE::WILDCARD)
    {
        *plenptr = 0;
    }
    else
    {
        if(toomit == 0)
        {// full prefix is contained
            *flagsptr |= 0x80;
        }

        ASSERT(ae == static_cast<uint8_t>(getAeOfPrefix(prefix.getAddr())));
        offset += prefix.copyRaw(reinterpret_cast<char *>(dst + offset), plenptr, toomit);
    }

    return offset;
}

std::string BabelUpdateFtlv::str() const
{
    std::stringstream string;

    string << BabelFtlv::str();
    string << ", AE: " << AE::toStr(ae) << ", Flags: " << static_cast<unsigned int>(flags) << ", Interval: " << interval
           << ", Prefix: " << prefix << ", Distance: " << dist << ", Router-Id: " << routerId
           << ", Next-hop: " << nextHop;

    return string.str();
}

//ROUTEREQ
void BabelRouteReqFtlv::copy(const BabelRouteReqFtlv& other)
{
    type = other.type;
    sendNum = other.sendNum;
    ae = other.ae;
    prefix = other.prefix;
}

BabelRouteReqFtlv& BabelRouteReqFtlv::operator=(const BabelRouteReqFtlv& other)
{
    if (this==&other) return *this;
    copy(other);
    return *this;
}

std::ostream& operator<<(std::ostream& os, const BabelRouteReqFtlv& i)
{
    return os << i.str();
}

BabelRouteReqFtlv::~BabelRouteReqFtlv()
{

}

/**
 * Length of raw TLV
 *
 * @return  Length of raw TLV in bytes
 */
int BabelRouteReqFtlv::rawTlvLength() const
{
    return sizeof(type) + sizeof(uint8_t) + sizeof(ae) + sizeof(uint8_t)
            + ((ae == AE::WILDCARD) ? 0 : prefix.lenInBytes());
}

/**
 * Convert FTLV to raw TLV and copy to memory
 *
 * @param   dst     Destination
 *
 * @return  Number of copied bytes
 */
int BabelRouteReqFtlv::copyRawTlv(char *dst) const
{
    ASSERT(dst != NULL);
    int offset = 0;

    //set type
    dst[offset] = type;
    offset += sizeof(type);

    //set length
    dst[offset] = static_cast<uint8_t>(sizeof(ae) + sizeof(uint8_t) + ((ae == AE::WILDCARD) ? 0 : prefix.lenInBytes()));
    offset += sizeof(uint8_t);

    //copy ae
    *reinterpret_cast<uint8_t *>(dst + offset) = ae;
    offset += sizeof(ae);

    //save plen pointer
    uint8_t *plenptr = reinterpret_cast<uint8_t *>(dst + offset);
    offset += sizeof(uint8_t);

    //copy prefix and plen
    if(ae == AE::WILDCARD)
    {
        *plenptr = static_cast<uint8_t>(0); //RFC 6126, 4.4.10: if AE is 0, than plen MUST be 0
    }
    else
    {
        ASSERT(ae == static_cast<uint8_t>(getAeOfPrefix(prefix.getAddr())));
        offset += prefix.copyRaw(reinterpret_cast<char *>(dst + offset), plenptr);
    }

    return offset;
}

std::string BabelRouteReqFtlv::str() const
{
    std::stringstream string;

    string << BabelFtlv::str();
    string << ", AE: " << AE::toStr(ae);
    string << ", prefix: " << prefix;

    return string.str();
}


//SEQNOREQ
void BabelSeqnoReqFtlv::copy(const BabelSeqnoReqFtlv& other)
{
    type = other.type;
    sendNum = other.sendNum;
    ae = other.ae;
    seqno = other.seqno;
    hopcount = other.hopcount;
    reserved = other.reserved;
    routerid = other.routerid;
    prefix = other.prefix;
}

BabelSeqnoReqFtlv& BabelSeqnoReqFtlv::operator=(const BabelSeqnoReqFtlv& other)
{
    if (this==&other) return *this;
    copy(other);
    return *this;
}

std::ostream& operator<<(std::ostream& os, const BabelSeqnoReqFtlv& i)
{
    return os << i.str();
}

BabelSeqnoReqFtlv::~BabelSeqnoReqFtlv()
{

}

/**
 * Length of raw TLV
 *
 * @return  Length of raw TLV in bytes
 */
int BabelSeqnoReqFtlv::rawTlvLength() const
{
    return sizeof(type) + sizeof(uint8_t) + sizeof(ae) + sizeof(uint8_t) + sizeof(seqno) + sizeof(hopcount) + sizeof(reserved) + (2 * sizeof(uint32_t)) + ((ae == AE::WILDCARD) ? 0 : prefix.lenInBytes());
}

/**
 * Convert FTLV to raw TLV and copy to memory
 *
 * @param   dst     Destination
 *
 * @return  Number of copied bytes
 */
int BabelSeqnoReqFtlv::copyRawTlv(char *dst) const
{
    ASSERT(ae != AE::WILDCARD); //RFC 6126, 4.4.11: AE MUST NOT be 0
    int offset = 0;

    //set type
    dst[offset] = type;
    offset += sizeof(type);

    //set length
    dst[offset] = static_cast<uint8_t>(sizeof(ae) + sizeof(uint8_t) + sizeof(seqno) + sizeof(hopcount) + sizeof(reserved) + (2 * sizeof(uint32_t)) + ((ae == AE::WILDCARD) ? 0 : prefix.lenInBytes()));
    offset += sizeof(uint8_t);

    //copy ae
    *reinterpret_cast<uint8_t *>(dst + offset) = ae;
    offset += sizeof(ae);

    //save plen pointer
    uint8_t *plenptr = reinterpret_cast<uint8_t *>(dst + offset);
    offset += sizeof(uint8_t);

    //copy seqno
    *reinterpret_cast<uint16_t *>(dst + offset) = htons(seqno);
    offset += sizeof(seqno);

    //copy hopcount
    *reinterpret_cast<uint8_t *>(dst + offset) = hopcount;
    offset += sizeof(hopcount);

    //copy reserved
    *reinterpret_cast<uint8_t *>(dst + offset) = reserved;
    offset += sizeof(reserved);

    //copy routerid
    *reinterpret_cast<uint32_t *>(dst + offset) = htonl(routerid.getRid()[0]);
    offset += sizeof(uint32_t);
    *reinterpret_cast<uint32_t *>(dst + offset) = htonl(routerid.getRid()[1]);
    offset += sizeof(uint32_t);

    //copy prefix
    ASSERT(ae == static_cast<uint8_t>(getAeOfPrefix(prefix.getAddr())));
    offset += prefix.copyRaw(reinterpret_cast<char *>(dst + offset), plenptr);

    return offset;
}

std::string BabelSeqnoReqFtlv::str() const
{
    std::stringstream string;

    string << BabelFtlv::str();
    string << ", AE: " << AE::toStr(ae);
    string << ", Seqno: " << static_cast<unsigned int>(seqno);
    string << ", Hop count: " << static_cast<unsigned int>(hopcount);
    string << ", Router-id: " << routerid;
    string << ", Prefix: " << prefix;

    return string.str();
}
