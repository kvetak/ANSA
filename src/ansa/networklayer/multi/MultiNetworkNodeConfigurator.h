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

#ifndef __ANSAINET_MULTINETWORKNODECONFIGURATOR_H_
#define __ANSAINET_MULTINETWORKNODECONFIGURATOR_H_

#include <omnetpp.h>

#include "inet/common/INETDefs.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"
#include "ansa/networklayer/common/ANSA_InterfaceEntry.h"

using namespace omnetpp;

namespace inet {

class INET_API MultiNetworkNodeConfigurator : public cSimpleModule, public ILifecycle
{

  protected:
    const char* PAR_CONFIG = "configData";
    const char* PAR_ENAIP4 = "enableIPv4";
    const char* PAR_ENAIP6 = "enableIPv6";
    const char* PAR_REPLACEADDR = "replaceIPv6Addresses";

    const char* XML_IFACES = "Interfaces";
    const char* XML_IFACE = "Interface";
    const char* XML_NAME = "name";
    const char* XML_DEFROUTER = "DefaultRouter";
    const char* XML_DEFROUTER6 = "DefaultRouter6";
    const char* XML_ROUTING = "Routing";
    const char* XML_ROUTING6 = "Routing6";
    const char* XML_STATIC = "Static";
    const char* XML_ROUTE = "Route";
    const char* XML_IPADDR = "IPAddress";
    const char* XML_IPADDR6 = "Ipv6Address";
    const char* XML_MASK = "Mask";
    const char* XML_MTU = "MTU";
    const char* XML_METRIC = "Metric";
    const char* XML_NETADDR = "NetworkAddress";
    const char* XML_NETMASK = "NetworkMask";
    const char* XML_NHADDR = "NextHopAddress";
    const char* XML_ADMINDIST = "AdministrativeDistance";


    NodeStatus *nodeStatus;
    IInterfaceTable *interfaceTable;
    IIpv4RoutingTable *rt4;
    Ipv6RoutingTable *rt6;


    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("this module doesn't handle messages, it runs only in initialize()"); }
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

    void parseConfig(cXMLElement *config);

    void parseInterfaces(cXMLElement *config);

    void parseDefaultRoutes(cXMLElement *config);
    void parseDefaultRoute4(cXMLElement* config);
    void parseDefaultRoute6(cXMLElement* config);

    void parseStaticRoutes(cXMLElement *config);
    void parseStaticRoutes4(cXMLElement* config);
    void parseStaticRoutes6(cXMLElement* config);

    InterfaceEntry* findInterfaceByAddress(Ipv4Address address);
    InterfaceEntry* findInterfaceByAddress(Ipv6Address address);

    Ipv4Route* prepareIPv4Route(Ipv4Address address, Ipv4Address netmask, Ipv4Address nexthop, InterfaceEntry* ie, int metric, unsigned int admindist, IRoute::SourceType srctype);
    Ipv6Route* prepareIPv6Route(Ipv6Address address, int prefixlength, Ipv6Address nexthop, InterfaceEntry* ie, int metric, unsigned int admindist, IRoute::SourceType srctype);

    static bool Str2Int(int *retValue, const char *str);
    static bool Str2Bool(bool *ret, const char *str);

private:
};

} //namespace

#endif
