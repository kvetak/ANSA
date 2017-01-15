//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
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
* @file BabelDef.cc
* @author Vit Rek (rek@kn.vutbr.cz)
* @author Vladimir Vesely (ivesely@fit.vutbr.cz)
* @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
* @brief Babel definitions
* @detail Defines constants and useful methods
*/
#include "ansa/routing/babel/BabelDef.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#include "winsock2.h"  // htonl, ntohl, ...
#else
#include <netinet/in.h>  // htonl, ntohl, ...
#endif
namespace inet {
using namespace Babel;

Register_Class(BabelMessage);


/**
 * Copy of object
 */
void BabelMessage::copy(const BabelMessage& other)
{
//    this->magic = other.magic;
//    this->version = other.version;
//    this->length = other.length;
//    delete [] this->body;
//    this->body = (other.body_arraysize==0) ? NULL : new char[other.body_arraysize];
//    body_arraysize = other.body_arraysize;
//    for (unsigned int i=0; i<body_arraysize; i++)
//        this->body[i] = other.body[i];
    this->setMagic(other.getMagic());
    this->setVersion(other.getVersion());
    this->setLength(other.getLength());
    this->setBodyArraySize(other.getBodyArraySize());
    this->setBody(other.getBody(),other.getBodyArraySize());
}

/**
 * Sets body array size
 *
 * Automatically sets length in header and control info
 * @param   size    Size of body
 */
void BabelMessage::setBodyArraySize(unsigned int size)
{
//    char *body_var2 = (size==0) ? NULL : new char[size];
//    unsigned int sz = body_arraysize < size ? body_arraysize : size;
//    for (unsigned int i=0; i<sz; i++)
//        body_var2[i] = this->body[i];
//    for (unsigned int i=sz; i<size; i++)
//        body_var2[i] = 0;
//    body_arraysize = size;
//    delete [] this->body;
//    this->body = body_var2;
//
//    // change length of body in header
//    this->length = body_arraysize;
//
//    // change length of packet in control info
//    this->setByteLength(body_arraysize + sizeof(magic) + sizeof(version) + sizeof(length));
    BabelMessage_Base::setBodyArraySize(size);

    // change length of body in header
    this->setLength(this->getBodyArraySize());

    // change length of packet in control info
    this->setByteLength(this->getBodyArraySize() + sizeof(getMagic()) + sizeof(getVersion()) + sizeof(getLength()));
}

/**
 * Sets content to body of message
 *
 * @param   data     Pointer to data
 * @param   size        Size of data
 */
void BabelMessage::setBody(const char *data, unsigned int size)
{
//    setBodyArraySize(size);
//    for(unsigned int i=0; i<size; ++i)
//    {
//        body[i] = data[i];
//    }
    setBodyArraySize(size);
    for(unsigned int i=0; i<size; ++i)
    {
        this->getBody()[i] = data[i];
    }
}


/**
 * Add content to end of body of message
 *
 * @param   content     Pointer to data
 * @param   size        Size of data
 */
void BabelMessage::addToBody(const char *data, unsigned int datasize)
{
//    unsigned int oldsize = getBodyArraySize();
//
//    setBodyArraySize(oldsize + datasize);
//    for(unsigned int i=0; i<datasize; ++i)
//    {
//        body[oldsize + i] = data[i];
//    }
    unsigned int oldsize = getBodyArraySize();

    setBodyArraySize(oldsize + datasize);
    for(unsigned int i=0; i<datasize; ++i)
    {
        this->getBody()[oldsize + i] = data[i];
    }
}

/**
 * Finds next tlv in body
 *
 * @param   offset  Offset from start of body in bytes
 * @return  Offset of next tlv from start of body
 */
int BabelMessage::getNextTlv(int offset) const
{
    int bodysize = static_cast<int>(getLength()); // parse only part of size specified in header field -> any following data are silently ignored
    int nextoffset = -1;

    if(static_cast<int>(getBodyArraySize()) < bodysize)
    {//message with body shorter than specified in header field -> parsing is not safe
        EV << "Babel message body length error - body is shorter than specified! Body ignored." << endl;
        return -1;
    }

    if(offset < 0)
    {// no previous tlv -> find first tlv
        nextoffset = 0; //first tlv is on top
    }
    else
    {// previous tlv specified
        if(bodysize <= offset)
        {// there is no type
            EV << "Babel message body out of bounds (body length: " << bodysize << ", real body size: " << getBodyArraySize() << ") - bad offset: " << offset << endl;
            return -1;
        }

        if(static_cast<uint8_t>(this->getBody()[offset]) == tlvT::PAD1)
        {// Pad1 TLV type - without length
            nextoffset = offset + 1;
        }
        else
        {// other TLV types - with length
            if(bodysize <= (offset + 1))
            {// there is no length in body
                EV << "Babel message body out of bounds (body length: " << bodysize << ", real body size: " << getBodyArraySize() << ") - bad TLV format" << endl;
                return -1;
            }

            // offset_of_next is: offset_of_previous + length_of_previous
            nextoffset = offset + sizeof(uint8_t) + sizeof(uint8_t) + static_cast<uint8_t>(this->getBody()[offset + 1]);
        }
    }

    if(bodysize > nextoffset)
    {// body contains next TLV
        return nextoffset;
    }

    // no next TLV
    return -1;
}

/**
 * @return  Pointer to body of message
 */

char *BabelMessage::getBody() const
{
    return body;
}

/**
 * Translates numerical value of Address Family to name
 *
 * @param   af  AF as number
 * @return  Name of AF as string
 */
std::string AF::toStr(int af)
{
    std::string afstr;

    switch(af)
    {
    case AF::NONE:
        afstr = "NONE";
        break;
    case AF::IPvX:
        afstr = "IPvX";
        break;
    case AF::IPv4:
        afstr = "IPv4";
        break;
    case AF::IPv6:
        afstr = "IPv6";
        break;
    default:
        afstr = "<unknown-af>";
        break;
    }

    return afstr;
}

/**
 * Determines AE of address
 *
 * @param   addr    Address as L3Address
 * @return  AE of address
 */
int Babel::getAeOfAddr(const L3Address& addr)
{
    if(addr.getType()==L3Address::IPv6)
    {
        if(isLinkLocal64(addr.toIPv6()))
        {
            return AE::LLIPv6;
        }
        else
        {
            return AE::IPv6;
        }
    }
    else
    {//IPv4
        return AE::IPv4;
    }
}

/**
 * Determines AE of prefix
 *
 * @param   Prefix as L3Address
 * @return  AE of prefix
 */
int Babel::getAeOfPrefix(const L3Address& prefix)
{
    if(prefix.getType()==L3Address::IPv6)
    {// IPv6
        return AE::IPv6;
    }
    else
    {//IPv4
        return AE::IPv4;
    }
}

/**
 * Copies IPv4 address to memory
 *
 * @param   addr    Address to copy
 * @param   dst     Destination memory
 * @param   aedst   Destination of AE field
 * @return  Number copied bytes
 */
int Babel::copyRawAddr(const IPv4Address& addr, char *dst, uint8_t *aedst)
{
    ASSERT(dst != NULL);

    *reinterpret_cast<uint8_t *>(dst)     = static_cast<uint8_t>(addr.getDByte(0));
    *reinterpret_cast<uint8_t *>(dst + 1) = static_cast<uint8_t>(addr.getDByte(1));
    *reinterpret_cast<uint8_t *>(dst + 2) = static_cast<uint8_t>(addr.getDByte(2));
    *reinterpret_cast<uint8_t *>(dst + 3) = static_cast<uint8_t>(addr.getDByte(3));

    if(aedst != NULL)
    {
        *aedst = AE::IPv4;
    }

    return AE::maxLen(AE::IPv4);
}

/**
 * Copies IPv6 address to memory
 *
 * @param   addr    Address to copy
 * @param   dst     Destination memory
 * @param   aedst   Destination of AE field
 * @return  Number copied bytes
 */
int Babel::copyRawAddr(const IPv6Address& addr, char *dst, uint8_t *aedst)
{
    ASSERT(dst != NULL);

    unsigned int bytes = 0;
    unsigned int shift = 0;

    if(isLinkLocal64(addr))
    {
        bytes = AE::maxLen(AE::LLIPv6);
        shift = 2;

        if(aedst != NULL)
        {
            *aedst = AE::LLIPv6;
        }
    }
    else
    {
        bytes = AE::maxLen(AE::IPv6);

        if(aedst != NULL)
        {
            *aedst = AE::IPv6;
        }
    }

    for(unsigned int j = 0; (j * sizeof(uint32_t)) < bytes; ++j)
    {
       for(unsigned int i = 0; i < sizeof(uint32_t) && ((j * sizeof(uint32_t)) + i) < bytes; ++i)
       {
           *reinterpret_cast<uint8_t *>(dst + (j * sizeof(uint32_t)) + i) = static_cast<uint8_t>((addr.words()[j + shift] >> (24 - (8 * i))) & 0xFF);
       }
    }

    return bytes;
}

/**
 * Copies address to memory
 *
 * @param   addr    Address to copy
 * @param   dst     Destination memory
 * @param   aedst   Destination of AE field
 * @return  Number copied bytes
 */
int Babel::copyRawAddr(const L3Address& addr, char *dst, uint8_t *aedst)
{
    return (addr.getType()==L3Address::IPv6) ? copyRawAddr(addr.toIPv6(), dst, aedst) : copyRawAddr(addr.toIPv4(), dst, aedst);
}

/**
 * Read address from memory
 *
 * @param   ae  AE of address
 * @param   src Address in memory
 * @return  Address in L3Address
 */
L3Address Babel::readRawAddr(uint8_t ae, char *src)
{
    ASSERT(src != NULL);

    uint32_t a[4] = { 0, 0, 0, 0 };
    L3Address addr;
    unsigned int bytes = 0;

    switch(ae)
    {
    case AE::WILDCARD:
       break;
    case AE::IPv4:

        bytes = AE::maxLen(AE::IPv4);

        for(unsigned int i = 0; i < bytes; ++i)
        {
            a[0] |= static_cast<uint8_t>(src[i]) << (24 - (8 * i));
        }

        addr.set(IPv4Address(a[0]));
       break;
    case AE::IPv6:

        bytes = AE::maxLen(AE::IPv6);

        for(unsigned int j = 0; j * sizeof(uint32_t) < bytes; ++j)
        {
            for(unsigned int i = 0; i < sizeof(uint32_t) && ((j * sizeof(uint32_t)) + i) < bytes; ++i)
            {
                a[j] |= static_cast<uint8_t>(src[(j * sizeof(uint32_t)) + i]) << (24 - (8 * i));
            }
        }
        addr.set(IPv6Address(a[0], a[1], a[2], a[3]));
       break;
    case AE::LLIPv6:

        bytes = AE::maxLen(AE::LLIPv6);

        a[0] = LINK_LOCAL_PREFIX;
        a[1] = 0;

        for(unsigned int j = 0; (j * sizeof(uint32_t)) < bytes; ++j)
        {
            for(unsigned int i = 0; i < sizeof(uint32_t) && ((j * sizeof(uint32_t)) + i) < bytes; ++i)
            {
                a[j + 2] |= static_cast<uint8_t>(src[(j * sizeof(uint32_t)) + i]) << (24 - (8 * i));
            }
        }
        addr.set(IPv6Address(a[0], a[1], a[2], a[3]));
       break;
    default:
        break;
    }

   return addr;
}
/**
 * Determines maximal
 *
 * @return  Lenght in bytes
 */
int AE::maxLen(int ae)
{
    int len = -1;

    switch(ae)
    {
    case AE::WILDCARD:
        len = 0;
        break;
    case AE::IPv4:
        len = 4;
        break;
    case AE::IPv6:
        len = 16;
        break;
    case AE::LLIPv6:
        len = 8;
        break;
    default:
        len = -1;
        break;
    }

    return len;
}

/**
 * Convert AE to AF
 *
 * @param   ae  Address encoding
 * @return  Address family
 */
int AE::toAF(int ae)
{
    int af = -1;

    switch(ae)
    {
    case AE::WILDCARD:
        af = -1;
        break;
    case AE::IPv4:
        af = AF::IPv4;
        break;
    case AE::IPv6:
        af = AF::IPv6;
        break;
    case AE::LLIPv6:
        af = AF::IPv6;
        break;
    default:
        af = -1;
        break;
    }

    return af;
}

/**
 * Translates AE to name
 *
 * @param   ae  Address encoding
 * @return  Name of address encoding as string
 */
std::string AE::toStr(int ae)
{
    std::string aestr;

    switch(ae)
    {
    case AE::WILDCARD:
        aestr = "WILDCARD";
        break;
    case AE::IPv4:
        aestr = "IPv4";
        break;
    case AE::IPv6:
        aestr = "IPv6";
        break;
    case AE::LLIPv6:
        aestr = "LL-IPv6";
        break;
    default:
        aestr = "<unknown-ae>";
        break;
    }

    return aestr;
}

std::ostream& operator<<(std::ostream& os, const BabelMessage& m)
{
    os << m.carriedTlvs();
    return os;
}

/**
 * List of carried tlvs
 *
 * @param   withlen Print length?
 * @return  List of TLVs as string
 */
std::string BabelMessage::carriedTlvs(bool withlen) const
{
    std::stringstream out;

    char *msgbody = getBody();
    for(int tlvoffset = getNextTlv(-1); tlvoffset != -1; tlvoffset = getNextTlv(tlvoffset))
    {
        uint8_t tlvtype = static_cast<uint8_t>(msgbody[tlvoffset]);
        out << ((tlvoffset != 0) ? ", " : "") << tlvT::toStr(tlvtype);

        if(withlen && tlvtype != tlvT::PAD1)
        {
            out << " (" << static_cast<unsigned int>(msgbody[tlvoffset + 1]) << ")";
        }
    }
    return out.str();
}

/**
 * Count statistical informations
 *
 * @param   stat    Data structure to which save info
 */
void BabelMessage::countStats(Babel::BabelStats *stat) const
{
    stat->messages.collect(getByteLength());


    char *msgbody = getBody();
    for(int tlvoffset = getNextTlv(-1); tlvoffset != -1; tlvoffset = getNextTlv(tlvoffset))
    {
        uint8_t tlvtype = static_cast<uint8_t>(msgbody[tlvoffset]);
        if(tlvtype <= tlvT::SEQNOREQ)
        {
            if(tlvtype == tlvT::PAD1)
            {// pad0 tlv has no length field
                stat->tlv[tlvtype].collect(1);
            }
            else
            {
                stat->tlv[tlvtype].collect(2 + static_cast<uint8_t>(msgbody[tlvoffset + 1]));
            }
        }
    }
}

/**
 * List of carried tlvs withi detailed info
 *
 * @param   withlen Print length?
 * @return  List of TLVs as string
 */
std::string BabelMessage::carriedTlvsDetails() const
{
    std::stringstream out;

    char *msgbody = getBody();
    for(int tlvoffset = getNextTlv(-1); tlvoffset != -1; tlvoffset = getNextTlv(tlvoffset))
    {
        uint8_t tlvtype = static_cast<uint8_t>(msgbody[tlvoffset]);
        uint8_t tlvlength = ((tlvtype != tlvT::PAD1) ? static_cast<uint8_t>(msgbody[tlvoffset + 1]) : 0);
        int af = -1;


        if(tlvoffset != 0)
        {
            out << endl;
        }


        if(tlvtype == tlvT::PAD1)
        {
            out << tlvT::toStr(tlvtype);
        }
        else
        {
            out << tlvT::toStr(tlvtype) << ": ";
            out << "Length=" << static_cast<unsigned int>(tlvlength);
        }

        switch(tlvtype)
        {
        case tlvT::PAD1:
            break;
        case tlvT::PADN:
            break;
        case tlvT::ACKREQ:
            out << ", Nonce=" << static_cast<unsigned int>(ntohs(*reinterpret_cast<uint16_t *>(msgbody + tlvoffset + 4)));
            out << ", Interval=" << static_cast<unsigned int>(ntohs(*reinterpret_cast<uint16_t *>(msgbody + tlvoffset + 6)));
            break;
        case tlvT::ACK:
            out << ", Nonce=" << static_cast<unsigned int>(ntohs(*reinterpret_cast<uint16_t *>(msgbody + tlvoffset + 2)));
            break;
        case tlvT::HELLO:
            out << ", Seqno=" << static_cast<unsigned int>(ntohs(*reinterpret_cast<uint16_t *>(msgbody + tlvoffset + 4)));
            out << ", Interval=" << static_cast<unsigned int>(ntohs(*reinterpret_cast<uint16_t *>(msgbody + tlvoffset + 6)));
            break;
        case tlvT::IHU:
            out << ", AE=" << static_cast<unsigned int>(*reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 2));
            af = AE::toAF(*reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 2));
            out << ", Rxcost=" << static_cast<unsigned int>(ntohs(*reinterpret_cast<uint16_t *>(msgbody + tlvoffset + 4)));
            out << ", Interval=" << static_cast<unsigned int>(ntohs(*reinterpret_cast<uint16_t *>(msgbody + tlvoffset + 6)));

            out << ", Address=";
            for(uint8_t i = 7; i <= tlvlength; ++i)
            {
                if(i != 7)
                {
                    out << ".";
                }

                if(af == AF::IPv4)
                {
                    out << static_cast<unsigned int>(*reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 1 + i));
                }
                else
                {
                    out << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(*reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 1 + i));
                }

            }
            out << std::dec;
            break;
        case tlvT::ROUTERID:

            out << ", Router-id=";
            out << std::hex;
            out << std::setw(4) << std::setfill('0') << static_cast<unsigned int>(ntohs(*reinterpret_cast<uint16_t *>(msgbody + tlvoffset + 4))) << ":";
            out << std::setw(4) << std::setfill('0') << static_cast<unsigned int>(ntohs(*reinterpret_cast<uint16_t *>(msgbody + tlvoffset + 6))) << ":";
            out << std::setw(4) << std::setfill('0') << static_cast<unsigned int>(ntohs(*reinterpret_cast<uint16_t *>(msgbody + tlvoffset + 8))) << ":";
            out << std::setw(4) << std::setfill('0') << static_cast<unsigned int>(ntohs(*reinterpret_cast<uint16_t *>(msgbody + tlvoffset + 10)));
            out << std::dec;
            break;
        case tlvT::NEXTHOP:
            out << ", AE=" << static_cast<unsigned int>(*reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 2));
            af = AE::toAF(*reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 2));

            out << ", Next-hop=";
            for(uint8_t i = 3; i <= tlvlength; ++i)
            {
                if(i != 3)
                {
                    out << ".";
                }

                if(af == AF::IPv4)
                {
                    out << static_cast<unsigned int>(*reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 1 + i));
                }
                else
                {
                    out << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(*reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 1 + i));
                }
            }
            out << std::dec;
            break;
        case tlvT::UPDATE:
            out << ", AE=" << static_cast<unsigned int>(*reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 2));
            af = AE::toAF(*reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 2));
            out << ", Flags=" << std:: hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(*reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 3)) << std::dec;
            out << ", Plen=" << static_cast<unsigned int>(*reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 4));
            out << ", Omitted=" << static_cast<unsigned int>(*reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 5));
            out << ", Interval=" << static_cast<unsigned int>(ntohs(*reinterpret_cast<uint16_t *>(msgbody + tlvoffset + 6)));
            out << ", Seqno=" << static_cast<unsigned int>(ntohs(*reinterpret_cast<uint16_t *>(msgbody + tlvoffset + 8)));
            out << ", Metric=" << static_cast<unsigned int>(ntohs(*reinterpret_cast<uint16_t *>(msgbody + tlvoffset + 10)));

            out << ", Prefix=";
            for(uint8_t i = 11; i <= tlvlength; ++i)
            {
                if(i != 11)
                {
                    out << ".";
                }

                if(af == AF::IPv4)
                {
                    out << static_cast<unsigned int>(*reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 1 + i));
                }
                else
                {
                    out << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(*reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 1 + i));
                }
            }
            out << std::dec;
            break;
        case tlvT::ROUTEREQ:
            out << ", AE=" << static_cast<unsigned int>(*reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 2));
            af = AE::toAF(*reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 2));
            out << ", Plen=" << static_cast<unsigned int>(*reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 3));

            out << ", Prefix=";
            out << std::hex;
            for(uint8_t i = 3; i <= tlvlength; ++i)
            {
                if(i != 3)
                {
                    out << ".";
                }

                if(af == AF::IPv4)
                {
                    out << static_cast<unsigned int>(*reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 1 + i));
                }
                else
                {
                    out << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(*reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 1 + i));
                }
            }
            out << std::dec;
            break;
        case tlvT::SEQNOREQ:
            out << ", AE=" << static_cast<unsigned int>(*reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 2));
            af = AE::toAF(*reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 2));
            out << ", Plen=" << static_cast<unsigned int>(*reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 3));
            out << ", Seqno=" << static_cast<unsigned int>(ntohs(*reinterpret_cast<uint16_t *>(msgbody + tlvoffset + 4)));
            out << ", Hop-count=" << static_cast<unsigned int>(*reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 6));

            out << ", Router-id=";
            out << std::hex;
            out << std::setw(4) << std::setfill('0') << static_cast<unsigned int>(ntohs(*reinterpret_cast<uint16_t *>(msgbody + tlvoffset + 8))) << ":";
            out << std::setw(4) << std::setfill('0') << static_cast<unsigned int>(ntohs(*reinterpret_cast<uint16_t *>(msgbody + tlvoffset + 10))) << ":";
            out << std::setw(4) << std::setfill('0') << static_cast<unsigned int>(ntohs(*reinterpret_cast<uint16_t *>(msgbody + tlvoffset + 12))) << ":";
            out << std::setw(4) << std::setfill('0') << static_cast<unsigned int>(ntohs(*reinterpret_cast<uint16_t *>(msgbody + tlvoffset + 14)));
            out << std::dec;


            out << ", Prefix=";
            for(uint8_t i = 15; i <= tlvlength; ++i)
            {
                if(i != 15)
                {
                    out << ".";
                }

                if(af == AF::IPv4)
                {
                    out << static_cast<unsigned int>(*reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 1 + i));
                }
                else
                {
                    out << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(*reinterpret_cast<uint8_t *>(msgbody + tlvoffset + 1 + i));
                }
            }
            out << std::dec;
            break;
        default:
            break;
        }

    }
    return out.str();
}

