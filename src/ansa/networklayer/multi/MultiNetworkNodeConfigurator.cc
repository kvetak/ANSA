//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
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
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */

#include "ansa/networklayer/multi/MultiNetworkNodeConfigurator.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"

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

        rt4 = ( par(PAR_ENAIP4) ? (dynamic_cast<IIpv4RoutingTable*>(node->getSubmodule("routingTable")->getSubmodule("ipv4"))) : nullptr);
        rt6 = ( par(PAR_ENAIP6) ? (dynamic_cast<Ipv6RoutingTable*>(node->getSubmodule("routingTable")->getSubmodule("ipv6"))) : nullptr);
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
                Ipv4Address ipv4addr = Ipv4Address((*ifElemIt)->getNodeValue());
                if (ipv4addr.isUnicast()) {
                    ie->ipv4Data()->setIPAddress(ipv4addr);
                }
                else {
                    EV_ERROR << "Something wrong with configured address." << endl;
                }
            }
            if (par(PAR_ENAIP4).boolValue() && nodeName == XML_MASK) {
                Ipv4Address ipv4mask = Ipv4Address((*ifElemIt)->getNodeValue());
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
                Ipv6Address ipv6;
                int prefixLen;

                // Check Ipv6 address validity and initialize prefixLen variable
                if (!ipv6.tryParseAddrWithPrefix(addrFull.c_str(), prefixLen)){
                    EV_ERROR << "Unable to set Ipv6 address." << endl;
                }

                //Initialize Ipv6 address and add it to interface
                ipv6 = Ipv6Address(addrFull.substr(0, addrFull.find_last_of('/')).c_str());
                ie->ipv6Data()->assignAddress(ipv6, false, 0, 0);

                //Do not add link-local addresses to routing table
                if (!ipv6.isLinkLocal() ) {
                    Ipv6Route* ipv6route = new Ipv6Route(ipv6.getPrefix(prefixLen), prefixLen, IRoute::IFACENETMASK);
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
    //Add default route for Ipv4
    parseDefaultRoute4(config);

    //Add default route for Ipv6
    parseDefaultRoute6(config);
}

void MultiNetworkNodeConfigurator::parseDefaultRoute4(cXMLElement* config) {
    //Add default route for Ipv4
    cXMLElement* defrou = config->getFirstChildWithTag(XML_DEFROUTER);
    if (defrou) {
        //Parse XML
        Ipv4Address ipv4addr = Ipv4Address(defrou->getNodeValue());
        //EV << "Def Route addrese is " << ipv4addr << endl;
        InterfaceEntry* ie = findInterfaceByAddress(ipv4addr);
        if (ie) {
            //EV << "!!!!!!!!!!" << ie->getName() << endl;
            //Prepare default route
            Ipv4Route* ipv4rou = prepareIPv4Route(
                    Ipv4Address::UNSPECIFIED_ADDRESS,
                    Ipv4Address::UNSPECIFIED_ADDRESS, ipv4addr, ie, 0,
                    Ipv4Route::dStatic, Ipv4Route::MANUAL);
            //Add default route as static
            rt4->addRoute(ipv4rou);
        } else {
            EV_ERROR
                            << "None of node's interfaces has the same Ipv4 subnet as default router address!"
                            << endl;
        }
    } else {
        EV
                  << "Node configuration does not contain Ipv4 default route configuration!"
                  << endl;
    }
}

void MultiNetworkNodeConfigurator::parseDefaultRoute6(cXMLElement* config) {
    //Add default route for Ipv6
    cXMLElement* defrou6 = config->getFirstChildWithTag(XML_DEFROUTER6);
    if (defrou6) {
        //Parse XML
        Ipv6Address ipv6addr = Ipv6Address(defrou6->getNodeValue());
        //EV << "Def Route addrese is " << ipv4addr << endl;
        InterfaceEntry* ie = findInterfaceByAddress(ipv6addr);
        if (ie) {
            //EV << "!!!!!!!!!!" << ie->getName() << endl;
            //Prepare default route
            Ipv6Route* ipv6rou = prepareIPv6Route(
                    Ipv6Address::UNSPECIFIED_ADDRESS, 0, ipv6addr, ie, 0,
                    Ipv6Route::dStatic, Ipv6Route::MANUAL);
            //Add default route as static
            rt6->addRoute(ipv6rou);
            //rt6->addDefaultRoute(ipv6addr,ie->getInterfaceId(),0);
        } else {
            EV_ERROR
                            << "None of node's interfaces has the same Ipv6 subnet as default router address!"
                            << endl;
        }
    } else {
        EV
                  << "Node configuration does not contain Ipv6 default route configuration!"
                  << endl;
    }
}

void MultiNetworkNodeConfigurator::parseStaticRoutes(cXMLElement* config) {
    //Parse Ipv4 static routes
    parseStaticRoutes4(config);
    //Parse Ipv6 static routes
    parseStaticRoutes6(config);
}

void MultiNetworkNodeConfigurator::parseStaticRoutes4(cXMLElement* config) {
    //Parse Ipv4 static routes
    std::stringstream exp;
    exp << XML_ROUTING << "/" << XML_STATIC;
    cXMLElement* stat = config->getElementByPath(exp.str().c_str());
    if (stat) {
        //EV << "!!!!!!!!!!!!" << stat->getTagName()<< "- "<< stat->hasChildren()<<  endl;
        for (auto rou : stat->getChildrenByTagName(XML_ROUTE)) {
            cXMLElement* xmladdr   = rou->getFirstChildWithTag(XML_NETADDR);
            cXMLElement* xmlmask   = rou->getFirstChildWithTag(XML_NETMASK);
            cXMLElement* xmlnhaddr = rou->getFirstChildWithTag(XML_NHADDR);
            cXMLElement* xmladist  = rou->getFirstChildWithTag(XML_ADMINDIST);

            Ipv4Address addr =
                    xmladdr ?
                            Ipv4Address(xmladdr->getNodeValue()) :
                            Ipv4Address::UNSPECIFIED_ADDRESS;
            Ipv4Address mask =
                    xmlmask ?
                            Ipv4Address(xmlmask->getNodeValue()) :
                            Ipv4Address::UNSPECIFIED_ADDRESS;
            Ipv4Address nhaddr =
                    xmlnhaddr ?
                            Ipv4Address(xmlnhaddr->getNodeValue()) :
                            Ipv4Address::UNSPECIFIED_ADDRESS;
            int adist = 1;
            Str2Int(&adist, xmladist ? xmladist->getNodeValue() : nullptr);

            InterfaceEntry* ie = findInterfaceByAddress(nhaddr);
            if (ie) {
                Ipv4Route* ipv4rou = prepareIPv4Route(addr,
                                                      mask,
                                                      nhaddr,
                                                      ie,
                                                      0,
                                                      adist,
                                                      Ipv4Route::MANUAL);
                rt4->addRoute(ipv4rou);
            } else {
                EV_ERROR << "None of node's interfaces has the same Ipv4 subnet as the next hop address!" << endl;
            }
        }
    } else {
        EV << "No Ipv4 static routing configuration!" << endl;
    }
}

void MultiNetworkNodeConfigurator::parseStaticRoutes6(cXMLElement* config) {
    //Parse Ipv4 static routes
    std::stringstream exp;
    exp << XML_ROUTING6 << "/" << XML_STATIC;
    cXMLElement* stat = config->getElementByPath(exp.str().c_str());
    if (stat) {
        for (auto rou : stat->getChildrenByTagName(XML_ROUTE)) {
            cXMLElement* xmladdr   = rou->getFirstChildWithTag(XML_NETADDR);
            cXMLElement* xmlnhaddr = rou->getFirstChildWithTag(XML_NHADDR);
            cXMLElement* xmladist  = rou->getFirstChildWithTag(XML_ADMINDIST);

            Ipv6Address addr =
                    xmladdr ?
                            Ipv6Address(xmladdr->getNodeValue()) :
                            Ipv6Address::UNSPECIFIED_ADDRESS;
            //FIXME: Vesely - Assumption that PLEN is 64 bits long
            int plen = 64;

            Ipv6Address nhaddr =
                    xmlnhaddr ?
                            Ipv6Address(xmlnhaddr->getNodeValue()) :
                            Ipv6Address::UNSPECIFIED_ADDRESS;

            int adist = 1;
            Str2Int(&adist, xmladist ? xmladist->getNodeValue() : nullptr);

            InterfaceEntry* ie = findInterfaceByAddress(nhaddr);
            if (ie) {
                Ipv6Route* ipv6rou = prepareIPv6Route(addr,
                                                      plen,
                                                      nhaddr,
                                                      ie,
                                                      0,
                                                      adist,
                                                      Ipv6Route::MANUAL);
                rt6->addRoute(ipv6rou);
            } else {
                EV_ERROR << "None of node's interfaces has the same Ipv4 subnet as the next hop address!" << endl;
            }
        }
    } else {
        EV << "No Ipv4 static routing configuration!" << endl;
    }

}

InterfaceEntry* MultiNetworkNodeConfigurator::findInterfaceByAddress(
        Ipv4Address address) {
    InterfaceEntry* ie = nullptr;
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
        InterfaceEntry* ietmp = interfaceTable->getInterface(i);
        if (ietmp->isLoopback()) continue;
        if (ietmp->ipv4Data()->getIPAddress().prefixMatches(address,ietmp->ipv4Data()->getNetmask().getNetmaskLength())) {
            ie = ietmp;
            break;
        }
    }
    return ie;
}

