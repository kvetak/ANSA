// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
/**
 * @file AnsaRoutingTable.cc
 * @date 25.1.2013
 * @author Tomas Prochazka (mailto:xproch21@stud.fit.vutbr.cz), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @brief Extended RoutingTable with new features for PIM
 */

#include "AnsaRoutingTable.h"

Define_Module(AnsaRoutingTable);

/** Printout of structure AnsaIPv4MulticastRoute (one route in table). */
std::ostream& operator<<(std::ostream& os, const AnsaIPv4MulticastRoute& e)
{
    os << e.info();
    return os;
};

/**
 * MULTICAST ROUTING TABLE DESTRUCTOR
 *
 * The method deletes Ansa Routing Table. Delete all entries in table.
 */
AnsaRoutingTable::~AnsaRoutingTable()
{
    for (unsigned int i=0; i<routes.size(); i++)
        delete routes[i];
    for (unsigned int i=0; i<multicastRoutes.size(); i++)
        delete multicastRoutes[i];
}

/**
 * GET NUMBER OF ROUTES
 *
 * Returns number of entries in multicast routing table.
 */
int AnsaRoutingTable::getNumRoutes() const
{
    return multicastRoutes.size();
}

/**
 * GET ROUTE
 *
 * Returns the k-th route. The returned route cannot be modified;
 * you must delete and re-add it instead. This rule is emphasized
 * by returning a const pointer.
 */
AnsaIPv4MulticastRoute *AnsaRoutingTable::getMulticastRoute(int k) const
{
    if (k < (int)multicastRoutes.size())
        return multicastRoutes[k];

    return NULL;
}

/**
 * UPDATE DISPLAY STRING
 *
 * Update string under multicast table icon - number of multicast routes.
 */
void AnsaRoutingTable::updateDisplayString()
{
    if (!ev.isGUI())
        return;

    char buf[80];
    sprintf(buf, "%d routes", multicastRoutes.size());
    getDisplayString().setTagArg("t",0,buf);
}



/**
 * INITIALIZE
 *
 * The method initializes Multicast Routing Table module. It get access to all needed objects.
 *
 * @param stage Stage of initialization.
 */
void AnsaRoutingTable::initialize(int stage)
{
    if (stage==0)
    {
        // get a pointer to IInterfaceTable
        ift = InterfaceTableAccess().get();

        nb = NotificationBoardAccess().get();

        IPForward = par("IPForward").boolValue();

        nb->subscribe(this, NF_INTERFACE_CREATED);
        nb->subscribe(this, NF_INTERFACE_DELETED);
        nb->subscribe(this, NF_INTERFACE_STATE_CHANGED);
        nb->subscribe(this, NF_INTERFACE_CONFIG_CHANGED);
        nb->subscribe(this, NF_INTERFACE_IPv4CONFIG_CHANGED);

        // watch multicast table
        WATCH_VECTOR(showMRoute);
        WATCH_PTRVECTOR(multicastRoutes);
        WATCH(IPForward);
        WATCH(routerId);
    }
    else if (stage==3)
    {
        // routerID selection must be after stage==2 when network autoconfiguration
        // assigns interface addresses
        configureRouterId();

        // we don't use notifications during initialize(), so we do it manually.
        // Should be in stage=3 because autoconfigurator runs in stage=2.
        updateNetmaskRoutes();

        //printRoutingTable();
    }
}


/**
 * ADD ROUTE
 *
 * Function check new multicast table entry and then add new entry to multicast table.
 *
 * @param entry New entry about new multicast group.
 * @see MulticastIPRoute
 * @see updateDisplayString()
 * @see generateShowIPMroute()
 */
