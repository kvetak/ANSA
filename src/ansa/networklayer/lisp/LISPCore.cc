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

#include "LISPCore.h"

Define_Module(LISPCore);

LISPCore::LISPCore()
{
    acceptMapRequestMapping = false;
    mapCacheTtl = DEFAULT_TTL_VAL;
}

LISPCore::~LISPCore()
{
    MapServers.clear();
    MapResolvers.clear();
}

bool LISPCore::isMapResolverV4() const {
    return mapResolverV4;
}

void LISPCore::setMapResolverV4(bool mapResolverV4) {
    this->mapResolverV4 = mapResolverV4;
}

bool LISPCore::isMapResolverV6() const {
    return mapResolverV6;
}

void LISPCore::setMapResolverV6(bool mapResolverV6) {
    this->mapResolverV6 = mapResolverV6;
}

bool LISPCore::isMapServerV4() const {
    return mapServerV4;
}

void LISPCore::setMapServerV4(bool mapServerV4) {
    this->mapServerV4 = mapServerV4;
}

bool LISPCore::isMapServerV6() const {
    return mapServerV6;
}

void LISPCore::setMapServerV6(bool mapServerV6) {
    this->mapServerV6 = mapServerV6;
}

void LISPCore::parseMapServerConfig(cXMLElement* config) {
    //Map server addresses initial setup
    cXMLElementList msa = config->getChildrenByTagName(ETRMAPSERVER_TAG);
    for (cXMLElementList::iterator i = msa.begin(); i != msa.end(); ++i) {
        cXMLElement* m = *i;

        //Integrity checks and retrieving address attribute value
        std::string mipv;
        if (!m->getAttribute(ADDRESS_ATTR)) {
            EV << "Config XML file missing tag or attribute - ADDRESS" << endl;
            continue;
        } else
            mipv = m->getAttribute(ADDRESS_ATTR);

        //Integrity checks and retrieving key attribute value
        if (!m->getAttribute(KEY_ATTR)
                || m->getAttribute(KEY_ATTR)[0] == '\0') {
            EV << "Config XML file missing tag or attribute - KEY" << endl;
            continue;
        }
        std::string mk = m->getAttribute(KEY_ATTR);

        //Integrity checks and retrieving proxy attribute value
        bool proxy = false;
        if (m->getAttribute(PROXY_ATTR)) {
            if (!strcmp(m->getAttribute(PROXY_ATTR), ENABLED_VAL))
                proxy = true;
        }

        //Integrity checks and retrieving proxy attribute value
        bool notify = false;
        if (m->getAttribute(NOTIFY_ATTR)) {
            if (!strcmp(m->getAttribute(NOTIFY_ATTR), ENABLED_VAL))
                notify = true;
        }

        //Integrity checks and retrieving proxy attribute value
        bool quickreg = false;
        if (m->getAttribute(QUICKREG_ATTR)) {
            if (!strcmp(m->getAttribute(QUICKREG_ATTR), ENABLED_VAL))
                quickreg = true;
        }

        MapServers.push_back(
                    LISPServerEntry(mipv, mk, proxy, notify, quickreg));
    }
}

LISPServerEntry* LISPCore::findServerEntryByAddress(ServerAddresses& list,
        IPvXAddress& addr) {
    for (ServerItem it = list.begin(); it != list.end(); ++it ) {
        if (it->getAddress() == addr)
            return &(*it);
    }
    return NULL;
}

void LISPCore::parseMapResolverConfig(cXMLElement* config) {
    //Map resolver addresses initial setup
    cXMLElementList mra = config->getChildrenByTagName(ITRMAPRESOLVER_TAG);
    for (cXMLElementList::iterator i = mra.begin(); i != mra.end(); ++i) {
        cXMLElement* m = *i;

        //Integrity checks and retrieving address attribute value
        std::string mipv;
        if (!m->getAttribute(ADDRESS_ATTR)) {
            EV << "Config XML file missing tag or attribute - ADDRESS" << endl;
            continue;
        } else
            mipv = m->getAttribute(ADDRESS_ATTR);

        MapResolvers.push_back(LISPServerEntry(mipv));
    }

    //Init Resolver queue
    MapResolverQueue = MapResolvers.begin();
}

void LISPCore::parseConfig(cXMLElement* config)
{
    //Config element is empty
    if (!config)
        return;

    //Map server addresses initial setup
    parseMapServerConfig(config);

    //Map resolver addresses initial setup
    parseMapResolverConfig(config);

    //Enable MS functionality
    mapServerV4 = par(MS4_PAR).boolValue();
    mapServerV6 = par(MS6_PAR).boolValue();
    cXMLElement* ms = config->getFirstChildWithTag(MAPSERVER_TAG);
    if (ms != NULL) {
        if ( !strcmp(ms->getAttribute(IPV4_ATTR), ENABLED_VAL) )
            mapServerV4 = true;
        if ( !strcmp(ms->getAttribute(IPV6_ATTR), ENABLED_VAL) )
            mapServerV6 = true;
    }
    par(MS4_PAR).setBoolValue(mapServerV4);
    par(MS6_PAR).setBoolValue(mapServerV6);

    //Enable MR functionality
    mapResolverV4 = par(MR4_PAR).boolValue();
    mapResolverV6 = par(MR6_PAR).boolValue();
    cXMLElement* mr = config->getFirstChildWithTag(MAPRESOLVER_TAG);
    if ( mr != NULL) {
        if ( !strcmp(mr->getAttribute(IPV4_ATTR), ENABLED_VAL) )
            mapResolverV4 = true;
        if ( !strcmp(mr->getAttribute(IPV6_ATTR), ENABLED_VAL) )
            mapResolverV6 = true;
    }
    par(MR4_PAR).setBoolValue(mapResolverV4);
    par(MR6_PAR).setBoolValue(mapResolverV6);
}

void LISPCore::scheduleRegistration() {
    //Acts as ETR register for LISP sites and schedule registration for each MS
    for (ServerCItem it = MapServers.begin(); it != MapServers.end(); ++it) {
        LISPRegisterTimer* regtim = new LISPRegisterTimer(REGISTER_TIMER);
        regtim->setServerAddress(it->getAddress());
        if (it->isQuickRegistration()) {
            regtim->setQuickRegCycles(QUICKREG_CYCLES_VAL);
            scheduleAt(simTime(), regtim);
        }
        else {
            regtim->setQuickRegCycles(0);
            scheduleAt(simTime() + (ciscoStartupDelays ? DEFAULT_REGTIMER_VAL : 0), regtim);
        }
    }
}

void LISPCore::scheduleRlocProbing() {
    //Cycle through all Local locators
    for (MapStorageItem it = MapDb->getMappingStorage().begin();
            it != MapDb->getMappingStorage().end(); ++it) {
        for (LocatorItem jt = it->getRlocs().begin();
                jt != it->getRlocs().end(); ++jt) {
            if (jt->isLocal()) {
                //Local RLOC is marked as up TODO: Vesely - Periodic checks?
                jt->setState(LISPRLocator::UP);
                continue;
            }

            //Non-local RLOC is marked as down
            jt->setState(LISPRLocator::DOWN);

            //Probe is configured for each RLOC and each of its EID...
            if ( !strcmp(par(RLOCPROBINGALGO_PAR).stringValue(), ALGO_CISCO_PARVAL) ) {
                LISPProbeEntry probe = LISPProbeEntry(jt->getRlocAddr());
                probe.addEid(it->getEidPrefix(), &(*jt) );
                ProbingSet.addProbeEntry(probe);
            }
            //...or each RLOC has all EIDs assigned to it
            else{
                LISPProbeEntry* probe = ProbingSet.findFirstProbeEntryByRloc(jt->getRlocAddr());
                //IF RLOC exists THEN append EID
                if (probe)
                    probe->addEid(it->getEidPrefix(), &(*jt) );
                //...OTHERWISE create new Probe
                else {
                    probe = new LISPProbeEntry(jt->getRlocAddr());
                    probe->addEid(it->getEidPrefix(), &(*jt) );
                    ProbingSet.addProbeEntry(*probe);
                }
            }
        }
    }

    //Schedule RlocProbes
    for (ProbeItem it = ProbingSet.getProbes().begin(); it != ProbingSet.getProbes().end(); ++it) {
        LISPRlocProbeTimer* rlocprobe = new LISPRlocProbeTimer(REQUESTPROBE_TIMER);
        //EV << it->getRlocAddr() << "\n" << it->getEids() << endl;
        rlocprobe->setRlocAddr(it->getRlocAddr());
        rlocprobe->setEidPref(it->getNextEid());
        rlocprobe->setPreviousNonce(DEFAULT_NONCE_VAL);
        rlocprobe->setRetryCount(0);
        //Shift IPv6 probes +30 seconds from IPv4 ones
        if (ciscoStartupDelays && it->getRlocAddr().isIPv6()) {
            scheduleAt(simTime() + DEFAULT_PROBETIMER_VAL/2, rlocprobe);
        }
        //IPv4 or otherwise IPv4/IPv6 probes start at the same time
        else {
            scheduleAt(simTime(), rlocprobe);
        }
    }
}

