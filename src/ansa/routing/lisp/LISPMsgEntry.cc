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


#include "ansa/routing/lisp/LISPMsgEntry.h"

namespace inet {

LISPMsgEntry::LISPMsgEntry(
        LISPMsgEntry::EMsgType ntyp, unsigned long nnonce, L3Address addr, simtime_t processed, bool fl, int64 siz) :
        type(ntyp), nonce(nnonce), address(addr), processedAt(processed), flag(fl), msgsize(siz)
{
}

LISPMsgEntry::~LISPMsgEntry() {
   nonce = DEFAULT_NONCE_VAL;
   type = LISPMsgEntry::UNKNOWN;
   processedAt = SIMTIME_ZERO;
   flag = false;
   msgsize = 0;
}

unsigned long LISPMsgEntry::getNonce() const {
    return nonce;
}

void LISPMsgEntry::setNonce(unsigned long nonce) {
    this->nonce = nonce;
}

const simtime_t& LISPMsgEntry::getProcessedAt() const {
    return processedAt;
}

void LISPMsgEntry::setProcessedAt(const simtime_t& processedAt) {
    this->processedAt = processedAt;
}

std::string LISPMsgEntry::info() const {
    std::stringstream os;
    os << getTypeString() << " (" << getNonce() << ") ";

    if (!flag) os << " received from ";
    else os << " sent to ";

    os << address;
    os << " of size " << msgsize << "B ";
    os << " at " << getProcessedAt();
    return os.str();
}

std::ostream& operator <<(std::ostream& os, const LISPMsgEntry& entry) {
    return os << entry.info();
}

LISPMsgEntry::EMsgType LISPMsgEntry::getType() const {
    return type;
}

void LISPMsgEntry::setType(EMsgType type) {
    this->type = type;
}

bool LISPMsgEntry::operator ==(const LISPMsgEntry& other) const {
    return this->nonce == other.nonce &&
           this->type == other.type &&
           this->processedAt == other.processedAt;
}

std::string LISPMsgEntry::getTypeString() const {
    switch(type) {
      case LISPMsgEntry::REQUEST:
        return "Map-Request";
      case LISPMsgEntry::REPLY:
        return "Map-Reply";
      case LISPMsgEntry::REGISTER:
        return "Map-Register";
      case LISPMsgEntry::NOTIFY:
        return "Map-Notify";
      case LISPMsgEntry::RLOC_PROBE:
          return "RLOC-Probe";
      case LISPMsgEntry::RLOC_PROBE_REPLY:
          return "RLOC-Probe-Reply";
      case LISPMsgEntry::DATA:
          return "Data";
      case LISPMsgEntry::UNKNOWN:
      default:
        return "unknown";
    }
}

const L3Address& LISPMsgEntry::getAddress() const {
    return address;
}

void LISPMsgEntry::setAddress(const L3Address& destination) {
    this->address = destination;
}

bool LISPMsgEntry::isFlag() const {
    return flag;
}

void LISPMsgEntry::setFlag(bool flag) {
    this->flag = flag;
}

}
