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


#include "LISPMapStorageBase.h"

LISPMapStorageBase::LISPMapStorageBase() {}

LISPMapStorageBase::~LISPMapStorageBase()
{
    this->clearMappingStorage();
}

void LISPMapStorageBase::clearMappingStorage() {
    MappingStorage.clear();
}

void LISPMapStorageBase::addMapEntry(LISPMapEntry& entry) {
    MappingStorage.push_front(entry);
    MappingStorage.sort();
}

void LISPMapStorageBase::removeMapEntry(const LISPMapEntry& entry) {
    MappingStorage.remove(entry);
}

LISPMapEntry* LISPMapStorageBase::findMapEntryByEidPrefix(const LISPEidPrefix& eidpref) {
    for (MapStorageItem it = MappingStorage.begin(); it != MappingStorage.end(); ++it) {
        if (it->getEidPrefix() == eidpref)
            return &(*it);
    }
    return NULL;
}

LISPMapEntry* LISPMapStorageBase::lookupMapEntry(IPvXAddress address) {
    LISPMapEntry* tmpit;
    int tmplen = 0;

    for (MapStorageItem it = MappingStorage.begin(); it != MappingStorage.end(); ++it) {
        int len = LISPCommon::doPrefixMatch(it->getEidPrefix().getEidAddr(), address);
        //EV << "Len je " << len << endl;
        if (len > tmplen && len >= it->getEidPrefix().getEidLength() ) {
            //EV << "Hledam " << address << " a nachazim " << it->getEidPrefix() << endl;
            tmpit = &(*it);
            tmplen = len;
        }
    }

    if (tmplen > 0)
        return tmpit;
    return NULL;
}

std::string LISPMapStorageBase::info() const {
    std::stringstream os;
    for (MapStorageCItem it = MappingStorage.begin(); it != MappingStorage.end(); ++it)
        os << it->info() << endl;
    return os.str();
}

std::ostream& operator <<(std::ostream& os, const LISPMapStorageBase& msb) {
    return os << msb.info();
}

void LISPMapStorageBase::parseMapEntry(cXMLElement* config) {
    if (!config)
            return;
    //Fill cache with static records
    cXMLElementList map = config->getChildrenByTagName(EID_TAG);
    for (cXMLElementList::iterator i = map.begin(); i != map.end(); ++i) {
        cXMLElement* m = *i;

        //Check integrity
        if (!m->getAttribute(ADDRESS_ATTR)) {
            EV << "Config XML file missing tag or attribute - ADDRESS" << endl;
            continue;
        }



        //Parse EID address
        std::string addr;
        std::string leng;
        LISPCommon::parseIpAddress(m->getAttribute(ADDRESS_ATTR), addr, leng);

        LISPEidPrefix pref = LISPEidPrefix(addr.c_str(), leng.c_str());
        LISPMapEntry me = LISPMapEntry(pref);

        //Parse RLOCs
        cXMLElementList loc = m->getChildrenByTagName(RLOC_TAG);
        for (cXMLElementList::iterator j = loc.begin(); j != loc.end(); ++j) {
            cXMLElement* n = *j;

            //Check ADDRESS integrity
            if (!n->getAttribute(ADDRESS_ATTR)) {
                EV << "Config XML file missing tag or attribute - ADDRESS" << endl;
                continue;
            }
            std::string rlocaddr = n->getAttribute(ADDRESS_ATTR);

            //Check PRIORITY integrity
            std::string pri;
            if (n->getAttribute(PRIORITY_ATTR))
                pri = n->getAttribute(PRIORITY_ATTR);
            else {
                EV << "Config XML file missing tag or attribute - PRIORITY" << endl;
                pri = "";
            }

            //Check WEIGHT integrity
            std::string wei;
            if (n->getAttribute(WEIGHT_ATTR))
                wei = n->getAttribute(WEIGHT_ATTR);
            else {
                EV << "Config XML file missing tag or attribute - WEIGHT" << endl;
                wei = "";
            }

            //Check LOCAL integrity
            bool local = false;
            if (n->getAttribute(LOCAL_ATTR)) {
                if (!strcmp(n->getAttribute(LOCAL_ATTR), ENABLED_VAL))
                    local = true;
            }

            //Add RLOC if parsed successfully
            if (!rlocaddr.empty())
            {
                LISPRLocator locator;
                if (!pri.empty() && !wei.empty()) {
                    locator = LISPRLocator(rlocaddr.c_str(), pri.c_str(), wei.c_str(), local);
                    me.addLocator(locator);
                }
                else {
                    locator = LISPRLocator(rlocaddr.c_str());
                    me.addLocator(locator);
                }
            }
        }

        //Create a new record
        this->addMapEntry(me);
    }
}

