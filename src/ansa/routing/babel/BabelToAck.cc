// The MIT License (MIT)
//
// Copyright (c) 2016 Brno University of Technology
//
//@author Vladimir Vesely (iveselyATfitDOTvutbrDOTcz)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
/**
* @file BabelToAck.cc
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
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
