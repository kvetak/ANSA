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

#include "ansa/linklayer/cdp/CDPInterface.h"

namespace inet {

CDPInterface::~CDPInterface() {
    // TODO Auto-generated destructor stub
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

    return NULL;
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


std::string CDPInterfaceTable::printStats()
{
    std::stringstream string;
    std::vector<CDPInterface *>::iterator it;

    for (it = interfaces.begin(); it != interfaces.end(); ++it)
    {// through all interfaces
        /*if((*it)->getAfSend() != AF::NONE)
        {
            string << (*it)->getIfaceName() << " interface statistics:" << endl;

            string << "Received " << (*it)->rxStat.str();
            if((*it)->rxStat.tlv[tlvT::UPDATE].getCount() > 0) string << "Update avg. size: " << ((*it)->rxStat.tlv[tlvT::ROUTERID].getSum() + (*it)->rxStat.tlv[tlvT::NEXTHOP].getSum() + (*it)->rxStat.tlv[tlvT::UPDATE].getSum()) / static_cast<double>((*it)->rxStat.tlv[tlvT::UPDATE].getCount()) << " B/TLV" << endl;

            string << endl << "Transmitted " << (*it)->txStat.str();
            if((*it)->txStat.tlv[tlvT::UPDATE].getCount() > 0) string << "Update avg. size: " << ((*it)->txStat.tlv[tlvT::ROUTERID].getSum() + (*it)->txStat.tlv[tlvT::NEXTHOP].getSum() + (*it)->txStat.tlv[tlvT::UPDATE].getSum()) / static_cast<double>((*it)->txStat.tlv[tlvT::UPDATE].getCount()) << " B/TLV" << endl;
        }*/
    }
    return string.str();
}
} /* namespace inet */
