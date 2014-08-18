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

#include <LISPMapEntry.h>

LISPMapEntry::LISPMapEntry() {
    expiry = SIMTIME_ZERO;
    mapState = this->INCOMPLETE;
}

LISPMapEntry::LISPMapEntry(LISPEidPrefix neid) {
    EID = neid;
    expiry = simTime() + 300;
    mapState = this->INCOMPLETE;
}


LISPMapEntry::LISPMapEntry(LISPEidPrefix neid, simtime_t time) {
    EID = neid;
    expiry = time;
    mapState = INCOMPLETE;
}


LISPMapEntry::LISPMapEntry(LISPEidPrefix neid, simtime_t time, MapState state) {
    EID = neid;
    expiry = time;
    mapState = state;
}

LISPMapEntry::LISPMapEntry(LISPEidPrefix neid, simtime_t time, MapState state, Locators rlocs) {
    EID = neid;
    expiry = time;
    mapState = state;
    RLOCs = rlocs;
}

LISPMapEntry::~LISPMapEntry() {
    expiry = SIMTIME_ZERO;
    mapState = LISP_DEFAULT_MAPSTATE;
    RLOCs.clear();
}

const LISPEidPrefix& LISPMapEntry::getEidPrefix() const {
    return EID;
}

void LISPMapEntry::setEidPrefix(const LISPEidPrefix& eidPrefix) {
    EID = eidPrefix;
}

const simtime_t& LISPMapEntry::getExpiry() const {
    return expiry;
}

void LISPMapEntry::setExpiry(const simtime_t& expiry) {
    this->expiry = expiry;
}

LISPMapEntry::MapState LISPMapEntry::getMapState() const {
    return mapState;
}

void LISPMapEntry::setMapState(MapState mapState) {
    this->mapState = mapState;
}

const Locators& LISPMapEntry::getRlocs() const {
    return RLOCs;
}

void LISPMapEntry::setRlocs(const Locators& rloCs) {
    RLOCs = rloCs;
}

bool LISPMapEntry::isLocatorExisting(IPvXAddress& address)
{
    for (LocatorItem it = RLOCs.begin(); it != RLOCs.end(); ++it)
    {
        if (it->getRloc() == address)
            return true;
    }
    return false;
    //return mcentry->rloc.find(address) != mcentry->rloc.end() ? true : false;
}

LISPRLocator* LISPMapEntry::getLocator(IPvXAddress& address)
{
    for (LocatorItem it = RLOCs.begin(); it != RLOCs.end(); ++it)
    {
        if (it->getRloc() == address)
            return &(*it);
    }
    return NULL;
}

void LISPMapEntry::addLocator(LISPRLocator& entry)
{
    RLOCs.push_back(entry);
};

bool LISPMapEntry::operator ==(const LISPMapEntry& other) const {
    //TODO: Add RLOCs checks
    return this->EID == other.EID && this->expiry == other.expiry && this->mapState == other.mapState;
}

std::string LISPMapEntry::getMapStateString() const {
    switch (this->mapState) {
        case INCOMPLETE:
            return "incomplete";
        case COMPLETE:
            return "complete";
        default:
            return "UNKNOWN";
    }
}

void LISPMapEntry::removeLocator(IPvXAddress& address) {
    LISPRLocator* rloc = getLocator(address);
    if (rloc)
        RLOCs.remove(*rloc);
    return;
}

std::string LISPMapEntry::info() const {
    std::stringstream os;
    os << this->EID << ", expires: " << this->expiry << ", state: " << this->mapState << endl;
    for (LocatorCItem it = RLOCs.begin(); it != RLOCs.end(); it++) {
        os << "  " << it->info() << endl;
    }
    return os.str();
}

std::ostream& operator <<(std::ostream& os, const LISPMapEntry& me) {
    return os << me.info();
}
