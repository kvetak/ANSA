//
// Copyright (C) 2012 Andras Varga
//
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

#ifndef __INET_IROUTINGTABLE_H
#define __INET_IROUTINGTABLE_H

#include "common/INETDefs.h"
#include "networklayer/common/L3Address.h"
#include "networklayer/contract/IRoute.h"
#include "networklayer/ipv4/IPv4Route.h"

namespace inet {

/**
 * A C++ interface to abstract the functionality of a routing table, regardless of address type.
 */
class INET_API IRoutingTable
{
  public:
    virtual ~IRoutingTable() {};

    /** @name Miscellaneous functions */
    //@{
    /**
     * Forwarding on/off
     */
    virtual bool isForwardingEnabled() const = 0;    //XXX IP modulba?

    /**
     * Multicast forwarding on/off
     */
    virtual bool isMulticastForwardingEnabled() const = 0;    //XXX IP modulba?

    /**
     * Returns routerId.
     */
    virtual L3Address getRouterIdAsGeneric() const = 0;

    /**
     * Checks if the address is a local one, i.e. one of the host's.
     */
    virtual bool isLocalAddress(const L3Address& dest) const = 0;    //XXX maybe into InterfaceTable?

    /**
     * Returns an interface given by its address. Returns nullptr if not found.
     */
    virtual InterfaceEntry *getInterfaceByAddress(const L3Address& address) const = 0;    //XXX should be find..., see next one

    /**
     * Prints the routing table.
     */
    virtual void printRoutingTable() const = 0;
    //@}

    /** @name Routing functions (query the route table) */
    //@{
    /**
     * The routing function. Performs longest prefix match for the given
     * destination address, and returns the resulting route. Returns nullptr
     * if there is no matching route.
     */
    virtual IRoute *findBestMatchingRoute(const L3Address& dest) const = 0;

    /**
     * Convenience function based on findBestMatchingRoute().
     *
     * Returns the output interface for the packets with dest as destination
     * address, or nullptr if the destination is not in routing table.
     */
    virtual InterfaceEntry *getOutputInterfaceForDestination(const L3Address& dest) const = 0;    //XXX redundant

    /**
     * Convenience function based on findBestMatchingRoute().
     *
     * Returns the gateway for the destination address. Returns the unspecified
     * address if the destination is not in routing table or the gateway field
     * is not filled in in the route.
     */
    virtual L3Address getNextHopForDestination(const L3Address& dest) const = 0;    //XXX redundant AND unused
    //@}

    /** @name Multicast routing functions */
    //@{

    /**
     * Checks if the address is in one of the local multicast group
     * address list.
     */
    virtual bool isLocalMulticastAddress(const L3Address& dest) const = 0;

    /**
     * Returns route for a multicast origin and group.
     */
    virtual IMulticastRoute *findBestMatchingMulticastRoute(const L3Address& origin, const L3Address& group) const = 0;
    //@}

    /** @name Route table manipulation */
    //@{

    /**
     * Returns the total number of unicast routes.
     */
    virtual int getNumRoutes() const = 0;

    /**
     * Returns the kth route.
     */
    virtual IRoute *getRoute(int k) const = 0;

    /**
     * Finds and returns the default route, or nullptr if it doesn't exist
     */
    virtual IRoute *getDefaultRoute() const = 0;    //XXX is this a universal concept?

    /**
     * Adds a route to the routing table. Routes are allowed to be modified
     * while in the routing table. (There is a notification mechanism that
     * allows routing table internals to be updated on a routing entry change.)
     */
    virtual void addRoute(IRoute *entry) = 0;

    /**
     * Removes the given route from the routing table, and returns it.
     * nullptr is returned if the route was not in the routing table.
     */
    virtual IRoute *removeRoute(IRoute *entry) = 0;

    /**
     * Deletes the given route from the routing table.
     * Returns true if the route was deleted, and false if it was
     * not in the routing table.
     */
    virtual bool deleteRoute(IRoute *entry) = 0;

    /**
     * Returns the total number of multicast routes.
     */
    virtual int getNumMulticastRoutes() const = 0;

