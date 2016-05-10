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

#ifndef GLBPVIRTUALROUTER_H_
#define GLBPVIRTUALROUTER_H_

#include <omnetpp.h>

#include "GLBPMessage_m.h"
#include "GLBPMessage.h"

#include "ansa/networklayer/common/AnsaInterfaceEntry.h"
#include "GLBPVirtualForwarder.h"

#include "inet/networklayer/arp/ipv4/ARP.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"
#include "inet/common/INETDefs.h"

namespace inet {
/**
* @file GLBPVirtualRouter.h
* @author Jan Holusa
* @brief Main class of GLBP.
*/
class GLBPVirtualRouter: public cSimpleModule, public cListener{
    protected:
        /** An GLBP Message Type.
         *  Contains different combinations of GLBP messages.
         */
        enum GLBPMessageType{
            HELLOm = 1,
            REQUESTm = 2,
            RESPONSEm = 3,
            COMBINEDm = 4,
            COUPm = 5,
        };

        /** An GLBP Load-Balancing type.
         *  Contains different load-balancing types. For now, just
         *  Round-Robin is implemented.
         */
        enum GLBPLoadBalancingType{
            ROUNDROBIN = 1,
            //TODO
//            WEIGHTED = 2,
//            ENDSTATIONS = 3,
        };

        std::string hostname;    //!< Hostname of the router device

        /* Variable needed for UDP */
        UDPSocket *socket;      //!< UDP socket used for sending messages
        int glbpUdpPort;        //!< glbp udp port (usually 3222)

        /* Variables needed for  OMNET */
        IInterfaceTable *ift = nullptr;                 //!< usable interfaces of this router
        ARP *arp = nullptr;                             //!< arp table for sending arp gratuious.
        AnsaInterfaceEntry *ie = nullptr;               //!< Interface which is running GLBP group
        std::vector<GLBPVirtualForwarder *> vfVector;   //!< Vector of VF's this Virtual Router is using (up to 4)
        cModule *containingModule = nullptr;            //!< helper for looking for particular module
        GLBPVirtualForwarder *vf = nullptr;

        /* GLBP specific variables */
        int glbpState;                      //!< state of hsrp virtual router
        int glbpGroup;                      //!< group of hsrp virtual router
        L3Address *glbpMulticast = nullptr; //!< multicast address of GLBP (224.0.0.102)
        IPv4Address *virtualIP = nullptr;   //!< Primary IP of the GLBP group
        MACAddress *virtualMAC = nullptr;   //!< MAC of the GLBP group
        bool preempt;                       //!< preemption flag
        unsigned int priority;              //!< priority value of hsrp group
        unsigned int helloTime;             //!< hello time interval
        unsigned int holdTime;              //!< holdtime interval
        unsigned int redirect;              //!< redirect interval
        unsigned int timeout;               //!< timeout interval
        GLBPLoadBalancingType loadBalancing; //!< Type of the load-balancing algorithm

        //VF specific
//        int vfCount = 1;        //!<
        int vfMax = 4;          //!< maximal mumber of VF. By Cisco specification is 4.
        int weight = 100;       //!< weight of the VF's. Default value is 100.

        /* GLBP VG timers */
        cMessage *hellotimer = nullptr;     //!< hellotimer OMNeT++ style
        cMessage *activetimer = nullptr;    //!< activetimer OMNeT++ style
        cMessage *standbytimer = nullptr;   //!< standbytimer OMNeT++ style

        /* GLBP VF timers*/
        std::vector <cMessage *> activetimerVf; //!< Vector of active timers for VF's in OMNeT++ stlye
        std::vector <cMessage *> redirecttimer; //!< Vector of redirect timers for VF's in OMNeT++ stlye
        std::vector <cMessage *> timeouttimer;  //!< Vector of timeout timers for VF's in OMNeT++ stlye

        cMessage *initmessage = nullptr;        //!< Self message initializing start of the GLBP protocol



    protected:
        /**
         * Startup initializacion of GLBP
         */
        //@{
        virtual void initialize( int stage);
        virtual int numInitStages() const {return NUM_INIT_STAGES;};
        //@}

        /**
         * Function is handeling incomming messages
         * and calling different methods depending on the
         * VG state.
         */
        void handleMessage(cMessage *msg);

        /**
         * Generate MAC address of VF according to GLBP specification.
         * Uses number of forwarder which is set by parameter and
         * GLBP group number.
         * @param n an integer argument which is typically
         * in range <1,4>.
         * @return pointer to MACAddress object.
         */
        MACAddress *setVirtualMAC(int n);

        /**
         * Schedule of different GLBP timers.
         * @param msg pointer to GLBP timer.
         */
        void scheduleTimer(cMessage *msg);

