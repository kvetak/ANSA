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

const int LISP_DEFAULT_PRIORITY = 100;
const int LISP_DEFAULT_WEIGHT = 100;

LISPRLocator::LISPRLocator() {

}

LISPRLocator::LISPRLocator(const char* addr) {
   rloc      = IPvXAddress(addr);
   state     = this->DOWN;
   priority  = LISP_DEFAULT_PRIORITY;
   weight    = LISP_DEFAULT_WEIGHT;
}

LISPRLocator::LISPRLocator(const char* addr, const char* prio, const char* wei) {
    rloc      = IPvXAddress(addr);
    state     = this->DOWN;
    priority  = (short)atoi(prio);
    weight    = (short)atoi(wei);
}

LISPRLocator::LISPRLocator(IPvXAddress addr) {
    rloc      = addr;
    state     = this->DOWN;
    priority  = LISP_DEFAULT_PRIORITY;
    weight    = LISP_DEFAULT_WEIGHT;
}

LISPRLocator::LISPRLocator(IPvXAddress addr, short prio, short wei) {
    rloc      = addr;
    state     = this->DOWN;
    priority  = prio;
    weight    = wei;
}

LISPRLocator::LISPRLocator(IPvXAddress addr, LocatorState stat, short prio, short wei) {
    rloc      = addr;
    state     = stat;
    priority  = prio;
    weight    = wei;
}

LISPRLocator::~LISPRLocator() {
    state     = DOWN;
    priority  = LISP_DEFAULT_PRIORITY;
    weight    = LISP_DEFAULT_WEIGHT;
}

short LISPRLocator::getPriority() const {
    return priority;
}

void LISPRLocator::setPriority(short priority) {
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

short LISPRLocator::getWeight() const {
    return weight;
}

bool LISPRLocator::operator ==(const LISPRLocator& other) const {
    return this->priority == other.priority && this->state == other.state &&
           this->weight == other.weight && this->rloc == other.rloc;
}

std::string LISPRLocator::info() const {
    std::stringstream os;
    os << this->rloc << "\t(" << this->state << ")\t" << this->priority << "/" << weight;
    return os.str();
}

void LISPRLocator::setWeight(short weight) {
    this->weight = weight;
}

std::ostream& operator <<(std::ostream& os, const LISPRLocator& locator) {
    return os << locator.info();
}

