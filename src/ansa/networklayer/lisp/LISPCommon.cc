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

#include <LISPCommon.h>

const char* ADDRESS_ATTR = "address";
const char* IPV4_ATTR = "ipv4";
const char* IPV6_ATTR = "ipv6";
const char* ENABLED_VAL = "enabled";

LISPCommon::LISPCommon() {
    // TODO Auto-generated constructor stub

}

LISPCommon::~LISPCommon() {
    // TODO Auto-generated destructor stub

}

void LISPCommon::parseIpAddress(const char* str, std::string& address, std::string& length) {
    std::string addr = str;
    int pos = addr.find("/");
    std::string len = addr.substr(pos + 1);
    addr.replace(pos, -1, "");

    address = addr;
    length = len;
}