void LISPCore::scheduleCacheSync(const LISPEidPrefix& eidPref) {
    //IF map-cache content changes AND cache synchronization is enabled THEN
    if (MapCache->getSyncType() != LISPMapCache::SYNC_NONE) {
        for (ServerCItem it = MapCache->getSyncSet().begin();
                it != MapCache->getSyncSet().end(); ++it) {
            //EV << "Sync for " << it->getAddress() << endl;
            LISPSyncTimer* synctim = new LISPSyncTimer(CACHESYNC_TIMER);
            synctim->setSetMember(it->getAddress());
            synctim->setEidPref(eidPref);
            synctim->setPreviousNonce(DEFAULT_NONCE_VAL);
            synctim->setRetryCount(0);
            scheduleAt(simTime(), synctim);
        }
    }
}

void LISPCore::scheduleWholeCacheSync(IPvXAddress& ssmember) {
    Enter_Method("scheduleWholeCacheSync()");
    //IF map-cache content changes AND cache synchronization is enabled THEN
    if (MapCache->getSyncType() != LISPMapCache::SYNC_NONE) {
        //EV << "Sync for " << it->getAddress() << endl;
        LISPSyncTimer* synctim = new LISPSyncTimer(CACHESYNC_TIMER);
        synctim->setSetMember(ssmember);
        synctim->setEidPref(LISPEidPrefix());
        synctim->setPreviousNonce(DEFAULT_NONCE_VAL);
        synctim->setRetryCount(0);
        scheduleAt(simTime(), synctim);
    }
}

void LISPCore::initPointers() {
    // access to the routing and interface table
    //Rt = AnsaRoutingTableAccess().get();
    // local MapCache
    MapCache = ModuleAccess<LISPMapCache>(MAPCACHE_MOD).get();
    //MapDb
    MapDb = ModuleAccess<LISPMapDatabase>(MAPDB_MOD).get();

    //MapDb pointer when needed
    bool hasSiteDB = this->getParentModule()->par(HASSITEDB_PAR).boolValue();
    if (hasSiteDB)
        SiteDb = ModuleAccess<LISPSiteDatabase>(SITEDB_MOD).get();

    //If node is acting as MS and it has not MapDB then throw error
    if (!hasSiteDB && (mapServerV4 || mapServerV6)) {
        throw cException("Cannot act as Map Server without Mapping Database!");
    }


    if (!MapCache || !MapDb) {
        error("Pointers to RoutingTable or MapCache or MapDatabase were not successfully initialized!");
    }

    //MsgLogger pointer
    MsgLog = ModuleAccess<LISPMsgLogger>(LOGGER_MOD).get();
    if (!MsgLog)
        EV << "MsgLogger not available, omitting logging." << endl;
}

void LISPCore::initSockets() {
    controlTraf.setOutputGate(gate(CONTROL_GATEOUT));
    controlTraf.setReuseAddress(true);
    controlTraf.bind(CONTROL_PORT_VAL);

    dataTraf.setOutputGate(gate(DATA_GATEOUT));
    dataTraf.setReuseAddress(true);
    dataTraf.bind(DATA_PORT_VAL);
}

void LISPCore::initialize(int stage)
{
    if (stage < numInitStages() - 1)
        return;

    //LISP configuration parsing
    parseConfig( par(CONFIG_PAR) );

    //Set parameters for MapCache behavior
    acceptMapRequestMapping = par(ACCEPTREQMAPPING_PAR).boolValue();

    //Set EchoNonceAlgorithm
    echoNonceAlgo = par(ECHONONCEALGO_PAR).boolValue();

    //Set CiscoStartupDelay for registration and probing
    ciscoStartupDelays = par(CISCOSTARTUPDELAY_PAR).boolValue();

    if (par(MAPCACHETTL_PAR).longValue() <= 0) {
        mapCacheTtl = DEFAULT_TTL_VAL;
        par(MAPCACHETTL_PAR).setLongValue(DEFAULT_TTL_VAL);
    }
    else
        mapCacheTtl = (unsigned short) ( par(MAPCACHETTL_PAR).longValue() );

    //Init pointers to other subcomponents
    initPointers();

    //Acts as ETR register for LISP sites and schedule registration for each MS
    if ( MapServers.empty() && !MapDb->getMappingStorage().empty() ) {
        EV << "EtrMappings are present but no MapServer is configured!" << endl;
    }
    else
        scheduleRegistration();

    //TODO: Vesely - What about local locators? How should check the reachibility?
    //Schedule MapETR RLOC probing where each non-local locator is tested for its EID every 60 seconds
    scheduleRlocProbing();

    //Register UDP port 4342
    initSockets();

    packetfrwd = 0;
    packetdrop= 0;
    updateDisplayString();
    initSignals();

    //Watchers
    WATCH_LIST(MapResolvers);
    WATCH_LIST(MapServers);
    WATCH_LIST(ProbingSet.getProbes());
}

void LISPCore::expireRegisterTimer(LISPRegisterTimer* regtim) {
    //Retrieve ServerEntry
    LISPServerEntry* se = findServerEntryByAddress(MapServers, regtim->getServerAddress());
    if (se) {
        //Register LISP Site
        sendMapRegister(*se);
        //TODO: Vesely - Not very nice QuickRegistration handling
        //Handle QuickRegistration
        if (regtim->getQuickRegCycles())
            regtim->setQuickRegCycles(regtim->getQuickRegCycles() - 1);

        se->setQuickRegistration(regtim->getQuickRegCycles());
        //Reschedule timer to expire
        if (se->isQuickRegistration())
            scheduleAt(simTime() + QUICK_REGTIMER_VAL, regtim);
        else
            scheduleAt(simTime() + DEFAULT_REGTIMER_VAL, regtim);
    } else
        //IF ServerEntry seized to exist THEN delete
        cancelAndDelete(regtim);
}

void LISPCore::expireRequestTimer(LISPRequestTimer* reqtim) {
    //Check whether previous request was replied. IF true THEN cancel ELSE retry sending Map-Request
    if (reqtim->getRetryCount() && MsgLog->findMsg(LISPMsgEntry::REPLY, reqtim->getPreviousNonce()))
        cancelAndDelete(reqtim);
    else {
        //Send Map-Request message
        unsigned long nonce;
        nonce = sendEncapsulatedMapRequest(reqtim->getSrcEid(), reqtim->getDstEid());

        //Iterate ResolverQueue to deterministically change queried MR
        //FIXME: Vesely - What if MR from unsupported AFI?
        MapResolverQueue++;
        if (MapResolverQueue == MapResolvers.end())
            MapResolverQueue = MapResolvers.begin();

        //Plan next timeout our cancel timer when number of retries is reached
        if (reqtim->getRetryCount() + 1 <= DEFAULT_MAXREQRETRIES_VAL) {
            scheduleAt(simTime() + (simtime_t) pow(DEFAULT_REQMULTIPLIER_VAL,
                       reqtim->getRetryCount()), reqtim);
            reqtim->setRetryCount(reqtim->getRetryCount() + 1);
            reqtim->setPreviousNonce(nonce);
        } else {
            EV << "Map-Request for EID " << reqtim->getDstEid()
                      << " timed out after " << (short) reqtim->getRetryCount()
                      << " retries!" << endl;
            cancelAndDelete(reqtim);
        }
    }
}