void AnsaRoutingTable::addMulticastRoute(const AnsaIPv4MulticastRoute *entry)
{
    Enter_Method("addMulticastRoute(...)");

    // check for null multicast group address
    if (entry->getMulticastGroup().isUnspecified())
        error("addMulticastRoute(): multicast group address cannot be NULL");

    // check that group address is multicast address
    if (!entry->getMulticastGroup().isMulticast())
        error("addMulticastRoute(): group address is not multicast address");

    // check for source or RP address
    if (entry->getOrigin().isUnspecified() && entry->getRP().isUnspecified())
        error("addMulticastRoute(): source or RP address has to be specified");

    // check that the incoming interface exists
    //FIXME for PIM-SM is needed unspecified next hop (0.0.0.0)
    //if (!entry->getInIntPtr() || entry->getInIntNextHop().isUnspecified())
        //error("addMulticastRoute(): incoming interface has to be specified");
    //if (!entry->getInIntPtr())
        //error("addMulticastRoute(): incoming interface has to be specified");

    // add to tables
    multicastRoutes.push_back(const_cast<AnsaIPv4MulticastRoute*>(entry));

    updateDisplayString();
    generateShowIPMroute();
}



/**
 * DELETE ROUTE
 *
 * Function check new multicast table entry and then add new entry to multicast table.
 *
 * @param entry Multicast entry which should be deleted from multicast table.
 * @return False if entry was not found in table. True if entry was deleted.
 * @see MulticastIPRoute
 * @see updateDisplayString()
 * @see generateShowIPMroute()
 */
bool AnsaRoutingTable::deleteMulticastRoute(const AnsaIPv4MulticastRoute *entry)
{
    Enter_Method("deleteMulticastRoute(...)");

    // find entry in routing table
    std::vector<AnsaIPv4MulticastRoute*>::iterator i;
    for (i=multicastRoutes.begin(); i!=multicastRoutes.end(); ++i)
    {
        if ((*i) == entry)
            break;
    }

    // if entry was found, it can be deleted
    if (i!=multicastRoutes.end())
    {
        // first delete all timers assigned to route
        if (entry->getSrt() != NULL)
        {
            cancelEvent(entry->getSrt());
            delete entry->getSrt();
        }
        if (entry->getGrt() != NULL)
        {
            cancelEvent(entry->getGrt());
            delete entry->getGrt();
        }
        if (entry->getSat())
        {
            cancelEvent(entry->getSat());
            delete entry->getSat();
        }
        if (entry->getKat())
        {
            cancelEvent(entry->getKat());
            delete entry->getKat();
        }
        if (entry->getEt())
        {
            cancelEvent(entry->getEt());
            delete entry->getEt();
        }

        // delete timers from outgoing interfaces
        InterfaceVector outInt = entry->getOutInt();
        for (unsigned int j = 0;j < outInt.size(); j++)
        {
            if (outInt[j].pruneTimer != NULL)
            {
                cancelEvent(outInt[j].pruneTimer);
                delete outInt[j].pruneTimer;
            }
        }

        // delete route
        multicastRoutes.erase(i);
        delete entry;
        updateDisplayString();
        generateShowIPMroute();
        return true;
    }
    return false;
}

/**
 * GET ROUTE FOR
 *
 * The method returns one route from multicast routing table for given group and source IP addresses.
 *
 * @param group IP address of multicast group.
 * @param source IP address of multicast source.
 * @return Pointer to found multicast route.
 * @see getRoute()
 */
AnsaIPv4MulticastRoute *AnsaRoutingTable::getRouteFor(IPv4Address group, IPv4Address source)
{
    Enter_Method("getMulticastRoutesFor(%x, %x)", group.getInt(), source.getInt()); // note: str().c_str() too slow here here
    EV << "MulticastRoutingTable::getRouteFor - group = " << group << ", source = " << source << endl;

    // search in multicast routing table
    AnsaIPv4MulticastRoute *route = NULL;

    int n = multicastRoutes.size();
    int i;
    // go through all multicast routes
    for (i = 0; i < n; i++)
    {
        route = getMulticastRoute(i);
        if (route->getMulticastGroup().getInt() == group.getInt() && route->getOrigin().getInt() == source.getInt())
            break;
    }

    if (i == n)
        return NULL;
    return route;
}

/**
 * GET ROUTE FOR
 *
 * The method returns all routes from multicast routing table for given multicast group.
 *
 * @param group IP address of multicast group.
 * @return Vecotr of pointers to routes in multicast table.
 * @see getRoute()
 */
