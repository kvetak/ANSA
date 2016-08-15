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


#include "inet/common/INETUtils.h"

//#include "inet/common/PatternMatcher.h"
#include "ansa/networklayer/clns/CLNSRoutingTable.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "ansa/networklayer/clns/CLNSInterfaceData.h"
#include "ansa/networklayer/clns/CLNSRoute.h"
#include "inet/common/NotifierConsts.h"
//#include "inet/networklayer/ipv4/RoutingTableParser.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"

#ifdef ANSAINET
#include "ansa/networklayer/common/ANSA_InterfaceEntry.h"
#endif
namespace inet {

using namespace utils;

Define_Module(CLNSRoutingTable);

std::ostream& operator<<(std::ostream& os, const CLNSRoute& e)
{
    os << e.info();
    return os;
};

const CLNSAddress& CLNSRoutingTable::getRouterId() const
{
  return routerId;
}

void CLNSRoutingTable::setRouterId(const CLNSAddress& routerId)
{
  this->routerId = routerId;
}

CLNSRoutingTable::~CLNSRoutingTable()
{
    for (auto & elem : routes)
        delete elem;

}

void CLNSRoutingTable::initialize(int stage)
{
  cSimpleModule::initialize(stage);

  if (stage == INITSTAGE_LOCAL) {
      // get a pointer to the host module and IInterfaceTable
      cModule *host = getContainingNode(this);
      host->subscribe(NF_INTERFACE_CREATED, this);
      host->subscribe(NF_INTERFACE_DELETED, this);
      host->subscribe(NF_INTERFACE_STATE_CHANGED, this);
      host->subscribe(NF_INTERFACE_CONFIG_CHANGED, this);
      host->subscribe(NF_INTERFACE_CLNSCONFIG_CHANGED, this);

      ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);

//      netmaskRoutes = par("netmaskRoutes");
//      forwarding = par("forwarding").boolValue();
//      multicastForward = par("multicastForwarding");
//      useAdminDist = par("useAdminDist");

      WATCH_PTRVECTOR(routes);
//      WATCH_PTRVECTOR(multicastRoutes);
//      WATCH(netmaskRoutes);
//      WATCH(forwarding);
//      WATCH(multicastForward);
//      WATCH(routerId);
  }
  else if (stage == INITSTAGE_NETWORK_LAYER) {
      NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
      isNodeUp = !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
      if (isNodeUp) {
          // set routerId if param is not "" (==no routerId) or "auto" (in which case we'll
          // do it later in a later stage, after network configurators configured the interfaces)
        //TODO: A1 What about routerID?
//          const char *routerIdStr = par("routerId").stringValue();
//          if (strcmp(routerIdStr, "") && strcmp(routerIdStr, "auto"))
//              routerId = CLNSAddress(routerIdStr);
      }
  }
  else if (stage == INITSTAGE_NETWORK_LAYER_3) {
      if (isNodeUp) {
          // read routing table file (and interface configuration)

        //TODO A1 load routing table from a file
//          const char *filename = par("routingFile");
//          RoutingTableParser parser(ift, this);
//          if (*filename && parser.readRoutingTableFromFile(filename) == -1)
//              throw cRuntimeError("Error reading routing table file %s", filename);
      }

      // routerID selection must be after network autoconfiguration assigned interface addresses
//      if (isNodeUp)
//          configureRouterId();

      // we don't use notifications during initialize(), so we do it manually.
//      updateNetmaskRoutes();
  }
}

void CLNSRoutingTable::invalidateCache()
{
    routingCache.clear();
//    localAddresses.clear();
//    localBroadcastAddresses.clear();
}

bool CLNSRoutingTable::isLocalAddress(const CLNSAddress& dest) const
{
    Enter_Method("isLocalAddress()");    // note: str().c_str() too slow here
//
//    if (localAddresses.empty()) {
//        // collect interface addresses if not yet done
//        for (int i = 0; i < ift->getNumInterfaces(); i++) {
//            CLNSAddress interfaceAddr = ift->getInterface(i)->ipv4Data()->getIPAddress();
//            localAddresses.insert(interfaceAddr);
//        }
//    }
//#if defined(ANSAINET)
//    for (int i=0; i<ift->getNumInterfaces(); i++)
//    {
//        ANSA_InterfaceEntry* ieVF = dynamic_cast<ANSA_InterfaceEntry *>(ift->getInterface(i));
//        if (ieVF && ieVF->hasIPAddress(dest))
//            return true;
//    }
//#endif
    auto it = localAddresses.find(dest);
    return it != localAddresses.end();
}

