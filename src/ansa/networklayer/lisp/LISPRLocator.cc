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

#include <LISPRLocator.h>

LISPRLocator::LISPRLocator() :
    //FIXME: Create IPvXAddress:: unspecified address const
    rloc (IPv4Address::UNSPECIFIED_ADDRESS),
    state(DOWN),
    priority(DEFAULT_PRIORITY_VAL), weight(DEFAULT_WEIGHT_VAL),
    mpriority(DEFAULT_MPRIORITY_VAL), mweight(DEFAULT_MWEIGHT_VAL)
{
}

LISPRLocator::LISPRLocator(const char* addr) :
    rloc( addr ),
    state(DOWN),
    priority(DEFAULT_PRIORITY_VAL), weight(DEFAULT_WEIGHT_VAL),
    mpriority(DEFAULT_MPRIORITY_VAL), mweight(DEFAULT_MWEIGHT_VAL)
{
}

LISPRLocator::LISPRLocator(const char* addr, const char* prio, const char* wei) :
    rloc( IPvXAddress(addr) ),
    state(DOWN),
    priority((unsigned char)atoi(prio)), weight((unsigned char)atoi(wei)),
    mpriority(DEFAULT_MPRIORITY_VAL), mweight(DEFAULT_MWEIGHT_VAL)
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

const IPvXAddress& LISPRLocator::getRloc() const {
    return rloc;
}

void LISPRLocator::setRloc(const IPvXAddress& rloc) {
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
    return this->priority == other.priority && this->state == other.state &&
           this->weight == other.weight && this->rloc == other.rloc;
}

std::string LISPRLocator::info() const {
    std::stringstream os;
    os << this->rloc << "\t(" << this->getStateString() << ")\t" << short(this->priority) << "/" << short(weight);
    return os.str();
}

std::string LISPRLocator::getStateString() const {
    switch(this->getState()) {
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

std::string TLocator::info() const {
    std::stringstream os;
    os << this->RLocator.getRloc()
       << ", priority: " << short(this->RLocator.getPriority())
       << ", weight: " << short(this->RLocator.getWeight())
       << ", Local: " << this->LocalLocBit
       << ", probing: " << this->piggybackBit
       << ", Reachable: " << this->RouteRlocBit;
    return os.str();
}

std::ostream& operator <<(std::ostream& os, const TLocator& tloc) {
    return os << tloc.info();
}
