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

#ifndef ANSAARP_H_
#define ANSAARP_H_

#include "ARP.h"
#include "INETDefs.h"

// Forward declarations:
class ARPPacket;
class IInterfaceTable;
class InterfaceEntry;
class AnsaInterfaceEntry;
class IRoutingTable;

class INET_API AnsaARP : public ARP
{
    public:
        AnsaARP() {};
        virtual ~AnsaARP() {};

    protected:
        virtual bool idDuplicateMACAddresss(InterfaceEntry *ie, ARPPacket *arp);
        virtual void handleMessage(cMessage *msg);
        virtual void sendPacketToNIC(cMessage *msg, InterfaceEntry* ie, int vforwarder, const MACAddress& macAddr, int etherType);
        virtual void sendPacketToNIC(cMessage *msg, InterfaceEntry* ie, const MACAddress& dstAddr, const MACAddress& srcAddr, int etherType);
        virtual void processOutboundPacket(cMessage *msg);
        virtual void processARPPacket(ARPPacket *arp);

    public:
        void sendARPGratuitous(InterfaceEntry *ie, MACAddress srcAddr, IPv4Address ipAddress, int opCode);
};

class INET_API AnsaArpAccess : public ModuleAccess<AnsaARP>
{
  public:
    AnsaArpAccess() : ModuleAccess<AnsaARP>("arp") {}
};

#endif /* ANSAARP_H_ */
