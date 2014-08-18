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

#include "LISPCore.h"

const char* MAPSERVERADDR_TAG   = "MapServerAddress";
const char* MAPRESOLVERADDR_TAG = "MapResolverAddress";
const char* MAPSERVER_TAG       = "MapServer";
const char* MAPRESOLVER_TAG     = "MapResolver";

Define_Module(LISPCore);

LISPCore::LISPCore()
{

}
LISPCore::~LISPCore()
{
    //mss->clear();
    //mrs->clear();
}

bool LISPCore::isMapResolverV4() const {
    return mapResolverV4;
}

void LISPCore::setMapResolverV4(bool mapResolverV4) {
    this->mapResolverV4 = mapResolverV4;
}

bool LISPCore::isMapResolverV6() const {
    return mapResolverV6;
}

void LISPCore::setMapResolverV6(bool mapResolverV6) {
    this->mapResolverV6 = mapResolverV6;
}

bool LISPCore::isMapServerV4() const {
    return mapServerV4;
}

void LISPCore::setMapServerV4(bool mapServerV4) {
    this->mapServerV4 = mapServerV4;
}

bool LISPCore::isMapServerV6() const {
    return mapServerV6;
}

void LISPCore::setMapServerV6(bool mapServerV6) {
    this->mapServerV6 = mapServerV6;
}

void LISPCore::parseConfig(cXMLElement* config)
{
    //Config element is empty
    if (!config)
        return;

    //Map server addresses initial setup
    cXMLElementList msa = config->getChildrenByTagName(MAPSERVERADDR_TAG);
    for (cXMLElementList::iterator i = msa.begin(); i != msa.end(); ++i) {
        cXMLElement* m = *i;
        //EV << "IPv4: " << m->getAttribute("ipv4") << "\tIPv6: " << m->getAttribute("ipv6");
        std::string mipv4 = m->getAttribute(IPV4_ATTR);
        std::string mipv6 = m->getAttribute(IPV6_ATTR);
        std::string mk = m->getAttribute("key");
        if ( (!mipv4.empty() || !mipv6.empty()) && !mk.empty())
            MapServers.push_back(LISPServerEntry(mipv4.c_str(), mipv6.c_str(), mk.c_str()));
    }

    //Map resolver addresses initial setup
    cXMLElementList mra = config->getChildrenByTagName(MAPRESOLVERADDR_TAG);
    for (cXMLElementList::iterator i = mra.begin(); i != mra.end(); ++i) {
        cXMLElement* m = *i;
        //EV << "IPv4: " << m->getAttribute("ipv4") << "\tIPv6: " << m->getAttribute("ipv6");
        std::string mipv4 = m->getAttribute(IPV4_ATTR);
        std::string mipv6 = m->getAttribute(IPV6_ATTR);
        if ( !mipv4.empty() || !mipv6.empty() )
            MapResolvers.push_back(LISPServerEntry(mipv4.c_str(),  mipv6.c_str()));
    }

    //Enable MS functionality
    mapServerV4 = par("mapServerV4");
    mapServerV4 = par("mapServerV6");
    cXMLElement* ms = config->getFirstChildWithTag(MAPSERVER_TAG);
    if (ms != NULL) {
        if ( !strcmp(ms->getAttribute(IPV4_ATTR), ENABLED_VAL) )
            mapServerV4 = true;
        if ( !strcmp(ms->getAttribute(IPV6_ATTR), ENABLED_VAL) )
            mapServerV6 = true;
    }

    //Enable MR functionality
    mapResolverV4 = par("mapResolverV4");
    mapResolverV4 = par("mapResolverV6");
    cXMLElement* mr = config->getFirstChildWithTag(MAPRESOLVER_TAG);
    if ( mr != NULL) {
        if ( !strcmp(mr->getAttribute("ipv4"), "enabled") )
            mapResolverV4 = true;
        if ( !strcmp(mr->getAttribute("ipv6"), "enabled") )
            mapResolverV6 = true;
    }

}

void LISPCore::initialize(int stage)
{
    if (stage < 3)
        return;
    // access to the routing and interface table
    Rt = AnsaRoutingTableAccess().get();
    Ift = InterfaceTableAccess().get();
    // get deviceId
    deviceId = par("deviceId");
    // local MapCache
    Lmc =  ModuleAccess<LISPMapCache>("lispMapCache").get();

    parseConfig(par("configData").xmlValue());

    //Watchers
    WATCH_LIST(MapResolvers);
    WATCH_LIST(MapServers);
    WATCH(mapServerV4);
    WATCH(mapServerV6);
    WATCH(mapResolverV4);
    WATCH(mapResolverV6);

    //socket.setOutputGate(gate("udpOut"));
    //socket.bind(4341);
    //setSocketOptions();
}

void LISPCore::handleMessage(cMessage *msg)
{



}

void LISPCore::updateDisplayString()
{
    if (ev.isGUI())
    {
        //TODO
    }
}

void LISPCore::receiveChangeNotification(int category, const cObject *details)
{
    //TODO
}

