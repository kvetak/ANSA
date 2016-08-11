// Copyright (C) 2012 - 2016 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
 * @file CLNSRoute.cc
 * @author Marcel Marek (mailto:imarek@fit.vutbr.cz), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 5.8.2016
 * @brief Class representing a route for CLNS
 * @detail Class representing a route for CLNS
 */


#include "ansa/networklayer/clns/CLNSRoute.h"

#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {

Register_Class(CLNSRoute);

CLNSRoute::~CLNSRoute()
{
    delete protocolData;
}


#ifdef ANSAINET
const char* inet::CLNSRoute::getSourceTypeAbbreviation() const {
    switch (sourceType) {
        case IFACENETMASK:
            return "C";
        case MANUAL:
            return (getDestination().isUnspecified() ? "S*": "S");
//        case ROUTER_ADVERTISEMENT:
//            return "ra";
//        case RIP:
//            return "R";
//        case OSPF:
//            return "O";
//        case BGP:
//            return "B";
//        case EIGRP:
//            return getAdminDist() < CLNSRoute::dEIGRPExternal ? "D" : "D EX";
//        case LISP:
//            return "l";
//        case BABEL:
//            return "ba";
//        case ODR:
//            return "o";
        case ISIS:
            return "i";
        default:
            return "???";
    }
}
#endif

std::string CLNSRoute::info() const
{
    std::stringstream out;
#ifdef ANSAINET
    out << getSourceTypeAbbreviation();
    out << " ";
    if (getDestination().isUnspecified())
        out << "0.0.0.0";
    else
        out << getDestination();
    out << "/";

    if (getGateway().isUnspecified())
    {
        out << " is directly connected";
    }
    else
    {
        out << " [" << getAdminDist() << "/" << getMetric() << "]";
        out << " via ";
        out << getGateway();
    }
    out << ", " << getInterfaceName();
#else
    out << "dest:";
    if (dest.isUnspecified())
        out << "*  ";
    else
        out << dest << "  ";
    out << "gw:";
    if (gateway.isUnspecified())
        out << "*  ";
    else
        out << gateway << "  ";

    out << "metric:" << metric << " ";
    out << "if:";
    if (!interfacePtr)
        out << "*";
    else
        out << interfacePtr->getName();
    if (interfacePtr && interfacePtr->ipv4Data())
        out << "(" << interfacePtr->ipv4Data()->getIPAddress() << ")";
    out << "  ";
    out << (gateway.isUnspecified() ? "DIRECT" : "REMOTE");
    out << " " << IRoute::sourceTypeName(sourceType);

#endif // ANSAINET
    return out.str();
}

//void CLNSAddress::_checkNetmaskLength(int length)
//{
//    if (length < 0 || length > 64)
//        throw cRuntimeError("CLNSAddress: wrong netmask length %d (not in 0..64)", length);
//}





}
