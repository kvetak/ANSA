/*******************************************************************
*
*    This library is free software, you can redistribute it
*    and/or modify
*    it under  the terms of the GNU Lesser General Public License
*    as published by the Free Software Foundation;
*    either version 2 of the License, or any later version.
*    The library is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*    See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/

#include "AnsaOSPFRouting.h"
#include "IPAddress.h"
#include "IPAddressResolver.h"
#include "IPControlInfo.h"
#include "AnsaOSPFcommon.h"
#include "AnsaOSPFArea.h"
#include "AnsaMessageHandler.h"
#include "RoutingTableAccess.h"
#include "InterfaceTableAccess.h"
#include "IPv4InterfaceData.h"
#include <string>
#include <stdlib.h>
#include <memory.h>
//#include <iostream>


Define_Module(AnsaOSPFRouting);


AnsaOSPFRouting::AnsaOSPFRouting()
{
    ospfRouter = NULL;
    ospfEnabled = false;
}

/**
 * Destructor.
 * Deletes the whole OSPF datastructure.
 */
AnsaOSPFRouting::~AnsaOSPFRouting(void)
{
    if(ospfEnabled)
      delete ospfRouter;
}


/**
 * OMNeT++ init method.
 * Runs at stage 2 after interfaces are registered(stage 0) and the routing
 * table is initialized(stage 1). Loads OSPF configuration information from
 * the config XML.
 * @param stage [in] The initialization stage.
 */
void AnsaOSPFRouting::initialize(int stage)
{    if (stage==0)
    {
        // get a pointer to the NotificationBoard module
        nb = NotificationBoardAccess().get();

        // subscribe interface change notifications
        nb->subscribe(this, NF_INTERFACE_STATE_CHANGED);

    }
    else 
      // we have to wait for stage 2 until interfaces get registered(stage 0)
      // and routerId gets assigned(stage 3)
      if (stage == 4)
      {
          rt = RoutingTableAccess().get();
          ift = InterfaceTableAccess().get();
  
          // Get routerId
          ospfRouter = new AnsaOSPF::Router(rt->getRouterId().getInt(), this);
  
          // read the OSPF AS configuration
          const char *fileName = par("configFile");
          
          if(par("AnsaLoadingStyle"))
          {
            if (fileName == NULL || (!strcmp(fileName, "")) || !AnsaLoadConfigFromXML(fileName))
              error("Error reading AS configuration from file %s", fileName);
          }
          else
          {
            if (fileName == NULL || (!strcmp(fileName, "")) || !LoadConfigFromXML(fileName))
              error("Error reading AS configuration from file %s", fileName);
          }
          if(ospfEnabled)
            ospfRouter->AddWatches();
      }
}


/**
 * Forwards OSPF messages to the message handler object of the OSPF datastructure.
 * @param msg [in] The OSPF message.
 */
void AnsaOSPFRouting::handleMessage(cMessage *msg)
{
//    if (simulation.getEventNumber() == 90591) {
//        __asm int 3;
//    }

    if(ospfEnabled)
      ospfRouter->GetMessageHandler()->MessageReceived(msg);
    else
      delete msg;
}


void AnsaOSPFRouting::receiveChangeNotification(int category, const cPolymorphic *details)
{
    if (simulation.getContextType()==CTX_INITIALIZE)
        return;  // ignore notifications during initialize
    
    Enter_Method_Silent();
    printNotificationBanner(category, details);
    
    if (category==NF_INTERFACE_STATE_CHANGED) // change state of notified interface
    {        
        InterfaceEntry *entry = check_and_cast<InterfaceEntry*>(details);
        
        AnsaOSPF::Interface* intf = ospfRouter->GetNonVirtualInterface(entry->getInterfaceId());
        
        if(intf != NULL)
        {
          EV << "Changing state of interface in OSPF\n";
          if (entry->isDown())
          { 
            intf->ProcessEvent(AnsaOSPF::Interface::InterfaceDown);
          }
          else
          {
            intf->ProcessEvent(AnsaOSPF::Interface::InterfaceUp);
          }
        }
    }

}

void AnsaOSPFRouting::finish()
{
    recordScalar("helloPacketSend", ospfRouter->stat.GetHelloPacketSend());
    recordScalar("ospfPacketSend", ospfRouter->stat.GetOspfPacketSend());
    recordScalar("helloPacketReceived", ospfRouter->stat.GetHelloPacketReceived());
    recordScalar("ospfPacketReceived", ospfRouter->stat.GetOspfPacketReceived());
}

/**
 * Looks up the interface name in IInterfaceTable, and returns interfaceId a.k.a ifIndex.
 */
int AnsaOSPFRouting::ResolveInterfaceName(const std::string& name) const
{
    InterfaceEntry* ie = ift->getInterfaceByName(name.c_str());
    if (!ie)
        opp_error("error reading XML config: IInterfaceTable contains no interface named '%s'", name.c_str());
    return ie->getInterfaceId();
}

