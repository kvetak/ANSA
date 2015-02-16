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

#include "LISPMapCache.h"
//Forward declaration
#include "IPv4InterfaceData.h"
#include "IPv6InterfaceData.h"

Define_Module(LISPMapCache);

LISPMapCache::LISPMapCache(){}

LISPMapCache::~LISPMapCache()
{
    this->clearMappingStorage();
}

void LISPMapCache::parseConfig(cXMLElement* config) {
    if ( opp_strcmp(config->getTagName(), MAPCACHE_TAG) )
        config = config->getFirstChildWithTag(MAPCACHE_TAG);

    this->parseMapEntry(config);

    if (!opp_strcmp(par(CACHESYNC_PAR), SYNCNAIVE_PARVAL))
        syncType = SYNC_NAIVE;
    else if (!opp_strcmp(par(CACHESYNC_PAR), SYNCSMART_PARVAL))
        syncType = SYNC_SMART;
    else
        syncType = SYNC_NONE;

    if (!opp_strcmp(par(SSADDR_PAR), SSADDR_RLOC_PARVAL))
        ssAddrType = SSADDR_RLOC;
    else if (!opp_strcmp(par(SSADDR_PAR), SSADDR_EID_PARVAL))
        ssAddrType = SSADDR_EID;
    else
        ssAddrType = SSADDR_NONLISP;

    if ( syncType != SYNC_NONE ) {
        parseSyncSetup(config);
        syncAck = this->par(SYNCACK_PAR).boolValue();
    }
}

void LISPMapCache::initialize(int stage)
{
    if (stage < numInitStages() - 1)
        return;

    initPointers();

    restartMapCache();


    //DeviceConfigurator* devConf = ModuleAccess<DeviceConfigurator>("deviceConfigurator").get();
    //parseConfig( par("configData").xmlValue() );

    //Create default record
    //LISPMapEntry m1 = LISPMapEntry(LISPEidPrefix(IPv4Address::UNSPECIFIED_ADDRESS, 0));
    //m1.setAction(LISPCommon::SEND_MAP_REQUEST);
    //this->addMapEntry(m1);

    //updateDisplayString();

    //Init signals
    initSignals();

    //Watchers
    WATCH_LIST(MappingStorage);
    WATCH(syncType);

    if (syncType != SYNC_NONE) {
        WATCH_LIST(SyncSet);
        WATCH(syncKey);
    }
}

void LISPMapCache::updateCacheEntry(const TRecord& record) {
    Enter_Method("updateCacheEntry()");

    //Add to mapping storage
    updateMapEntry(record);

    //Change Timeout message appropriately
    //EV << "TTL> " << record.recordTTL << endl;
    updateTimeout(record.EidPrefix, simTime() + record.recordTTL * 60);

    //MappingStorage.sort();
    emit(sigSize, (long)MappingStorage.size());

    updateDisplayString();
}

void LISPMapCache::syncCacheEntry(LISPMapEntry& entry) {
    Enter_Method("syncCacheEntry()");

    EV << "Synchronizing " << endl << entry.info();

    //Add to mapping storage
    syncMapEntry(entry);

    //Change Timeout message appropriately
    updateTimeout(entry.getEidPrefix(), entry.getExpiry());

    //MappingStorage.sort();
    emit(sigSize, (long)MappingStorage.size());

    updateDisplayString();
}

LISPMapEntryTimer* LISPMapCache::findExpirationTimer(const LISPEidPrefix& eidPref) {
    for (TimeoutCItem it = CacheTimeouts.begin(); it != CacheTimeouts.end(); ++it) {
        if ( (*it)->getEidPref() == eidPref )
            return *it;
    }
    return NULL;
}

void LISPMapCache::updateTimeout(const LISPEidPrefix& eidPref, simtime_t time) {
    LISPMapEntryTimer* metim = findExpirationTimer(eidPref);
    //If ME did not existed
    if (!metim) {
        LISPMapEntryTimer* metim = new LISPMapEntryTimer(CACHE_TIMER);
        metim->setEidPref(eidPref);
        CacheTimeouts.push_back(metim);
        scheduleAt(time, metim);
    }
    //...update expiration timer
    else {
        cancelEvent(metim);
        scheduleAt(time, metim);
    }
}

