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

#ifndef LLDPNEIGHBOURTABLE_H_
#define LLDPNEIGHBOURTABLE_H_

#include <omnetpp.h>
#include "inet/common/INETDefs.h"

namespace inet {

class INET_API LLDPNeighbour : public cObject
{
protected:
    std::string chassisId;
    int portId;
    const char *ttl;        //?
    const char *portDes;
    const char *systemName;
    const char *systemDes;
    const char *systemCap;
    const char *managementAdd;
    const char *organizationallySpec;




public:
    LLDPNeighbour();
    virtual ~LLDPNeighbour();

    virtual std::string info() const;
    virtual std::string detailedInfo() const {return info();}
    friend std::ostream& operator<<(std::ostream& os, const LLDPNeighbour& i)
    {
        return os << i.info();
    }

    std::string getChassisId() {return this->chassisId;}
    int getPortId() {return this->portId;}

    void setChassisId(std::string c) {this->chassisId = c;}
    void setPortId(int p) {portId = p;}

};

class LLDPNeighbourTable {
protected:
    std::vector<LLDPNeighbour *> neighbours;

public:
    LLDPNeighbourTable();
    virtual ~LLDPNeighbourTable();

    std::vector<LLDPNeighbour *>& getAgents() {return neighbours;}
    LLDPNeighbour *findAgentByMSAP(std::string chassisId, int portId);
    void addNeighbour(LLDPNeighbour * neighbour);
    void removeNeighbour(LLDPNeighbour * neighbour);
    void removeNeighbour(std::string chassisId, int portId);
    std::string printStats();
};

} /* namespace inet */

#endif /* LLDPNEIGHBOURTABLE_H_ */