/**
 * Loads a list of OSPF Areas connected to this router from the config XML.
 * @param routerNode [in]  XML node describing this router.
 * @param areaList   [out] A hash of OSPF Areas connected to this router. The hash key is the Area ID.
 */
void AnsaOSPFRouting::GetAreaListFromXML(const cXMLElement& routerNode, std::map<std::string, int>& areaList) const
{
    cXMLElementList routerConfig = routerNode.getChildren();
    for (cXMLElementList::iterator routerConfigIt = routerConfig.begin(); routerConfigIt != routerConfig.end(); routerConfigIt++) {
        std::string nodeName = (*routerConfigIt)->getTagName();
        if ((nodeName == "PointToPointInterface") ||
            (nodeName == "BroadcastInterface") ||
            (nodeName == "NBMAInterface") ||
            (nodeName == "PointToMultiPointInterface"))
        {
            std::string areaId = (*routerConfigIt)->getChildrenByTagName("AreaID")[0]->getNodeValue();
            if (areaList.find(areaId) == areaList.end()) {
                areaList[areaId] = 1;
            }
        }
    }
}


/**
 * Loads basic configuration information for a given area from the config XML.
 * Reads the configured address ranges, and whether this Area should be handled as a stub Area.
 * @param asConfig [in] XML node describing the configuration of the whole Autonomous System.
 * @param areaID   [in] The Area to be added to the OSPF datastructure.
 */
void AnsaOSPFRouting::LoadAreaFromXML(const cXMLElement& asConfig, const std::string& areaID)
{
    std::string areaXPath("Area[@id='");
    areaXPath += areaID;
    areaXPath += "']";

    cXMLElement* areaConfig = asConfig.getElementByPath(areaXPath.c_str());
    if (areaConfig == NULL) {
        error("No configuration for Area ID: %s", areaID.c_str());
    }
    else {
        EV << "    loading info for Area id = " << areaID << "\n";
    }

    AnsaOSPF::Area* area = new AnsaOSPF::Area(ULongFromAddressString(areaID.c_str()));

    cXMLElementList areaDetails = areaConfig->getChildren();
    for (cXMLElementList::iterator arIt = areaDetails.begin(); arIt != areaDetails.end(); arIt++) {
        std::string nodeName = (*arIt)->getTagName();
        if (nodeName == "AddressRange") {
            AnsaOSPF::IPv4AddressRange addressRange;
            addressRange.address = IPv4AddressFromAddressString((*arIt)->getChildrenByTagName("Address")[0]->getNodeValue());
            addressRange.mask = IPv4AddressFromAddressString((*arIt)->getChildrenByTagName("Mask")[0]->getNodeValue());
            std::string status = (*arIt)->getChildrenByTagName("Status")[0]->getNodeValue();
            if (status == "Advertise") {
                area->AddAddressRange(addressRange, true);
            } else {
                area->AddAddressRange(addressRange, false);
            }
        }
        if ((nodeName == "Stub") && (areaID != "0.0.0.0")) {    // the backbone cannot be configured as a stub
            area->SetExternalRoutingCapability(false);
            area->SetStubDefaultCost(atoi((*arIt)->getChildrenByTagName("DefaultCost")[0]->getNodeValue()));
        }
    }
    // Add the Area to the router
    ospfRouter->AddArea(area);
}


/**
 * Loads OSPF configuration information for a router interface.
 * Handles PointToPoint, Broadcast, NBMA and PointToMultiPoint interfaces.
 * @param ifConfig [in] XML node describing the configuration of an OSPF interface.
 */
