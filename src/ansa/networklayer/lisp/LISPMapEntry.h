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

#ifndef LISPMAPENTRY_H_
#define LISPMAPENTRY_H_

#include <omnetpp.h>
#include "LISPRLocator.h"
#include "LISPEidPrefix.h"

#define LISP_DEFAULT_MAPSTATE INCOMPLETE

typedef std::list<LISPRLocator> Locators;
typedef std::list<LISPRLocator>::iterator LocatorItem;
typedef std::list<LISPRLocator>::const_iterator LocatorCItem;

class LISPMapEntry {
  public:
    enum MapState {INCOMPLETE, COMPLETE};

    LISPMapEntry();
    LISPMapEntry(LISPEidPrefix neid);
    LISPMapEntry(LISPEidPrefix neid, simtime_t time);
    LISPMapEntry(LISPEidPrefix neid, simtime_t time, MapState state);
    LISPMapEntry(LISPEidPrefix neid, simtime_t time, MapState state, Locators rlocs);
    virtual ~LISPMapEntry();

    bool operator== (const LISPMapEntry& other) const;

    const LISPEidPrefix& getEidPrefix() const;
    void setEidPrefix(const LISPEidPrefix& eidPrefix);
    const simtime_t& getExpiry() const;
    void setExpiry(const simtime_t& expiry);
    MapState getMapState() const;
    std::string getMapStateString() const;
    void setMapState(MapState mapState);
    const Locators& getRlocs() const;
    void setRlocs(const Locators& rloCs);

    std::string info() const;

    virtual bool isLocatorExisting(IPvXAddress& address);
    virtual void addLocator(LISPRLocator& entry);
    virtual LISPRLocator* getLocator(IPvXAddress& address);
    virtual void removeLocator(IPvXAddress& address);

  private:
    LISPEidPrefix EID;
    simtime_t expiry;
    MapState mapState;
    Locators RLOCs;
};

//Free function
std::ostream& operator<< (std::ostream& os, const LISPMapEntry& me);

#endif /* LISPMAPENTRY_H_ */
