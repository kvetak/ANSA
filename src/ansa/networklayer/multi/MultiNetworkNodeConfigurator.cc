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
        EV_ERROR << "ANSA configurator does not have any input XML" << endl;
        return;
    }

    //Configure interface addressing
    parseInterfaces(config);
    //Configure default routes
    parseDefaultRoutes(config);
    //Configure static routes
    parseStaticRoutes(config);

}

void MultiNetworkNodeConfigurator::parseInterfaces(cXMLElement* config) {
    cXMLElement* ifaces = config->getFirstChildWithTag(XML_IFACES);
    if (!ifaces) {
        EV << "ANSA configurator does not have any Interfaces tag" << endl;
        return;
    }
    //Go through all interfaces and look for GLBP section
    cXMLElementList ifc = ifaces->getChildrenByTagName(XML_IFACE);
    if (ifc.empty()) {
        EV << "ANSA configurator does not have any Interface tag" << endl;
        return;
    }
    for (cXMLElementList::iterator i = ifc.begin(); i != ifc.end(); ++i) {
        cXMLElement *m = *i;
        std::string ifname = m->getAttribute(XML_NAME);

        InterfaceEntry* ie = interfaceTable->getInterfaceByName(ifname.c_str());
        //EV << "!!!!!!!!!!!!!!!!" << interfaceTable->getInterfaceByName(ifname.c_str())->info();

        //Setup BW and DLY
        ANSA_InterfaceEntry* aie = dynamic_cast<ANSA_InterfaceEntry*>(ie);
        if (aie) {
            //aie->setBandwidth(bwPar.doubleValue() / 1000);
            //aie->setDelay(getDefaultDelay(linkType));
        }

        cXMLElementList ifDetails = m->getChildren();
        bool ipv6DataCleaned = !(par(PAR_REPLACEADDR).boolValue());

        for (cXMLElementList::iterator ifElemIt = ifDetails.begin(); ifElemIt != ifDetails.end(); ifElemIt++)
        {
            std::string nodeName = (*ifElemIt)->getTagName();

            // IPV4 configuration
            if (par(PAR_ENAIP4).boolValue() && nodeName == XML_IPADDR) {
                IPv4Address ipv4addr = IPv4Address((*ifElemIt)->getNodeValue());
                if (ipv4addr.isUnicast()) {
                    ie->ipv4Data()->setIPAddress(ipv4addr);
                }
                else {
                    EV_ERROR << "Something wrong with configured address." << endl;
                }
            }
            if (par(PAR_ENAIP4).boolValue() && nodeName == XML_MASK) {
                IPv4Address ipv4mask = IPv4Address((*ifElemIt)->getNodeValue());
                if (ipv4mask.isValidNetmask()) {
                    ie->ipv4Data()->setNetmask(ipv4mask);
                }
                else {
                    EV_ERROR << "Something wrong with the mask." << endl;
                }
            }
            if (par(PAR_ENAIP6).boolValue() && nodeName == XML_IPADDR6) {
                if (!ipv6DataCleaned) {
                    for (int i = 0; i < ie->ipv6Data()->getNumAddresses(); i++) {
                        ie->ipv6Data()->removeAddress(ie->ipv6Data()->getAddress(i));
                    }
                    ipv6DataCleaned = true;
                }

                std::string addrFull = (*ifElemIt)->getNodeValue();
                IPv6Address ipv6;
                int prefixLen;

                // Check IPv6 address validity and initialize prefixLen variable
                if (!ipv6.tryParseAddrWithPrefix(addrFull.c_str(), prefixLen)){
                    EV_ERROR << "Unable to set IPv6 address." << endl;
                }

                //Initialize IPv6 address and add it to interface
                ipv6 = IPv6Address(addrFull.substr(0, addrFull.find_last_of('/')).c_str());
                ie->ipv6Data()->assignAddress(ipv6, false, 0, 0);

                //Do not add link-local addresses to routing table
                if (!ipv6.isLinkLocal() ) {
                    IPv6Route* ipv6route = new IPv6Route(ipv6.getPrefix(prefixLen), prefixLen, IRoute::IFACENETMASK);
                    ipv6route->setInterface(ie);
                    rt6->addRoutingProtocolRoute(ipv6route);
                }
            }

            if (nodeName == XML_MTU) {
                ie->setMtu(atoi((*ifElemIt)->getNodeValue()));
            }
            /* FIXME: Vesely - Add Metric
            if (nodeName == XML_METRIC) {
                ie->set(atoi((*ifElemIt)->getNodeValue()));
            }
            */

            if (nodeName=="Bandwidth")
            { // Bandwidth in kbps
                int tempNumber = 0;
                if (!Str2Int(&tempNumber, (*ifElemIt)->getNodeValue())) {
                    throw cRuntimeError("Bad value for bandwidth on interface!");
                }
                ANSA_InterfaceEntry* aie = dynamic_cast<ANSA_InterfaceEntry*>(ie);
                if (aie) { aie->setBandwidth(tempNumber); }

            }
            if (nodeName=="Delay")
            { // Delay in tens of microseconds
                int tempNumber = 0;
                if (!Str2Int(&tempNumber, (*ifElemIt)->getNodeValue())) {
                    throw cRuntimeError("Bad value for delay on interface!");
                }
                ANSA_InterfaceEntry* aie = dynamic_cast<ANSA_InterfaceEntry*>(ie);
                if (aie) { aie->setDelay(tempNumber); }
            }

        }

        //EV << "\n\n@@@@@@@@@@@@\n" << interfaceTable->getInterfaceByName(ifname.c_str())->info();
    }
}