void LISPCore::expireRlocProbeTimer(LISPRlocProbeTimer* probetim) {
    //EV << "LISPRLOCPROBE" << endl;
    simtime_t nexttime;
    //IF last RLOC-Probe was successfully replied...
    if ( MsgLog->findMsg(LISPMsgEntry::RLOC_PROBE_REPLY, probetim->getPreviousNonce()) ) {
        nexttime = DEFAULT_PROBETIMER_VAL;
        probetim->setRetryCount(0);
        probetim->setPreviousNonce(DEFAULT_NONCE_VAL);
        scheduleAt(simTime() + nexttime, probetim);
    }
    //...OR max-retrycount reached THEN timeout and mark locator as down
    else if (probetim->getRetryCount() + 1 == DEFAULT_MAXREQRETRIES_VAL) {
        LISPProbeEntry* probe =
                        ProbingSet.findProbeEntryByRlocAndEid(probetim->getRlocAddr(), probetim->getEidPref());
        EV << "Probe RetryCount reached, marking RLOC " << probetim->getRlocAddr() << " as down!" << endl;
        probe->setRlocStatusForAllEids(LISPRLocator::DOWN);

        nexttime = DEFAULT_PROBETIMER_VAL;
        probetim->setRetryCount(0);
        probetim->setPreviousNonce(DEFAULT_NONCE_VAL);
        scheduleAt(simTime() + nexttime, probetim);
    }
    //...otherwise send RLOC-Probe, iterate RetryCount and prepare next probe
    else {
        //Send Map-Request RLOC-Probe message
        //EV << "RLOC: " << probetim->getRlocAddr() << "  EID: " << probetim->getEidPref() << endl;
        unsigned long nonce;
        nonce = sendRlocProbe(probetim->getRlocAddr(), probetim->getEidPref());

        LISPProbeEntry* probe =
                ProbingSet.findProbeEntryByRlocAndEid(probetim->getRlocAddr(), probetim->getEidPref());
        if (!probe)
            throw cException("Could not find record!");
        probe->setLastTimeProbed(simTime());

        if ( !strcmp(par(RLOCPROBINGALGO_PAR).stringValue(), ALGO_EIDRR_PARVAL) ) {
            //Round-robin through EIDs using this locator
            LISPEidPrefix eidPref = ProbingSet.findFirstProbeEntryByRloc(probetim->getRlocAddr())->getNextEid();
            EV << "Before " << probetim->getEidPref() << " / after " << eidPref << endl;
            probetim->setEidPref(eidPref);
        }

        nexttime = (simtime_t) pow(DEFAULT_REQMULTIPLIER_VAL, probetim->getRetryCount());
        probetim->setRetryCount(probetim->getRetryCount() + 1);
        probetim->setPreviousNonce(nonce);
        //Schedule next RLOC-Probe check
        scheduleAt(simTime() + nexttime, probetim);
    }
}

void LISPCore::expireSyncTimer(LISPSyncTimer* synctim) {
    //Check whether previous request was replied. IF true THEN cancel ELSE retry sending Map-Request
    if (synctim->getRetryCount() && MsgLog->findMsg(LISPMsgEntry::CACHE_SYNC_ACK, synctim->getPreviousNonce()))
        cancelAndDelete(synctim);
    else {
        //EV << "MemberIP: " << synctim->getSetMember() << ", EID: " << synctim->getEidPref() << endl;
        //Send CacheSync message
        unsigned long nonce;
        nonce = sendCacheSync(synctim->getSetMember(), synctim->getEidPref());

        //Plan next timeout our cancel timer when number of retries is reached
        if (MapCache->isSyncAck() && synctim->getRetryCount() + 1 <= DEFAULT_MAXREQRETRIES_VAL) {
            scheduleAt(simTime()
                    + (simtime_t)pow(DEFAULT_REQMULTIPLIER_VAL,synctim->getRetryCount()), synctim);
            synctim->setRetryCount(synctim->getRetryCount() + 1);
            synctim->setPreviousNonce(nonce);
        }
        else {
            EV << "CacheSync for synchronization set member " << synctim->getSetMember()
               << " timed out or is not set for acknowledgment!" << endl;
            cancelAndDelete(synctim);
        }
    }
}

void LISPCore::handleTimer(cMessage* msg) {
    //Register timer expires
    if ( dynamic_cast<LISPRegisterTimer*>(msg) ) {
        expireRegisterTimer(check_and_cast<LISPRegisterTimer*>(msg));
    }
    //Request timer expires
    else if (dynamic_cast<LISPRequestTimer*>(msg)) {
        expireRequestTimer(check_and_cast<LISPRequestTimer*>(msg));
    }
    else if (dynamic_cast<LISPRlocProbeTimer*>(msg)) {
        expireRlocProbeTimer(check_and_cast<LISPRlocProbeTimer*>(msg));
    }
    else if (dynamic_cast<LISPSyncTimer*>(msg)) {
        //EV << "SyncTimer expired" << endl;
        expireSyncTimer(check_and_cast<LISPSyncTimer*>(msg));
    }
    else {
        error("Unrecognized timer (%s)", MsgLog->getClassName());
        cancelAndDelete(msg);
    }
}

void LISPCore::handleCotrol(cMessage* msg) {
    //EV << "Control" << endl;
    //If encapsulated control message THEN decapsulate
    if (dynamic_cast<LISPEncapsulated*>(msg)) {
        //EV << "Vevnitr" << endl;
        //Get top most ControlInfo because other are empty
        cPacket* plispenc = (cPacket*)msg;
        cObject* control = plispenc->removeControlInfo();

        //Unwrap unnecessary envelopes
        cPacket* pip = plispenc->decapsulate();
        cPacket* pudp = pip->decapsulate();
        cPacket* plisp = pudp->decapsulate();

        //Copy control info
        plisp ->setControlInfo(control);

        //Delete other outer layers
        delete plispenc;
        delete pip;
        delete pudp;

        //Set msg pointer to inner LISP message
        msg = plisp;
    }
    //Map-Notify must be evaluated earlier because it is inherited from Map-Register
    if ( dynamic_cast<LISPMapNotify*>(msg) ) {
            receiveMapNotify( check_and_cast<LISPMapNotify*>(msg) );
    }
    else if ( dynamic_cast<LISPMapRegister*>(msg) ) {
        LISPMapRegister* lmreg = check_and_cast<LISPMapRegister*>(msg);
        //Register site
        receiveMapRegister( lmreg );
        //Send Map-Notify if solicited
        if (lmreg->getMapNotifyBit())
            sendMapNotify(lmreg);
    }
    else if ( dynamic_cast<LISPMapRequest*>(msg) ) {
        LISPMapRequest* lmreq = check_and_cast<LISPMapRequest*>(msg);
        //Process RLOC-Probe
        if ( lmreq->getProbeBit() )
            receiveRlocProbe(lmreq);
        //Process ordinary Map-Request
        else
            receiveMapRequest( lmreq );
    }
    else if ( dynamic_cast<LISPMapReply*>(msg) ) {
        LISPMapReply* lmrep = check_and_cast<LISPMapReply*>(msg);
        //Process RLOC-Probe-Reply
        if ( lmrep->getProbeBit() )
            receiveRlocProbeReply(lmrep);
        //Process ordinary Map-Reply
        else
            receiveMapReply( lmrep );
    }
    //Cache-Sync must be evaluated earlier because it is inherited from Cache-Sync-Ack
    else if ( dynamic_cast<LISPCacheSync*>(msg) ) {
        LISPCacheSync* lcs = check_and_cast<LISPCacheSync*>(msg);
        receiveCacheSync(lcs);

        //Send Ack if necessary
        if (lcs->getAckBit())
            sendCacheSyncAck(lcs);
    }
    else if ( dynamic_cast<LISPCacheSyncAck*>(msg) ) {
        LISPCacheSyncAck* lcsa = check_and_cast<LISPCacheSyncAck*>(msg);
        receiveCacheSyncAck(lcsa);
    }

    delete msg;
}

void LISPCore::handleDataEncaps(cMessage* msg) {
    IPvXAddress dstAddr, srcAddr;
    //IPv4 packet
    if (msg->arrivedOn(IPV4_GATEIN)) {
        IPv4Datagram* datagram = check_and_cast<IPv4Datagram*>(msg);
        srcAddr = datagram->getSrcAddress();
        dstAddr = datagram->getDestAddress();
    }
    //IPv6 packet
    else if (msg->arrivedOn(IPV6_GATEIN)) {
        IPv6Datagram* datagram = check_and_cast<IPv6Datagram*>(msg);
        srcAddr = datagram->getSrcAddress();
        dstAddr = datagram->getDestAddress();
    }

    LISPMapEntry* me = MapCache->lookupMapEntry(dstAddr);

    //Known EID-to-RLOC mapping
    if (me && !me->getEidPrefix().getEidAddr().isUnspecified()) {
        sendEncapsulatedDataMessage(srcAddr, dstAddr, me, (cPacket*)msg);

        updateStats(PACKET_FORWARD);
    }
    //Unknown EID-to-RLOC mapping
    else {
        //Schedule LISP Map-Request
        LISPRequestTimer* reqtim = new LISPRequestTimer(REQUEST_TIMER);
        reqtim->setSrcEid(srcAddr);
        reqtim->setDstEid(dstAddr);
        reqtim->setRetryCount(0);
        reqtim->setPreviousNonce(DEFAULT_NONCE_VAL);
        scheduleAt(simTime(), reqtim);

        //Drop packet
        EV << "No LISP map-cache record, thus dropping packet" << endl;
        delete msg;

        updateStats(PACKET_DROP);
    }

}