void AnsaOSPFRouting::LoadInterfaceParameters(const cXMLElement& ifConfig)
{
    AnsaOSPF::Interface* intf          = new AnsaOSPF::Interface;
    std::string      ifName        = ifConfig.getAttribute("ifName");
    int              ifIndex       = ResolveInterfaceName(ifName);
    std::string      interfaceType = ifConfig.getTagName();

    EV << "        loading " << interfaceType << " " << ifName << " ifIndex[" << ifIndex << "]\n";

    intf->SetIfIndex(ifIndex);
    if (interfaceType == "PointToPointInterface") {
        intf->SetType(AnsaOSPF::Interface::PointToPoint);
    } else if (interfaceType == "BroadcastInterface") {
        intf->SetType(AnsaOSPF::Interface::Broadcast);
    } else if (interfaceType == "NBMAInterface") {
        intf->SetType(AnsaOSPF::Interface::NBMA);
    } else if (interfaceType == "PointToMultiPointInterface") {
        intf->SetType(AnsaOSPF::Interface::PointToMultiPoint);
    } else {
        delete intf;
        error("Loading %s ifIndex[%d] aborted", interfaceType.c_str(), ifIndex);
    }

    AnsaOSPF::AreaID    areaID    = 0;
    cXMLElementList ifDetails = ifConfig.getChildren();

    for (cXMLElementList::iterator ifElemIt = ifDetails.begin(); ifElemIt != ifDetails.end(); ifElemIt++) {
        std::string nodeName = (*ifElemIt)->getTagName();
        if (nodeName == "AreaID") {
            areaID = ULongFromAddressString((*ifElemIt)->getNodeValue());
            intf->SetAreaID(areaID);
        }
        if (nodeName == "InterfaceOutputCost") {
            intf->SetOutputCost(atoi((*ifElemIt)->getNodeValue()));
        }
        if (nodeName == "RetransmissionInterval") {
            intf->SetRetransmissionInterval(atoi((*ifElemIt)->getNodeValue()));
        }
        if (nodeName == "InterfaceTransmissionDelay") {
            intf->SetTransmissionDelay(atoi((*ifElemIt)->getNodeValue()));
        }
        if (nodeName == "RouterPriority") {
            intf->SetRouterPriority(atoi((*ifElemIt)->getNodeValue()));
        }
        if (nodeName == "HelloInterval") {
            intf->SetHelloInterval(atoi((*ifElemIt)->getNodeValue()));
        }
        if (nodeName == "RouterDeadInterval") {
            intf->SetRouterDeadInterval(atoi((*ifElemIt)->getNodeValue()));
        }
        if (nodeName == "AuthenticationType") {
            std::string authenticationType = (*ifElemIt)->getNodeValue();
            if (authenticationType == "SimplePasswordType") {
                intf->SetAuthenticationType(AnsaOSPF::SimplePasswordType);
            } else if (authenticationType == "CrytographicType") {
                intf->SetAuthenticationType(AnsaOSPF::CrytographicType);
            } else {
                intf->SetAuthenticationType(AnsaOSPF::NullType);
            }
        }
        if (nodeName == "AuthenticationKey") {
            std::string key = (*ifElemIt)->getNodeValue();
            AnsaOSPF::AuthenticationKeyType keyValue;
            memset(keyValue.bytes, 0, 8 * sizeof(char));
            int keyLength = key.length();
            if ((keyLength > 4) && (keyLength <= 18) && (keyLength % 2 == 0) && (key[0] == '0') && (key[1] == 'x')) {
                for (int i = keyLength; (i > 2); i -= 2) {
                    keyValue.bytes[(i - 2) / 2] = HexPairToByte(key[i - 1], key[i]);
                }
            }
            intf->SetAuthenticationKey(keyValue);
        }
        if (nodeName == "PollInterval") {
            intf->SetPollInterval(atoi((*ifElemIt)->getNodeValue()));
        }
        if ((interfaceType == "NBMAInterface") && (nodeName == "NBMANeighborList")) {
            cXMLElementList neighborList = (*ifElemIt)->getChildren();
            for (cXMLElementList::iterator neighborIt = neighborList.begin(); neighborIt != neighborList.end(); neighborIt++) {
                std::string neighborNodeName = (*neighborIt)->getTagName();
                if (neighborNodeName == "NBMANeighbor") {
                    AnsaOSPF::Neighbor* neighbor = new AnsaOSPF::Neighbor;
                    neighbor->SetAddress(IPv4AddressFromAddressString((*neighborIt)->getChildrenByTagName("NetworkInterfaceAddress")[0]->getNodeValue()));
                    neighbor->SetPriority(atoi((*neighborIt)->getChildrenByTagName("NeighborPriority")[0]->getNodeValue()));
                    intf->AddNeighbor(neighbor);
                }
            }
        }
        if ((interfaceType == "PointToMultiPointInterface") && (nodeName == "PointToMultiPointNeighborList")) {
            cXMLElementList neighborList = (*ifElemIt)->getChildren();
            for (cXMLElementList::iterator neighborIt = neighborList.begin(); neighborIt != neighborList.end(); neighborIt++) {
                std::string neighborNodeName = (*neighborIt)->getTagName();
                if (neighborNodeName == "PointToMultiPointNeighbor") {
                    AnsaOSPF::Neighbor* neighbor = new AnsaOSPF::Neighbor;
                    neighbor->SetAddress(IPv4AddressFromAddressString((*neighborIt)->getNodeValue()));
                    intf->AddNeighbor(neighbor);
                }
            }
        }

    }
    // add the interface to it's Area
    AnsaOSPF::Area* area = ospfRouter->GetArea(areaID);
    if (area != NULL) {
        area->AddInterface(intf);
        intf->ProcessEvent(AnsaOSPF::Interface::InterfaceUp); // notification should come from the blackboard...
    } else {
        delete intf;
        error("Loading %s ifIndex[%d] in Area %d aborted", interfaceType.c_str(), ifIndex, areaID);
    }
}


/**
 * Loads the configuration information of a route outside of the Autonomous System(external route).
 * @param externalRouteConfig [in] XML node describing the parameters of an external route.
 */
