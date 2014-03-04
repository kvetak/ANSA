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

#include "EigrpInterfaceTable.h"

#include <algorithm>

#include "InterfaceTable.h"
#include "InterfaceEntry.h"
#include "IPv4Address.h"
#include "InterfaceTableAccess.h"
#include "IPv4InterfaceData.h"

Define_Module(EigrpInterfaceTable);


std::ostream& operator<<(std::ostream& os, const EigrpInterface& iface)
{

    os << "ID = " << iface.getInterfaceId() << ", Hello = " << iface.getHelloInt()
                << ", Hold = " << iface.getHoldInt();
    return os;
}

EigrpInterface::~EigrpInterface()
{
    delete hellot;
}

EigrpInterface::EigrpInterface(int interfaceId, int networkId, bool enabled) :
       interfaceId(interfaceId), networkId(networkId), enabled(enabled)
{
    hellot = NULL;
    helloInt = 5;
    holdInt = 15;

    bandwidth = 1544;
    delay = 20000;
    reliability = 255;
    load = 1;
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

/*EigrpInterface *EigrpInterfaceTable::findInterfaceByName(const char *ifaceName)
{
    EigrpInterface *eigrpIface;
    InterfaceEntry *iface;
    int ifaceId;

    if ((iface = ifTable->getInterfaceByName(ifaceName)) != NULL)
    {
        ifaceId = iface->getInterfaceId();

        TODO toto je na hovno, mělo by to být ve zvláštní metodě
        if ((eigrpIface = findInterfaceById(ifaceId)) == NULL)
        { // create new EIGRP interface
            eigrpIface = new EigrpInterface(ifaceId);
            addInterface(eigrpIface);
        }
    }
    else
    {
        throw cRuntimeError("Interface %s not found", ifaceName);
    }

    return NULL;
}*/