std::string BabelMessage::info() const
{
    return carriedTlvs();
}

std::string BabelMessage::detailedInfo() const
{
    return carriedTlvsDetails();
}

/**
 * Resets timer
 *
 * @param   timer   Timer to reset
 * @param   delay   Delay in seconds
 */
void Babel::resetTimer(BabelTimer *timer, double delay)
{
    if(timer != NULL)
    {// existing timer -> get his owner (cSimpleModule)

        //if is scheduled, get his sender module, otherwise get owner module
        cSimpleModule *owner = dynamic_cast<cSimpleModule *>((timer->isScheduled()) ? timer->getSenderModule() : timer->getOwner());
        if(owner != NULL)
        {// owner is cSimpleModule object -> can call his methods
            owner->scheduleAt(simTime() + delay, owner->cancelEvent(timer));
        }
    }
}

/**
 * Cancell and delete timer
 *
 * @param   timer   Pointer to pointer to timer
 * @note    Sets timer to NULL
 */
void Babel::deleteTimer(BabelTimer **timer)
{
    if(timer != NULL && (*timer) != NULL)
    {// existing timer -> get his owner (cSimpleModule)

        //if is scheduled, get his sender module, otherwise get owner module
        cSimpleModule *owner = dynamic_cast<cSimpleModule *>(((*timer)->isScheduled()) ? (*timer)->getSenderModule() : (*timer)->getOwner());
        if(owner != NULL)
        {// owner is cSimpleModule object -> can call his methods
            owner->cancelAndDelete((*timer));
            (*timer) = NULL;
        }
    }
}

