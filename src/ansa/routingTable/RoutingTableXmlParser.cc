//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


//  Cleanup and rewrite: Andras Varga, 2004


#include "RoutingTableXmlParser.h"
#include "IPv4InterfaceData.h"
#include <stdlib.h>
#include <omnetpp.h>
#include <string>



RoutingTableXmlParser::RoutingTableXmlParser(IInterfaceTable *i, IRoutingTable *r)
{
    ift = i;
    rt = r;
}


bool RoutingTableXmlParser::readRoutingTableFromXml(const char *filename, const char *RouterId)
{
    cXMLElement* routerConfig = ev.getXMLDocument(filename);
    if (routerConfig == NULL) {
        return false;
    }

    // load information on this router
    std::string routerXPath("Router[@id='");
    routerXPath += RouterId;
    routerXPath += "']";

    cXMLElement* routerNode = routerConfig->getElementByPath(routerXPath.c_str());
    if (routerNode == NULL)
        opp_error("No configuration for Router ID: %s", RouterId);

    cXMLElement* IntNode = routerNode->getFirstChildWithTag("Interfaces");
    if (IntNode)
        readInterfaceFromXml(IntNode);

    cXMLElement* routingNode = routerNode->getFirstChildWithTag("Routing");
    if (routingNode){
       cXMLElement* staticNode = routingNode->getFirstChildWithTag("Static");
       if (staticNode)
          readStaticRouteFromXml(staticNode);
    }
    return true;
}

void RoutingTableXmlParser::readInterfaceFromXml(cXMLElement* Node)
{
    InterfaceEntry* ie;

    cXMLElementList intConfig = Node->getChildren();
    for (cXMLElementList::iterator intConfigIt = intConfig.begin(); intConfigIt != intConfig.end(); intConfigIt++)
    {
      std::string nodeName = (*intConfigIt)->getTagName();
      if (nodeName == "Interface" && (*intConfigIt)->getAttribute("name"))
      {
        std::string intName=(*intConfigIt)->getAttribute("name");
        std::string typeName=intName.substr(0,3);
        
        ie=ift->getInterfaceByName(intName.c_str());
        
        if (!ie)
          opp_error("Error in routing file: interface name `%s' not registered by any L2 module", intName.c_str());
        
        //implicitne nastavenia
        if (typeName=="eth")
              ie->setBroadcast(true);
        if (typeName=="ppp")
              ie->setPointToPoint(true);
              
        IPv4InterfaceData::IPAddressVector mcg = ie->ipv4Data()->getMulticastGroups();
        
        //registracia do multicast groups
        mcg.push_back(IPAddress("224.0.0.1"));
        mcg.push_back(IPAddress("224.0.0.2"));
        ie->ipv4Data()->setMulticastGroups(mcg);
        
        ie->ipv4Data()->setMetric(1); 
        ie->setMtu(1500);
        
        cXMLElementList ifDetails = (*intConfigIt)->getChildren();  
        for (cXMLElementList::iterator ifElemIt = ifDetails.begin(); ifElemIt != ifDetails.end(); ifElemIt++) 
        {
          std::string nodeName = (*ifElemIt)->getTagName();
          
          if (nodeName=="IPAddress") 
          {
            ie->ipv4Data()->setIPAddress(IPAddress((*ifElemIt)->getNodeValue()));
          }
          
          if (nodeName=="Mask") 
          {
            ie->ipv4Data()->setNetmask(IPAddress((*ifElemIt)->getNodeValue()));
          }
          
          if (nodeName=="MTU") 
          {
            ie->setMtu(atoi((*ifElemIt)->getNodeValue()));
          }
            
        }
        
      }
    }    
}

void RoutingTableXmlParser::readStaticRouteFromXml(cXMLElement* Node)
{
  cXMLElementList intConfig = Node->getChildren();
  for (cXMLElementList::iterator intConfigIt = intConfig.begin(); intConfigIt != intConfig.end(); intConfigIt++)
  {
    std::string nodeName = (*intConfigIt)->getTagName();
    if (nodeName == "Route")
    {
        IPRoute *e = new IPRoute();
        cXMLElementList ifDetails = (*intConfigIt)->getChildren();  
        for (cXMLElementList::iterator ifElemIt = ifDetails.begin(); ifElemIt != ifDetails.end(); ifElemIt++) 
        {
          std::string nodeName = (*ifElemIt)->getTagName();
          
          if (nodeName=="NetworkAddress") 
          {
            e->setHost(IPAddress((*ifElemIt)->getNodeValue()));
          }
          
          if (nodeName=="NetworkMask") 
          {
            e->setNetmask(IPAddress((*ifElemIt)->getNodeValue()));
          }
          
          if (nodeName=="NextHopAddress") 
          {
            e->setGateway(IPAddress((*ifElemIt)->getNodeValue()));
            InterfaceEntry *intf=NULL;
            for (int i=0; i<ift->getNumInterfaces(); i++)
            {
              intf = ift->getInterface(i);
              if (((intf->ipv4Data()->getIPAddress()).doAnd(intf->ipv4Data()->getNetmask()))==((e->getGateway()).doAnd(intf->ipv4Data()->getNetmask())))
                  break;
               
            }
            if (intf)
              e->setInterface(intf);
            else
              opp_error("Error.");
            e->setMetric(1);
          }
          if (nodeName=="ExitInterface") 
          {
            InterfaceEntry *ie=ift->getInterfaceByName((*ifElemIt)->getNodeValue());
            if (!ie)
                opp_error("Interface does not exists");
            
            e->setInterface(ie);
            e->setGateway(IPAddress::UNSPECIFIED_ADDRESS);
            e->setMetric(0);
          }
          if (nodeName=="StaticRouteMetric") 
          {
            e->setMetric(atoi((*ifElemIt)->getNodeValue()));
          }  
        }
        rt->addRoute(e);
    }
    
  }
}
