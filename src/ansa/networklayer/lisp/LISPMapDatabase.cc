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

#include "LISPMapDatabase.h"

Define_Module(LISPMapDatabase);

LISPMapDatabase::LISPMapDatabase(){}

LISPMapDatabase::~LISPMapDatabase()
{

}

void LISPMapDatabase::addSite(LISPSite& si)
{
    SiteDatabase.push_back(si);
}

void LISPMapDatabase::initialize(int stage)
{
    if (stage < 3)
        return;

    parseConfig( par("configData").xmlValue() );

    //Display in simulation
    WATCH_LIST(SiteDatabase);
}

void LISPMapDatabase::parseConfig(cXMLElement* config) {
    if (!config)
        return;

    cXMLElementList map = config->getChildrenByTagName(SITE_TAG);

    for (cXMLElementList::iterator i = map.begin(); i != map.end(); ++i) {
        cXMLElement* m = *i;

        //Check integrity
        if (!m->getAttribute(NAME_ATTR) || !m->getAttribute(KEY_ATTR)) {
            EV << "Config XML file missing tag or attribute - NAME/KEY" << endl;
            continue;
        }

        //Parse Site tag
        std::string nam = m->getAttribute(NAME_ATTR);
        std::string ke  = m->getAttribute(KEY_ATTR);

        if (!nam.empty() && ! ke.empty()) {
            //Create new SiteInfo entry
            LISPSite site = LISPSite(nam, ke);

            //Parse potential EIDs
            site.parseMapEntry(m);

            //Add entry to SiteStorage
            this->addSite(site);
        }
    }
}

LISPSite* LISPMapDatabase::findSiteByAggregate(const IPvXAddress& addr) {
    for (SiteStorageItem it = SiteDatabase.begin(); it != SiteDatabase.end(); ++it) {
        if ( it->lookupMapEntry(addr) )
            return &(*it);
    }
    return NULL;
}

void LISPMapDatabase::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}

LISPSite* LISPMapDatabase::findSiteInfoByKey(std::string& siteKey) {
    for (SiteStorageItem it = SiteDatabase.begin(); it != SiteDatabase.end(); ++it) {
        if ( !it->getKey().compare(siteKey) )
            return &(*it);
    }
    return NULL;
}