InterfaceEntry* MultiNetworkNodeConfigurator::findInterfaceByAddress(
        Ipv6Address address) {
    InterfaceEntry* ie = nullptr;
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
        InterfaceEntry* ietmp = interfaceTable->getInterface(i);
        if (ietmp->isLoopback()) continue;

        for (int j = 0; j < ietmp->ipv6Data()->getNumAddresses(); j++) {
            Ipv6Address ipv6tmp = ietmp->ipv6Data()->getAddress(j);
            //FIXME: Vesely - Ugly assumption that prefix length is 64 bits
            if (ipv6tmp.matches(address,64)) {
                ie = ietmp;
                break;
            }
        }
    }
    return ie;
}

Ipv4Route* MultiNetworkNodeConfigurator::prepareIPv4Route(Ipv4Address address,
        Ipv4Address netmask, Ipv4Address nexthop, InterfaceEntry* ie,
        int metric, unsigned int admindist, IRoute::SourceType srctype) {
    Ipv4Route* ipv4rou = new Ipv4Route();
    ipv4rou->setDestination(address);
    ipv4rou->setNetmask(netmask);
    ipv4rou->setGateway(nexthop);
    ipv4rou->setInterface(ie);
    ipv4rou->setMetric(metric);
    ipv4rou->setAdminDist(admindist);
    ipv4rou->setSourceType(srctype);
    return ipv4rou;
}

Ipv6Route* MultiNetworkNodeConfigurator::prepareIPv6Route(Ipv6Address address,
        int prefixlength, Ipv6Address nexthop, InterfaceEntry* ie, int metric,
        unsigned int admindist, IRoute::SourceType srctype) {
    Ipv6Route* ipv6rou = new Ipv6Route(address, prefixlength, srctype);
    ipv6rou->setNextHop(nexthop);
    ipv6rou->setInterface(ie);
    ipv6rou->setMetric(metric);
    ipv6rou->setAdminDist(admindist);
    return ipv6rou;
}
//TODO: Vesely - I am not sure that ANSA configs has this syntax for Ipv6
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
