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

#ifndef __ANSAINET_MULTINETWORKNODECONFIGURATOR_H_
#define __ANSAINET_MULTINETWORKNODECONFIGURATOR_H_

#include <omnetpp.h>

#include "inet/common/INETDefs.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/networklayer/ipv6/IPv6RoutingTable.h"

using namespace omnetpp;

namespace inet {

class INET_API MultiNetworkNodeConfigurator : public cSimpleModule, public ILifecycle
{

  protected:
    const char* PAR_CONFIG = "configData";
    const char* PAR_ENAIP4 = "enableIPv4";
    const char* PAR_ENAIP6 = "enableIPv6";

    NodeStatus *nodeStatus;
    IInterfaceTable *interfaceTable;
    IIPv4RoutingTable *rt4;
    IPv6RoutingTable *rt6;


    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("this module doesn't handle messages, it runs only in initialize()"); }
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

    void parseConfig(cXMLElement *config);
};

} //namespace

#endif
