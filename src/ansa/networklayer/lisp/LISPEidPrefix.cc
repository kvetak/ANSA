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
 */

#include "ansa/networklayer/lisp/LISPEidPrefix.h"

LISPEidPrefix::LISPEidPrefix() {
    eidAddr = inet::IPv4Address::UNSPECIFIED_ADDRESS;
    eidLen = DEFAULT_EIDLENGTH_VAL;
    eidNetwork = LISPCommon::getNetworkAddress(eidAddr, eidLen);
}

LISPEidPrefix::LISPEidPrefix(const char* address, const char* length) {
    eidAddr = inet::L3Address(address);
    eidLen = (unsigned char)atoi(length);
    eidNetwork = LISPCommon::getNetworkAddress(eidAddr, eidLen);
}

LISPEidPrefix::LISPEidPrefix(inet::L3Address address, unsigned char length) {
    eidAddr = address;
    eidLen = length;
    eidNetwork = LISPCommon::getNetworkAddress(eidAddr, eidLen);
}

LISPEidPrefix::~LISPEidPrefix() {
    eidAddr = inet::L3Address();
    eidNetwork = inet::L3Address();
    eidLen = DEFAULT_EIDLENGTH_VAL;
}

const inet::L3Address& LISPEidPrefix::getEidAddr() const {
    return eidNetwork;
}

void LISPEidPrefix::setEidAddr(const inet::L3Address& eid) {
    this->eidAddr = eid;
}

unsigned char LISPEidPrefix::getEidLength() const {
    return eidLen;
}

std::string LISPEidPrefix::info() const {
    std::stringstream os;

    /*
    if (eid.isIPv6() && eid == inet::IPv6Address::UNSPECIFIED_ADDRESS)
        os << "::0";
    else if (!eid.isIPv6() && eid == inet::IPv4Address::UNSPECIFIED_ADDRESS)
        os << "0.0.0.0";
    else
    */
        os << eidAddr;
    os << "/" << short(this->eidLen);
    return os.str();

}

void LISPEidPrefix::setEidLength(unsigned char eidLen) {
    this->eidLen = eidLen;
}

bool LISPEidPrefix::operator ==(const LISPEidPrefix& other) const {
    return this->eidAddr == other.eidAddr && this->eidLen == other.eidLen;
}

std::ostream& operator <<(std::ostream& os, const LISPEidPrefix& ep) {
    return os << ep.info();
}

LISPCommon::Afi LISPEidPrefix::getEidAfi() const {
    return (eidAddr.getType()==inet::L3Address::IPv6) ? LISPCommon::AFI_IPV6 : LISPCommon::AFI_IPV4;
}

bool LISPEidPrefix::operator <(const LISPEidPrefix& other) const {
    if (getEidAfi() < other.getEidAfi()) return true;
    //if (eidAfi > other.eidAfi) return false;

    if (eidAddr < other.eidAddr) return true;
    //if (eidAddr > other.eidAddr) return false;

    if (eidLen < other.eidLen) return true;
    //if (eidLen > other.eidLen) return false;

    return false;
}

bool LISPEidPrefix::isComponentOf(const LISPEidPrefix& coarserEid) const {
    //Component has coarser mask or AFIs do not match
    if (eidLen > coarserEid.eidLen || getEidAfi() != coarserEid.getEidAfi())
        return false;

    int result = LISPCommon::doPrefixMatch(eidAddr, coarserEid.eidAddr);

    return (result == -1 || result >= coarserEid.eidLen ) ? true : false;
}

