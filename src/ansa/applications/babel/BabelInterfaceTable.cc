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
* @file BabelInterfaceTable.cc
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
* @brief Babel Interface Table
* @detail Represents data structure of interfaces included to Babel routing
*/

#include "ansa/applications/babel/BabelInterfaceTable.h"


using namespace Babel;


BabelInterface::~BabelInterface()
{
    if(socket4 != NULL)
    {
        //socket4->close();
        delete socket4;
        socket4 = NULL;
    }

    if(socket6 != NULL)
    {
        //socket6->close();
        delete socket6;
        socket6 = NULL;
    }

    deleteHTimer();
    deleteUTimer();
}

std::string BabelInterface::str() const
{
    std::stringstream string;

    string << getIfaceName();
    string << ":" << ((enabled) ? "ena" : "dis");
    string << " Send:" << Babel::AF::toStr(afsend);
    string << " Dist:" << Babel::AF::toStr(afdist);
    string << " SH:" << ((splitHorizon) ? "ena" : "dis");
    string << " Wired:" << ((wired) ? "ena" : "dis");
    string << " HSeqno:" << helloSeqno;
    string << " HInt:" << helloInterval;
    string << " UInt:" << updateInterval;
#ifdef ____BABEL_DEBUG
    if(helloTimer) string << " HelloT:" << helloTimer->getName() ;
    if(updateTimer) string << " UpdateT:" << updateTimer->getName();
    if(socket4) string << " UDP4sock:" << socket4->getSocketId();
    if(socket6) string << " UDP6sock:" << socket6->getSocketId();
    if(ccm) string << " CCM:" <<  dynamic_cast<cSimpleModule *>(ccm)->getName();
#endif
    return string.str();
}

void BabelInterface::addDirectlyConn(const netPrefix<inet::L3Address>& pre)
{
    std::vector<netPrefix<inet::L3Address> >::iterator it;

    for (it = directlyconn.begin(); it != directlyconn.end(); ++it)
    {// through all prefixes search for same
        if((*it) == pre)
        {// found same -> do not add again
            return;
        }
    }

    directlyconn.push_back(pre);
}


void BabelInterface::resetHTimer()
{
    ASSERT(helloTimer != NULL);

    resetTimer(helloTimer, CStoS(helloInterval));
}

void BabelInterface::resetHTimer(double delay)
{
    ASSERT(helloTimer != NULL);

    resetTimer(helloTimer, delay);
}

void BabelInterface::resetUTimer()
{
    ASSERT(updateTimer != NULL);

    resetTimer(updateTimer, CStoS(updateInterval));
}

void BabelInterface::resetUTimer(double delay)
{
    ASSERT(updateTimer != NULL);

    resetTimer(updateTimer, delay);
}

void BabelInterface::deleteHTimer()
{
    if(helloTimer != NULL)
    {// existing timer -> get his owner (cSimpleModule)

        //if is scheduled, get his sender module, otherwise get owner module
        cSimpleModule *owner = dynamic_cast<cSimpleModule *>((helloTimer->isScheduled()) ? helloTimer->getSenderModule() : helloTimer->getOwner());
        if(owner != NULL)
        {// owner is cSimpleModule object -> can call his methods
            owner->cancelAndDelete(helloTimer);
            helloTimer = NULL;
        }
    }
}

void BabelInterface::deleteUTimer()
{
    if(updateTimer != NULL)
    {// existing timer -> get his owner (cSimpleModule)

        //if is scheduled, get his sender module, otherwise get owner module
        cSimpleModule *owner = dynamic_cast<cSimpleModule *>((updateTimer->isScheduled()) ? updateTimer->getSenderModule() : updateTimer->getOwner());
        if(owner != NULL)
        {// owner is cSimpleModule object -> can call his methods
            owner->cancelAndDelete(updateTimer);
            updateTimer = NULL;
        }
    }
}


BabelInterface * BabelInterfaceTable::findInterfaceById(const int ifaceId)
{
    std::vector<BabelInterface *>::iterator it;

    for (it = interfaces.begin(); it != interfaces.end(); ++it)
    {// through all interfaces search for same interfaceId
        if((*it)->getInterfaceId() == ifaceId)
        {// found same
            return (*it);
        }
    }

    return NULL;
}

BabelInterface * BabelInterfaceTable::addInterface(BabelInterface * iface)
{
    if(findInterfaceById(iface->getInterfaceId()) != NULL)
    {// interface already in table
        throw cRuntimeError("Adding to BabelInterfaceTable interface, which is already in it - id %d", iface);
    }

    interfaces.push_back(iface);

    return iface;
}

/**
 * Remove interface
 *
 * @param   iface   Interface to delete
 *
 */
void BabelInterfaceTable::removeInterface(BabelInterface * iface)
{
    std::vector<BabelInterface *>::iterator it;

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
void BabelInterfaceTable::removeInterface(int ifaceId)
{
    std::vector<BabelInterface *>::iterator it;

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

BabelInterfaceTable::~BabelInterfaceTable()
{
    std::vector<BabelInterface *>::iterator it;

    for (it = interfaces.begin(); it != interfaces.end(); ++it)
    {// through all interfaces
        delete (*it);
    }
    interfaces.clear();
}


std::string BabelInterfaceTable::printStats()
{
    std::stringstream string;
    std::vector<BabelInterface *>::iterator it;

    for (it = interfaces.begin(); it != interfaces.end(); ++it)
    {// through all interfaces
        if((*it)->getAfSend() != AF::NONE)
        {
            string << (*it)->getIfaceName() << " interface statistics:" << endl;

            string << "Received " << (*it)->rxStat.str();
            if((*it)->rxStat.tlv[tlvT::UPDATE].getCount() > 0) string << "Update avg. size: " << ((*it)->rxStat.tlv[tlvT::ROUTERID].getSum() + (*it)->rxStat.tlv[tlvT::NEXTHOP].getSum() + (*it)->rxStat.tlv[tlvT::UPDATE].getSum()) / static_cast<double>((*it)->rxStat.tlv[tlvT::UPDATE].getCount()) << " B/TLV" << endl;

            string << endl << "Transmitted " << (*it)->txStat.str();
            if((*it)->txStat.tlv[tlvT::UPDATE].getCount() > 0) string << "Update avg. size: " << ((*it)->txStat.tlv[tlvT::ROUTERID].getSum() + (*it)->txStat.tlv[tlvT::NEXTHOP].getSum() + (*it)->txStat.tlv[tlvT::UPDATE].getSum()) / static_cast<double>((*it)->txStat.tlv[tlvT::UPDATE].getCount()) << " B/TLV" << endl;
        }
    }
    return string.str();
}
