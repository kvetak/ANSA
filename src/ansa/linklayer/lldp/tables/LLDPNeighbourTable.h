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
* @file LLDPNeighbourTable.h
* @author Tomas Rajca
*/

#ifndef LLDPNEIGHBOURTABLE_H_
#define LLDPNEIGHBOURTABLE_H_

#include <omnetpp.h>
#include "inet/common/INETDefs.h"
#include "ansa/linklayer/lldp/tables/LLDPAgentTable.h"

namespace inet {

/**
 * Class holding information about a LLDP neighbour.
 */
class INET_API LLDPNeighbour: public cObject
{
    friend class LLDPNeighbourTable;
protected:
    cMessage *rxInfoTtl;
    LLDPAgent *agent;

    std::string chassisId;
    std::string portId;
    std::string msap;

    simtime_t lastUpdate;
    uint16_t ttl;
    std::string portDes;
    std::string systemName;
    std::string systemDes;
    std::string systemCap;      // system capabilities
    std::string enabledCap;       // enabled capabilities
    LLDPManAddTab managementAdd;
    int mtu;

public:
    LLDPNeighbour(LLDPAgent *agent, const char *cId, const char *pId);
    virtual ~LLDPNeighbour();

    virtual std::string info() const override;
    virtual std::string detailedInfo() const override {return info();}
    friend std::ostream& operator<<(std::ostream& os, const LLDPNeighbour& i)
    {
        return os << i.info();
    }

    // getters
    std::string getChassisId() {return this->chassisId;}
    std::string getPortId() {return this->portId;}
    cMessage *getRxInfoTtl() {return this->rxInfoTtl;}
    simtime_t getLastUpdate() {return this->lastUpdate;}
    std::string getSystemName() {return this->systemName;}
    uint16_t getTtl() {return this->ttl;}
    LLDPAgent *getAgent() {return this->agent;}
    LLDPManAddTab getManagementAdd() {return managementAdd;}
    int getMtu() {return mtu;}
    std::string getMsap(){return msap;}

    // setters
    void setChassisId(const char *c) {this->chassisId = c;}
    void setPortId(const char *p) {this->portId = p;}
    void setLastUpdate(simtime_t l) {this->lastUpdate = l;}
    void setTtl(uint16_t t) {this->ttl = t;}
    void setPortDes(std::string p) {this->portDes = p;}
    void setSystemName(std::string s) {this->systemName = s;}
    void setSystemDes(std::string s) {this->systemDes = s;}
    void setSystemCap(std::string s) {this->systemCap = s;}
    void setEnabledCap(std::string e) {this->enabledCap = e;}
    void setMtu(int m) {this->mtu = m;}
};


/**
 * Class holding information about a LLDP neighbours.
 * Expired entries are automatically deleted.
 */
class INET_API LLDPNeighbourTable : public cSimpleModule
{
protected:
    std::vector<LLDPNeighbour *> neighbours;

    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *) override;

public:
    enum TimerKind {
        ttl = 1
    };
    LLDPNeighbourTable();
    virtual ~LLDPNeighbourTable();
    std::vector<LLDPNeighbour *>& getNeighbours() {return neighbours;}

    /**
     * Returns the neighbour with the specified MSAP (chassis ID + port ID).
     */
    LLDPNeighbour *findNeighbourByMSAP(std::string msap);

    /**
     * Created and adds the a neighbour to the table. The operation might fail
     * if in the table is already neighbour with the same MSAP
     */
    LLDPNeighbour *addNeighbour(LLDPAgent *agent, std::string chassisId, std::string portId);

    /**
     * Removes a neighbour from the table. If the neighbour was
     * not found in the table then it is untouched, otherwise deleted.
     */
    void removeNeighbour(LLDPNeighbour * neighbour);

    /**
     * Removes a neighbour specified with MSAP from the table. If the neighbour was
     * not found in the table then it is untouched, otherwise deleted.
     */
    void removeNeighbour(std::string msap);

    /**
     * Removes all neighbours which where learned with specified agent.
     */
    void removeNeighboursByAgent(LLDPAgent *ag);

    /*
     * Restarts neighbour TTL timer to specified holdtime
     */
    void restartRxInfoTtl(LLDPNeighbour *neighbour, uint16_t holdTime);
};

} /* namespace inet */

#endif /* LLDPNEIGHBOURTABLE_H_ */
