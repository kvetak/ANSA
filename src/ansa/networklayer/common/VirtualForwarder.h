//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
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
/**
 * @file VirtualForwarder.h
 * @author Petr Vitek
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */

#ifndef VIRTUALFORWARDER_H_
#define VIRTUALFORWARDER_H_

#include <vector>
#include <algorithm>

#include "inet/networklayer/contract/IL3AddressType.h"
namespace inet{

class VirtualForwarder : public cObject
{
    public:
        typedef std::vector<Ipv4Address> Ipv4AddressVector;

    protected:
        Ipv4AddressVector ipAddr;
        MacAddress macAddr;
        bool disable;

    public:
        VirtualForwarder();
        virtual ~VirtualForwarder() {};

        /** @name Field getters. Note they are non-virtual and inline, for performance reasons. */
        //@{
        bool hasIPAddress(const Ipv4Address& addr) const;
        const MacAddress& getMacAddress() const  {return macAddr;}
        bool isDisable() { return disable; };
        //@}

        /** @name Field setters */
        //@{
        virtual void setMACAddress(const MacAddress& addr) { macAddr = addr; };
        virtual void setDisable() { disable = true; };
        virtual void setEnable() { disable = false; };
        //@}

        bool operator==(const VirtualForwarder& other) const { return getMacAddress() == other.getMacAddress(); }

        /**
         * Adding IP address to Virtual Forwarder
         * @param addr
         */
        virtual void addIPAddress(const Ipv4Address &addr);

        /**
         * Deleting IP addresses from Virtual Forwarder
         * @param addr
         */
        virtual void removeIPAddress(const Ipv4Address &addr);

        virtual std::string info() const;
};
}//namespace inet
#endif /* VIRTUALFORWARDER_H_ */