/**
 * Translate TLV type to name
 *
 * @param   tlvtype Type of TLV
 * @return  Name of TLV as string
 */
std::string tlvT::toStr(int tlvtype)
{
    std::string tlvstr;

    switch(tlvtype)
    {
    case tlvT::PAD1:
        tlvstr = "PAD1";
        break;
    case tlvT::PADN:
        tlvstr = "PADN";
        break;
    case tlvT::ACKREQ:
        tlvstr = "ACKREQ";
        break;
    case tlvT::ACK:
        tlvstr = "ACK";
        break;
    case tlvT::HELLO:
        tlvstr = "HELLO";
        break;
    case tlvT::IHU:
        tlvstr = "IHU";
        break;
    case tlvT::ROUTERID:
        tlvstr = "ROUTERID";
        break;
    case tlvT::NEXTHOP:
        tlvstr = "NEXTHOP";
        break;
    case tlvT::UPDATE:
        tlvstr = "UPDATE";
        break;
    case tlvT::ROUTEREQ:
        tlvstr = "ROUTEREQ";
        break;
    case tlvT::SEQNOREQ:
        tlvstr = "SEQNOREQ";
        break;
    default:
        tlvstr = "<unknown-tlv>";
    }

    return tlvstr;
}

/**
 * Translates type of timer to name
 *
 * @param   timerT  Type of timer
 * @return  Name of timer as string
 */
