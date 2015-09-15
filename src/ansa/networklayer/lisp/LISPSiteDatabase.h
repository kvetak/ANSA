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

#ifndef __ANSA_LISPSITEDATABASE_H_
#define __ANSA_LISPSITEDATABASE_H_

#include <omnetpp.h>
#include "ansa/networklayer/lisp/LISPSite.h"

typedef std::list<LISPSite> SiteStorage;
typedef SiteStorage::iterator SiteStorageItem;
typedef SiteStorage::const_iterator SiteStorageCItem;

typedef std::list <LISPEtrTimer*> EtrTimeouts;
typedef EtrTimeouts::const_iterator SiteTimeoutCItem;
typedef EtrTimeouts::iterator SiteTimeoutItem;

class LISPSiteDatabase : public cSimpleModule
{
  public:
    void addSite(LISPSite& si);
    LISPSite* findSiteInfoByKey(std::string& siteKey);
    LISPSite* findSiteInfoBySiteName(std::string& siteName);
    LISPSite* findSiteByAggregate(const inet::L3Address& addr);

    LISPEtrTimer* findExpirationTimer(inet::L3Address addr, std::string name);

    LISPSiteRecord* updateSiteEtr(LISPSite* si, inet::L3Address src, bool proxy);
    void updateEtrEntries(LISPSiteRecord* siteRec, const TRecord& mapEntry, std::string name);
    void updateTimeout(inet::L3Address addr, std::string name);

  protected:
    EtrTimeouts SiteTimeouts;
    SiteStorage SiteDatabase;

    void parseConfig(cXMLElement* config);

    virtual void initialize(int stage);
    virtual int numInitStages() const { return 4; }
    virtual void handleMessage(cMessage *msg);
    void updateDisplayString();
};

#endif
