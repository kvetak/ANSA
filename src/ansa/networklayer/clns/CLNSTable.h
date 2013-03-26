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

#ifndef CLNSTABLE_H_
#define CLNSTABLE_H_

//#include "INETDefs.h"

#include <iomanip>
#include <csimplemodule.h>
#include "INotifiable.h"
#include "NotificationBoard.h"
#include <vector>
#include "InterfaceTableAccess.h"
#include "ISIStypes.h" //TODO delete -> this imply reverted dependency

class IInterfaceTable;
class InterfaceEntry;

/* It will get rename to CLNPRoute or RoutingTable under CLNP */

class CLNSRoute: public cObject {

private:

    unsigned char *destPrefix; //system-ID, in future maybe even area-id
    short length; //length of destPrefix for matching
    /* TODO change to more general type */
    ISISNeighbours_t nextHop; //vector with nextHop system-IDs -> vector because of load balancing
    uint32_t metric;

public:
    CLNSRoute(unsigned char *destPrefix, int length, ISISNeighbours_t nextHop, uint32_t metric){
        this->destPrefix = new unsigned char [ISIS_SYSTEM_ID];
        memcpy(this->destPrefix, destPrefix, ISIS_SYSTEM_ID + 1);

        this->length = length;
//        this->nextHop = nextHop;

        for(ISISNeighbours_t::iterator it = nextHop.begin(); it != nextHop.end(); ++it){
            this->nextHop.push_back((*it)->copy());
//            EV<< (*it)->id;
        }

        this->metric = metric;


    }

    bool operator==(const CLNSRoute& route) const {
        if(length == route.length && memcmp(destPrefix, route.destPrefix, length) == 0){
            return true;
        }else {
            return false;
        }

    }

    virtual std::string info() const;


    unsigned char *getDestPrefix() const;
    short getLength() const;
    uint32_t getMetric() const;
    ISISNeighbours_t getNextHop() const;
    void setDestPrefix(unsigned char *destPrefix);
    void setLength(short  length);
    void setMetric(uint32_t metric);
    void setNextHop(ISISNeighbours_t nextHop);


};

class CLNSTable: public cSimpleModule, protected INotifiable {
protected:
    IInterfaceTable *ift; // cached pointer
    NotificationBoard *nb; // cached pointer
    int test;
    std::vector<int> table;
    typedef std::vector<CLNSRoute*> RouteVector;
    RouteVector routeVector;

    virtual int numInitStages() const  {return 5;}
    virtual void initialize(int stage);
    /* TODO */
    virtual void parseXMLConfigFile();

    /**
     * Raises an error.
     */
    virtual void handleMessage(cMessage *);

    virtual void receiveChangeNotification(int category, const cObject *details);
public:
    CLNSTable();
    virtual ~CLNSTable();

    /* add route to routeVector - no checking */
    void addRecord(CLNSRoute *route);
    /* delete all entries in routeVector */
    void dropTable(void);
};

#endif /* CLNSTABLE_H_ */
