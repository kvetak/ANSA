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
#include <algorithm>

#include "IPv4Address.h"
#include "EigrpMessage_m.h"
#include "EigrpMetricHelper.h"


/* Struct for metric parameters
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
};*/

template<typename IPAddress> class EigrpRouteSource;

template<typename IPAddress>
class EigrpRoute : public cObject
{
  protected:
    int routeId;            /** Unique ID of route */

    IPAddress routeAddress; /**< IP address of destination */
    IPAddress routeMask;    /**< Mask of destination */

    int queryOrigin;        /**< State of DUAL */
    std::vector<int> replyStatusTable;  /**< Reply status for each neighbor*/
    uint64_t fd;            /**< Feasible distance */
    EigrpWideMetricPar rd;      /**< Parameters for computation of reported distance that belongs to this router */
    uint64_t Dij;           /**< Shortest distance (Dij) */
    EigrpRouteSource<IPAddress> *successor; /**< Actual successor for route reported to neighbors of router */
    int numSuccessors;      /** Number of successors for the route */

    int referenceCounter;   /**< Counts amount of references to this object. */

  public:
    EigrpRoute(IPAddress& address, IPAddress& mask);
    virtual ~EigrpRoute();

    bool operator==(const EigrpRoute<IPAddress>& route) const
    {
        return routeId == route.getRouteId();
    }

    int decrementRefCnt() {return --referenceCounter; }
    void incrementRefCnt() { ++referenceCounter; }
    int getRefCnt() {return referenceCounter; }

    int getRouteId() const {return routeId; }
    void setRouteId(int routeId) { this->routeId = routeId; }

    uint64_t getFd() const { return fd; }
    void setFd(uint64_t fd) { this->fd = fd; }

    EigrpWideMetricPar getRdPar() const { return rd; }
    void setRdPar(EigrpWideMetricPar rd) { this->rd = rd; }

    uint64_t getDij() const { return Dij; }
    void setDij(uint64_t Dij) { this->Dij = Dij; }

    int getQueryOrigin() const {return queryOrigin; }
    void setQueryOrigin(int queryOrigin) { this->queryOrigin = queryOrigin; }

    int getReplyStatusSum() const { return replyStatusTable.size(); }
    bool getReplyStatus(int neighborId);
    void setReplyStatus(int neighborId) { replyStatusTable.push_back(neighborId); }
    bool unsetReplyStatus(int neighborId);

    IPAddress getRouteAddress() const { return routeAddress; }
    void setRouteAddress(IPAddress routeAddress) { this->routeAddress = routeAddress; }

    IPAddress getRouteMask() const { return routeMask; }
    void setRouteMask(IPAddress routeMask) { this->routeMask = routeMask; }

    bool isActive() const { return replyStatusTable.size() > 0; }

    EigrpRouteSource<IPAddress> *getSuccessor() const { return this->successor; }
    void setSuccessor(EigrpRouteSource<IPAddress> * successor) { this->successor = successor; }

    void setNumSucc(int numSuccessors) { this->numSuccessors = numSuccessors; }
    int getNumSucc() const {return numSuccessors; }

    //void addSource(EigrpRouteSource<IPAddress> *routeSrc) { sourceVec.push_back(routeSrc); }
    //EigrpRouteSource<IPAddress> *removeSource(EigrpRouteSource<IPAddress> * source);
    //EigrpRouteSource<IPAddress> *findSource(int sourceId);
    //int getNumSources() const { return sourceVec.size(); }
    //EigrpRouteSource<IPAddress> *getSource(int k) const { return sourceVec[k]; }

    //EigrpRouteSource<IPAddreint decrementReplyStatusSum() { return --replyStatus; }ss> *getFirstSuccessor();
    //bool hasFeasibleSuccessor();
    //bool satisfiesFC(EigrpRouteSource<IPAddress> *source) const { return source->getRd() < this->fd; }
};

template<typename IPAddress>
class EigrpRouteSource : public cObject
{
  protected:
    int sourceId;                   /** Unique ID of source */
    int routeId;                    /** Unique ID of route (same as in EigrpRoute) */

    IPAddress originator;           /**< IP of originating router */
    int nextHopId;                  /**< Id of next hop neighbor (usually same as sourceId, 0 -> connected) */
    IPAddress nextHop;              /**< IP address of next hop router (0.0.0.0 -> connected), only informational. It does not correspond to the sourceId (next hop may not be source of the route). */
    int interfaceId;                /** ID of outgoing interface for next hop */
    const char *interfaceName;
    bool internal;                  /**< Source of the route (internal or external) */
    uint64_t rd;                    /**< Reported distance from neighbor (RDkj) */
    uint64_t metric;                /**< Actual metric value via that next Hop (not Dij - shortest distance) */
    EigrpWideMetricPar metricParams;    /**< Parameters for metric computation */
    EigrpWideMetricPar rdParams;        /**< Parameters from neighbor */
    bool successor;                 /**< If next hop is successor */
    // TODO oznaceni sumarizovane cesty
    bool valid;                     /**< Invalid sources will be deleted */
    bool delayedRemove;             /**< Source will be deleted after receiving Ack from neighbor */

