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
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */


#ifndef LISPTSTRUCTS_H_
#define LISPTSTRUCTS_H_

#include <string>
#include "ansa/routing/lisp/LISPMapEntry.h"

namespace inet {

class TAfiAddr {
  public:
    TAfiAddr() {};
    TAfiAddr(L3Address addr) : address(addr) {};

    L3Address address;
    LISPCommon::Afi afi() const { return address.getType() == L3Address::Ipv6 ? LISPCommon::AFI_IPV6 : LISPCommon::AFI_IPV4; };

    std::string info() const;
};

class TLocator {
  public:
    bool LocalLocBit;                   // local locator Bit
    bool piggybackBit;                  // piggyback Bit
    bool RouteRlocBit;                  // route RLOC Bit
    LISPRLocator RLocator;              //<-- see LISP class struct

    std::string info() const;
};

typedef std::list<TLocator> TLocators;
typedef TLocators::const_iterator TLocatorCItem;
typedef TLocators::iterator TLocatorItem;

class TRecord {
  public:
    unsigned int recordTTL;             // TTL
    unsigned char locatorCount;         // locator count
    unsigned char ACT;                  // Action flags
    bool ABit;                          // Authoritative Bit
    unsigned short mapVersion;          // Map-Version
    LISPEidPrefix EidPrefix;            //<-- see LISP class struct
    TLocators locators;                 //<-- see struct above

    const std::string getActionString() const;

    std::string info() const;
};

typedef std::list<TRecord> TRecords;
typedef TRecords::const_iterator TRecordCItem;
typedef TRecords::iterator TRecordItem;
typedef std::list<LISPEidPrefix> TRecs;
typedef TRecs::iterator TRecsItem;
typedef TRecs::const_iterator TRecsCItem;
typedef std::list<TAfiAddr> TAfiAddrs;
typedef TAfiAddrs::const_iterator TAfiAddrCItem;

class TMapEntry {
  public:
    TMapEntry() : MapEntry(LISPMapEntry()) {};
    TMapEntry(LISPMapEntry me) : MapEntry(me) {};

    LISPMapEntry MapEntry;
};

typedef std::list<TMapEntry> TMapEntries;
typedef TMapEntries::iterator TMEItem;
typedef TMapEntries::const_iterator TMECItem;

//Free functions
std::ostream& operator<< (std::ostream& os, const TAfiAddr& taa);
std::ostream& operator<< (std::ostream& os, const TAfiAddrs& tas);
std::ostream& operator<< (std::ostream& os, const TRecs& recs);
std::ostream& operator<< (std::ostream& os, const TLocator& tloc);
std::ostream& operator<< (std::ostream& os, const TRecord& trec);
std::ostream& operator<< (std::ostream& os, const TRecords& trecs);
std::ostream& operator<< (std::ostream& os, const TMapEntry& tme);
std::ostream& operator<< (std::ostream& os, const TMapEntries& tmes);

}
#endif /* LISPTSTRUCTS_H_ */
