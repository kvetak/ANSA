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


#ifndef LISPSITERECORD_H_
#define LISPSITERECORD_H_

#include <sstream>
#include <string>
#include "ansa/networklayer/lisp/LISPServerEntry.h"
#include "ansa/networklayer/lisp/LISPMapStorageBase.h"

class LISPSiteRecord : public LISPMapStorageBase {
  public:
    LISPSiteRecord();
    virtual ~LISPSiteRecord();

    bool operator== (const LISPSiteRecord& other) const;
    bool operator< (const LISPSiteRecord& other) const;

    LISPServerEntry& getServerEntry();
    void setServerEntry(const LISPServerEntry& serverEntry);

    std::string info() const;

  protected:
    LISPServerEntry ServerEntry;

};

typedef std::list<LISPSiteRecord> Etrs;
typedef Etrs::iterator EtrItem;
typedef Etrs::const_iterator EtrCItem;

typedef std::list<LISPSiteRecord*> EtrPs;
typedef EtrPs::iterator EtrPItem;
typedef EtrPs::const_iterator EtrPCItem;
//Free function
std::ostream& operator<< (std::ostream& os, const LISPSiteRecord& sr);


#endif /* LISPSITERECORD_H_ */