void LISPMapCache::parseSyncSetup(cXMLElement* config) {


    if (!config)
        return;

    cXMLElement* sset = config->getFirstChildWithTag(SYNCSET_TAG);
    if (!sset) {
        EV << "Config XML file missing tag or attribute - SYNCHRONIZATIONSET" << endl;
        return;
    }

    //Integrity checks and retrieving key attribute value
    if (!sset->getAttribute(KEY_ATTR) || sset->getAttribute(KEY_ATTR)[0] == '\0') {
        EV << "Config XML file missing tag or attribute - KEY" << endl;
        return;
    }
    std::string sk = sset->getAttribute(KEY_ATTR);
    syncKey = sk;

    cXMLElementList members = sset->getChildrenByTagName(SETMEMBER_TAG);
    for (cXMLElementList::iterator i = members.begin(); i != members.end(); ++i) {
        cXMLElement* m = *i;

        //Check ADDRESS integrity
        if (!m->getAttribute(ADDRESS_ATTR)) {
            EV << "Config XML file missing tag or attribute - ADDRESS" << endl;
            continue;
        }
        std::string addr = m->getAttribute(ADDRESS_ATTR);

        //Create a new member record
        if (!isInSyncSet(IPvXAddress(addr.c_str())))
            SyncSet.push_back(LISPServerEntry(addr));
    }

}

LISPMapEntry* LISPMapCache::lookupMapEntry(IPvXAddress address) {
    LISPMapEntry* me = LISPMapStorageBase::lookupMapEntry(address);
    if (!me || me->getEidPrefix().getEidAddr().isUnspecified()) {
        emit(sigMiss, true);
    }
    else
        emit(sigLookup, true);
    return me;
}

void LISPMapCache::initPointers() {
    Ift = InterfaceTableAccess().get();
    MapDb = ModuleAccess<LISPMapDatabase>(MAPDB_MOD).get();
}

void LISPMapCache::handleMessage(cMessage *msg)
{
    //Receive Map-Entry expiration timer
    if (  dynamic_cast<LISPMapEntryTimer*>(msg) ) {
        LISPMapEntryTimer* metim = check_and_cast<LISPMapEntryTimer*>(msg);

        //Find ME where EidPref should be unique key
        LISPMapEntry* me = findMapEntryByEidPrefix(metim->getEidPref());

        //Remove ME from cache
        removeMapEntry(*me);

        //Remove timer
        CacheTimeouts.remove(metim);

        delete msg;
    }
    //...otherwise generate error
    else {
        error("Map-Cache cannot receive other messages than ME-timeouts!");
    }
}

bool LISPMapCache::isSyncAck() const {
    return syncAck;
}

const std::string& LISPMapCache::getSyncKey() const {
    return syncKey;
}

const ServerAddresses& LISPMapCache::getSyncSet() const {
    return SyncSet;
}

LISPMapCache::EMapSync LISPMapCache::getSyncType() const {
    return syncType;
}

void LISPMapCache::initSignals() {
    sigMiss     = registerSignal(SIG_CACHE_MISS);
    sigLookup   = registerSignal(SIG_CACHE_LOOKUP);
    sigSize     = registerSignal(SIG_CACHE_SIZE);

    sigPeerUp = registerSignal("SIG-PEERUP");
}

void LISPMapCache::updateDisplayString() {
    if (!ev.isGUI())
        return;
    std::ostringstream description;
    description << MappingStorage.size() - 1 << " entries";
    this->getDisplayString().setTagArg("t", 0, description.str().c_str());
    this->getDisplayString().setTagArg("t", 1, "t");
}

void LISPMapCache::restartMapCache() {
    Enter_Method("restartMapCache()");
    clearMappingStorage();

    //DeviceConfigurator* devConf = ModuleAccess<DeviceConfigurator>("deviceConfigurator").get();
    parseConfig( par("configData").xmlValue() );

    //Create default record
    LISPMapEntry m1 = LISPMapEntry(LISPEidPrefix(IPv4Address::UNSPECIFIED_ADDRESS, 0));
    m1.setAction(LISPCommon::SEND_MAP_REQUEST);
    this->addMapEntry(m1);

    updateDisplayString();
}

bool LISPMapCache::isInSyncSet(IPvXAddress ssmember) const {
    for (ServerCItem it = SyncSet.begin(); it != SyncSet.end(); ++it) {
        if (it->getAddress().equals(ssmember))
            return true;
    }
    return false;
}

