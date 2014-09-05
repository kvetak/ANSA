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

#include "LISPCore.h"

Define_Module(LISPCore);

LISPCore::LISPCore()
{

}
LISPCore::~LISPCore()
{
    //mss->clear();
    //mrs->clear();
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

void LISPCore::parseConfig(cXMLElement* config)
{
    //Config element is empty
    if (!config)
        return;

    //Map server addresses initial setup
    cXMLElementList msa = config->getChildrenByTagName(MAPSERVERADDR_TAG);
    for (cXMLElementList::iterator i = msa.begin(); i != msa.end(); ++i) {
        cXMLElement* m = *i;
        //EV << "IPv4: " << m->getAttribute("ipv4") << "\tIPv6: " << m->getAttribute("ipv6");

        //Integrity checks and retrieving ipv4 attribute value
        std::string mipv4;
        if (!m->getAttribute(IPV4_ATTR)) {
            EV << "Config XML file missing tag or attribute - IPv4 - replaced by empty" << endl;
            mipv4 = EMPTY_STRING_VAL;
        }
        else
            mipv4 = m->getAttribute(IPV4_ATTR);

        //Integrity checks and retrieving ipv6 attribute value
        std::string mipv6;
        if (!m->getAttribute(IPV6_ATTR)) {
            EV << "Config XML file missing tag or attribute - IPv6 - replaced by empty" << endl;
            mipv6 = EMPTY_STRING_VAL;
        }
        else
            mipv6 = m->getAttribute(IPV6_ATTR);

        //Integrity checks and retrieving ipv6 attribute value
        if (!m->getAttribute(KEY_ATTR)) {
            EV << "Config XML file missing tag or attribute - KEY" << endl;
            continue;
        }
        std::string mk = m->getAttribute(KEY_ATTR);

        if ( (!mipv4.empty() || !mipv6.empty()) && !mk.empty())
            MapServers.push_back(LISPServerEntry(mipv4, mipv6, mk));
        else
            EV << "Skipping XML MapServerAddress configuration" << endl;
    }

    //Map resolver addresses initial setup
    cXMLElementList mra = config->getChildrenByTagName(MAPRESOLVERADDR_TAG);
    for (cXMLElementList::iterator i = mra.begin(); i != mra.end(); ++i) {
        cXMLElement* m = *i;
        //EV << "IPv4: " << m->getAttribute("ipv4") << "\tIPv6: " << m->getAttribute("ipv6");

        //Integrity checks and retrieving ipv4 attribute value
        std::string mipv4;
        if (!m->getAttribute(IPV4_ATTR)) {
            EV << "Config XML file missing tag or attribute - IPv4 - replaced by empty" << endl;
            mipv4 = EMPTY_STRING_VAL;
        }
        else
            mipv4 = m->getAttribute(IPV4_ATTR);

        //Integrity checks and retrieving ipv6 attribute value
        std::string mipv6;
        if (!m->getAttribute(IPV6_ATTR)) {
            EV << "Config XML file missing tag or attribute - IPv6 - replaced by empty" << endl;
            mipv6 = EMPTY_STRING_VAL;
        }
        else
            mipv6 = m->getAttribute(IPV6_ATTR);

        if ( !mipv4.empty() || !mipv6.empty() )
            MapResolvers.push_back(LISPServerEntry(mipv4,  mipv6));
        else
            EV << "Skipping XML MapResolverAddress configuration" << endl;
    }

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

    //EtrMappings
    cXMLElement* etrm = config->getFirstChildWithTag(ETRMAP_TAG);
    if (etrm)
        this->EtrMapping.parseMapEntry(etrm);
}

void LISPCore::registerSite() {
    for (ServerItem it = MapServers.begin(); it != MapServers.end(); ++it)
        sendMapRegister(*it);
}

void LISPCore::initialize(int stage)
{
    if (stage < 3)
        return;
    // access to the routing and interface table
    Rt = AnsaRoutingTableAccess().get();
    Ift = InterfaceTableAccess().get();
    // local MapCache
    MapCache =  ModuleAccess<LISPMapCache>(MAPCACHE_MOD).get();

    parseConfig(par(CONFIG_PAR).xmlValue());

    //MapDb pointer when needed
    bool hasMapDB = this->getParentModule()->par(HASMAPDB_PAR).boolValue();
    if (hasMapDB)
        MapDb = ModuleAccess<LISPMapDatabase>(MAPDB_MOD).get();

    //If node is acting as MS and it has not MapDB then throw error
    if ( !hasMapDB && (mapServerV4 || mapServerV6)  ) {
        throw cException("Cannot act as Map Server without Mapping Database!");
    }

    //Acts as ETR register for LISP sites
    if (!MapServers.empty())
        scheduleAt(simTime(), new cMessage(REGISTER_TIMER));

    initSockets();

    //Watchers
    WATCH_LIST(MapResolvers);
    WATCH_LIST(MapServers);
    WATCH_LIST(EtrMapping.getMappingStorage());
}

void LISPCore::handleTimer(cMessage* msg) {
    //Register timer expires
    if ( !strcmp(msg->getName(), REGISTER_TIMER) ) {
        registerSite();
        //Reschedule timer to execute after next 60 seconds
        scheduleAt(simTime() + 60, msg);
    }
    else {
        error("Unrecognized timer (%s)", msg->getClassName());
        cancelAndDelete(msg);
    }
}

void LISPCore::handleCotrol(cMessage* msg) {
    EV << "Control" << endl;
    if ( dynamic_cast<LISPMapRegister*>(msg) ) {
        receiveMapRegister(dynamic_cast<LISPMapRegister*>(msg));
    }
    delete msg;
}

void LISPCore::handleData(cMessage* msg) {
}

void LISPCore::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
        handleTimer(msg);
    else if (msg->arrivedOn("ipv4In") || msg->arrivedOn("ipv6In"))
        handleData(msg);
    else if (msg->arrivedOn("udpIn"))
        handleCotrol(msg);
    else {
        error("Unrecognized message (%s)", msg->getClassName());
        delete msg;
    }
}

