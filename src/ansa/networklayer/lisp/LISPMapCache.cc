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

#include "LISPMapCache.h"
#include "LISPStructures.h"
#include "deviceConfigurator.h"

Define_Module(LISPMapCache);

LISPMapCache::LISPMapCache(){}
LISPMapCache::~LISPMapCache()
{
    this->clear();
}

bool LISPMapCache::isMapCacheItemExist(EidPrefix prefix)
{
    return MappingCache.find(prefix) != MappingCache.end() ? true : false;
}

void LISPMapCache::clear()
{
    return MappingCache.clear();
};

void LISPMapCache::addEntry(IPvXAddress eid, int length)
{
    EidPrefix pref;
    pref.eid = eid;
    pref.eidLen = length;

    //Brand new record
    if (!isMapCacheItemExist(pref))
    {
        MapCacheEntry& mce = MappingCache[pref];
        mce.mapState = INCOMPLETE;
        mce.expiry = simTime() + 300;
    }
};

void LISPMapCache::updateEntryState(MapCacheEntry* mcentry, MapState state)
{
    mcentry->mapState = state;
};

void LISPMapCache::updateEntryExpiry(MapCacheEntry* mcentry, simtime_t time)
{
    mcentry->expiry = time;
};

void LISPMapCache::removeEntry(IPvXAddress address, int length)
{
    EidPrefix pref;
    pref.eid = address;
    pref.eidLen = length;

    MapCacheItem it = MappingCache.find(pref);
    MappingCache.erase(it);
};

MapCacheItem* LISPMapCache::lookup(IPvXAddress address)
{
    MapCacheItem* tmpit;
    int tmplen = 0;

    for (MapCacheItem it = MappingCache.begin(); it != MappingCache.end(); ++it) {
        int len = LISPStructures::doPrefixMatch(it->first.eid, address);
        if (len > tmplen && len >= it->first.eidLen) {
            tmpit = &it;
            tmplen = len;
        }
    }
    if (tmplen > 0)
        return tmpit;
    return NULL;
};

MapCacheItem* LISPMapCache::getEntry(IPvXAddress address, int length)
{
    EidPrefix pref;
    pref.eid = address;
    pref.eidLen = length;

    if (!isMapCacheItemExist(pref))
        return NULL;
    //TODO: Vesely - Clean local variable
    MapCacheItem ite = MappingCache.find(pref);
    return &ite;
}

void LISPMapCache::addLocator(MapCacheEntry* mcentry, IPvXAddress address)
{
    Locator loc;
    loc.state = DOWN;
    loc.priority = 100;
    loc.weight = 100;
    mcentry->rloc.insert(std::pair<IPvXAddress,Locator>(address, loc));
};

bool LISPMapCache::isLocatorExist(MapCacheEntry* mcentry, IPvXAddress address)
{
    return mcentry->rloc.find(address) != mcentry->rloc.end() ? true : false;
}

void LISPMapCache::updateLocatorState(Locator* loc, LocatorState state)
{
    loc->state = state;
};

void LISPMapCache::updateLocatorPriority(Locator* loc, unsigned char priority)
{
    loc->priority = priority;
};

void LISPMapCache::updateLocatorWeight(Locator* loc, unsigned char weight)
{
    loc->weight = weight;
};

Locator* LISPMapCache::getLocator(MapCacheEntry* mcentry, IPvXAddress address)
{
    if (!isLocatorExist(mcentry, address))
        return NULL;
    Locators::iterator ite = mcentry->rloc.find(address);
    return &ite->second;
}

void LISPMapCache::removeLocator(MapCacheEntry* mcentry, IPvXAddress address)
{
    Locators::iterator it = mcentry->rloc.find(address);
    mcentry->rloc.erase(it);
};

int LISPMapCache::size() const
{
    return MappingCache.size();
};

void LISPMapCache::initialize(int stage)
{
    if (stage < 3)
        return;

    deviceId = par("deviceId");
    DeviceConfigurator* devConf = ModuleAccess<DeviceConfigurator>("deviceConfigurator").get();

}

void LISPMapCache::handleMessage(cMessage *msg)
{

}


