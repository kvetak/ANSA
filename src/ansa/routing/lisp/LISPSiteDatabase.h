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

#ifndef __ANSA_LISPSITEDATABASE_H_
#define __ANSA_LISPSITEDATABASE_H_

#include <omnetpp.h>
#include "ansa/routing/lisp/LISPSite.h"

namespace inet {

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
    LISPSite* findSiteByAggregate(const L3Address& addr);

    LISPEtrTimer* findExpirationTimer(L3Address addr, std::string name);

    LISPSiteRecord* updateSiteEtr(LISPSite* si, L3Address src, bool proxy);
    void updateEtrEntries(LISPSiteRecord* siteRec, const TRecord& mapEntry, std::string name);
    void updateTimeout(L3Address addr, std::string name);

  protected:
    EtrTimeouts SiteTimeouts;
    SiteStorage SiteDatabase;

    void parseConfig(cXMLElement* config);

    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    void updateDisplayString();
};
}
#endif