void MultiNetworkNodeConfigurator::parseDefaultRoutes(cXMLElement* config) {
    //Add default route for IPv4
    cXMLElement* defrou = config->getFirstChildWithTag(XML_DEFROUTER);
    if (defrou) {
        //Parse XML
        IPv4Address ipv4addr = IPv4Address(defrou->getNodeValue());
        //EV << "Def Route addrese is " << ipv4addr << endl;
        InterfaceEntry* ie = nullptr;
        for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
            InterfaceEntry* ietmp = interfaceTable->getInterface(i);
            if (ietmp->isLoopback()) continue;
            if (ietmp->ipv4Data()->getIPAddress().prefixMatches(ipv4addr,ietmp->ipv4Data()->getNetmask().getNetmaskLength())) {
                ie = ietmp;
                break;
            }
        }
        if (ie) {
            //EV << "!!!!!!!!!!" << ie->getName() << endl;
            //Prepare default route
            IPv4Route* ipv4rou = new IPv4Route();
            ipv4rou->setDestination(IPv4Address::UNSPECIFIED_ADDRESS);
            ipv4rou->setNetmask(IPv4Address::UNSPECIFIED_ADDRESS);
            ipv4rou->setGateway(ipv4addr);
            ipv4rou->setInterface(ie);
            ipv4rou->setMetric(0);
            ipv4rou->setAdminDist(IPv4Route::dStatic);
            //Add default route as static
            rt4->addRoute(ipv4rou);
        }
        else {
            EV_ERROR << "None of node's interfaces has the same IPv4 subnet as default router address!" << endl;
        }
    }
    else { EV << "Node configuration does not contain IPv4 default route configuration!" << endl;}

    //Add default route for IPv6
    cXMLElement* defrou6 = config->getFirstChildWithTag(XML_DEFROUTER6);
    if (defrou6) {
        //Parse XML
        IPv6Address ipv6addr = IPv6Address(defrou6->getNodeValue());
        //EV << "Def Route addrese is " << ipv4addr << endl;
        InterfaceEntry* ie = nullptr;
        for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
            InterfaceEntry* ietmp = interfaceTable->getInterface(i);
            if (ietmp->isLoopback()) continue;

            for (int j = 0; j < ietmp->ipv6Data()->getNumAddresses(); j++) {
                IPv6Address ipv6tmp = ietmp->ipv6Data()->getAddress(j);
                //FIXME: Vesely - Ugly assumption that prefix length is 64 bits
                if (ipv6tmp.matches(ipv6addr,64)) {
                    ie = ietmp;
                    break;
                }
            }

        }
        if (ie) {
            //EV << "!!!!!!!!!!" << ie->getName() << endl;
            //Prepare default route
            IPv6Route* ipv6rou = new IPv6Route(IPv6Address::UNSPECIFIED_ADDRESS,0,IPv6Route::MANUAL);
            ipv6rou->setNextHop(ipv6addr);
            ipv6rou->setInterface(ie);
            ipv6rou->setMetric(0);
            ipv6rou->setAdminDist(IPv6Route::dStatic);
            //Add default route as static
            rt6->addRoute(ipv6rou);
            //rt6->addDefaultRoute(ipv6addr,ie->getInterfaceId(),0);
        }
        else {
            EV_ERROR << "None of node's interfaces has the same IPv6 subnet as default router address!" << endl;
        }
    }
    else { EV << "Node configuration does not contain IPv6 default route configuration!" << endl;}
}

bool MultiNetworkNodeConfigurator::Str2Int(int* retValue, const char* str) {
    if (retValue == NULL || str == NULL){
       return false;
    }

    char *tail = NULL;
    long value = 0;
    errno = 0;

    value = strtol(str, &tail, 0);

    if (*tail != '\0' || errno == ERANGE || errno == EINVAL || value < INT_MIN || value > INT_MAX){
       return false;
    }

    *retValue = (int) value;
    return true;
}

void MultiNetworkNodeConfigurator::parseStaticRoutes(cXMLElement* config) {
}

bool MultiNetworkNodeConfigurator::Str2Bool(bool* ret, const char* str) {
    if (  (strcmp(str, "yes") == 0)
       || (strcmp(str, "enabled") == 0)
       || (strcmp(str, "on") == 0)
       || (strcmp(str, "true") == 0)){

       *ret = true;
       return true;
    }

    if (  (strcmp(str, "no") == 0)
       || (strcmp(str, "disabled") == 0)
       || (strcmp(str, "off") == 0)
       || (strcmp(str, "false") == 0)){

       *ret = false;
       return true;
    }

    int value;
    if (Str2Int(&value, str)){
       if (value == 1){
          *ret = true;
          return true;
       }

       if (value == 0){
          *ret = false;
          return true;
       }
    }

    return false;
}

} //namespace
