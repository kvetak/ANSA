//
// Marek Cerny, 2MSK
// FIT VUT 2011
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

#ifndef __ANSA_ROUTING_TABLE6_ACCESS_H
#define __ANSA_ROUTING_TABLE6_ACCESS_H

#include <omnetpp.h>
#include "ModuleAccess.h"
#include "ansaRoutingTable6.h"

/**
 * Gives access to AnsaRoutingTable6
 */
class INET_API AnsaRoutingTable6Access : public ModuleAccess<AnsaRoutingTable6>
{
    public:
        AnsaRoutingTable6Access() : ModuleAccess<AnsaRoutingTable6>("routingTable6") {}
};

#endif


