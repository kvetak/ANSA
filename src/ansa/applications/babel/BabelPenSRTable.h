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
* @file BabelPenSRTable.h
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
* @brief Babel Pending Seqno Requests Table header file
* @detail Represents data structure for saving pending Seqno requests
*/

#ifndef BABELPENSRTABLE_H_
#define BABELPENSRTABLE_H_

#include <omnetpp.h>

#include "ansa/applications/babel/BabelDef.h"
#include "ansa/applications/babel/BabelNeighbourTable.h"
#include "ansa/applications/babel/BabelFtlv.h"

class BabelPenSR : public cObject
{
protected:
    BabelSeqnoReqFtlv request;
    BabelNeighbour *receivedFrom;
    int resendNum;
    BabelInterface *outIface;
    inet::L3Address forwardTo;
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
                BabelInterface *oi, const inet::L3Address& fwddst, Babel::BabelTimer *rt):
        request(req),
        receivedFrom(recfrom),
        resendNum(resend),
        outIface(oi),
        forwardTo(fwddst),
        resendTimer(rt)
        {
            ASSERT(outIface != NULL);

            if(resendTimer != NULL);
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

    const inet::L3Address& getForwardTo() const {return forwardTo;}
    void setForwardTo(const inet::L3Address& ft) {forwardTo = ft;}

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

    BabelPenSR *findPenSR(const Babel::netPrefix<inet::L3Address>& p);
    BabelPenSR *findPenSR(const Babel::netPrefix<inet::L3Address>& p, BabelInterface *iface);
    BabelPenSR *addPenSR(BabelPenSR *request);
    void removePenSR(BabelPenSR *request);
    void removePenSR(const Babel::netPrefix<inet::L3Address>& p);
    void removePenSRsByNeigh(BabelNeighbour *neigh);
    void removePenSRs();
};

#endif /* BABELPENSRTABLE_H_ */
