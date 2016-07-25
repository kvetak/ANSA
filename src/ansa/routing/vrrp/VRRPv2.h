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
* @file VRRPv2.h
* @author Petr Vitek
* @brief
* @detail
*/
#ifndef __ANSA_VRRPV2_H_
#define __ANSA_VRRPV2_H_

#include <omnetpp.h>
#include "VRRPv2VirtualRouter.h"

/**
 * The VRRPv2 class represents a communication gateway with VRRPv2modules modules.
 *
 * @author Petr Vitek
 */
class VRRPv2 : public cSimpleModule
{
    protected:
        std::string hostname;
        std::vector<VRRPv2VirtualRouter *> virtualRouterTable;

    protected:
        virtual int numInitStages() const { return 4; };
        virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg);
        virtual void updateDisplayString();

        std::string getHostname() { return hostname; };

    public:
        VRRPv2();

        /**
         * Dynamic creation of modules Virtual Router
         * @param interface interafceId
         * @param vrid      Virtual Router ID
         */
        virtual void addVirtualRouter(int interface, int vrid, const char* ifnam);

};

#endif