std::string timerT::toStr(int timerT)
{
    std::string timerstr;

    switch(timerT)
    {
    case timerT::HELLO:
        timerstr = "BabelHelloT";
        break;
    case timerT::UPDATE:
        timerstr = "BabelUpdateT";
        break;
    case timerT::BUFFER:
        timerstr = "BabelBufferT";
        break;
    case timerT::BUFFERGC:
        timerstr = "BabelBufferGCT";
        break;
    case timerT::TOACKRESEND:
        timerstr = "BabelToAckResendT";
        break;
    case timerT::NEIGHHELLO:
        timerstr = "BabelNeighHelloT";
        break;
    case timerT::NEIGHIHU:
        timerstr = "BabelNeighIhuT";
        break;
    case timerT::ROUTEEXPIRY:
        timerstr = "BabelRouteExpiryT";
        break;
    case timerT::ROUTEBEFEXPIRY:
        timerstr = "BabelRouteBefExpiryT";
        break;
    case timerT::SOURCEGC:
        timerstr = "BabelSourceGCT";
        break;
    case timerT::SRRESEND:
        timerstr = "BabelSeqnoReqResendT";
        break;
    default:
        timerstr = "<unknown-timer-type>";
    }

    return timerstr;
}

/**
 * Format statistical info
 *
 * @return  Statistical informations as string
 */
