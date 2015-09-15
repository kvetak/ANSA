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
* @file RIPRoutingTableEntry.h
* @author Jiri Trhlik (mailto:jiritm@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
* @brief RIP RTE
* @detail Represents RIP Routing Table Entry
*/

#ifndef RIPROUTINGTABLEENTRY_H_
#define RIPROUTINGTABLEENTRY_H_

#include "ansa/networklayer/ipv4/AnsaIPv4Route.h"

#include "ansa/applications/rip/RIPTimer_m.h"

namespace RIP
{

/**
 *  Just extends IPv4 route for RIP.
 */
class RoutingTableEntry : public ANSAIPv4Route
{
  public:
    RoutingTableEntry(inet::IPv4Address network, inet::IPv4Address netMask);
    RoutingTableEntry(RoutingTableEntry& entry);
    virtual ~RoutingTableEntry();

  protected:
    RIPTimer *_timeout;            ///< Pointer for the Route timeout timer
    RIPTimer *_GCTimeout;          ///< Pointer for the Route Garbage-Collection Timer
    bool _changeFlag;              ///< The Route Changed Flag
    unsigned short int _routeTag;  ///< The Route routeTag
    RoutingTableEntry *_copy;      ///< If this RTE is stored in the "RIPng routing table", this is reference to the RTE in the "global routing table" and vice versa

  public:
    /**
     * For printing RIP route information
     * virtual info() or virtual detailedInfo() can't be redefined,
     * because of consistency of printed route information in ANSARoutingTable
     */
    virtual std::string RIPInfo() const;

    bool isChangeFlagSet() const { return _changeFlag; }
    void setChangeFlag() { _changeFlag = true; }
    void clearChangeFlag() { _changeFlag = false; }

    RIPTimer *getTimer() const { return _timeout; }
    RIPTimer *getGCTimer() const { return _GCTimeout; }
    unsigned short int getRouteTag() const { return _routeTag; }
    RoutingTableEntry *getCopy() const { return _copy; }

    void setTimer(RIPTimer *t) { _timeout = t; }
    void setGCTimer(RIPTimer *t) { _GCTimeout = t; }
    void setRouteTag(unsigned short int tag) { _routeTag = tag;}
    void setCopy(RoutingTableEntry *copy) { _copy = copy; }
};

} /* namespace RIP */

#endif /* RIPROUTINGTABLEENTRY_H_ */
