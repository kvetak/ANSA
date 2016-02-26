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
* @file BabelDef.h
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
* @brief Babel definitions header file
* @detail Defines constants and useful methods
*/

#ifndef BABELDEF_H_
#define BABELDEF_H_

#include <iostream>
#include <iomanip>
#include <string>
//#include "L3Address.h"
#include "inet/networklayer/common/L3Address.h"
//#include "cmessage.h"
//#include "simtime_t.h"

#include "ansa/routing/babel/BabelMessage_m.h"

//#define BABEL_DEBUG     // Print debug informations
//#define NDEBUG        // Disable ASSERTs
namespace inet {
namespace Babel
{

namespace defval
{
const uint8_t MAGIC = 42;
const uint8_t VERSION = 2;

const int PORT = 6696;
const L3Address MCASTG6 = IPv6Address("ff02::1:6");
const L3Address MCASTG4 = IPv4Address("224.0.0.111");

//Default intervals (in CENTIseconds!!!)
const uint16_t HELLO_INTERVAL_CS = 400;
const uint16_t HELLO_INTERVAL_WIRE_CS = 2000;
const uint16_t IHU_INTERVAL_MULT = 3;
const double IHU_HOLD_INTERVAL_MULT = 3.5;      ///< IHU hold interval coefficient (multiplied by received IHU interval)
const double ROUTE_EXPIRY_INTERVAL_MULT = 3.5;  ///< Route expiry interval coefficient (multiplied by received Update interval)
const uint16_t UPDATE_INTERVAL_MULT = 4;        ///< Update interval coefficient (multiplied by hello interval)
const double SOURCE_GC_INTERVAL = 180.0;        ///< Source Garbage-collection timer
//const double PENSR_RESEND_INTERVAL = 180.0;        ///< TODO - zjistit u babeld

const unsigned int BUFFER_MT_DIVISOR = 4;       ///< Maximum-buffer time divisor (hello interval is divided by this number)
const double BUFFER_GC_INTERVAL = 300.0;        ///< Buffer Garbage-collection timer

//const uint16_t ACKREQ_INTERVAL_CS = HELLO_INTERVAL_CS / 2;  //TODO - zjistit jak je to u babeld

//TODO - tip k optimalizaci - predpocitat intervaly v sekundach (double) - ted se pri kazden resetovani timeru prepocitava CS na S - plati pouze pro nektere (Babel intervaly musi by v CS)

const int RESEND_NUM = 3;                   ///< How many times try to resend message for receive ACK
const uint8_t SEQNUMREQ_HOPCOUNT = 127;     ///< Maximal number of forwarding SeqnoRequest TLV
const uint16_t NOM_RXCOST_WIRED = 96;       ///< Nominal Rxcost on wired links
const uint16_t NOM_RXCOST_WIRELESS = 256;   ///< Nominal Rxcost on wireless links

//const_simtime_t HELLO_INTERVAL = 4.0; //simtime_t(4, SIMTIME_S);
//const_simtime_t IHU_INTERVAL = 12.0; //simtime_t(12, SIMTIME_S);
//const_simtime_t UPDATE_INTERVAL = 16.0; //simtime_t(16, SIMTIME_S);
}

/**
 * Convert centiseconds to seconds
 */
template <typename T>
inline double CStoS(T cs)
{
    return static_cast<double>(cs) / 100.0;
}

/**
 * Compute number of bytes needed for save information of plen bits
 */
template <typename T>
inline int bitsToBytesLen(T plen)
{
    return static_cast<int>(ceil(static_cast<double>(plen) / 8.0));
}

int getAeOfAddr(const L3Address& addr);
int getAeOfPrefix(const L3Address& prefix);

int copyRawAddr(const IPv4Address& addr, char *dst, uint8_t *aedst=NULL);
int copyRawAddr(const IPv6Address& addr, char *dst, uint8_t *aedst=NULL);
int copyRawAddr(const L3Address& addr, char *dst, uint8_t *aedst=NULL);

L3Address readRawAddr(uint8_t ae, char *src);

//uint16_t computeRxcostKoutofj(uint16_t history, uint16_t nominalrxcost, unsigned int k=2, unsigned int j=3);
//uint16_t koutofj(uint16_t history, uint16_t nominalrxcost, uint16_t txcost, unsigned int k=2, unsigned int j=3);

//uint16_t computeRxcostEtx(uint16_t history, uint16_t nominalrxcost);
//uint16_t etx(uint16_t history, uint16_t nominalrxcost, uint16_t txcost);

const uint16_t COST_INF = 0xFFFF;                       ///< Cost infinity
const uint32_t LINK_LOCAL_PREFIX = 0xFE800000;          ///< First 32 bits of IPv6 Link-local prefix
const uint16_t HISTORY_LEN = sizeof(uint16_t) * 8;      ///< Size of bit vector maintaining history of receiving Hello TLVs


const int IPV4_HEADER_SIZE = 20;        ///< Size of IPv4 datagram header
const int IPV6_HEADER_SIZE = 40;        ///< Size of IPv6 datagram header
const int UDP_HEADER_SIZE = 8;          ///< Size of UDP packet header
const int BABEL_HEADER_SIZE = 4;        ///< Size of Babel message header


const int USE_ACK = -1;                 ///< Send TLV with ACK request
const double SEND_NOW = 0.0;            ///< Send TLV without buffering
const double SEND_URGENT = 0.2;         ///< Send TLV with short buffering
const double SEND_BUFFERED = DBL_MAX;   ///< Send TLV with buffering


inline bool isLinkLocal64(const IPv6Address& addr)
{
    return (addr.words()[0] == LINK_LOCAL_PREFIX && addr.words()[1] == 0);
}

inline uint16_t plusmod16(uint16_t a, int b)
{
    return ((a + b) & 0xFFFF);
}

inline int16_t minusmod16(uint16_t a, uint16_t b)
{
    return ((a - b) & 0xFFFF);
}


/**
 * @return  0 if equals, 1 if a > b, and -1 if a < b
 */
inline int comparemod16(uint16_t a, uint16_t b)
{
    if(a == b)
    {
        return 0;
    }
    else
    {
        return ((b - a) & 0x8000) ? 1 : -1;
    }
}
/*
template <typename T>
inline T roughly(T value, double variance=0.25)
{
    ASSERT(variance > 0.0 && variance <= 1.0);

    return value * uniform(1.0 - variance, 1.0 + variance);
}
*/



/*
// TODO v nekterych porovnanich se lisi s comparemod16 - napr. a = 0xffff; b = 0xefff;
inline int mycomparemod16(uint16_t a, uint16_t b)
{
    if(a == b)
    {
        return 0;
    }
    else if((a < b && b - a > (2^(16 - 1))) || (a > b && a - b < (2^(16 - 1))))
    {
        return 1;
    }
    else if((a < b && b - a < (2^(16 - 1))) || (a > b && a - b > (2^(16 - 1))))
    {
        return -1;
    }

}
*/

/**
 * Types of timer
 */
struct timerT
{
    enum
    {
        HELLO = 1,
        UPDATE,
        BUFFER,
        BUFFERGC,
        TOACKRESEND,
        NEIGHHELLO,
        NEIGHIHU,
        ROUTEEXPIRY,
        ROUTEBEFEXPIRY,
        SOURCEGC,
        SRRESEND
    };

