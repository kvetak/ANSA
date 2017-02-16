//
// Copyright (C) 2009 - today, Brno University of Technology, Czech Republic
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
* @file CDPNeighbourTable.h
* @author Tomas Rajca
*/

#ifndef CDPTABLEENTRY_H_
#define CDPTABLEENTRY_H_

#include "ansa/linklayer/cdp/CDPTimer_m.h"
#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {

/**
 * Class holding information about a neighbouring device.
 * Device are identified by the name and port which they
 * are connected
 */
class INET_API CDPNeighbour : public cObject {
    friend class CDPNeighbourTable;

private:
    std::string name;
    int portReceive;
    std::string portSend;
    simtime_t lastUpdate;
    simtime_t ttl;
    bool fullDuplex;
    std::string capabilities;
    std::string platform;
    std::string version;
    InterfaceEntry *interface;
    L3Address *address;
    uint16_t nativeVlan;
    std::string vtpDomain;

    CDPTimer *holdtimeTimer;

public:
    CDPNeighbour();
    virtual ~CDPNeighbour();
    virtual std::string info() const override;
    friend std::ostream& operator<<(std::ostream& os, const CDPNeighbour& e)
    {
        return os << e.info();
    }

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
    void setHoldtimeTimer(CDPTimer *holdTimeTimer){this->holdtimeTimer = holdTimeTimer;}
    void setFullDuplex(bool fullDuplex){this->fullDuplex = fullDuplex;}
    void setCapabilities(std::string capabilities){this->capabilities = capabilities;}
    void setPlatform(std::string platform){this->platform = platform;}
    void setVersion(std::string version){this->version = version;}
    void setInterface(InterfaceEntry *interface){this->interface = interface;}
    void setAddress(L3Address *address){this->address = address;}
    void setNativeVlan(uint16_t natVlan){this->nativeVlan = natVlan;}
    void setVtpDomain(std::string domain){this->vtpDomain = domain;}
};


/**
 * Class holding informatation about neighbouring devices.
 * Devices are identified by the name and port which they
 * are connected
 */
class INET_API CDPNeighbourTable : public cSimpleModule
{
  protected:
    std::vector<CDPNeighbour *> neighbours;

    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *) override;
  public:
    virtual ~CDPNeighbourTable();

    std::vector<CDPNeighbour *>& getNeighbours() {return neighbours;}

    /**
     * Get cdp neighbour from neighbour table by name and receive port
     *
     * @param   name    name of neighbour
     * @param   port    receive port number
     *
     * @return  cdp neighbour
     */
    CDPNeighbour * findNeighbour(std::string name, int port);

    /**
     * Adds the a neighbor to the table. The operation might fail
     * if there is a neighbour with the same (ie,address) in the table.
     *
     * @param   neighbour   neighbour to add
     */
    void addNeighbour(CDPNeighbour * neighbour);

    /**
     * Remove all neighbours from the table.
     */
    void removeNeighbours();

    /**
     * Remove a neighbour from the table.
     * If the neighbour was not found in the table then it is untouched,
     * otherwise deleted.
     *
     * @param   neighbour   neighbour to delete
     */
    void removeNeighbour(CDPNeighbour * neighbour);

    /**
     * Remove a neighbour identified by name and port from the table.
     * If the neighbour was not found in the table then it is untouched,
     * otherwise deleted.
     *
     * @param   name    name of neighbour
     * @param   port    receive port number
     */
    void removeNeighbour(std::string name, int port);

    /**
     * Count neighbours learned from specified port
     *
     * @param   portReceive
     */
    int countNeighboursOnPort(int portReceive);
};

} /* namespace inet */

#endif /* CDPTABLEENTRY_H_ */
