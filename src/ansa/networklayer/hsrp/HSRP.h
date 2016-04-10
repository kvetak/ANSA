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
#include "HSRPVirtualRouter.h"

namespace inet {
/**
 * The HSRP class represents a communication gateway with HSRP modules modules.
 *
 * @author Jan Holusa
 */
class HSRP : public cSimpleModule{
    protected:
        const char* CONFIG_PAR = "configData";
        std::vector<HSRPVirtualRouter *> virtualRouterTable;
        std::vector<int> multicastInterfaces;
        L3Address *HsrpMulticast = nullptr;
        UDPSocket *socket;    // bound to the HSRP port (see udpPort parameter)

        IInterfaceTable *ift = nullptr;
    protected:
        virtual void initialize( int stage);
        virtual int numInitStages() const {return NUM_INIT_STAGES;};
        virtual void handleMessage(cMessage *msg);
        virtual void updateDisplayString();
        void sendMessage(OP_CODE opCode);
        void parseConfig(cXMLElement *config);
        void addVirtualRouter(int interface, int vrid, const char* ifnam, std::string vip, int priority, bool preempt);
        void checkAndJoinMulticast(int InterfaceId);

    public:
        HSRP();
        virtual ~HSRP();
};

} /* namespace inet */

#endif /* HSRP_H_ */