void AnsaOSPFRouting::LoadExternalRoute(const cXMLElement& externalRouteConfig)
{
    std::string               ifName  = externalRouteConfig.getAttribute("ifName");
    int                       ifIndex = ResolveInterfaceName(ifName);
    OSPFASExternalLSAContents asExternalRoute;
    AnsaOSPF::RoutingTableEntry   externalRoutingEntry; // only used here to keep the path cost calculation in one place
    AnsaOSPF::IPv4AddressRange    networkAddress;

    EV << "        loading ExternalInterface " << ifName << " ifIndex[" << ifIndex << "]\n";

    cXMLElementList ifDetails = externalRouteConfig.getChildren();
    for (cXMLElementList::iterator exElemIt = ifDetails.begin(); exElemIt != ifDetails.end(); exElemIt++) {
        std::string nodeName = (*exElemIt)->getTagName();
        if (nodeName == "AdvertisedExternalNetwork") {
            networkAddress.address = IPv4AddressFromAddressString((*exElemIt)->getChildrenByTagName("Address")[0]->getNodeValue());
            networkAddress.mask    = IPv4AddressFromAddressString((*exElemIt)->getChildrenByTagName("Mask")[0]->getNodeValue());
            asExternalRoute.setNetworkMask(ULongFromIPv4Address(networkAddress.mask));
        }
        if (nodeName == "ExternalInterfaceOutputParameters") {
            std::string metricType = (*exElemIt)->getChildrenByTagName("ExternalInterfaceOutputType")[0]->getNodeValue();
            int         routeCost  = atoi((*exElemIt)->getChildrenByTagName("ExternalInterfaceOutputCost")[0]->getNodeValue());

            asExternalRoute.setRouteCost(routeCost);
            if (metricType == "Type2") {
                asExternalRoute.setE_ExternalMetricType(true);
                externalRoutingEntry.SetType2Cost(routeCost);
                externalRoutingEntry.SetPathType(AnsaOSPF::RoutingTableEntry::Type2External);
            } else {
                asExternalRoute.setE_ExternalMetricType(false);
                externalRoutingEntry.SetCost(routeCost);
                externalRoutingEntry.SetPathType(AnsaOSPF::RoutingTableEntry::Type1External);
            }
        }
        if (nodeName == "ForwardingAddress") {
            asExternalRoute.setForwardingAddress(ULongFromAddressString((*exElemIt)->getNodeValue()));
        }
        if (nodeName == "ExternalRouteTag") {
            std::string externalRouteTag = (*exElemIt)->getNodeValue();
            char        externalRouteTagValue[4];

            memset(externalRouteTagValue, 0, 4 * sizeof(char));
            int externalRouteTagLength = externalRouteTag.length();
            if ((externalRouteTagLength > 4) && (externalRouteTagLength <= 10) && (externalRouteTagLength % 2 == 0) && (externalRouteTag[0] == '0') && (externalRouteTag[1] == 'x')) {
                for (int i = externalRouteTagLength; (i > 2); i -= 2) {
                    externalRouteTagValue[(i - 2) / 2] = HexPairToByte(externalRouteTag[i - 1], externalRouteTag[i]);
                }
            }
            asExternalRoute.setExternalRouteTag((externalRouteTagValue[0] << 24) + (externalRouteTagValue[1] << 16) + (externalRouteTagValue[2] << 8) + externalRouteTagValue[3]);
        }
    }
    // add the external route to the OSPF datastructure
    ospfRouter->UpdateExternalRoute(networkAddress.address, asExternalRoute, ifIndex);
}


/**
 * Loads the configuration of a host getRoute(a host directly connected to the router).
 * @param hostRouteConfig [in] XML node describing the parameters of a host route.
 */
void AnsaOSPFRouting::LoadHostRoute(const cXMLElement& hostRouteConfig)
{
    AnsaOSPF::HostRouteParameters hostParameters;
    AnsaOSPF::AreaID              hostArea;

    std::string ifName = hostRouteConfig.getAttribute("ifName");
    hostParameters.ifIndex = ResolveInterfaceName(ifName);

    EV << "        loading HostInterface " << ifName << " ifIndex[" << static_cast<short> (hostParameters.ifIndex) << "]\n";

    cXMLElementList ifDetails = hostRouteConfig.getChildren();
    for (cXMLElementList::iterator hostElemIt = ifDetails.begin(); hostElemIt != ifDetails.end(); hostElemIt++) {
        std::string nodeName = (*hostElemIt)->getTagName();
        if (nodeName == "AreaID") {
            hostArea = ULongFromAddressString((*hostElemIt)->getNodeValue());
        }
        if (nodeName == "AttachedHost") {
            hostParameters.address = IPv4AddressFromAddressString((*hostElemIt)->getNodeValue());
        }
        if (nodeName == "LinkCost") {
            hostParameters.linkCost = atoi((*hostElemIt)->getNodeValue());
        }
    }
    // add the host route to the OSPF datastructure.
    AnsaOSPF::Area* area = ospfRouter->GetArea(hostArea);
    if (area != NULL) {
        area->AddHostRoute(hostParameters);

    } else {
        error("Loading HostInterface ifIndex[%d] in Area %d aborted", hostParameters.ifIndex, hostArea);
    }
}


/**
 * Loads the configuration of an OSPf virtual link(virtual connection between two backbone routers).
 * @param virtualLinkConfig [in] XML node describing the parameters of a virtual link.
 */