    EigrpRoute<IPAddress> *routeInfo;          /**< Pointer to route */

  public:

    EigrpRouteSource(int interfaceId, const char *ifaceName, int nextHopId, int routeId, EigrpRoute<IPAddress> *routeInfo);
    virtual ~EigrpRouteSource();

    bool operator==(const EigrpRouteSource<IPAddress>& source) const
    {
        return sourceId == source.getSourceId();
    }

    uint64_t getRd() const { return rd; }
    void setRd(uint64_t rd) { this->rd = rd; }

    bool isInternal() const { return internal; }
    void setInternal(bool internal) { this->internal = internal; }

    uint64_t getMetric() const { return metric; }
    void setMetric(uint64_t metric) { this->metric = metric; }

    IPAddress getNextHop() const { return nextHop; }
    void setNextHop(IPAddress& nextHop) { this->nextHop = nextHop; }

    EigrpWideMetricPar getMetricParams() const { return metricParams; }
    void setMetricParams(EigrpWideMetricPar& par) { this->metricParams = par; }

    EigrpWideMetricPar getRdParams() const { return rdParams; }
    void setRdParams(EigrpWideMetricPar& rdParams) { this->rdParams = rdParams; }

    bool isSuccessor() const { return successor; }
    void setSuccessor(bool successor) { this->successor = successor; }

    int getNexthopId() const { return nextHopId; }
    void setNexthopId(int nextHopId) { this->nextHopId = nextHopId; }

    int getIfaceId() const { return interfaceId; }
    void setIfaceId(int interfaceId) { this->interfaceId = interfaceId; }

    EigrpRoute<IPAddress> *getRouteInfo() const { return routeInfo; }
    void setRouteInfo(EigrpRoute<IPAddress> *routeInfo) { this->routeInfo = routeInfo; }

    int getRouteId() const { return routeId; }
    void setRouteId(int routeId) { this->routeId = routeId; }

    int getSourceId() const { return sourceId; }
    void setSourceId(int sourceId) { this->sourceId = sourceId; }

    IPAddress getOriginator() const { return originator; }
    void setOriginator(IPAddress& originator) { this->originator = originator; }

    bool isValid() const { return valid; }
    void setValid(bool valid) { this->valid = valid; }

    bool isUnreachable() const { return metric == EigrpMetricHelper::METRIC_INF; }
    /** Sets metric and RD to infinity */
    void setUnreachableMetric()
    {
        metric = EigrpMetricHelper::METRIC_INF; metricParams.bandwidth = EigrpMetricHelper::BANDWIDTH_INF; metricParams.delay = EigrpMetricHelper::DELAY_INF;
        rd = EigrpMetricHelper::METRIC_INF; rdParams.bandwidth = EigrpMetricHelper::BANDWIDTH_INF; rdParams.delay = EigrpMetricHelper::DELAY_INF; }

    bool isSetDelayedRemove() const { return delayedRemove; }
    void setDelayedRemove(bool delayedRemove) { this->delayedRemove = delayedRemove; }

    const char *getIfaceName() const { return interfaceName; }
};



template class EigrpRouteSource<IPv4Address>;

template<typename IPAddress>
EigrpRouteSource<IPAddress>::EigrpRouteSource(int interfaceId, const char *ifaceName, int nextHopId, int routeId, EigrpRoute<IPAddress> *routeInfo) :
    routeId(routeId), nextHopId(nextHopId), interfaceId(interfaceId), interfaceName(ifaceName), routeInfo(routeInfo)
{
    metric = EigrpMetricHelper::METRIC_INF;
    rd = 0;
    internal = true;
    successor = false;
    sourceId = 0;
    valid = true;
    delayedRemove = false;

    routeInfo->incrementRefCnt();
}

template<typename IPAddress>
EigrpRouteSource<IPAddress>::~EigrpRouteSource()
{
    if (getRouteInfo()->decrementRefCnt() == 0)
        delete getRouteInfo();
}

template<typename IPAddress>
EigrpRoute<IPAddress>::EigrpRoute(IPAddress& address, IPAddress& mask) :
    routeId(0), routeAddress(address), routeMask(mask)
{
    fd = Dij = EigrpMetricHelper::METRIC_INF;
    rd.delay = EigrpMetricHelper::METRIC_INF;

    successor = NULL;
    queryOrigin = 1;

    numSuccessors = 0;
    referenceCounter = 0;
}

template<typename IPAddress>
EigrpRoute<IPAddress>::~EigrpRoute()
{
}

template<typename IPAddress>
bool EigrpRoute<IPAddress>::getReplyStatus(int neighborId)
{
    std::vector<int>::iterator it;

    if ((it = std::find(replyStatusTable.begin(), replyStatusTable.end(), neighborId)) != replyStatusTable.end())
        return true;

    return false;
}

/**
 * Clear handle for specified neighbor in Reply Status table.
 */
template<typename IPAddress>
bool EigrpRoute<IPAddress>::unsetReplyStatus(int neighborId)
{
    std::vector<int>::iterator it;

    if ((it = std::find(replyStatusTable.begin(), replyStatusTable.end(), neighborId)) != replyStatusTable.end())
    {
        replyStatusTable.erase(it);
        return true;
    }
    return false;
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
