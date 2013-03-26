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

#include "CLNSTable.h"


Define_Module(CLNSTable);

unsigned char *CLNSRoute::getDestPrefix() const
{
    return destPrefix;
}


short CLNSRoute::getLength() const
{
    return length;
}

uint32_t CLNSRoute::getMetric() const
{
    return metric;
}

ISISNeighbours_t CLNSRoute::getNextHop() const
{
    return nextHop;
}

void CLNSRoute::setDestPrefix(unsigned char *destPrefix)
{
    this->destPrefix = destPrefix;
}


void CLNSRoute::setLength(short  length)
{
    this->length = length;
}

void CLNSRoute::setMetric(uint32_t metric)
{
    this->metric = metric;
}

void CLNSRoute::setNextHop(ISISNeighbours_t nextHop)
{
    this->nextHop = nextHop;
}

std::string CLNSRoute::info() const{
    std::stringstream out;
    ISISNeighbours_t neig = getNextHop();
    InterfaceEntry *entry;

    int interfaceID = -1;
    entry = (*neig.begin())->entry;
    //print system id

    for (unsigned int i = 0; i < 7; i++)
    {
        out << std::setfill('0') << std::setw(2) << std::hex << (unsigned int) this->destPrefix[i];
        if (i % 2 == 1)
            out << ".";
    }
//    out << std::setfill('0') << std::setw(2) << std::hex << (unsigned int) this->destPrefix[6];

    out << "/" << getLength() << " --> ";
    out << std::dec << " metric: " << (uint32_t) metric << " " << (unsigned int) metric << " via: ";
//    std::cout << " metric: " << std::setfill('0') << std::setw(2) << std::dec << (unsigned int) metric << " " << (unsigned int) metric << " via: ";

    for (ISISNeighbours_t::iterator nIt = neig.begin(); nIt != neig.end(); ++nIt)
    {

        for (unsigned int i = 0; i < 7; i++)
        {
            out << std::setfill('0') << std::setw(2) << std::dec << (unsigned int) (*nIt)->id[i];
            if (i % 2 == 1)
                out << ".";
        }
        out << " if=";
        if((*nIt)->entry != NULL){
            out << (*nIt)->entry->getInterfaceId() << endl;
        }else {
            out << "-1" << endl;
        }
    }
//    if (entry != NULL)
//    {
//        interfaceID = entry->getInterfaceId();
//    }
//    out << "if=" << interfaceID << " next hop:";// << (*neig.begin())->id; // FIXME try printing interface name
//    for (unsigned int i = 0; i < 7; i++)
//                {
//                    out << std::setfill('0') << std::setw(2) << std::dec << (unsigned int) (*neig.begin())->id[i];
//                    if (i % 2 == 1)
//                        out << ".";
//                }

//    out << " " << routeSrcName(getSrc());
//    if (getExpiryTime()>0)
//        out << " exp:" << getExpiryTime();
    return out.str();
}


std::ostream& operator<<(std::ostream& os, const CLNSRoute& e)
{
    os << e.info();
//    os << "ahoj\n";
    return os;
};




CLNSTable::CLNSTable() {
    // TODO Auto-generated constructor stub

}

CLNSTable::~CLNSTable() {
    // TODO Auto-generated destructor stub
}
void CLNSTable::initialize(int stage){
    if (stage==1)
    {
        ift = InterfaceTableAccess().get();
        nb = NotificationBoardAccess().get();

        nb->subscribe(this, NF_INTERFACE_CREATED);
        nb->subscribe(this, NF_INTERFACE_DELETED);
        nb->subscribe(this, NF_INTERFACE_STATE_CHANGED);
        nb->subscribe(this, NF_INTERFACE_CONFIG_CHANGED);
        nb->subscribe(this, NF_INTERFACE_IPv6CONFIG_CHANGED);

        WATCH_VECTOR(table);
        WATCH_PTRVECTOR(routeVector);
        WATCH(test);
//        WATCH_MAP(destCache); // FIXME commented out for now
//        isrouter = par("isRouter");
//        WATCH(isrouter);

    }
    else if (stage==4)
    {
        // configurator adds routes only in stage==3
        //updateDisplayString();
    }
}


void CLNSTable::parseXMLConfigFile()
{
    return;
}

void CLNSTable::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module doesn't process messages");
}

void CLNSTable::receiveChangeNotification(int category, const cObject *details)
{
    if (simulation.getContextType()==CTX_INITIALIZE)
        return;  // ignore notifications during initialize

//    Enter_Method_Silent();
//    printNotificationBanner(category, details);

//    if (category==NF_INTERFACE_CREATED)
//    {
//        // add netmask route for the new interface
//        updateNetmaskRoutes();
//    }
//    else if (category==NF_INTERFACE_DELETED)
//    {
//        // remove all routes that point to that interface
//        InterfaceEntry *entry = check_and_cast<InterfaceEntry*>(details);
//        deleteInterfaceRoutes(entry);
//    }
//    else if (category==NF_INTERFACE_STATE_CHANGED)
//    {
//        invalidateCache();
//    }
//    else if (category==NF_INTERFACE_CONFIG_CHANGED)
//    {
//        invalidateCache();
//    }
//    else if (category==NF_INTERFACE_IPv4CONFIG_CHANGED)
//    {
//        // if anything IPv4-related changes in the interfaces, interface netmask
//        // based routes have to be re-built.
//        updateNetmaskRoutes();
//    }
}

/* This method needs to be rewritten because it has been done in hurry */

void CLNSTable::addRecord(CLNSRoute *route)
{
    //TODO
//    this->routeVector.push_back(route);
//    return;
    for (std::vector<CLNSRoute*>::iterator it = this->routeVector.begin(); it != this->routeVector.end(); ++it)
    {
        //comparison is based on matching destPrefix and length
        if ((*(*it)) == (*route))
        {
            if ((*it)->getMetric() > route->getMetric())
            {
                this->routeVector.erase(it);
                this->routeVector.push_back(route);

            }
            return;
        }
    }

    this->routeVector.push_back(route);


}

void CLNSTable::dropTable(void){

    this->routeVector.clear();
}
