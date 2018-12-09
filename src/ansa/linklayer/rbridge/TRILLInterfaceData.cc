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
 * @file TrillInterfaceData.cc
 * @author Marcel Marek (mailto:xscrew02@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 24.3.2013
 * @brief Represents TRILL related data on interface.
 * @detail Represents TRILL related data on interface.
 * @todo Z9
 */

#include "TrillInterfaceData.h"

namespace inet {

TrillInterfaceData::TrillInterfaceData()
{



    // TODO Auto-generated constructor stub

}

TrillInterfaceData::~TrillInterfaceData()
{
    // TODO Auto-generated destructor stub
}

void TrillInterfaceData::setDefaults(void){

    this->disabled = false;

    this->trunk = false;

    this->access = false;

    this->p2p = false;

    this->announcingSet.push_back(1); //TODO A2 FIX change to some configurable default VLAN

    this->enabledgSet.push_back(1);

    this->nonAdj = false;

    this->inhibitionInterval = 30;

    this->disLearning = false;

    this->vlanId = 1;

    this->inhibited = false;

    this->desigVLAN = 1;

    this->vlanMapping = false;
}

int TrillInterfaceData::getVlanId() const
{
    return vlanId;
}

bool TrillInterfaceData::isAccess() const
{
    return access;
}

bool TrillInterfaceData::isDisLearning() const
{
    return disLearning;
}

bool TrillInterfaceData::isDisabled() const
{
    return disabled;
}

bool TrillInterfaceData::isP2p() const
{
    return p2p;
}

bool TrillInterfaceData::isTrunk() const
{
    return trunk;
}

void TrillInterfaceData::setAccess(bool access)
{
    this->access = access;
}

void TrillInterfaceData::setDisLearning(bool disLearning)
{
    this->disLearning = disLearning;
}

void TrillInterfaceData::setDisabled(bool disabled)
{
    this->disabled = disabled;
}

void TrillInterfaceData::setP2p(bool p2p)
{
    this->p2p = p2p;
}

void TrillInterfaceData::setTrunk(bool trunk)
{
    this->trunk = trunk;
}

void TrillInterfaceData::setVlanId(int vlanId)
{
    this->vlanId = vlanId;
}


bool TrillInterfaceData::isInhibited() const
{
    return inhibited;
}

bool TrillInterfaceData::isEnabled(int vlanId)
{
    for(VLANVector::iterator it = this->enabledgSet.begin(); it != this->enabledgSet.end(); ++it){
        if((*it) == vlanId){
            return true;
        }
    }
    return false;
}

int TrillInterfaceData::getDesigVlan() const
{
    return desigVLAN;
}

bool TrillInterfaceData::isVlanMapping() const
{
    return vlanMapping;
}

int TrillInterfaceData::getDesiredDesigVlan() const
{
    return desiredDesigVLAN;
}

void TrillInterfaceData::setDesiredDesigVlan(int desiredDesigVlan)
{
    desiredDesigVLAN = desiredDesigVlan;
}

void TrillInterfaceData::setVlanMapping(bool vlanMapping)
{
    this->vlanMapping = vlanMapping;
}

void TrillInterfaceData::setDesigVlan(int desigVlan)
{
    desigVLAN = desigVlan;
}

void TrillInterfaceData::setInhibited(bool inhibited)
{
    this->inhibited = inhibited;
}

bool TrillInterfaceData::isAppointedForwarder(int vlanId, TRILLNickname nickname){

    //TODO A2
//    if(nickname == 0){
//        nickname = getNicknameOfThisRBridge();
//    }
    std::map<int, TRILLNickname>::iterator it = this->appointedForwarder.find(vlanId);
    if(it != this->appointedForwarder.end()){
        if((*it).second == nickname){
            return true;
        }
    }else{
        return false;
    }
//    for(VLANVector::iterator iter = appointedForwarder.begin(); iter != appointedForwarder.end(); ++iter){
//        if(vlanId == (*iter)){
//            return true;
//        }
//    }

    return false;
}

void TrillInterfaceData::setAppointedForwarder(int vlanId, TRILLNickname nickname)
{
    this->appointedForwarder.clear();
    this->appointedForwarder.insert(std::make_pair(vlanId, nickname));
}

void TrillInterfaceData::addAppointedForwarder(int vlanId, TRILLNickname nickname)
{
    this->appointedForwarder.insert(std::make_pair(vlanId, nickname));
}

void TrillInterfaceData::clearAppointedForwarder(){
    this->appointedForwarder.clear();
}

/*
 * TODO B2 To be determined if nickname has to match too.
 */
void TrillInterfaceData::removeAppointedFowrwarder(int vlanId, TRILLNickname nickname)
{
    std::map<int, TRILLNickname>::iterator it = this->appointedForwarder.find(vlanId);
    if(it != this->appointedForwarder.end()){

        this->appointedForwarder.erase(it);
    }else{
//        return false;
    }
}

}
