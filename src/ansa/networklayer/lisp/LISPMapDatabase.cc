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

#include "LISPMapDatabase.h"

const char* SITE_TAG    = "Site";
const char* NAME_ATTR   = "name";
const char* KEY_ATTR    = "key";

Define_Module(LISPMapDatabase);

LISPMapDatabase::LISPMapDatabase(){}
LISPMapDatabase::~LISPMapDatabase()
{

}

void LISPMapDatabase::addSite(LISPSiteInfo& si)
{
    SiteDatabase.push_back(si);
}
void LISPMapDatabase::initialize(int stage)
{
    if (stage < 3)
        return;

    deviceId = par("deviceId");

    //DeviceConfigurator* devConf = ModuleAccess<DeviceConfigurator>("deviceConfigurator").get();
    //devConf->loadlLISPConfigForMapServer(this);
    //devConf->loadlLISPConfigForMapResolver(this);

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
            LISPSiteInfo si = LISPSiteInfo(nam, ke);

            //Parse potential EIDs
            si.parseMapEntry(m);

            //Add entry to SiteStorage
            this->addSite(si);
        }
    }
}

void LISPMapDatabase::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}
