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
* @file BabelToAck.cc
* @author Vit Rek (rek@kn.vutbr.cz)
* @author Vladimir Vesely (ivesely@fit.vutbr.cz)
* @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
* @brief Babel ToACK
* @detail Represents data structure for saving messages waiting for Acknowledgment
*/

#include "ansa/routing/babel/BabelToAck.h"
namespace inet {
using namespace Babel;

BabelToAck::~BabelToAck()
{
    if(resendTimer != NULL)
    {// existing timer -> get his owner (cSimpleModule)

        //if is scheduled, get his sender module, otherwise get owner module
        cSimpleModule *owner = dynamic_cast<cSimpleModule *>((resendTimer->isScheduled()) ? resendTimer->getSenderModule() : resendTimer->getOwner());
        if(owner != NULL)
        {// owner is cSimpleModule object -> can call his methods
            owner->cancelAndDelete(resendTimer);
            resendTimer = NULL;
        }
    }

    delete msg;
}

std::string BabelToAck::str() const
{
    std::stringstream string;

    string << "Destination: " << dst << ", out-iface: ";

    if(outIface != NULL)
    {
        string << outIface->getIfaceName();
    }
    else
    {
        string << "-";
    }

    string << ", nonce: " << nonce;
    string << ", remaining resends: " << resendNum;
    string << ", waiting for " << dstNodes.size() << " ACKs";
    if(resendTimer != NULL && resendTimer->isScheduled()) string << ", next resend scheduled at T=" << resendTimer->getArrivalTime();

    return string.str();
}

std::ostream& operator<<(std::ostream& os, const BabelToAck& toack)
{
    return os << toack.str();
}


void BabelToAck::addDstNode(L3Address dn)
{
    std::vector<L3Address>::iterator it;

    for (it = dstNodes.begin(); it != dstNodes.end(); ++it)
    {// through all nodes search for same destination address
        if((*it) == dn)
        {// already in
            return;
        }
    }

    // not in -> create new
    dstNodes.push_back(dn);
}

void BabelToAck::removeDstNode(L3Address dn)
{
    std::vector<L3Address>::iterator it;

    for (it = dstNodes.begin(); it != dstNodes.end();)
    {// find node
        if((*it) == dn)
        {// found -> delete
            it = dstNodes.erase(it);
            return;
        }
        else
        {// do not delete -> get next
            ++it;
        }
    }
}
}
