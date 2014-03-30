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

#ifndef __INET_EIGRPRTP_H_
#define __INET_EIGRPRTP_H_

#include <omnetpp.h>

#include "EigrpTransQueue.h"
#include "EigrpInterfaceTable.h"
#include "EigrpIpv4NeighborTable.h"

class EigrpRtp;

class EigrpRequestQueue: public cObject
{
  private:
    typedef std::list<EigrpMsgReq *> MessageQueue;

    MessageQueue reqQueue;          /**< Queue with requests waiting for sending (rel/unrel) */

    // TODO: dat pryc, pouze pro testovani
    friend class EigrpRtp;

  public:
    virtual ~EigrpRequestQueue();

    void pushReq(EigrpMsgReq *req);
    EigrpMsgReq *findReqByIf(int ifaceId, bool sent = true);
    EigrpMsgReq *findUnrelReqByIf(int ifaceId);
    EigrpMsgReq *findReqBySeq(uint32_t seqNumber);
    EigrpMsgReq *removeReq(EigrpMsgReq *msgReq);
    void removeAllMsgsToIf(int ifaceId);
    void removeAllMsgsToNeigh(int ifaceId);
    int getNumReq() const { return reqQueue.size(); }

    void printInfo() const;
};

/**
 * TODO - Generated class
 */
class EigrpRtp : public cSimpleModule
{
    struct NeighborInfo
    {
        int neighborId;
        int neighborIfaceId;
        uint32_t lastSeqNum;
        int numOfAck;
    };
    typedef std::list<EigrpMsgReq *> MessageQueue;

    const char *RTP_OUTPUT_GW;

    uint32_t seqNumber;             /**< Sequence number for reliable transport of messages */

    EigrpRequestQueue *requestQ;

    EigrpInterfaceTable *eigrpIft;
    EigrpIpv4NeighborTable *eigrpNt;

    void processRequest(cMessage *msg);
    void processTimer(cMessage *msg);
    void processHeader(cMessage *msg);

    EigrpTimer *createTimer(char timerKind, void *context);

    void scheduleNextMsg(int ifaceId);
    void scheduleMsg(EigrpMsgReq *msgReq);
    void scheduleNewMsg(EigrpMsgReq *msgReq);
    void sendRelMsg(EigrpMsgReq *msgReq);
    void sendUnrelMsg(EigrpMsgReq *msgReq) { requestQ->removeReq(msgReq); send(msgReq, RTP_OUTPUT_GW); /* Do not duplicate EigrpMsgReq */ }
    void discardMsg(EigrpMsgReq *msgReq);

    EigrpNeighbor<IPv4Address> *getNeighborId(EigrpMessage *msg);

  protected:
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual int numInitStages() const { return 4; }

  public:
    EigrpRtp();
    virtual ~EigrpRtp();

};

#endif
