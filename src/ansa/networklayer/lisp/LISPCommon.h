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

#ifndef LISPCOMMON_H_
#define LISPCOMMON_H_

#include <string>

extern const char* ADDRESS_ATTR;
extern const char* IPV4_ATTR;
extern const char* IPV6_ATTR;
extern const char* ENABLED_VAL;

class LISPCommon {
public:
    LISPCommon();
    virtual ~LISPCommon();

    static void parseIpAddress(const char* str, std::string &address, std::string &length);
};

#endif /* LISPCOMMON_H_ */