void AnsaOSPFRouting::LoadVirtualLink(const cXMLElement& virtualLinkConfig)
{
    AnsaOSPF::Interface* intf     = new AnsaOSPF::Interface;
    std::string      endPoint = virtualLinkConfig.getAttribute("endPointRouterID");
    AnsaOSPF::Neighbor*  neighbor = new AnsaOSPF::Neighbor;

    EV << "        loading VirtualLink to " << endPoint << "\n";

    intf->SetType(AnsaOSPF::Interface::Virtual);
    neighbor->SetNeighborID(ULongFromAddressString(endPoint.c_str()));
    intf->AddNeighbor(neighbor);

    cXMLElementList ifDetails = virtualLinkConfig.getChildren();
    for (cXMLElementList::iterator ifElemIt = ifDetails.begin(); ifElemIt != ifDetails.end(); ifElemIt++) {
        std::string nodeName = (*ifElemIt)->getTagName();
        if (nodeName == "TransitAreaID") {
            intf->SetTransitAreaID(ULongFromAddressString((*ifElemIt)->getNodeValue()));
        }
        if (nodeName == "RetransmissionInterval") {
            intf->SetRetransmissionInterval(atoi((*ifElemIt)->getNodeValue()));
        }
        if (nodeName == "InterfaceTransmissionDelay") {
            intf->SetTransmissionDelay(atoi((*ifElemIt)->getNodeValue()));
        }
        if (nodeName == "HelloInterval") {
            intf->SetHelloInterval(atoi((*ifElemIt)->getNodeValue()));
        }
        if (nodeName == "RouterDeadInterval") {
            intf->SetRouterDeadInterval(atoi((*ifElemIt)->getNodeValue()));
        }
        if (nodeName == "AuthenticationType") {
            std::string authenticationType = (*ifElemIt)->getNodeValue();
            if (authenticationType == "SimplePasswordType") {
                intf->SetAuthenticationType(AnsaOSPF::SimplePasswordType);
            } else if (authenticationType == "CrytographicType") {
                intf->SetAuthenticationType(AnsaOSPF::CrytographicType);
            } else {
                intf->SetAuthenticationType(AnsaOSPF::NullType);
            }
        }
        if (nodeName == "AuthenticationKey") {
            std::string key = (*ifElemIt)->getNodeValue();
            AnsaOSPF::AuthenticationKeyType keyValue;
            memset(keyValue.bytes, 0, 8 * sizeof(char));
            int keyLength = key.length();
            if ((keyLength > 4) && (keyLength <= 18) && (keyLength % 2 == 0) && (key[0] == '0') && (key[1] == 'x')) {
                for (int i = keyLength; (i > 2); i -= 2) {
                    keyValue.bytes[(i - 2) / 2] = HexPairToByte(key[i - 1], key[i]);
                }
            }
            intf->SetAuthenticationKey(keyValue);
        }
    }

    // add the virtual link to the OSPF datastructure.
    AnsaOSPF::Area* transitArea = ospfRouter->GetArea(intf->GetAreaID());
    AnsaOSPF::Area* backbone    = ospfRouter->GetArea(AnsaOSPF::BackboneAreaID);

    if ((backbone != NULL) && (transitArea != NULL) && (transitArea->GetExternalRoutingCapability())) {
        backbone->AddInterface(intf);
    } else {
        delete intf;
        error("Loading VirtualLink to %s through Area %d aborted", endPoint.c_str(), intf->GetAreaID());
    }
}


/**
 * Loads the configuration of the OSPF datastructure from the config XML.
 * @param filename [in] The path of the XML config file.
 * @return True if the configuration was succesfully loaded.
 * @throws an getError() otherwise.
 */
bool AnsaOSPFRouting::LoadConfigFromXML(const char * filename)
{
    cXMLElement* asConfig = ev.getXMLDocument(filename);
    if (asConfig == NULL) {
        error("Cannot read AS configuration from file: %s", filename);
    }

    // load information on this router
    std::string routerXPath("Router[@id='");
    IPAddress routerId(ospfRouter->GetRouterID());
    routerXPath += routerId.str();
    routerXPath += "']";

    cXMLElement* routerNode = asConfig->getElementByPath(routerXPath.c_str());
    if (routerNode == NULL) {
        error("No configuration for Router ID: %s", routerId.str().c_str());
    }
    else {
        EV << "OSPFRouting: Loading info for Router id = " << routerId.str() << "\n";
    }

    if (routerNode->getChildrenByTagName("RFC1583Compatible").size() > 0) {
        ospfRouter->SetRFC1583Compatibility(true);
    }

    std::map<std::string, int> areaList;
    GetAreaListFromXML(*routerNode, areaList);

    // load area information
    for (std::map<std::string, int>::iterator areaIt = areaList.begin(); areaIt != areaList.end(); areaIt++) {
        LoadAreaFromXML(*asConfig, areaIt->first);
    }
    // if the router is an area border router then it MUST be part of the backbone(area 0)
    if ((areaList.size() > 1) && (areaList.find("0.0.0.0") == areaList.end())) {
        LoadAreaFromXML(*asConfig, "0.0.0.0");
    }

    // load interface information
    cXMLElementList routerConfig = routerNode->getChildren();
    for (cXMLElementList::iterator routerConfigIt = routerConfig.begin(); routerConfigIt != routerConfig.end(); routerConfigIt++) {
        std::string nodeName = (*routerConfigIt)->getTagName();
        if ((nodeName == "PointToPointInterface") ||
            (nodeName == "BroadcastInterface") ||
            (nodeName == "NBMAInterface") ||
            (nodeName == "PointToMultiPointInterface"))
        {
            LoadInterfaceParameters(*(*routerConfigIt));
        }
        if (nodeName == "ExternalInterface") {
            LoadExternalRoute(*(*routerConfigIt));
        }
        if (nodeName == "HostInterface") {
            LoadHostRoute(*(*routerConfigIt));
        }
        if (nodeName == "VirtualLink") {
            LoadVirtualLink(*(*routerConfigIt));
        }
    }
    return true;
}

