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

#include "LISPSiteInfo.h"

LISPSiteInfo::LISPSiteInfo() {
    lastTime = 0;
    name = "";
    key = "";
    registredBy = "";
}

LISPSiteInfo::LISPSiteInfo(std::string nam, std::string ke) {
    lastTime = 0;
    name = nam;
    key = ke;
    registredBy = "";
}

LISPSiteInfo::LISPSiteInfo(simtime_t time, std::string nam, std::string ke, std::string reg) {
    lastTime = time;
    name = nam;
    key = ke;
    registredBy = reg;
}


LISPSiteInfo::~LISPSiteInfo() {
    lastTime = 0;
    name = "";
    key = "";
    registredBy = "";

}

const std::string& LISPSiteInfo::getKey() const {
    return key;
}

void LISPSiteInfo::setKey(const std::string& key) {
    this->key = key;
}

const simtime_t& LISPSiteInfo::getLastTime() const {
    return lastTime;
}

void LISPSiteInfo::setLastTime(const simtime_t& lastTime) {
    this->lastTime = lastTime;
}

const std::string& LISPSiteInfo::getName() const {
    return name;
}

void LISPSiteInfo::setName(const std::string& name) {
    this->name = name;
}

const std::string& LISPSiteInfo::getRegistredBy() const {
    return registredBy;
}

void LISPSiteInfo::setRegistredBy(const std::string& registredBy) {
    this->registredBy = registredBy;
}

std::string LISPSiteInfo::info() const {
    std::stringstream os;
    os << this->name << ", key: " << this->key
       << ", lastTime: " << this->lastTime
       << ", registredBy: " << this->registredBy << endl;
    os << LISPMapStorageBase::info();
    return os.str();
}

std::ostream& operator <<(std::ostream& os, const LISPSiteInfo& si) {
    return os << si.info();
}
