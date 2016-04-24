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

#include "ansa/networklayer/multi/ANSA_MultiNetworkLayerLowerMultiplexer.h"

#define WITH_CDP
#define WITH_LLDP

#ifdef WITH_CDP
#include "ansa/linklayer/cdp/CDPUpdate.h"
#endif // ifdef WITH_CDP

#ifdef WITH_LLDP
#include "ansa/linklayer/lldp/LLDPUpdate_m.h"
#endif // ifdef WITH_LLDP

#ifdef WITH_IPv4
#include "inet/networklayer/arp/ipv4/ARPPacket_m.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/ipv6/IPv6Datagram.h"
#endif // ifdef WITH_IPv6

#ifdef WITH_GENERIC
#include "inet/networklayer/generic/GenericDatagram.h"
#endif // ifdef WITH_GENERIC

namespace inet {

Define_Module(ANSA_MultiNetworkLayerLowerMultiplexer);

void ANSA_MultiNetworkLayerLowerMultiplexer::handleMessage(cMessage *message)
{
    cGate *arrivalGate = message->getArrivalGate();
    const char *arrivalGateName = arrivalGate->getBaseName();
    if (!strcmp(arrivalGateName, "ifUpperIn"))
        //send(message, "ifLowerOut", arrivalGate->getIndex() / getProtocolCount());
    {
        //EV << arrivalGate->getIndex() << ", proto:" << getProtocolCount() << ", vys:" << arrivalGate->getIndex() / getProtocolCount();
        send(message, "ifLowerOut", arrivalGate->getIndex() / getProtocolCount());
    }
    else if (!strcmp(arrivalGateName, "ifLowerIn"))
    {
        send(message, "ifUpperOut", getProtocolCount() * arrivalGate->getIndex() + getProtocolIndex(message));
    }
    else
        throw cRuntimeError("Unknown arrival gate");
}

int ANSA_MultiNetworkLayerLowerMultiplexer::getProtocolCount()
{
    return 5;
}

int ANSA_MultiNetworkLayerLowerMultiplexer::getProtocolIndex(cMessage *message)
{
    // TODO: handle the case when some network protocols are disabled
    if (false)
        ;
#ifdef WITH_IPv4
    else if (dynamic_cast<IPv4Datagram *>(message) || dynamic_cast<ARPPacket *>(message))
        return 0;
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
    else if (dynamic_cast<IPv6Datagram *>(message))
        return 1;
#endif // ifdef WITH_IPv6
#ifdef WITH_GENERIC
    else if (dynamic_cast<GenericDatagram *>(message))
        return 2;
#endif // ifdef WITH_GENERIC
#ifdef WITH_CDP
    else if (dynamic_cast<CDPUpdate *>(message))
        return 3;
#endif // ifdef WITH_CDP
#ifdef WITH_LLDP
    else if (dynamic_cast<LLDPUpdate *>(message))
        return 4;
#endif // ifdef WITH_LLDP
    else
        throw cRuntimeError("Unknown message");
}

} // namespace inet
