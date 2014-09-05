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

#include <LISPEidPrefix.h>

LISPEidPrefix::LISPEidPrefix() {
    this->eidLen = DEFAULT_EIDLENGTH_VAL;
}

LISPEidPrefix::LISPEidPrefix(const char* address, const char* length) {
    eid = IPvXAddress(address);
    eidLen = (unsigned char)atoi(length);
    setEidAfi();
}

LISPEidPrefix::~LISPEidPrefix() {
    this->eidLen = DEFAULT_EIDLENGTH_VAL;
    eidAfi = AFI_UKNOWN;
}

const IPvXAddress& LISPEidPrefix::getEid() const {
    return eid;
}

void LISPEidPrefix::setEid(const IPvXAddress& eid) {
    this->eid = eid;
}

unsigned char LISPEidPrefix::getEidLen() const {
    return eidLen;
}

std::string LISPEidPrefix::info() const {
    std::stringstream os;

    os << this->eid << "/" << short(this->eidLen);
    return os.str();

}

void LISPEidPrefix::setEidLen(unsigned char eidLen) {
    this->eidLen = eidLen;
}

bool LISPEidPrefix::operator ==(const LISPEidPrefix& other) const {
    return this->eid == other.eid && this->eidLen == other.eidLen;
}

std::ostream& operator <<(std::ostream& os, const LISPEidPrefix& ep) {
    return os << ep.info();
}

LISPEidPrefix::Afi LISPEidPrefix::getEidAfi() const {
    return eidAfi;
}

void LISPEidPrefix::setEidAfi() {
    if ( eid.isIPv6() )
        eidAfi = AFI_IPV6;
    else
        eidAfi = AFI_IPV4;
}
