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


#include <LISPMapEntry.h>

LISPMapEntry::LISPMapEntry() : expiry(SIMTIME_ZERO), Action(LISPCommon::NO_ACTION)
{
}

LISPMapEntry::LISPMapEntry(LISPEidPrefix neid) :
    EID(neid),
    expiry(SIMTIME_ZERO), Action(LISPCommon::NO_ACTION)
{
}

LISPMapEntry::~LISPMapEntry() {
    expiry = SIMTIME_ZERO;
    RLOCs.clear();
    Action = LISPCommon::NO_ACTION;
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
    return this->RLOCs.size() ? COMPLETE : INCOMPLETE;
}

const Locators& LISPMapEntry::getRlocs() const {
    return RLOCs;
}

void LISPMapEntry::setRlocs(const Locators& rloCs) {
    RLOCs = rloCs;
}

const LISPCommon::EAct LISPMapEntry::getAction() const {
    return Action;
}

void LISPMapEntry::setAction(const LISPCommon::EAct& action) {
    Action = action;
}

bool LISPMapEntry::isLocatorExisting(const IPvXAddress& address) const
{
    for (LocatorCItem it = RLOCs.begin(); it != RLOCs.end(); ++it)
    {
        if (it->getRlocAddr() == address)
            return true;
    }
    return false;
    //return mcentry->rloc.find(address) != mcentry->rloc.end() ? true : false;
}

LISPRLocator* LISPMapEntry::getLocator(const IPvXAddress& address)
{
    for (LocatorItem it = RLOCs.begin(); it != RLOCs.end(); ++it)
    {
        if (it->getRlocAddr() == address)
            return &(*it);
    }
    return NULL;
}

void LISPMapEntry::addLocator(LISPRLocator& entry)
{
    RLOCs.push_back(entry);
    RLOCs.sort();
};

bool LISPMapEntry::operator ==(const LISPMapEntry& other) const {
    return EID == other.EID &&
           expiry == other.expiry &&
           RLOCs == other.RLOCs;
}

std::string LISPMapEntry::getMapStateString() const {
    switch (this->getMapState()) {
        case INCOMPLETE:
            return "incomplete";
        case COMPLETE:
            return "complete";
        default:
            return "UNKNOWN";
    }
}

Locators& LISPMapEntry::getRlocs() {
    return RLOCs;
}

const std::string LISPMapEntry::getActionString() const {
    switch (Action) {
    case LISPCommon::DROP:
      return "drop";
    case LISPCommon::SEND_MAP_REQUEST:
      return "send-map-request";
    case LISPCommon::NATIVELY_FORWARD:
      return "natively-forward";
      case LISPCommon::NO_ACTION:
      default:
        return "no-action";
    }
}

bool LISPMapEntry::operator <(const LISPMapEntry& other) const {
    if (EID < other.EID) return true;
    //if (EID > other.EID) return false;

    if (expiry < other.expiry) return true;
    //if (expiry > other.expiry) return false;

    if (Action < other.Action) return true;
    //if (Action > other.Action) return false;

    if (RLOCs < other.RLOCs) return true;
    //if (RLOCs > other.RLOCs) return false;

    return false;
}

void LISPMapEntry::removeLocator(IPvXAddress& address) {
    LISPRLocator* rloc = getLocator(address);
    if (rloc) {
        RLOCs.remove(*rloc);
    }
    return;
}

std::string LISPMapEntry::info() const {
    std::stringstream os;

    os << EID;

    //map-cache entry
    os << ", expires: ";
    if (expiry == SIMTIME_ZERO)
         os << " never";
    else
        os << expiry;

    os << ", state: " << getMapStateString();

    os << ", action: " << getActionString();

    for (LocatorCItem it = RLOCs.begin(); it != RLOCs.end(); ++it) {
        os << "\n   " << it->info();
    }

    return os.str();
}

std::ostream& operator <<(std::ostream& os, const LISPMapEntry& me) {
    return os << me.info();
}

std::ostream& operator <<(std::ostream& os, const Locators& rlocs) {
    for (LocatorCItem it = rlocs.begin(); it != rlocs.end(); ++it)
        os << it->info() << endl;
    return os;
}