/**
 * Loads OSPF configuration information for a router interface.
 * Handles PointToPoint, Broadcast, NBMA and PointToMultiPoint interfaces.
 * @param ifConfig [in] XML node describing the configuration of an OSPF interface.
 */
void AnsaOSPFRouting::AnsaLoadInterface(AnsaOSPF::Area &area, AnsaOSPF::IPv4AddressRange &addressRange)
{
        
  cXMLElementList intConfig = IntNode->getChildren();
  for (cXMLElementList::iterator intConfigIt = intConfig.begin(); intConfigIt != intConfig.end(); intConfigIt++)
  {
    std::string nodeName = (*intConfigIt)->getTagName();
    if (nodeName == "Interface" && (*intConfigIt)->getAttribute("name"))
    {
      AnsaOSPF::IPv4Address ifAddress = IPv4AddressFromAddressString((*intConfigIt)->getFirstChildWithTag("IPAddress")->getNodeValue());
      if((ifAddress != AnsaOSPF::NullIPv4Address) && ((ifAddress & addressRange.mask) == (addressRange.address & addressRange.mask)))
      {
        AnsaOSPF::IPv4AddressRange  ifAddressRange;
        std::string                 ifName        = (*intConfigIt)->getAttribute("name");
        int                         ifIndex       = ResolveInterfaceName(ifName);
        
        std::map<int, AnsaOSPF::AreaID>::iterator ifIt = ifToAreaList.find(ifIndex);
        if (ifIt != ifToAreaList.end())
        {
          if(ifIt->second == area.GetAreaID())
            return;
          else
            error("OSPFRouting: Loading interface %s ifIndex[%d] aborted - Can not be in multiple areas!!!", ifName.c_str(), ifIndex);
        }
        
        InterfaceEntry* ie = ift->getInterfaceById(ifIndex);
        
        AnsaOSPF::Interface*  intf = new AnsaOSPF::Interface;
        intf->SetIfIndex(ifIndex);
        intf->SetAreaID(area.GetAreaID());
        
        if(ie->isPointToPoint())
        {
          intf->SetType(AnsaOSPF::Interface::PointToPoint);
          intf->SetRouterPriority(0);
        } else if(ie->isBroadcast())
            {
              intf->SetType(AnsaOSPF::Interface::Broadcast);
            }

        double datarate = ie->getDatarate();
        if (datarate > 0.0)
            intf->SetOutputCost((int)(100000000.0/datarate));

        EV << "        loading " << ifName << " ifIndex[" << ifIndex << "]\n";
        
        cXMLElementList ifDetails = (*intConfigIt)->getChildren();
        for (cXMLElementList::iterator ifElemIt = ifDetails.begin(); ifElemIt != ifDetails.end(); ifElemIt++) 
        {
            std::string nodeName = (*ifElemIt)->getTagName();
            if (nodeName == "Mask") {
                ifAddressRange.mask = IPv4AddressFromAddressString((*ifElemIt)->getNodeValue());
                ifAddressRange.address = ifAddress & ifAddressRange.mask;
            }
            if (nodeName == "OspfNetworkType") {
                std::string interfaceType = (*ifElemIt)->getNodeValue();
                if (interfaceType == "point-to-point") {
                    intf->SetType(AnsaOSPF::Interface::PointToPoint);
                    intf->SetRouterPriority(0);
                } else if (interfaceType == "broadcast") {
                    intf->SetType(AnsaOSPF::Interface::Broadcast);
                } else if (interfaceType == "non-broadcast") {
                    intf->SetType(AnsaOSPF::Interface::NBMA);
                } else if (interfaceType == "point-to-multipoint") {
                    intf->SetType(AnsaOSPF::Interface::PointToMultiPoint);
                } else {
                    delete intf;
                    error("Loading %s ifIndex[%d] aborted", interfaceType.c_str(), ifIndex);
                }
            }
            if (nodeName == "OspfCost") {
                intf->SetOutputCost(atoi((*ifElemIt)->getNodeValue()));
            }
            if (nodeName == "OspfRetransmissionInterval") {
                intf->SetRetransmissionInterval(atoi((*ifElemIt)->getNodeValue()));
            }
            if (nodeName == "OspfInterfaceTransmissionDelay") {
                intf->SetTransmissionDelay(atoi((*ifElemIt)->getNodeValue()));
            }
            if (nodeName == "OspfPriority") {
                intf->SetRouterPriority(atoi((*ifElemIt)->getNodeValue()));
            }
            if (nodeName == "OspfHelloInterval") {
                intf->SetHelloInterval(atoi((*ifElemIt)->getNodeValue()));
            }
            if (nodeName == "OspfDeadInterval") {
                intf->SetRouterDeadInterval(atoi((*ifElemIt)->getNodeValue()));
            }
            if (nodeName == "OspfAuthenticationType") {
                std::string authenticationType = (*ifElemIt)->getNodeValue();
                if (authenticationType == "ClearText") {
                    intf->SetAuthenticationType(AnsaOSPF::SimplePasswordType);
                } else if (authenticationType == "MessageDigest") {
                    intf->SetAuthenticationType(AnsaOSPF::CrytographicType);
                } else {
                    intf->SetAuthenticationType(AnsaOSPF::NullType);
                }
            }
            if (nodeName == "OspfAuthenticationKey") {
                std::string key = (*ifElemIt)->getNodeValue();
                AnsaOSPF::AuthenticationKeyType keyValue;
                memset(keyValue.bytes, 0, 8 * sizeof(char));
                int keyLength = key.length();
                if ((keyLength > 4) && (keyLength <= 18) && (keyLength % 2 == 0) && (key[0] == '0') && (key[1] == 'x')) {
                    for (int i = keyLength; (i > 2); i -= 2) {
                        keyValue.bytes[(i - 2) / 2] = HexPairToByte(key[i - 1], key[i]);
                    }
                }
                intf->SetAuthenticationKey(keyValue);
            }
            if (nodeName == "OspfPollInterval") {
                intf->SetPollInterval(atoi((*ifElemIt)->getNodeValue()));
            }
        }
        
        area.AddAddressRange(ifAddressRange, true);
        area.AddInterface(intf);
        ifToAreaList[ifIndex] = area.GetAreaID();
        
        IPv4InterfaceData::IPAddressVector mcg = ie->ipv4Data()->getMulticastGroups();

        mcg.push_back(IPAddress("224.0.0.5"));
        if(intf->GetType() == AnsaOSPF::Interface::Broadcast)
            mcg.push_back(IPAddress("224.0.0.6"));
        ie->ipv4Data()->setMulticastGroups(mcg);
      }
    }     
  }
}

