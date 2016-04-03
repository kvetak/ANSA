/*
 * HSRPVirtualRouter.h
 *
 *  Created on: 24. 3. 2016
 *      Author: Honza
 */

#ifndef HSRPVIRTUALROUTER_H_
#define HSRPVIRTUALROUTER_H_

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

namespace inet {

class HSRPVirtualRouter : public cSimpleModule{
    protected:
        std::string hostname;
        UDPSocket *socket;    // bound to the HSRP port (see udpPort parameter)
        int hsrpUdpPort; //hsrp udp port (usually 1985)
        IL3AddressType *addressType = nullptr;    // address type of the routing table
        int HsrpState;
        int HSRPgroup;
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
        void setVirtualMAC();
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
        void parseConfig(cXMLElement *config);


    public:
        HSRPVirtualRouter();
        virtual ~HSRPVirtualRouter();
    };

} /* namespace inet */

#endif /* HSRPVIRTUALROUTER_H_ */

