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
* @file BabelFtlv.h
* @author Vit Rek (rek@kn.vutbr.cz)
* @author Vladimir Vesely (ivesely@fit.vutbr.cz)
* @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
* @brief Babel FTLVs header file
* @detail Includes classes represent Full TLVs for internal representation
*/

#ifndef BABELFTLV_H_
#define BABELFTLV_H_

#include <ostream>
#include "stdint.h"
#include "ansa/routing/babel/BabelDef.h"
//#include "BabelTopologyTable.h"
namespace inet {
class INET_API BabelFtlv {
  protected:
    uint8_t type;
    int sendNum;

    void copy(const BabelFtlv& other);

  public:
    BabelFtlv() :   type(0),
                    sendNum(0)
                    {};

    BabelFtlv(uint8_t t, int sn=1) :
                    type(t),
                    sendNum(sn)
                    {};

    BabelFtlv(const BabelFtlv& other) {copy(other);}
    BabelFtlv& operator=(const BabelFtlv& other);
    virtual BabelFtlv *dup() const = 0;
    //friend std::ostream& operator<<(std::ostream& os, const BabelFtlv& i);
    virtual ~BabelFtlv();

    virtual int rawTlvLength() const;
    virtual int copyRawTlv(char *dst) const = 0;
    virtual std::string str() const;

    uint8_t getType() const {return type;}
    void setType(uint8_t t) {type = t;}

    int getSendNum() const {return sendNum;}
    void setSendNum(int sn) {sendNum = sn;}
    int decSendNum() {return --sendNum;}

};

class INET_API BabelPad1Ftlv : public BabelFtlv {
  protected:

    void copy(const BabelPad1Ftlv& other);

  public:
    BabelPad1Ftlv(int sn=1) :
                    BabelFtlv(Babel::tlvT::PAD1, sn)
                    {};

    BabelPad1Ftlv(const BabelPad1Ftlv& other) {copy(other);}
    BabelPad1Ftlv& operator=(const BabelPad1Ftlv& other);
    virtual BabelPad1Ftlv *dup() const {return new BabelPad1Ftlv(*this);}
    virtual ~BabelPad1Ftlv();

    virtual int rawTlvLength() const;
    virtual int copyRawTlv(char *dst) const;
    virtual std::string str() const;

};

class BabelPadNFtlv : public BabelFtlv {
  protected:
    uint8_t n;
    void copy(const BabelPadNFtlv& other);

  public:
    BabelPadNFtlv() : BabelFtlv(Babel::tlvT::PADN),
                    n(0)
                    {};

    BabelPadNFtlv(uint8_t npad, int sn=1) :
                    BabelFtlv(Babel::tlvT::PADN, sn),
                    n(npad)
                    {};

    BabelPadNFtlv(const BabelPadNFtlv& other) {copy(other);}
    BabelPadNFtlv& operator=(const BabelPadNFtlv& other);
    virtual BabelPadNFtlv *dup() const {return new BabelPadNFtlv(*this);}
    virtual ~BabelPadNFtlv();

    virtual int rawTlvLength() const;
    virtual int copyRawTlv(char *dst) const;
    virtual std::string str() const;

};

class INET_API BabelAckReqFtlv : public BabelFtlv {
  protected:
    uint16_t reserved;
    uint16_t nonce;
    uint16_t interval;

    void copy(const BabelAckReqFtlv& other);

  public:
    BabelAckReqFtlv() : BabelFtlv(Babel::tlvT::ACKREQ),
                    reserved(0),
                    nonce(0),
                    interval(0)
                    {};

    BabelAckReqFtlv(uint16_t n, uint16_t i, int sn=1) :
                    BabelFtlv(Babel::tlvT::ACKREQ, sn),
                    reserved(0),
                    nonce(n),
                    interval(i)
                    {};

    BabelAckReqFtlv(const BabelAckReqFtlv& other) {copy(other);}
    BabelAckReqFtlv& operator=(const BabelAckReqFtlv& other);
    virtual BabelAckReqFtlv *dup() const {return new BabelAckReqFtlv(*this);}
    //friend std::ostream& operator<<(std::ostream& os, const BabelAckReqFtlv& i);
    virtual ~BabelAckReqFtlv();

