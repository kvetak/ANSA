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

#ifndef HSRP_H_
#define HSRP_H_

#include <omnetpp.h>
#include "HSRPMessage_m.h"

#include "inet/transportlayer/contract/udp/UDPSocket.h"
#include "inet/networklayer/arp/ipv4/ARP.h"
#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/networklayer/arp/ipv4/ARPPacket_m.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
//#include "inet/common/lifecycle/ILifecycle.h"

namespace inet {

class HSRP : public cSimpleModule{
    protected:
        std::string hostname;
        UDPSocket *socket;    // bound to the HSRP port (see udpPort parameter)
        int hsrpUdpPort; //hsrp udp port (usually 1985)
        IL3AddressType *addressType = nullptr;    // address type of the routing table
        int HsrpState;
        IInterfaceTable *ift = nullptr;
        IRoutingTable *rt = nullptr;
        ARP *arp = nullptr;
        L3Address *HsrpMulticast = nullptr;
        IPv4Address *virtualIP = nullptr;
        MACAddress *virtualMAC = nullptr;
        cMessage *hellotimer = nullptr;
        cMessage *activetimer = nullptr;
        cMessage *standbytimer = nullptr;
        cMessage *initmessage = nullptr;
        cModule *containingModule = nullptr;

    protected:
        virtual void initialize( int stage);
        virtual int numInitStages() const {return NUM_INIT_STAGES;};
        void handleMessage(cMessage *msg);
        void sendMessage(OP_CODE opCode);
        HSRPMessage *generateMessage(OP_CODE opCode);
        void bindMulticast();
        void scheduleTimer(cMessage *msg);
        void handleMessageStandby(HSRPMessage *msg);
        void handleMessageSpeak(HSRPMessage *msg);
        void handleMessageActive(HSRPMessage *msg);
        void handleMessageListen(HSRPMessage *msg);
        void handleMessageLearn(HSRPMessage *msg);
        void initState();
        void listenState();
        void learnState();
        void speakState();


    public:
        HSRP();
        virtual ~HSRP();
};

} /* namespace inet */

#endif /* HSRP_H_ */
