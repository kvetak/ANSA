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
    name = "";
    key = "";
}

LISPSiteInfo::LISPSiteInfo(std::string nam, std::string ke) {
    name = nam;
    key = ke;
}

LISPSiteInfo::~LISPSiteInfo() {
    name = "";
    key = "";
}

const std::string& LISPSiteInfo::getKey() const {
    return key;
}

void LISPSiteInfo::setKey(const std::string& key) {
    this->key = key;
}

const std::string& LISPSiteInfo::getName() const {
    return name;
}

void LISPSiteInfo::setName(const std::string& name) {
    this->name = name;
}

std::string LISPSiteInfo::info() const {
    std::stringstream os;
    os << this->name << ", key: " << this->key << ", proxy-reply: " << proxyReply << endl;
    os << LISPMapStorageBase::info();
    return os.str();
}

std::ostream& operator <<(std::ostream& os, const LISPSiteInfo& si) {
    return os << si.info();
}

bool LISPSiteInfo::isProxyReply() const {
    return proxyReply;
}

void LISPSiteInfo::setProxyReply(bool proxyReply) {
    this->proxyReply = proxyReply;
}
