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

#include "ansa/linklayer/cdp/CDPTimer_m.h"
#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {

class CDPNeighbour : public cObject {

private:
    std::string name;
    int portReceive;
    std::string portSend;
    simtime_t lastUpdate;
    simtime_t ttl;
    CDPTimer *holdtimeTimer;
    bool fullDuplex;
    std::string capabilities;
    std::string platform;
    std::string version;
    InterfaceEntry *interface;
    L3Address *address;
    L3Address *odrDefaultGateway;
    uint16_t nativeVlan;
    std::string vtpDomain;

public:
    CDPNeighbour();
    virtual ~CDPNeighbour();
    virtual std::string info() const override;

    // getters
    std::string getName(){return this->name;}
    int getPortReceive(){return this->portReceive;}
    std::string getPortSend(){return this->portSend;}
    simtime_t getLastUpdate(){return this->lastUpdate;}
    simtime_t getTtl(){return this->ttl;}
    CDPTimer *getHoldtimeTimer(){return this->holdtimeTimer;}
    bool getFullDuplex(){return this->fullDuplex;}
    std::string getCapabilities(){return this->capabilities;}
    std::string getPlatform(){return this->platform;}
    std::string getVersion(){return this->version;}
    InterfaceEntry *getInterface(){return this->interface;}
    L3Address *getAddress(){return this->address;}
    uint16_t getNativeVlan(){return this->nativeVlan;}
    std::string getVtpDomain(){return this->vtpDomain;}

    // setters
    void setName(std::string name){this->name = name;}
    void setPortReceive(int portReceive){this->portReceive = portReceive;}
    void setPortSend(std::string portSend){this->portSend = portSend;}
    void setLastUpdated(simtime_t lastUpdate){this->lastUpdate = lastUpdate;}
    void setTtl(simtime_t ttl){this->ttl = ttl;}
    void setHoldtimeTimer(CDPTimer *holdTimeTimer){this->holdtimeTimer = holdtimeTimer;}
    void setFullDuplex(bool fullDuplex){this->fullDuplex = fullDuplex;}
    void setCapabilities(std::string capabilities){this->capabilities = capabilities;}
    void setPlatform(std::string platform){this->platform = platform;}
    void setVersion(std::string version){this->version = version;}
    void setInterface(InterfaceEntry *interface){this->interface = interface;}
    void setAddress(L3Address *address){this->address = address;}
    void setNativeVlan(uint16_t natVlan){this->nativeVlan = natVlan;}
    void setVtpDomain(std::string domain){this->vtpDomain = domain;}
};


class CDPNeighbourTable
{
  protected:
    std::vector<CDPNeighbour *> neighbours;

  public:
    virtual ~CDPNeighbourTable();

    std::vector<CDPNeighbour *>& getNeighbours() {return neighbours;}
    CDPNeighbour * findNeighbour(std::string name, int port);
    void addNeighbour(CDPNeighbour * neighbour);
    void removeNeighbours();
    void removeNeighbour(CDPNeighbour * neighbour);
    void removeNeighbour(std::string name, int port);
    int countNeighboursOnPort(int portReceive);
};

} /* namespace inet */

#endif /* CDPTABLEENTRY_H_ */
