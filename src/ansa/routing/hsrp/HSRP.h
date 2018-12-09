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
* @author Jan Holusa
* @author Vladimir Vesely (ivesely@fit.vutbr.cz)
* @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
*/

#ifndef HSRP_H_
#define HSRP_H_

#include <omnetpp.h>

#include "ansa/routing/hsrp/HSRPMessage_m.h"
#include "ansa/routing/hsrp/HSRPVirtualRouter.h"

#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/contract/ipv4/Ipv4ControlInfo.h"
#include "inet/common/ModuleAccess.h"

namespace inet {
/**
 * @file HSRP.h
 * @author Jan Holusa
 * @brief The HSRP class represents a communication gateway with HSRP modules modules.
 */
class HSRP : public cSimpleModule{
    protected:
        const char* CONFIG_PAR = "configData";                      //!< Path to config file with HSRP section specified in onmetpp.ini file
        std::vector<HSRPVirtualRouter *> virtualRouterTable;        //!< HSRP groups on this router
        std::vector<int> multicastInterfaces;                       //!< Joined multicast interfaces for HSRP listening
        L3Address *hsrpMulticast = nullptr;                         //!< HSRP multicast address
        UdpSocket *socket;                                          //!< bound to the HSRP port (see udpPort parameter)
        std::string hostname;                                       //!< hostname of the device
        cModule *containingModule;                                  //!< Pointer to router which contain HSRP module

        IInterfaceTable *ift = nullptr;                             //!< Pointer to interface table
    protected:
        /**
         * Startup initializacion of HSRP
         */
        //@{
        virtual void initialize( int stage);
        virtual int numInitStages() const {return NUM_INIT_STAGES;};
        //@}

        /**
         * OMNET++ function for handeling incoming messages
         * @param msg incoming (or self) message
         */
        virtual void handleMessage(cMessage *msg);
        virtual void updateDisplayString();

        /**
         * Config file parser. Adding VirtualRouters.
         * @param config part of XML file containing configuration of interface
         * @see addVirtualRouter(int interface, int vrid, const char* ifnam, std::string vip, int priority, bool preempt, int redirect, int timeout, int hellotime, int holdtime)
         */
        void parseConfig(cXMLElement *config);

        /**
         * Dynamically add GLBPVirtualRouter module
         * @param interface id of the interface containing this GLBPVirtualRouter module
         * @param vrid group id
         * @param ifnam name of the interface
         * @param vip string of virtual ip
         * @param priority priority of VG
         * @param preempt boolean value saying that preemption is set
         * @param hellotime hellotime timer value
         * @param holdtime holdtime timer value
         */
        void addVirtualRouter(int interface, int vrid, const char* ifnam, std::string vip, int priority, bool preempt, int hellotime, int holdtime);

        /**
         * Check if interface is not yet joined multicast group
         * and if not, join it.
         * @param InterfaceId id of the interface to be joined to multicast group
         * @see glbpMulticastAddress
         */
        void checkAndJoinMulticast(int InterfaceId);

        /**
         * Check if virtual IP is unique in this router.
         * Checks Physical IP of actual interface and other local interfaces
         * @param virtip string of virtual IP to be checked
         * @param iid id of interface to be checked
         * @return true if is unique, false if there already exist this address.
         */
        bool is_unique(std::string virtip, int iid);

    public:
        HSRP();
        virtual ~HSRP();
};

} /* namespace inet */

#endif /* HSRP_H_ */
