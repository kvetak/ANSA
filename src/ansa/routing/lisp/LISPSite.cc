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
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */


#include "ansa/routing/lisp/LISPSite.h"

namespace inet {

LISPSite::LISPSite() {
    name = "";
    key = "";
}

LISPSite::LISPSite(std::string nam, std::string ke) {
    name = nam;
    key = ke;
}

LISPSite::~LISPSite() {
    name = "";
    key = "";
    clearETRs();
}

const std::string& LISPSite::getKey() const {
    return key;
}

void LISPSite::setKey(const std::string& key) {
    this->key = key;
}

const std::string& LISPSite::getSiteName() const {
    return name;
}

void LISPSite::setSiteName(const std::string& name) {
    this->name = name;
}

std::string LISPSite::info() const {
    std::stringstream os;
    os << name << ", key: \"" << key << "\"" << endl
       << "Maintained EIDs>" << endl;
    for (MapStorageCItem it = MappingStorage.begin(); it != MappingStorage.end(); ++it)
        os << it->getEidPrefix() << endl;
    if (ETRs.size()) {
        os << "\nRegistered ETRs>" << endl;
        for (EtrCItem it = ETRs.begin(); it != ETRs.end(); ++it)
            os << it->info() << endl;
    }
    return os.str();
}

std::ostream& operator <<(std::ostream& os, const LISPSite& si) {
    return os << si.info();
}

const Etrs& LISPSite::getETRs() const {
    return ETRs;
}

void LISPSite::setETRs(const Etrs& etrs) {
    ETRs = etrs;
}

void LISPSite::clearETRs() {
    ETRs.clear();
}

void LISPSite::addRecord(LISPSiteRecord& srec) {
    ETRs.push_back(srec);
}

LISPSiteRecord* LISPSite::findRecordByAddress(L3Address& address) {
    for (EtrItem it = ETRs.begin(); it != ETRs.end(); ++it) {
        if (it->getServerEntry().getAddress() == address)
            return &(*it);
    }
    return NULL;
}

bool LISPSite::operator ==(const LISPSite& other) const {
    return name == other.name
           && key == other.key
           && ETRs == other.ETRs;
}

void LISPSite::removeRecord(LISPSiteRecord& srec) {
    ETRs.remove(srec);
}

Etrs LISPSite::findAllRecordsByEid(const L3Address& address) {
    Etrs result;
    for (EtrItem it = ETRs.begin(); it != ETRs.end(); ++it) {
        if ( it->lookupMapEntry(address) )
            result.push_back( *it );
            //EV << "Allrecs> " << it->info() << endl;
    }
    return result;
}

bool LISPSite::isEidMaintained(const L3Address& address) {
    for (MapStorageCItem it = MappingStorage.begin(); it != MappingStorage.end(); ++it) {
        //IF non-comparable AFIs THEN skip
        if ( (address.getType() == L3Address::Ipv6) xor (it->getEidPrefix().getEidAddr().getType() == L3Address::Ipv6) )
            continue;
        //Count number of common bits and if greater than Maintained EID length then return true
        int commonbits = LISPCommon::doPrefixMatch(it->getEidPrefix().getEidAddr(), address);
        //EV << "Comparison of " << it->getEidPrefix().getEidAddr() << " and "<< address << " yields " << commonbits << " common bits from " << it->getEidPrefix().getEidLength() << endl;
        if (commonbits == -1 || commonbits >= it->getEidPrefix().getEidLength() )
            return true;
    }
    return false;
}

}