    static std::string toStr(int timerT);
};

typedef cMessage BabelTimer;



/*class INET_API BabelTimer : public cMessage
{
//TODO - dodelat konstruktory
    virtual std::string detailedInfo() const
    {
        if(getContextPointer())
        {
            cObject *obj = dynamic_cast<cObject *>(getContextPointer());
            if(obj)
            {
                return obj->detailedInfo();
            }
        }
        return std::string("");
    }
};
*/


void resetTimer(BabelTimer *timer, double delay);
void deleteTimer(BabelTimer **timer);

/**
 * Address families used on interfaces
 */
struct AF
{
    enum
    {
        NONE = 0,
        IPvX = 1,       ///< Both IPv4 and IPv6
        IPv4 = 4,       ///< IPv4 only
        IPv6 = 6        ///< IPv6 only
    };

    static std::string toStr(int af);
};

/**
 * Address encoding
 */
struct AE
{
    enum
    {
        WILDCARD = 0,   ///< Wildcard
        IPv4 = 1,       ///< IPv4
        IPv6 = 2,       ///< IPv6
        LLIPv6 = 3      ///< link-local IPv6
    };

    static int maxLen(int ae);
    static int toAF(int ae);
    static std::string toStr(int ae);
};

/**
 * TLV types
 */
struct tlvT
{
    enum
    {
        PAD1 = 0,
        PADN = 1,
        ACKREQ = 2,
        ACK = 3,
        HELLO = 4,
        IHU = 5,
        ROUTERID = 6,
        NEXTHOP = 7,
        UPDATE = 8,
        ROUTEREQ = 9,
        SEQNOREQ = 10
    };

