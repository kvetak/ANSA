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
* @brief
* @detail
*/

#ifndef VIRTUALFORWARDER_H_
#define VIRTUALFORWARDER_H_

#include <vector>
#include <algorithm>

#include "MACAddress.h"
#include "IPv4Address.h"

class VirtualForwarder : public cObject
{
    public:
        typedef std::vector<IPv4Address> IPv4AddressVector;

    protected:
        IPv4AddressVector ipAddr;
        MACAddress macAddr;
        bool disable;

    public:
        VirtualForwarder();
        virtual ~VirtualForwarder() {};

        /** @name Field getters. Note they are non-virtual and inline, for performance reasons. */
        //@{
        bool hasIPAddress(const IPv4Address& addr) const;
        const MACAddress& getMacAddress() const  {return macAddr;}
        bool isDisable() { return disable; };
        //@}

        /** @name Field setters */
        //@{
        virtual void setMACAddress(const MACAddress& addr) { macAddr = addr; };
        virtual void setDisable() { disable = true; };
        virtual void setEnable() { disable = false; };
        //@}

        bool operator==(const VirtualForwarder& other) const { return getMacAddress() == other.getMacAddress(); }

        /**
         * Adding IP address to Virtual Forwarder
         * @param addr
         */
        virtual void addIPAddress(const IPv4Address &addr);

        /**
         * Deleting IP addresses from Virtual Forwarder
         * @param addr
         */
        virtual void removeIPAddress(const IPv4Address &addr);

        virtual std::string info() const;
};

#endif /* VIRTUALFORWARDER_H_ */
