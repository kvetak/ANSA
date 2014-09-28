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

#ifndef __INET_LISPMAPDATABASE_H_
#define __INET_LISPMAPDATABASE_H_

#include <omnetpp.h>
#include "LISPSite.h"

typedef std::list<LISPSite> SiteStorage;
typedef SiteStorage::iterator SiteStorageItem;
typedef SiteStorage::const_iterator SiteStorageCItem;

class LISPMapDatabase : public cSimpleModule
{
    public:
        LISPMapDatabase();
        virtual ~LISPMapDatabase();

        void addSite(LISPSite& si);
        LISPSite* findSiteInfoByKey(std::string& siteKey);
        LISPSite* findSiteByAggregate(const IPvXAddress& addr);

    protected:
        SiteStorage SiteDatabase;

        void parseConfig(cXMLElement* config);

        virtual int numInitStages() const { return 4; }
        virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg);

};

#endif