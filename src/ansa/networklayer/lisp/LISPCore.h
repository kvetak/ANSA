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
#include "UDPControlInfo.h"

#include "AnsaRoutingTable.h"
#include "AnsaRoutingTableAccess.h"

#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"

#include "LISPServerEntry.h"
#include "LISPMapCache.h"
#include "LISPMapDatabase.h"
#include "LISPMessages_m.h"
#include "LISPCommon.h"


typedef std::list< LISPServerEntry > ServerAddresses;
typedef ServerAddresses::iterator ServerItem;

class LISPCore : public cSimpleModule
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
    IInterfaceTable*    Ift;                ///< Provides access to the interface table.
    AnsaRoutingTable*   Rt;                 ///< Provides access to the IPv4 routing table.
    LISPMapCache*       MapCache;
    LISPMapDatabase*    MapDb;

    LISPMapStorageBase  EtrMapping;

    ServerAddresses     MapServers;
    ServerAddresses     MapResolvers;

    UDPSocket controlTraf;

    bool mapServerV4;
    bool mapServerV6;
    bool mapResolverV4;
    bool mapResolverV6;

    bool isMapResolver() {return mapServerV4 || mapServerV6;}
    bool isMapServer()   {return mapResolverV4 || mapResolverV6;}

    virtual int numInitStages() const { return 4; }
    virtual void initialize(int stage);

    void handleTimer(cMessage *msg);
    void handleCotrol(cMessage *msg);
    void handleData(cMessage *msg);

    virtual void handleMessage(cMessage *msg);

    virtual void updateDisplayString();

    void parseConfig(cXMLElement* config);
    void initSockets();

    void registerSite();
    void sendMapRegister(LISPServerEntry& se);
    void receiveMapRegister(LISPMapRegister* lmr);


};

#endif