std::string bstats::str() const
{
    std::stringstream string;

    string << messages.getCount() << " messages (" << messages.getSum() << " bytes total";
    if(messages.getCount() > 0) string << ", avg " << messages.getMean() << " B/msg, min: " << messages.getMin() << " B, max: " << messages.getMax() << " B";
    string << ") contained:"<< endl;

    for(int i = tlvT::PAD1; i <= tlvT::SEQNOREQ; ++i)
    {
        string << std::setw(10) << tlvT::toStr(i) << ": " << std::setw(8) << tlv[i].getCount();

        if(tlv[i].getClassName() > 0)
        {
            string << " (" << tlv[i].getSum() << " B";
            string << ", avg " << tlv[i].getMean() << " B/TLV, min: " << tlv[i].getMin() << ", max: " << tlv[i].getMax() << ")";
        }
        string << endl;
    }
    return string.str();
}

/**
 * Set name of statistics
 */
void bstats::setName(const char *name)
{
    if(name)
    {
        messages.setName(name);

        for(int i = tlvT::PAD1; i <= tlvT::SEQNOREQ; ++i)
        {
            tlv[i].setName(std::string(name).append("-").append(tlvT::toStr(i)).c_str());
        }
    }
}


void rid::copy(const rid& other)
{
    id[0] = other.id[0];
    id[1] = other.id[1];
}


std::string rid::str() const
{
    uint16_t groups[4] = {
        uint16_t(id[0]>>16), uint16_t(id[0]&0xffff), uint16_t(id[1]>>16), uint16_t(id[1]&0xffff)
    };

    std::stringstream string;
    string << std::hex;
    string << std::setw(4) << std::setfill('0') << groups[0] << ":";
    string << std::setw(4) << std::setfill('0') << groups[1] << ":";
    string << std::setw(4) << std::setfill('0') << groups[2] << ":";
    string << std::setw(4) << std::setfill('0') << groups[3];

    return string.str();
}

void routeDistance::copy(const routeDistance& other)
{
    seqno = other.seqno;
    metric = other.metric;
}

std::string routeDistance::str() const
{
    std::stringstream string;
    string << "(" << seqno << ", " << metric << ")";

    return string.str();
}