void LISPCore::handleDataDecaps(cMessage* msg) {
    if (dynamic_cast<LISPHeader*>(msg)) {
        /*
        LISPHeader* lidata = check_and_cast<LISPHeader*>(msg);
        UDPDataIndication* udpi = check_and_cast<UDPDataIndication*>(lidata->getControlInfo());
        IPvXAddress dst = udpi->getDestAddr();
        MsgLog->addMsg(lidata, LISPMsgEntry::DATA, dst, false);
        */

        cPacket* pip = ((cPacket*)msg)->decapsulate();

        //Ordinary encapsulated data message that should be forwarded by this ETR
        if (dynamic_cast<IPv4Datagram*>(pip))
            send(pip, IPV4_GATEOUT);
        else if (dynamic_cast<IPv6Datagram*>(pip))
            send(pip, IPV6_GATEOUT);

        updateStats(PACKET_FORWARD);
    }
    else
        error("Unrecognized data packet (%s)", MsgLog->getClassName());

    delete msg;
}

void LISPCore::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage())
        handleTimer(msg);
    else if (msg->arrivedOn(IPV4_GATEIN) || msg->arrivedOn(IPV6_GATEIN) )
        handleDataEncaps(msg);
    else if (msg->arrivedOn(DATA_GATEIN))
        handleDataDecaps(msg);
    else if (msg->arrivedOn(CONTROL_GATEIN))
        handleCotrol(msg);
    else {
        error("Unrecognized message (%s)", MsgLog->getClassName());
        delete msg;
    }
}

void LISPCore::updateDisplayString()
{
    if (!ev.isGUI())
        return;
    std::ostringstream description;
    description << packetfrwd << " forwarded" << endl
                << packetdrop << " dropped";
    this->getDisplayString().setTagArg("t", 0, description.str().c_str());
    this->getDisplayString().setTagArg("t", 1, "t");
}

void LISPCore::sendMapRegister(LISPServerEntry& se) {
    LISPMapRegister* lmreg = new LISPMapRegister(REGISTER_MSG);

    //General fields
    unsigned long newnonce = (unsigned long) ev.getRNG(0)->intRand();
    lmreg->setNonce(newnonce);

    lmreg->setProxyBit(se.isProxyReply());
    lmreg->setMapNotifyBit(se.isMapNotify());

    //Authentication data
    lmreg->setKeyId(LISPCommon::KID_NONE);
    lmreg->setAuthDataLen(se.getKey().size());
    lmreg->setAuthData(se.getKey().c_str());

    //TRecords
    for (MapStorageItem it = MapDb->getMappingStorage().begin(); it != MapDb->getMappingStorage().end(); ++it) {
        TRecord rec = prepareTRecord(&(*it), true, LISPCommon::NO_ACTION);
        lmreg->getRecords().push_back(rec);
    }
    //As many records as there are EtrMappings
    lmreg->setRecordCount(lmreg->getRecords().size());

    //Count dynamic size
    lmreg->setByteLength(countPacketSize(lmreg));

    //Log and send
    //EV << "Map-Register message generated." << endl;
    MsgLog->addMsg(lmreg, LISPMsgEntry::REGISTER, se.getAddress(), true);
    controlTraf.sendTo(lmreg, se.getAddress() , CONTROL_PORT_VAL);
}

void LISPCore::receiveMapRegister(LISPMapRegister* lmreg) {
    //Log
    UDPDataIndication* udpi = check_and_cast<UDPDataIndication*>(lmreg->getControlInfo());
    IPvXAddress src = udpi->getSrcAddr();
    MsgLog->addMsg(lmreg, LISPMsgEntry::REGISTER, src, false);

    //Check whether device has SiteDB
    if (!mapServerV4 && !mapServerV6) {
        EV << "non-MS device received Map-Register!" << endl;
        return;
    }

    //Check for 0 records
    if (lmreg->getRecordCount() == 0) {
        EV << "Map-Register contains zero records!" << endl;
        return;
    }

    //Check the authentication method
    if (lmreg->getKeyId() != 0) {
        EV << "Map-Register contains unsupported authentication method!" << endl;
        return;
    }

    //FIXME: Vesely - Searching for SiteInfo should NOT be done based on key. Remove simplification!
    std::string authData = lmreg->getAuthData();
    LISPSite* si = SiteDb->findSiteInfoByKey(authData);

    if (si) {
        UDPDataIndication* udpi = check_and_cast<UDPDataIndication*>(lmreg->getControlInfo());
        IPvXAddress src = udpi->getSrcAddr();

        LISPSiteRecord* srec = SiteDb->updateSiteEtr(si, src, lmreg->getProxyBit());

        for (TRecordCItem it = lmreg->getRecords().begin(); it != lmreg->getRecords().end(); ++it) {
            //Trying to store EID from AF that server does not operate
            if ( (it->EidPrefix.getEidAddr().isIPv6()  && !mapServerV6)
                 || (!it->EidPrefix.getEidAddr().isIPv6() && !mapServerV4) ) {
                EV << "Map-register contains EID " << it->EidPrefix << " with AF not recognized by map server!" << endl;
                continue;
            }
            //Trying to store EID from different scope then Maintained EIDs
            if ( !si->isEidMaintained(it->EidPrefix.getEidAddr()) ) {
                EV << "EID " << it->EidPrefix << " is not recognized for site " << si->getSiteName() << endl;
                continue;
            }

            //Store it into SiteDatabase
            SiteDb->updateEtrEntries(srec, *it, si->getSiteName());
        }
    }
    else {
        EV << "Map-Register contains unknown LISP site!" << endl;
    }
}

void LISPCore::sendMapNotify(LISPMapRegister* lmreg) {
    LISPMapNotify* lmnot = new LISPMapNotify(NOTIFY_MSG);

    UDPDataIndication* udpi = check_and_cast<UDPDataIndication*>(lmreg->getControlInfo());
    IPvXAddress dst = udpi->getSrcAddr();

    //Copy Map-Register content onto Map-Notify
    lmnot->setNonce(lmreg->getNonce());
    lmnot->setRecordCount(lmreg->getRecordCount());
    //lmnot->setProxyBit(lmreg->getProxyBit());
    //lmnot->setMapNotifyBit(lmreg->getMapNotifyBit());
    lmnot->setKeyId(lmreg->getKeyId());
    lmnot->setAuthDataLen(lmreg->getAuthDataLen());
    lmnot->setAuthData(lmreg->getAuthData());
    lmnot->setRecords(lmreg->getRecords());

    //Count dynamic size
    lmnot->setByteLength(countPacketSize(lmnot));

    //Log and send
    MsgLog->addMsg(lmnot, LISPMsgEntry::NOTIFY, dst, true);
    controlTraf.sendTo(lmnot, dst, CONTROL_PORT_VAL);
}

void LISPCore::receiveMapNotify(LISPMapNotify* lmnot) {
    //Retrieve source IP address
    UDPDataIndication* udpi = check_and_cast<UDPDataIndication*>(lmnot->getControlInfo());
    IPvXAddress src = udpi->getSrcAddr();

    //Log
    MsgLog->addMsg(lmnot, LISPMsgEntry::NOTIFY, src, false);

    //Check whether last Map-Register was acknowledged
    if ( MsgLog->findMsg(LISPMsgEntry::REGISTER, lmnot->getNonce() ) ) {
        EV << "Map-Register was acknowledged by Map-Notify with nonce " << lmnot->getNonce() << endl;
        //Retrieve appropriate ServerEntry
        LISPServerEntry* se = findServerEntryByAddress(MapServers, src);
        if (se) {
            //TODO: Instead of parsing some simple check that every ETR was replied
            /*
             * FIXME: Vesely - Testing purposes MapRequest generation
            IPvXAddress src = IPvXAddress("192.168.2.1");
            IPvXAddress dst = IPvXAddress("192.168.1.1");

            LISPRequestTimer* reqtim = new LISPRequestTimer(REQUEST_TIMER);
            reqtim->setSrcEid(src);
            reqtim->setDstEid(dst);
            reqtim->setRetryCount(0);
            reqtim->setPreviousNonce(DEFAULT_NONCE_VAL);
            scheduleAt(simTime(), reqtim);
            */
        }
        else
            EV << "ServerEntry has not been found!" << endl;
    }
    else
        EV << "Unsolicited Map-Notify received!" << endl;
}

