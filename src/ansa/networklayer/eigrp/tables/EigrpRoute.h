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

/*
 * EigrpRoute.h
 *
 *  Created on: Feb 17, 2014
 *      Author: honza
 */

#ifndef EIGRPROUTE_H_
#define EIGRPROUTE_H_

#include <omnetpp.h>

#include "IPv4Address.h"


// Struct for metric parameters
struct EigrpMetricPar
{
    uint32_t delay;         // Delay sum on path
    uint32_t bandwidth;     // Min BW on path
    uint8_t reliability;    // Min rel. on path
    uint8_t load;           // Max load on path
    uint32_t mtu;           // Min MTU on path, only 24 bits used
    uint8_t hopCount;       // Hop count to destination
    uint8_t internalTag;    // Tag for filtering

    EigrpMetricPar() : delay(0), bandwidth(0), reliability(0), load(0), mtu(0), hopCount(0), internalTag(0) {}
};

template<typename IPAddress>
class EigrpRoute : public cObject
{
  protected:
    int routeId;            /** Unique ID of route */

    IPAddress routeAddress; /**< IP address of destination */
    IPAddress routeMask;    /**< Mask of destination */

    bool active;            /**< Route is passive or active */
    int queryOrigin;        /**< State of DUAL */
    int replyStatus;     // zatim jako citac, pripadne upravit pro identifikaci sousedu (zavest handle v NT a ty vkladat sem treba do vektoru)
    uint32_t fd;            /**< Feasible distance */
    // Zatím jsem rd nikde nevyuzil!
    EigrpMetricPar rd;      /**< Parameters for computation of reported distance that belongs to this router */
    uint32_t Dij;           /**< Shortest distance (Dij) */

    int referenceCounter;   /**< Counts amount of references to this object. */

  public:
    EigrpRoute(IPAddress& address, IPAddress& mask, int routeId);
    virtual ~EigrpRoute();

    bool operator==(const EigrpRoute<IPAddress>& route) const
    {
        return routeId == route.getRouteId();
    }

    int decrementRefCnt() {return --referenceCounter; }
    void incrementRefCnt() { ++referenceCounter; }

    int getRouteId() const {return routeId; }
    void setRouteId(int routeId) { this->routeId = routeId; }

    bool isActive() const { return active; }
    void setActive(bool active) { this->active = active; }

    uint32_t getFd() const { return fd; }
    void setFd(uint32_t fd) { this->fd = fd; }

    EigrpMetricPar getRdPar() const { return rd; }
    void setRdPar(EigrpMetricPar rd) { this->rd = rd; }

    uint32_t getDij() const { return Dij; }
    void setDij(uint32_t Dij) { this->Dij = Dij; }

    int getQueryOrigin() const {return queryOrigin; }
    void setQueryOrigin(int queryOrigin) { this->queryOrigin = queryOrigin; }

    int getReplyStatus() const { return replyStatus; }
    void setReplyStatus(int replyStatus) { this->replyStatus = replyStatus; }
    int decrementReplyStatus() { return --replyStatus; }

    IPAddress getRouteAddress() const { return routeAddress; }
    void setRouteAddress(IPAddress routeAddress) { this->routeAddress = routeAddress; }

    IPAddress getRouteMask() const { return routeMask; }
    void setRouteMask(IPAddress routeMask) { this->routeMask = routeMask; }

    //void addSource(EigrpRouteSource<IPAddress> *routeSrc) { sourceVec.push_back(routeSrc); }
    //EigrpRouteSource<IPAddress> *removeSource(EigrpRouteSource<IPAddress> * source);
    //EigrpRouteSource<IPAddress> *findSource(int sourceId);
    //int getNumSources() const { return sourceVec.size(); }
    //EigrpRouteSource<IPAddress> *getSource(int k) const { return sourceVec[k]; }

    //EigrpRouteSource<IPAddress> *getFirstSuccessor();
    //bool hasFeasibleSuccessor();
    //bool satisfiesFC(EigrpRouteSource<IPAddress> *source) const { return source->getRd() < this->fd; }
};

template<typename IPAddress>
class EigrpRouteSource : public cObject
{
  protected:
    int routeId;                    /** Unique ID of route (same as in EigrpRoute) */
    int sourceId;                   /**< Source of the route, correspond to neighbor ID (0 -> connected) */

    int nextHopId;                  /**< Id of next hop neighbor (usually same as sourceId, 0 -> connected) */
    IPAddress nextHop;              /**< IP address of next hop router (0.0.0.0 -> connected), only informational. It does not correspond to the sourceId (next hop may not be source of the route). */
    int interfaceId;                /** ID of outgoing interface for next hop */
    bool internal;                  /**< Source of the route (internal or external) */
    uint32_t rd;                    /**< Reported distance from neighbor (RDkj) */
    uint32_t metric;                /**< Actual metric value via that next Hop (not Dij - shortest distance) */
    EigrpMetricPar metricParams;    /**< Parameters for metric computation */
    EigrpMetricPar rdParams;        /**< Parameters from neighbor */
    bool successor;                 /**< If next hop is successor */
    // TODO oznaceni sumarizovane cesty

    EigrpRoute<IPAddress> *routeInfo;          /**< Pointer to route */

  public:
    static const int CONNECTED_ID = 0;  /**< Connected source */

    EigrpRouteSource(int interfaceId, int sourceId, int routeId, EigrpRoute<IPAddress> *routeInfo);
    virtual ~EigrpRouteSource();

    bool operator==(const EigrpRouteSource<IPAddress>& source) const
    {
        return sourceId == source.getSourceId();
    }