void LISPMapCache::notifySyncset(cComponent* src) {
    Enter_Method("notifySyncSet()");

    //Parse interface name
    std::string iface = src->getFullPath().substr(src->getFullPath().find_last_of('.') + 1);
    std::string name = iface.substr(0, iface.find_first_of('['));
    std::string index = iface.substr(iface.find_first_of('[') + 1, iface.find_first_of(']') - iface.find_first_of('[') - 1);

    std::ostringstream os;
    os << name << index ;
    //EV << "YYYYYY" << os.str() << endl;

    IPv4InterfaceData* int4Data = Ift->getInterfaceByName(os.str().c_str())->ipv4Data();
    IPv4Address adr4 =
            (int4Data) ?
                    int4Data->getIPAddress() :
                    IPv4Address::UNSPECIFIED_ADDRESS;
    IPv6InterfaceData* int6Data = Ift->getInterfaceByName(os.str().c_str())->ipv6Data();
    IPv6Address adr6 =
            (int6Data) ?
                    int6Data->getPreferredAddress() :
                    IPv6Address::UNSPECIFIED_ADDRESS;

    //Emt nonLISP
    if (ssAddrType == SSADDR_NONLISP) {
        if (!adr4.isUnspecified()) {
            //EV << "!!!!!!!!!!!!" << adr4 << endl;
            emit(sigPeerUp, adr4.str().c_str());
        }
        if (!adr6.isUnspecified() && !adr6.isLinkLocal()) {
            //EV << "!!!!!!!!!!!!" << adr6 << endl;
            emit(sigPeerUp, adr6.str().c_str());
        }
    }
    //Emit RLOCs or EIDs
    else  {
        //IPv4
        if (!adr4.isUnspecified()) {
            MapStorage entrylist = MapDb->findMapEntriesByLocator(adr4);
            if (ssAddrType == SSADDR_RLOC) {
                if (!entrylist.empty()) {
                    //EV << "!!!!!!!!!!!!" << adr4 << endl;
                    emit(sigPeerUp, adr4.str().c_str());
                }
            }
            else if (ssAddrType == SSADDR_EID) {
                for (MapStorageItem it = entrylist.begin(); it != entrylist.end(); ++it) {
                    for (int i = 0; i < Ift->getNumInterfaces(); ++i) {
                        IPv4InterfaceData* int4Data = Ift->getInterface(i)->ipv4Data();
                        IPv4Address nadr4 =
                                (int4Data) ?
                                        int4Data->getIPAddress() :
                                        IPv4Address::UNSPECIFIED_ADDRESS;
                        if (it->getEidPrefix().getEidAddr().equals(
                                LISPCommon::getNetworkAddress(nadr4, it->getEidPrefix().getEidLength())
                                )
                           ) {
                            //EV << "!!!!!!!!!!!!" << nadr4 << endl;
                            emit(sigPeerUp, nadr4.str().c_str());
                        }
                    }
                }
            }
        }

        //IPv6
        if (!adr6.isUnspecified() && !adr6.isLinkLocal()) {
            MapStorage entrylist = MapDb->findMapEntriesByLocator(adr6);
            if (ssAddrType == SSADDR_RLOC) {
                if (!entrylist.empty()) {
                    //EV << "!!!!!!!!!!!!" << adr6 << endl;
                    emit(sigPeerUp, adr6.str().c_str());
                }
            }
            else if (ssAddrType == SSADDR_EID) {
                for (MapStorageItem it = entrylist.begin(); it != entrylist.end(); ++it) {
                    for (int i = 0; i < Ift->getNumInterfaces(); ++i) {
                        IPv6InterfaceData* int6Data = Ift->getInterface(i)->ipv6Data();
                        IPv6Address nadr6 =
                                (int6Data) ?
                                        int6Data->getPreferredAddress() :
                                        IPv6Address::UNSPECIFIED_ADDRESS;
                        if (it->getEidPrefix().getEidAddr().equals(
                                LISPCommon::getNetworkAddress(nadr6, it->getEidPrefix().getEidLength())
                                )
                           ) {
                            //EV << "!!!!!!!!!!!!" << nadr6 << endl;
                            emit(sigPeerUp, nadr6.str().c_str());
                        }
                    }
                }
            }
        }
    }

}