        /*
         * Compare incoming message with my priority, if the
         * priority is the same, compare IP address of
         * src IP address and incoming interface IP address
         * @param msg incoming message
         */
        bool isHigherPriorityThan(GLBPMessage *msg);

        /**
         * OMNeT++ signal handlers
         */
        //@{
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG) override;
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, bool b) override;
        //@}

        /**
         * Generate GLBP Hello TLV message from own class parameters.
         * @return pointer to GLBPHello message object.
         */
        GLBPHello *generateHelloTLV();

        /**
         * Generate GLBP RequesResponse TLV message.
         * @param integer number of forwarder (1-5, where 5 is sign
         * that there is no more available VF)
         * @return pointer to GLBPRequestResponse message object.
         */
        GLBPRequestResponse *generateReqRespTLV(int forwarder);

        /**
         * Send HELLOm, COMBINEDm (contains GLBPHello and GLBPRequestResponse).
         * TODO: send COUPm
         * @param type of glbp message (HELLOm, COMBINEDm or COUPm)
         */
        void sendMessage(GLBPMessageType type);

        /**
         * Send GLBPMessage containing only one GLBPRequestResponse message
         * which is advertisement that VF is just turned into ACTIVE
         * state.
         * @param type type of message which is always GLBPRequestResponse.
         * @param forwarder is number of forwarder.
         */
        void sendMessage(GLBPMessageType type, int forwarder);

        /**
         * Send GLBPMessage containing only empty GLBPRequestResponse packet part.
         * This is sign that router is asking to be a VF or AVG is
         * responding to request. This message is send as unicast.
         * @param type GLBPMessageType but just REQUESTm or RESPONSEm
         * @param IP address where to send unicast.
         */
        void sendMessageRequestResponse(GLBPMessageType type, IPv4Address *IP);

        /**
         * Learn hellotimer and holdtimer from AVG
         * @param msg avg message
         */
        void learnTimers(GLBPHello *msg);

        /**
         * VG State machine
         */
        //@{
        void initState();
        void listenState();
        void speakState();
        void standbyState();
        void activeState();
        void disabledState();
        void startAvg();
        void stopAvg();
        //@}

        /**
         * Helper for AVG to getting another MAC Address
         * of VF which is working.
         * @return MACAddress pointer
         * @see receiveSignal(cComponent *source, simsignal_t signalID, bool b)
         */
        MACAddress *getNextActiveMac();

        /**
         * Load balancing mechanism
         * @param type Type of the loadbalancing algorithm
         */
        MACAddress *loadBalancingNext(GLBPLoadBalancingType type);

        /**
         * VF state machine functions.
         */
        //@{
        void startVf(int n);
        void stopVf();
        void addVf(int n, MACAddress *macOfPrimary);
        int isVfSelfMessage(cMessage *msg);
        void handleVfSelfMessage(cMessage *msg);
        bool isVfActive();
        void addOrStartVf(GLBPMessage *msg);
        void handleMessageRequestResponse(GLBPMessage *msg);
        bool isHigherVfPriorityThan(GLBPMessage *GLBPm, int forwarder, int pos);
        void deleteVf(int forwarder);
        int getFreeVf();
        //@}

        /**
         * Reaction to interface state change
         */
        //@{
        void interfaceUp();
        void interfaceDown();
        //@}

        /**
         * Message hendler for router in LISTEN state
         * @param msg
         */
        void handleMessageListen(GLBPMessage *msg);

        /**
         * Message hendler for router in SPEAK state
         * @param msg
         */
        void handleMessageSpeak(GLBPMessage *msg);

        /**
         * Message hendler for router in STANDBY state
         * @param msg
         */
        void handleMessageStandby(GLBPMessage *msg);

        /**
         * Message hendler for router in ACTIVE state
         * @param msg
         */
        void handleMessageActive(GLBPMessage *msg);

        /**
         * Methods for debugging. Using outputs into simulation log window.
         */
        //@{
        void DebugStateMachine(int from, int to);
        void DebugVfStateMachine(int from, int to, int forwarder);
        void DebugSendMessage(GLBPMessageType t);
        void DebugGetMessage(GLBPMessage *msg);
        void DebugUnknown(int who);
        std::string intToGlbpState(int state);
        std::string intToMessageType(GLBPMessageType msg);
        //@}

    public:
        /**
         * Group getter
         * @return number of GLBP Group.
         */
        int getGroup() const { return glbpGroup; };

        GLBPVirtualRouter();
        virtual ~GLBPVirtualRouter();
};

} /* namespace inet */

#endif /* GLBPVIRTUALROUTER_H_ */
