// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
 * @file RBPortTable.cc
 * @author Marcel Marek (mailto:xscrew02@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 20.3.2013
 * @brief
 * @detail
 * @todo
 */

#include "RBPortTable.h"
#include "InterfaceTableAccess.h"
#include "InterfaceTable.h"
#include "RBVLANTable.h"


void RBPort::setInterfaceId(int interfaceId){
    this->interfaceId = interfaceId;
}

void RBPort::setType(RBPort::RBPType type){
    this->type = type;
}

int RBPort::getGateIndex() const
{
    return gateIndex;
}

int RBPort::getInterfaceId() const
{
    return interfaceId;
}

RBPort::RBPType RBPort::getType() const
{
    return type;
}

void RBPort::setGateIndex(int gateIndex)
{
    this->gateIndex = gateIndex;
}

void RBPort::initDefaults(){
    this->disabled = false;
    this->trunk = false;
    this->access = false;
    this->p2p = false;

    this->learning = true; //TODO check

    this->announcingSet.push_back(1);
    this->enabledgSet.push_back(1);
    this->nonAdj = false;
}


Define_Module(RBPortTable);
RBPortTable::RBPortTable()
{
    // TODO Auto-generated constructor stub

}

RBPortTable::~RBPortTable()
{
    // TODO Auto-generated destructor stub
}
void RBPortTable::initialize(int stage){
    if(stage == 4){
        RBVLANTable *vlanTable = ModuleAccess<RBVLANTable>("rBVLANTable").get();
        IInterfaceTable *ifaceTable = InterfaceTableAccess().get();
        for(int i = 0; i < ifaceTable->getNumInterfaces(); i++){
            InterfaceEntry * ifaceEntry = ifaceTable->getInterface(i);
            RBPort port;
            port.setInterfaceId(ifaceEntry->getInterfaceId());
            port.setGateIndex(ifaceEntry->getNetworkLayerGateIndex());

            //TODO
            //port.initDefaults();

            portTable.push_back(port);
        }
    }

}

RBPort * RBPortTable::getPortByGateIndex(int gateIndex){
    for(tRBPortTable::iterator it = this->portTable.begin(); it != this->portTable.end(); ++it){
        if((*it).getGateIndex() == gateIndex){
            return &(*it);
        }
    }
}
