/*
 * EigrpMsgReq.cc
 *
 *  Created on: Apr 3, 2014
 *      Author: honza
 */

#include "ansa/networklayer/eigrp/messages/EigrpMsgReq.h"

int EigrpMsgReq::findMsgRoute(int routeId) const
{
    EigrpMsgRoute rt;

    for (unsigned i = 0; i < getRoutesArraySize(); i++)
    {
        rt = getRoutes(i);
        if (rt.routeId == routeId)
        {
            return i;
        }
    }
    return -1;
}
