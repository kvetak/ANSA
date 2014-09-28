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


#include <LISPProbeSet.h>

LISPProbeSet::LISPProbeSet() {
}

LISPProbeSet::~LISPProbeSet() {
    Probes.clear();
}

std::string LISPProbeSet::info() const {
    std::stringstream os;
    for (ProbeCItem it = Probes.begin(); it != Probes.end(); ++it) {
        os << it->info();
    }
    return os.str();
}

bool LISPProbeSet::hasProbeEntry(IPvXAddress& rloc, LISPEidPrefix& eidPref) {
    return findProbeEntryByRlocAndEid(rloc, eidPref);
}

LISPProbeEntry* LISPProbeSet::findProbeEntryByRlocAndEid(IPvXAddress& rloc, const LISPEidPrefix& eidPref) {
    //EV << "Vstup " << Probes.size() << endl;
    for (ProbeItem it = Probes.begin(); it != Probes.end(); ++it) {
        //EV << it->getRlocAddr() << " -- " << it->hasEid(eidPref) << endl;
        if (it->getRlocAddr() == rloc && it->hasEid(eidPref))
            return &(*it);
    }
    return NULL;
}

LISPProbeEntry* LISPProbeSet::findFirstProbeEntryByRloc(const IPvXAddress& rloc) {
    for (ProbeItem it = Probes.begin(); it != Probes.end(); ++it) {
        if (it->getRlocAddr() == rloc)
            return &(*it);
    }
    return NULL;
}


void LISPProbeSet::addProbeEntry(LISPProbeEntry& probe) {
    Probes.push_back(probe);
}

void LISPProbeSet::removeProbeEntry(IPvXAddress& rloc, LISPEidPrefix& eidPref) {
    Probes.remove( *(findProbeEntryByRlocAndEid(rloc, eidPref)) );
}

std::ostream& operator <<(std::ostream& os, const LISPProbeSet& prs) {
    return os << prs.info();
}

ProbeEntries& LISPProbeSet::getProbes() {
    return Probes;
}
