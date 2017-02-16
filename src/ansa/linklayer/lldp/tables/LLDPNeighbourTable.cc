//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
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
* @file LLDPNeighbourTable.cc
* @author Tomas Rajca
* @author Vladimir Vesely (ivesely@fit.vutbr.cz)
* @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
*/

#include "ansa/linklayer/lldp/tables/LLDPNeighbourTable.h"
#include <algorithm>


namespace inet {
Register_Abstract_Class(LLDPNeighbour);
Define_Module(LLDPNeighbourTable);

LLDPNeighbour::LLDPNeighbour(LLDPAgent *a, const char *cId, const char *pId) {
    chassisId = cId;
    portId = pId;

    msap = cId;
    msap += pId;

    rxInfoTtl = new cMessage("NeighbourTtlTimer", LLDPNeighbourTable::ttl);
    rxInfoTtl->setContextPointer(this);

    agent = a;
}

LLDPNeighbour::~LLDPNeighbour()
{
    //if is scheduled, get his sender module, otherwise get owner module
    cSimpleModule *owner = dynamic_cast<cSimpleModule *>((rxInfoTtl->isScheduled()) ? rxInfoTtl->getSenderModule() : rxInfoTtl->getOwner());
    if(owner != nullptr)
    {// owner is cSimpleModule object -> can call his methods
        owner->cancelAndDelete(rxInfoTtl);
        rxInfoTtl = nullptr;
    }
}

std::string LLDPNeighbour::info() const
{
    std::stringstream string;

    string << "DevID: " << systemName << ", LocInt: " << agent->getIfaceName() << ", holdTime: " << ttl-round((simTime()-lastUpdate).dbl()) <<  ", enCap: " << enabledCap << ", Port ID: " << portId;

    return string.str();
}


///************************ LLDP AGENT TABLE ****************************///


void LLDPNeighbourTable::restartRxInfoTtl(LLDPNeighbour *neighbour, uint16_t holdTime)
{
    Enter_Method_Silent();
    cancelEvent(neighbour->rxInfoTtl);
    scheduleAt(simTime() + holdTime, neighbour->rxInfoTtl);
}

void LLDPNeighbourTable::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        WATCH_PTRVECTOR(neighbours);
    }
}

void LLDPNeighbourTable::handleMessage(cMessage *msg)
{
    // self message (timer)
    if (msg->isSelfMessage()) {
        if (msg->getKind() == ttl) {
            EV_INFO << "LLDP delete neighbour" << endl;
            LLDPNeighbour *neighbour = check_and_cast<LLDPNeighbour *>((cObject *)msg->getContextPointer());
            removeNeighbour(neighbour);
        }
        else
        {
            EV_WARN << "Unknown type of self message" << endl;
        }
    }
    else
        throw cRuntimeError("LLDPNeighborTable received a message although it does not have gates.");
}


LLDPNeighbour * LLDPNeighbourTable::findNeighbourByMSAP(std::string msap)
{
    std::vector<LLDPNeighbour *>::iterator it;

    // through all neighbours search for same MSAP
    for (it = neighbours.begin(); it != neighbours.end(); ++it)
        if(msap.compare((*it)->getMsap()) == 0)
            return (*it);

    return nullptr;
}


LLDPNeighbour *LLDPNeighbourTable::addNeighbour(LLDPAgent *agent, std::string chassisId, std::string portId)
{
    std::string msap = chassisId + portId;
    if(findNeighbourByMSAP(msap) != nullptr)
    {// neighbour already in table
        EV_WARN << "Adding to LLDPNeighbourTable neighbour, which is already in it - name " << chassisId << ", port " << portId << endl;
    }

    LLDPNeighbour *neighbour = new LLDPNeighbour(agent, chassisId.c_str(), portId.c_str());
    take(neighbour->rxInfoTtl);
    neighbours.push_back(neighbour);

    return neighbour;
}

void LLDPNeighbourTable::removeNeighbour(LLDPNeighbour * neighbour)
{
    if(neighbour == nullptr)
        return;

    auto n = find(neighbours.begin(), neighbours.end(), neighbour);
    if (n != neighbours.end())
    {
        neighbour->agent->getSt()->ageoutsTotal++;
        delete *n;
        neighbours.erase(n);
    }
}

void LLDPNeighbourTable::removeNeighbour(std::string msap)
{
    std::vector<LLDPNeighbour *>::iterator it;

    for (it = neighbours.begin(); it != neighbours.end();)
    {// through all neighbours
        if( msap.compare((*it)->getMsap()) == 0 )
        {// found same
            (*it)->agent->getSt()->ageoutsTotal++;
            delete (*it);
            it = neighbours.erase(it);
            return;
        }        else
        {// do not delete -> get next
            ++it;
        }
    }
}

void LLDPNeighbourTable::removeNeighboursByAgent(LLDPAgent *ag)
{
    std::vector<LLDPNeighbour *>::iterator it, lIt;

    lIt = it = neighbours.begin();
    for (it = neighbours.begin(); it != neighbours.end(); )
    {// through all manAddresses
        if((*it)->getAgent() == ag)
        {
            lIt = it;
            delete (*it);
            it = neighbours.erase(it);
        }
        else
            ++it;
    }
}

LLDPNeighbourTable::LLDPNeighbourTable() {

}

LLDPNeighbourTable::~LLDPNeighbourTable() {
    for (auto & elem : neighbours) {
        cancelAndDelete(elem->getRxInfoTtl());
        delete (elem);
    }
    neighbours.clear();
}

} /* namespace inet */
