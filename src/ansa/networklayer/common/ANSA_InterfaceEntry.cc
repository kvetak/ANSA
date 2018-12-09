//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
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
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 * @param interfaceModule
 */
#include "ansa/networklayer/common/ANSA_InterfaceEntry.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"

namespace inet{

ANSA_InterfaceEntry::ANSA_InterfaceEntry(cModule *interfaceModule) : InterfaceEntry(interfaceModule)
{
    bandwidth = 0;
    delay = 0;
    reliability = 255;
    recvLoad = 1;
    transLoad = 1;
}

void ANSA_InterfaceEntry::addVirtualForwarder(VirtualForwarder* vforw)
{
    vforwarder.push_back(vforw);
}

void ANSA_InterfaceEntry::removeVirtualForwarder(VirtualForwarder* vforw)
{
    for (int i = 0; i < (int) vforwarder.size(); i++)
        if (vforwarder[i] == vforw)
            vforwarder[i] = NULL;

    delete vforw;
}

int ANSA_InterfaceEntry::getVirtualForwarderId(const Ipv4Address& addr)
{
    if (vforwarder.empty())
        return -1;

    for (int i = 0; i < (int) vforwarder.size(); i++)
        if (!vforwarder[i]->isDisable() && (vforwarder[i]->hasIPAddress(addr)))
            return i;

    return -1;
}

int ANSA_InterfaceEntry::getVirtualForwarderId(const MacAddress& addr)
{
    if (vforwarder.empty())
        return -1;

    for (int i = 0; i < (int) vforwarder.size(); i++)
        if (!vforwarder[i]->isDisable() && vforwarder[i]->getMacAddress() == addr)
            return i;

    return -1;
}

const MacAddress& ANSA_InterfaceEntry::getMacAddressByIP(const Ipv4Address& addr) const
{
    if (vforwarder.empty())
        return getMacAddress();

    for (int i = 0; i < (int) vforwarder.size(); i++)
        if (!vforwarder[i]->isDisable() && vforwarder[i]->hasIPAddress(addr))
            return vforwarder[i]->getMacAddress();

    return getMacAddress();
}

const MacAddress& ANSA_InterfaceEntry::getMacVirtualForwarderById(int vforwarderId) const
{
    if (vforwarder.empty())
        return MacAddress::UNSPECIFIED_ADDRESS;

    return vforwarder.at(vforwarderId)->getMacAddress();
}

bool ANSA_InterfaceEntry::hasIPAddress(const Ipv4Address& addr) const
{
    if (vforwarder.empty())
        return false;

    for (int i = 0; i < (int) vforwarder.size(); i++)
        if (!vforwarder[i]->isDisable() && vforwarder[i]->hasIPAddress(addr))
            return true;

    return false;
}

bool ANSA_InterfaceEntry::hasNetworkAddress(const L3Address& address) const
{
    if (InterfaceEntry::hasNetworkAddress(address))
        return true;

    switch(address.getType()) {
    case L3Address::Ipv4:
        return hasIPAddress(address.toIPv4());

    case L3Address::MAC:
        return hasMacAddress(address.toMAC());

    default:
        break;
    }
    return false;
}


bool ANSA_InterfaceEntry::hasMacAddress(const MacAddress& addr) const
{
    if (vforwarder.empty())
        return false;

    for (int i = 0; i < (int) vforwarder.size(); i++)
        if (!vforwarder[i]->isDisable() && vforwarder[i]->getMacAddress() == addr)
            return true;

    return false;
}

std::string ANSA_InterfaceEntry::info() const {
    std::stringstream out;
    out << (getName()[0] ? getName() : "*");
    out << " id=" << getInterfaceId();
    if (getNetworkLayerGateIndex() == -1)
        out << " ifOut[-]";
    else
        out << "  ifOut[" << getNetworkLayerGateIndex() << "]";
    out << " MTU:" << getMTU();
    if (!isUp())
        out << " DOWN";
    if (isBroadcast())
        out << " BROADCAST";
    if (isMulticast())
        out << " MULTICAST";
    if (isPointToPoint())
        out << " POINTTOPOINT";
    if (isLoopback())
        out << " LOOPBACK";
    out << "\nBW: " << bandwidth << "bit/s"
            << ", DLY: " << delay << " us"
            << ", rely:" << reliability << "/255"
            << ", rload: " << recvLoad << "/255"
            << ", tload:" << transLoad << "/255";
    out << "\nMAC:\t\t";
    if (getMacAddress().isUnspecified())
        out << "n/a";
    else
        out << getMacAddress();
    //Ipv4
    if (ipv4data != nullptr) {
        out << "\nIPv4 ucast:\t" << ipv4data->getIPAddress() << "/" << ipv4data->getNetmask().getNetmaskLength();
        out << "\nIPv4 mcast:\t";
        for (int i = 0; i < ipv4data->getNumOfJoinedMulticastGroups(); i++) {
            out << (i > 0 ? ", " : "") << ipv4data->getJoinedMulticastGroup(i);
        }
    }

    if (ipv6data != nullptr) {
        //Ipv6
        out << "\nIPv6 ucast:\t";
        for (int i = 0; i < ipv6data->getNumAddresses(); i++) {
            out << (i > 0 ? ", " : "") << ipv6data->getAddress(i);
        }
        out << "\nIPv6 mcast:\t";
        bool comma = false;
        for (auto j : ipv6data->getJoinedMulticastGroups()) {
            out << (comma ? ", " : "") << j;
            comma = true;
        }
    }
    return out.str();
}

void ANSA_InterfaceEntry::setDatarate(double d) {
    if (datarate != d) {
        datarate = d;
        switch ((long)datarate) {
            case 64000: //56k modem
                bandwidth = 64;
                delay = 20000;
                break;
            case 56000: //56k modem
                bandwidth = 56;
                delay = 20000;
                break;
            case 1544000: //T1
                bandwidth = 1544;
                delay = 20000;
                break;
            case 10000000: //Eth10
                bandwidth = 10000;
                delay = 1000;
                break;
            default: //>Eth10
                bandwidth = 100000;
                delay = 100;
                break;
        }
        configChanged(F_DATARATE);
    }
}

}


