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

#ifndef EIGRPTRANSQUEUE_H_
#define EIGRPTRANSQUEUE_H_

#include <list>

#include "EigrpMessage_m.h"
#include "EigrpMsgReq.h"


/*class EigrpMsgReq
{
  public:
    struct EigrpMsgRoute
    {

        EigrpMsgRoute() : sourceId(0), unreachable(false), invalid(false) {}
    };

  private:
    typedef std::vector<EigrpMsgRoute *> ReqRouteVector;

  public:
    EigrpMsgReq(HeaderOpcode type, int destInterface) :
        type(type), destNeighbor(0), destInterface(destInterface), seqNumber(0) {}

    int getDestInterface() const { return destInterface; }
    void setDestInterface(int destInterface) { this->destInterface = destInterface; }

    int getDestNeighbor() const { return destNeighbor; }
    void setDestNeighbor(int destNeighbor) { this->destNeighbor = destNeighbor; }

    HeaderOpcode getType() const { return type; }
    void setType(HeaderOpcode type) { this->type = type; }

    uint32_t getSeqNumber() const { return seqNumber; }
    void setSeqNumber(uint32_t seqNumber) { this->seqNumber = seqNumber; }

    void addReqRoute(EigrpMsgRoute *reqRoute) { routes.push_back(reqRoute); }
    EigrpMsgReq::EigrpMsgRoute *getReqRoute(int sourceId);

    EigrpTimer *getPacingTimer() const { return pacingt; }
    void setPacingTimer(EigrpTimer *pacingt) { this->pacingt = pacingt; }
};

class EigrpTransQueue
{
  private:
    typedef std::list<EigrpMsgReq *> MessageQueue;
    MessageQueue msgQueue;

  public:
    EigrpTransQueue();
    virtual ~EigrpTransQueue();
    void pushMessage(EigrpMsgReq *msgReq) { msgQueue.push_back(msgReq); }
    EigrpMsgReq *getFirstMessage() { return msgQueue.front(); }
    EigrpMsgReq *popMessage() { EigrpMsgReq *msgReq = msgQueue.front(); msgQueue.pop_back(); return msgReq; }
    int getNumReq() const { return msgQueue.size(); }
    EigrpMsgReq *findMessage(uint32_t seqNumber);
    EigrpMsgReq *removeMessage(EigrpMsgReq *msgReq);
};*/

#endif /* EIGRPTRANSQUEUE_H_ */
