/**
 * @file MulticastRoutingTable.cc
 * @brief File contains implementation of multicast routing table.
 * @date 10.3.2012
 * @author Haczek
 */

#include "MulticastRoutingTable.h"

Define_Module(MulticastRoutingTable);

using namespace std;

/** Printout of structure MulticastIPRoute (one route in table). */
std::ostream& operator<<(std::ostream& os, const MulticastIPRoute& e)
{
	return os;
};

/**
 * MULTICAST ROUTING TABLE DESTRUCTOR
 *
 * The method deletes Multicast Routing Table. Delete all entries in table.
 */
MulticastRoutingTable::~MulticastRoutingTable()
{
    for (unsigned int i=0; i<multicastRoutes.size(); i++)
        delete multicastRoutes[i];
}

/**
 * INITIALIZE
 *
 * The method initializes Multicast Routing Table module. It get access to all needed objects.
 *
 * @param stage Stage of initialization.
 */
void MulticastRoutingTable::initialize(int stage)
{
    if (stage==0)
    {
        // get a pointer to IInterfaceTable
        ift = AnsaInterfaceTableAccess().get();

        // watch multicast table
        //WATCH_PTRVECTOR(multicastRoutes);
        WATCH_VECTOR(showMRoute);
    }
}

/**
 * UPDATE DISPLAY STRING
 *
 * Update string under multicast table icon - number of multicast routes.
 */
void MulticastRoutingTable::updateDisplayString()
{
    if (!ev.isGUI())
        return;

    char buf[80];
    sprintf(buf, "%d routes", multicastRoutes.size());
    getDisplayString().setTagArg("t",0,buf);
}

/**
 * PRINT ROUTING TABLE
 *
 * Can be used for debugging purposes.
 */
void MulticastRoutingTable::printRoutingTable() const
{
    EV << "-- Multicast routing table --\n";
    for (int i=0; i<getNumRoutes(); i++)
        EV << getRoute(i)->detailedInfo() << "\n";
    EV << "\n";
}

/**
 * HANDLE MESSAGE
 *
 * Module does not have any gate, it cannot get messages.
 */
void MulticastRoutingTable::handleMessage(cMessage *msg)
{
    opp_error("This module doesn't process messages");
}

/**
 * FIND ROUTE
 *
 * Finds route according to given parameters (source, group, RP, ID and next hop of incoming int).
 *
 * @param source IP address of multicast source.
 * @param group IP address of multicast group.
 * @param RP IP address of RP router.
 * @param intId ID of incoming (RPF) interface.
 * @param nextHop IP address of RPF neighbor.
 * @return Pointer to route in multicast table.
 * @see getRoute()
 * @see routeMatches()
 * @see getNumRoutes()
 */
const MulticastIPRoute *MulticastRoutingTable::findRoute(const IPAddress& source, const IPAddress& group,
    const IPAddress& RP, int intId, const IPAddress& nextHop) const
{
    int n = getNumRoutes();
    for (int i=0; i<n; i++)
        if (routeMatches(getRoute(i), source, group, RP, intId, nextHop))
            return getRoute(i);
    return NULL;
}

/**
 * GET NUMBER OF ROUTES
 *
 * Returns number of entries in multicast routing table.
 */
int MulticastRoutingTable::getNumRoutes() const
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
MulticastIPRoute *MulticastRoutingTable::getRoute(int k) const
{
    if (k < (int)multicastRoutes.size())
        return multicastRoutes[k];

    return NULL;
}

/**
 * ROUTE MATCHES
 *
 * Finds a match between route entry and given parameters.
 *
 * @param entry Link to route.
 * @param source IP address of multicast source.
 * @param group IP address of multicast group.
 * @param RP IP address of RP router.
 * @param intId ID of incoming (RPF) interface.
 * @param nextHop IP address of RPF neighbor.
 * @return Pointer to route in multicast table.
 */
