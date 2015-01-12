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
#include "IPv4InterfaceData.h"
#include "IPv6InterfaceData.h"

Define_Module(LISPMapDatabase);

void LISPMapDatabase::initialize(int stage)
{
    if (stage < numInitStages() - 1)
        return;

    Ift = InterfaceTableAccess().get();
    advertonlyowneids = par(ADVERTONLYOWNEID_PAR).boolValue();

    //EtrMappings
    parseEtrMappings( par(CONFIG_PAR).xmlValue() );

    updateDisplayString();

    WATCH_LIST(MappingStorage);
}

void LISPMapDatabase::handleMessage(cMessage *msg)
{

}

void LISPMapDatabase::parseEtrMappings(cXMLElement* config) {
    //EtrMappings
    if ( opp_strcmp(config->getTagName(), ETRMAP_TAG) )
        config = config->getFirstChildWithTag(ETRMAP_TAG);

    if (config)
        parseMapEntry(config);
    else {
        EV << "Does not contain any EtrMappings. This may be undesirable for xTR functionality!" << endl;
        return;
    }

    //Filter map-storage so that it contains only EIDs of router interfaces
    if (advertonlyowneids) {
        MapStorage mstmp;

        for (MapStorageItem it = MappingStorage.begin(); it != MappingStorage.end(); ++it) {
            for (int i = 0; i < Ift->getNumInterfaces(); ++i) {
                IPv4InterfaceData* int4Data = Ift->getInterface(i)->ipv4Data();
                IPv4Address adr4 =
                        (int4Data) ?
                                int4Data->getIPAddress() :
                                IPv4Address::UNSPECIFIED_ADDRESS;
                IPv6InterfaceData* int6Data = Ift->getInterface(i)->ipv6Data();
                IPv6Address adr6 =
                        (int6Data) ?
                                int6Data->getPreferredAddress() :
                                IPv6Address::UNSPECIFIED_ADDRESS;
                if (it->getEidPrefix().getEidAddr().equals(LISPCommon::getNetworkAddress(adr4, it->getEidPrefix().getEidLength()))
                    || it->getEidPrefix().getEidAddr().equals(LISPCommon::getNetworkAddress(adr6, it->getEidPrefix().getEidLength()))
                   ) {
                    mstmp.push_back(*it);
                    break;
                }
            }
        }

        MappingStorage = mstmp;
    }

    //Set EtrMappings LOCAL locators and add other locators to probingset
    for (int i = 0; i < Ift->getNumInterfaces(); ++i) {
        IPv4InterfaceData* int4Data = Ift->getInterface(i)->ipv4Data();
        IPv4Address adr4 =
                (int4Data) ?
                        int4Data->getIPAddress() :
                        IPv4Address::UNSPECIFIED_ADDRESS;
        IPv6InterfaceData* int6Data = Ift->getInterface(i)->ipv6Data();
        IPv6Address adr6 =
                (int6Data) ?
                        int6Data->getPreferredAddress() :
                        IPv6Address::UNSPECIFIED_ADDRESS;
        for (MapStorageItem it = MappingStorage.begin(); it != MappingStorage.end(); ++it) {
            //Mark local and foreign locators
            for (LocatorItem jt = it->getRlocs().begin();
                    jt != it->getRlocs().end(); ++jt) {
                //IF locator is local THEN mark it...
                if (jt->getRlocAddr().equals(adr4) || jt->getRlocAddr().equals(adr6))
                    jt->setLocal(true);
            }
        }
    }
}

bool LISPMapDatabase::isOneOfMyEids(IPvXAddress addr) {
    return (lookupMapEntry(addr) ? true : false);
}

void LISPMapDatabase::updateDisplayString() {
    if (!ev.isGUI())
        return;
    std::ostringstream description;
    description << MappingStorage.size() << " EIDs";
    this->getDisplayString().setTagArg("t", 0, description.str().c_str());
    this->getDisplayString().setTagArg("t", 1, "t");
}
