//
// Copyright (C) 2013 Opensim Ltd.
// Author: Levente Meszaros
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "ansa/networklayer/multi/ANSA_MultiNetworkLayerUpperMultiplexer.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/networklayer/contract/ipv6/IPv6ControlInfo.h"
#include "inet/networklayer/contract/generic/GenericNetworkProtocolControlInfo.h"


namespace inet {

Define_Module(ANSA_MultiNetworkLayerUpperMultiplexer);

void ANSA_MultiNetworkLayerUpperMultiplexer::initialize() {
    int transportUpperInGateSize = gateSize("transportUpperIn");
    int transportUpperOutGateSize = gateSize("transportUpperOut");
    if (transportUpperInGateSize != transportUpperOutGateSize)
        throw cRuntimeError("Connection error: transportUpperIn[] and transportUpperOut[] gate size differ");
    int transportLowerInGateSize = gateSize("transportLowerIn");
    int transportLowerOutGateSize = gateSize("transportLowerOut");
    if (transportLowerInGateSize != transportLowerOutGateSize)
        throw cRuntimeError("Connection error: transportLowerIn[] and transportLowerOut[] gate size differ");
    if (transportUpperInGateSize * getProtocolCount() != transportLowerOutGateSize)
        throw cRuntimeError("Connection error: transportUpperIn[] / transportLowerOut[] gate size ratio not correct");
    int pingUpperInGateSize = gateSize("pingUpperIn");
    int pingUpperOutGateSize = gateSize("pingUpperOut");
    if (pingUpperInGateSize != pingUpperOutGateSize)
        throw cRuntimeError("Connection error: pingUpperIn[] and pingUpperOut[] gate size differ");
    int pingLowerInGateSize = gateSize("pingLowerIn");
    int pingLowerOutGateSize = gateSize("pingLowerOut");
    if (pingLowerInGateSize != pingLowerOutGateSize)
        throw cRuntimeError("Connection error: pingLowerIn[] and pingLowerOut[] gate size differ");
    if (pingUpperInGateSize * getProtocolCount() != pingLowerOutGateSize)
        throw cRuntimeError("Connection error: pingUpperIn[] / pingLowerOut[] gate size ratio not correct");
}

void ANSA_MultiNetworkLayerUpperMultiplexer::handleMessage(cMessage* message) {
    cGate *arrivalGate = message->getArrivalGate();
    const char *arrivalGateName = arrivalGate->getBaseName();
    if (!strcmp(arrivalGateName, "transportUpperIn")) {
        if (dynamic_cast<cPacket *>(message)) {
            int res = getProtocolIndex(message);
            if (res > -1) {
                send(message, "transportLowerOut", getProtocolCount() * arrivalGate->getIndex() + res); }
            else {
                EV << "Message dropped because appropriate L3 layer is not present!" << endl;
                delete message;
            }
        }
        else {
            // sending down commands
            for (int i = 0; i < getProtocolCount(); i++) {
                //Skip for L3 layers that are not present
                if (i == 0 && !(getParentModule()->par("enableIPv4"))) continue;
                if (i == 1 && !(getParentModule()->par("enableIPv6"))) continue;
                if (i == 2 && !(getParentModule()->par("enableCLNS"))) continue;
                cMessage *duplicate = message->dup();
                cObject *controlInfo = message->getControlInfo();
                if (controlInfo)
                    duplicate->setControlInfo(controlInfo->dup());
                send(duplicate, "transportLowerOut", getProtocolCount() * arrivalGate->getIndex() + i);
            }
            delete message;
        }
    }
    else if (!strcmp(arrivalGateName, "transportLowerIn"))
        send(message, "transportUpperOut", arrivalGate->getIndex() / getProtocolCount());
    else if (!strcmp(arrivalGateName, "pingUpperIn")) {
        int res = getProtocolIndex(message);
        if (res > -1) {
            send(message, "pingLowerOut", getProtocolCount() * arrivalGate->getIndex() + getProtocolIndex(message)); }
        else {
            EV << "Message dropped because appropriate L3 layer is not present!" << endl;
            delete message;
        }
    }
    else if (!strcmp(arrivalGateName, "pingLowerIn"))
        send(message, "pingUpperOut", arrivalGate->getIndex() / getProtocolCount());
    else
        throw cRuntimeError("Unknown arrival gate");
}

int ANSA_MultiNetworkLayerUpperMultiplexer::getProtocolCount() {
    return 3;
}

int ANSA_MultiNetworkLayerUpperMultiplexer::getProtocolIndex(
        cMessage* message) {
    cPacket *packet = check_and_cast<cPacket *>(message);
    cObject *controlInfo = packet->getControlInfo();
    // Vesely - resolved: handle the case when some network protocols are disabled

    //Return -1 if appropriate L3 is not present
    if (dynamic_cast<IPv4ControlInfo *>(controlInfo))
        return (getParentModule()->par("enableIPv4")) ? 0 : -1;
    else if (dynamic_cast<IPv6ControlInfo *>(controlInfo))
        return (getParentModule()->par("enableIPv6")) ? 1 : -1;
    else if (dynamic_cast<GenericNetworkProtocolControlInfo *>(controlInfo))
        return (getParentModule()->par("enableCLNS")) ? 2 : -1;
    else
        throw cRuntimeError("Unknown control info");
}

} // namespace inet

