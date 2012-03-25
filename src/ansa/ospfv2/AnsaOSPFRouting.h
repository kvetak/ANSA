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

#ifndef __INET_OSPFROUTING_H
#define __INET_OSPFROUTING_H

#include <vector>
#include <map>
#include <omnetpp.h>
#include "IRoutingTable.h"
#include "IInterfaceTable.h"
#include "NotificationBoard.h"
#include "AnsaOSPFInterface.h"
#include "OSPFPacket_m.h"
#include "AnsaOSPFRouter.h"

/**
 * OMNeT++ module class acting as a facade for the OSPF datastructure.
 * Handles the configuration loading and forwards the OMNeT++ messages(OSPF packets).
 */
class AnsaOSPFRouting :  public cSimpleModule, protected INotifiable
{
  private:
    IInterfaceTable*     ift;        ///< Provides access to the interface table.
    IRoutingTable*       rt;         ///< Provides access to the IP routing table.
    AnsaOSPF::Router*    ospfRouter; ///< Root object of the OSPF datastructure.
    bool                 ospfEnabled;
    
    cXMLElement* IntNode;  // interface configuration xml element 
    cXMLElement* OspfNode;  // ospf configuration xml element
    
    std::map<int, AnsaOSPF::AreaID> ifToAreaList;

    int     ResolveInterfaceName(const std::string& name) const;
    void    GetAreaListFromXML(const cXMLElement& routerNode, std::map<std::string, int>& areaList) const;
    void    LoadAreaFromXML(const cXMLElement& asConfig, const std::string& areaID);
    void    LoadInterfaceParameters(const cXMLElement& ifConfig);
    void    LoadExternalRoute(const cXMLElement& externalRouteConfig);
    void    LoadHostRoute(const cXMLElement& hostRouteConfig);
    void    LoadVirtualLink(const cXMLElement& virtualLinkConfig);

    bool    LoadConfigFromXML(const char * filename);
    
    void    AnsaLoadArea(const cXMLElement& areaConfig);
    void    AnsaLoadInterface(AnsaOSPF::Area &area, AnsaOSPF::IPv4AddressRange &addressRange);
    
    bool    AnsaLoadConfigFromXML(const char * filename);

  public:
    AnsaOSPFRouting();
    virtual ~AnsaOSPFRouting(void);

  protected:

    NotificationBoard *nb; // cached pointer
  
    virtual int numInitStages() const  {return 5;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

     /**
     * Called by the NotificationBoard whenever a change of a category
     * occurs to which this client has subscribed.
     */
    virtual void receiveChangeNotification(int category, const cPolymorphic *details);
};

#endif  // __INET_OSPFROUTING_H