MapStorage& LISPMapStorageBase::getMappingStorage() {
    return MappingStorage;
}

LISPMapEntry* LISPMapStorageBase::findMapEntryFromByLocator(const IPvXAddress& rloc, const LISPEidPrefix& eidPref) {
    MapStorageItem startme;
    if (eidPref == LISPEidPrefix() || eidPref == MappingStorage.back().getEidPrefix() )
        startme = MappingStorage.begin();
    else {
        for (MapStorageItem it = MappingStorage.begin(); it != MappingStorage.end(); ++it) {
            if (it->getEidPrefix() == eidPref) {
                startme = it;
                break;
            }
        }
    }

    for (MapStorageItem it = startme; it != MappingStorage.end(); ++it) {
        if (it->isLocatorExisting(rloc))
            return &(*it);
    }
    return NULL;
}

bool LISPMapStorageBase::updateMapEntry(const TRecord& rec) {
    bool isnew = false;
    LISPMapEntry* entry;
    //Create new MapEntry ...
    if ( !findMapEntryByEidPrefix(rec.EidPrefix) ) {
        //IF new ME has not EID supported THEN skip
        entry = new LISPMapEntry(rec.EidPrefix);
        isnew = true;
        addMapEntry(*entry);
    }
    //... and/or ... retrieve MapEntry
    entry = findMapEntryByEidPrefix(rec.EidPrefix);

    //Update MapEntry parameters
    entry->setExpiry(simTime() + rec.recordTTL * 60);

    //Update action
    entry->setAction( (LISPCommon::EAct)rec.ACT );

    //TODO: Abit is not processed. Authoritative reply is same as unauthoritative

    //Ordinary Map-Reply
    if (rec.locatorCount) {
        //Fill it with EID-to-RLOC mappings
        for (TLocatorCItem jt = rec.locators.begin(); jt != rec.locators.end(); ++jt) {
            LISPRLocator rloc = jt->RLocator;
            if (!entry->isLocatorExisting(rloc.getRlocAddr()))
                entry->addLocator(rloc);

            //Update state
            LISPRLocator* rl = entry->getLocator(rloc.getRlocAddr());
            rl->updateRlocator(jt->RLocator);
            if (jt->RouteRlocBit)
                rl->setState(LISPRLocator::UP);
        }
        entry->getRlocs().sort();
    }
    else {
        EV << "Record for EID " << rec.EidPrefix << " has zero locators!";
    }
    return isnew;
}

bool LISPMapStorageBase::syncMapEntry(LISPMapEntry& mapentry) {
    bool isnew = false;
    LISPMapEntry* entry = findMapEntryByEidPrefix(mapentry.getEidPrefix());
    //Create new MapEntry ...
    if ( !entry ) {
        isnew = true;
        addMapEntry(mapentry);
    }
    //...or ... retrieve MapEntry
    else {
        //Update MapEntry parameters
        entry->setExpiry(mapentry.getExpiry());
        //Update action
        entry->setAction(mapentry.getAction());

        //Ordinary Map-Reply
        if (mapentry.getRlocs().size()) {
            //Fill it with EID-to-RLOC mappings
            for (LocatorCItem jt = mapentry.getRlocs().begin(); jt != mapentry.getRlocs().end(); ++jt) {
                LISPRLocator rloc = *jt;
                if (!entry->isLocatorExisting(rloc.getRlocAddr()))
                    entry->addLocator(rloc);
                else {
                    //Update state
                    LISPRLocator* rl = entry->getLocator(rloc.getRlocAddr());
                    rl->updateRlocator(rloc);
                }
            }
            mapentry.getRlocs().sort();
        }
        else {
            EV << "Record for EID " << mapentry.getEidPrefix() << " has zero locators!";
        }
    }

    return isnew;
}

std::ostream& operator <<(std::ostream& os, const MapStorage& mapstor) {
    for (MapStorageCItem it = mapstor.begin(); it != mapstor.end(); ++it)
        os << it->info() << endl;
    return os;
}
