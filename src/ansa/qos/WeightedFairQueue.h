

#ifndef __ANSA_WFQ_H
#define __ANSA_WFQ_H

#include <vector>
#include <iostream>
#include <omnetpp.h>
#include "PassiveQueueBase.h"
#include "IPDatagram.h"
#include "TCPSegment.h"
#include "UDPPacket.h"

namespace WFQ {

class SNPacket {

private:
    cMessage *msg;
    //uint64_t sn;
    int sn;

public:
    SNPacket() {msg = NULL; sn = 0;}
    SNPacket(cMessage *c_msg, int c_sn) {msg = c_msg; sn = c_sn;}
    
    int getSn() {return sn;}
    void setSn(int n_sn) {sn = n_sn;}
    void substractSn(int subNum) {sn = sn - subNum;}
    
    cMessage *getMsg() {return msg;}
    void setMsg(cMessage *n_msg) {msg = n_msg;}
};

class Flow {

public:
    IPAddress srcAddress;
    IPAddress destAddress;
    int transportProtocol;
    unsigned char diffServCodePoint;
    int srcPort;
    int destPort;


    bool operator==(const Flow& flow) const;

};



class QueueRecord {

private:
    Flow queueID;

    int lastSn;
    std::vector<SNPacket> snPackets;

public:
    QueueRecord() {lastSn = 0;}
    ~QueueRecord();
    
    void setLastSn(int n_sn) {lastSn = n_sn;}
    int getLastSn() {return lastSn;}
    int getQueueLength() {return snPackets.size();}
    bool isQueueEmpty();
    bool isFromThisFlow(const Flow& flow);
    void setQueueID(const Flow& packetInfo);
    void enqueue(SNPacket& newSNPacket);
    void substractSnFromAll(int subNum);
    int getLowestSn() {return (snPackets.front()).getSn();}
    cMessage *dequeue();

};

}



class INET_API WeightedFairQueue : public PassiveQueueBase
{
  protected:
    // configuration
    int numQueues;
    int holdCapacity;
    int currentHold;
    int cdt;

    // state
    WFQ::QueueRecord **queues;
    cGate *outGate;
    

  public:
    WeightedFairQueue();
    WeightedFairQueue(int c_hold, int c_flows, int c_cdt);
    ~WeightedFairQueue();

  protected:

    virtual void initialize();

    /**
     * Redefined from PassiveQueueBase.
     */
    virtual bool enqueue(cMessage *msg);

    /**
     * Redefined from PassiveQueueBase.
     */
    virtual cMessage *dequeue();

    /**
     * Redefined from PassiveQueueBase.
     */
    virtual void sendOut(cMessage *msg);
    
  private:
    bool putIntoQueue(const WFQ::Flow& packetInfo, cMessage * msg);
    int findQueueIndex(const WFQ::Flow& packetInfo);
    
    
};


#endif


