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

/**
 * Extends IPv6Route by administrative distance.
 * @see IPv6Route
 */
class ANSAIPv6Route : public IPv6Route
{
  public:

    /** Cisco like administrative distances */
    enum RouteAdminDist
    {
        DirectlyConnected = 0,
        Static = 1,
        EIGRPSummary = 5,
        BGPExternal = 20,
        EIGRPInternal = 90,
        IGRP = 100,
        OSPF = 110,
        ISIS = 115,
        RIP = 120,
        EGP = 140,
        ODR = 160,
        EIGRPExternal = 170,
        BGPInternal = 200,
        DHCPlearned = 254,
        Unknown = 255,
    };

  protected:
    IInterfaceTable *ift; // cached pointer
    RouteAdminDist _adminDist;

  public:
    ANSAIPv6Route(IPv6Address destPrefix, int length, RouteSrc src);

    virtual std::string info() const;
    virtual std::string detailedInfo() const;

    void setAdminDist(RouteAdminDist adminDist)  {_adminDist = adminDist;}

    RouteAdminDist getAdminDist() const  {return _adminDist;}
};

/**
 * Extends RoutingTable6 by administrative distance.
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
    virtual ANSAIPv6Route *findRoute(const IPv6Address& prefix, int prefixLength);

    /**
     * Prepares routing table for adding new route.
     * e.g. removes route with the same prefix, prefix length and lower administrative distance
     * @return true, if it is safe to add route,
     *         false otherwise
     */
    virtual bool prepareForAddRoute(ANSAIPv6Route *route);

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
};

#endif

