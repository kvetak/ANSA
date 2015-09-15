//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004-2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#include <omnetpp.h>
#include "ansa/util/IPGen/AnsaIPTrafGen.h"
#include "networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "networklayer/contract/ipv6/IPv6ControlInfo.h"
#include "networklayer/common/L3AddressResolver.h"


Define_Module(AnsaIPTrafSink);


void AnsaIPTrafSink::initialize()
{
    numReceived = 0;
    WATCH(numReceived);
}

void AnsaIPTrafSink::handleMessage(cMessage *msg)
{
    processPacket(check_and_cast<cPacket *>(msg));

    if (ev.isGUI())
    {
        char buf[32];
        sprintf(buf, "rcvd: %d pks", numReceived);
        getDisplayString().setTagArg("t",0,buf);
    }

}

void AnsaIPTrafSink::printPacket(cPacket *msg)
{
    inet::L3Address src, dest;
    int protocol = -1;
    if (dynamic_cast<inet::IPv4ControlInfo *>(msg->getControlInfo())!=NULL)
    {
        inet::IPv4ControlInfo *ctrl = (inet::IPv4ControlInfo *)msg->getControlInfo();
        src = ctrl->getSrcAddr();
        dest = ctrl->getDestAddr();
        protocol = ctrl->getProtocol();
    }
    else if (dynamic_cast<inet::IPv6ControlInfo *>(msg->getControlInfo())!=NULL)
    {
        inet::IPv6ControlInfo *ctrl = (inet::IPv6ControlInfo *)msg->getControlInfo();
        src = ctrl->getSrcAddr();
        dest = ctrl->getDestAddr();
        protocol = ctrl->getProtocol();
    }

    ev  << msg << endl;
    ev  << "Payload length: " << msg->getByteLength() << " bytes" << endl;
    if (protocol!=-1)
        ev  << "src: " << src << "  dest: " << dest << "  protocol=" << protocol << "\n";
}

void AnsaIPTrafSink::processPacket(cPacket *msg)
{
    EV << "Received packet: ";
    printPacket(msg);
    std::cout << "Arrive: " << msg->getFullName() << " at time = " << msg->getArrivalTime() << endl;
    delete msg;

    numReceived++;
}



//===============================================


Define_Module(AnsaIPTrafGen);

int AnsaIPTrafGen::counter;

void AnsaIPTrafGen::initialize(int stage)
{
    // because of IPAddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if (stage!=3)
        return;

    AnsaIPTrafSink::initialize();

    protocol = par("protocol");
    msgByteLength = par("packetLength");
    numPackets = par("numPackets");
    simtime_t startTime = par("startTime");

    const char *destAddrs = par("destAddresses");
    cStringTokenizer tokenizer(destAddrs);
    const char *token;
    while ((token = tokenizer.nextToken())!=NULL)
        destAddresses.push_back(inet::L3AddressResolver().resolve(token));

    counter = 0;

    numSent = 0;
    WATCH(numSent);

    if (destAddresses.empty())
        return;

    cMessage *timer = new cMessage("sendTimer");
    scheduleAt(startTime, timer);
}

inet::L3Address AnsaIPTrafGen::chooseDestAddr()
{
    int k = intrand(destAddresses.size());
    return destAddresses[k];
}

void AnsaIPTrafGen::sendPacket()
{
    char msgName[32];
    //sprintf(msgName,"appData-%d", counter++);
    const char *name = par("packetName").stringValue();
    sprintf(msgName,name,counter++);

    cPacket *payload = new cPacket(msgName);
    payload->setByteLength(msgByteLength);

    inet::L3Address destAddr = chooseDestAddr();
    if (!(destAddr.getType() == inet::L3Address::IPv6))
    {
        // send to IPv4
        inet::IPv4ControlInfo *controlInfo = new inet::IPv4ControlInfo();
        controlInfo->setDestAddr(destAddr.toIPv4());
        controlInfo->setProtocol(protocol);
        payload->setControlInfo(controlInfo);

        EV << "Sending packet: ";
        printPacket(payload);
        std::cout << "Send: " << payload->getFullName() << " at time = " << simTime()  << endl;
        send(payload, "ipOut");
    }
    else
    {
        // send to IPv6
        inet::IPv6ControlInfo *controlInfo = new inet::IPv6ControlInfo();
        controlInfo->setDestAddr(destAddr.toIPv6());
        controlInfo->setProtocol(protocol);
        payload->setControlInfo(controlInfo);

        EV << "Sending packet: ";
        printPacket(payload);

        send(payload, "ipv6Out");
    }
    numSent++;
}

void AnsaIPTrafGen::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        // send, then reschedule next sending
        sendPacket();

        if (!numPackets || numSent<numPackets)
            scheduleAt(simTime()+(double)par("packetInterval"), msg);
    }
    else
    {
        // process incoming packet
        processPacket(PK(msg));
    }

    if (ev.isGUI())
    {
        char buf[40];
        sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
        getDisplayString().setTagArg("t",0,buf);
    }
}