    static std::string toStr(int tlvtype);
};

typedef struct bstats
{
    cStdDev messages;
    cStdDev tlv[tlvT::SEQNOREQ + 1];

    bstats(const char* n=NULL)
    {
        if(n) setName(n);
    }

    std::string str() const;
    void setName(const char *name);
} BabelStats;

class rid
{
protected:
    uint32_t id[2];

    void copy(const rid& other);

public:
    rid()
    {
        id[0] = 0;
        id[1] = 0;
    };

    rid(uint32_t h, uint32_t l)
    {
        id[0] = h;
        id[1] = l;
    }

    rid(const IPv6Address& a)
    {
        id[0] = a.words()[2];
        id[1] = a.words()[3];
    }
    rid(const rid& other)
    {
        copy(other);
    }
    virtual ~rid() {}

    rid& operator=(const rid& other)
    {
        if (this==&other) return *this;
        copy(other);
        return *this;
    }

    friend bool operator==(const rid& l, const rid& r)
    {
        return (l.id[0] == r.id[0]) && (l.id[1] == r.id[1]);
    }

    friend bool operator!=(const rid& l, const rid& r)
    {
        return !(l == r);
    }

    friend std::ostream& operator<<(std::ostream& os, const rid& r)
    {
        return os << r.str();
    }

    std::string str() const;
    const uint32_t* getRid() const {return id;}
    void setRid(uint32_t h, uint32_t l) {id[0] = h; id[1] = l;}
    void setRid(const IPv6Address& a) {id[0] = a.words()[2]; id[1] = a.words()[3];}
};


class routeDistance
{
protected:
    uint16_t seqno;
    uint16_t metric;

    void copy(const routeDistance& other);

public:
    routeDistance():
            seqno(0),
            metric(0)
            {}

    routeDistance(uint16_t s, uint16_t m):
            seqno(s),
            metric(m)
            {}

    routeDistance(const routeDistance& other)
            {
                copy(other);
            }


    virtual ~routeDistance() {}

    routeDistance& operator=(const routeDistance& other)
    {
        if (this==&other) return *this;
        copy(other);
        return *this;
    }
    friend std::ostream& operator<<(std::ostream& os, const routeDistance& dis)
    {
        return os << dis.str();
    }
    friend bool operator==(const routeDistance& l, const routeDistance& r)
    {
        return (l.seqno == r.seqno) && (l.metric == r.metric);
    }

    friend bool operator!=(const routeDistance& l, const routeDistance& r)
    {
        return !(l == r);
    }

    friend bool operator<(const routeDistance& l, const routeDistance& r)
    {
        return comparemod16(l.seqno, r.seqno) == 1 || (l.seqno == r.seqno && l.metric < r.metric);
    }
    friend bool operator>=(const routeDistance& l, const routeDistance& r)
    {
        return !(l < r);
    }

