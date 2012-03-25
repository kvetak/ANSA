
#include <omnetpp.h>
#include "WeightedFairQueue.h"


Define_Module(WeightedFairQueue);

WeightedFairQueue::WeightedFairQueue()
{
    numQueues = 256;
    holdCapacity = 1024;
    cdt = 256;
    currentHold = 0;
    queues = NULL;
}

WeightedFairQueue::WeightedFairQueue(int c_hold, int c_flows, int c_cdt)
{
    numQueues = c_flows;
    holdCapacity = c_hold;
    cdt = c_cdt;
    currentHold = 0;
    queues = NULL;
}

WeightedFairQueue::~WeightedFairQueue()
{
    for (int i=0; i<numQueues; i++)
        delete queues[i];
    delete [] queues;
}


void WeightedFairQueue::initialize()
{
    PassiveQueueBase::initialize();

    outGate = gate("out");

    // configuration

    
    queues = new WFQ::QueueRecord *[numQueues];
    for (int i=0; i<numQueues; i++)
    {
        queues[i] = new WFQ::QueueRecord();
    }

    
}

bool WeightedFairQueue::enqueue(cMessage *msg)
{
    WFQ::Flow packetInfo;
    

    if(currentHold >= holdCapacity)
    {
        delete msg;
        return true;
    }
    
    cMessage *copy = msg->dup();

    if (dynamic_cast<IPDatagram *>(copy))
    {
        // IPv4 QoS: map DSCP to queue number
        IPDatagram *datagram = (IPDatagram *)copy;
        
        packetInfo.srcAddress = datagram->getSrcAddress();
        packetInfo.destAddress = datagram->getDestAddress();
        packetInfo.transportProtocol = datagram->getTransportProtocol();
        packetInfo.diffServCodePoint = (unsigned char) datagram->getDiffServCodePoint()/32;
        if(packetInfo.transportProtocol == IP_PROT_TCP)
        {
            TCPSegment *tcpSegm = dynamic_cast<TCPSegment *> (datagram->decapsulate());
            packetInfo.srcPort = tcpSegm->getSrcPort();
            packetInfo.destPort = tcpSegm->getDestPort();
            delete tcpSegm;
        }
        else if(packetInfo.transportProtocol == IP_PROT_UDP)
        {
            UDPPacket *udpDatag  = dynamic_cast<UDPPacket *> (datagram->decapsulate());
            packetInfo.srcPort = udpDatag->getSourcePort();
            packetInfo.destPort = udpDatag->getDestinationPort();
            delete udpDatag;
        }
        else
        {
            packetInfo.srcPort = 0;
            packetInfo.destPort = 0;
        }
        
        
    }
    else
    {
        packetInfo.srcAddress = IPAddress::UNSPECIFIED_ADDRESS;
        packetInfo.destAddress = IPAddress::UNSPECIFIED_ADDRESS;
        packetInfo.transportProtocol = 0;
        packetInfo.diffServCodePoint = 0;
        packetInfo.srcPort = 0;
        packetInfo.destPort = 0;
    }
    
    delete copy;
    
    if(!putIntoQueue(packetInfo, msg))
    {
        delete msg;
        return true;
    }

    return false;
}

cMessage *WeightedFairQueue::dequeue()
{
    if(currentHold <= 0)
    {
        return NULL;
    }
    
    
    int index = 0;
    while (queues[index]->isQueueEmpty())
      ++index;
    
    int sn = queues[index]->getLowestSn();
    
    for (int i=0; i<numQueues; i++)
    {
        if(!queues[i]->isQueueEmpty() && queues[i]->getLowestSn() < sn )
        {
            sn = queues[i]->getLowestSn();
            index = i;
        }
    }
    
    
    --currentHold;
    cMessage *msg = queues[index]->dequeue();
    
    
    std::cout << "---- Dequeuing ---- " << std::endl;
    std::cout << "Queue ID: " << index << " Queue Length: " << queues[index]->getQueueLength() << " SN: " << sn << std::endl;
    std::cout << "Hold Queue Length: " << currentHold << std::endl << std::endl;
    
    if(currentHold > 0)
    {
      index = 0;
      while (queues[index]->isQueueEmpty())
        ++index;
      sn = queues[index]->getLowestSn();
    
      for (int i=0; i<numQueues; i++)
      {
        if(!queues[i]->isQueueEmpty() && queues[i]->getLowestSn() < sn )
        {
            sn = queues[i]->getLowestSn();
        }
      }
    
      for (int i=0; i<numQueues; i++)
      {
        queues[i]->substractSnFromAll(sn);

      }
    }
    
    
    return msg;
}

