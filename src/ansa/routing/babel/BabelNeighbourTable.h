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
* @file BabelNeighbourTable.h
* @author Vit Rek (rek@kn.vutbr.cz)
* @author Vladimir Vesely (ivesely@fit.vutbr.cz)
* @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
* @brief Babel Neighbour Table header file
* @detail Represents data structure of Babel routing neighbour
*/

#ifndef __ANSA_BABELNEIGHBOURTABLE_H_
#define __ANSA_BABELNEIGHBOURTABLE_H_

#include "inet/common/INETDefs.h"

#include "ansa/routing/babel/BabelDef.h"
#include "ansa/routing/babel/BabelInterfaceTable.h"

namespace inet {
class INET_API BabelNeighbour : public cObject
{
  protected:
    BabelInterface *interface;
    L3Address address;
    uint16_t history;
    uint16_t cost;
    uint16_t txcost;
    uint16_t expHSeqno;
    uint16_t neighHelloInterval;
    uint16_t neighIhuInterval;
    Babel::BabelTimer *neighHelloTimer;
    Babel::BabelTimer *neighIhuTimer;


  public:
    BabelNeighbour(BabelInterface *iface, const L3Address& addr,
            Babel::BabelTimer *nht, Babel::BabelTimer *nit):
        interface(iface),
        address(addr),
        history(0),
        cost(0xFFFF),
        txcost(0xFFFF),
        expHSeqno(0),
        neighHelloInterval(0),
        neighIhuInterval(0),
        neighHelloTimer(nht),
        neighIhuTimer(nit)
        {
            ASSERT(iface != NULL);
            ASSERT(nht != NULL);
            ASSERT(nit != NULL);

            neighHelloTimer->setContextPointer(this);
            neighIhuTimer->setContextPointer(this);
        }

    BabelNeighbour():
        interface(NULL),
        history(0),
        cost(0xFFFF),
        txcost(0xFFFF),
        expHSeqno(0),
        neighHelloInterval(0),
        neighIhuInterval(0),
        neighHelloTimer(NULL),
        neighIhuTimer(NULL)
        {}
    virtual ~BabelNeighbour();
    virtual std::string str() const;
    virtual std::string detailedInfo() const {return str();}
    friend std::ostream& operator<<(std::ostream& os, const BabelNeighbour& bn);

    BabelInterface *getInterface() const {return interface;}
    void setInterface(BabelInterface * iface) {interface = iface;}

    const L3Address& getAddress() const {return address;}
    void setAddress(const L3Address& addr) {address = addr;}

    uint16_t getHistory() const {return history;}
    void setHistory(uint16_t h) {history = h;}

    uint16_t getCost() const {return cost;}
    void setCost(uint16_t c) {cost = c;}

    uint16_t getTxcost() const {return txcost;}
    void setTxcost(uint16_t t) {txcost = t;}

    uint16_t getExpHSeqno() const {return expHSeqno;}
    void setExpHSeqno(uint16_t ehsn) {expHSeqno = ehsn;}

    uint16_t getNeighHelloInterval() const {return neighHelloInterval;}
    void setNeighHelloInterval(uint16_t nhi) {neighHelloInterval = nhi;}

    uint16_t getNeighIhuInterval() const {return neighIhuInterval;}
    void setNeighIhuInterval(uint16_t nii) {neighIhuInterval = nii;}

    void noteReceive() {history = (history >> 1) | 0x8000;}
    void noteLoss() {history = history >> 1;}

    bool recomputeCost();
    uint16_t computeRxcost() const;

    void resetNHTimer();
    void resetNITimer();
    void resetNHTimer(double delay);
    void resetNITimer(double delay);
    void deleteNHTimer();
    void deleteNITimer();
};




class BabelNeighbourTable
{
  protected:
    std::vector<BabelNeighbour *> neighbours;

  public:
    BabelNeighbourTable()
        {}
    virtual ~BabelNeighbourTable();

    std::vector<BabelNeighbour *>& getNeighbours() {return neighbours;}
    int getNumOfNeighOnIface(BabelInterface *iface);

    BabelNeighbour *findNeighbour(BabelInterface *iface, const L3Address& addr);
    BabelNeighbour *addNeighbour(BabelNeighbour *neigh);
    void removeNeighbour(BabelNeighbour *neigh);
    void removeNeighboursOnIface(BabelInterface *iface);
};
}
#endif
