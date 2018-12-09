// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
 * @file CLNSTable.cc
 * @author Marcel Marek (mailto:xscrew02@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 3.2.2013
 * @brief Holds CLNS routes.
 * @detail Holds CLNS routes.
 * @todo TODO A2 Add hopCount for TRILL.
 */
#include "ansa/networklayer/clns/CLNSTable.h"

namespace inet {

//Define_Module(CLNSTable);

//unsigned char *CLNSRoute::getDestPrefix() const
//{
//    return destPrefix;
//}
//
//
//short CLNSRoute::getLength() const
//{
//    return length;
//}
//
//uint32_t CLNSRoute::getMetric() const
//{
//    return metric;
//}
//
//ISISNeighbours_t CLNSRoute::getNextHop() const
//{
//    return nextHop;
//}
//
//void CLNSRoute::setDestPrefix(unsigned char *destPrefix)
//{
//    this->destPrefix = destPrefix;
//}
//
//
//void CLNSRoute::setLength(short  length)
//{
//    this->length = length;
//}
//
//void CLNSRoute::setMetric(uint32_t metric)
//{
//    this->metric = metric;
//}
//
//void CLNSRoute::setNextHop(ISISNeighbours_t nextHop)
//{
//    this->nextHop = nextHop;
//}
//
//std::string CLNSRoute::info() const{
//    std::stringstream out;
//    ISISNeighbours_t neig = getNextHop();
////    InterfaceEntry *entry;
//
////    int interfaceID = -1;
////    entry = (*neig.begin())->entry;
//    //print system id
//
//    for (unsigned int i = 0; i < 7; i++)
//    {
//        out << std::setfill('0') << std::setw(2) << std::dec << (unsigned int) this->destPrefix[i];
//        if (i % 2 == 1)
//            out << ".";
//    }
////    out << std::setfill('0') << std::setw(2) << std::hex << (unsigned int) this->destPrefix[6];
//
//    out << "/" << getLength() << " --> ";
//    out << std::dec << " metric: " << (unsigned int) metric << " via: ";
////    std::cout << " metric: " << std::setfill('0') << std::setw(2) << std::dec << (unsigned int) metric << " " << (unsigned int) metric << " via: ";
//
//    for (ISISNeighbours_t::iterator nIt = neig.begin(); nIt != neig.end(); ++nIt)
//    {
//
//        for (unsigned int i = 0; i < 7; i++)
//        {
//            out << std::setfill('0') << std::setw(2) << std::dec << (unsigned int) (*nIt)->id[i];
//            if (i % 2 == 1)
//                out << ".";
//        }
//        out << " if=";
//        if((*nIt)->entry != NULL){
//            out << (*nIt)->entry->getInterfaceId() << endl;
//        }else {
//            out << "-1" << endl;
//        }
//    }
////    if (entry != NULL)
////    {
////        interfaceID = entry->getInterfaceId();
////    }
////    out << "if=" << interfaceID << " next hop:";// << (*neig.begin())->id; // FIXME try printing interface name
////    for (unsigned int i = 0; i < 7; i++)
////                {
////                    out << std::setfill('0') << std::setw(2) << std::dec << (unsigned int) (*neig.begin())->id[i];
////                    if (i % 2 == 1)
////                        out << ".";
////                }
//
////    out << " " << routeSrcName(getSrc());
////    if (getExpiryTime()>0)
////        out << " exp:" << getExpiryTime();
//    return out.str();
//}
//
//
//std::ostream& operator<<(std::ostream& os, const CLNSRoute& e)
//{
//    os << e.info();
////    os << "ahoj\n";
//    return os;
//};




//CLNSTable::CLNSTable() {
//    // TODO Auto-generated constructor stub
//
//}
//
//CLNSTable::~CLNSTable() {
//    // TODO Auto-generated destructor stub
//}
//void CLNSTable::initialize(int stage){
//    if (stage==1)
//    {
////        ift = InterfaceTableAccess().get();
//        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
///*
//        nb = NotificationBoardAccess().get();
//
//        nb->subscribe(this, interfaceCreatedSignal);
//        nb->subscribe(this, interfaceDeletedSignal);
//        nb->subscribe(this, interfaceStateChangedSignal);
//        nb->subscribe(this, interfaceConfigChangedSignal);
//        nb->subscribe(this, interfaceIpv6ConfigChangedSignal);
//*/
//
//        WATCH_VECTOR(table);
//        WATCH_PTRVECTOR(routeVector);
//        WATCH(test);
////        WATCH_MAP(destCache); // FIXME commented out for now
////        isrouter = par("isRouter");
////        WATCH(isrouter);
//
//    }
//    else if (stage==4)
//    {
//        // configurator adds routes only in stage==3
//        //updateDisplayString();
//    }
//}


//void CLNSTable::parseXMLConfigFile()
//{
//    return;
//}
//
//void CLNSTable::handleMessage(cMessage *msg)
//{
//    throw cRuntimeError("This module doesn't process messages");
//}
//
//void CLNSTable::receiveChangeNotification(int category, const cObject *details)
//{
    //TODO A! Handle notifications during initialize
//    if (simulation.getContextType()==CTX_INITIALIZE)
//        return;  // ignore notifications during initialize

//    Enter_Method_Silent();
//    printNotificationBanner(category, details);

//    if (category==interfaceCreatedSignal)
//    {
//        // add netmask route for the new interface
//        updateNetmaskRoutes();
//    }
//    else if (category==interfaceDeletedSignal)
//    {
//        // remove all routes that point to that interface
//        InterfaceEntry *entry = check_and_cast<InterfaceEntry*>(details);
//        deleteInterfaceRoutes(entry);
//    }
//    else if (category==interfaceStateChangedSignal)
//    {
//        invalidateCache();
//    }
//    else if (category==interfaceConfigChangedSignal)
//    {
//        invalidateCache();
//    }
//    else if (category==interfaceIpv4ConfigChangedSignal)
//    {
//        // if anything Ipv4-related changes in the interfaces, interface netmask
//        // based routes have to be re-built.
//        updateNetmaskRoutes();
//    }
//}

/* This method needs to be rewritten because it has been done in hurry */

//void CLNSTable::addRecord(CLNSRoute *route)
//{
//    //TODO B1
////    this->routeVector.push_back(route);
////    return;
//    for (std::vector<CLNSRoute*>::iterator it = this->routeVector.begin(); it != this->routeVector.end(); ++it)
//    {
//        //comparison is based on matching destPrefix and length
//        if ((*(*it)) == (*route))
//        {
////            if ((*it)->getMetric() > route->getMetric())
////            {
//            /* above "if" is commented because we shoudn't compare metric, just replace the records */
//                this->routeVector.erase(it);
//                this->routeVector.push_back(route);
//
////            }
//            return;
//        }
//    }
//
//    this->routeVector.push_back(route);
//
//
//}
//
//void CLNSTable::dropTable(void){
//
//    this->routeVector.clear();
//}
//
//int CLNSTable::getGateIndexBySystemID(unsigned char *systemID){
//    for (std::vector<CLNSRoute*>::iterator it = this->routeVector.begin(); it != this->routeVector.end(); ++it){
////        if(memcmp((*it)->destPrefix, systemID, ISIS_SYSTEM_ID) == 0 ){
////            return (*it)->nextHop.at(0)->entry->getNetworkLayerGateIndex();
////        }
//    }
//
//    return -1;
//}
//
//unsigned char *CLNSTable::getNextHopSystemIDBySystemID(unsigned char *systemID){
//    for (std::vector<CLNSRoute*>::iterator it = this->routeVector.begin(); it != this->routeVector.end(); ++it){
////        if(memcmp((*it)->destPrefix, systemID, ISIS_SYSTEM_ID) == 0 ){
////            unsigned char *tmpSysId = new unsigned char [ISIS_SYSTEM_ID];
////            memcpy(tmpSysId, (*it)->nextHop.front()->id, ISIS_SYSTEM_ID);
////            return tmpSysId;
////        }
//    }
//
//    return NULL;
//}

}//end namespace inet
