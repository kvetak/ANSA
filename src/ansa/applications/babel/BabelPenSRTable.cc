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
* @file BabelPenSRTable.cc
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
* @brief Babel Pending Seqno Requests Table
* @detail Represents data structure for saving pending Seqno requests
*/

#include "ansa/applications/babel/BabelPenSRTable.h"

using namespace Babel;

BabelPenSR::~BabelPenSR()
{
    deleteResendTimer();
}

std::ostream& operator<<(std::ostream& os, const BabelPenSR& psr)
{
    return os << psr.str();
}

std::string BabelPenSR::str() const
{
    std::stringstream string;
#ifdef BABEL_DEBUG
    string << request.getPrefix();
    string << ", originator: " << request.getRouterId();
    string << ", requested SN: " << request.getSeqno();
    if(receivedFrom != NULL) string << ", received from " << receivedFrom->getAddress();
    string << ", remaining resends: " << resendNum;
    if(resendTimer != NULL && resendTimer->isScheduled()) string << ", next resend at T=" << resendTimer->getArrivalTime() << ", on interface " << outIface->getIfaceName();
#else
    string << request.getPrefix();
        string << " orig:" << request.getRouterId();
        string << " reqSN:" << request.getSeqno();
        if(receivedFrom != NULL) string << " from:" << receivedFrom->getAddress();
        string << " remains:" << resendNum;
        string << " on " << outIface->getIfaceName();
#endif
    return string.str();
}


void BabelPenSR::resetResendTimer()
{
    ASSERT(resendTimer != NULL);

    if(outIface != NULL)
    {
        resetTimer(resendTimer, CStoS(outIface->getHInterval() / 2));
    }
    else
    {
        resetTimer(resendTimer, CStoS(defval::HELLO_INTERVAL_CS / 2));
    }
}

void BabelPenSR::resetResendTimer(double delay)
{
    ASSERT(resendTimer != NULL);

    resetTimer(resendTimer, delay);
}

void BabelPenSR::deleteResendTimer()
{
    deleteTimer(&resendTimer);
}





BabelPenSRTable::~BabelPenSRTable()
{
    removePenSRs();
}



BabelPenSR *BabelPenSRTable::findPenSR(const netPrefix<inet::L3Address>& p)
{
    std::vector<BabelPenSR *>::iterator it;

    for (it = requests.begin(); it != requests.end(); ++it)
    {// through all sources search for same prefix
        if((*it)->getRequest().getPrefix() == p)
        {// found same
            return (*it);
        }
    }

    return NULL;
}

BabelPenSR *BabelPenSRTable::findPenSR(const netPrefix<inet::L3Address>& p, BabelInterface *iface)
{
    std::vector<BabelPenSR *>::iterator it;

    for (it = requests.begin(); it != requests.end(); ++it)
    {// through all sources search for same prefix
        if((*it)->getRequest().getPrefix() == p && (*it)->getOutIface() == iface)
        {// found same
            return (*it);
        }
    }

    return NULL;
}


BabelPenSR *BabelPenSRTable::addPenSR(BabelPenSR *request)
{
    ASSERT(request != NULL);

    BabelPenSR *intable = findPenSR(request->getRequest().getPrefix(), request->getOutIface());

    if(intable != NULL)
    {// request already in table
        return intable;
    }

    requests.push_back(request);

    return request;
}

void BabelPenSRTable::removePenSR(BabelPenSR *request)
{
    std::vector<BabelPenSR *>::iterator it;

    for (it = requests.begin(); it != requests.end();)
    {// through all requests
        if((*it) == request)
        {// found same
            delete (*it);
            it = requests.erase(it);
            return;
        }
        else
        {// do not delete -> get next
            ++it;
        }
    }
}


void BabelPenSRTable::removePenSR(const netPrefix<inet::L3Address>& p)
{
    std::vector<BabelPenSR *>::iterator it;

    for (it = requests.begin(); it != requests.end();)
    {// through all requests
        if((*it)->getRequest().getPrefix() == p)
        {// found same
            delete (*it);
            it = requests.erase(it);
        }
        else
        {// do not delete -> get next
            ++it;
        }
    }
}

void BabelPenSRTable::removePenSRsByNeigh(BabelNeighbour *neigh)
{
    std::vector<BabelPenSR *>::iterator it;

    for (it = requests.begin(); it != requests.end();)
    {// through all requests
        if((*it)->getReceivedFrom() == neigh)
        {// found same
            delete (*it);
            it = requests.erase(it);
        }
        else
        {// do not delete -> get next
            ++it;
        }
    }
}

void BabelPenSRTable::removePenSRs()
{
    std::vector<BabelPenSR *>::iterator it;

    for (it = requests.begin(); it != requests.end(); ++it)
    {// through all routes
        delete (*it);
    }
    requests.clear();
}