void CLNSRoutingTable::addLocalAddress(const CLNSAddress& address)
{
    localAddresses.insert(address);
}


void CLNSRoutingTable::printRoutingTable() const
{
  Enter_Method("");
    EV << "-- Routing table --\n";
    EV << stringf("%-16s %-16s %-4s %-16s %s\n",
            "Destination", "Gateway", "Iface", "", "Metric");

    for (int i = 0; i < getNumRoutes(); i++) {
        CLNSRoute *route = getRoute(i);
        InterfaceEntry *interfacePtr = route->getInterface();
        EV << stringf("%-26s %-26s %-4s (.%d) %d\n",
                route->getDestination().isUnspecified() ? "*" : route->getDestination().str().c_str(),
                route->getGateway().isUnspecified() ? "*" : route->getGateway().str().c_str(),
                !interfacePtr ? "*" : interfacePtr->getName(),
                !interfacePtr ? 99 : interfacePtr->clnsData()->getCircuitId(),
                route->getMetric());
    }
    EV << "\n";
}


CLNSRoute *CLNSRoutingTable::getRoute(int k) const
{
    if (k < (int)routes.size())
        return routes[k];
    return nullptr;
}

void CLNSRoutingTable::addRoute(CLNSRoute *entry)
{
    Enter_Method("addRoute(...)");

    internalAddRoute(entry);

    invalidateCache();
    updateDisplayString();

    emit(NF_ROUTE_ADDED, entry);
}


void CLNSRoutingTable::internalAddRoute(CLNSRoute *entry)
{
//    if (!entry->getNetmask().isValidNetmask())
//        throw cRuntimeError("addRoute(): wrong netmask %s in route", entry->getNetmask().str().c_str());

//    if (entry->getNetmask().getInt() != 0 && (entry->getDestination().getInt() & entry->getNetmask().getInt()) == 0)
//        throw cRuntimeError("addRoute(): all bits of destination address %s is 0 inside non zero netmask %s",
//                entry->getDestination().str().c_str(), entry->getNetmask().str().c_str());

//    if ((entry->getDestination().getInt() & ~entry->getNetmask().getInt()) != 0)
//        throw cRuntimeError("addRoute(): suspicious route: destination IP address %s has bits set outside netmask %s",
//                entry->getDestination().str().c_str(), entry->getNetmask().str().c_str());

    // check that the interface exists
    if (!entry->getInterface())
        throw cRuntimeError("addRoute(): interface cannot be nullptr");

    // if this is a default route, remove old default route (we're replacing it)
//    if (entry->getNetmask().isUnspecified()) {
//        CLNSRoute *oldDefaultRoute = getDefaultRoute();
//        if (oldDefaultRoute != nullptr)
//            deleteRoute(oldDefaultRoute);
//    }

    // The 'routes' vector may contain multiple routes with the same destination/netmask.
    // Routes are stored in descending netmask length and ascending administrative_distance/metric order,
    // so the first matching is the best one.
    // XXX Should only the route with the best metric be stored? Then the worse route should be deleted and
    //     internalAddRoute() should return a bool indicating if it was successful.

    // add to tables
    // we keep entries sorted by netmask desc, metric asc in routeList, so that we can
    // stop at the first match when doing the longest netmask matching
    auto pos = upper_bound(routes.begin(), routes.end(), entry, RouteLessThan(*this));
    routes.insert(pos, entry);

    entry->setRoutingTable(this);
}

