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


#ifndef LISPMAPENTRY_H_
#define LISPMAPENTRY_H_

#include <omnetpp.h>
#include "ansa/networklayer/lisp/LISPCommon.h"
#include "ansa/networklayer/lisp/LISPRLocator.h"
#include "ansa/networklayer/lisp/LISPEidPrefix.h"

#define LISP_DEFAULT_MAPSTATE INCOMPLETE

typedef std::list<LISPRLocator> Locators;
typedef Locators::iterator LocatorItem;
typedef Locators::const_iterator LocatorCItem;

class LISPMapEntry {
  public:
    enum MapState {INCOMPLETE, COMPLETE};

    LISPMapEntry();
    LISPMapEntry(LISPEidPrefix neid);

    virtual ~LISPMapEntry();

    bool operator== (const LISPMapEntry& other) const;
    bool operator< (const LISPMapEntry& other) const;

    const LISPEidPrefix& getEidPrefix() const;
    void setEidPrefix(const LISPEidPrefix& eidPrefix);
    const simtime_t& getExpiry() const;
    void setExpiry(const simtime_t& expiry);
    LISPMapEntry::MapState getMapState() const;
    std::string getMapStateString() const;
    const Locators& getRlocs() const;
    Locators& getRlocs();
    void setRlocs(const Locators& rloCs);
    const LISPCommon::EAct getAction() const;
    const std::string getActionString() const;
    void setAction(const LISPCommon::EAct& action);
    unsigned int getTtl() const;
    void setTtl(unsigned int ttl);

    std::string info() const;

    virtual bool isLocatorExisting(const inet::L3Address& address) const;
    virtual void addLocator(LISPRLocator& entry);
    virtual LISPRLocator* getLocator(const inet::L3Address& address);
    virtual void removeLocator(inet::L3Address& address);

    LISPRLocator* getBestUnicastLocator();


  protected:
    LISPEidPrefix EID;
    Locators RLOCs;

    //Map-cache entry
    unsigned int ttl;
    simtime_t expiry;

    //Cache-Action
    LISPCommon::EAct Action;
};

//Free function
std::ostream& operator<< (std::ostream& os, const LISPMapEntry& me);
std::ostream& operator<< (std::ostream& os, const Locators& rlocs);

#endif /* LISPMAPENTRY_H_ */
