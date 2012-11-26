/**
 * @file MulticastRoutingTable.h
 * @brief File contains implementation of multicast routing table.
 * @date 10.3.2012
 * @author Haczek
 */

#ifndef __MULTICASTROUTINGTABLE_H
#define __MULTICASTROUTINGTABLE_H

#include <omnetpp.h>
#include "MulticastIPRoute.h"
#include "IPAddress.h"
#include "NotificationBoard.h"
#include "AnsaInterfaceTableAccess.h"
#include "AnsaInterfaceTable.h"

/**
 * RouteVector represents multicast table. It is list of multicast routes.
 */
typedef std::vector<MulticastIPRoute *> RouteVector;

/**
 * @brief Class represent multicast routing table.
 * @details It contains entries = routes for each
 * multicast source and group. There are methods to get right route from table, add
 * new route, delete old on, etc.
 */
class INET_API MulticastRoutingTable: public cSimpleModule
{
	protected:
		RouteVector multicastRoutes;						/**< Multicast routing table. */
		std::vector<std::string> showMRoute;				/**< Output of multicast routing table, same as Cisco mroute. */
		IInterfaceTable *ift; 								/**< Pointer to interface table. */

	protected:
	    virtual bool routeMatches(const MulticastIPRoute *entry,
	    	    const IPAddress& source, const IPAddress& group,
	    	    const IPAddress& RP, int intId, const IPAddress& nextHop) const;
	    virtual void updateDisplayString();


	public:
		// print multicast routing table
		void generateShowIPMroute();
		virtual void printRoutingTable() const;

		// Returns routes for a multicast address.
		virtual std::vector<MulticastIPRoute*> getRouteFor(IPAddress group);
		virtual MulticastIPRoute *getRouteFor(IPAddress group, IPAddress source);
		virtual std::vector<MulticastIPRoute*> getRoutesForSource(IPAddress source);

		// for manipulation with the table
		virtual int getNumRoutes() const;
		virtual MulticastIPRoute *getRoute(int k) const;
		virtual const MulticastIPRoute *findRoute(const IPAddress& source, const IPAddress& group,
				const IPAddress& RP, int intId, const IPAddress& nextHop) const;

		// the most important !!!
		virtual void addRoute(const MulticastIPRoute *entry);
		virtual bool deleteRoute(const MulticastIPRoute *entry);

	public:
	    MulticastRoutingTable(){};
	    virtual ~MulticastRoutingTable();

	protected:
	    virtual int numInitStages() const  {return 4;}
	    virtual void initialize(int stage);
	    virtual void handleMessage(cMessage *);
};


#endif /* MULTICASTROUTINGTABLE_H_ */
