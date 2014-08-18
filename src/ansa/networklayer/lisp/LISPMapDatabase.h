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

#ifndef __INET_LISPMAPDATABASE_H_
#define __INET_LISPMAPDATABASE_H_

#include <omnetpp.h>
#include "LISPSiteInfo.h"

typedef std::list<LISPSiteInfo> SiteStorage;
typedef SiteStorage::iterator SiteStorageItem;

extern const char* SITE_TAG;
extern const char* NAME_ATTR;
extern const char* KEY_ATTR;

class LISPMapDatabase : public cSimpleModule
{
    public:
        LISPMapDatabase();
        virtual ~LISPMapDatabase();

        virtual void addSite(LISPSiteInfo& si);

    protected:
        const char  *deviceId;
        SiteStorage SiteDatabase;

        void parseConfig(cXMLElement* config);

        virtual int numInitStages() const { return 4; }
        virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg);
};

#endif
