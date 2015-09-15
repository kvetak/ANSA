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

#ifndef __INET_LISPMAPCACHE_H_
#define __INET_LISPMAPCACHE_H_

#include <omnetpp.h>
#include "ansa/networklayer/lisp/LISPMapStorageBase.h"
#include "ansa/networklayer/lisp/LISPTimers_m.h"
#include "ansa/networklayer/lisp/LISPServerEntry.h"
#include "networklayer/contract/IInterfaceTable.h"
#include "networklayer/common/InterfaceTableAccess.h"
#include "ansa/networklayer/lisp/LISPMapDatabase.h"

typedef std::list <LISPMapEntryTimer*> CacheEntriesTimeouts;
typedef CacheEntriesTimeouts::const_iterator TimeoutCItem;
typedef CacheEntriesTimeouts::iterator TimeoutItem;

class LISPMapCache : public cSimpleModule, public LISPMapStorageBase
{
  public:
    enum EMapSync
    {
        SYNC_NONE,
        SYNC_NAIVE,
        SYNC_SMART
    };

    enum ESSAddrType
    {
        SSADDR_NONLISP,
        SSADDR_RLOC,
        SSADDR_EID
    };

    LISPMapCache();
    virtual ~LISPMapCache();

    const std::string& getSyncKey() const;
    const ServerAddresses& getSyncSet() const;
    LISPMapCache::EMapSync getSyncType() const;
    bool isInSyncSet(inet::L3Address ssmember) const;
    bool isSyncAck() const;

    LISPMapEntryTimer* findExpirationTimer(const LISPEidPrefix& eidPref);
    void updateTimeout(const LISPEidPrefix& eidPref, simtime_t time);

    LISPMapEntry* lookupMapEntry(inet::L3Address address);

    void updateCacheEntry(const TRecord& record);
    void syncCacheEntry(LISPMapEntry& entry);

    void notifySyncset(cComponent *src);

    void updateDisplayString();

    void restartMapCache();

  protected:
    inet::IInterfaceTable*    Ift;                ///< Provides access to the interface table.
    LISPMapDatabase*     MapDb;

    CacheEntriesTimeouts CacheTimeouts;

    EMapSync syncType;
    ESSAddrType ssAddrType;

    ServerAddresses SyncSet;
    std::string syncKey;
    bool syncAck;

    simsignal_t sigMiss;
    simsignal_t sigLookup;
    simsignal_t sigSize;

    simsignal_t sigPeerUp;

    void initPointers();
    void initSignals();

    void parseConfig(cXMLElement* config);
    void parseSyncSetup(cXMLElement* config);

    virtual int numInitStages() const { return 4; }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);

};

#endif
