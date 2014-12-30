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

#include "LISPSiteDatabase.h"

Define_Module(LISPSiteDatabase);

void LISPSiteDatabase::initialize(int stage)
{
    if (stage < numInitStages() - 1)
        return;

    //Import site database
    parseConfig( par(CONFIG_PAR).xmlValue() );

    //Display in simulation
    WATCH_LIST(SiteDatabase);
}

void LISPSiteDatabase::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}

LISPSite* LISPSiteDatabase::findSiteInfoByKey(std::string& siteKey) {
    for (SiteStorageItem it = SiteDatabase.begin(); it != SiteDatabase.end(); ++it) {
        if ( !it->getKey().compare(siteKey) )
            return &(*it);
    }
    return NULL;
}

LISPSite* LISPSiteDatabase::findSiteByAggregate(const IPvXAddress& addr) {
    LISPSite* res = NULL;
    int tmplen = 0;
    for (SiteStorageItem it = SiteDatabase.begin(); it != SiteDatabase.end(); ++it) {
        LISPMapEntry* me = it->lookupMapEntry(addr);
        if ( me ) {
            int len = LISPCommon::doPrefixMatch(me->getEidPrefix().getEidAddr(), addr);
            if (len >= tmplen) {
                tmplen = len;
                res = &(*it);
            }
        }
    }
    return res;
}

void LISPSiteDatabase::parseConfig(cXMLElement* config) {
    if (!config)
        return;

    if ( opp_strcmp(config->getTagName(), MAPSERVER_TAG) )
        config = config->getFirstChildWithTag(MAPSERVER_TAG);

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

void LISPSiteDatabase::addSite(LISPSite& si)
{
    SiteDatabase.push_back(si);
}
