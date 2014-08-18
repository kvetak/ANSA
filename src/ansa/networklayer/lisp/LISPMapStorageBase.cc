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

#include "LISPMapStorageBase.h"

const char* EID_TAG         = "EID";
const char* RLOC_TAG        = "RLOC";
const char* PRIORITY_ATTR   = "priority";
const char* WEIGHT_ATTR     = "weight";

LISPMapStorageBase::LISPMapStorageBase() {}

LISPMapStorageBase::~LISPMapStorageBase()
{
    this->clear();
}

void LISPMapStorageBase::clear() {
    MappingStorage.clear();
}

void LISPMapStorageBase::addMapEntry(LISPMapEntry& entry) {
    MappingStorage.push_back(entry);
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

LISPMapEntry* LISPMapStorageBase::lookupMapEntry(IPvXAddress& address) {
    LISPMapEntry* tmpit;
    int tmplen = 0;

    for (MapStorageItem it = MappingStorage.begin(); it != MappingStorage.end(); ++it) {
        int len = doPrefixMatch(it->getEidPrefix().getEid(), address);
        if (len > tmplen && len >= it->getEidPrefix().getEidLen() ) {
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
    for (MapStorageCItem it = MappingStorage.begin(); it != MappingStorage.end(); ++it) {
        os << it->info() << endl;
    }
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

            //Add RLOC if parsed successfully
            if (!rlocaddr.empty())
            {
                LISPRLocator locator;
                if (!pri.empty() && !wei.empty()) {
                    locator = LISPRLocator(rlocaddr.c_str(), pri.c_str(), wei.c_str());
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

int LISPMapStorageBase::getNumMatchingPrefixBits6(IPv6Address addr1, IPv6Address addr2)
{
    for (int j = 3; j != 0; j--)
    {
        const uint32 *w1 = addr1.words();
        const uint32 *w2 = addr2.words();
        uint32 res = w1[j] ^ w2[j];
        for (int i = 31; i >= 0; i--) {
            if (res & (1 << i)) {
               // 1, means not equal, so stop
               return 32 * j + 31 - i;
            }
        }
    }
    return !addr1.compare(addr2) ? 0 : -1;
}

int LISPMapStorageBase::doPrefixMatch(IPvXAddress addr1, IPvXAddress addr2)
{
    //IPv4 vs IPv6 incomparable
    if (addr1.isIPv6() xor addr2.isIPv6())
        return -1;
    //IPv4
    if (!addr1.isIPv6())
        return addr1.get4().getNumMatchingPrefixBits(addr2.get4());
    //IPv6
    return getNumMatchingPrefixBits6(addr1.get6(), addr2.get6());
}

/*
bool LISPMapStorageBase::isMapEntryExisting(LISPEidPrefix prefix)
{
    return MappingStorage.find(prefix) != MappingStorage.end() ? true : false;
}


void LISPMapStorageBase::addEntry(IPvXAddress eid, int length)
{
    LISPEidPrefix pref = new LISPEidPrefix(eid, length);
    //pref.eid = eid;
    //pref.eidLen = length;

    //Brand new record
    if (!isMapEntryExisting(pref))
    {
        LISPMapEntry& mce = MappingStorage[pref];
        mce = new LISPMapEntry(simTime() + 300, LISPMapEntry::INCOMPLETE);
        //mce.mapState = INCOMPLETE;
        //mce.expiry = simTime() + 300;
    }
};

void LISPMapStorageBase::updateEntryState(LISPMapEntry* mcentry, LISPMapEntry::MapState state)
{
    mcentry->mapState = state;
};

void LISPMapStorageBase::updateEntryExpiry(LISPMapEntry* mcentry, simtime_t time)
{
    mcentry->expiry = time;
};

void LISPMapStorageBase::removeEntry(IPvXAddress address, int length)
{
    LISPEidPrefix pref;
    pref.eid = address;
    pref.eidLen = length;

    MapStorageItem it = MappingStorage.find(pref);
    MappingStorage.erase(it);
};

MapStorageItem* LISPMapStorageBase::lookup(IPvXAddress address)
{
    MapStorageItem* tmpit;
    int tmplen = 0;

    for (MapStorageItem it = MappingStorage.begin(); it != MappingStorage.end(); ++it) {
        int len = LISPStructures::doPrefixMatch(it->first.eid, address);
        if (len > tmplen && len >= it->first.eidLen) {
            tmpit = &it;
            tmplen = len;
        }
    }
    if (tmplen > 0)
        return tmpit;
    return NULL;
};

MapStorageItem* LISPMapStorageBase::getEntry(IPvXAddress address, int length)
{
    LISPEidPrefix pref;
    pref.eid = address;
    pref.eidLen = length;

    if (!isMapEntryExisting(pref))
        return NULL;
    //TODO: Vesely - Clean local variable
    //MapStorageItem ite = MappingStorage.find(pref);
    return &(MappingStorage.find(pref));
}

int LISPMapStorageBase::size() const
{
    return MappingStorage.size();
};

*/
