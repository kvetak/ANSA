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

#include "networklayer/ipv6/IPv6Datagram.h"
#include "networklayer/ipv4/IPv4Datagram.h"
#include "networklayer/common/IPProtocolId_m.h"
#include "transportlayer/udp/UDP.h"
#include "transportlayer/udp/UDPPacket.h"
#include "transportlayer/contract/udp/UDPSocket.h"
#include "transportlayer/contract/udp/UDPControlInfo.h"

#include "ansa/networklayer/lisp/LISPServerEntry.h"
#include "ansa/networklayer/lisp/LISPMapCache.h"
#include "ansa/networklayer/lisp/LISPMapDatabase.h"
#include "ansa/networklayer/lisp/LISPSiteDatabase.h"
#include "ansa/networklayer/lisp/LISPMessages_m.h"
#include "ansa/networklayer/lisp/LISPTimers_m.h"
#include "ansa/networklayer/lisp/LISPCommon.h"
#include "ansa/networklayer/lisp/LISPMsgLogger.h"
#include "ansa/networklayer/lisp/LISPProbeSet.h"


class LISPCoreBase : public cSimpleModule
{
  public:
    virtual int numInitStages() const { return 5; }
    virtual void initialize(int stage) = 0;
    virtual void handleMessage(cMessage *msg) = 0;

    virtual void scheduleWholeCacheSync(inet::L3Address& ssmember) = 0;
};

class LISPDownLis : public cListener {
   public:
    LISPDownLis(LISPMapCache* mc) : MapCache(mc) {}
     virtual ~LISPDownLis() { MapCache = NULL; }

     virtual void receiveSignal(cComponent *src, simsignal_t id, bool b) {
         EV << "Signal IFACE DOWN/UP initiated by " << src->getFullPath() << endl;
         //Going DOWN or UP
         if (b)
             MapCache->restartMapCache();
         else
             MapCache->notifySyncset(src);
     }
   protected:
       LISPMapCache* MapCache;
};

//FIXME: Vesely - Remove this Listener and use timer-based version instead
class LISPUpLis : public cListener {
   public:
     LISPUpLis(LISPMapCache* mc, LISPCoreBase* core) : MapCache(mc), Core(core)  {}
     virtual ~LISPUpLis() { Core = NULL; }

     virtual void receiveSignal(cComponent *src, simsignal_t id, const char* c) {
         EV << "Signal SS-PEER-UP initiated by " << src->getFullPath() << " with addr " << c << endl;
         inet::L3Address addr = inet::L3Address(c);
         //EV << "||||||||||||||||" << addr << endl;
         if (MapCache->isInSyncSet(addr))
             Core->scheduleWholeCacheSync(addr);
         else
             EV << "Does not have SS member in syncset" << endl;
     }

   protected:
     LISPMapCache* MapCache;
     LISPCoreBase* Core;
};


class LISPCore : public LISPCoreBase
{
  public:
    enum EProbeAlgo
    {
        PROBE_CISCO,
        PROBE_SIMPLE,
        PROBE_SOPHIS
    };

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

    bool isOneOfMyEids(inet::L3Address addr);

  protected:
    LISPMapCache*       MapCache;
    LISPSiteDatabase*   SiteDb;
    LISPMapDatabase*    MapDb;
    LISPMsgLogger*      MsgLog;

    LISPProbeSet        ProbingSet;

    ServerAddresses     MapServers;
    ServerAddresses     MapResolvers;
    ServerCItem         MapResolverQueue;

    inet::UDPSocket controlTraf;
    inet::UDPSocket dataTraf;

    bool mapServerV4;
    bool mapServerV6;
    bool mapResolverV4;
    bool mapResolverV6;

    bool acceptMapRequestMapping;
    unsigned short mapCacheTtl;
    bool echoNonceAlgo;
    bool ciscoStartupDelays;

    EProbeAlgo rlocProbingAlgo;