    virtual int rawTlvLength() const;
    virtual int copyRawTlv(char *dst) const;
    virtual std::string str() const;

    uint16_t getReserved() const {return reserved;}
    void setReserved(uint16_t r) {reserved = r;}

    uint16_t getNonce() const {return nonce;}
    void setNonce(uint16_t n) {nonce = n;}

    uint16_t getInterval() const {return interval;}
    void setInterval(uint16_t i) {interval = i;}
};

class INET_API BabelAckFtlv : public BabelFtlv {
  protected:
    uint16_t nonce;

    void copy(const BabelAckFtlv& other);

  public:
    BabelAckFtlv() : BabelFtlv(Babel::tlvT::ACK),
                    nonce(0)
                    {};

    BabelAckFtlv(uint16_t n, int sn=1) :
                    BabelFtlv(Babel::tlvT::ACK, sn),
                    nonce(n)
                    {};

    BabelAckFtlv(const BabelAckFtlv& other) {copy(other);}
    BabelAckFtlv& operator=(const BabelAckFtlv& other);
    virtual BabelAckFtlv *dup() const {return new BabelAckFtlv(*this);}
    //friend std::ostream& operator<<(std::ostream& os, const BabelAckFtlv& i);
    virtual ~BabelAckFtlv();

    virtual int rawTlvLength() const;
    virtual int copyRawTlv(char *dst) const;
    virtual std::string str() const;

    uint16_t getNonce() const {return nonce;}
    void setNonce(uint16_t n) {nonce = n;}
};

class INET_API BabelHelloFtlv : public BabelFtlv {
  protected:
    uint16_t reserved;
    uint16_t seqno;
    uint16_t interval;

    void copy(const BabelHelloFtlv& other);

  public:
    BabelHelloFtlv() : BabelFtlv(Babel::tlvT::HELLO),
                    reserved(0),
                    seqno(0),
                    interval(0)
                    {};

    BabelHelloFtlv(uint16_t seqn, uint16_t i, int sn=1) :
                    BabelFtlv(Babel::tlvT::HELLO, sn),
                    reserved(0),
                    seqno(seqn),
                    interval(i)
                    {};

    BabelHelloFtlv(const BabelHelloFtlv& other) {copy(other);}
    BabelHelloFtlv& operator=(const BabelHelloFtlv& other);
    virtual BabelHelloFtlv *dup() const {return new BabelHelloFtlv(*this);}
    //friend std::ostream& operator<<(std::ostream& os, const BabelHelloFtlv& i);
    virtual ~BabelHelloFtlv();

    virtual int rawTlvLength() const;
    virtual int copyRawTlv(char *dst) const;
    virtual std::string str() const;

    uint16_t getReserved() const {return reserved;}
    void setReserved(uint16_t r) {reserved = r;}

    uint16_t getSeqno() const {return seqno;}
    void setSeqno(uint16_t s) {seqno = s;}

    uint16_t getInterval() const {return interval;}
    void setInterval(uint16_t i) {interval = i;}
};

class INET_API BabelIhuFtlv : public BabelFtlv {
  protected:
    uint8_t ae;
    uint8_t reserved;
    uint16_t rxcost;
    uint16_t interval;
    L3Address address;

    void copy(const BabelIhuFtlv& other);

  public:
    BabelIhuFtlv() : BabelFtlv(Babel::tlvT::IHU),
                    ae(0),
                    reserved(0),
                    rxcost(0),
                    interval(0)
                    {};

    BabelIhuFtlv(uint8_t addressencoding, uint16_t rx, uint16_t i, const L3Address& a, int sn=1) :
                    BabelFtlv(Babel::tlvT::IHU, sn),
                    ae(addressencoding),
                    reserved(0),
                    rxcost(rx),
                    interval(i),
                    address(a)
                    {};

    BabelIhuFtlv(uint16_t rx, uint16_t i, const L3Address& a, int sn=1) :
                    BabelFtlv(Babel::tlvT::IHU, sn),
                    reserved(0),
                    rxcost(rx),
                    interval(i),
                    address(a)
                    {
                        ae = Babel::getAeOfAddr(a);
                    };

