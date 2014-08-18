//
// Copyright (C) 2013 Brno University of Technology
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
//@author Vladimir Vesely (<a href="mailto:ivesely@fit.vutbr.cz">ivesely@fit.vutbr.cz</a>)

#ifndef __INET_LISPCORE_H_
#define __INET_LISPCORE_H_

#include <omnetpp.h>
#include "UDPSocket.h"
#include "LISPServerEntry.h"
#include "AnsaRoutingTable.h"
#include "AnsaRoutingTableAccess.h"
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "LISPMapCache.h"

typedef std::list< LISPServerEntry > ServerAddresses;

extern const char* MAPSERVER_TAG;
extern const char* MAPRESOLVER_TAG;
extern const char* MAPSERVERADDR_TAG;
extern const char* MAPRESOLVERADDR_TAG;

class LISPCore : public cSimpleModule, protected INotifiable
{
  public:
    LISPCore();
    virtual ~LISPCore();

    bool isMapResolverV4() const;
    void setMapResolverV4(bool mapResolverV4);
    bool isMapResolverV6() const;
    void setMapResolverV6(bool mapResolverV6);
    bool isMapServerV4() const;
    void setMapServerV4(bool mapServerV4);
    bool isMapServerV6() const;
    void setMapServerV6(bool mapServerV6);

  protected:
    const char  *deviceId;   ///< Id of the device which contains this routing process.
    std::string hostName;    ///< Device name from the network topology.

    IInterfaceTable*    Ift;                ///< Provides access to the interface table.
    AnsaRoutingTable*   Rt;                 ///< Provides access to the IPv4 routing table.
    LISPMapCache*       Lmc;

    ServerAddresses        MapServers;
    ServerAddresses        MapResolvers;

    bool mapServerV4;
    bool mapServerV6;
    bool mapResolverV4;
    bool mapResolverV6;

    bool isMapResolver() {return mapServerV4 || mapServerV6;}
    bool isMapServer()   {return mapResolverV4 || mapResolverV6;}

    UDPSocket socket;

    virtual int numInitStages() const { return 4; }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual void receiveChangeNotification(int category, const cObject *details);
    virtual void updateDisplayString();

    void parseConfig(cXMLElement* config);
};

#endif
