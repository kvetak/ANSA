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

#include "ansa/linklayer/cdp/tables/CDPInterfaceTable.h"

namespace inet {


std::string CDPInterface::info() const
{
    std::stringstream out;

    out << "CDP on interface " << interface->getFullName() << " is ";
    if (enabled)
        out << "enabled";
    else
        out << "disabled";

    return out.str();
}


CDPInterface::CDPInterface(InterfaceEntry *iface) :
    interface(iface),
    enabled(true),
    fastStart(2)
{
    updateTimer = new CDPTimer();
    updateTimer->setTimerType(UpdateTime);
    updateTimer->setContextPointer(this);
}

CDPInterface::~CDPInterface() {
    if(updateTimer != nullptr)
    {
        //if is scheduled, get his sender module, otherwise get owner module
        cSimpleModule *owner = dynamic_cast<cSimpleModule *>((updateTimer->isScheduled()) ? updateTimer->getSenderModule() : updateTimer->getOwner());
        if(owner != nullptr)
        {// owner is cSimpleModule object -> can call his methods
            owner->cancelAndDelete(updateTimer);
            updateTimer = nullptr;
        }
    }
}

CDPInterface * CDPInterfaceTable::findInterfaceById(const int ifaceId)
{
    std::vector<CDPInterface *>::iterator it;

    for (it = interfaces.begin(); it != interfaces.end(); ++it)
    {// through all interfaces search for same interfaceId
        if((*it)->getInterfaceId() == ifaceId)
        {// found same
            return (*it);
        }
    }

    return nullptr;
}

CDPInterface * CDPInterfaceTable::addInterface(CDPInterface * iface)
{
    if(findInterfaceById(iface->getInterfaceId()) != NULL)
    {// interface already in table
        throw cRuntimeError("Adding to CDPInterfaceTable interface, which is already in it - id %d", iface);
    }

    interfaces.push_back(iface);

    return iface;
}


/**
 * Remove all interfaces
 *
 */
void CDPInterfaceTable::removeInterfaces()
{
    std::vector<CDPInterface *>::iterator it;

    for (it = interfaces.begin(); it != interfaces.end();)
    {
        delete (*it);
        it = interfaces.erase(it);
    }
}

/**
 * Remove interface
 *
 * @param   iface   Interface to delete
 *
 */
void CDPInterfaceTable::removeInterface(CDPInterface * iface)
{
    std::vector<CDPInterface *>::iterator it;

    for (it = interfaces.begin(); it != interfaces.end();)
    {// through all interfaces
        if((*it) == iface)
        {// found same
            delete (*it);
            it = interfaces.erase(it);
            return;
        }
        else
        {// do not delete -> get next
            ++it;
        }
    }
}

/**
 * Removes interface
 *
 * @param   ifaceId ID of interface to delete
 */
void CDPInterfaceTable::removeInterface(int ifaceId)
{
    std::vector<CDPInterface *>::iterator it;

    for (it = interfaces.begin(); it != interfaces.end();)
    {// through all interfaces
        if((*it)->getInterfaceId() == ifaceId)
        {// found same
            delete (*it);
            it = interfaces.erase(it);
            return;
        }        else
        {// do not delete -> get next
            ++it;
        }
    }
}

CDPInterfaceTable::~CDPInterfaceTable()
{
    std::vector<CDPInterface *>::iterator it;

    for (it = interfaces.begin(); it != interfaces.end(); ++it)
    {// through all interfaces
        delete (*it);
    }
    interfaces.clear();
}

} /* namespace inet */