    BabelIhuFtlv(const BabelIhuFtlv& other) {copy(other);}
    BabelIhuFtlv& operator=(const BabelIhuFtlv& other);
    virtual BabelIhuFtlv *dup() const {return new BabelIhuFtlv(*this);}
    //friend std::ostream& operator<<(std::ostream& os, const BabelIhuFtlv& i);
    virtual ~BabelIhuFtlv();

    virtual int rawTlvLength() const;
    virtual int copyRawTlv(char *dst) const;
    virtual std::string str() const;

    uint8_t getAe() const {return ae;}
    void setAe(uint8_t addressencoding) {ae = addressencoding;}

    uint8_t getReserved() const {return reserved;}
    void setReserved(uint8_t r) {reserved = r;}

    uint16_t getRxcost() const {return rxcost;}
    void setRxcost(uint16_t r) {rxcost = r;}

    uint16_t getInterval() const {return interval;}
    void setInterval(uint16_t i) {interval = i;}

    const L3Address& getAddress() const {return address;}
    void setAddress(const L3Address& a) {address = a;}
};

class INET_API BabelRouterIdFtlv : public BabelFtlv {
  protected:
    uint16_t reserved;
    Babel::rid routerId;

    void copy(const BabelRouterIdFtlv& other);

  public:
    BabelRouterIdFtlv() : BabelFtlv(Babel::tlvT::ROUTERID),
                    reserved(0)
                    {};

    BabelRouterIdFtlv(const Babel::rid& r, int sn=1) :
                    BabelFtlv(Babel::tlvT::ROUTERID, sn),
                    reserved(0),
                    routerId(r)
                    {};

    BabelRouterIdFtlv(const BabelRouterIdFtlv& other) {copy(other);}
    BabelRouterIdFtlv& operator=(const BabelRouterIdFtlv& other);
    virtual BabelRouterIdFtlv *dup() const {return new BabelRouterIdFtlv(*this);}
    //friend std::ostream& operator<<(std::ostream& os, const BabelRouterIdFtlv& i);
    virtual ~BabelRouterIdFtlv();

    virtual int rawTlvLength() const;
    virtual int copyRawTlv(char *dst) const;
    virtual std::string str() const;

    uint16_t getReserved() const {return reserved;}
    void setReserved(uint16_t r) {reserved = r;}

    const Babel::rid& getRouterId() const {return routerId;}
    void setRouterId(const Babel::rid& r) {routerId = r;}
};

class INET_API BabelNextHopFtlv : public BabelFtlv {
  protected:
    uint8_t ae;
    uint8_t reserved;
    L3Address nextHop;

    void copy(const BabelNextHopFtlv& other);

  public:
    BabelNextHopFtlv() : BabelFtlv(Babel::tlvT::NEXTHOP),
                    ae(0),
                    reserved(0)
                    {};

    BabelNextHopFtlv(uint8_t addressencoding, const L3Address& nh, int sn=1) :
                    BabelFtlv(Babel::tlvT::NEXTHOP, sn),
                    ae(addressencoding),
                    reserved(0),
                    nextHop(nh)
                    {};

    BabelNextHopFtlv(const L3Address& nh, int sn=1) :
                    BabelFtlv(Babel::tlvT::NEXTHOP, sn),
                    reserved(0),
                    nextHop(nh)
                    {
                        ae = Babel::getAeOfAddr(nh);
                    };

    BabelNextHopFtlv(const BabelNextHopFtlv& other) {copy(other);}
    BabelNextHopFtlv& operator=(const BabelNextHopFtlv& other);
    virtual BabelNextHopFtlv *dup() const {return new BabelNextHopFtlv(*this);}
    //friend std::ostream& operator<<(std::ostream& os, const BabelNextHopFtlv& i);
    virtual ~BabelNextHopFtlv();

    virtual int rawTlvLength() const;
    virtual int copyRawTlv(char *dst) const;
    virtual std::string str() const;

    uint8_t getAe() const {return ae;}
    void setAe(uint8_t addressencoding) {ae = addressencoding;}

    uint8_t getReserved() const {return reserved;}
    void setReserved(uint8_t r) {reserved = r;}

