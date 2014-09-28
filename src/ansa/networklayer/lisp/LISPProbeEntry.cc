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

#include "LISPProbeEntry.h"

LISPProbeEntry::LISPProbeEntry(IPvXAddress nrloc) :
    rlocAddr(nrloc), lastTimeProbed(SIMTIME_ZERO), curInit(true)
{
}

LISPProbeEntry::~LISPProbeEntry()
{
    eids.clear();
}

std::string LISPProbeEntry::info() const {
    std::stringstream os;
    os << "RLOC " << rlocAddr << eids;
    return os.str();
}

bool LISPProbeEntry::hasEid(const LISPEidPrefix& eid) {
    for (EidCItem it = eids.begin(); it != eids.end(); ++it) {
        if (it->first == eid)
            return true;
    }
    return false;
}

void LISPProbeEntry::addEid(const LISPEidPrefix eid, LISPRLocator* rloc) {
    //EV << eid << "\n" << rloc->info();
    EidStat es = EidStat(eid, rloc);
    eids.push_back(es);
}

std::ostream& operator <<(std::ostream& os, const LISPProbeEntry& entry) {
    return os << entry.info();
}

bool LISPProbeEntry::operator ==(const LISPProbeEntry& other) const {
    return rlocAddr == other.rlocAddr && eids == other.eids;
}

LISPEidPrefix LISPProbeEntry::getNextEid() {
    //IF there are no EIDs THEN error
    //FIXME: Probably not a correct thing in case of RLOCs changing quickly
    if (!eids.size())
        throw cException("getNextEid cannot return nothing!");

    if (curInit) {
        curEid = eids.begin();
        curInit = false;
    }
    //Cycle to beginning
    if (curEid == eids.end() )
        curEid = eids.begin();

    //...OTHERWISE iterate pointer and return current EID
    //EV << "EID1> " << this->eids.front().first << endl;
    //EV << "EID2> " << this->eids.begin()->first << endl;
    LISPEidPrefix tmp = this->curEid->first;
    //EV << "TMP> " << tmp << endl;
    ++curEid;
    return tmp;
}

const IPvXAddress& LISPProbeEntry::getRlocAddr() const {
    return rlocAddr;
}


std::ostream& operator <<(std::ostream& os, const EidStat& es) {
    os << "\tis ";
    if (es.second->getState() == LISPRLocator::UP)
        os << "up for EID ";
    else
        os << "down for EID ";
    return os << es.first;
}

std::ostream& operator <<(std::ostream& os, const EidsStats& eids) {
    for (EidCItem it = eids.begin(); it != eids.end(); ++it)
        os << endl << *it;
    return os;
}

const simtime_t& LISPProbeEntry::getLastTimeProbed() const {
    return lastTimeProbed;
}

void LISPProbeEntry::setLastTimeProbed(const simtime_t lastTimeProbed) {
    this->lastTimeProbed = lastTimeProbed;
}

void LISPProbeEntry::setRlocStatusForEid(const LISPEidPrefix& eid, LISPRLocator::LocatorState stat) {
    for (EidItem it = eids.begin(); it != eids.end(); ++it) {
        if (it->first == eid) {
            it->second->setState(stat);
            break;
        }
    }
}

void LISPProbeEntry::setRlocStatusForAllEids(LISPRLocator::LocatorState stat) {
    for (EidItem it = eids.begin(); it != eids.end(); ++it) {
        it->second->setState(stat);
    }
}

const EidsStats& LISPProbeEntry::getEids() const {
    return eids;
}
