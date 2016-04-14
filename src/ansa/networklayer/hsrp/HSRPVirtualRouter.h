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
#include "ansa/networklayer/common/AnsaInterfaceEntry.h"

#include "inet/transportlayer/contract/udp/UDPSocket.h"
#include "inet/networklayer/arp/ipv4/ARP.h"
#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/networklayer/arp/ipv4/ARPPacket_m.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"

namespace inet {

class HSRPVirtualRouter : public cSimpleModule, public cListener{
    protected:
        std::string hostname;

        /* Variable needed for UDP */
        UDPSocket *socket;      //UDP socket used for sending messages
        int hsrpUdpPort;        //hsrp udp port (usually 1985)

        /* Variables needed for  OMNET */
        IInterfaceTable *ift = nullptr;      //usable interfaces of this router
        ARP *arp = nullptr;                  //arp table for sending arp gratuious.
        AnsaInterfaceEntry *ie = nullptr;    //Interface which is running HSRP group
        VirtualForwarder *vf = nullptr;      //Particular HSRP group is represented by VF on each interface
        cModule *containingModule = nullptr; //helper for looking for particular module

        /* HSRP specific variables */
        int HsrpState;                      //state of hsrp virtual router
        int HSRPgroup;                      //group of hsrp virtual router
        L3Address *HsrpMulticast = nullptr; //multicast address of HSRP (224.0.0.2)
        IPv4Address *virtualIP = nullptr;   //Primary IP of the HSRP group
        MACAddress *virtualMAC = nullptr;   //MAC of the HSRP group
        bool preempt;                       //preemption flag
        int priority;                       //priority value of hsrp group
        int helloTime;
        int holdTime;

        /* HSRP timers */
        cMessage *hellotimer = nullptr;
        cMessage *activetimer = nullptr;
        cMessage *standbytimer = nullptr;
        cMessage *initmessage = nullptr;

        /* Signals */


    protected:
        virtual void initialize( int stage);
        virtual int numInitStages() const {return NUM_INIT_STAGES;};
        void handleMessage(cMessage *msg);
        void sendMessage(OP_CODE opCode);
        void setVirtualMAC();
        HSRPMessage *generateMessage(OP_CODE opCode);
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
        void learnTimers(HSRPMessage *msg);
        bool isHigherPriorityThan(HSRPMessage *HSRPm);
        void interfaceDown();
        void interfaceUp();

        //debug
        void DebugStateMachine(int from, int to);
        void DebugGetMessage(HSRPMessage *msg);
        void DebugSendMessage(int op_code);
        std::string intToHsrpState(int state);
        std::string intToMessageType(int msg);

        //signal handler
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG) override;

    public:
        virtual AnsaInterfaceEntry* getInterface() { return ie; };
        int getGroup() const { return HSRPgroup; };

        HSRPVirtualRouter();
        virtual ~HSRPVirtualRouter();
    };

} /* namespace inet */

#endif /* HSRPVIRTUALROUTER_H_ */