    const L3Address& getNextHop() const {return nextHop;}
    void setNextHop(const L3Address& nh) {nextHop = nh;}
};


class INET_API BabelUpdateFtlv : public BabelFtlv {
  protected:
    uint8_t ae;
    uint8_t flags;
    uint16_t interval;
    Babel::netPrefix<L3Address> prefix;
    Babel::routeDistance dist;
    Babel::rid routerId;
    L3Address nextHop;

    void copy(const BabelUpdateFtlv& other);

  public:
    BabelUpdateFtlv() : BabelFtlv(Babel::tlvT::UPDATE),
                    ae(0),
                    flags(0),
                    interval(0)
                    {};

    BabelUpdateFtlv(uint8_t addressencoding, uint16_t i, const Babel::netPrefix<L3Address>& p,
            const Babel::routeDistance& d, const Babel::rid& r, const L3Address& nh, int sn=1) :
                    BabelFtlv(Babel::tlvT::UPDATE, sn),
                    ae(addressencoding),
                    flags(0),
                    interval(i),
                    prefix(p),
                    dist(d),
                    routerId(r),
                    nextHop(nh)
                    {};

    BabelUpdateFtlv(uint16_t i, const Babel::netPrefix<L3Address>& p,
            const Babel::routeDistance& d, const Babel::rid& r, const L3Address& nh, int sn=1) :
                    BabelFtlv(Babel::tlvT::UPDATE, sn),
                    flags(0),
                    interval(i),
                    prefix(p),
                    dist(d),
                    routerId(r),
                    nextHop(nh)
                    {
                        ae = Babel::getAeOfPrefix(p.getAddr());
                    };
/*    BabelUpdateFtlv(const BabelRoute& route, uint16_t i, int sn=1) :
                    BabelFtlv(Babel::tlvT::UPDATE, sn),
                    flags(0),
                    interval(i),
                    prefix(route.getPrefix()),
                    dist(Babel::routeDistance(route.getRDistance().getSeqno(), route.metric())),
                    routerId(route.getOriginator()),
                    nextHop(route.getNextHop())
                    {};
*/
    BabelUpdateFtlv(const BabelUpdateFtlv& other) {copy(other);}
    BabelUpdateFtlv& operator=(const BabelUpdateFtlv& other);
    virtual BabelUpdateFtlv *dup() const {return new BabelUpdateFtlv(*this);}
    //friend std::ostream& operator<<(std::ostream& os, const BabelUpdateFtlv& i);
    virtual ~BabelUpdateFtlv();

    virtual int rawTlvLength() const;
    virtual int rawTlvLength(const Babel::netPrefix<L3Address>& prevprefix) const;
    virtual int copyRawTlv(char *dst) const;
    virtual int copyRawTlv(char *dst, Babel::netPrefix<L3Address> *prevprefix) const;
    virtual std::string str() const;

    uint8_t getAe() const {return ae;}
    void setAe(uint8_t addressencoding) {ae = addressencoding;}

    uint8_t getFlags() const {return flags;}
    void setFlags(uint8_t f) {flags = f;}

    uint16_t getInterval() const {return interval;}
    void setInterval(uint16_t i) {interval = i;}

    const Babel::netPrefix<L3Address>& getPrefix() const {return prefix;}
    void setPrefix(const Babel::netPrefix<L3Address>& p) {prefix = p; ae = Babel::getAeOfPrefix(p.getAddr());}

    Babel::routeDistance getDistance() const {return dist;}
    void setDistance(Babel::routeDistance d) {dist = d;}

    const Babel::rid& getRouterId() const {return routerId;}
    void setRouterId(const Babel::rid& r) {routerId = r;}

    const L3Address& getNextHop() const {return nextHop;}
    void setNextHop(const L3Address& nh) {nextHop = nh;}

};

class INET_API BabelRouteReqFtlv : public BabelFtlv {
  protected:
    uint8_t ae;
    Babel::netPrefix<L3Address> prefix;

    void copy(const BabelRouteReqFtlv& other);

  public:
    BabelRouteReqFtlv() : BabelFtlv(Babel::tlvT::ROUTEREQ),
                    ae(Babel::AE::WILDCARD)
                    {};

