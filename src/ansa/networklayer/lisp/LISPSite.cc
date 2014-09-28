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


#include "LISPSite.h"

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

const std::string& LISPSite::getName() const {
    return name;
}

void LISPSite::setName(const std::string& name) {
    this->name = name;
}

std::string LISPSite::info() const {
    std::stringstream os;
    os << name << ", key: \"" << key << "\"" << endl
       << "Maintained EIDs>" << endl << LISPMapStorageBase::info();
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

LISPSiteRecord* LISPSite::findRecordByAddress(IPvXAddress& address) {
    for (EtrItem it = ETRs.begin(); it != ETRs.end(); ++it) {
        if (it->getServerEntry().getAddress() == address)
            return &(*it);
    }
    return NULL;
}

void LISPSite::removeRecord(LISPSiteRecord& srec) {
    ETRs.remove(srec);
}

Etrs LISPSite::findAllRecordsByEid(const IPvXAddress& address) {
    Etrs result;
    for (EtrItem it = ETRs.begin(); it != ETRs.end(); ++it) {
        if ( it->lookupMapEntry(address) )
            result.push_back( *it );
            //EV << "Allrecs> " << it->info() << endl;
    }
    return result;
}