TRecord LISPCore::prepareTRecord(LISPMapEntry* me, bool Abit, LISPCommon::EAct action) {
    TRecord rec;
    rec.recordTTL = mapCacheTtl;
    rec.EidPrefix = me->getEidPrefix();
    rec.mapVersion = DEFAULT_MAPVER_VAL;
    rec.ABit = Abit;
    rec.ACT = action;
    rec.locatorCount = me->getRlocs().size();
    for (LocatorCItem jt = me->getRlocs().begin(); jt != me->getRlocs().end(); ++jt) {
        TLocator loc;
        loc.RLocator = *jt;
        //IF ETR is responding THEN local is relevant...
        if (Abit)
            loc.LocalLocBit = jt->isLocal();
        //...otherwise MS is responding
        else
            loc.LocalLocBit = false;
        loc.piggybackBit = false;
        //Reachbility is measured according to RLOC state
        loc.RouteRlocBit = (jt->getState() == LISPRLocator::UP) ? true : false;
        rec.locators.push_back(loc);
    }
    return rec;
}

unsigned long LISPCore::sendEncapsulatedMapRequest(IPvXAddress& srcEid, IPvXAddress& dstEid) {
    LISPMapRequest* lmreq = new LISPMapRequest(REQUEST_MSG);

    //General fields
    unsigned long newnonce = (unsigned long) ev.getRNG(0)->intRand();
    lmreq->setNonce(newnonce);

    lmreq->setProbeBit(false);

    lmreq->setSourceEid(TAfiAddr(srcEid));

    LISPMapEntry* me = MapDb->lookupMapEntry(srcEid);

    //TODO: Vesely - If there is no ME match THEN send it without MapReply
    if (me) {
        //Prepare ITR-RLOCs
        for (LocatorCItem it = me->getRlocs().begin(); it != me->getRlocs().end(); ++it)
            if (it->isLocal())
                lmreq->getItrRlocs().push_back(TAfiAddr(it->getRlocAddr()));
        lmreq->setItrRlocCount(lmreq->getItrRlocs().size() - 1);

        //Multiple Recs in Map-Request, TODO: Vesely - Currently it is not supported yet
        lmreq->setRecordCount(1);
        LISPEidPrefix irec = dstEid.isIPv6() ? LISPEidPrefix(dstEid, 128) : LISPEidPrefix(dstEid, 32);
        lmreq->getRecs().push_back(irec);

        //Map-Reply Record
        lmreq->setMapDataBit(true);
        TRecord rep = prepareTRecord(me, true, LISPCommon::NO_ACTION);
        lmreq->setMapReply(rep);
    }
    else {
        error("Cannot send Map-Request because srcEID does not have ME!");
        return DEFAULT_NONCE_VAL;
    }

    //Count dynamic size
    lmreq->setByteLength(countPacketSize(lmreq));

    //Create UDP Envelope
    UDPPacket* udpPacket = new UDPPacket(REQUEST_MSG);
    udpPacket->setByteLength(UDP_HEADER_BYTES);
    udpPacket->encapsulate(lmreq);
    udpPacket->setSourcePort(CONTROL_PORT_VAL);
    udpPacket->setDestinationPort(CONTROL_PORT_VAL);

    cPacket* middle;
    //Create IPv6 envelope
    if (srcEid.isIPv6() && dstEid.isIPv6()) {
        IPv6Datagram *datagram = new IPv6Datagram(REQUEST_MSG);
        datagram->setByteLength(IPv6_HEADER_BYTES);
        datagram->encapsulate(udpPacket);
        datagram->setSrcAddress(srcEid.get6());
        datagram->setDestAddress(dstEid.get6());
        datagram->setHopLimit(DEFAULT_IPTTL_VAL);
        datagram->setTransportProtocol(IP_PROT_UDP);
        middle = datagram;

    }
    //...else IPv4 envelope
    else {
        IPv4Datagram *datagram = new IPv4Datagram(REQUEST_MSG);
        datagram->setByteLength(IP_HEADER_BYTES);
        datagram->encapsulate(udpPacket);
        datagram->setSrcAddress(srcEid.get4());
        datagram->setDestAddress(dstEid.get4());
        datagram->setTimeToLive(DEFAULT_IPTTL_VAL);
        datagram->setTransportProtocol(IP_PROT_UDP);
        middle = datagram;
    }

    //LISPHeader
    LISPEncapsulated* lencreq = new LISPEncapsulated(ENCAPSULATEDREQUEST_MSG);
    lencreq->setByteLength(4);
    lencreq->encapsulate(middle);

    //Log and send
    MsgLog->addMsg(lmreq, LISPMsgEntry::REQUEST, MapResolverQueue->getAddress(), true);
    controlTraf.sendTo(lencreq, MapResolverQueue->getAddress(), CONTROL_PORT_VAL);

    return lmreq->getNonce();
}

void LISPCore::receiveMapRequest(LISPMapRequest* lmreq) {
    UDPDataIndication* udpi = check_and_cast<UDPDataIndication*>(lmreq->getControlInfo());
    IPvXAddress src = udpi->getSrcAddr();
    //Log
    MsgLog->addMsg(lmreq, LISPMsgEntry::REQUEST, src, true);

    //Process as MR-MS
    if ( ( this->mapResolverV4 && !src.isIPv6() ) || ( this->mapResolverV6 && src.isIPv6() ) ) {

        //Parse request(s)
        //Vesely - Current version should contain single request, thus FOR has 1 iteration
        for (TRecsCItem it = lmreq->getRecs().begin(); it != lmreq->getRecs().end(); ++it) {
            //EV << "EID> " << it->getEidAddr() << endl;
            LISPSite* si = SiteDb->findSiteByAggregate(it->getEidAddr());
            if (si) {
                //EV << "Site> " << si->info() << endl;
                Etrs res = si->findAllRecordsByEid(it->getEidAddr());
                //EV << "ResSIze> " << res.size() << endl;
                if ( !res.size() ) {
                    EV << "Site with aggregate but without specific EIDs. Sending Negative Map-Reply!" << endl;
                    sendNegativeMapReply(lmreq, *it, true);
                    continue;
                }

                //Filter retrieve records according to address family
                bool proxyReply = false;
                for (EtrItem jt = res.begin(); jt != res.end(); ++jt) {
                    //TODO: Vesely - Is it okay to erase ETR servers which have different AFI?
                    if ( (*jt).getServerEntry().getAddress().isIPv6() xor src.isIPv6() )
                        res.erase(jt++);
                    //Filter only the best EID from each ETR TODO: Vesely - What if I want more less precise mapping answers?
                    LISPMapEntry me = *( (*jt).lookupMapEntry( it->getEidAddr() ) );
                    (*jt).clearMappingStorage();
                    (*jt).addMapEntry(me);
                    if ( (*jt).getServerEntry().isProxyReply() )
                        proxyReply = true;
                }

                //TODO: Vesely - EID inconsistency check
                //      Cisco assumes that all ETRs registered same EID-to-RLOC mapping
                //Check whether all uses the same EID result. Take the first as template.
                LISPMapEntry me = res.begin()->getMappingStorage().front();
                for (EtrItem jt = res.begin(); jt != res.end(); ++jt) {
                    LISPEidPrefix pref = (*jt).getMappingStorage().front().getEidPrefix();
                    if ( ! (me.getEidPrefix() == pref) ) {
                        EV << "Precision inconsistency exists for EID " << it->getEidAddr() << endl;
                        EV << me.getEidPrefix() << " versus " << pref;
                        continue;
                    }
                }
                //Check whether all uses the same locator set. Take the first one as template.
                for (EtrItem jt = res.begin(); jt != res.end(); ++jt) {
                    if (me.getRlocs() != (*jt).getMappingStorage().front().getRlocs()) {
                        EV << "Locator inconsistency exists for EID " << it->getEidAddr() << endl;
                        EV << me.getRlocs() << endl << (*jt).getMappingStorage().front().getRlocs();
                        continue;
                    }
                }

                if (res.size()) {
                    //Check whether there are ETRs requesting proxy-reply
                    if (proxyReply) {
                        //Proxy-reply instead of authoritative ETRs
                        //FIXME: If there are multiple Recs then what about Nonces?
                        sendMapReplyFromMs(lmreq, res);
                    }
                    //...otherwise delegate Map-Request towards ETR
                    else {
                        delegateMapRequest(lmreq, res);
                    }
                }
                else {
                    EV << "No ME left in answer! Sending Negative Map-Reply!" << endl;
                    sendNegativeMapReply(lmreq, *it, true);
                    continue;
                }
            }
            else {
                EV << "No site with aggregate matching EID. Sending Negative Map-Reply!" << endl;
                sendNegativeMapReply(lmreq, *it, false);
                continue;
            }
        }

    }
    //Process as ETR (who knows the answer)
    else {
        //Parse request(s)
        for (TRecsCItem it = lmreq->getRecs().begin(); it != lmreq->getRecs().end(); ++it) {
            sendMapReplyFromEtr(lmreq, it->getEidAddr());
        }
    }
    //Cache MapReply record
    if (acceptMapRequestMapping)
        cacheMapping(lmreq->getMapReply());
}

