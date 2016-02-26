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
* @file BabelPenSRTable.h
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
* @brief Babel Pending Seqno Requests Table header file
* @detail Represents data structure for saving pending Seqno requests
*/

#ifndef BABELPENSRTABLE_H_
#define BABELPENSRTABLE_H_

#include "inet/common/INETDefs.h"

#include "ansa/routing/babel/BabelDef.h"
#include "ansa/routing/babel/BabelNeighbourTable.h"
#include "ansa/routing/babel/BabelFtlv.h"
namespace inet {
class INET_API BabelPenSR : public cObject
{
protected:
    BabelSeqnoReqFtlv request;
    BabelNeighbour *receivedFrom;
    int resendNum;
    BabelInterface *outIface;
    L3Address forwardTo;
    Babel::BabelTimer *resendTimer;
public:
    BabelPenSR():
        receivedFrom(NULL),
        resendNum(0),
        outIface(NULL),
        resendTimer(NULL)
        {}

    BabelPenSR(const BabelSeqnoReqFtlv& req,
                BabelNeighbour *recfrom,
                int resend,
                BabelInterface *oi, const L3Address& fwddst, Babel::BabelTimer *rt):
        request(req),
        receivedFrom(recfrom),
        resendNum(resend),
        outIface(oi),
        forwardTo(fwddst),
        resendTimer(rt)
        {
            ASSERT(outIface != NULL);

            if(resendTimer != NULL)
            {
                resendTimer->setContextPointer(this);
            }
        }
    virtual ~BabelPenSR();
    virtual std::string str() const;
    virtual std::string detailedInfo() const {return str();}
    friend std::ostream& operator<<(std::ostream& os, const BabelPenSR& psr);

    const BabelSeqnoReqFtlv& getRequest() const {return request;}
    void setRequest(const BabelSeqnoReqFtlv& r) {request = r;}

    BabelNeighbour* getReceivedFrom() const {return receivedFrom;}
    void setReceivedFrom(BabelNeighbour* rf) {receivedFrom = rf;}

    int getResendNum() const {return resendNum;}
    void setResendNum(int rn) {resendNum = rn;}
    int decResendNum() {return --resendNum;}

    BabelInterface* getOutIface() const {return outIface;}
    void setOutIface(BabelInterface* oi) {outIface = oi;}

    const L3Address& getForwardTo() const {return forwardTo;}
    void setForwardTo(const L3Address& ft) {forwardTo = ft;}

    Babel::BabelTimer* getResendTimer() const {return resendTimer;}
    void setResendTimer(Babel::BabelTimer* rt) {resendTimer = rt;}

    void resetResendTimer();
    void resetResendTimer(double delay);
    void deleteResendTimer();
};

class BabelPenSRTable
{
protected:
    std::vector<BabelPenSR *> requests;


public:
    BabelPenSRTable()
    {}
    virtual ~BabelPenSRTable();

    std::vector<BabelPenSR *>& getRequests() {return requests;}

    BabelPenSR *findPenSR(const Babel::netPrefix<L3Address>& p);
    BabelPenSR *findPenSR(const Babel::netPrefix<L3Address>& p, BabelInterface *iface);
    BabelPenSR *addPenSR(BabelPenSR *request);
    void removePenSR(BabelPenSR *request);
    void removePenSR(const Babel::netPrefix<L3Address>& p);
    void removePenSRsByNeigh(BabelNeighbour *neigh);
    void removePenSRs();
};
}
#endif /* BABELPENSRTABLE_H_ */