std::vector<AnsaIPv4MulticastRoute*> AnsaRoutingTable::getRouteFor(IPv4Address group)
{
    Enter_Method("getMulticastRoutesFor(%x)", group.getInt()); // note: str().c_str() too slow here here
    EV << "MulticastRoutingTable::getRouteFor - address = " << group << endl;
    std::vector<AnsaIPv4MulticastRoute*> routes;

    // search in multicast table
    int n = multicastRoutes.size();
    for (int i = 0; i < n; i++)
    {
        AnsaIPv4MulticastRoute *route = getMulticastRoute(i);
        if (route->getMulticastGroup().getInt() == group.getInt())
            routes.push_back(route);
    }

    return routes;
}

/**
 * GENERATE SHOW IP MROUTE
 *
 * This method should be called after each change of multicast routing table. It is output which
 * represents state of the table. Format is same as format on Cisco routers.
 */
void AnsaRoutingTable::generateShowIPMroute()
{
    EV << "MulticastRoutingTable::generateShowIPRoute()" << endl;
    showMRoute.clear();

    int n = getNumRoutes();
    const AnsaIPv4MulticastRoute* ipr;

    for (int i=0; i<n; i++)
    {
        ipr = getMulticastRoute(i);
        std::stringstream os;
        os << "(";
        if (ipr->getOrigin().isUnspecified()) os << "*, "; else os << ipr->getOrigin() << ",  ";
        os << ipr->getMulticastGroup() << "), ";
        if (!ipr->getRP().isUnspecified()) os << "RP is " << ipr->getRP()<< ", ";
        os << "flags: ";
        std::vector<flag> flags = ipr->getFlags();
        for (unsigned int j = 0; j < flags.size(); j++)
        {
            switch(flags[j])
            {
                case D:
                    os << "D";
                    break;
                case S:
                    os << "S";
                    break;
                case C:
                    os << "C";
                    break;
                case P:
                    os << "P";
                    break;
                case A:
                    os << "A";
                    break;
                case F:
                    os << "F";
                    break;
                case T:
                    os << "T";
                    break;
                //FIXME next flags for PIM-SM
            }
        }
        os << endl;

        os << "Incoming interface: ";
        if (ipr->getInIntPtr()) os << ipr->getInIntPtr()->getName() << ", ";
        else os << "Null, ";
        if (ipr->getInIntNextHop().isUnspecified()) os << "RPF neighbor 0.0.0.0" << endl;
        else os << "RPF neighbor " << ipr->getInIntNextHop() << endl;
        os << "Outgoing interface list:" << endl;

        InterfaceVector all = ipr->getOutInt();
        if (all.size() == 0)
            os << "Null" << endl;
        else
            if (ipr->getOutShowIntStatus())                         // hack for PIM-SM output - problem with register state and outgoing interface
            {
                for (unsigned int k = 0; k < all.size(); k++)
                {
                    os << all[k].intPtr->getName() << ", ";
                    if (all[k].forwarding == Forward) os << "Forward/"; else os << "Pruned/";
                    if (all[k].mode == Densemode) os << "Dense"; else os << "Sparse";
                    os << endl;
                }
            }
            else
                os << "Null" << endl;
        showMRoute.push_back(os.str());
    }
    std::stringstream out;
}

/**
 * GET ROUTES FOR SOURCES
 *
 * The method returns all routes from multicast routing table for given source.
 *
 * @param source IP address of multicast source.
 * @return Vector of found multicast routes.
 * @see getNetwork()
 */
std::vector<AnsaIPv4MulticastRoute*> AnsaRoutingTable::getRoutesForSource(IPv4Address source)
{
    Enter_Method("getRoutesForSource(%x)", source.getInt()); // note: str().c_str() too slow here here
    EV << "MulticastRoutingTable::getRoutesForSource - source = " << source << endl;
    std::vector<AnsaIPv4MulticastRoute*> routes;

    // search in multicast table
    int n = multicastRoutes.size();
    int i;
    for (i = 0; i < n; i++)
    {
        //FIXME works only for classfull adresses (function getNetwork) !!!!
        AnsaIPv4MulticastRoute *route = getMulticastRoute(i);
        if (route->getOrigin().getNetwork().getInt() == source.getInt())
            routes.push_back(route);
    }
    return routes;
}
