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

#include <LISPServerEntry.h>

LISPServerEntry::LISPServerEntry() {
    ipv4 = IPv4Address::UNSPECIFIED_ADDRESS;
    ipv6 = IPv6Address::UNSPECIFIED_ADDRESS;
    key = "";
}

LISPServerEntry::LISPServerEntry(const char* nipv4, const char* nipv6) {
    ipv4 = IPv4Address(nipv4);
    ipv6 = IPv6Address(nipv6);
    key = "";
}

LISPServerEntry::LISPServerEntry(const char* nipv4, const char* nipv6,
        const char* nkey) {
    ipv4 = IPv4Address(nipv4);
    ipv6 = IPv6Address(nipv6);
    key = key;
}


LISPServerEntry::~LISPServerEntry() {
    // TODO Auto-generated destructor stub

}

const IPv4Address& LISPServerEntry::getIpv4() const {
    return ipv4;
}

void LISPServerEntry::setIpv4(const IPv4Address& ipv4) {
    this->ipv4 = ipv4;
}

const IPv6Address& LISPServerEntry::getIpv6() const {
    return ipv6;
}

void LISPServerEntry::setIpv6(const IPv6Address& ipv6) {
    this->ipv6 = ipv6;
}

const std::string& LISPServerEntry::getKey() const {
    return key;
}

bool LISPServerEntry::operator ==(const LISPServerEntry& other) const {
    return ipv4 == other.ipv4 && ipv6 == other.ipv6 && key == other.key;
}

std::string LISPServerEntry::info() const {
    std::stringstream os;
    os << "IPv4: " << ipv4.str() << "\tIPv6: " << ipv6.str();
    if (!this->key.empty())
        os << "\tkey: " << this->key;
    return os.str();
}

void LISPServerEntry::setKey(const std::string& key) {
    this->key = key;
}

std::ostream& operator <<(std::ostream& os, const LISPServerEntry& entry) {
    return os << entry.info();
}
