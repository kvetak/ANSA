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

class LLDPNeighbour
{
//    friend class LLDPNeighbourTable;
protected:
    const char* chassisId;
    const char* portId;
    const char* msap;
    /*  const char *ttl;
    const char *portDes;
    const char *systemName;
    const char *systemDes;
    const char *systemCap;
    const char *managementAdd;
    const char *organizationallySpec;*/




public:
    LLDPNeighbour();
    //LLDPNeighbour(const char *cId, const char *pId);
    virtual ~LLDPNeighbour();

    virtual std::string info() const;
    virtual std::string detailedInfo() const {return info();}
    friend std::ostream& operator<<(std::ostream& os, const LLDPNeighbour& i)
    {
        return os << i.info();
    }

    const char* getChassisId() {return this->chassisId;}
    const char* getPortId() {return this->portId;}

    void setChassisId(const char* c) {this->chassisId = c;}
    void setPortId(const char* p) {portId = p;}

    const char *getMsap(){return msap;}

};


class LLDPNeighbourTable
{
protected:
    std::vector<LLDPNeighbour *> neighbours;
public:
    LLDPNeighbourTable();
    virtual ~LLDPNeighbourTable();

    std::vector<LLDPNeighbour *>& getNeighbours() {return neighbours;}
    LLDPNeighbour *findNeighbourByMSAP(const char* msap);
    void addNeighbour(LLDPNeighbour * neighbour);
    void removeNeighbour(LLDPNeighbour * neighbour);
    void removeNeighbour(const char* msap);
    std::string printStats();
};

} /* namespace inet */

#endif /* LLDPNEIGHBOURTABLE_H_ */
