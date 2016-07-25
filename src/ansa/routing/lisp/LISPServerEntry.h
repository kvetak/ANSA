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


#ifndef LISPSERVERENTRY_H_
#define LISPSERVERENTRY_H_

#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/L3Address.h"
#include "ansa/routing/lisp/LISPCommon.h"

namespace inet {

class LISPServerEntry {
  public:
    LISPServerEntry();
    LISPServerEntry(std::string nipv);
    LISPServerEntry(std::string nipv, std::string nkey,
                    bool proxy, bool notify, bool quick);
    virtual ~LISPServerEntry();

    bool operator== (const LISPServerEntry& other) const;
    bool operator< (const LISPServerEntry& other) const;

    std::string info() const;

    const std::string& getKey() const;
    void setKey(const std::string& key);
    bool isMapNotify() const;
    void setMapNotify(bool mapNotify);
    bool isProxyReply() const;
    void setProxyReply(bool proxyReply);
    bool isQuickRegistration() const;
    void setQuickRegistration(bool quickRegistration);
    const L3Address& getAddress() const;
    void setAddress(const L3Address& address);
    simtime_t getLastTime() const;
    void setLastTime(simtime_t lastTime);

  private:
    L3Address address;
    std::string key;
    bool proxyReply;
    bool mapNotify;
    bool quickRegistration;
    simtime_t lastTime;
};

typedef std::list< LISPServerEntry > ServerAddresses;
typedef ServerAddresses::iterator ServerItem;
typedef ServerAddresses::const_iterator ServerCItem;

//Free function
std::ostream& operator<< (std::ostream& os, const LISPServerEntry& entry);

}
#endif /* LISPSERVERENTRY_H_ */
