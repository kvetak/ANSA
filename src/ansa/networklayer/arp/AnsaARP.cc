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

#include "AnsaARP.h"
#include "Ieee802Ctrl_m.h"
#include "AnsaIPv4RoutingDecision_m.h"
#include "ARPPacket_m.h"
#include "IPv4Datagram.h"
#include "IPv4InterfaceData.h"
#include "AnsaInterfaceEntry.h"
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"

Define_Module(AnsaARP);

void AnsaARP::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        requestTimedOut(msg);
    }
    else if (dynamic_cast<ARPPacket *>(msg))
    {
        ARPPacket *arp = (ARPPacket *)msg;
        processARPPacket(arp);
    }
    else // not ARP
    {
        processOutboundPacket(msg);
    }
    if (ev.isGUI())
        updateDisplayString();
}

void AnsaARP::processOutboundPacket(cMessage *msg)
{
    EV << "Packet " << msg << " arrived from higher layer, ";

    // get next hop address from control info in packet
    IPv4Address nextHopAddr;
    int vforwarder = -1;
    int interfaceId;

    if (dynamic_cast<AnsaIPv4RoutingDecision *>(msg->getControlInfo()))
    {
        AnsaIPv4RoutingDecision *controlInfo = check_and_cast<AnsaIPv4RoutingDecision*>(msg->removeControlInfo());
        nextHopAddr = controlInfo->getNextHopAddr();
        interfaceId = controlInfo->getInterfaceId();
        vforwarder = controlInfo->getVforwarderId();
        delete controlInfo;
    } else {
        IPv4RoutingDecision *controlInfo = check_and_cast<IPv4RoutingDecision*>(msg->removeControlInfo());
        nextHopAddr = controlInfo->getNextHopAddr();
        interfaceId = controlInfo->getInterfaceId();
        delete controlInfo;
    }

    InterfaceEntry *ie = ift->getInterfaceById(interfaceId);

    // if output interface is not broadcast, don't bother with ARP
    if (!ie->isBroadcast())
    {
        EV << "output interface " << ie->getName() << " is not broadcast, skipping ARP\n";
        send(msg, nicOutBaseGateId + ie->getNetworkLayerGateIndex());
        return;
    }

    // determine what address to look up in ARP cache
    if (!nextHopAddr.isUnspecified())
    {
        EV << "using next-hop address " << nextHopAddr << "\n";
    }
    else
    {
        // try proxy ARP
        IPv4Datagram *datagram = check_and_cast<IPv4Datagram *>(msg);
        nextHopAddr = datagram->getDestAddress();
        EV << "no next-hop address, using destination address " << nextHopAddr << " (proxy ARP)\n";
    }

    if (nextHopAddr.isLimitedBroadcastAddress() ||
            nextHopAddr == ie->ipv4Data()->getIPAddress().makeBroadcastAddress(ie->ipv4Data()->getNetmask())) // also include the network broadcast
    {
        EV << "destination address is broadcast, sending packet to broadcast MAC address\n";
        sendPacketToNIC(msg, ie, vforwarder, MACAddress::BROADCAST_ADDRESS, ETHERTYPE_IPv4);
        return;
    }

    if (nextHopAddr.isMulticast())
    {
        MACAddress macAddr = mapMulticastAddress(nextHopAddr);
        EV << "destination address is multicast, sending packet to MAC address " << macAddr << "\n";
        sendPacketToNIC(msg, ie, vforwarder, macAddr, ETHERTYPE_IPv4);
        return;
    }

    if (globalARP)
    {
        ARPCache::iterator it = globalArpCache.find(nextHopAddr);
        if (it==globalArpCache.end())
            throw cRuntimeError("Address not found in global ARP cache: %s", nextHopAddr.str().c_str());
        sendPacketToNIC(msg, ie, vforwarder, (*it).second->macAddress, ETHERTYPE_IPv4);
        return;
    }

    // try look up
    ARPCache::iterator it = arpCache.find(nextHopAddr);
    //ASSERT(it==arpCache.end() || ie==(*it).second->ie); // verify: if arpCache gets keyed on InterfaceEntry* too, this becomes unnecessary
    if (it==arpCache.end())
    {
        // no cache entry: launch ARP request
        ARPCacheEntry *entry = new ARPCacheEntry();
        ARPCache::iterator where = arpCache.insert(arpCache.begin(), std::make_pair(nextHopAddr, entry));
        entry->myIter = where; // note: "inserting a new element into a map does not invalidate iterators that point to existing elements"
        entry->ie = ie;

        EV << "Starting ARP resolution for " << nextHopAddr << "\n";
        initiateARPResolution(entry);

        // and queue up packet
        entry->pendingPackets.push_back(msg);
        pendingQueue.insert(msg);
    }
    else if ((*it).second->pending)
    {
        // an ARP request is already pending for this address -- just queue up packet
        EV << "ARP resolution for " << nextHopAddr << " is pending, queueing up packet\n";
        (*it).second->pendingPackets.push_back(msg);
        pendingQueue.insert(msg);
    }
    else if ((*it).second->lastUpdate+cacheTimeout<simTime())
    {
        EV << "ARP cache entry for " << nextHopAddr << " expired, starting new ARP resolution\n";

        // cache entry stale, send new ARP request
        ARPCacheEntry *entry = (*it).second;
        entry->ie = ie; // routing table may have changed
        initiateARPResolution(entry);

        // and queue up packet
        entry->pendingPackets.push_back(msg);
        pendingQueue.insert(msg);
    }
    else
    {
        // valid ARP cache entry found, flag msg with MAC address and send it out
        EV << "ARP cache hit, MAC address for " << nextHopAddr << " is " << (*it).second->macAddress << ", sending packet down\n";
        sendPacketToNIC(msg, ie, vforwarder, (*it).second->macAddress, ETHERTYPE_IPv4);
    }
}

