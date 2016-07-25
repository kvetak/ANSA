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


#ifndef LISPMAPSTORAGEBASE_H_
#define LISPMAPSTORAGEBASE_H_

#include "ansa/routing/lisp/LISPEidPrefix.h"
#include "ansa/routing/lisp/LISPMapEntry.h"
#include "ansa/routing/lisp/LISPTStructs.h"
#include "ansa/routing/lisp/LISPCommon.h"

namespace inet {

typedef std::list<LISPMapEntry> MapStorage;
typedef MapStorage::iterator MapStorageItem;
typedef MapStorage::const_iterator MapStorageCItem;

class LISPMapStorageBase
{
  public:
    LISPMapStorageBase();
    virtual ~LISPMapStorageBase();

    MapStorage& getMappingStorage();
    void clearMappingStorage();

    std::string info() const;

    void parseMapEntry(cXMLElement* config);
    void addMapEntry(LISPMapEntry& entry);
    bool updateMapEntry(const TRecord& record);
    bool syncMapEntry(LISPMapEntry& entry);
    void removeMapEntry(const LISPMapEntry& entry);

    LISPMapEntry* findMapEntryByEidPrefix(const LISPEidPrefix& eidpref);
    LISPMapEntry* findMapEntryFromByLocator(const L3Address& rloc, const LISPEidPrefix& eidPref);
    MapStorage findMapEntriesByLocator(const L3Address& rloc);

    LISPMapEntry* lookupMapEntry(L3Address address);

  protected:
    /**
      * Main ADT that acts as mapping cache/database
      */
    MapStorage MappingStorage;
};

//Free function
std::ostream& operator<< (std::ostream& os, const LISPMapStorageBase& msb);
std::ostream& operator<< (std::ostream& os, const MapStorage& mapstor);

}

#endif /* LISPMAPSTORAGEBASE_H_ */
