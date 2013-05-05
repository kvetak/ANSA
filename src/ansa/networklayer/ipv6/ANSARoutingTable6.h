// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_ANSA_ANSAROUTINGTABLE6_H
#define __INET_ANSA_ANSAROUTINGTABLE6_H

#include "RoutingTable6.h"

class IInterfaceTable;
class InterfaceEntry;
class ANSARoutingTable6;

/**
 * Extends IPv6Route by administrative distance.
 * TODO: watch this class and changes in INET! - especially IPv6Route class
 * @see IPv6Route
 */
class ANSAIPv6Route : public IPv6Route
{
  public:

    /** Should be set if route source is a "routing protocol" **/
    enum RoutingProtocolSource
    {
        pUnknown = 0,
        pRIP,            //RIPng
        pBGP,            //BGP
        pISIS1,          //ISIS L1
        pISIS2,          //ISIS L2
        pISISinterarea,  //ISIS interarea
        pISISsum,        //ISIS summary
        pOSPFintra,      //OSPF intra
        pOSPFinter,      //OSPF inter
        pOSPFext1,       //OSPF ext 1
        pOSPFext2,       //OSPF ext 2
        pOSPFNSSAext1,   //OSPF NSSA ext 1
        pOSPFNSSAext2,   //OSPF NSSA ext 2
        pEIGRP,          //EIGRP
        pEIGRPext        //EIGRP external
    };

    /** Cisco like administrative distances (includes IPv4 protocols)*/
    enum RouteAdminDist
    {
        dDirectlyConnected = 0,
        dStatic = 1,
        dEIGRPSummary = 5,
        dBGPExternal = 20,
        dEIGRPInternal = 90,
        dIGRP = 100,
        dOSPF = 110,
        dISIS = 115,
        dRIP = 120,
        dEGP = 140,
        dODR = 160,
        dEIGRPExternal = 170,
        dBGPInternal = 200,
        dDHCPlearned = 254,
        dUnknown = 255
    };

    enum ChangeCodes // field codes for changed() - IPv4Route-like
    {
        F_NEXTHOP,
        F_IFACE,
        F_METRIC,
        F_ADMINDIST,
        F_ROUTINGPROTSOURCE
    };

  protected:
    IInterfaceTable *ift;     ///< cached pointer
    ANSARoutingTable6 *rt;    ///< the routing table in which this route is inserted, or NULL
    unsigned int  _adminDist;
    /** Should be set if route source is a "routing protocol" **/
    RoutingProtocolSource _routingProtocolSource;

    void changed(int fieldCode);
    void changedSilent(int fieldCode);

  public:
    ANSAIPv6Route(IPv6Address destPrefix, int length, RouteSrc src);

    /** To be called by the routing table when this route is added or removed from it */
    virtual void setRoutingTable(ANSARoutingTable6 *rt) {this->rt = rt;}
    ANSARoutingTable6 *getRoutingTable() const {return rt;}

    virtual std::string info() const;
    virtual std::string detailedInfo() const;
    virtual const char *getRouteSrcName() const;

    unsigned int getAdminDist() const  { return _adminDist; }
    RoutingProtocolSource getRoutingProtocolSource() const { return _routingProtocolSource; }
    const char *getInterfaceName() const;

    virtual void setAdminDist(unsigned int adminDist)  { if (adminDist != _adminDist) { _adminDist = adminDist; changed(F_ADMINDIST);} }
    virtual void setRoutingProtocolSource(RoutingProtocolSource routingProtocolSource) {  if (routingProtocolSource != _routingProtocolSource) { _routingProtocolSource = routingProtocolSource; changed(F_ROUTINGPROTSOURCE);} }
    virtual void setInterfaceId(int interfaceId)  { if (interfaceId != _interfaceID) { _interfaceID = interfaceId; changed(F_IFACE);} }
    virtual void setNextHop(const IPv6Address& nextHop)  {if (nextHop != _nextHop) { _nextHop = nextHop; changed(F_NEXTHOP);} }
    virtual void setMetric(int metric)  { if (metric != _metric) { _metric = metric; changed(F_METRIC);} }

    /**
     * Silent versions of the setters. Used if more than one route information is changed.
     * Silent versions do not fire "route changed notification".
     */
    virtual void setAdminDistSilent(RouteAdminDist adminDist)  { if (adminDist != _adminDist) { _adminDist = adminDist; changedSilent(F_ADMINDIST);} }
    virtual void setRoutingProtocolSourceSilent(RoutingProtocolSource routingProtocolSource) {  if (routingProtocolSource != _routingProtocolSource) { _routingProtocolSource = routingProtocolSource; changedSilent(F_ROUTINGPROTSOURCE);} }
    virtual void setInterfaceIdSilent(int interfaceId)  { if (interfaceId != _interfaceID) { _interfaceID = interfaceId; changedSilent(F_IFACE);} }
    virtual void setNextHopSilent(const IPv6Address& nextHop)  {if (nextHop != _nextHop) { _nextHop = nextHop; changedSilent(F_NEXTHOP);} }
    virtual void setMetricSilent(int metric)  { if (metric != _metric) { _metric = metric; changedSilent(F_METRIC);} }
};

/**
 * Extends RoutingTable6 by administrative distance.
 * TODO: watch this class and changes in INET! - especially RoutingTable6 class
 * @see RoutingTable6
 */
class ANSARoutingTable6 : public RoutingTable6
{
  protected:
    virtual void addRoute(ANSAIPv6Route *route);

  public:
    ANSARoutingTable6();
    virtual ~ANSARoutingTable6();

    /**
     * Finds route with the given prefix and prefix length.
     * @return NULL, if route does not exist
     */
    virtual IPv6Route *findRoute(const IPv6Address& prefix, int prefixLength);

    /**
     * Prepares routing table for adding new route.
     * e.g. removes route with the same prefix, prefix length and lower administrative distance
     * and purge destination cache
     * @return true, if it is safe to add route,
     *         false otherwise
     */
    virtual bool prepareForAddRoute(IPv6Route *route);

    virtual void addOrUpdateOnLinkPrefix(const IPv6Address& destPrefix, int prefixLength,
                                 int interfaceId, simtime_t expiryTime);

    virtual void addOrUpdateOwnAdvPrefix(const IPv6Address& destPrefix, int prefixLength,
                                 int interfaceId, simtime_t expiryTime);

    /**
     * @see prepareForAddRoute
     */
    virtual void addStaticRoute(const IPv6Address& destPrefix, int prefixLength,
                        unsigned int interfaceId, const IPv6Address& nextHop,
                        int metric = 0);

    /**
     * @see prepareForAddRoute
     */
    virtual void addDefaultRoute(const IPv6Address& raSrcAddr, unsigned int ifID,
        simtime_t routerLifetime);

    /**
     * @see prepareForAddRoute
     */
    virtual void addRoutingProtocolRoute(ANSAIPv6Route *route);

    /**
     * Must be reimplemented because of cache handling.
     */
    virtual void removeRoute(IPv6Route *route);

    /**
     * To be called from route objects whenever a field changes. Used for
     * maintaining internal data structures and firing "routing table changed"
     * notifications.
     */
    virtual void routeChanged(ANSAIPv6Route *entry, int fieldCode);

    /**
     * Same as routeChanged, except route changed notification is not fired.
     * @see routeChanged
     */
    virtual void routeChangedSilent(ANSAIPv6Route *entry, int fieldCode);

    /**
     * Must be reimplemented because of cache handling.
     */
    virtual void receiveChangeNotification(int category, const cObject *details);
};

#endif

