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
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */

#include "ansa/routing/lisp/LISPRLocator.h"

namespace inet {

LISPRLocator::LISPRLocator() :
    //FIXME: Create L3Address:: unspecified address const
    rloc (Ipv4Address::UNSPECIFIED_ADDRESS),
    state(DOWN),
    priority(DEFAULT_PRIORITY_VAL), weight(DEFAULT_WEIGHT_VAL),
    mpriority(DEFAULT_MPRIORITY_VAL), mweight(DEFAULT_MWEIGHT_VAL),
    local(false)
{
}

LISPRLocator::LISPRLocator(const char* addr) :
    rloc( addr ),
    state(DOWN),
    priority(DEFAULT_PRIORITY_VAL), weight(DEFAULT_WEIGHT_VAL),
    mpriority(DEFAULT_MPRIORITY_VAL), mweight(DEFAULT_MWEIGHT_VAL),
    local(false)
{
}

LISPRLocator::LISPRLocator(const char* addr, const char* prio, const char* wei, bool loca) :
    rloc( L3Address(addr) ),
    state(DOWN),
    priority((unsigned char)atoi(prio)), weight((unsigned char)atoi(wei)),
    mpriority(DEFAULT_MPRIORITY_VAL), mweight(DEFAULT_MWEIGHT_VAL),
    local(loca)
{
}

LISPRLocator::~LISPRLocator() {
    state     = DOWN;
    priority  = DEFAULT_PRIORITY_VAL;
    weight    = DEFAULT_WEIGHT_VAL;
    mpriority = DEFAULT_MPRIORITY_VAL;
    mweight   = DEFAULT_MWEIGHT_VAL;
}

unsigned char LISPRLocator::getPriority() const {
    return priority;
}

void LISPRLocator::setPriority(unsigned char priority) {
    this->priority = priority;
}

const L3Address& LISPRLocator::getRlocAddr() const {
    return rloc;
}

void LISPRLocator::setRlocAddr(const L3Address& rloc) {
    this->rloc = rloc;
}

LISPRLocator::LocatorState LISPRLocator::getState() const {
    return state;
}

void LISPRLocator::setState(LocatorState state) {
    this->state = state;
}

unsigned char LISPRLocator::getWeight() const {
    return weight;
}

bool LISPRLocator::operator ==(const LISPRLocator& other) const {
    return this->rloc == other.rloc &&
           this->priority == other.priority &&
           this->weight == other.weight &&
           this->mpriority == other.mpriority &&
           this->mweight == other.mweight &&
           this->state == other.state;

}

bool LISPRLocator::operator ==(const LISPRLocator& other) {
    return this->rloc == other.rloc &&
           this->priority == other.priority &&
           this->weight == other.weight &&
           this->mpriority == other.mpriority &&
           this->mweight == other.mweight &&
           this->state == other.state;
}

std::string LISPRLocator::info() const {
    std::stringstream os;
    os << this->rloc << "\t(" << getStateString() << ")\t"
       << "pri/wei=" << short(priority) << "/" << short(weight);
    if (local)
        os << "\tLocal";
    if (mpriority != DEFAULT_MPRIORITY_VAL && mweight != DEFAULT_MWEIGHT_VAL )
        os << "mpri/mwei=" << short(mpriority) << "/" << short(mweight);
    return os.str();
}

std::string LISPRLocator::getStateString() const {
    switch(state) {
        case UP:
            return "up";
        case ADMIN_DOWN:
            return "admin-down";
        case DOWN:
        default:
            return "down";
    }
}

void LISPRLocator::setWeight(unsigned char weight) {
    this->weight = weight;
}

std::ostream& operator <<(std::ostream& os, const LISPRLocator& locator) {
    return os << locator.info();
}

unsigned char LISPRLocator::getMpriority() const {
    return mpriority;
}

void LISPRLocator::setMpriority(unsigned char mpriority) {
    this->mpriority = mpriority;
}

unsigned char LISPRLocator::getMweight() const {
    return mweight;
}

void LISPRLocator::setMweight(unsigned char mweight) {
    this->mweight = mweight;
}

bool LISPRLocator::isLocal() const {
    return local;
}

void LISPRLocator::setLocal(bool local) {
    this->local = local;
}

void LISPRLocator::updateRlocator(const LISPRLocator& rloc) {
    state = rloc.getState();
    priority = rloc.getPriority();
    weight = rloc.getWeight();
    mpriority = rloc.getMpriority();
    mweight = rloc.getMweight();
    local = rloc.isLocal();
}

bool LISPRLocator::operator< (const LISPRLocator& other) const {
    if ( !(rloc.getType() == L3Address::Ipv6) && (other.rloc.getType() == L3Address::Ipv6) )
        return true;
    else if ( (rloc.getType() == L3Address::Ipv6) && !(other.rloc.getType() == L3Address::Ipv6) )
        return false;

    if (rloc < other.rloc) return true;

    if (state < other.state) return true;

    if (priority < other.priority) return true;

    if (weight < other.weight) return true;

    if (mpriority < other.mpriority) return true;

    if (mweight < other.mweight) return true;

    return false;
}

LISPCommon::Afi LISPRLocator::getRlocAfi() const {
    return rloc.getType() == L3Address::Ipv6 ? LISPCommon::AFI_IPV6 : LISPCommon::AFI_IPV4;
}

}
