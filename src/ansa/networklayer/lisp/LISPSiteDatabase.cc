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

#include "LISPSiteDatabase.h"

Define_Module(LISPSiteDatabase);

void LISPSiteDatabase::initialize(int stage)
{
    if (stage < numInitStages() - 1)
        return;

    //Import site database
    parseConfig( par(CONFIG_PAR).xmlValue() );

    updateDisplayString();

    //Display in simulation
    WATCH_LIST(SiteDatabase);
}

void LISPSiteDatabase::updateEtrEntries(LISPSiteRecord* siteRec, const TRecord& trec, std::string name) {
    Enter_Method("updateSiteEtr()");

    //Update record
    siteRec->updateMapEntry(trec);

    //Update timeout
    updateTimeout(siteRec->getServerEntry().getAddress(), name);

    updateDisplayString();
}

void LISPSiteDatabase::updateTimeout(IPvXAddress addr, std::string name) {
    LISPEtrTimer* etrtim = findExpirationTimer(addr, name);
    //If ME did not existed
    if (!etrtim) {
        LISPEtrTimer* etrtim = new LISPEtrTimer(ETRTIMEOUT_TIMER);
        etrtim->setEtrAddr(addr);
        etrtim->setSiteName(name.c_str());
        SiteTimeouts.push_back(etrtim);
        scheduleAt(simTime() + DEFAULT_ETRTIMEOUT_VAL, etrtim);
    }
    //...update expiration timer
    else {
        cancelEvent(etrtim);
        scheduleAt(simTime() + DEFAULT_ETRTIMEOUT_VAL, etrtim);
    }
}

LISPSite* LISPSiteDatabase::findSiteInfoBySiteName(std::string& siteName) {
    for (SiteStorageItem it = SiteDatabase.begin(); it != SiteDatabase.end(); ++it) {
        if ( !it->getSiteName().compare(siteName) )
            return &(*it);
    }
    return NULL;
}

LISPEtrTimer* LISPSiteDatabase::findExpirationTimer(IPvXAddress addr, std::string name) {
    for (SiteTimeoutItem it = SiteTimeouts.begin(); it != SiteTimeouts.end(); ++it) {
        if ( (*it)->getEtrAddr().equals(addr) && !name.compare((*it)->getSiteName())  )
            return *it;
    }
    return NULL;
}

void LISPSiteDatabase::handleMessage(cMessage *msg)
{
    //Receive Map-Entry expiration timer
    if (  dynamic_cast<LISPEtrTimer*>(msg) ) {
        LISPEtrTimer* etrtim = check_and_cast<LISPEtrTimer*>(msg);

        //Find ME where EidPref should be unique key
        std::string nam = etrtim->getSiteName();
        LISPSite* si = findSiteInfoBySiteName(nam);
        si->removeRecord( *( si->findRecordByAddress(etrtim->getEtrAddr()) ) );

        //Remove timer
        SiteTimeouts.remove(etrtim);

        delete msg;
    }
    //...otherwise generate error
    else {
        error("Site-Database cannot receive other messages than ETR-timeouts!");
    }
}

LISPSite* LISPSiteDatabase::findSiteInfoByKey(std::string& siteKey) {
    for (SiteStorageItem it = SiteDatabase.begin(); it != SiteDatabase.end(); ++it) {
        if ( !it->getKey().compare(siteKey) )
            return &(*it);
    }
    return NULL;
}

LISPSite* LISPSiteDatabase::findSiteByAggregate(const IPvXAddress& addr) {
    LISPSite* res = NULL;
    int tmplen = 0;
    for (SiteStorageItem it = SiteDatabase.begin(); it != SiteDatabase.end(); ++it) {
        LISPMapEntry* me = it->lookupMapEntry(addr);
        if ( me ) {
            int len = LISPCommon::doPrefixMatch(me->getEidPrefix().getEidAddr(), addr);
            if (len >= tmplen) {
                tmplen = len;
                res = &(*it);
            }
        }
    }
    return res;
}

void LISPSiteDatabase::parseConfig(cXMLElement* config) {
    if (!config)
        return;

    if ( opp_strcmp(config->getTagName(), MAPSERVER_TAG) )
        config = config->getFirstChildWithTag(MAPSERVER_TAG);

    if (!config)
            return;

    cXMLElementList map = config->getChildrenByTagName(SITE_TAG);

    for (cXMLElementList::iterator i = map.begin(); i != map.end(); ++i) {
        cXMLElement* m = *i;

        //Check integrity
        if (!m->getAttribute(NAME_ATTR) || !m->getAttribute(KEY_ATTR)) {
            EV << "Config XML file missing tag or attribute - NAME/KEY" << endl;
            continue;
        }

        //Parse Site tag
        std::string nam = m->getAttribute(NAME_ATTR);
        std::string ke  = m->getAttribute(KEY_ATTR);

        if (!nam.empty() && ! ke.empty()) {
            //Create new SiteInfo entry
            LISPSite site = LISPSite(nam, ke);

            //Parse potential EIDs
            site.parseMapEntry(m);

            //Add entry to SiteStorage
            this->addSite(site);
        }
    }
}

void LISPSiteDatabase::addSite(LISPSite& si)
{
    SiteDatabase.push_back(si);
}

LISPSiteRecord* LISPSiteDatabase::updateSiteEtr(LISPSite* si, IPvXAddress src, bool proxy) {
    Enter_Method_Silent();
    LISPSiteRecord* srec;
    LISPServerEntry* se;
    //Create new record ...
    if ( !si->findRecordByAddress(src) ) {
        srec = new LISPSiteRecord();
        se = new LISPServerEntry(src.str(), EMPTY_STRING_VAL, proxy, false, false);
        srec->setServerEntry(*se);
        //Store new record into SiteStorage
        si->addRecord(*srec);
    }
    //...and/or retrieve record
    srec = si->findRecordByAddress(src);

    srec->getServerEntry().setLastTime(simTime());
    srec->getServerEntry().setProxyReply(proxy);

    return srec;
}

void LISPSiteDatabase::updateDisplayString() {
    std::ostringstream description;
    description << SiteDatabase.size() << " sites" << endl
                << SiteTimeouts.size() << " ETRs";
    this->getDisplayString().setTagArg("t", 0, description.str().c_str());
    this->getDisplayString().setTagArg("t", 1, "t");
}
