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

#include <EigrpTransQueue.h>


/*EigrpTransQueue::EigrpTransQueue()
{

}

EigrpTransQueue::~EigrpTransQueue()
{
    MessageQueue::iterator it;
    for (it = msgQueue.begin(); it != msgQueue.end(); it++)
    {
        // TODO bude problém - musí se zrušit všechny timery (pacing)
        delete *it;
    }
}

EigrpMsgReq *EigrpTransQueue::findMessage(uint32_t seqNumber)
{
    MessageQueue::iterator it;
    for (it = msgQueue.begin(); it != msgQueue.end(); it++)
    {
        if ((*it)->getSeqNumber() == seqNumber)
        return *it;
    }
    return NULL;
}

EigrpMsgReq *EigrpTransQueue::removeMessage(EigrpMsgReq *msgReq)
{
    MessageQueue::iterator it;
    for (it = msgQueue.begin(); it != msgQueue.end(); it++)
    {
        if ((*it)->getSeqNumber() == msgReq->getSeqNumber())
        {
            msgQueue.erase(it);
            return msgReq;
        }
    }
    return NULL;
}*/

/*EigrpMsgRoute *EigrpTransQueue::getReqRoute(int sourceId)
{
    ReqRouteVector::iterator it;
    for (it = routes.begin(); it != routes.end(); it++)
    {
        if ((*it)->sourceId == sourceId)
        return *it;
    }
    return NULL;
}*/

