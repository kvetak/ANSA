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

#ifndef LISPMAPSTORAGEBASE_H_
#define LISPMAPSTORAGEBASE_H_

//#include "ILISPMapStorage.h"
#include "LISPEidPrefix.h"
#include "LISPMapEntry.h"
#include "LISPCommon.h"

typedef std::list<LISPMapEntry> MapStorage;
typedef MapStorage::iterator MapStorageItem;
typedef MapStorage::const_iterator MapStorageCItem;

extern const char* EID_TAG;
extern const char* RLOC_TAG;
extern const char* PRIORITY_ATTR;
extern const char* WEIGHT_ATTR;

class LISPMapStorageBase
{
    public:
        LISPMapStorageBase();
        virtual ~LISPMapStorageBase();

        std::string info() const;
        void parseMapEntry(cXMLElement* config);

    protected:
        /**
         * Main ADT that acts as mapping cache/database
         */
        MapStorage MappingStorage;

        void clear();
        void addMapEntry(LISPMapEntry& entry);
        void removeMapEntry(const LISPMapEntry& entry);
        LISPMapEntry* findMapEntryByEidPrefix(const LISPEidPrefix& eidpref);
        LISPMapEntry* lookupMapEntry(IPvXAddress& address);

        static int doPrefixMatch(IPvXAddress addr1, IPvXAddress addr2);
        static int getNumMatchingPrefixBits6(IPv6Address addr1, IPv6Address addr2);

/*
        virtual bool isMapEntryExisting(LISPEidPrefix prefix);


        virtual void addEntry(IPvXAddress eid, int length);
        virtual void updateEntryState(LISPMapEntry* mcentry, LISPMapEntry::MapState state);
        virtual void updateEntryExpiry(LISPMapEntry* mcentry, simtime_t time);
        virtual void removeEntry(IPvXAddress address, int length);
        virtual MapStorageItem* lookup(IPvXAddress address);
        virtual MapStorageItem* getEntry(IPvXAddress address, int length);

        virtual int size() const;
*/
};

//Free function
std::ostream& operator<< (std::ostream& os, const LISPMapStorageBase& msb);

#endif /* LISPMAPSTORAGEBASE_H_ */
