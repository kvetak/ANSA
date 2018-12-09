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


#include "ansa/routing/lisp/LISPServerEntry.h"

namespace inet {

LISPServerEntry::LISPServerEntry() {
    address = L3Address();
    key = "";
    proxyReply = false;
    mapNotify  = false;
    quickRegistration = false;
    lastTime = SIMTIME_ZERO;
}

LISPServerEntry::LISPServerEntry(std::string nipv) {
    address = !nipv.empty() ? L3Address(nipv.c_str()) : L3Address();
    key = EMPTY_STRING_VAL;
    proxyReply = false;
    mapNotify  = false;
    quickRegistration = false;
    lastTime = SIMTIME_ZERO;
}

LISPServerEntry::LISPServerEntry(std::string nipv, std::string nkey,
                                 bool proxy, bool notify, bool quick) {
    address = !nipv.empty() ? L3Address(nipv.c_str()) : L3Address();
    key = nkey;
    proxyReply = proxy;
    mapNotify  = notify;
    quickRegistration = quick;
    lastTime = simTime();
}

LISPServerEntry::~LISPServerEntry() {
    address = L3Address();
    key = EMPTY_STRING_VAL;
    proxyReply = false;
    mapNotify = false;
    quickRegistration = false;
    lastTime = SIMTIME_ZERO;
}

const std::string& LISPServerEntry::getKey() const {
    return key;
}

std::ostream& operator <<(std::ostream& os, const LISPServerEntry& entry) {
    return os << entry.info();
}

bool LISPServerEntry::operator <(const LISPServerEntry& other) const {
    if (!(address.getType() == L3Address::Ipv6) && other.address.getType() == L3Address::Ipv6)
        return true;
    else if (address.getType() == L3Address::Ipv6 && !(other.address.getType() == L3Address::Ipv6) )
        return false;
    if (address < other.address)
        return true;
    if (lastTime < other.lastTime)
        return true;
    return false;
}

bool LISPServerEntry::operator ==(const LISPServerEntry& other) const {
    return address == other.address &&
           key == other.key &&
           proxyReply == other.proxyReply &&
           mapNotify == other.mapNotify &&
           quickRegistration == other.quickRegistration &&
           lastTime == other.lastTime;
}

std::string LISPServerEntry::info() const {
    std::stringstream os;
    os << address.str();
    if (!key.empty())
        os << ", key: \"" << key << "\"";
    if (lastTime != SIMTIME_ZERO)
        os << ", last at: " << lastTime;
    if (proxyReply)
        os << ", proxy-reply";
    if (mapNotify)
        os << ", map-notify";
    if (quickRegistration)
        os << ", quick-registration";
    return os.str();
}

void LISPServerEntry::setKey(const std::string& key) {
    this->key = key;
}

bool LISPServerEntry::isMapNotify() const {
    return mapNotify;
}

void LISPServerEntry::setMapNotify(bool mapNotify) {
    this->mapNotify = mapNotify;
}

bool LISPServerEntry::isProxyReply() const {
    return proxyReply;
}

void LISPServerEntry::setProxyReply(bool proxyReply) {
    this->proxyReply = proxyReply;
}

bool LISPServerEntry::isQuickRegistration() const {
    return quickRegistration;
}

void LISPServerEntry::setQuickRegistration(bool quickRegistration) {
    this->quickRegistration = quickRegistration;
}

const L3Address& LISPServerEntry::getAddress() const {
    return address;
}

void LISPServerEntry::setAddress(const L3Address& address) {
    this->address = address;
}

simtime_t LISPServerEntry::getLastTime() const {
    return lastTime;
}

void LISPServerEntry::setLastTime(simtime_t lastTime) {
    this->lastTime = lastTime;
}

}