    /**
     * Returns the kth multicast route.
     */
    virtual IMulticastRoute *getMulticastRoute(int k) const = 0;

    /**
     * Adds a multicast route to the routing table. Routes are allowed to be modified
     * while in the routing table. (There is a notification mechanism that
     * allows routing table internals to be updated on a routing entry change.)
     */
    virtual void addMulticastRoute(IMulticastRoute *entry) = 0;

    /**
     * Removes the given route from the routing table, and returns it.
     * nullptr is returned of the route was not in the routing table.
     */
    virtual IMulticastRoute *removeMulticastRoute(IMulticastRoute *entry) = 0;

    /**
     * Deletes the given multicast route from the routing table.
     * Returns true if the route was deleted, and false if it was
     * not in the routing table.
     */
    virtual bool deleteMulticastRoute(IMulticastRoute *entry) = 0;

    //Added by Michal Ruprich from ANSA
    /**
     * Deletes invalid routes from the routing table. Invalid routes are those
     * where the isValid() method returns false.
     */
    virtual void purge() = 0;

    /**
     * Utility function: Returns a vector of all addresses of the node.
     */
    virtual std::vector<IPv4Address> gatherAddresses() const = 0;

    /**
     * To be called from route objects whenever a field changes. Used for
     * maintaining internal data structures and firing "routing table changed"
     * notifications.
     */
    virtual void routeChanged(IPv4Route *entry, int fieldCode) = 0;

    /**
     * To be called from multicast route objects whenever a field changes. Used for
     * maintaining internal data structures and firing "routing table changed"
     * notifications.
     */
    virtual void multicastRouteChanged(IPv4MulticastRoute *entry, int fieldCode) = 0;
    //@}

    virtual IRoute *createRoute() = 0;

    //added by Michal Ruprich from ANSA
    /**
     * For debugging
     */
    virtual void printMulticastRoutingTable() const = 0;

    /**
     * Returns the host or router this routing table lives in.
     */
    virtual cModule *getHostModule() = 0;

    /** @name Interfaces */
    //@{
    virtual void configureInterfaceForIPv4(InterfaceEntry *ie) = 0;

    /**
     * Returns an interface given by its address. Returns NULL if not found.
     */
    virtual InterfaceEntry *getInterfaceByAddress(const IPv4Address& address) const = 0;
    //@}

    /**
     * Returns routerId.
     */
    virtual IPv4Address getRouterId() = 0;

    /**
     * Sets routerId.
     */
    virtual void setRouterId(IPv4Address a) = 0;


    /** @name Routing functions (query the route table) */
    //@{
    /**
     * Checks if the address is a local network broadcast address, i.e. one of the
     * broadcast addresses derived from the interface addresses and netmasks.
     */
    virtual bool isLocalBroadcastAddress(const IPv4Address& dest) const = 0;

    /**
     * Returns the interface entry having the specified address
     * as its local broadcast address.
     */
    virtual InterfaceEntry *findInterfaceByLocalBroadcastAddress(const IPv4Address& dest) const = 0;

    /**
     * The routing function. Performs longest prefix match for the given
     * destination address, and returns the resulting route. Returns NULL
     * if there is no matching route.
     */
    virtual inet::IPv4Route *findBestMatchingRoute(const IPv4Address& dest) const = 0;

    /**
     * Convenience function based on findBestMatchingRoute().
     *
     * Returns the output interface for the packets with dest as destination
     * address, or NULL if the destination is not in routing table.
     */
    virtual InterfaceEntry *getInterfaceForDestAddr(const IPv4Address& dest) const = 0;

    /**
     * Convenience function based on findBestMatchingRoute().
     *
     * Returns the gateway for the destination address. Returns the unspecified
     * address if the destination is not in routing table or the gateway field
     * is not filled in in the route.
     */
    virtual IPv4Address getGatewayForDestAddr(const IPv4Address& dest) const = 0;
    //@}
};

} // namespace inet

#endif // ifndef __INET_IROUTINGTABLE_H

