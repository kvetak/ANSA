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
* @file BabelTopologyTable.h
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
* @brief Babel Topology table header file
* @detail Represents data structure for saving all known routes
*/

#ifndef BABELTOPOLOGYTABLE_H_
#define BABELTOPOLOGYTABLE_H_


#include "inet/common/INETDefs.h"

#include "ansa/routing/babel/BabelDef.h"
#include "ansa/routing/babel/BabelNeighbourTable.h"

namespace inet {
class INET_API BabelRoute : public cObject
{
protected:
    Babel::netPrefix<L3Address> prefix;
    Babel::rid originator;
    BabelNeighbour *neighbour;
    Babel::routeDistance rdistance;
    L3Address nexthop;
    bool selected;
    uint16_t updateInterval;
    Babel::BabelTimer *expiryTimer;
    Babel::BabelTimer *befExpiryTimer;
    void *rtentry;
public:
    BabelRoute():
        neighbour(NULL),
        selected(false),
        updateInterval(0),
        expiryTimer(NULL),
        befExpiryTimer(NULL),
        rtentry(NULL)
        {}
    BabelRoute(const Babel::netPrefix<L3Address>& pre, BabelNeighbour *neigh,
            const Babel::rid& orig, const Babel::routeDistance& dist, const L3Address& nh,
            uint16_t ui, Babel::BabelTimer *et, Babel::BabelTimer *bet):
        prefix(pre),
        originator(orig),
        neighbour(neigh),
        rdistance(dist),
        nexthop(nh),
        selected(false),
        updateInterval(ui),
        expiryTimer(et),
        befExpiryTimer(bet),
        rtentry(NULL)
        {
            if(expiryTimer != NULL)
            {
                expiryTimer->setContextPointer(this);
            }

            if(befExpiryTimer != NULL)
            {
                befExpiryTimer->setContextPointer(this);
            }
        }
    virtual ~BabelRoute();
    virtual std::string str() const;
    virtual std::string detailedInfo() const {return str();}
    friend std::ostream& operator<<(std::ostream& os, const BabelRoute& br);

    const Babel::netPrefix<L3Address>& getPrefix() const {return prefix;}
    void setPrefix(Babel::netPrefix<L3Address>& p) {prefix = p;}

    const Babel::rid& getOriginator() const {return originator;}
    void setOriginator(const Babel::rid& o) {originator = o;}

    BabelNeighbour *getNeighbour() const {return neighbour;}
    void setNeighbour(BabelNeighbour *n) {neighbour = n;}

    Babel::routeDistance& getRDistance() {return rdistance;}
    const Babel::routeDistance& getRDistance() const {return rdistance;}
    void setRDistance(const Babel::routeDistance& rd) {rdistance = rd;}

    const L3Address& getNextHop() const {return nexthop;}
    void setNextHop(const L3Address& nh) {nexthop = nh;}

    bool getSelected() const {return selected;}
    void setSelected(bool s) {selected = s;}

    uint16_t getUpdateInterval() const {return updateInterval;}
    void setUpdateInterval(uint16_t ui) {updateInterval = ui;}

    Babel::BabelTimer* getETimer() const {return expiryTimer;}
    void setETimer(Babel::BabelTimer* et) {expiryTimer = et;}

    Babel::BabelTimer* getBETimer() const {return befExpiryTimer;}
    void setBETimer(Babel::BabelTimer* bet) {befExpiryTimer = bet;}

    void *getRTEntry() const {return rtentry;}
    void setRTEntry(void *rte) {rtentry = rte;}

    uint16_t metric() const;

    void resetETimer();
    void resetETimer(double delay);
    void resetBETimer();
    void resetBETimer(double delay);
    void deleteETimer();
    void deleteBETimer();
};


class BabelTopologyTable {
protected:
    std::vector<BabelRoute *> routes;
public:
    BabelTopologyTable()
    {}
    virtual ~BabelTopologyTable();
    std::vector<BabelRoute *>& getRoutes() {return routes;}

    BabelRoute *findRoute(const Babel::netPrefix<L3Address>& p, BabelNeighbour *n, const Babel::rid& orig);
    BabelRoute *findRoute(const Babel::netPrefix<L3Address>& p, BabelNeighbour *n);
    BabelRoute *findSelectedRoute(const Babel::netPrefix<L3Address>& p);
    bool containShorterCovRoute(const Babel::netPrefix<L3Address>& p);
    BabelRoute *findRouteNotNH(const Babel::netPrefix<L3Address>& p, const L3Address& nh);
    BabelRoute *addRoute(BabelRoute *route);
    bool retractRoutesOnIface(BabelInterface *iface);
    bool removeRoute(BabelRoute *route);
    void removeRoutes();
};
}
#endif /* BABELTOPOLOGYTABLE_H_ */
