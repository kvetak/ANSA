/*
 * GLBP.h
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

#ifndef GLBP_H_
#define GLBP_H_

#include <omnetpp.h>

#include "GLBPHello_m.h"
#include "GLBPReqResp_m.h"
#include "GLBPVirtualRouter.h"

#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/common/ModuleAccess.h"
//#include "inet/transportlayer/contract/udp/UDPSocket.h"

namespace inet {

class GLBP : public cSimpleModule {
    protected:
        /*Path to config file with GLBP section specified in onmet.ini file*/
        const char* CONFIG_PAR = "configData";
        /*GLBP groups on this router*/
        std::vector<GLBPVirtualRouter *> virtualRouterTable;
        /*Joined multicast interfaces for GLBP listening*/
        std::vector<int> multicastInterfaces;
        /*GLBP multicast address*/
        L3Address *glbpMulticastAddress = nullptr;
        UDPSocket *socket;

        IInterfaceTable *ift = nullptr;

    protected:
        /*Startup initializacion of GLBP*/
        virtual void initialize( int stage);
        virtual int numInitStages() const {return NUM_INIT_STAGES;};
        /*OMNET++ function for handeling incoming messages*/
        virtual void handleMessage(cMessage *msg);
        virtual void updateDisplayString();
        /*Sending messages to UDP or to GLBPVirtualRouter module*/
//        void sendMessage(OP_CODE opCode);
        /**/
        void parseConfig(cXMLElement *config);
        void addVirtualRouter(int interface, int vrid, const char* ifnam, std::string vip, int priority, bool preempt);
        void checkAndJoinMulticast(int InterfaceId);

    public:
        GLBP();
        virtual ~GLBP();
};

} /* namespace inet */

#endif /* GLBP_H_ */
