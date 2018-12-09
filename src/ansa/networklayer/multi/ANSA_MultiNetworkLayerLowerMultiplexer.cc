//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
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
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */

#include "ansa/networklayer/multi/ANSA_MultiNetworkLayerLowerMultiplexer.h"

#define WITH_CDP
#define WITH_LLDP

#ifdef WITH_CDP
#include "ansa/linklayer/cdp/CDPUpdate.h"
#endif // ifdef WITH_CDP

#ifdef WITH_LLDP
#include "ansa/linklayer/lldp/LLDPUpdate.h"
#endif // ifdef WITH_LLDP

#ifdef WITH_IPv4
#include "inet/networklayer/arp/ipv4/ARPPacket_m.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/ipv6/IPv6Datagram.h"
#endif // ifdef WITH_IPv6


#include "ansa/networklayer/isis/ISISMessage_m.h"


namespace inet {

Define_Module(ANSA_MultiNetworkLayerLowerMultiplexer);

void ANSA_MultiNetworkLayerLowerMultiplexer::handleMessage(cMessage *message)
{
    cGate *arrivalGate = message->getArrivalGate();
    const char *arrivalGateName = arrivalGate->getBaseName();
    if (!strcmp(arrivalGateName, "ifUpperIn"))
    {
        send(message, "ifLowerOut", arrivalGate->getIndex() / getProtocolCount());
    }
    else if (!strcmp(arrivalGateName, "ifLowerIn"))
    {
        int res = getProtocolIndex(message);
        if (res > -1) {
            send(message, "ifUpperOut", getProtocolCount() * arrivalGate->getIndex() + res); }
        else {
            EV << "Message dropped because appropriate L3 layer is not present!" << endl;
            delete message;
        }
    }
    else
        throw cRuntimeError("Unknown arrival gate");
}

int ANSA_MultiNetworkLayerLowerMultiplexer::getProtocolCount()
{
    return 5;
}

void ANSA_MultiNetworkLayerLowerMultiplexer::initialize() {

}

int ANSA_MultiNetworkLayerLowerMultiplexer::getProtocolIndex(cMessage *message)
{
    //Vesely - Resolved: handle the case when some network protocols are disabled
    if (false)
        ;
#ifdef WITH_IPv4
    else if (dynamic_cast<IPv4Datagram *>(message) || dynamic_cast<ArpPacket *>(message))
        return (getParentModule()->par("enableIPv4")) ? 0 : -1;
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
    else if (dynamic_cast<IPv6Datagram *>(message))
        return (getParentModule()->par("enableIPv6")) ? 1 : -1;
#endif // ifdef WITH_IPv6

    else if (dynamic_cast<ISISMessage *>(message))
        return (getParentModule()->par("enableCLNS")) ? 2 : -1;

    else if (dynamic_cast<CDPUpdate *>(message))
        return (getParentModule()->par("enableCDP")) ? 3 : -1;
    else if (dynamic_cast<LLDPUpdate *>(message))
        return (getParentModule()->par("enableLLDP")) ? 4 : -1;
    else
        throw cRuntimeError("Unknown message");
}

} // namespace inet
