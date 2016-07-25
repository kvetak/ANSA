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


#ifndef LISPPROBESET_H_
#define LISPPROBESET_H_

#include "ansa/routing/lisp/LISPProbeEntry.h"

namespace inet {

typedef std::list<LISPProbeEntry> ProbeEntries;
typedef ProbeEntries::const_iterator ProbeCItem;
typedef ProbeEntries::iterator ProbeItem;

class LISPProbeSet {
  public:
    LISPProbeSet();
    virtual ~LISPProbeSet();

    std::string info() const;

    bool hasProbeEntry(L3Address& rloc, LISPEidPrefix& eidPref);
    LISPProbeEntry* findProbeEntryByRlocAndEid(L3Address& rloc, const LISPEidPrefix& eidPref);
    LISPProbeEntry* findFirstProbeEntryByRloc(const L3Address& rloc);
    void addProbeEntry(LISPProbeEntry& probe);
    void removeProbeEntry(L3Address& rloc, LISPEidPrefix& eidPref);
    ProbeEntries& getProbes();

  private:
    ProbeEntries Probes;
};

//Free function
std::ostream& operator<< (std::ostream& os, const LISPProbeSet& prs);

}

#endif /* LISPPROBESET_H_ */
