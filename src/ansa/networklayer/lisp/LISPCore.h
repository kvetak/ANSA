//
// Copyright (C) 2013, 2014 Brno University of Technology
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
 * @author Vladimir Vesely / ivesely@fit.vutbr.cz / http://www.fit.vutbr.cz/~ivesely/
 */

#ifndef __INET_LISPCORE_H_
#define __INET_LISPCORE_H_

#include <omnetpp.h>

//#include "AnsaRoutingTable.h"
//#include "AnsaRoutingTableAccess.h"

#include "IPv6Datagram.h"
#include "IPv4Datagram.h"
#include "IPProtocolId_m.h"
#include "UDP.h"
#include "UDPPacket.h"
#include "UDPSocket.h"
#include "UDPControlInfo.h"

#include "LISPServerEntry.h"
#include "LISPMapCache.h"
#include "LISPMapDatabase.h"
#include "LISPSiteDatabase.h"
#include "LISPMessages_m.h"
#include "LISPTimers_m.h"
#include "LISPCommon.h"
#include "LISPMsgLogger.h"
#include "LISPProbeSet.h"

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

    bool isOneOfMyEids(IPvXAddress addr);

  protected:
    LISPMapCache*       MapCache;
    LISPSiteDatabase*   SiteDb;
    LISPMapDatabase*    MapDb;
    LISPMsgLogger*      MsgLog;

    LISPProbeSet        ProbingSet;

    ServerAddresses     MapServers;
    ServerAddresses     MapResolvers;
    ServerCItem         MapResolverQueue;

    UDPSocket controlTraf;
    UDPSocket dataTraf;

    bool mapServerV4;
    bool mapServerV6;
    bool mapResolverV4;
    bool mapResolverV6;

    bool acceptMapRequestMapping;
    unsigned short mapCacheTtl;
    bool echoNonceAlgo;
    bool ciscoStartupDelays;

    bool isMapResolver() {return mapServerV4 || mapServerV6;}
    bool isMapServer()   {return mapResolverV4 || mapResolverV6;}

    virtual int numInitStages() const { return 5; }
    void initPointers();
    void initSockets();
    virtual void initialize(int stage);

    void handleTimer(cMessage *msg);
    void handleCotrol(cMessage *msg);
    void handleDataEncaps(cMessage *msg);
    void handleDataDecaps(cMessage *msg);
    virtual void handleMessage(cMessage *msg);

    virtual void updateDisplayString();

    void parseConfig(cXMLElement* config);
        LISPServerEntry* findServerEntryByAddress(ServerAddresses& list, IPvXAddress& addr);

    void sendMapRegister(LISPServerEntry& se);
    void receiveMapRegister(LISPMapRegister* lmreg);
    void sendMapNotify(LISPMapRegister* lmreg);
    void receiveMapNotify(LISPMapNotify* lmnot);

    unsigned long sendEncapsulatedMapRequest(IPvXAddress& srcEid, IPvXAddress& dstEid);
    void receiveMapRequest(LISPMapRequest* lmreq);
    void delegateMapRequest(LISPMapRequest* lmreq, Etrs& etrs);

    void sendMapReplyFromMs(LISPMapRequest* lmreq, Etrs& etrs);
    void sendMapReplyFromEtr(LISPMapRequest* lmreq, const IPvXAddress& eidAddress);
    void sendNegativeMapReply(LISPMapRequest* lmreq, const LISPEidPrefix& eidPref, bool hasSite);
    void receiveMapReply(LISPMapReply* lmrep);

    unsigned long sendRlocProbe(IPvXAddress& dstLoc, LISPEidPrefix& eid);
    void receiveRlocProbe(LISPMapRequest* lmreq);
    void sendRlocProbeReply(LISPMapRequest* lmreq, long bitmask);
    void receiveRlocProbeReply(LISPMapReply* lmrep);

    TRecord prepareTRecord(LISPMapEntry* me, bool Abit, LISPCommon::EAct action);
    void expireRegisterTimer(LISPRegisterTimer* regtim);
    void expireRequestTimer(LISPRequestTimer* reqtim);
    void expireRlocProbeTimer(LISPRlocProbeTimer* probetim);
    void expireSyncTimer(LISPSyncTimer* synctim);

    unsigned long sendCacheSync(IPvXAddress& setmember, LISPEidPrefix& eidPref);
    void receiveCacheSync(LISPCacheSync* lcs);
    void sendCacheSyncAck(LISPCacheSync* lcs);
    void receiveCacheSyncAck(LISPCacheSyncAck* lcsa);

    void parseMapServerConfig(cXMLElement* config);
    void parseMapResolverConfig(cXMLElement* config);

    void scheduleRegistration();
    void scheduleRlocProbing();
    void scheduleCacheSync(const LISPEidPrefix& eidPref);

    void cacheMapping(const TRecord& record);

    unsigned long sendEncapsulatedDataMessage(IPvXAddress srcaddr, IPvXAddress dstaddr, LISPMapEntry* mapentry, cPacket* packet);
};

#endif
