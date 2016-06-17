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

#include "ansa/networklayer/common/ANSA_InterfaceEntry.h"
#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet{

ANSA_InterfaceEntry::ANSA_InterfaceEntry(cModule *interfaceModule) : InterfaceEntry(interfaceModule) {}

void ANSA_InterfaceEntry::addVirtualForwarder(VirtualForwarder* vforw)
{
    vforwarder.push_back(vforw);
}

void ANSA_InterfaceEntry::removeVirtualForwarder(VirtualForwarder* vforw)
{
    for (int i = 0; i < (int) vforwarder.size(); i++)
        if (vforwarder[i] == vforw)
            vforwarder[i] = NULL;

    delete vforw;
}

int ANSA_InterfaceEntry::getVirtualForwarderId(const IPv4Address& addr)
{
    if (vforwarder.empty())
        return -1;

    for (int i = 0; i < (int) vforwarder.size(); i++)
        if (!vforwarder[i]->isDisable() && (vforwarder[i]->hasIPAddress(addr)))
            return i;

    return -1;
}

int ANSA_InterfaceEntry::getVirtualForwarderId(const MACAddress& addr)
{
    if (vforwarder.empty())
        return -1;

    for (int i = 0; i < (int) vforwarder.size(); i++)
        if (!vforwarder[i]->isDisable() && vforwarder[i]->getMacAddress() == addr)
            return i;

    return -1;
}

const MACAddress& ANSA_InterfaceEntry::getMacAddressByIP(const IPv4Address& addr) const
{
    if (vforwarder.empty())
        return getMacAddress();

    for (int i = 0; i < (int) vforwarder.size(); i++)
        if (!vforwarder[i]->isDisable() && vforwarder[i]->hasIPAddress(addr))
            return vforwarder[i]->getMacAddress();

    return getMacAddress();
}

const MACAddress& ANSA_InterfaceEntry::getMacVirtualForwarderById(int vforwarderId) const
{
    if (vforwarder.empty())
        return MACAddress::UNSPECIFIED_ADDRESS;

    return vforwarder.at(vforwarderId)->getMacAddress();
}

bool ANSA_InterfaceEntry::hasIPAddress(const IPv4Address& addr) const
{
    if (vforwarder.empty())
        return false;

    for (int i = 0; i < (int) vforwarder.size(); i++)
        if (!vforwarder[i]->isDisable() && vforwarder[i]->hasIPAddress(addr))
            return true;

    return false;
}

bool ANSA_InterfaceEntry::hasMacAddress(const MACAddress& addr) const
{
    if (vforwarder.empty())
        return false;

    for (int i = 0; i < (int) vforwarder.size(); i++)
        if (!vforwarder[i]->isDisable() && vforwarder[i]->getMacAddress() == addr)
            return true;

    return false;
}
}
