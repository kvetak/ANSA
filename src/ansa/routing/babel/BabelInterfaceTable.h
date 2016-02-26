// The MIT License (MIT)
//
// Copyright (c) 2016 Brno University of Technology
//
//@author Vladimir Vesely (iveselyATfitDOTvutbrDOTcz)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
/**
* @file BabelInterfaceTable.h
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
* @brief Babel Interface Table header file
* @detail Represents data structure of interfaces included to Babel routing
*/

#ifndef __ANSA_BABELINTERFACETABLE_H_
#define __ANSA_BABELINTERFACETABLE_H_

#include "inet/common/INETDefs.h"

#include "ansa/routing/babel/BabelDef.h"
//#include "UDPSocket.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"
//#include "InterfaceEntry.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "ansa/routing/babel/cost/IBabelCostComputation.h"
namespace inet {
class BabelInterface : public cObject
{
  protected:
    InterfaceEntry *interface;  ///< Physical network interface
    int afsend;                 ///< Address family used by interface for sending messages
    int afdist;                 ///< Address family of distributed prefixes
    bool splitHorizon;          ///< Enable/disable split-horizon feature
    bool wired;                 ///< Interface type
    uint16_t helloSeqno;        ///< Hello sequence number
    uint16_t helloInterval;     ///< Hello interval (centiseconds)
    uint16_t updateInterval;    ///< Update interval (centiseconds)
    Babel::BabelTimer* helloTimer;     ///< Hello timer
    Babel::BabelTimer* updateTimer;    ///< Update timer
    UDPSocket* socket4;         ///< UDP socket for unicast IPv4
    UDPSocket* socket6;         ///< UDP socket for unicast IPv6
    bool enabled;               ///< Interface status
    uint16_t nominalrxcost;
    std::vector<Babel::netPrefix<L3Address> > directlyconn;
    IBabelCostComputation *ccm;


  public:
    Babel::BabelStats rxStat;
    Babel::BabelStats txStat;

    BabelInterface() :
        interface(NULL),
        afsend(Babel::AF::IPv6),
        afdist(Babel::AF::IPv6),
        splitHorizon(false),
        wired(false),
        helloSeqno(0),
        helloInterval(Babel::defval::HELLO_INTERVAL_CS),
        updateInterval(Babel::defval::UPDATE_INTERVAL_MULT * Babel::defval::HELLO_INTERVAL_CS),
        helloTimer(NULL),
        updateTimer(NULL),
        socket4(NULL),
        socket6(NULL),
        enabled(false),
        nominalrxcost(100),
        ccm(NULL)
        {}

    BabelInterface(InterfaceEntry *iface, uint16_t hellSeq) :
        interface(iface),
        afsend(Babel::AF::IPv6),
        afdist(Babel::AF::IPv6),
        splitHorizon(false),
        wired(false),
        helloSeqno(hellSeq),
        helloInterval(Babel::defval::HELLO_INTERVAL_CS),
        updateInterval(Babel::defval::UPDATE_INTERVAL_MULT * Babel::defval::HELLO_INTERVAL_CS),
        helloTimer(NULL),
        updateTimer(NULL),
        socket4(NULL),
        socket6(NULL),
        enabled(false),
        nominalrxcost(100),
        ccm(NULL)
        {
            ASSERT(iface != NULL);

            rxStat.setName(std::string(this->getIfaceName()).append("-rx").c_str());
            txStat.setName(std::string(this->getIfaceName()).append("-tx").c_str());
        }

    virtual ~BabelInterface();
    virtual std::string str() const;
    virtual std::string detailedInfo() const {return str();}
    friend std::ostream& operator<<(std::ostream& os, const BabelInterface& i)
    {
        return os << i.str();
    }

    int getInterfaceId() {return (interface) ? interface->getInterfaceId() : -1;}

    const char *getIfaceName() const {return (interface) ? interface->getName() : "-";}

    InterfaceEntry *getInterface() {return interface;}
    void setInterface(InterfaceEntry *i) {interface = i;}

    int getAfSend() {return afsend;}
    void setAfSend(int afs) {afsend = afs;}

    int getAfDist() {return afdist;}
    void setAfDist(int afd) {afdist = afd;}

    bool getSplitHorizon() {return splitHorizon;}
    void setSplitHorizon(bool sh) {splitHorizon = sh;}

    bool getWired() {return wired;}
    void setWired(bool w) {wired = w;}

    uint16_t getHSeqno() {return helloSeqno;}
    void setHSeqno(uint16_t s) {helloSeqno = s;}
    uint16_t getIncHSeqno() {return ++helloSeqno;}

    uint16_t getNominalRxcost() {return nominalrxcost;}
    void setNominalRxcost(uint16_t nrx) {nominalrxcost = nrx;}

    uint16_t getHInterval() {return helloInterval;}
    void setHInterval(uint16_t i) {helloInterval = i;}

    uint16_t getUInterval() {return updateInterval;}
    void setUInterval(uint16_t i) {updateInterval = i;}

    Babel::BabelTimer* getHTimer() {return helloTimer;}
    void setHTimer(Babel::BabelTimer* t) {helloTimer = t;}

    Babel::BabelTimer* getUTimer() {return updateTimer;}
    void setUTimer(Babel::BabelTimer* t) {updateTimer = t;}

    UDPSocket* getSocket4() {return socket4;}
    void setSocket4(UDPSocket* s) {socket4 = s;}

    UDPSocket* getSocket6() {return socket6;}
    void setSocket6(UDPSocket* s) {socket6 = s;}

    bool getEnabled() {return enabled;}
    void setEnabled(bool e) {enabled = e;}

    void addDirectlyConn(const Babel::netPrefix<L3Address>& pre);
    const std::vector<Babel::netPrefix<L3Address> >& getDirectlyConn() const {return directlyconn;}

    IBabelCostComputation* getCostComputationModule() {return ccm;}
    void setCostComputationModule(IBabelCostComputation* cm) {ccm = cm;}


    void resetHTimer();
    void resetHTimer(double delay);
    void resetUTimer();
    void resetUTimer(double delay);

    void deleteHTimer();
    void deleteUTimer();

};




/**
 * TODO - Generated class
 */
class BabelInterfaceTable
{
  protected:
    std::vector<BabelInterface *> interfaces;

  public:
    virtual ~BabelInterfaceTable();

    std::vector<BabelInterface *>& getInterfaces() {return interfaces;}
    BabelInterface * findInterfaceById(const int ifaceId);
    BabelInterface * addInterface(BabelInterface * iface);
    void removeInterface(BabelInterface * iface);
    void removeInterface(int ifaceId);
    std::string printStats();

};
}
#endif
