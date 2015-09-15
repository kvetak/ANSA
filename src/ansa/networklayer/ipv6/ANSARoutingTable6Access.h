//
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//
//

/**
 * @file ANSARoutingTableAccess6.h
 * @date 21.5.2013
 * @author Jiri Trhlik (mailto:jiritm@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @brief File contains implementation of access class.
 */

#ifndef __INET_ANSA_ROUTING_TABLE6_ACCESS_H
#define __INET_ANSA_ROUTING_TABLE6_ACCESS_H

#include "common/INETDefs.h"

#include "common/ModuleAccess.h"
#include "ansa/networklayer/ipv6/ANSARoutingTable6.h"


/**
 * Gives access to ANSARoutingTable6
 */
class ANSARoutingTable6Access : public inet::ModuleAccess<ANSARoutingTable6>
{
    public:
        ANSARoutingTable6Access() : inet::ModuleAccess<ANSARoutingTable6>("routingTable6") {}
};

#endif


