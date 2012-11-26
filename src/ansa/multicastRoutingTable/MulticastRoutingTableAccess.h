/**
 * @file MulticastRoutingTableAccess.h
 * @brief File contains implementation of access class.
 * @brief 17.3.2012
 * @author Veronika Rybova
 */

#ifndef MULTICASTROUTINGTABLEACCESS_H_
#define MULTICASTROUTINGTABLEACCESS_H_

#include <omnetpp.h>
#include "ModuleAccess.h"
#include "MulticastRoutingTable.h"

/**
 * @brief Class gives access to the MulticastRoutingTable.
 */
class INET_API MulticastRoutingTableAccess : public ModuleAccess<MulticastRoutingTable>
{
    public:
		MulticastRoutingTableAccess() : ModuleAccess<MulticastRoutingTable>("multicastRoutingTable") {}
};

#endif /* MULTICASTROUTINGTABLEACCESS_H_ */