    bool isMapResolver() {return mapServerV4 || mapServerV6;}
    bool isMapServer()   {return mapResolverV4 || mapResolverV6;}

    simsignal_t sigFrwd, sigDrop;
    unsigned int packetfrwd, packetdrop;
    virtual void updateDisplayString();
    void updateStats(bool flag);

    LISPDownLis* DownListener;
    LISPUpLis* UpListener;

    virtual int numInitStages() const { return 5; }
    void initPointers();
    void initSockets();
    void initSignals();
    virtual void initialize(int stage);

    void handleTimer(cMessage *msg);
    void handleCotrol(cMessage *msg);
    void handleDataEncaps(cMessage *msg);
    void handleDataDecaps(cMessage *msg);
    virtual void handleMessage(cMessage *msg);

    void parseConfig(cXMLElement* config);
        LISPServerEntry* findServerEntryByAddress(ServerAddresses& list, inet::L3Address& addr);

    void sendMapRegister(LISPServerEntry& se);
    void receiveMapRegister(LISPMapRegister* lmreg);
    void sendMapNotify(LISPMapRegister* lmreg);
    void receiveMapNotify(LISPMapNotify* lmnot);

    unsigned long sendEncapsulatedMapRequest(inet::L3Address& srcEid, inet::L3Address& dstEid);
    void receiveMapRequest(LISPMapRequest* lmreq);
    void delegateMapRequest(LISPMapRequest* lmreq, Etrs& etrs);

    void sendMapReplyFromMs(LISPMapRequest* lmreq, Etrs& etrs);
    void sendMapReplyFromEtr(LISPMapRequest* lmreq, const inet::L3Address& eidAddress);
    void sendNegativeMapReply(LISPMapRequest* lmreq, const LISPEidPrefix& eidPref, bool hasSite);
    void receiveMapReply(LISPMapReply* lmrep);

    unsigned long sendRlocProbe(inet::L3Address& dstLoc, LISPEidPrefix& eid);
    void receiveRlocProbe(LISPMapRequest* lmreq);
    void sendRlocProbeReply(LISPMapRequest* lmreq, long bitmask);
    void receiveRlocProbeReply(LISPMapReply* lmrep);

    TRecord prepareTRecord(LISPMapEntry* me, bool Abit, LISPCommon::EAct action);
    void expireRegisterTimer(LISPRegisterTimer* regtim);
    void expireRequestTimer(LISPRequestTimer* reqtim);
    void expireRlocProbeTimer(LISPRlocProbeTimer* probetim);
    void expireSyncTimer(LISPSyncTimer* synctim);

    unsigned long sendCacheSync(inet::L3Address& setmember, LISPEidPrefix& eidPref);
    void receiveCacheSync(LISPCacheSync* lcs);
    void sendCacheSyncAck(LISPCacheSync* lcs);
    void receiveCacheSyncAck(LISPCacheSyncAck* lcsa);

    void parseMapServerConfig(cXMLElement* config);
    void parseMapResolverConfig(cXMLElement* config);

    void scheduleRegistration();
    void scheduleRlocProbing();
    void scheduleCacheSync(const LISPEidPrefix& eidPref);
    void scheduleWholeCacheSync(inet::L3Address& ssmember);

    void cacheMapping(const TRecord& record);

    unsigned long sendEncapsulatedDataMessage(inet::L3Address srcaddr, inet::L3Address dstaddr, LISPMapEntry* mapentry, cPacket* packet);

    unsigned int getItrRlocSize(TAfiAddrs& Itrs);
    unsigned int getRecsSize(TRecs& Recs);
    unsigned int getRecordSize(TRecord& Record);
    unsigned int getRecordsSize(TRecords& Records);
    unsigned int getLispMapEntrySize(TMapEntries& MapEntries);
    int64 countPacketSize(LISPMessage* lispmsg);
};

#endif