CLNSRoute *CLNSRoutingTable::findBestMatchingRoute(const CLNSAddress& dest) const
{
    Enter_Method("findBestMatchingRoute");

    auto it = routingCache.find(dest);
    if (it != routingCache.end()) {
        if (it->second == nullptr || it->second->isValid())
            return it->second;
    }

    // find best match (one with longest prefix)
    // default route has zero prefix length, so (if exists) it'll be selected as last resort
    CLNSRoute *bestRoute = nullptr;
    for (auto e : routes) {
        if (e->isValid()) {
//            if (CLNSAddress::maskedAddrAreEqual(dest, e->getDestination(), e->getNetmask())) {    // match
          if (dest == e->getDestination()) {    // match
                bestRoute = const_cast<CLNSRoute *>(e);
                break;
            }
        }
    }

    routingCache[dest] = bestRoute;
    return bestRoute;
}



// The 'routes' vector stores the routes in this order.
// The best matching route should precede the other matching routes,
// so the method should return true if a is better the b.
bool CLNSRoutingTable::routeLessThan(const CLNSRoute *a, const CLNSRoute *b) const
{
    // longer prefixes are better, because they are more specific
//    if (a->getNetmask() != b->getNetmask())
//        return a->getNetmask() > b->getNetmask();

    if (a->getDestination() != b->getDestination())
        return a->getDestination() < b->getDestination();

    // for the same destination/netmask:

    // smaller administration distance is better (if useAdminDist)
    if (useAdminDist && (a->getAdminDist() != b->getAdminDist()))
        return a->getAdminDist() < b->getAdminDist();

    // smaller metric is better
    return a->getMetric() < b->getMetric();
}




void CLNSRoutingTable::handleMessage(cMessage *msg)
{
  throw cRuntimeError("This module doesn't process messages");
}

void CLNSRoutingTable::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG)
{
    if (getSimulation()->getContextType() == CTX_INITIALIZE)
        return; // ignore notifications during initialize

    Enter_Method_Silent();
    printNotificationBanner(signalID, obj);

    if (signalID == NF_INTERFACE_CREATED) {
        // add netmask route for the new interface
      //TODO A1 NETMASK
//        updateNetmaskRoutes();
    }
    else if (signalID == NF_INTERFACE_DELETED) {
        // remove all routes that point to that interface
        const InterfaceEntry *entry = check_and_cast<const InterfaceEntry *>(obj);
//        deleteInterfaceRoutes(entry);
    }
    else if (signalID == NF_INTERFACE_STATE_CHANGED) {
        invalidateCache();
    }
    else if (signalID == NF_INTERFACE_CONFIG_CHANGED) {
        invalidateCache();
    }
    else if (signalID == NF_INTERFACE_CLNSCONFIG_CHANGED) {
        // if anything CLNS-related changes in the interfaces, interface netmask
        // based routes have to be re-built.
//        updateNetmaskRoutes();
    }
}

bool CLNSRoutingTable::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
//    if (dynamic_cast<NodeStartOperation *>(operation)) {
//        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_NETWORK_LAYER) {
//            // read routing table file (and interface configuration)
//            const char *filename = par("routingFile");
//            RoutingTableParser parser(ift, this);
//            if (*filename && parser.readRoutingTableFromFile(filename) == -1)
//                throw cRuntimeError("Error reading routing table file %s", filename);
//        }
//        else if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_TRANSPORT_LAYER) {
//            configureRouterId();
//            updateNetmaskRoutes();
//            isNodeUp = true;
//        }
//    }
//    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
//        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_NETWORK_LAYER) {
//            while (!routes.empty())
//                delete removeRoute(routes[0]);
//            isNodeUp = false;
//        }
//    }
//    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
//        if ((NodeCrashOperation::Stage)stage == NodeCrashOperation::STAGE_CRASH) {
//            while (!routes.empty())
//                delete removeRoute(routes[0]);
//            isNodeUp = false;
//        }
//    }
    return true;
}

//CLNSRoute *CLNSRoutingTable::createNewRoute()
//{
//    return new CLNSRoute();
//}

void CLNSRoutingTable::updateDisplayString()
{
    if (!hasGUI())
        return;

    char buf[80];
    if (routerId.isUnspecified())
        sprintf(buf, "%d routes", (int)routes.size());
    else
        sprintf(buf, "routerId: %s\n%d routes", routerId.str().c_str(), (int)routes.size());
    getDisplayString().setTagArg("t", 0, buf);
}



} //namespace