void LISPCore::delegateMapRequest(LISPMapRequest* lmreq, Etrs& etrs) {
    //TODO: Vesely - Which ETR to use? First one, the last performing registration?
    //               Currently the first one is used!
    LISPMapRequest* copy = lmreq->dup();

    //Log and send
    IPvXAddress dst = etrs.front().getServerEntry().getAddress();
    MsgLog->addMsg(lmreq, LISPMsgEntry::REQUEST, dst, true);
    controlTraf.sendTo(copy, dst, CONTROL_PORT_VAL);
}

void LISPCore::sendMapReplyFromMs(LISPMapRequest* lmreq, Etrs& etrs) {
    LISPMapReply* lmrep = new LISPMapReply(REPLY_MSG);

    //General fields
    lmrep->setNonce( lmreq->getNonce() );
    lmrep->setProbeBit(false);
    //TODO: Vesely - Work on EchoNonceBack algorithm, currently off
    //lmrep->setEchoNonceBit(false);
    //TODO: Vesely - Work on SecurityBit fields, currently off
    //lmrep->setSecBit(false);

    //Currently the first ETR info is used because they should be all same
    //TODO: Vesely - Work on merging
    LISPMapEntry& me = etrs.front().getMappingStorage().front();

    TRecord rec = prepareTRecord(&me, false, LISPCommon::NO_ACTION);

    lmrep->getRecords().push_back(rec);
    //Record count should be 1 for Map-Reply
    lmrep->setRecordCount(lmrep->getRecords().size());

    //Count dynamic size
    lmrep->setByteLength(countPacketSize(lmrep));

    //Respond to !!!first!!! ITR-RLOC in Map-Request message TODO: Vesely - Is this ok?
    //Log and send
    IPvXAddress dst = lmreq->getItrRlocs().front().address;
    MsgLog->addMsg(lmrep, LISPMsgEntry::REPLY, dst, true);
    controlTraf.sendTo(lmrep, dst, CONTROL_PORT_VAL);
}

void LISPCore::sendMapReplyFromEtr(LISPMapRequest* lmreq, const IPvXAddress& eidAddress) {
    LISPMapReply* lmrep = new LISPMapReply(REPLY_MSG);

    //General fields
    lmrep->setNonce( lmreq->getNonce() );
    lmrep->setProbeBit(false);
    //TODO: Vesely - Work on EchoNonceBack algorithm, currently off
    //lmrep->setEchoNonceBit(false);
    //TODO: Vesely - Work on SecurityBit fields, currently off
    //lmrep->setSecBit(false);

    LISPMapEntry* me = MapDb->lookupMapEntry( eidAddress );
    if (!me)
        error("ETR does not have EID in its ETRMappings. Inconsistency between MRMS and ETR.");

    TRecord rec = prepareTRecord(me, true, LISPCommon::NO_ACTION);
    lmrep->getRecords().push_back(rec);
    lmrep->setRecordCount(lmrep->getRecords().size());

    //Count dynamic size
    lmrep->setByteLength(countPacketSize(lmrep));

    //Respond to !!!first!!! ITR-RLOC in Map-Request message TODO: Vesely - Is this ok?
    //Log and send
    IPvXAddress dst = lmreq->getItrRlocs().front().address;
    MsgLog->addMsg(lmrep, LISPMsgEntry::REPLY, dst, true);
    controlTraf.sendTo(lmrep, dst, CONTROL_PORT_VAL);
}

void LISPCore::receiveMapReply(LISPMapReply* lmrep) {
    UDPDataIndication* udpi = check_and_cast<UDPDataIndication*>(lmrep->getControlInfo());
    IPvXAddress src = udpi->getSrcAddr();
    //Log
    MsgLog->addMsg(lmrep, LISPMsgEntry::REPLY, src, false);
    if ( !MsgLog->findMsg(LISPMsgEntry::REQUEST, lmrep->getNonce()) ) {
        EV << "Received unsolicited Map-Reply with nonce " << lmrep->getNonce() << endl;
        return;
    }
    else
        EV << "Map-Reply is response to previous Map-Request with nonce " << lmrep->getNonce() << endl;

    //Iterate through records and store them into map-cache
    for (TRecordCItem it = lmrep->getRecords().begin(); it != lmrep->getRecords().end(); ++it) {
        cacheMapping(*it);
    }
}

void LISPCore::sendNegativeMapReply(LISPMapRequest* lmreq, const LISPEidPrefix& eidPref, bool hasSite) {
    LISPMapReply* lmrep = new LISPMapReply(NEGATIVEREPLY_MSG);

    //General fields
    lmrep->setNonce( lmreq->getNonce() );
    lmrep->setProbeBit( false );
    //TODO: Vesely - Work on EchoNonceBack algorithm, currently off
    lmrep->setEchoNonceBit(false);
    //TODO: Vesely - Work on SecurityBit fields, currently off
    lmrep->setSecBit(false);

    TRecord rec;
    //If aggregate exist then 1 minute TTL otherwise 15
    rec.recordTTL = hasSite ? NOETR_TTL_VAL : NOEID_TTL_VAL;
    rec.EidPrefix = eidPref;
    rec.mapVersion = DEFAULT_MAPVER_VAL;
    rec.ABit = false;
    rec.ACT = LISPCommon::NATIVELY_FORWARD;
    rec.locatorCount = 0;
    lmrep->getRecords().push_back(rec);
    //RecordCount should be 1 in case of Negative Map-Reply
    lmrep->setRecordCount(lmrep->getRecords().size());

    //Count dynamic size
    lmreq->setByteLength(countPacketSize(lmreq));

    //Respond to !!!first!!! ITR-RLOC in Map-Request message TODO: Vesely - Is this ok?
    //Log and send
    IPvXAddress dst = lmreq->getItrRlocs().front().address;
    MsgLog->addMsg(lmrep, LISPMsgEntry::NEGATIVE_REPLY, dst, true);
    controlTraf.sendTo(lmrep, dst, CONTROL_PORT_VAL);
}

unsigned long LISPCore::sendRlocProbe(IPvXAddress& dstLoc, LISPEidPrefix& eid) {
    LISPMapRequest* lmreq = new LISPMapRequest(REQUESTPROBE_MSG);
    lmreq->setDisplayString("b=15,15,oval,green,green,0");

    //General fields
    unsigned long newnonce = (unsigned long) ev.getRNG(0)->intRand();
    lmreq->setNonce(newnonce);
    //It is RLOC-Probe, hebce set the Probebit
    lmreq->setProbeBit(true);

    lmreq->setSourceEid( TAfiAddr( eid.getEidAddr() ) );

    //TODO: Vesely - What about sending more RLOCs at once?
    LISPMapEntry* me = MapDb->findMapEntryByEidPrefix(eid);
    //TODO: Vesely - If there is no ME match THEN send it without MapReply
    if (me) {
        //Prepare ITR-RLOCs
        for (LocatorCItem it = me->getRlocs().begin(); it != me->getRlocs().end(); ++it)
            if (it->isLocal())
                lmreq->getItrRlocs().push_back(TAfiAddr(it->getRlocAddr()));

        //Multiple Recs in Map-Request, TODO: Vesely - Currently it is not supported yet
        lmreq->setRecordCount(1);
        lmreq->getRecs().push_back(eid);

        //Map-Reply Record
        lmreq->setMapDataBit(true);
        TRecord rep = prepareTRecord( &(*me), true, LISPCommon::NO_ACTION );
        lmreq->setMapReply(rep);
    }
    else {
        error("Cannot send RLOC-Probe because srcEID does not have ME!");
        return DEFAULT_NONCE_VAL;
    }

    //Count dynamic size
    lmreq->setByteLength(countPacketSize(lmreq));

    //Log and send
    MsgLog->addMsg(lmreq, LISPMsgEntry::RLOC_PROBE, dstLoc, true);
    controlTraf.sendTo(lmreq, dstLoc, CONTROL_PORT_VAL);

    return lmreq->getNonce();
}

