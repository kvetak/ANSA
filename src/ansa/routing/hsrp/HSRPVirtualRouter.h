/*
 * HSRPVirtualRouter.h
 *
 *  Created on: 24. 3. 2016
 *      Author: Jan Holusa
 */

#ifndef HSRPVIRTUALROUTER_H_
#define HSRPVIRTUALROUTER_H_

#include <omnetpp.h>
#include "ansa/routing/hsrp/HSRPMessage_m.h"

#include "ansa/networklayer/common/ANSA_InterfaceEntry.h"

#include "inet/transportlayer/contract/udp/UDPSocket.h"
#include "inet/networklayer/arp/ipv4/ARP.h"
#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/networklayer/arp/ipv4/ARPPacket_m.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/common/lifecycle/ILifecycle.h"

namespace inet {
/**
* @file HSRPVirtualRouter.h
* @author Jan Holusa
* @brief Main class of HSRP.
*/
class HSRPVirtualRouter : public cSimpleModule, public cListener
{
    protected:
        std::string hostname;                   //!< Hostname of the router device

        /* Variable needed for UDP */
        UDPSocket *socket;                      //!< UDP socket used for sending messages
        int hsrpUdpPort;                        //!< hsrp udp port (usually 1985)

        /* Variables needed for  OMNET */
        IInterfaceTable *ift = nullptr;         //!< Usable interfaces of this router
        ARP *arp = nullptr;                     //!< Arp table for sending arp gratuious.
        ANSA_InterfaceEntry *ie = nullptr;       //!< Interface which is running HSRP group
        VirtualForwarder *vf = nullptr;         //!< Particular HSRP group is represented by VF on each interface
        cModule *containingModule = nullptr;    //!< helper for looking for particular module

        /* HSRP specific variables */
        int hsrpState;                          //!< State of hsrp virtual router
        int hsrpGroup;                          //!< Group of hsrp virtual router
        L3Address *hsrpMulticast = nullptr;     //!< Multicast address of HSRP (224.0.0.2)
        IPv4Address *virtualIP = nullptr;       //!< Primary IP of the HSRP group
        MACAddress *virtualMAC = nullptr;       //!< MAC of the HSRP group
        bool preempt;                           //!< Preemption flag
        int priority;                           //!< Priority value of hsrp group
        int helloTime;                          //!< Hello time value
        int holdTime;                           //!< Hold time value

        /* HSRP timers */
        cMessage *hellotimer = nullptr;         //!< Hello timer in OMNeT++ style
        cMessage *activetimer = nullptr;        //!< Active timer in OMNeT++ style
        cMessage *standbytimer = nullptr;       //!< Standby timer in OMNeT++ style

        cMessage *initmessage = nullptr;        //!< Self message initializing start of the HSRP protocol

        /* Signals */


    protected:
        /**
         * Startup initializacion of HSRP
         */
        //@{
        virtual void initialize( int stage) override;
        virtual int numInitStages() const override {return NUM_INIT_STAGES;} ;
        //@}

        /**
         * OMNET++ function for handeling incoming messages
         * @param msg incoming (or self) message
         */
        void handleMessage(cMessage *msg) override;

        /**
         * Message sender with set opCode
         * @param opCode opcode of the message to send
         * @see generateMessage(opCode)
         */
        void sendMessage(OP_CODE opCode);

        /**
         * Generate message from method params
         * @param opCode opcode of the message to send
         * @see sendMessage(opCode)
         */
        HSRPMessage *generateMessage(OP_CODE opCode);

        /**
         * Generate MAC address of HSRP group
         * and set it as a virtualMac parameter
         * @see virtualMAC.
         */
        void setVirtualMAC();

        /**
         * Schedule of different HSRP timers.
         * @param msg pointer to HSRP timer.
         */
        void scheduleTimer(cMessage *msg);

        /**
         * Incoming message handlers for different
         * HSRP states.
         * @see initState()
         * @see listenState()
         * @see learnState
         */
        //@{
        void handleMessageStandby(HSRPMessage *msg);
        void handleMessageSpeak(HSRPMessage *msg);
        void handleMessageActive(HSRPMessage *msg);
        void handleMessageListen(HSRPMessage *msg);
        void handleMessageLearn(HSRPMessage *msg);
        //@}

        /**
         * HSRP State machine
         */
        //@{
        void initState();
        void listenState();
        void learnState();
        //@}

        /**
         * Learn hellotime and holdtime from
         * incoming message.
         * @param msg pointer to incoming message
         */
        void learnTimers(HSRPMessage *msg);

        /**
         * Compare incoming message with my priority, if the
         * priority is the same, compare IP address of
         * src IP address and incoming interface IP address
         * @param msg incoming message
         */
        bool isHigherPriorityThan(HSRPMessage *HSRPm);

        /**
         * Reaction to interface state change
         */
        //@{
        void interfaceDown();
        void interfaceUp();
        void stopHSRP();
        //@}

        /**
         * Methods for debugging. Using outputs into simulation log window.
         */
        //@{
        void DebugStateMachine(int from, int to);
        void DebugGetMessage(HSRPMessage *msg);
        void DebugSendMessage(int op_code);
        std::string intToHsrpState(int state);
        std::string intToMessageType(int msg);
        //@}

        /**
         * OMNeT++ signal handler
         */
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG) override;

    public:
        /**
         * Interface getter
         * @return AnsaInterfaceEntry pointer
         */
        virtual ANSA_InterfaceEntry* getInterface() { return ie; };

        /**
         * Group getter
         * @return number of HSRP Group.
         */
        int getGroup() const { return hsrpGroup; };

        HSRPVirtualRouter();
        virtual ~HSRPVirtualRouter();
    };

} /* namespace inet */

#endif /* HSRPVIRTUALROUTER_H_ */