namespace Babel
{

template<typename IPAddress>
void netPrefix<IPAddress>::copy(const netPrefix<IPAddress>& other)
{
    addr = other.addr;
    len = other.len;
}

template<>
netPrefix<IPv4Address>::netPrefix()
{
    addr = IPv4Address();
    len = 0;
}

template<>
netPrefix<IPv6Address>::netPrefix()
{
    addr = IPv6Address();
    len = 0;
}

template<>
netPrefix<L3Address>::netPrefix()
{
    addr = L3Address(IPv6Address());   // default L3Address Constructor uses IPv4 -> force IPv6
    len = 0;
}

template<>
void netPrefix<IPv4Address>::set(IPv4Address a, uint8_t plen)
{
    ASSERT(plen >= 0 && plen <= 32);

    addr = a.doAnd(IPv4Address::makeNetmask(plen));
    len = plen;
}

template<>
void netPrefix<IPv6Address>::set(IPv6Address a, uint8_t plen)
{
    ASSERT(plen >= 0 && plen <= 128);

    addr.set(0,0,0,0);
    addr.setPrefix(a, plen);
    len = plen;
}

template<>
void netPrefix<L3Address>::set(L3Address a, uint8_t plen)
{
    ASSERT(plen >= 0 && plen <= 128);

    if(a.getType()==L3Address::IPv6)
    {
        addr.set(IPv6Address().setPrefix(a.toIPv6(), plen));
    }
    else
    {
        addr.set(a.toIPv4().doAnd(IPv4Address::makeNetmask(plen)));
    }

    len = plen;
}

/**
 * Set prefix from memory
 *
 * @param   ae      Address encoding of prefix in memory
 * @param   prefix  Prefix in memory
 * @param   plen    Length of prefix
 *
 */
template<>
void netPrefix<IPv4Address>::set(uint8_t ae, char *prefix, uint8_t plen)
{
    ASSERT(ae == AE::IPv4);
    ASSERT(prefix != NULL);
    ASSERT(plen >= 0 && plen <= 32);


    int plenbytes = bitsToBytesLen(plen);

    uint32_t p = 0;

    for(int i = 0; i < plenbytes; ++i)
    {
        p |= static_cast<uint8_t>(prefix[i]) << (24 - (8 * i));
    }

    addr = IPv4Address(p).doAnd(IPv4Address::makeNetmask(plen));
    len = plen;
}

/**
 * Set prefix from memory
 *
 * @param   ae      Address encoding of prefix in memory
 * @param   prefix  Prefix in memory
 * @param   plen    Length of prefix
 *
 */
template<>
void netPrefix<IPv6Address>::set(uint8_t ae, char *prefix, uint8_t plen)
{
    ASSERT(ae == AE::IPv6);
    ASSERT(prefix != NULL);
    ASSERT(plen >= 0 && plen <= 128);


    unsigned int plenbytes = static_cast<unsigned int>(bitsToBytesLen(plen));

    uint32_t p[4] = { 0, 0, 0, 0 };

    for(unsigned int j = 0; j * sizeof(uint32_t) < plenbytes; ++j)
    {
        for(unsigned int i = 0; i < sizeof(uint32_t) && ((j * sizeof(uint32_t)) + i) < plenbytes; ++i)
        {
            p[j] |= static_cast<uint8_t>(prefix[(j * sizeof(uint32_t)) + i]) << (24 - (8 * i));
        }
    }

    addr.setPrefix(IPv6Address(p[0], p[1], p[2], p[3]), plen);
    len = plen;
}

/**
 * Set prefix from memory
 *
 * @param   ae      Address encoding of prefix in memory
 * @param   prefix  Prefix in memory
 * @param   plen    Length of prefix
 *
 */
template<>
void netPrefix<L3Address>::set(uint8_t ae, char *prefix, uint8_t plen)
{
    ASSERT((ae == AE::IPv4 && (plen >= 0 && plen <= 32)) || (ae == AE::IPv6 && (plen >= 0 && plen <= 128)));
    ASSERT(prefix != NULL);

    unsigned int plenbytes = static_cast<unsigned int>(bitsToBytesLen(plen));

    if(ae == AE::IPv4)
    {// IPv4 prefix
        uint32_t p = 0;

        for(unsigned int i = 0; i < plenbytes; ++i)
        {
            p |= static_cast<uint8_t>(prefix[i]) << (24 - (8 * i));
        }

        addr = IPv4Address(p).doAnd(IPv4Address::makeNetmask(plen));

    }
    else if(ae == AE::IPv6)
    {// IPv6 prefix
        uint32_t p[4] = { 0, 0, 0, 0 };

        for(unsigned int j = 0; j * sizeof(uint32_t) < plenbytes; ++j)
        {
            for(unsigned int i = 0; i < sizeof(uint32_t) && ((j * sizeof(uint32_t)) + i) < plenbytes; ++i)
            {
                p[j] |= static_cast<uint8_t>(prefix[(j * sizeof(uint32_t)) + i]) << (24 - (8 * i));

            }
        }

        addr = IPv6Address().setPrefix(IPv6Address(p[0], p[1], p[2], p[3]), plen);
    }

    len = plen;
}

/**
 * Set prefix from memory with omitted bytes
 *
 * @param   ae      Address encoding of prefix in memory
 * @param   prefix  Prefix in memory
 * @param   plen    Length of prefix
 * @param   omitted Number of omitted bytes
 * @param   prefprefix Previous prefix from wich get omitted bytes
 *
 */
template<>
void netPrefix<IPv4Address>::set(uint8_t ae, char *prefix, uint8_t plen, uint8_t omitted, netPrefix<IPv4Address> *prevprefix)
{
    ASSERT(ae == AE::IPv4);
    ASSERT(prefix != NULL);
    ASSERT(plen >= 0 && plen <= 32);
    ASSERT(omitted >= 0 && omitted <= sizeof(uint32_t));
    ASSERT(omitted == 0 || (omitted > 0 && prevprefix != NULL));


    int plenbytes = bitsToBytesLen(plen);

    uint32_t p = 0;

    for(int i = 0; i < plenbytes; ++i)
    {
        if(i < omitted)
        {// byte is omitted -> read from previous prefix
            p |= prevprefix->getAddr().getInt() & (0xFF << (24 - (8 * i)));
        }
        else
        {
            p |= static_cast<uint8_t>(prefix[i - omitted]) << (24 - (8 * i));
        }
    }

    addr = IPv4Address(p).doAnd(IPv4Address::makeNetmask(plen));
    len = plen;
}

/**
 * Set prefix from memory with omitted bytes
 *
 * @param   ae      Address encoding of prefix in memory
 * @param   prefix  Prefix in memory
 * @param   plen    Length of prefix
 * @param   omitted Number of omitted bytes
 * @param   prefprefix Previous prefix from wich get omitted bytes
 *
 */
template<>
void netPrefix<IPv6Address>::set(uint8_t ae, char *prefix, uint8_t plen, uint8_t omitted, netPrefix<IPv6Address> *prevprefix)
{
    ASSERT(ae == AE::IPv6);
    ASSERT(prefix != NULL);
    ASSERT(plen >= 0 && plen <= 128);
    ASSERT(omitted >= 0 && omitted <= (4 * sizeof(uint32_t)));
    ASSERT(omitted == 0 || (omitted > 0 && prevprefix != NULL));

    unsigned int plenbytes = static_cast<unsigned int>(bitsToBytesLen(plen));

    uint32_t p[4] = { 0, 0, 0, 0 };

    for(unsigned int j = 0; j * sizeof(uint32_t) < plenbytes; ++j)
    {
        for(unsigned int i = 0; i < sizeof(uint32_t) && ((j * sizeof(uint32_t)) + i) < plenbytes; ++i)
        {
            if(((j * sizeof(uint32_t)) + i) < omitted)
            {// byte is omitted -> read from previous prefix
                p[j] |= prevprefix->getAddr().words()[j] & (0xFF << (24 - (8 * i)));
            }
            else
            {
                p[j] |= static_cast<uint8_t>(prefix[(j * sizeof(uint32_t)) + i - omitted]) << (24 - (8 * i));
            }
        }
    }


    addr.setPrefix(IPv6Address(p[0], p[1], p[2], p[3]), plen);
    len = plen;
}

/**
 * Set prefix from memory with omitted bytes
 *
 * @param   ae      Address encoding of prefix in memory
 * @param   prefix  Prefix in memory
 * @param   plen    Length of prefix
 * @param   omitted Number of omitted bytes
 * @param   prefprefix Previous prefix from wich get omitted bytes
 *
 */
template<>
void netPrefix<L3Address>::set(uint8_t ae, char *prefix, uint8_t plen, uint8_t omitted, netPrefix<L3Address> *prevprefix)
{
    ASSERT((ae == AE::IPv4 && (plen >= 0 && plen <= 32) && (omitted >= 0 && omitted <= sizeof(uint32_t))) ||
            (ae == AE::IPv6 && (plen >= 0 && plen <= 128) && (omitted >= 0 && omitted <= (4 * sizeof(uint32_t)))));
    ASSERT(prefix != NULL);
    ASSERT(omitted == 0 || (omitted > 0 && prevprefix != NULL));

    unsigned int plenbytes = static_cast<unsigned int>(bitsToBytesLen(plen));

    if(ae == AE::IPv4)
    {// IPv4 prefix
        uint32_t p = 0;

        for(unsigned int i = 0; i < plenbytes; ++i)
        {
            if(i < omitted)
            {// byte is omitted -> read from previous prefix
                p |= prevprefix->getAddr().toIPv4().getInt() & (0xFF << (24 - (8 * i)));
            }
            else
            {
                p |= static_cast<uint8_t>(prefix[i - omitted]) << (24 - (8 * i));
            }
        }

        addr = IPv4Address(p).doAnd(IPv4Address::makeNetmask(plen));

    }
    else if(ae == AE::IPv6)
    {// IPv6 prefix
        uint32_t p[4] = { 0, 0, 0, 0 };

        for(unsigned int j = 0; j * sizeof(uint32_t) < plenbytes; ++j)
        {
            for(unsigned int i = 0; i < sizeof(uint32_t) && ((j * sizeof(uint32_t)) + i) < plenbytes; ++i)
            {
                if(((j * sizeof(uint32_t)) + i) < omitted)
                {// byte is omitted -> read from previous prefix
                    p[j] |= prevprefix->getAddr().toIPv6().words()[j] & (0xFF << (24 - (8 * i)));
                }
                else
                {
                    p[j] |= static_cast<uint8_t>(prefix[(j * sizeof(uint32_t)) + i - omitted]) << (24 - (8 * i));
                }
            }
        }

        addr = IPv6Address().setPrefix(IPv6Address(p[0], p[1], p[2], p[3]), plen);
    }

    len = plen;
}

/**
 * Copy prefix to memory
 *
 * @param   dst Destination memory
 * @param   plendst Destination of plen
 * @param   toomit  Number of bytes to omit
 * @return  Number of copied bytes
 */
template<>
int netPrefix<IPv4Address>::copyRaw(char *dst, uint8_t *plendst, int toomit) const
{

    ASSERT(dst != NULL);
    ASSERT(toomit >= 0 && static_cast<unsigned int>(toomit) <= sizeof(uint32_t));

    unsigned int i = toomit;
    unsigned int plenbytes = static_cast<unsigned int>(bitsToBytesLen(len));
    int copied = 0;


    for(; i < plenbytes; ++i)
    {
        *reinterpret_cast<uint8_t *>(dst + i - toomit) = static_cast<uint8_t>((addr.getInt() >> (24 - (8 * i))) & 0xFF);
        ++copied;
    }


    if(plendst != NULL)
    {// know where to save prefix length
        *plendst = len;
    }

    return copied;
}

/**
 * Copy prefix to memory
 *
 * @param   dst Destination memory
 * @param   plendst Destination of plen
 * @param   toomit  Number of bytes to omit
 * @return  Number of copied bytes
 */
template<>
int netPrefix<IPv6Address>::copyRaw(char *dst, uint8_t *plendst, int toomit) const
{
    ASSERT(dst != NULL);
    ASSERT(toomit >= 0 && static_cast<unsigned int>(toomit) <= (4 * sizeof(uint32_t)));

    unsigned int j = toomit / sizeof(uint32_t);
    unsigned int i = toomit % sizeof(uint32_t);
    unsigned int plenbytes = static_cast<unsigned int>(bitsToBytesLen(len));
    int copied = 0;


    for(; j * sizeof(uint32_t) < plenbytes; ++j, i = 0)
    {
       for(; i < sizeof(uint32_t) && ((j * sizeof(uint32_t)) + i) < plenbytes; ++i)
       {
           *reinterpret_cast<uint8_t *>(dst + (j * sizeof(uint32_t)) + i - toomit) = static_cast<uint8_t>((addr.words()[j] >> (24 - (8 * i))) & 0xFF);
           ++copied;
       }
    }

    if(plendst != NULL)
    {// know where to save prefix length
        *plendst = len;
    }

    return copied;
}

/**
 * Copy prefix to memory
 *
 * @param   dst Destination memory
 * @param   plendst Destination of plen
 * @param   toomit  Number of bytes to omit
 * @return  Number of copied bytes
 */
template<>
int netPrefix<L3Address>::copyRaw(char *dst, uint8_t *plendst, int toomit) const
{
    ASSERT(dst != NULL);
    ASSERT(toomit >= 0 && ( ( (addr.getType()==L3Address::IPv6) && static_cast<unsigned int>(toomit) <= (4 * sizeof(uint32_t)))
                          ||
                            (!(addr.getType()==L3Address::IPv6) && static_cast<unsigned int>(toomit) <=      sizeof(uint32_t)))
                          );

    uint32_t ad[4];
    if (addr.getType()==L3Address::IPv6) {
        ad[0] = addr.toIPv6().words()[0];
        ad[1] = addr.toIPv6().words()[1];
        ad[3] = addr.toIPv6().words()[2];
        ad[3] = addr.toIPv6().words()[3];
    }
    else {
        ad[0] = addr.toIPv4().getInt();
        ad[1] = 0;
        ad[3] = 0;
        ad[3] = 0;
    }
    unsigned int j = toomit / sizeof(uint32_t);
    unsigned int i = toomit % sizeof(uint32_t);
    unsigned int plenbytes = static_cast<unsigned int>(bitsToBytesLen(len));
    int copied = 0;


    for(; j * sizeof(uint32_t) < plenbytes; ++j, i = 0)
    {
       for(; i < sizeof(uint32_t) && ((j * sizeof(uint32_t)) + i) < plenbytes; ++i)
       {
           *reinterpret_cast<uint8_t *>(dst + (j * sizeof(uint32_t)) + i - toomit) = static_cast<uint8_t>((ad[j] >> (24 - (8 * i))) & 0xFF);
           ++copied;
       }
    }

    if(plendst != NULL)
    {// know where to save prefix length
        *plendst = len;
    }

    return copied;
}

template<typename IPAddress>
std::string netPrefix<IPAddress>::str() const
{
    std::stringstream string;
    string << addr << "/" << static_cast<int>(len);

    return string.str();
}


/**
 * Count bytes to omit
 *
 * @param   prevprefix   Previous prefix
 * @return  Number of bytes to omit
 */
template<>
int netPrefix<IPv4Address>::bytesToOmit(const netPrefix<IPv4Address>& prevprefix) const
{
    // maximally compare length of shorter prefix bytes
    unsigned int plenbytes = static_cast<unsigned int>((lenInBytes() < prevprefix.lenInBytes()) ? lenInBytes() : prevprefix.lenInBytes());
    unsigned int i = 0;
    int toomit = 0;

    ASSERT(plenbytes <= 4);

    for(i = 0; i < plenbytes; ++i)
    {
        if(addr.getDByte(i) != prevprefix.getAddr().getDByte(i))
        {// different bytes -> end
            return toomit;
        }
        ++toomit;
    }

    return toomit;
}

/**
 * Count bytes to omit
 *
 * @param   prevprefix   Previous prefix
 * @return  Number of bytes to omit
 */
template<>
int netPrefix<IPv6Address>::bytesToOmit(const netPrefix<IPv6Address>& prevprefix) const
{
    // maximally compare length of shorter prefix bytes
    unsigned int plenbytes = static_cast<unsigned int>((lenInBytes() < prevprefix.lenInBytes()) ? lenInBytes() : prevprefix.lenInBytes());
    unsigned int j = 0;
    unsigned int i = 0;
    int toomit = 0;

    ASSERT(plenbytes <= 16);

    if(isLinkLocal64(addr) && isLinkLocal64(prevprefix.getAddr()))
    {// both are link-local -> compare only last 64 bits
        j = 2;
    }

    for(; j * sizeof(uint32_t) < plenbytes; ++j)
    {
        for(i = 0; i < sizeof(uint32_t) && ((j * sizeof(uint32_t)) + i) < plenbytes; ++i)
        {
            if(((addr.words()[j] >> (24 - (8 * i))) & 0xFF) != ((prevprefix.getAddr().words()[j] >> (24 - (8 * i))) & 0xFF))
            {
                return toomit;
            }

            ++toomit;
        }
    }


    return toomit;
}

/**
 * Count bytes to omit
 *
 * @param   prevprefix   Previous prefix
 * @return  Number of bytes to omit
 */
template<>
int netPrefix<L3Address>::bytesToOmit(const netPrefix<L3Address>& prevprefix) const
{
    // maximally compare length of shorter prefix bytes
    unsigned int plenbytes = static_cast<unsigned int>((lenInBytes() < prevprefix.lenInBytes()) ? lenInBytes() : prevprefix.lenInBytes());
    unsigned int j = 0;
    unsigned int i = 0;
    int toomit = 0;

    if((addr.getType()==L3Address::IPv6) != (prevprefix.getAddr().getType()==L3Address::IPv6))
    {// different AF
        return 0;
    }

    ASSERT(    (  addr.getType()==L3Address::IPv6  && plenbytes <= 16)
            || (!(addr.getType()==L3Address::IPv6) && plenbytes <= 4)
          );

    if(addr.getType()==L3Address::IPv6 && isLinkLocal64(addr.toIPv6()) && isLinkLocal64(prevprefix.getAddr().toIPv6()))
    {// both are link-local IPv6 -> compare only last 64 bits
        j = 2;
    }

    uint32_t ad[4];
    if (addr.getType()==L3Address::IPv6) {
        ad[0] = addr.toIPv6().words()[0];
        ad[1] = addr.toIPv6().words()[1];
        ad[3] = addr.toIPv6().words()[2];
        ad[3] = addr.toIPv6().words()[3];
    }
    else {
        ad[0] = addr.toIPv4().getInt();
        ad[1] = 0;
        ad[3] = 0;
        ad[3] = 0;
    }
    uint32_t prev[4];
    if (addr.getType()==L3Address::IPv6) {
        prev[0] = prevprefix.getAddr().toIPv6().words()[0];
        prev[1] = prevprefix.getAddr().toIPv6().words()[1];
        prev[3] = prevprefix.getAddr().toIPv6().words()[2];
        prev[3] = prevprefix.getAddr().toIPv6().words()[3];
    }
    else {
        prev[0] = prevprefix.getAddr().toIPv4().getInt();
        prev[1] = 0;
        prev[3] = 0;
        prev[3] = 0;
    }
    for(; j * sizeof(uint32_t) < plenbytes; ++j)
    {
        for(i = 0; i < sizeof(uint32_t) && ((j * sizeof(uint32_t)) + i) < plenbytes; ++i)
        {
            if(((ad[j] >> (24 - (8 * i))) & 0xFF) != ((prev[j] >> (24 - (8 * i))) & 0xFF))
            {
                return toomit;
            }

            ++toomit;
        }
    }

    return toomit;
}

template class netPrefix<IPv4Address>;
template class netPrefix<IPv6Address>;
template class netPrefix<L3Address>;

}
}
