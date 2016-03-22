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

#ifndef CDPTABLEENTRY_H_
#define CDPTABLEENTRY_H_

//#include <omnetpp.h>
#include "CDPTimer_m.h"

namespace inet {

class CDPTableEntry {
public:
    CDPTableEntry();
    virtual ~CDPTableEntry();

    std::string getName(){return this->name;}
    void setName(std::string name){this->name = name;}

    int getPortReceive(){return this->portReceive;}
    void setPortReceive(int portReceive){this->portReceive = portReceive;}

    std::string getPortSend(){return this->portSend;}
    void setPortSend(std::string portSend){this->portSend = portSend;}

    simtime_t getLastUpdate(){return this->lastUpdate;}
    void setLastUpdated(simtime_t lastUpdate){this->lastUpdate = lastUpdate;}

    CDPTimer *getHoldTimeTimer(){return this->holdTimeTimer;}
    void setHoldTimeTimer(CDPTimer *holdTimeTimer){this->holdTimeTimer = holdTimeTimer;}

    bool getFullDuplex(){return this->fullDuplex;}
    void setFullDuplex(bool fullDuplex){this->fullDuplex = fullDuplex;}

    std::string getCapabilities(){return this->capabilities;}
    void setCapabilities(std::string capabilities){this->capabilities = capabilities;}

    std::string getPlatform(){return this->platform;}
    void setPlatform(std::string platform){this->platform = platform;}

    std::string getVersion(){return this->version;}
    void setVersion(std::string version){this->version = version;}

private:
    std::string name;
    int portReceive;
    std::string portSend;
    simtime_t lastUpdate;
    CDPTimer *holdTimeTimer;
    bool fullDuplex;
    std::string capabilities;
    std::string platform;
    std::string version;

};

} /* namespace inet */

#endif /* CDPTABLEENTRY_H_ */