void AnsaARP::processARPPacket(ARPPacket *arp)
{
    EV << "ARP packet " << arp << " arrived:\n";
    dumpARPPacket(arp);

    // extract input port
    IPv4RoutingDecision *controlInfo = check_and_cast<IPv4RoutingDecision*>(arp->removeControlInfo());
    InterfaceEntry *ie = ift->getInterfaceById(controlInfo->getInterfaceId());
    AnsaInterfaceEntry *ieVF = dynamic_cast<AnsaInterfaceEntry *>(ie);
    delete controlInfo;

    //
    // Recipe a'la RFC 826:
    //
    // ?Do I have the hardware type in ar$hrd?
    // Yes: (almost definitely)
    //   [optionally check the hardware length ar$hln]
    //   ?Do I speak the protocol in ar$pro?
    //   Yes:
    //     [optionally check the protocol length ar$pln]
    //     Merge_flag := false
    //     If the pair <protocol type, sender protocol address> is
    //         already in my translation table, update the sender
    //         hardware address field of the entry with the new
    //         information in the packet and set Merge_flag to true.
    //     ?Am I the target protocol address?
    //     Yes:
    //       If Merge_flag is false, add the triplet <protocol type,
    //           sender protocol address, sender hardware address> to
    //           the translation table.
    //       ?Is the opcode ares_op$REQUEST?  (NOW look at the opcode!!)
    //       Yes:
    //         Swap hardware and protocol fields, putting the local
    //             hardware and protocol addresses in the sender fields.
    //         Set the ar$op field to ares_op$REPLY
    //         Send the packet to the (new) target hardware address on
    //             the same hardware on which the request was received.
    //

    MACAddress srcMACAddress = arp->getSrcMACAddress();
    IPv4Address srcIPAddress = arp->getSrcIPAddress();

    if (srcMACAddress.isUnspecified())
        error("wrong ARP packet: source MAC address is empty");
    if (srcIPAddress.isUnspecified())
        error("wrong ARP packet: source IPv4 address is empty");

    bool mergeFlag = false;
    bool duplicate = idDuplicateMACAddresss(ie, arp);
    // "If ... sender protocol address is already in my translation table"
    ARPCache::iterator it = arpCache.find(srcIPAddress);
    if (!duplicate && it!=arpCache.end())
    {
        // "update the sender hardware address field"
        ARPCacheEntry *entry = (*it).second;
        updateARPCache(entry, srcMACAddress);
        mergeFlag = true;
    }

    // "?Am I the target protocol address?"
    // if Proxy ARP is enabled, we also have to reply if we're a router to the dest IPv4 address
    if (addressRecognized(arp->getDestIPAddress(), ie))
    {
        // "If Merge_flag is false, add the triplet protocol type, sender
        // protocol address, sender hardware address to the translation table"
        if (!duplicate && !mergeFlag)
        {
            ARPCacheEntry *entry;
            if (it!=arpCache.end())
            {
                entry = (*it).second;
            }
            else
            {
                entry = new ARPCacheEntry();
                ARPCache::iterator where = arpCache.insert(arpCache.begin(), std::make_pair(srcIPAddress, entry));
                entry->myIter = where;
                entry->ie = ie;

                entry->pending = false;
                entry->timer = NULL;
                entry->numRetries = 0;
            }
            updateARPCache(entry, srcMACAddress);
        }

        // "?Is the opcode ares_op$REQUEST?  (NOW look at the opcode!!)"
        switch (arp->getOpcode())
        {
            case ARP_REQUEST:
            {
                EV << "Packet was ARP REQUEST, sending REPLY\n";

                // find our own IPv4 address and MAC address on the given interface
                MACAddress myMACAddress = ie->getMacAddress();
                IPv4Address myIPAddress = ie->ipv4Data()->getIPAddress();

                if (ieVF)
                    myMACAddress = ieVF->getMacAddressByIP(arp->getDestIPAddress());

                // "Swap hardware and protocol fields", etc.
                arp->setName("arpREPLY");
                IPv4Address origDestAddress = arp->getDestIPAddress();
                arp->setDestIPAddress(srcIPAddress);
                arp->setDestMACAddress(srcMACAddress);
                arp->setSrcIPAddress(origDestAddress);
                arp->setSrcMACAddress(myMACAddress);
                arp->setOpcode(ARP_REPLY);
                delete arp->removeControlInfo();
                sendPacketToNIC(arp, ie, srcMACAddress, myMACAddress, ETHERTYPE_ARP);
                numRepliesSent++;
                emit(sentReplySignal, 1L);
                break;
            }
            case ARP_REPLY:
            {
                EV << "Discarding packet\n";
                delete arp;
                break;
            }
            case ARP_RARP_REQUEST: throw cRuntimeError("RARP request received: RARP is not supported");
            case ARP_RARP_REPLY: throw cRuntimeError("RARP reply received: RARP is not supported");
            default: throw cRuntimeError("Unsupported opcode %d in received ARP packet", arp->getOpcode());
        }
    }
    else
    {
        // address not recognized
        EV << "IPv4 address " << arp->getDestIPAddress() << " not recognized, dropping ARP packet\n";
        delete arp;
    }
}

