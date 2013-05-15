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

#ifndef RIPNGROUTINGTABLEENTRY_H_
#define RIPNGROUTINGTABLEENTRY_H_

#include "ANSARoutingTable6.h"

#include "RIPngTimer_m.h"

class RIPngProcess;

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
    RIPngProcess *_process;        ///< pointer to the RIPng process to which interface belongs
    bool _changeFlag;              ///< The Route Changed Flag
    unsigned short int _routeTag;  ///< The Route routeTag
    RoutingTableEntry *_copy;      ///< If this RTE is stored in the "RIPng routing table", this is reference to the RTE in the "global routing table" and vice versa

  public:
    /**
     * For printing RIPng route information
     * virtual info() or virtual detailedInfo() can't be redefined,
     * because of consistency of printed route information in ANSARoutingTable6
     */
    virtual std::string RIPngInfo() const;

    bool isChangeFlagSet() const { return _changeFlag; }
    void setChangeFlag() { _changeFlag = true; }
    void clearChangeFlag() { _changeFlag = false; }

    RIPngTimer *getTimer() const { return _timeout; }
    RIPngTimer *getGCTimer() const { return _GCTimeout; }
    unsigned short int getRouteTag() const { return _routeTag; }
    RoutingTableEntry *getCopy() const { return _copy; }

    void setTimer(RIPngTimer *t) { _timeout = t; }
    void setGCTimer(RIPngTimer *t) { _GCTimeout = t; }
    void setRouteTag(unsigned short int tag) { _routeTag = tag;}
    void setCopy(RoutingTableEntry *copy) { _copy = copy; }

    void setProcess(RIPngProcess *p) { _process = p; }
    RIPngProcess *getProcess() { return _process; }
};

} /* namespace RIPng */

#endif /* RIPNGROUTINGTABLEENTRY_H_ */
