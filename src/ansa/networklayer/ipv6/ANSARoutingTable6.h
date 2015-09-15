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

/**
 * @file ANSARoutingTable6.h
 * @date 21.5.2013
 * @author Jiri Trhlik (mailto:jiritm@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @brief Extended RoutingTable6
 * @details Adds administrative distance, fixes routing table cache, IPv4-like routes updates
 */

#ifndef __INET_ANSA_ANSAROUTINGTABLE6_H
#define __INET_ANSA_ANSAROUTINGTABLE6_H

#include "networklayer/ipv6/IPv6RoutingTable.h"
#include "networklayer/ipv6/IPv6Route.h"

class IInterfaceTable;
class InterfaceEntry;
class ANSARoutingTable6;

/**
 * Extends inet::IPv6Route by administrative distance.
 * TODO: watch this class and changes in INET! - especially inet::IPv6Route class
 * @see inet::IPv6Route
 */
class ANSAIPv6Route : public inet::IPv6Route
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
        pEIGRPext,       //EIGRP external
        pBABEL           //BABEL
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
        dBABEL = 125,
        dEGP = 140,
        dODR = 160,
        dEIGRPExternal = 170,
        dBGPInternal = 200,
        dDHCPlearned = 254,
        dUnknown = 255
    };

    enum ChangeCodes // field codes for changed() - inet::IPv4Route-like
    {
        F_NEXTHOP,
        F_IFACE,
        F_METRIC,
        F_ADMINDIST,
        F_ROUTINGPROTSOURCE
    };

  protected:
    inet::IInterfaceTable *ift;     ///< cached pointer
    ANSARoutingTable6 *rt;    ///< the routing table in which this route is inserted, or NULL
    unsigned int  _adminDist;
    /** Should be set if route source is a "routing protocol" **/
    RoutingProtocolSource _routingProtocolSource;

    void changed(int fieldCode);
    void changedSilent(int fieldCode);

  public:
    ANSAIPv6Route(inet::IPv6Address destPrefix, int length, RouteSrc src);

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
    virtual void setNextHop(const inet::IPv6Address& nextHop)  {if (nextHop != _nextHop) { _nextHop = nextHop; changed(F_NEXTHOP);} }
    virtual void setMetric(int metric)  { if (metric != _metric) { _metric = metric; changed(F_METRIC);} }

    /**
     * Silent versions of the setters. Used if more than one route information is changed.
     * Silent versions do not fire "route changed notification".
     */
    virtual void setAdminDistSilent(RouteAdminDist adminDist)  { if (adminDist != _adminDist) { _adminDist = adminDist; changedSilent(F_ADMINDIST);} }
    virtual void setRoutingProtocolSourceSilent(RoutingProtocolSource routingProtocolSource) {  if (routingProtocolSource != _routingProtocolSource) { _routingProtocolSource = routingProtocolSource; changedSilent(F_ROUTINGPROTSOURCE);} }
    virtual void setInterfaceIdSilent(int interfaceId)  { if (interfaceId != _interfaceID) { _interfaceID = interfaceId; changedSilent(F_IFACE);} }
    virtual void setNextHopSilent(const inet::IPv6Address& nextHop)  {if (nextHop != _nextHop) { _nextHop = nextHop; changedSilent(F_NEXTHOP);} }
    virtual void setMetricSilent(int metric)  { if (metric != _metric) { _metric = metric; changedSilent(F_METRIC);} }
};

/**
 * Extends RoutingTable6 by administrative distance.
 * TODO: watch this class and changes in INET! - especially RoutingTable6 class
 * @see RoutingTable6
 */
class ANSARoutingTable6 : public inet::IPv6RoutingTable
{
  protected:
    virtual void addRoute(ANSAIPv6Route *route);

  public:
    ANSARoutingTable6();
    virtual ~ANSARoutingTable6();
    NotificationBoard* nb;
    /**
     * Finds route with the given prefix and prefix length.
     * @return NULL, if route does not exist
     */
    virtual inet::IPv6Route *findRoute(const inet::IPv6Address& prefix, int prefixLength);

    /**
     * Finds route with the given prefix, prefix length and nexthop.
     *
     * @param   prefix  Network prefix
     * @param   prefixLength    Length of netwok prefix
     * @param   nexthop     Next-hop address
     * @return  If found return pointer to route, otherwise NULL
     */
    virtual inet::IPv6Route *findRoute(const inet::IPv6Address& prefix, int prefixLength, const inet::IPv6Address& nexthop);

    /**
     * Prepares routing table for adding new route.
     * e.g. removes route with the same prefix, prefix length and lower administrative distance
     * and purge destination cache
     *
     * Method uses removeRouteSilent() for removing routes - NF_IPv6_ROUTE_DELETED notification cannot be fired,
     * because other routing protocol listening to this notification could add his route (route is already deleted
     * so this route would be added without any problem with administrative distance), than this method
     * return true and the protocol calling this method adds his route also.
     *
     * One could create another notification, like NF_IPv6_ROUTE_DELETED_INTERNAL, in the future.
     * Adding a route would not be allowed on receipt this notification.
     *
     * @return true, if it is safe to add route,
     *         false otherwise
     */
    virtual bool prepareForAddRoute(inet::IPv6Route *route);

    virtual void addOrUpdateOnLinkPrefix(const inet::IPv6Address& destPrefix, int prefixLength,
                                 int interfaceId, simtime_t expiryTime);

    virtual void addOrUpdateOwnAdvPrefix(const inet::IPv6Address& destPrefix, int prefixLength,
                                 int interfaceId, simtime_t expiryTime);

    /**
     * @see prepareForAddRoute
     */
    virtual void addStaticRoute(const inet::IPv6Address& destPrefix, int prefixLength,
                        unsigned int interfaceId, const inet::IPv6Address& nextHop,
                        int metric = 0);

    /**
     * @see prepareForAddRoute
     */
    virtual void addDefaultRoute(const inet::IPv6Address& raSrcAddr, unsigned int ifID,
        simtime_t routerLifetime);

    /**
     * @see prepareForAddRoute
     */
    virtual void addRoutingProtocolRoute(ANSAIPv6Route *route);

//    /**
//     * Must be reimplemented because of cache handling.
//     */
//    virtual void removeRoute(inet::IPv6Route *route);

    /**
     * Same as removeRoute, except route deleted notification is not fired.
     * @see prapareForAddRoute
     */
    virtual void removeRouteSilent(inet::IPv6Route *route);

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

