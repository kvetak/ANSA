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

#ifndef LISPSITEINFO_H_
#define LISPSITEINFO_H_

#include <omnetpp.h>
#include <sstream>
#include <string>
#include "LISPMapStorageBase.h"
#include "LISPCommon.h"

class LISPSiteInfo : public LISPMapStorageBase
{
  public:
    LISPSiteInfo();
    LISPSiteInfo(std::string nam, std::string ke);
    LISPSiteInfo(simtime_t time, std::string nam, std::string ke, std::string reg);
    virtual ~LISPSiteInfo();

    std::string info() const;

    const std::string& getKey() const;
    void setKey(const std::string& key);
    const std::string& getName() const;
    void setName(const std::string& name);
    bool isProxyReply() const;
    void setProxyReply(bool proxyReply);

  private:
    std::string name;
    std::string key;
    bool proxyReply;
};

std::ostream& operator<< (std::ostream& os, const LISPSiteInfo& si);

#endif /* LISPSITEINFO_H_ */