/**
 * Loads basic configuration information for a given area from the config XML.
 * Reads the configured address ranges, and whether this Area should be handled as a stub Area.
 * @param areaConfig [in] XML node describing the configuration of the whole Autonomous System.
 */
void AnsaOSPFRouting::AnsaLoadArea(const cXMLElement& areaConfig)
{
    std::string idString = areaConfig.getAttribute("id");
    AnsaOSPF::AreaID    areaID = ULongFromAddressString(idString.c_str());
    
    if (areaID == AnsaOSPF::BackboneAreaID && idString != "0.0.0.0")
      error("Wrong format of  area ID: %s", idString.c_str());
    else
      EV << "    loading info for Area id = " << idString << "\n";

    AnsaOSPF::Area* area = new AnsaOSPF::Area(areaID);
    
    
    // load interfaces into area
    cXMLElementList networkConfig = areaConfig.getFirstChildWithTag("Networks")->getChildren();
    for (cXMLElementList::iterator networkConfigIt = networkConfig.begin(); networkConfigIt != networkConfig.end(); networkConfigIt++)
    {
      std::string nodeName = (*networkConfigIt)->getTagName();
      if (nodeName == "Network")
      {
        AnsaOSPF::IPv4AddressRange addressRange;
        addressRange.address = IPv4AddressFromAddressString((*networkConfigIt)->getFirstChildWithTag("IPAddress")->getNodeValue());
        addressRange.mask = ~IPv4AddressFromAddressString((*networkConfigIt)->getFirstChildWithTag("Wildcard")->getNodeValue());
        
        AnsaLoadInterface(*area, addressRange);
      }
    }
    
    if (areaConfig.getChildrenByTagName("Stub").size() > 0 && (areaID != AnsaOSPF::BackboneAreaID)) {    // the backbone cannot be configured as a stub
      area->SetExternalRoutingCapability(false);
      cXMLElement* defaultCost = areaConfig.getFirstChildWithTag("DefaultCost");
      if(defaultCost != NULL)
        area->SetStubDefaultCost(atoi(defaultCost->getNodeValue()));
      else
        area->SetStubDefaultCost(1);
    }
    
    ospfRouter->AddArea(area);

/*
    cXMLElementList areaNetworks = areaConfig->getChildren();
    for (cXMLElementList::iterator arIt = areaDetails.begin(); arIt != areaDetails.end(); arIt++) {
        std::string nodeName = (*arIt)->getTagName();
        if (nodeName == "AddressRange") {
            AnsaOSPF::IPv4AddressRange addressRange;
            addressRange.address = IPv4AddressFromAddressString((*arIt)->getChildrenByTagName("Address")[0]->getNodeValue());
            addressRange.mask = IPv4AddressFromAddressString((*arIt)->getChildrenByTagName("Mask")[0]->getNodeValue());
            std::string status = (*arIt)->getChildrenByTagName("Status")[0]->getNodeValue();
            if (status == "Advertise") {
                area->AddAddressRange(addressRange, true);
            } else {
                area->AddAddressRange(addressRange, false);
            }
        }
        if ((nodeName == "Stub") && (areaID != "0.0.0.0")) {    // the backbone cannot be configured as a stub
            area->SetExternalRoutingCapability(false);
            area->SetStubDefaultCost(atoi((*arIt)->getChildrenByTagName("DefaultCost")[0]->getNodeValue()));
        }
    }
    // Add the Area to the router
    ospfRouter->AddArea(area);
*/
}


