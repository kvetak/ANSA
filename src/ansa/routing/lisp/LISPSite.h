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


#ifndef LISPSITE_H_
#define LISPSITE_H_

#include <omnetpp.h>
#include "ansa/routing/lisp/LISPCommon.h"
#include "ansa/routing/lisp/LISPSiteRecord.h"
#include "ansa/routing/lisp/LISPTimers_m.h"

namespace inet {

class LISPSite : public LISPMapStorageBase
{
  public:
    LISPSite();
    LISPSite(std::string nam, std::string ke);
    virtual ~LISPSite();

    bool operator== (const LISPSite& other) const;

    std::string info() const;

    const std::string& getKey() const;
    void setKey(const std::string& key);
    const std::string& getSiteName() const;
    void setSiteName(const std::string& name);
    const Etrs& getETRs() const;
    void setETRs(const Etrs& etRs);
    void clearETRs();

    void addRecord(LISPSiteRecord& srec);
    void removeRecord(LISPSiteRecord& srec);
    LISPSiteRecord* findRecordByAddress(L3Address& address);
    Etrs findAllRecordsByEid(const L3Address& address);
    bool isEidMaintained(const L3Address& address);

  protected:

    std::string name;
    std::string key;
    Etrs ETRs;

};

//Free function
std::ostream& operator<< (std::ostream& os, const LISPSite& si);

}
#endif /* LISPSITE_H_ */
