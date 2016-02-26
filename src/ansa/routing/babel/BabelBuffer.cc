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
* @file BabelBuffer.cc
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
* @brief Babel Buffer
* @detail Represents buffer used for sending messages
*/
#include "ansa/routing/babel/BabelBuffer.h"
namespace inet {
using namespace Babel;

BabelBuffer::~BabelBuffer()
{
    if(flushTimer != NULL)
    {// existing timer -> get his owner (cSimpleModule)

        //if is scheduled, get his sender module, otherwise get owner module
        cSimpleModule *owner = dynamic_cast<cSimpleModule *>((flushTimer->isScheduled()) ? flushTimer->getSenderModule() : flushTimer->getOwner());
        if(owner != NULL)
        {// owner is cSimpleModule object -> can call his methods
            owner->cancelAndDelete(flushTimer);
            flushTimer = NULL;
        }
    }
}


std::string BabelBuffer::str() const
{
    std::stringstream string;


#ifdef BABEL_DEBUG

    string << "Destination: " << dst << ", out-iface: ";

    if(outIface != NULL)
    {
        string << outIface->getIfaceName();
    }
    else
    {
        string << "-";
    }

    if(flushTimer->isScheduled()) string << ", flush scheduled at T=" << flushTimer->getArrivalTime();

    string << ", " << tlvs.size() << " TLVs";
    if(tlvs.size() > 0)
    {
        string << " (";

        for (std::vector<BabelFtlv *>::const_iterator it = tlvs.begin(); it != tlvs.end(); ++it)
        {
            string << ((it != tlvs.begin()) ? "; " : "") << (*it)->str();
        }

        string << ")";
    }
#else
    string << dst << " on ";

    if(outIface != NULL)
    {
        string << outIface->getIfaceName();
    }
    else
    {
        string << "-";
    }


    string << ", " << tlvs.size() << " TLVs";
    if(tlvs.size() > 0)
    {
        string << " (";

        for (std::vector<BabelFtlv *>::const_iterator it = tlvs.begin(); it != tlvs.end(); ++it)
        {
            string << ((it != tlvs.begin()) ? "; " : "") << tlvT::toStr((*it)->getType());
        }

        string << ")";
    }

    if(flushTimer->isScheduled()) string << ", flush at:" << flushTimer->getArrivalTime();

#endif
    return string.str();
}



std::ostream& operator<<(std::ostream& os, const BabelBuffer& buff)
{
    return os << buff.str();
}

/**
 * Get most similar Update FTLV
 *
 * @param   nh  Next hop address
 * @param   origin  RouterId of originator
 */
std::vector<BabelFtlv *>::iterator BabelBuffer::getSimilarUpdateTlv(L3Address nh, Babel::rid origin)
{
    std::vector<BabelFtlv *>::iterator sametype = tlvs.end();
    std::vector<BabelFtlv *>::iterator samenh = tlvs.end();
    std::vector<BabelFtlv *>::iterator samerid = tlvs.end();

    std::vector<BabelFtlv *>::iterator it;

    for (it = tlvs.begin(); it != tlvs.end(); ++it)
    {// through all ftlvs search for Update ftlv
        if((*it)->getType() == tlvT::UPDATE)
        {// found same type
            if(sametype == tlvs.end())
            {// found first ftlv with same type -> note
                sametype = it;
            }

            BabelUpdateFtlv *uftlv = dynamic_cast<BabelUpdateFtlv *>((*it));
            if(uftlv->getNextHop() == nh && uftlv->getRouterId() == origin)
            {// most similar -> choose this
                return it;
            }
            else if(uftlv->getNextHop() == nh && samenh == tlvs.end())
            {// found first ftlv with same nh -> note
                    sametype = it;
            }
            else if(uftlv->getRouterId() == origin && samerid == tlvs.end())
            {// found first ftlv with same rid -> note
                    samerid = it;
            }
        }
    }


    if(samerid != tlvs.end())
    {
        return samerid;
    }
    else if(samenh != tlvs.end())
    {
        return samenh;
    }
    else
    {
        return sametype;
    }
}

/**
 * Is FTLV of type in buffer?
 *
 * @param   tlvtype Type of tlv
 *
 * @return  True if buffer contain FTLV of type tlvtype, otherwise false
 */
bool BabelBuffer::containTlv(uint8_t tlvtype) const
{
    std::vector<BabelFtlv *>::const_iterator it;

    for (it = tlvs.begin(); it != tlvs.end(); ++it)
    {// find tlv with same type
        if((*it)->getType() == tlvtype)
        {// found
            return true;
        }

    }
    return false;
}
}
