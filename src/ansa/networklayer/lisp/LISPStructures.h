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


#ifndef LISPSTRUCTURES_H_
#define LISPSTRUCTURES_H_

#include "IPvXAddress.h"
#include "IPv6Address.h"

/**
 * Enumeration of mapping states

enum MapState {INCOMPLETE, COMPLETE};
*/
/**
 * Enumeration of locator states

enum LocatorState {DOWN, UP};
*/

/**
 * EID Prefix

struct EidPrefix
{
    IPvXAddress eid;
    int eidLen;

    bool operator< (const EidPrefix& other) const {
        if(eid != other.eid)
            return eid < other.eid;
        else
            return eidLen < other.eidLen;
    }
};
 */
/**
 * RLOC

struct Locator
{
    LocatorState state;
    unsigned char priority;
    unsigned char weight;
};

typedef std::map<IPvXAddress, Locator> Locators;
typedef Locators::iterator LocatorItem;
*/
/**
 * Vector of Mapping Cache entries

struct MapEntry
{
    simtime_t expiry;
    MapState mapState;
    Locators rloc;
};

struct SiteInfo
{
    simtime_t lastTime;
    std::string registredBy;
    std::string name;
    std::string key;

    bool operator< (const SiteInfo& other) const {
        if ( strcmp(name.c_str(), other.name.c_str() ) < 0)
            return true;
        else
            return false;
    }
};
*/

//typedef std::map<EidPrefix, MapEntry> MapStorage;
//typedef std::map<EidPrefix, MapEntry>::iterator MapStorageItem;

//typedef std::map<SiteInfo, MapStorage> SiteStorage;
//typedef std::map<SiteInfo, MapStorage>::iterator SiteStorageItem;

//typedef std::map<LISPEidPrefix, LISPMapEntry> MapStorage;
//typedef std::map<LISPEidPrefix, LISPMapEntry>::iterator MapStorageItem;
//typedef std::map<LISPSiteInfo, MapStorage> SiteStorage;
//typedef std::map<LISPSiteInfo, MapStorage>::iterator SiteStorageItem;


/*
std::ostream& operator<< (std::ostream& os, const EidPrefix& ep)
{
    os << "" << ep.eid << "/" << ep.eidLen << ", ";
    return os;
}

std::ostream& operator<< (std::ostream& os, const Locator& loc)
{
    os << loc.state << "\t" << loc.priority << "\t" << loc.weight;
    return os;
}

std::ostream& operator<< (std::ostream& os, MapEntry& me)
{
    os << "expires: " << me.expiry << ", " << "state: " << me.mapState << endl;

    os << "\tLocator\tState\tPriority\tWeight" << endl;
    for (LocatorItem it = me.rloc.begin(); it != me.rloc.end(); ++it) {
        os << it->first << "\t" << it->second << endl;
    }
    return os;
}

std::ostream& operator<< (std::ostream& os, MapStorage& mc)
{
    for (MapStorageItem it = mc.begin(); it != mc.end(); ++it)
        os << it->first << it->second;
    return os;
}

std::ostream& operator<< (std::ostream& os, const SiteInfo& si)
{
    os << "Site " << si.name << ", key " << si.key << ", registered by " << si.registredBy << ", last time " << si.lastTime << endl;
    return os;
}

std::ostream& operator<< (std::ostream& os, SiteStorage& ss)
{
    for (SiteStorageItem it = ss.begin(); it != ss.end(); ++it)
        os << it->first << it->second;
    return os;
}
*/


class LISPStructures
{
  public:
    static int getNumMatchingPrefixBits6(IPv6Address addr1, IPv6Address addr2)
    {
        for (int j = 3; j != 0; j--)
        {
            const uint32 *w1 = addr1.words();
            const uint32 *w2 = addr2.words();
            uint32 res = w1[j] ^ w2[j];
            for (int i = 31; i >= 0; i--) {
                if (res & (1 << i)) {
                   // 1, means not equal, so stop
                   return 32 * j + 31 - i;
                }
            }
        }
        return !addr1.compare(addr2) ? 0 : -1;
    }

    static int doPrefixMatch(IPvXAddress addr1, IPvXAddress addr2)
    {
        //IPv4 vs IPv6 incomparable
        if (addr1.isIPv6() xor addr2.isIPv6())
            return -1;
        //IPv4
        if (!addr1.isIPv6())
            return addr1.get4().getNumMatchingPrefixBits(addr2.get4());
        //IPv6
        return getNumMatchingPrefixBits6(addr1.get6(), addr2.get6());
    }
};



#endif /* LISPSTRUCTERS_H_ */