    friend bool operator>(const routeDistance& l, const routeDistance& r)
    {
        return comparemod16(l.seqno, r.seqno) == -1 || (l.seqno == r.seqno && l.metric > r.metric);
    }
    friend bool operator<=(const routeDistance& l, const routeDistance& r)
    {
        return !(l > r);
    }

    std::string str() const;

    uint16_t getMetric() const {return metric;}
    void setMetric(uint16_t m) {metric = m;}

    uint16_t getSeqno() const {return seqno;}
    void setSeqno(uint16_t s) {seqno = s;}

};


template<typename IPAddress>
class netPrefix
{
  protected:
    IPAddress addr;
    uint8_t len;

    void copy(const netPrefix<IPAddress>& other);

  public:
    netPrefix();
    netPrefix(IPAddress a, uint8_t plen) {set(a, plen);}
    netPrefix(uint8_t ae, char *prefix, uint8_t plen) {set(ae, prefix, plen);}
    netPrefix(uint8_t ae, char *prefix, uint8_t plen, uint8_t omitted, netPrefix<IPAddress> *prevprefix)
    {
        set(ae, prefix, plen, omitted, prevprefix);
    }
    netPrefix(const netPrefix<IPAddress>& other)
    {
        copy(other);
    }


    netPrefix<IPAddress>& operator=(const netPrefix<IPAddress>& other)
    {
        if (this==&other) return *this;
        copy(other);
        return *this;
    }
    friend std::ostream& operator<<(std::ostream& os, const netPrefix& np)
    {
        os << np.str();
        return os;
    }
    friend bool operator==(const netPrefix<IPAddress>& l, const netPrefix<IPAddress>& r)
    {
        return (l.addr == r.addr) && (l.len == r.len);
    }
    friend bool operator!=(const netPrefix<IPAddress>& l, const netPrefix<IPAddress>& r)
    {
        return !(l == r);
    }

    virtual ~netPrefix() {}

    void set(IPAddress a, uint8_t plen);
    void set(uint8_t ae, char *prefix, uint8_t plen);
    void set(uint8_t ae, char *prefix, uint8_t plen, uint8_t omitted, netPrefix<IPAddress> *prevprefix);

    std::string str() const;
    uint8_t getLen() const {return len;}
    void setLen(uint8_t l) {len = l;}

    const IPAddress& getAddr() const {return addr;}
    void setAddr(IPAddress a) {addr = a;}

    int lenInBytes() const {return bitsToBytesLen(len);}
    int copyRaw(char *dst, uint8_t *plendst, int toomit=0) const;
    //int copyCompressedRaw(char *dst, uint8_t *aedst, uint8_t *plendst, uint8_t *omitteddst, netPrefix<IPAddress> prevprefix) const;
    int bytesToOmit(const netPrefix<IPAddress>& prevprefix) const;
};

}

/**
 * Babel packet class
 */
class INET_API BabelMessage : public BabelMessage_Base
{
  private:
    void copy(const BabelMessage& other);

  public:
    BabelMessage(const char *name=NULL, int kind=0) : BabelMessage_Base(name,kind) {}
    BabelMessage(const BabelMessage& other) : BabelMessage_Base(other) {copy(other);}
    BabelMessage& operator=(const BabelMessage& other) {if (this==&other) return *this; BabelMessage_Base::operator=(other); copy(other); return *this;}
    virtual BabelMessage *dup() const {return new BabelMessage(*this);}
    // ADD CODE HERE to redefine and implement pure virtual functions from BabelMessage_Base
    friend std::ostream& operator<<(std::ostream& os, const BabelMessage& m);
    virtual std::string info() const;
    virtual std::string detailedInfo() const;


    std::string carriedTlvs(bool withlen=false) const;
    std::string carriedTlvsDetails() const;
    void countStats(Babel::BabelStats *stat) const;
    virtual void setBodyArraySize(unsigned int size);
    virtual char *getBody() const;
    virtual void setBody(const char *data, unsigned int size);
    virtual void addToBody(const char *data, unsigned int size);
    virtual int getNextTlv(int offset) const;
};


}
#endif /* BABELDEF_H_ */