bool MulticastRoutingTable::routeMatches(const MulticastIPRoute *entry,
    const IPAddress& source, const IPAddress& group,
    const IPAddress& RP, int intId, const IPAddress& nextHop) const
{
    if (!source.isUnspecified() && !source.equals(entry->getSource()))
        return false;
    if (!group.isUnspecified() && !group.equals(entry->getGroup()))
        return false;
    if (!RP.isUnspecified() && !RP.equals(entry->getRP()))
        return false;
    if (intId!=entry->getInIntId())
        return false;
    if (!nextHop.isUnspecified() && !nextHop.equals(entry->getInIntNextHop()))
        return false;
    return true;
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
vector<MulticastIPRoute*> MulticastRoutingTable::getRouteFor(IPAddress group)
{
    Enter_Method("getMulticastRoutesFor(%x)", group.getInt()); // note: str().c_str() too slow here here
    EV << "MulticastRoutingTable::getRouteFor - address = " << group << endl;
    vector<MulticastIPRoute*> routes;

    // search in multicast table
    int n = multicastRoutes.size();
    for (int i = 0; i < n; i++)
    {
    	MulticastIPRoute *route = getRoute(i);
    	if (route->getGroup().getInt() == group.getInt())
    		routes.push_back(route);
    }

    return routes;
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
MulticastIPRoute *MulticastRoutingTable::getRouteFor(IPAddress group, IPAddress source)
{
    Enter_Method("getMulticastRoutesFor(%x, %x)", group.getInt(), source.getInt()); // note: str().c_str() too slow here here
    EV << "MulticastRoutingTable::getRouteFor - group = " << group << ", source = " << source << endl;

    // search in multicast routing table
    MulticastIPRoute *route = NULL;
    int n = multicastRoutes.size();
    int i;
    // go through all multicast routes
    for (i = 0; i < n; i++)
    {
    	route = getRoute(i);
    	if (route->getGroup().getInt() == group.getInt() && route->getSource().getInt() == source.getInt())
    		break;
    }

    if (i == n)
    	return NULL;
    return route;
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
std::vector<MulticastIPRoute*> MulticastRoutingTable::getRoutesForSource(IPAddress source)
{
	Enter_Method("getRoutesForSource(%x)", source.getInt()); // note: str().c_str() too slow here here
	EV << "MulticastRoutingTable::getRoutesForSource - source = " << source << endl;
	vector<MulticastIPRoute*> routes;

	// search in multicast table
	int n = multicastRoutes.size();
	int i;
	for (i = 0; i < n; i++)
	{
		//FIXME works only for classfull adresses (function getNetwork) !!!!
		MulticastIPRoute *route = getRoute(i);
		if (route->getSource().getNetwork().getInt() == source.getInt())
			routes.push_back(route);
	}
	return routes;
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
void MulticastRoutingTable::addRoute(const MulticastIPRoute *entry)
{
    Enter_Method("addMulticastRoute(...)");

    // check for null multicast group address
    if (entry->getGroup().isUnspecified())
        error("addMulticastRoute(): multicast group address cannot be NULL");

    // check that group address is multicast address
	if (!entry->getGroup().isMulticast())
		error("addMulticastRoute(): group address is not multicast address");

    // check for source or RP address
    if (entry->getSource().isUnspecified() && entry->getRP().isUnspecified())
    	error("addMulticastRoute(): source or RP address has to be specified");

    // check that the incoming interface exists
    if (!entry->getInIntPtr() || entry->getInIntNextHop().isUnspecified())
        error("addMulticastRoute(): incoming interface has to be specified");

    // add to tables
    multicastRoutes.push_back(const_cast<MulticastIPRoute*>(entry));

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
bool MulticastRoutingTable::deleteRoute(const MulticastIPRoute *entry)
{
    Enter_Method("deleteMulticastRoute(...)");

    // find entry in routing table
    vector<MulticastIPRoute*>::iterator i;
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
 * GENERATE SHOW IP MROUTE
 *
 * This method should be called after each change of multicast routing table. It is output which
 * represents state of the table. Format is same as format on Cisco routers.
 */
void MulticastRoutingTable::generateShowIPMroute()
{
	EV << "MulticastRoutingTable::generateShowIPRoute()" << endl;
	showMRoute.clear();

	int n = getNumRoutes();
	const MulticastIPRoute* ipr;

	for (int i=0; i<n; i++)
	{
		ipr = getRoute(i);
		stringstream os;
		os << "(";
		if (ipr->getSource().isUnspecified()) os << "*, "; else os << ipr->getSource() << ",  ";
		os << ipr->getGroup() << "), ";
		if (!ipr->getRP().isUnspecified()) os << "RP is " << ipr->getRP()<< ", ";
		os << "flags: ";
		vector<flag> flags = ipr->getFlags();
		for (unsigned int j = 0; j < flags.size(); j++)
		{
			EV << "MulticastRoutingTable::generateShowIPRoute(): Flag = " << flags[j] << endl;
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
			}
		}
		os << endl;

		os << "Incoming interface: ";
		if (ipr->getInIntPtr()) os << ipr->getInIntPtr()->getName() << ", ";
		os << "RPF neighbor " << ipr->getInIntNextHop() << endl;
		os << "Outgoing interface list:" << endl;

		InterfaceVector all = ipr->getOutInt();
		if (all.size() == 0)
			os << "Null" << endl;
		else
			for (unsigned int k = 0; k < all.size(); k++)
			{
				os << all[k].intPtr->getName() << ", ";
				if (all[k].forwarding == Forward) os << "Forward/"; else os << "Pruned/";
				if (all[k].mode == Densemode) os << "Dense"; else os << "Sparse";
				os << endl;
			}
		showMRoute.push_back(os.str());
	}
	stringstream out;
}