void AnsaARP::sendPacketToNIC(cMessage *msg, InterfaceEntry *ie, int vforwarder, const MACAddress& macAddress, int etherType)
{
    if (vforwarder != -1 && dynamic_cast<AnsaInterfaceEntry *>(ie))
    {
        AnsaInterfaceEntry* ieVF = dynamic_cast<AnsaInterfaceEntry *>(ie);
        sendPacketToNIC(msg, ie, macAddress, ieVF->getMacVirtualForwarderById(vforwarder), etherType);
        return;
    }

    ARP::sendPacketToNIC(msg, ie, macAddress, etherType);
}

void AnsaARP::sendPacketToNIC(cMessage *msg, InterfaceEntry *ie, const MACAddress& dstAddress, const MACAddress& srcAddress, int etherType)
{
    // add control info with MAC address
    Ieee802Ctrl *controlInfo = new Ieee802Ctrl();

    controlInfo->setDest(dstAddress);
    controlInfo->setEtherType(etherType);
    controlInfo->setSrc(srcAddress);
    msg->setControlInfo(controlInfo);

    // send out
    send(msg, nicOutBaseGateId + ie->getNetworkLayerGateIndex());
}

void AnsaARP::sendARPGratuitous(InterfaceEntry *ie, MACAddress srcAddr, IPv4Address ipAddr, int opCode)
{
    Enter_Method_Silent();

    // both must be set
    ASSERT(!srcAddr.isUnspecified());
    ASSERT(!ipAddr.isUnspecified());

    // fill out everything in ARP Request packet except dest MAC address
    ARPPacket *arp = new ARPPacket("arpGrt");
    arp->setByteLength(ARP_HEADER_BYTES);
    arp->setOpcode(opCode);
    arp->setSrcMACAddress(srcAddr);
    arp->setSrcIPAddress(ipAddr);
    arp->setDestIPAddress(ipAddr);
    arp->setDestMACAddress(MACAddress::BROADCAST_ADDRESS);

    // add control info with MAC address
    Ieee802Ctrl *controlInfo = new Ieee802Ctrl();
    controlInfo->setDest(MACAddress::BROADCAST_ADDRESS);
    controlInfo->setEtherType(ETHERTYPE_ARP);
    controlInfo->setSrc(srcAddr);
    arp->setControlInfo(controlInfo);

    // send out
    send(arp, nicOutBaseGateId + ie->getNetworkLayerGateIndex());
}

bool AnsaARP::idDuplicateMACAddresss(InterfaceEntry *ie, ARPPacket *arp)
{
    if (arp->getSrcMACAddress() == ie->getMacAddress())
        return true;

    AnsaInterfaceEntry* ieVF = dynamic_cast<AnsaInterfaceEntry *>(ie);
    if (ieVF)
        if (ieVF->hasMacAddress(arp->getSrcMACAddress()))
            return true;

    return false;
}
