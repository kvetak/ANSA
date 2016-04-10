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
* @file VirtualForwarder.cc
* @author Petr Vitek
* @brief
* @detail
*/
#include "VirtualForwarder.h"
namespace inet{
VirtualForwarder::VirtualForwarder()
{
    disable = false;
}

void VirtualForwarder::addIPAddress(const IPv4Address &addr)
{
    if (!hasIPAddress(addr))
        ipAddr.push_back(addr);
}

void VirtualForwarder::removeIPAddress(const IPv4Address &addr)
{
    int n = ipAddr.size();
    int i;
    for (i = 0; i < n; i++)
        if (ipAddr[i] == addr)
            break;
    if (i != n)
        ipAddr.erase(ipAddr.begin() + i);
}

bool VirtualForwarder::hasIPAddress(const IPv4Address& addr) const
{
    return find(ipAddr.begin(), ipAddr.end(), addr) != ipAddr.end();
}

std::string VirtualForwarder::info() const
{
    std::stringstream out;
    if (ipAddr.size() > 0)
    {
        out << "mac:" << macAddr;
        out << " status:" << ((disable) ? "disable" : "enable");
        out << " ip:";
        for (int i = 0; i < (int) ipAddr.size(); ++i)
            out << (i>0?",":"") << ipAddr[i];
    }
    return out.str();
}
}//namespace inet