void WeightedFairQueue::sendOut(cMessage *msg)
{
    send(msg, outGate);
}

bool WeightedFairQueue::putIntoQueue(const WFQ::Flow& packetInfo, cMessage * msg)
{
    int index = findQueueIndex(packetInfo);
    if(index >= 0 && queues[index]->getQueueLength() < cdt)
    {
        if(queues[index]->isQueueEmpty())
        {
            queues[index]->setQueueID(packetInfo);
            queues[index]->setLastSn(0);
        }

        cPacket *pkt = dynamic_cast<cPacket *> (msg->dup());
        int sn = queues[index]->getLastSn() + ((32384 / (packetInfo.diffServCodePoint + 1)) * pkt->getByteLength());
        WFQ::SNPacket newSNPacket(msg, sn);
        
        
        delete pkt;

        queues[index]->enqueue(newSNPacket);
        queues[index]->setLastSn(sn);
        ++currentHold;
        
        std::cout << "---- Enqueuing ---- " << std::endl;
        std::cout << "Flow = sIP:" << packetInfo.srcAddress.str();
        std::cout << " dIP:" << packetInfo.destAddress.str();
        std::cout << " Pre:" << packetInfo.diffServCodePoint;
        std::cout << " Pro:" << packetInfo.transportProtocol;
        std::cout << " sP:" << packetInfo.srcPort;
        std::cout << " dP:" << packetInfo.destPort << std::endl;
        std::cout << "Queue ID: " << index << " Queue Length: " << queues[index]->getQueueLength() << " SN: " << sn << std::endl << std::endl;
        
        return true;
    }

    return false;
}

int WeightedFairQueue::findQueueIndex(const WFQ::Flow& packetInfo)
{
    int empty = -1;
    
    for (int i=0; i<numQueues; i++)
    {
        if(empty == -1)
        {
            if(queues[i]->isQueueEmpty())
                empty = i;
        }
        
        if(queues[i]->isFromThisFlow(packetInfo))
            return i;
    }
    
    return empty;

}


bool WFQ::Flow::operator==(const Flow& flow) const
{
    if(srcAddress != flow.srcAddress)
        return false;
    if(destAddress != flow.destAddress)
        return false;
    if(transportProtocol != flow.transportProtocol)
        return false;
    if(diffServCodePoint != flow.diffServCodePoint)
        return false;
    if(srcPort != flow.srcPort)
        return false;
    if(destPort != flow.destPort)
        return false;
    
    return true;
}


WFQ::QueueRecord::~QueueRecord()
{
    if(!isQueueEmpty())
    {
        for (std::vector<SNPacket>::iterator it = snPackets.begin(); it!=snPackets.end(); ++it)
        {
            delete it->getMsg();
        }
    }    
}

bool WFQ::QueueRecord::isQueueEmpty()
{
    if(getQueueLength() > 0)
        return false;

    return true;
}



bool WFQ::QueueRecord::isFromThisFlow(const Flow& flow)
{
    if(!isQueueEmpty() && queueID == flow)
        return true;

    return false;
}


void WFQ::QueueRecord::setQueueID(const Flow& packetInfo)
{
    queueID.srcAddress = packetInfo.srcAddress;
    queueID.destAddress = packetInfo.destAddress;
    queueID.transportProtocol = packetInfo.transportProtocol;
    queueID.diffServCodePoint = packetInfo.diffServCodePoint;
    queueID.srcPort = packetInfo.srcPort;
    queueID.destPort = packetInfo.destPort;
}

void WFQ::QueueRecord::enqueue(SNPacket& newSNPacket)
{
    setLastSn(newSNPacket.getSn());
    snPackets.push_back(newSNPacket);
}

void WFQ::QueueRecord::substractSnFromAll(int subNum)
{
    if(!isQueueEmpty())
    {
        for (std::vector<SNPacket>::iterator it = snPackets.begin(); it!=snPackets.end(); ++it)
        {
            it->substractSn(subNum);
        }
        
        lastSn -= subNum;
    }
}

cMessage *WFQ::QueueRecord::dequeue()
{
    cMessage *msg = (snPackets.front()).getMsg();
    snPackets.erase(snPackets.begin());
    
    return msg;
}

