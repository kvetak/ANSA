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

#ifndef ANSAINTERFACEENTRY_H_
#define ANSAINTERFACEENTRY_H_

#include "ansa/networklayer/common/VirtualForwarder.h"

namespace inet{
class INET_API AnsaInterfaceEntry : public InterfaceEntry
{
    public:
        typedef std::vector<VirtualForwarder *> VirtualForwarderVector;

    protected:
        VirtualForwarderVector vforwarder;

    public:
        AnsaInterfaceEntry(cModule *interfaceModule);
        virtual ~AnsaInterfaceEntry() {};

        virtual bool hasMacAddress(const MACAddress& addr) const;
        virtual bool hasIPAddress(const IPv4Address& addr) const;
        virtual const MACAddress& getMacVirtualForwarderById(int vforwarderId) const;
        virtual const MACAddress& getMacAddressByIP(const IPv4Address& addr) const;
        virtual int getVirtualForwarderId(const IPv4Address& addr);
        virtual int getVirtualForwarderId(const MACAddress& addr);

        /**
         * Adding Virtual Forwarder to this Interface
         * @param addr
         */
        virtual void addVirtualForwarder(VirtualForwarder* vforw);

        /**
         * Adding Virtual Forwarder from this Interface
         * @param addr
         */
        virtual void removeVirtualForwarder(VirtualForwarder* vforw);
};
}
#endif /* ANSAINTERFACEENTRY_H_ */