void LISPCore::receiveRlocProbe(LISPMapRequest* lmreq) {
    UDPDataIndication* udpi = check_and_cast<UDPDataIndication*>(lmreq->getControlInfo());
    IPvXAddress src = udpi->getSrcAddr();
    IPvXAddress dst = udpi->getSrcAddr();
    //Log
    MsgLog->addMsg(lmreq, LISPMsgEntry::RLOC_PROBE, src, false);

    long bitmask = 0;
    unsigned char i = 0;

    for (TRecsCItem it = lmreq->getRecs().begin(); it != lmreq->getRecs().end(); ++it) {
        LISPEidPrefix eidPref = LISPEidPrefix(it->getEidAddr(), it->getEidLength());
        LISPMapEntry* me = MapDb->findMapEntryByEidPrefix(eidPref);
        if (me) {
            if (!me->isLocatorExisting(src) || !me->isLocatorExisting(dst))
                EV << "Source RLOC "<< src << " or probed RLOC " << dst << " are not part of EID " << eidPref <<" MapEntry!" << endl;
            bitmask |= 1 << i;
        }
        else
            EV << "Received RLOC-Probe for unknown EID (none ME found)!" << endl;
        //Iterate bitmask pointer
        i++;
    }

    //Cache Map-Reply data
    if (acceptMapRequestMapping && lmreq->getMapDataBit())
        cacheMapping(lmreq->getMapReply());

    //Use bitmask
    sendRlocProbeReply(lmreq, bitmask);
}

void LISPCore::sendRlocProbeReply(LISPMapRequest* lmreq, long bitmask) {
    LISPMapReply* lmrep = new LISPMapReply(REPLYPROBE_MSG);
    lmrep->setDisplayString("b=15,15,oval,lightgreen,lightgreen,0");

    //Retrieve source IP address
    UDPDataIndication* udpi = check_and_cast<UDPDataIndication*>(lmreq->getControlInfo());
    IPvXAddress src = udpi->getSrcAddr();

    //EV << "Bitmask is: " << bitmask;
    lmrep->setNonce(lmreq->getNonce());
    //It is RLOC-Probe-Reply, hence set the Probebit
    lmrep->setProbeBit(true);

    //TODO: Vesely - Work on EchoNonceBack algorithm, currently off
    //lmrep->setEchoNonceBit(false);
    //TODO: Vesely - Work on SecurityBit fields, currently off
    //lmrep->setSecBit(false);

    unsigned char i = 0;
    for (TRecsCItem it = lmreq->getRecs().begin(); it != lmreq->getRecs().end(); ++it) {
        if (! (bitmask & (1 << i) ) )
           continue;

        LISPMapEntry* me = MapDb->findMapEntryByEidPrefix(*it);

        TRecord rec = prepareTRecord(me, true, LISPCommon::NO_ACTION);
        lmrep->getRecords().push_back(rec);
        lmrep->setRecordCount(lmrep->getRecords().size());

        i++;
    }

    //Count dynamic size
    lmrep->setByteLength(countPacketSize(lmrep));

    //Log and send
    MsgLog->addMsg(lmrep, LISPMsgEntry::RLOC_PROBE_REPLY, src, true);
    controlTraf.sendTo(lmrep, src, CONTROL_PORT_VAL);
}

void LISPCore::receiveRlocProbeReply(LISPMapReply* lmrep) {
    UDPDataIndication* udpi = check_and_cast<UDPDataIndication*>(lmrep->getControlInfo());
    IPvXAddress src = udpi->getSrcAddr();
    //Log
    MsgLog->addMsg(lmrep, LISPMsgEntry::RLOC_PROBE_REPLY, src, false);
    if ( !MsgLog->findMsg(LISPMsgEntry::RLOC_PROBE, lmrep->getNonce()) ) {
        EV << "Received unsolicited RLOC-Probe Reply with nonce " << lmrep->getNonce() << endl;
        return;
    }
    else
        EV << "RLOC-Probe-Reply is response to previous Probe with nonce " << lmrep->getNonce() << endl;

    //Mark RLOC as up
    for (TRecordCItem it = lmrep->getRecords().begin(); it != lmrep->getRecords().end(); ++it) {
        LISPProbeEntry* probe =
                        ProbingSet.findProbeEntryByRlocAndEid(src, it->EidPrefix);
        if (!probe)
            EV << "RLOC " << src << " does not exist in ProbeSet" << endl;
        else if ( !strcmp(par(RLOCPROBINGALGO_PAR).stringValue(), ALGO_EIDGROUPED_PARVAL) )
            probe->setRlocStatusForEid(it->EidPrefix, LISPRLocator::UP);
        else
            probe->setRlocStatusForAllEids(LISPRLocator::UP);

        if (acceptMapRequestMapping)
            cacheMapping(*it);
    }
}

unsigned long LISPCore::sendCacheSync(IPvXAddress& setmember, LISPEidPrefix& eidPref) {
    //EV << "sendCacheSync() - " << setmember << "   EID: " << eidPref << endl;
    //if (eidPref.getEidAddr().isUnspecified()) {
    //    EV << "Cannot send CacheSync containing default MapCache record!" << endl;
    //    return DEFAULT_NONCE_VAL;
    //}

    LISPCacheSync* lcs = new LISPCacheSync(CACHESYNC_MSG);

    //General fields
    unsigned long newnonce = (unsigned long) ev.getRNG(0)->intRand();
    lcs->setNonce(newnonce);

    //Authentication data
    lcs->setKeyId(LISPCommon::KID_NONE);
    lcs->setAuthDataLen(MapCache->getSyncKey().size());
    lcs->setAuthData(MapCache->getSyncKey().c_str());

    lcs->setAckBit(MapCache->isSyncAck() ? true : false);

    if (MapCache->getSyncType() == LISPMapCache::SYNC_NAIVE || eidPref.getEidAddr().isUnspecified()) {
        //TRecords
        for (MapStorageItem it = MapCache->getMappingStorage().begin();
                it != MapCache->getMappingStorage().end(); ++it) {
            if (it->getEidPrefix().getEidAddr().isUnspecified())
                continue;
            lcs->getMapEntries().push_back(*it);
        }
    } else if (MapCache->getSyncType() == LISPMapCache::SYNC_SMART) {
        LISPMapEntry* me = MapCache->findMapEntryByEidPrefix(eidPref);
        if (me)
            lcs->getMapEntries().push_back(*me);
        else
            error("ME seizes to exist between method calls!");
    }
    else {
        EV << "MapCache is not configured with supported synchronization capability!" << endl;
        return DEFAULT_NONCE_VAL;
    }

    //As many records as there are EtrMappings
    lcs->setRecordCount(lcs->getMapEntries().size());

    //Count dynamic size
    lcs->setByteLength(countPacketSize(lcs));

    //Log and send
    MsgLog->addMsg(lcs, LISPMsgEntry::CACHE_SYNC, setmember, true);
    controlTraf.sendTo(lcs, setmember, CONTROL_PORT_VAL);

    return lcs->getNonce();
}

void LISPCore::receiveCacheSync(LISPCacheSync* lcs) {
    //Log
    UDPDataIndication* udpi = check_and_cast<UDPDataIndication*>(lcs->getControlInfo());
    IPvXAddress src = udpi->getSrcAddr();
    MsgLog->addMsg(lcs, LISPMsgEntry::CACHE_SYNC, src, false);

    //Check for 0 records
    if (lcs->getRecordCount() == 0) {
        EV << "Cache-Sync contains zero records!" << endl;
        return;
    }

    //Check the authentication method
    if (lcs->getKeyId() != 0) {
        EV << "Cache-Sync contains unsupported authentication method!" << endl;
        return;
    }

    //Check authentication data
    std::string authData = lcs->getAuthData();
    if (MapCache->getSyncKey() != authData) {
        EV << "Password " << authData << " in Cache-Sync does not matched configured one " << MapCache->getSyncKey() << endl;
        return;
    }

    //Check whether map cache synchronization is enabled
    if (MapCache->getSyncType() == LISPMapCache::SYNC_NONE) {
        EV << "MapCache is not configured with synchronization capability!" << endl;
        return;
    }

    if (MapCache->getMappingStorage().size() != lcs->getRecordCount()) {
        EV << "State prior to synchronization:\n\tLocal MapCache: "
           << MapCache->getMappingStorage().size() << " entries, CacheSync: "
           << (unsigned short) lcs->getRecordCount() << " entries." << endl;
    }

    for (TMEItem it = lcs->getMapEntries().begin(); it != lcs->getMapEntries().end(); ++it)
        MapCache->syncCacheEntry(it->MapEntry);

}

