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

#include "ansa/networklayer/eigrp/tables/EigrpInterfaceTable.h"

#include <algorithm>

Define_Module(EigrpInterfaceTable);


std::ostream& operator<<(std::ostream& out, const EigrpInterface& iface)
{
    out << iface.getInterfaceName() << "(" << iface.getInterfaceId() << ")";
    out << "  Peers:" << iface.getNumOfNeighbors();
    out << "  Passive:";
    if (iface.isPassive()) out << "enabled";
    else out << "disabled";
    //out << "  stubs:" << iface.getNumOfStubs();
    out << "  HelloInt:" << iface.getHelloInt();
    out << "  HoldInt:" << iface.getHoldInt();
    out << "  SplitHorizon:" ;
    if (iface.isSplitHorizonEn()) out << "enabled";
    else out << "disabled";
    /*out << "  bw:" << iface.getBandwidth();
    out << "  dly:" << iface.getDelay();
    out << "  rel:" << iface.getReliability() << "/255";
    out << "  rLoad:" << iface.getLoad() << "/255";
    out << "  pendingMsgs:" << iface.getPendingMsgs();*/
    return out;
}

EigrpInterface::~EigrpInterface()
{
    delete hellot;
}

EigrpInterface::EigrpInterface(inet::InterfaceEntry *iface, int networkId, bool enabled) :
       interfaceId(iface->getInterfaceId()), networkId(networkId), enabled(enabled)
{
    hellot = NULL;
    neighborCount = 0;
    stubCount = 0;
    splitHorizon = true;
    passive = false;

    bandwidth = iface->getBandwidth();
    delay = iface->getDelay();
    reliability = iface->getReliability();
    load = iface->getTransLoad();
    mtu = iface->getMTU();
    interfaceName = iface->getName();
    relMsgs = 0;
    pendingMsgs = 0;

    if (!iface->isMulticast() && bandwidth <= 1544)
    { // Non-broadcast Multi Access interface (no multicast) with bandwidth equal or lower than T1 link
        helloInt = 60;
        holdInt = 180;
    }
    else
    {
        helloInt = 5;
        holdInt = 15;
    }
}

bool EigrpInterface::isMulticastAllowedOnIface(inet::InterfaceEntry *iface)
{
    if (iface->isMulticast())
        if (getNumOfNeighbors() > 1)
            return true;

    return false;
}


EigrpInterfaceTable::~EigrpInterfaceTable()
{
    int cnt = eigrpInterfaces.size();
    EigrpInterface *iface;

    for (int i = 0; i < cnt; i++)
    {
        iface = eigrpInterfaces[i];

        // Must be there
        cancelHelloTimer(iface);

        eigrpInterfaces[i] = NULL;
        delete iface;
    }
    eigrpInterfaces.clear();
}

void EigrpInterfaceTable::initialize(int stage)
{
    if (stage == 3)
    {
        WATCH_PTRVECTOR(eigrpInterfaces);
    }
}

void EigrpInterfaceTable::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}

void EigrpInterfaceTable::cancelHelloTimer(EigrpInterface *iface)
{
    EigrpTimer *timer;

    if ((timer = iface->getHelloTimer()) != NULL)
    {
        if (timer->isScheduled())
            cancelEvent(timer);
    }
}

void EigrpInterfaceTable::addInterface(EigrpInterface *interface)
{
    //TODO check duplicity
    eigrpInterfaces.push_back(interface);
}

EigrpInterface *EigrpInterfaceTable::removeInterface(EigrpInterface *iface)
{
    InterfaceVector::iterator it;
    it = std::find(eigrpInterfaces.begin(), eigrpInterfaces.end(), iface);

    if (it != eigrpInterfaces.end())
    {
        eigrpInterfaces.erase(it);
        return iface;
    }

    return NULL;
}

EigrpInterface *EigrpInterfaceTable::findInterfaceById(int ifaceId)
{
    InterfaceVector::iterator it;
    EigrpInterface * iface;

    for (it = eigrpInterfaces.begin(); it != eigrpInterfaces.end(); it++)
    {
        iface = *it;
        if (iface->getInterfaceId() == ifaceId)
        {
            return iface;
        }
    }

    return NULL;
}