void LISPCore::updateDisplayString()
{
    if (ev.isGUI())
    {
        //TODO
    }
}

void LISPCore::initSockets() {
    controlTraf.setOutputGate(gate("udpOut"));
    controlTraf.setReuseAddress(true);
    controlTraf.bind(CONTROL_PORT);
}

void LISPCore::sendMapRegister(LISPServerEntry& se) {
    if (se.getIpv4() == IPv4Address::UNSPECIFIED_ADDRESS && se.getIpv6() == IPv6Address::UNSPECIFIED_ADDRESS )
        return;

    LISPMapRegister* lmr = new LISPMapRegister(REGISTER_MSG);

    //General fields
    lmr->setNonce(DEFAULT_NONCE_VAL);
    lmr->setRecordCount(EtrMapping.getMappingStorage().size());

    //Authentication data
    lmr->setKeyId(LISPCommon::NONE);
    lmr->setAuthDataLen(se.getKey().size());
    lmr->setAuthData(se.getKey().c_str());

    //TRecords
    for (MapStorageCItem it = EtrMapping.getMappingStorage().begin(); it != EtrMapping.getMappingStorage().end(); ++it) {
        TRecord rec;
        rec.recordTTL = DEFAULT_TTL_VAL;
        rec.EidPrefix = it->getEidPrefix();
        rec.mapVersion = DEFAULT_MAPVER_VAL;
        rec.ABit = true;
        rec.ACT = LISPCommon::NO_ACTION;
        rec.locatorCount = it->getRlocs().size();
        for (LocatorCItem jt = it->getRlocs().begin(); jt != it->getRlocs().end(); ++jt) {
            TLocator loc;
            loc.RLocator = *jt;
            loc.LocalLocBit = true;
            loc.piggybackBit = false;
            loc.RouteRlocBit = true;
            rec.locators.push_back(loc);
        }
        lmr->getRecords().push_back(rec);
    }

    if (se.getIpv4() != IPv4Address::UNSPECIFIED_ADDRESS)
        controlTraf.sendTo(lmr, se.getIpv4() , CONTROL_PORT);
    if (se.getIpv6() != IPv6Address::UNSPECIFIED_ADDRESS) {
        LISPMapRegister* lmrcopy = lmr->dup();
        controlTraf.sendTo(lmrcopy, se.getIpv6() , CONTROL_PORT);
    }
}

void LISPCore::receiveMapRegister(LISPMapRegister* lmr) {
    //Check whether device has MapDB
    if (!mapServerV4 && !mapServerV6) {
        EV << "non-MS device received Map-Register!" << endl;
        return;
    }

    //Check for 0 records
    if (lmr->getRecordCount() == 0) {
        EV << "Map-Register contained zero records!" << endl;
        return;
    }

    //Check the authentication method
    if (lmr->getKeyId() != 0) {
        EV << "Map-Register contained unsupported authentication method!" << endl;
        return;
    }
    std::string authData = lmr->getAuthData();
    LISPSiteInfo* si = MapDb->findSiteInfoByKey(authData);
    if (si) {
        //Set Proxy-reply settings
        si->setProxyReply(lmr->getProxyBit());
        for (TRecordCItem it = lmr->getRecords().begin(); it != lmr->getRecords().end(); ++it) {
            //EID found in MS database
            LISPMapEntry* me = si->findMapEntryByEidPrefix(it->EidPrefix);
            if (me) {

                me->setRegistredBy(lmr->getSenderModule()->getParentModule()->getName());
                me->setLastTime(simTime());
                for (TLocatorCItem jt = it->locators.begin(); jt != it->locators.end(); ++jt) {
                    LISPRLocator rloc = jt->RLocator;
                    rloc.setState(LISPRLocator::UP);
                    me->addLocator(rloc);
                }
            }
        }
    }



}
