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

#include "ansa/networklayer/multi/MultiNetworkNodeConfigurator.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/networklayer/ipv6/IPv6InterfaceData.h"

namespace inet {

Define_Module(MultiNetworkNodeConfigurator);

void MultiNetworkNodeConfigurator::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        cModule *node = getContainingNode(this);
        if (!node)
            throw cRuntimeError("The container @networkNode module not found");

        nodeStatus = dynamic_cast<NodeStatus*>(node->getSubmodule("status"));
        interfaceTable = dynamic_cast<IInterfaceTable*>(node->getSubmodule("interfaceTable"));

        rt4 = ( par(PAR_ENAIP4) ? (dynamic_cast<IIPv4RoutingTable*>(node->getSubmodule("routingTable")->getSubmodule("ipv4"))) : nullptr);
        rt6 = ( par(PAR_ENAIP6) ? (dynamic_cast<IPv6RoutingTable*>(node->getSubmodule("routingTable")->getSubmodule("ipv6"))) : nullptr);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER_3 && par("enableANSAConfig").boolValue() ) {
        if (!nodeStatus || nodeStatus->getState() == NodeStatus::UP) {
            parseConfig(par(PAR_CONFIG).xmlValue());
        }
    }

}

bool MultiNetworkNodeConfigurator::handleOperationStage(
        LifecycleOperation* operation, int stage, IDoneCallback* doneCallback) {
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
//        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_LINK_LAYER)
//            prepareNode();
//        else if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_NETWORK_LAYER && networkConfigurator)
//            configureNode();
        ;
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {    /*nothing to do*/
        ;
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {    /*nothing to do*/
        ;
    }
    else
        throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

void MultiNetworkNodeConfigurator::parseConfig(cXMLElement* config) {
    //Config element is empty
    if (!config) {
        EV << "ANSA configurator does not have any input XML" << endl;
        return;
    }

    //Go through all interfaces and look for GLBP section
    cXMLElementList ifc = config->getChildrenByTagName("Interface");
    for (cXMLElementList::iterator i = ifc.begin(); i != ifc.end(); ++i) {
        cXMLElement *m = *i;
        std::string ifname = m->getAttribute("name");

        InterfaceEntry* ie = interfaceTable->getInterfaceByName(ifname.c_str());
        EV << "!!!!!!!!!!!!!!!!" << interfaceTable->getInterfaceByName(ifname.c_str())->info();


        cXMLElementList ifDetails = m->getChildren();
        bool ipv6DataCleaned = false;

        for (cXMLElementList::iterator ifElemIt = ifDetails.begin(); ifElemIt != ifDetails.end(); ifElemIt++)
        {
            std::string nodeName = (*ifElemIt)->getTagName();

            // IPV4 configuration
            if (par(PAR_ENAIP4).boolValue() && nodeName=="IPAddress") {
                IPv4Address ipv4addr = IPv4Address((*ifElemIt)->getNodeValue());
                if (ipv4addr.isUnicast()) {
                    //ie->ipv4Data()->setIPAddress(ipv4addr);
                }
                else {
                    EV << "Something wrong with configured address." << endl;
                }
            }
            if (par(PAR_ENAIP4).boolValue() && nodeName=="Mask") {
                IPv4Address ipv4mask = IPv4Address((*ifElemIt)->getNodeValue());
                if (ipv4mask.isValidNetmask()) {
                    ie->ipv4Data()->setNetmask(ipv4mask);
                }
                else {
                    EV << "Something wrong with the mask." << endl;
                }
            }
            if (par(PAR_ENAIP6).boolValue() && nodeName=="IPv6Address") {
                if (!ipv6DataCleaned) {
                    for (int i = 0; i < ie->ipv6Data()->getNumAddresses(); i++) {
                        ie->ipv6Data()->removeAddress(ie->ipv6Data()->getAddress(i));
                    }
                    ipv6DataCleaned = true;
                }

                std::string addrFull = (*ifElemIt)->getNodeValue();
                IPv6Address ipv6;
                int prefixLen;

                 // check if it's a valid IPv6 address string with prefix and get prefix
                 if (!ipv6.tryParseAddrWithPrefix(addrFull.c_str(), prefixLen)){
                    EV << "Unable to set IPv6 address." << endl;
                 }

                 ipv6 = IPv6Address(addrFull.substr(0, addrFull.find_last_of('/')).c_str());

                 // IPv6NeighbourDiscovery doesn't implement DAD for non-link-local addresses
                 // -> we have to set the address as non-tentative
                 ie->ipv6Data()->assignAddress(ipv6, false, 0, 0);
            }

            if (nodeName=="MTU") {
                ie->setMtu(atoi((*ifElemIt)->getNodeValue()));
            }
            if (nodeName=="Metric") {
                ie->setMtu(atoi((*ifElemIt)->getNodeValue()));
            }
/* FIXME: Vesely - EIGRP support
            if (nodeName=="Bandwidth")
            { // Bandwidth in kbps
                if (!xmlParser::Str2Int(&tempNumber, (*ifElemIt)->getNodeValue()))
                    throw cRuntimeError("Bad value for bandwidth on interface %s", ifaceName);
                ie->setBandwidth(tempNumber);

            }
            if (nodeName=="Delay")
            { // Delay in tens of microseconds
                if (!xmlParser::Str2Int(&tempNumber, (*ifElemIt)->getNodeValue()))
                    throw cRuntimeError("Bad value for delay on interface %s", ifaceName);
                ie->setDelay(tempNumber * 10);
            }
*/
        }

        EV << "\n\n@@@@@@@@@@@@\n" << interfaceTable->getInterfaceByName(ifname.c_str())->info();
    }

}

} //namespace