/**
 * Loads the configuration of the OSPF datastructure from the config XML.
 * @param filename [in] The path of the XML config file.
 * @return True if the configuration was succesfully loaded.
 * @throws an getError() otherwise.
 */
bool AnsaOSPFRouting::AnsaLoadConfigFromXML(const char * filename)
{
    cXMLElement* asConfig = ev.getXMLDocument(filename);
    if (asConfig == NULL) {
        error("Cannot read OSPF configuration from file: %s", filename);
    }

    // load information on this router
    std::string routerXPath("Router[@id='");
    IPAddress routerId(ospfRouter->GetRouterID());
    routerXPath += routerId.str();
    routerXPath += "']";

    cXMLElement* routerNode = asConfig->getElementByPath(routerXPath.c_str());
    if (routerNode == NULL)
        error("No configuration for Router ID: %s", routerId.str().c_str());

    cXMLElement* routingNode = routerNode->getFirstChildWithTag("Routing");

    if (routingNode != NULL){
        IntNode = routerNode->getFirstChildWithTag("Interfaces");
        OspfNode = routingNode->getFirstChildWithTag("Ospf");
    }
    
    if (routingNode == NULL || OspfNode == NULL || IntNode == NULL) {
        EV << "OSPFRouting: OSPF is not enabled on Router id = " << routerId.str() << "\n";
        return true;
    }
    else {
        ospfEnabled = true;
        EV << "OSPFRouting: Loading info for Router id = " << routerId.str() << "\n";
    }
    
    if (OspfNode->getChildrenByTagName("RFC1583Compatible").size() > 0) {
        ospfRouter->SetRFC1583Compatibility(true);
    }
    
    // load areas information
    cXMLElementList areasConfig = OspfNode->getFirstChildWithTag("Areas")->getChildren();
    for (cXMLElementList::iterator areaConfigIt = areasConfig.begin(); areaConfigIt != areasConfig.end(); areaConfigIt++)
    {
      std::string nodeName = (*areaConfigIt)->getTagName();
      if (nodeName == "Area" && (*areaConfigIt)->getAttribute("id"))
        AnsaLoadArea(*(*areaConfigIt));
    }
    
    // starts OSPF on interfaces
    for (std::map<int, AnsaOSPF::AreaID>::iterator ifMapIt = ifToAreaList.begin(); ifMapIt != ifToAreaList.end(); ifMapIt++) {
        ospfRouter->GetArea(ifMapIt->second)->GetInterface(ifMapIt->first)->ProcessEvent(AnsaOSPF::Interface::InterfaceUp);;
    }


/*

    std::map<std::string, int> areaList;
    GetAreaListFromXML(*routerNode, areaList);

    // load area information
    for (std::map<std::string, int>::iterator areaIt = areaList.begin(); areaIt != areaList.end(); areaIt++) {
        LoadAreaFromXML(*asConfig, areaIt->first);
    }
    // if the router is an area border router then it MUST be part of the backbone(area 0)
    if ((areaList.size() > 1) && (areaList.find("0.0.0.0") == areaList.end())) {
        LoadAreaFromXML(*asConfig, "0.0.0.0");
    }

    // load interface information
    cXMLElementList routerConfig = routerNode->getChildren();
    for (cXMLElementList::iterator routerConfigIt = routerConfig.begin(); routerConfigIt != routerConfig.end(); routerConfigIt++) {
        std::string nodeName = (*routerConfigIt)->getTagName();
        if ((nodeName == "PointToPointInterface") ||
            (nodeName == "BroadcastInterface") ||
            (nodeName == "NBMAInterface") ||
            (nodeName == "PointToMultiPointInterface"))
        {
            LoadInterfaceParameters(*(*routerConfigIt));
        }
        if (nodeName == "ExternalInterface") {
            LoadExternalRoute(*(*routerConfigIt));
        }
        if (nodeName == "HostInterface") {
            LoadHostRoute(*(*routerConfigIt));
        }
        if (nodeName == "VirtualLink") {
            LoadVirtualLink(*(*routerConfigIt));
        }
    }
    */
    return true;
}
