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
* @file RIPngInterface.cc
* @author Jiri Trhlik (mailto:), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
* @brief
* @detail
*/

#ifndef ROUTINGTABLEENTRY_H_
#define ROUTINGTABLEENTRY_H_

#include "ANSARoutingTable6.h"

#include "RIPngTimer_m.h"

namespace RIPng
{

/**
 *  Just extends IPv6 route for RIPng.
 */
class RoutingTableEntry : public ANSAIPv6Route
{
  public:
    RoutingTableEntry(IPv6Address destPrefix, int length);
    RoutingTableEntry(RoutingTableEntry& entry);
    virtual ~RoutingTableEntry();

  protected:
    RIPngTimer *_timeout;          ///< Pointer for the Route timeout timer
    RIPngTimer *_GCTimeout;        ///< Pointer for the Route Garbage-Collection Timer
    bool _changeFlag;              ///< The Route Changed Flag
    unsigned short int _routeTag;  ///< The Route routeTag
    RoutingTableEntry *_copy;      ///< If this RTE is stored in the "RIPng routing table", this is reference to the RTE in the "global routing table" and vice versa

  public:
    bool isChangeFlagSet() { return _changeFlag; }
    void setChangeFlag() { _changeFlag = true; }
    void clearChangeFlag() { _changeFlag = false; }

    RIPngTimer *getTimer() { return _timeout; }
    RIPngTimer *getGCTimer() { return _GCTimeout; }
    unsigned short int getRouteTag() { return _routeTag; }
    RoutingTableEntry *getCopy() { return _copy; }

    void setTimer(RIPngTimer *t) { _timeout = t; }
    void setGCTimer(RIPngTimer *t) { _GCTimeout = t; }
    void setRouteTag(unsigned short int tag) { _routeTag = tag;}
    void setCopy(RoutingTableEntry *copy) { _copy = copy; }
};

} /* namespace RIPng */

#endif /* ROUTINGTABLEENTRY_H_ */