    uint32_t getRd() const { return rd; }
    void setRd(uint32_t rd) { this->rd = rd; }

    bool isInternal() const { return internal; }
    void setInternal(bool internal) { this->internal = internal; }

    uint32_t getMetric() const { return metric; }
    void setMetric(uint32_t metric) { this->metric = metric; }

    IPAddress getNextHop() const { return nextHop; }
    void setNextHop(IPAddress& nextHop) { this->nextHop = nextHop; }

    EigrpMetricPar getMetricParams() const { return metricParams; }
    void setMetricParams(EigrpMetricPar& par) { this->metricParams = par; }

    EigrpMetricPar getRdParams() const { return rdParams; }
    void setRdParams(EigrpMetricPar& rdParams) { this->rdParams = rdParams; }

    bool isSuccessor() const { return successor; }
    void setSuccessor(bool successor) { this->successor = successor; }

    int getSourceId() const { return sourceId; }
    void setSourceId(int sourceId) { this->sourceId = sourceId; }

    int getNexthopId() const { return nextHopId; }
    void setNexthopId(int nextHopId) { this->nextHopId = nextHopId; }

    int getIfaceId() const { return interfaceId; }
    void setIfaceId(int interfaceId) { this->interfaceId = interfaceId; }

    bool isUnreachable() const { return metric == UINT32_MAX; }

    EigrpRoute<IPAddress> *getRouteInfo() const { return routeInfo; }
    void setRouteInfo(EigrpRoute<IPAddress> *routeInfo) { this->routeInfo = routeInfo; }

    int getRouteId() const { return routeId; }
    void setRouteId(int routeId) { this->routeId = routeId; }


    /*uint8_t getReliability() const { return reliability; }
    void setReliability(uint8_t reliability) { this->reliability = reliability; }

    uint8_t getHopCount() const { return hopCount; }
    void setHopCount(uint8_t hopCount) { this->hopCount = hopCount; }

    uint8_t getInternalTag() const { return internalTag; }
    void setInternalTag(uint8_t internalTag) { this->internalTag = internalTag; }

    uint8_t getLoad() const { return load; }
    void setLoad(uint8_t load) { this->load = load; }

    uint32_t getMtu() const { return mtu; }
    void setMtu(uint32_t mtu) { this->mtu = mtu; }

    uint32_t getBandwidth() const { return bandwidth; }
    void setBandwidth(uint32_t bandwidth) { this->bandwidth = bandwidth; }

    uint32_t getDelay() const { return delay; }
    void setDelay(uint32_t delay) { this->delay = delay; }*/
};



template class EigrpRouteSource<IPv4Address>;

template<typename IPAddress>
EigrpRouteSource<IPAddress>::EigrpRouteSource(int interfaceId, int nextHopId, int routeId, EigrpRoute<IPAddress> *routeInfo) :
    routeId(routeId), nextHopId(nextHopId), interfaceId(interfaceId), routeInfo(routeInfo)
{
    metric = UINT32_MAX;
    rd = 0;
    internal = true;
    successor = false;

    routeInfo->incrementRefCnt();
}

template<typename IPAddress>
EigrpRouteSource<IPAddress>::~EigrpRouteSource()
{
     if (getRouteInfo()->decrementRefCnt() == 0)
         delete getRouteInfo();
}

template<typename IPAddress>
EigrpRoute<IPAddress>::EigrpRoute(IPAddress& address, IPAddress& mask, int routeId) :
    routeId(routeId), routeAddress(address), routeMask(mask)
{
    fd = Dij = UINT32_MAX;
    rd.delay = UINT32_MAX;

    active = false;
    queryOrigin = 1;
    replyStatus = 0;

    referenceCounter = 0;
}

template<typename IPAddress>
EigrpRoute<IPAddress>::~EigrpRoute()
{
}

/*
template<typename IPAddress>
EigrpRouteSource<IPAddress> *EigrpRoute<IPAddress>::removeSource(EigrpRouteSource<IPAddress> * source)
{
    typename std::vector<EigrpRouteSource<IPv4Address> *>::iterator it;

    for (it = sourceVec.begin(); it != sourceVec.end(); it++)
    {
        if ((*it) == source)
        {
            return source;
        }
    }

    return NULL;
}

template<typename IPAddress>
EigrpRouteSource<IPAddress> *EigrpRoute<IPAddress>::findSource(int sourceId)
{
    typename std::vector<EigrpRouteSource<IPv4Address> *>::iterator it;

    for (it = sourceVec.begin(); it != sourceVec.end(); it++)
    {
        if ((*it)->getSourceId() == sourceId)
        {
            return *it;
        }
    }

    return NULL;
}

template<typename IPAddress>
EigrpRouteSource<IPAddress> *EigrpRoute<IPAddress>::getFirstSuccessor()
{
    typename std::vector<EigrpRouteSource<IPv4Address> *>::iterator it;

    for (it = sourceVec.begin(); it != sourceVec.end(); it++)
    {
        if ((*it)->isSuccessor())
        {
            return *it;
        }
    }

    return NULL;
}

template<typename IPAddress>
bool EigrpRoute<IPAddress>::hasFeasibleSuccessor()
{
    typename std::vector<EigrpRouteSource<IPAddress> *>::iterator it;

    for (it = sourceVec.begin(); it != sourceVec.end(); it++)
    {
        if ((*it)->getRd() < this->fd)
        {
            return true;
        }
    }

    return false;
}
*/

#endif /* EIGRPROUTE_H_ */
