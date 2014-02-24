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

#ifndef __INET_LISPMAPCACHE_H_
#define __INET_LISPMAPCACHE_H_

#include <omnetpp.h>
#include "LISPStructures.h"

class LISPMapCache : public cSimpleModule
{

  protected:

    /**
     * Main ADT that acts as mapping cache/database
     */
    MapCache MappingCache;

  public:

    LISPMapCache();
    virtual ~LISPMapCache();

    virtual bool isMapCacheItemExist(EidPrefix prefix);
    virtual void clear();

    virtual void addEntry(IPvXAddress eid, int length);
    virtual void updateEntryState(MapCacheEntry* mcentry, MapState state);
    virtual void updateEntryExpiry(MapCacheEntry* mcentry, simtime_t time);
    virtual void removeEntry(IPvXAddress address, int length);
    virtual MapCacheItem* lookup(IPvXAddress address);
    virtual MapCacheItem* getEntry(IPvXAddress address, int length);

    virtual bool isLocatorExist(MapCacheEntry* mcentry, IPvXAddress address);
    virtual void addLocator(MapCacheEntry* mcentry, IPvXAddress address);
    virtual void updateLocatorState(Locator* loc, LocatorState state);
    virtual void updateLocatorPriority(Locator* loc, unsigned char priority);
    virtual void updateLocatorWeight(Locator* loc, unsigned char weight);
    virtual Locator* getLocator(MapCacheEntry* mcentry, IPvXAddress address);
    virtual void removeLocator(MapCacheEntry* mcentry, IPvXAddress address);

    virtual int size() const;

  protected:

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

#endif
