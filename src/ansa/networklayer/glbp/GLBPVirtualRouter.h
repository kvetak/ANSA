/*
 * GLBPVirtualRouter.h
 *
 *  Created on: 18.4. 2016
 *      Author: Jan Holusa
 */
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

//#include "GLBP_m.h"
//#include "GLBPHello_m.h"
#include "GLBPMessage_m.h"
#include "GLBPMessage.h"

#include "ansa/networklayer/common/AnsaInterfaceEntry.h"
#include "GLBPVirtualForwarder.h"

#include "inet/networklayer/arp/ipv4/ARP.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"
#include "inet/common/INETDefs.h"

namespace inet {

class GLBPVirtualRouter: public cSimpleModule, public cListener{
    protected:
        enum GLBPMessageType{
            HELLOm = 1,
            REQUESTm = 2,
            RESPONSEm = 3,
            COMBINEDm = 4,
            COUPm = 5,
        };

        std::string hostname;

        /* Variable needed for UDP */
        UDPSocket *socket;      //UDP socket used for sending messages
        int glbpUdpPort;        //glbp udp port (usually 3222)

        /* Variables needed for  OMNET */
        IInterfaceTable *ift = nullptr;      //usable interfaces of this router
        ARP *arp = nullptr;                  //arp table for sending arp gratuious.
        AnsaInterfaceEntry *ie = nullptr;    //Interface which is running GLBP group
        std::vector<GLBPVirtualForwarder *> vfVector; //Vector of VF's this Virtual Router is using (up to 4)
        cModule *containingModule = nullptr; //helper for looking for particular module
        GLBPVirtualForwarder *vf = nullptr;

        /* GLBP specific variables */
        int glbpState;                      //state of hsrp virtual router
        int glbpGroup;                      //group of hsrp virtual router
        L3Address *glbpMulticast = nullptr; //multicast address of GLBP (224.0.0.102)
        IPv4Address *virtualIP = nullptr;   //Primary IP of the GLBP group
        MACAddress *virtualMAC = nullptr;   //MAC of the GLBP group
        bool preempt;                       //preemption flag
        int priority;                       //priority value of hsrp group
        int helloTime;
        int holdTime;
        int redirect;
        int timeout;

        //VF specific
        int vfCount = 1;
        int vfMax = 4;
        int weight = 100; //TODO Parametrem

        /* GLBP VG timers */
        cMessage *hellotimer = nullptr;
        cMessage *activetimer = nullptr;
        cMessage *standbytimer = nullptr;

        /* GLBP VF timers*/
        std::vector <cMessage *> activetimerVf;// = nullptr;
        std::vector <cMessage *> redirecttimer;// = nullptr;
        std::vector <cMessage *> timeouttimer;// = nullptr;

        cMessage *initmessage = nullptr;



    protected:
        virtual void initialize( int stage);
        virtual int numInitStages() const {return NUM_INIT_STAGES;};
        void handleMessage(cMessage *msg);
//        void sendMessage(OP_CODE opCode);
        MACAddress *setVirtualMAC(int n);
        void scheduleTimer(cMessage *msg);
        bool isHigherPriorityThan(GLBPMessage *msg);
        //signal handler
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG) override;
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, bool b) override;

//        GLBPMessage *generateMessage();
        GLBPHello *generateHelloTLV();
        GLBPRequestResponse *generateReqRespTLV(int forwarder);
        void sendMessage(GLBPMessageType type);
        void sendMessage(GLBPMessageType type, int forwarder);
        void sendMessageRequestResponse(GLBPMessageType type, IPv4Address *IP);

        //VG state machine
        void initState();
        void listenState();
        void speakState();
        void standbyState();
        void activeState();
        void disabledState();
        void startAvg();
        void stopAvg();
        MACAddress *getNextActiveMac();

        //VF state machine
        void startVf(int n);
        void stopVf();
        void addVf(int n, MACAddress *macOfPrimary);
        int isVfSelfMessage(cMessage *msg);
        void handleVfSelfMessage(cMessage *msg);
        void vfIncrement();
        bool isVfActive();
        void addOrStartVf(GLBPMessage *msg);
        void handleMessageRequestResponse(GLBPMessage *msg);
        bool isHigherVfPriorityThan(GLBPMessage *GLBPm, int forwarder, int pos);
        void deleteVf(int forwarder);
        int getFreeVf();

        void interfaceUp();
        void interfaceDown();

        void handleMessageListen(GLBPMessage *msg);
        void handleMessageSpeak(GLBPMessage *msg);
        void handleMessageStandby(GLBPMessage *msg);
        void handleMessageActive(GLBPMessage *msg);

        //debug
        void DebugStateMachine(int from, int to);
        void DebugSendMessage(GLBPMessageType t);
        void DebugGetMessage(GLBPMessage *msg);
        void DebugUnknown(int who);

        std::string intToGlbpState(int state);
        std::string intToMessageType(GLBPMessageType msg);
    public:
        int getGroup() const { return glbpGroup; };

        GLBPVirtualRouter();
        virtual ~GLBPVirtualRouter();
};

} /* namespace inet */

#endif /* GLBPVIRTUALROUTER_H_ */
