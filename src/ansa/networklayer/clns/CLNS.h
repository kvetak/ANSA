// Copyright (C) 2012 - 2016 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
 * @file CLNS.h
 * @author Marcel Marek (mailto:imarek@fit.vutbr.cz)
 * @date 5.8.2016
 */


#ifndef __ANSAINET_CLNS_H_
#define __ANSAINET_CLNS_H_

#include <vector>

#include "inet/common/INETDefs.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/networklayer/contract/INetworkProtocol.h"
//#include "ansa/networklayer/isis/ISISMessage_m.h"
//#include "inet/common/ProtocolMap.h"
#include "inet/common/queue/QueueBase.h"
#include "inet/common/packet/Packet.h"

#include "inet/networklayer/contract/clns/ClnsAddress.h"
//#include "inet/networklayer/common/InterfaceEntry.h"


using namespace omnetpp;

namespace inet {

//class ISISMessage;
class IInterfaceTable;
class CLNSRoutingTable;
class InterfaceEntry;

/**
 * TODO - Generated class
 */
class INET_API CLNS : public QueueBase, public INetworkProtocol, public IProtocolRegistrationListener
{
  public:
    typedef std::vector<ClnsAddress> CLNSAddressVector;

  private:

    // local addresses cache (to speed up isLocalAddress())
    CLNSAddressVector addressVector;


    CLNSAddressVector  localAddresses; //TODO A! Load it from .xml

  protected:
    CLNSRoutingTable *rt = nullptr;
    IInterfaceTable *ift = nullptr;

    int transportInGateBaseId = -1;
    int queueOutGateBaseId = -1;

//    ProtocolMapping mapping;
    bool isUp = false;


    // statistics
    int numMulticast = 0;
    int numLocalDeliver = 0;
    int numDropped = 0;    // forwarding off, no outgoing interface, too large but "don't fragment" is set, TTL exceeded, etc
    int numUnroutable = 0;
    int numForwarded = 0;

    std::set<const Protocol *> upperProtocols;    // where to send packets after decapsulation




  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg);
    virtual void updateDisplayString();

    virtual void endService(cPacket *packet) override;

    virtual void handlePacketFromHL(Packet *packet);
    virtual void handleIncomingISISMessage(Packet* packet, const InterfaceEntry *fromIE);
    virtual const InterfaceEntry *getSourceInterfaceFrom(Packet *packet);


  public:
    ClnsAddress getKAddress(unsigned int k) const;

    virtual void handleRegisterService(const Protocol& protocol, cGate *out, ServicePrimitive servicePrimitive) override;
    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *in, ServicePrimitive servicePrimitive) override;

};

} //namespace

#endif