void LISPCore::sendCacheSyncAck(LISPCacheSync* lcs) {
    LISPCacheSyncAck* lca = new LISPCacheSyncAck(CACHESYNCACK_MSG);

    UDPDataIndication* udpi = check_and_cast<UDPDataIndication*>(lcs->getControlInfo());
    IPvXAddress dst = udpi->getSrcAddr();

    //Copy Map-Register content onto Map-Notify
    lca->setNonce(lcs->getNonce());
    lca->setRecordCount(lcs->getRecordCount());
    lca->setKeyId(lcs->getKeyId());
    lca->setAuthDataLen(lcs->getAuthDataLen());
    lca->setAuthData(lcs->getAuthData());
    lca->setMapEntries(lcs->getMapEntries());

    //Count dynamic size
    lca->setByteLength(countPacketSize(lca));

    //Log and send
    MsgLog->addMsg(lca, LISPMsgEntry::CACHE_SYNC_ACK, dst, true);
    controlTraf.sendTo(lca, dst, CONTROL_PORT_VAL);
}

void LISPCore::receiveCacheSyncAck(LISPCacheSyncAck* lca) {
    //Retrieve source IP address
    UDPDataIndication* udpi = check_and_cast<UDPDataIndication*>(lca->getControlInfo());
    IPvXAddress src = udpi->getSrcAddr();

    //Log
    MsgLog->addMsg(lca, LISPMsgEntry::CACHE_SYNC_ACK, src, false);

    //Check whether last Map-Register was acknowledged
    if ( MsgLog->findMsg(LISPMsgEntry::CACHE_SYNC, lca->getNonce() ) ) {
        EV << "Cache-Sync was acknowledged by Cache-Sync-Ack with nonce " << lca->getNonce() << endl;
    }
    else
        EV << "Unsolicited Cache-Sync-Ack received!" << endl;
}

bool LISPCore::isOneOfMyEids(IPvXAddress addr) {
    return MapDb->isOneOfMyEids(addr) ;
}

void LISPCore::cacheMapping(const TRecord& record) {
    //Insert into mapcache
    MapCache->updateCacheEntry(record);
    //Schedule cache synchronization procedure
    scheduleCacheSync(record.EidPrefix);
}

void LISPCore::initSignals() {
    sigFrwd     = registerSignal(SIG_PACKET_FORWARD);
    sigDrop     = registerSignal(SIG_PACKET_DROP);

    DownListener = new LISPDownLis(MapCache);
    this->getParentModule()->getParentModule()->subscribe("SIG-IFACEDOWN", DownListener);

    UpListener = new LISPUpLis(MapCache, this);
    this->getParentModule()->getParentModule()->getParentModule()->subscribe("SIG-PEERUP", UpListener);


}

void LISPCore::updateStats(bool flag) {
    if (flag) {
        packetfrwd++;
        emit(sigFrwd, true);
    }
    else {
        packetdrop++;
        emit(sigDrop, true);
    }
    updateDisplayString();
}

unsigned long LISPCore::sendEncapsulatedDataMessage(IPvXAddress srcaddr, IPvXAddress dstaddr, LISPMapEntry* mapentry, cPacket* packet) {

    //LISPHeader
    LISPHeader* lidata = new LISPHeader(DATA_MSG);

    //TODO: Vesely - Dealing with EchoNonceAlgorithm
    if (echoNonceAlgo) {
        //General fields
        unsigned long newnonce = (unsigned long) ev.getRNG(0)->intRand();
        lidata->setNonce(newnonce);
        lidata->setNonceBit(true);
        lidata->setEchoNonceBit(true);
    }
    else {
        lidata->setNonceBit(false);
        lidata->setEchoNonceBit(false);
        lidata->setNonce(DEFAULT_NONCE_VAL);
    }

    //Envelope around original IP packet
    lidata->setByteLength(countPacketSize(lidata));
    lidata->encapsulate(packet);

    //Retrieve the best RLOC
    IPvXAddress rlocaddr = mapentry->getBestUnicastLocator()->getRlocAddr();
    //EV << "Chosen RLOC addr is>" << rlocaddr << endl;

    //Data packets are omitted from msg logging
    //MsgLog->addMsg(lidata, LISPMsgEntry::DATA, rlocaddr, true);

    dataTraf.sendTo(lidata, rlocaddr, DATA_PORT_VAL);

    return lidata->getNonce();
}

int64 LISPCore::countPacketSize(LISPMessage* lispmsg) {
    if (dynamic_cast<LISPHeader*>(lispmsg))
        return (8);
    else if (dynamic_cast<LISPMapRequest*>(lispmsg) && !dynamic_cast<LISPEncapsulated*>(lispmsg)) {
        LISPMapRequest* lmreq = check_and_cast<LISPMapRequest*>(lispmsg);
        return (16 + getItrRlocSize(lmreq->getItrRlocs()) + getRecsSize(lmreq->getRecs()) + getRecordSize(lmreq->getMapReply()));
    }
    else if (dynamic_cast<LISPMapReply*>(lispmsg)) {
        LISPMapReply* lmrep = check_and_cast<LISPMapReply*>(lispmsg);
        return (12 + getRecordsSize(lmrep->getRecords()));
    }
    else if (dynamic_cast<LISPMapNotify*>(lispmsg)
            || dynamic_cast<LISPMapRegister*>(lispmsg)) {
        LISPMapRegister* lmreg = check_and_cast<LISPMapRegister*>(lispmsg);
        return (20 + getRecordsSize(lmreg->getRecords()));
    }
    else if (dynamic_cast<LISPCacheSyncAck*>(lispmsg)
            || dynamic_cast<LISPCacheSync*>(lispmsg)) {
        LISPCacheSyncAck* lcs = check_and_cast<LISPCacheSyncAck*>(lispmsg);
        return (20 + getLispMapEntrySize(lcs->getMapEntries()));
    }
    return 0;
}

unsigned int LISPCore::getItrRlocSize(TAfiAddrs& Itrs) {
    unsigned int result = 0;
    for (TAfiAddrCItem it = Itrs.begin(); it != Itrs.end(); ++it) {
        result += (it->afi() == LISPCommon::AFI_IPV6 ? 16 : 4);
    }
    return result;
}

unsigned int LISPCore::getRecsSize(TRecs& Recs) {
    unsigned int result = 0;
    for (TRecsCItem it = Recs.begin(); it != Recs.end(); ++it) {
        result += 4 + (it->getEidAfi() == LISPCommon::AFI_IPV6 ? 16 : 4);
    }
    return result;
}

unsigned int LISPCore::getRecordSize(TRecord& Record) {
    unsigned int result = 3;
    result += Record.EidPrefix.getEidAfi() == LISPCommon::AFI_IPV6 ? 16 : 4;
    for (TLocatorCItem it = Record.locators.begin(); it != Record.locators.end(); ++it) {
        result += 8 + (it->RLocator.getRlocAfi() == LISPCommon::AFI_IPV6 ? 16 : 4);
    }
    return result;
}

unsigned int LISPCore::getRecordsSize(TRecords& Records) {
    unsigned int result = 0;
    for (TRecordItem it = Records.begin(); it != Records.end(); ++it) {
        result += getRecordSize((*it));
    }
    return result;
}

unsigned int LISPCore::getLispMapEntrySize(TMapEntries& MapEntries) {
    unsigned int result = 0;
    for (TMECItem it = MapEntries.begin(); it != MapEntries.end(); ++it) {
        result += 3;
        result += it->MapEntry.getEidPrefix().getEidAfi() == LISPCommon::AFI_IPV6 ? 16 : 4;
        for (LocatorCItem jt = it->MapEntry.getRlocs().begin(); jt != it->MapEntry.getRlocs().end(); ++jt) {
            result += 8 + (jt->getRlocAfi() == LISPCommon::AFI_IPV6 ? 16 : 4);
        }
    }
    return result;
}

