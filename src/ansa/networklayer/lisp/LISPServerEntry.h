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

#ifndef LISPSERVERENTRY_H_
#define LISPSERVERENTRY_H_

#include <omnetpp.h>
#include "IPvXAddress.h"

class LISPServerEntry {
  public:
    LISPServerEntry();
    LISPServerEntry(const char* nipv4, const char* nipv6);
    LISPServerEntry(const char* nipv4, const char* nipv6, const char* nkey);
    virtual ~LISPServerEntry();

    bool operator== (const LISPServerEntry& other) const;

    std::string info() const;

    const IPv4Address& getIpv4() const;
    void setIpv4(const IPv4Address& ipv4);
    const IPv6Address& getIpv6() const;
    void setIpv6(const IPv6Address& ipv6);
    const std::string& getKey() const;
    void setKey(const std::string& key);

  private:
    IPv4Address ipv4;
    IPv6Address ipv6;
    std::string key;
};

//Free function
std::ostream& operator<< (std::ostream& os, const LISPServerEntry& entry);

#endif /* LISPSERVERENTRY_H_ */
