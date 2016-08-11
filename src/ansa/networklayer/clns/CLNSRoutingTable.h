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

#ifndef __ANSAINET_CLNSROUTINGTABLE_H_
#define __ANSAINET_CLNSROUTINGTABLE_H_

#include <vector>

#include "inet/common/INETDefs.h"

#include "ansa/networklayer/clns/CLNSAddress.h"
#include "ansa/networklayer/clns/CLNSRoute.h"
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/common/lifecycle/ILifecycle.h"

using namespace omnetpp;

namespace inet {

class IInterfaceTable;

/**
 * TODO - Generated class
 */
class CLNSRoutingTable : public cSimpleModule, public IRoutingTable, protected cListener, public ILifecycle
{

  private:
    typedef std::vector<CLNSRoute *> RouteVector;
    RouteVector routes;    // Unicast route array, sorted by //netmask desc, dest asc, metric asc
    CLNSAddress routerId;

  protected:
    IInterfaceTable *ift = nullptr;    // cached pointer

    bool isNodeUp = false;

    // routing cache: maps destination address to the route
    typedef std::map<CLNSAddress, CLNSRoute *> RoutingCache;
    mutable RoutingCache routingCache;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG) override;


  private:
    void invalidateCache();

  public:
    virtual ~CLNSRoutingTable();

        /** @name Miscellaneous functions */
        //@{
        /**
         * Forwarding on/off
         */
        virtual bool isForwardingEnabled() const override { return true;}    //XXX IP modulba?

        /**
         * Multicast forwarding on/off
         */
        virtual bool isMulticastForwardingEnabled() const override {return false;}    //XXX IP modulba?

        /**
         * Returns routerId.
         */
        virtual L3Address getRouterIdAsGeneric() const override { return getRouterId(); }

        /**
         * Checks if the address is a local one, i.e. one of the host's.
         */
        virtual bool isLocalAddress(const L3Address& dest) const override {return false;}    //TODO A1

        /**
         * Returns an interface given by its address. Returns nullptr if not found.
         */
        virtual InterfaceEntry *getInterfaceByAddress(const L3Address& address) const override {return nullptr;}    //XXX should be find..., see next one

        /**
         * Prints the routing table.
         */
        virtual void printRoutingTable() const override;
        //@}

        /** @name Routing functions (query the route table) */
        //@{
        /**
         * The routing function. Performs longest prefix match for the given
         * destination address, and returns the resulting route. Returns nullptr
         * if there is no matching route.
         */
        virtual IRoute *findBestMatchingRoute(const L3Address& dest) const override { return findBestMatchingRoute(dest.toCLNS()); }
        virtual CLNSRoute *findBestMatchingRoute(const CLNSAddress& dest) const;


        /**
         * Convenience function based on findBestMatchingRoute().
         *
         * Returns the output interface for the packets with dest as destination
         * address, or nullptr if the destination is not in routing table.
         */
        virtual InterfaceEntry *getOutputInterfaceForDestination(const L3Address& dest) const override { return nullptr;}    //XXX redundant

        /**
         * Convenience function based on findBestMatchingRoute().
         *
         * Returns the gateway for the destination address. Returns the unspecified
         * address if the destination is not in routing table or the gateway field
         * is not filled in in the route.
         */
        virtual L3Address getNextHopForDestination(const L3Address& dest) const override { return CLNSAddress::UNSPECIFIED_ADDRESS;}    //XXX redundant AND unused
        //@}

        /** @name Multicast routing functions */
        //@{

        /**
         * Checks if the address is in one of the local multicast group
         * address list.
         */
        virtual bool isLocalMulticastAddress(const L3Address& dest) const override { return false;}

        /**
         * Returns route for a multicast origin and group.
         */
        virtual IMulticastRoute *findBestMatchingMulticastRoute(const L3Address& origin, const L3Address& group) const override  { return nullptr;}
        //@}

        /** @name Route table manipulation */
        //@{

        /**
         * Returns the total number of unicast routes.
         */
        virtual int getNumRoutes() const override { return routes.size(); }

        /**
         * Returns the kth route.
         */
        virtual CLNSRoute *getRoute(int k) const override;

        /**
         * Finds and returns the default route, or nullptr if it doesn't exist
         */
        virtual IRoute *getDefaultRoute() const override  { return nullptr;}    //XXX is this a universal concept?

        /**
         * Adds a route to the routing table. Routes are allowed to be modified
         * while in the routing table. (There is a notification mechanism that
         * allows routing table internals to be updated on a routing entry change.)
         */
        virtual void addRoute(IRoute *entry) override {};

        /**
         * Removes the given route from the routing table, and returns it.
         * nullptr is returned if the route was not in the routing table.
         */
        virtual IRoute *removeRoute(IRoute *entry) override { return nullptr;}

        /**
         * Deletes the given route from the routing table.
         * Returns true if the route was deleted, and false if it was
         * not in the routing table.
         */
        virtual bool deleteRoute(IRoute *entry) override {return true;}

        /**
         * Returns the total number of multicast routes.
         */
        virtual int getNumMulticastRoutes() const override  {return 0;}

        /**
         * Returns the kth multicast route.
         */
        virtual IMulticastRoute *getMulticastRoute(int k) const override { return nullptr;}

        /**
         * Adds a multicast route to the routing table. Routes are allowed to be modified
         * while in the routing table. (There is a notification mechanism that
         * allows routing table internals to be updated on a routing entry change.)
         */
        virtual void addMulticastRoute(IMulticastRoute *entry) override {}

        /**
         * Removes the given route from the routing table, and returns it.
         * nullptr is returned of the route was not in the routing table.
         */
        virtual IMulticastRoute *removeMulticastRoute(IMulticastRoute *entry) override { return nullptr;}

        /**
         * Deletes the given multicast route from the routing table.
         * Returns true if the route was deleted, and false if it was
         * not in the routing table.
         */
        virtual bool deleteMulticastRoute(IMulticastRoute *entry) override {return true;}
        //@}

        virtual IRoute *createRoute() override { return new CLNSRoute(); }
    const CLNSAddress& getRouterId() const;
    void setRouterId(const CLNSAddress& routerId);
    /**
     * ILifecycle method
     */
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;
};

} //namespace

#endif