    BabelRouteReqFtlv(uint8_t addressencoding, const Babel::netPrefix<L3Address>& p, int sn=1) :
                    BabelFtlv(Babel::tlvT::ROUTEREQ, sn),
                    ae(addressencoding),
                    prefix(p)
                    {};

    BabelRouteReqFtlv(const Babel::netPrefix<L3Address>& p, int sn=1) :
                    BabelFtlv(Babel::tlvT::ROUTEREQ, sn),
                    prefix(p)
                    {
                        ae = Babel::getAeOfPrefix(p.getAddr());
                    };

    BabelRouteReqFtlv(const BabelRouteReqFtlv& other) {copy(other);}
    BabelRouteReqFtlv& operator=(const BabelRouteReqFtlv& other);
    virtual BabelRouteReqFtlv *dup() const {return new BabelRouteReqFtlv(*this);}
    //friend std::ostream& operator<<(std::ostream& os, const BabelRouteReqFtlv& i);
    virtual ~BabelRouteReqFtlv();

    virtual int rawTlvLength() const;
    virtual int copyRawTlv(char *dst) const;
    virtual std::string str() const;

    uint8_t getAe() const {return ae;}
    void setAe(uint8_t addressencoding) {ae = addressencoding;}

    const Babel::netPrefix<L3Address>& getPrefix() const {return prefix;}
    void setPrefix(const Babel::netPrefix<L3Address>& p) {prefix = p;}
};

class INET_API BabelSeqnoReqFtlv : public BabelFtlv {
  protected:
    uint8_t ae;
    uint16_t seqno;
    uint8_t hopcount;
    uint8_t reserved;
    Babel::rid routerid;
    Babel::netPrefix<L3Address> prefix;

    void copy(const BabelSeqnoReqFtlv& other);

  public:
    BabelSeqnoReqFtlv() : BabelFtlv(Babel::tlvT::SEQNOREQ),
                    ae(0),
                    seqno(0),
                    hopcount(0),
                    reserved(0)
                    {};

    BabelSeqnoReqFtlv(uint8_t addressencoding, uint16_t seqn,
            uint8_t hc, const Babel::rid& r,
            const Babel::netPrefix<L3Address>& p, int sn=1) :
                    BabelFtlv(Babel::tlvT::SEQNOREQ, sn),
                    ae(addressencoding),
                    seqno(seqn),
                    hopcount(hc),
                    reserved(0),
                    routerid(r),
                    prefix(p)
                    {};

    BabelSeqnoReqFtlv(uint16_t seqn, uint8_t hc, const Babel::rid& r,
            const Babel::netPrefix<L3Address>& p, int sn=1) :
                    BabelFtlv(Babel::tlvT::SEQNOREQ, sn),
                    seqno(seqn),
                    hopcount(hc),
                    reserved(0),
                    routerid(r),
                    prefix(p)
                    {
                        ae = Babel::getAeOfPrefix(p.getAddr());
                    };

    BabelSeqnoReqFtlv(const BabelSeqnoReqFtlv& other) {copy(other);}
    BabelSeqnoReqFtlv& operator=(const BabelSeqnoReqFtlv& other);
    virtual BabelSeqnoReqFtlv *dup() const {return new BabelSeqnoReqFtlv(*this);}
    //friend std::ostream& operator<<(std::ostream& os, const BabelSeqnoReqFtlv& i);
    virtual ~BabelSeqnoReqFtlv();

    virtual int rawTlvLength() const;
    virtual int copyRawTlv(char *dst) const;
    virtual std::string str() const;

    uint8_t getAe() const {return ae;}
    void setAe(uint8_t addressencoding) {ae = addressencoding;}

    uint16_t getSeqno() const {return seqno;}
    void setSeqno(uint16_t s) {seqno = s;}

    uint8_t getHopcount() const {return hopcount;}
    void setHopcount(uint8_t hc) {hopcount = hc;}

    const Babel::rid& getRouterId() const {return routerid;}
    void setRouterId(const Babel::rid& r) {routerid = r;}

    const Babel::netPrefix<L3Address>& getPrefix() const {return prefix;}
    void setPrefix(const Babel::netPrefix<L3Address>& p) {prefix = p;}
};
}
#endif /* BABELFTLV_H_ */
