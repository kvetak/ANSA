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

#include "EigrpDual.h"

#include <omnetpp.h>

#define EIGRP_DUAL_DEBUG

namespace eigrpDual
{
// User messages
const char *userMsgs[] =
{
    // RECV_UPDATE
    "received Update",
    // RECV_QUERY
    "received Query",
    // RECV_REPLY
    "received Reply",
    // NEIGHBOR_DOWN
    "neighbor down",
    // INTERFACE_DOWN
    "interface down",
    //INTERFACE_UP
    "interface up",
};
};  // end of namespace eigrpDual

/**
 * @param event Type of input event
 * @param source Data from messages are stored into source. If the source is new,
 *        then it is not inserted into route.
 * @param route Contains actual metric and RD. Dij remains unchanged.
 */
void EigrpDual::processEvent(DualEvent event, EigrpRouteSource<IPv4Address> *source, int neighborId)
{
    EigrpRoute<IPv4Address> *route = source->getRouteInfo();

    ev << "DUAL: " << eigrpDual::userMsgs[event];
    ev << " for route " << route->getRouteAddress() << " via " << source->getNextHop();
    ev << " (" << source->getMetric() << "/" << source->getRd() << ")" << endl;

    switch (route->getQueryOrigin())
    {
    case 0: // active state
        processQo0(event, source, route, neighborId);
        break;

    case 1:
        if (route->getReplyStatusSum() == 0)
        { // passive state
            processQo1Passive(event, source, route, neighborId);
        }
        else
        { // active state
            processQo1Active(event, source, route, neighborId);
        }
        break;

    case 2: // active state
        processQo2(event, source, route, neighborId);
        break;

    case 3: // active state
        processQo3(event, source, route, neighborId);
        break;

    default:
        //DUAL detected invalid state of route
        ASSERT(false);
        break;
    }
}

//
// States of DUAL
//

void EigrpDual::processQo0(DualEvent event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, int neighborId)
{
    uint32_t dmin;
    bool hasReplyStatus;

    switch (event)
    {
    case RECV_UPDATE:
        processTransition7(event, source, route, neighborId);
        break;

    case RECV_QUERY:
        if (source->isSuccessor())
            processTransition5(event, source, route, neighborId);
        else
            processTransition6(event, source, route, neighborId);
        break;

    case RECV_REPLY:
    case NEIGHBOR_DOWN:
    case INTERFACE_DOWN:
        if (event == NEIGHBOR_DOWN || event == INTERFACE_DOWN)
        {
            source->setMetric(UINT32_MAX);
            source->setRd(UINT32_MAX);
        }

        hasReplyStatus = route->unsetReplyStatus(neighborId);
        if (route->getReplyStatusSum() == 0)
        { // As last reply from neighbor
            // Check FC with FDij(t)
            if (pdm->hasFeasibleSuccessor(route, dmin))
                processTransition14(event, source, route, dmin, neighborId);
            else
                processTransition11(event, source, route, dmin, neighborId);
        }
        else if (hasReplyStatus)
        { // As not last reply from neighbor
            processTransition8(event, source, route, neighborId);
        }
        // else do nothing (connected source)
        break;

    case INTERFACE_UP:
        // do nothing
        break;

    default:
        ev << "DUAL received invalid input event num. " << event << " in active state 0" << endl;
        ASSERT(false);
        break;
    }
}

void EigrpDual::processQo1Passive(DualEvent event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, int neighborId)
{
    uint32_t dmin;

    switch (event)
    {
    case EigrpDual::RECV_UPDATE:
        if (pdm->hasFeasibleSuccessor(route, dmin))
            processTransition2(event, source, route, dmin, neighborId);
        else
            processTransition4(event, source, route, dmin, neighborId);
        break;

    case INTERFACE_UP:
        dmin = pdm->findRouteDMin(route);

        if (pdm->getFirstSuccessor(route) != NULL)
        { // Exists other routes to the destination
            // Force loss of all sources of the route
            route->setFd(0);
            processTransition4(event, source, route, dmin, neighborId);
        }
        else
        { // Route is new
            processTransition2(event, source, route, dmin, neighborId);
        }
        break;

    case NEIGHBOR_DOWN:
    case INTERFACE_DOWN:
        // Set metric and RD of neighbor to max
        source->setRd(UINT32_MAX);
        source->setMetric(UINT32_MAX);

        if (pdm->hasFeasibleSuccessor(route, dmin))
            processTransition2(event, source, route, dmin, neighborId);
        else
            processTransition4(event, source, route, dmin, neighborId);
        break;

    case RECV_QUERY:
        if (pdm->hasFeasibleSuccessor(route, dmin))
        {
            if (source->isSuccessor())
                processTransition2(event, source, route, dmin, neighborId);
            else
                processTransition1(event, source, route, dmin, neighborId);
        }
        else
        {
            if (source->isSuccessor())
                processTransition3(event, source, route, dmin, neighborId);
            else
                processTransition4(event, source, route, dmin, neighborId);
        }
        break;

    default:
        ASSERT(false);
        ev << "DUAL received invalid input event in passive state 0, skipped" << endl;
        break;
    }
}

void EigrpDual::processQo1Active(DualEvent event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, int neighborId)
{
    bool hasReplyStatus;

    switch (event)
    {
    case RECV_UPDATE:
        if (source->isSuccessor())
            processTransition9(event, source, route, neighborId);
        else
            processTransition17(event, source, route, neighborId);
        break;

    case RECV_QUERY:
        if (source->isSuccessor())
            processTransition5(event, source, route, neighborId);
        else
            processTransition6(event, source, route, neighborId);
        break;

    case RECV_REPLY:
        hasReplyStatus = route->unsetReplyStatus(neighborId);
        if (route->getReplyStatusSum() == 0) // Last reply
            processTransition15(event, source, route, neighborId);
        else if (hasReplyStatus) // Not last reply
            processTransition18(event, source, route, neighborId);
        // else do nothing
        break;

    case NEIGHBOR_DOWN:
    case INTERFACE_DOWN:
        source->setMetric(UINT32_MAX);
        source->setRd(UINT32_MAX);

        hasReplyStatus = route->unsetReplyStatus(neighborId);

        // Transition 9 should take precedence over transition 15 (fail of link
        // to S (as last reply) versus fail of link to not S (as last reply)
        // TODO ověřit!
        if (source->isSuccessor())
            processTransition9(event, source, route, neighborId);

        if (route->getReplyStatusSum() == 0)
        { // As last reply from neighbor
            processTransition15(event, source, route, neighborId);
        }
        else if (hasReplyStatus)
        { // As not last reply from neighbor
            processTransition18(event, source, route, neighborId);
        }
        // else do nothing
        break;

    case INTERFACE_UP:
        // do nothing
        break;

    default:
        ASSERT(false);
        ev << "DUAL received invalid input event in active state 1, skipped" << endl;
        break;
    }
}

void EigrpDual::processQo2(DualEvent event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, int neighborId)
{
    uint32_t dmin;
    bool hasReplyStatus;

    switch (event)
    {
    case RECV_UPDATE:
        processTransition7(event, source, route, neighborId);
        break;

    case RECV_QUERY:
        if (!source->isSuccessor())
            processTransition6(event, source, route, neighborId);

        else
        {
            ASSERT(false);
            // do nothing (DUAL can not receive Query form S when it is in active state)
        }
        break;

    case RECV_REPLY:
    case NEIGHBOR_DOWN:
    case INTERFACE_DOWN:
        if (event == NEIGHBOR_DOWN || event == INTERFACE_DOWN)
        {
            source->setMetric(UINT32_MAX);
            source->setRd(UINT32_MAX);
        }

        hasReplyStatus = route->unsetReplyStatus(neighborId);
        if (route->getReplyStatusSum() == 0)
        { // As last reply from neighbor
            // Check FC with FDij(t)
            if (pdm->hasFeasibleSuccessor(route, dmin))
                processTransition16(event, source, route, dmin, neighborId);
            else
                processTransition12(event, source, route, dmin, neighborId);
        }
        else if (hasReplyStatus)
        { // As not last reply from neighbor
            processTransition8(event, source, route, neighborId);
        }
        // else do nothing (connected source)
        break;

    case INTERFACE_UP:
        // do nothing
        break;

    default:
        ASSERT(false);
        ev << "DUAL received invalid input event in active state 2, skipped" << endl;
        break;
    }
}

void EigrpDual::processQo3(DualEvent event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, int neighborId)
{
    bool hasReplyStatus;

    switch (event)
    {
    case RECV_UPDATE:
        if (source->isSuccessor())
            processTransition10(event, source, route, neighborId);
        else
            processTransition17(event, source, route, neighborId);
        break;

    case RECV_QUERY:
        if (!source->isSuccessor())
            processTransition6(event, source, route, neighborId);
        else
        {
            ASSERT(false);
            // do nothing (DUAL can not receive Query form S when it is in active state)
        }
        break;

    case RECV_REPLY:
        hasReplyStatus = route->unsetReplyStatus(neighborId);
        if (route->getReplyStatusSum() == 0) // Last reply
            processTransition13(event, source, route, neighborId);
        else if (hasReplyStatus) // Not last reply
            processTransition18(event, source, route, neighborId);
        // else do nothing
        break;

    case NEIGHBOR_DOWN:
    case INTERFACE_DOWN:
        source->setMetric(UINT32_MAX);
        source->setRd(UINT32_MAX);

        hasReplyStatus = route->unsetReplyStatus(neighborId);

        // Transition 10 should take precedence over transition 13 (fail of link
        // to S (as last reply) versus fail of link to not S (as last reply)
        // TODO ověřit!
        if (source->isSuccessor())
            processTransition10(event, source, route, neighborId);

        if (route->getReplyStatusSum() == 0)
        { // As last reply from neighbor
            processTransition13(event, source, route, neighborId);
        }
        else if (hasReplyStatus)
        { // As not last reply from neighbor
            processTransition18(event, source, route, neighborId);
        }
        // else do nothing
        break;

    case INTERFACE_UP:
        // do nothing
        break;

    default:
        ASSERT(false);
        ev << "DUAL received invalid input event in active state 3, skipped" << endl;
        break;
    }
}

//
// Transitions of DUAL
//

void EigrpDual::processTransition1(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, uint32_t dmin, int neighborId)
{
    EV << "DUAL: transit from oij=1 (passive) to oij=1 (passive) by transition 1" << endl;

    EigrpRouteSource<IPv4Address> *successor = pdm->getFirstSuccessor(route);
    pdm->sendReply(route, neighborId, successor);

    if (source->isUnreachable())
        pdm->removeSourceFromTT(source);
}

void EigrpDual::processTransition2(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, uint32_t dmin, int neighborId)
{
    EV << "DUAL: transit from oij=1 (passive) to oij=1 (passive) by transition 2" << endl;

    EigrpRouteSource<IPv4Address> *successor;
    typename std::vector<EigrpRouteSource<IPv4Address> *>::iterator it;
    uint32_t oldDij;    // Dij before the event

    oldDij = route->getDij();

    // Find successors and update route in TT and RT
    successor = pdm->updateRoute(route, dmin);

    if (event == RECV_QUERY)
    {
        ASSERT(successor != NULL);
        pdm->sendReply(route, neighborId, successor);
    }

    // Send Update about new Successor if Dij changed
    if (route->getDij() != oldDij && successor != NULL && pdm->hasNeighbor(successor))
        pdm->sendUpdate(IEigrpPdm::UNSPEC_RECEIVER, route, successor);

    pdm->purgeTT(route->getRouteId());
}

void EigrpDual::processTransition3(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, uint32_t dmin, int neighborId)
{
    EV << "DUAL: transit from oij=1 (passive) to oij=3 (active) by transition 3" << endl;
    // Note: source is successor

    int numPeers;
    route->setQueryOrigin(3);

    // Actualize distance of route and FD in TT
    // route->setDij(source->getMetric()); // not there (in transition to passive state DUAL can not detect change of Dij)
    route->setRdPar(source->getMetricParams());
    route->setFd(route->getDij());

    // Send Query with actual distance via successor to all peers
    numPeers = pdm->setReplyStatusTable(route, source, false);
    EV << "DUAL: peers = " << numPeers << endl;

    if (numPeers > 0)
    {
        pdm->sendQuery(IEigrpPdm::UNSPEC_RECEIVER, route, source);
    }
    else
    { // Diffusion computation can not be performed, go back to passive state
        goToPassiveFromQo3(event, source, route, neighborId);
        return;
    }

    if (source->isUnreachable())
        pdm->removeSourceFromTT(source);
}

void EigrpDual::processTransition4(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, uint32_t dmin, int neighborId)
{
    EV << "DUAL: transit from oij=1 (passive) to oij=1 (active) by transition 4" << endl;

    int numPeers;
    EigrpRouteSource<IPv4Address> *oldSuccessor;

    route->setQueryOrigin(1);

    // Get old successor
    oldSuccessor = pdm->getFirstSuccessor(route);
    // Old successor may not be null
    if (oldSuccessor == NULL) oldSuccessor = source;

    // Actualize distance of route in TT
    route->setDij(oldSuccessor->getMetric());
    route->setRdPar(oldSuccessor->getMetricParams());
    // Actualize FD in TT
    route->setFd(route->getDij());

    // Send Reply with RDij if input event is Query
    if (event == RECV_QUERY)
        pdm->sendReply(route, neighborId, oldSuccessor);

    // Start own diffusion computation
    numPeers = pdm->setReplyStatusTable(route, source, true);
    EV << "DUAL: peers = " << numPeers << endl;
    if (numPeers > 0)
    {
        int srcNeighbor = (neighborId != IEigrpPdm::UNSPEC_RECEIVER) ? neighborId : IEigrpPdm::UNSPEC_SENDER;
        pdm->sendQuery(IEigrpPdm::UNSPEC_RECEIVER, route, oldSuccessor, srcNeighbor);
    }
    else
    { // Go to passive state
        gotoPassiveFromQo1(event, source, route, neighborId);
        return;
    }

    if (source->isUnreachable())
        pdm->removeSourceFromTT(source);
}

void EigrpDual::processTransition5(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, int neighborId)
{
    EV << "DUAL: transit from oij=" << route->getQueryOrigin() << " (active) to oij=2 (active) by transition 5" << endl;

    route->setQueryOrigin(2);

    if (source->isUnreachable())
        pdm->removeSourceFromTT(source);
}

void EigrpDual::processTransition6(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, int neighborId)
{
    EV << "DUAL: transit from oij=" << route->getQueryOrigin() << " (active) to oij=" << route->getQueryOrigin() << " (active) by transition 6" << endl;

    EigrpRouteSource<IPv4Address> *oldSuccessor = pdm->getFirstSuccessor(route);
    if (oldSuccessor == NULL)   oldSuccessor = source;

    pdm->sendReply(route, neighborId, oldSuccessor);

    if (source->isUnreachable())
        pdm->removeSourceFromTT(source);
}

void EigrpDual::processTransition7(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, int neighborId)
{
    EV << "DUAL: transit from oij=" << route->getQueryOrigin() << " (active) to oij=" << route->getQueryOrigin() << " (active) by transition 7" << endl;

    // TODO: neměla by se nová vzdálenost přes S zaznamenat do Dij a RDij cesty??? Podle mě jo.
    //        Musím nějak zjistit, jestli se zaznamená do RDij (v 9. přechodu ne)

    // Actualize Dij of route by new distance via successor
    route->setDij(source->getMetric());

    if (source->isUnreachable())
        pdm->removeSourceFromTT(source);
}

void EigrpDual::processTransition8(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, int neighborId)
{
    EV << "DUAL: transit from oij=" << route->getQueryOrigin() << " (active) to oij=" << route->getQueryOrigin() << " (active) by transition 8" << endl;

    EV << "     Reply status = " << route->getReplyStatusSum() << endl;

    if (source->isUnreachable())
        pdm->removeSourceFromTT(source);
}

void EigrpDual::processTransition9(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, int neighborId)
{
    EV << "DUAL: transit from oij=1 (active) to oij=0 (active) by transition 9" << endl;

    route->setQueryOrigin(0);
    EV << "     Reply status = " << route->getReplyStatusSum() << endl;

    // Actualize Dij of route
    route->setDij(source->getMetric());

    if (route->getReplyStatusSum() == 0)
    { // Reply status table is empty, go to passive state or start new diffusing computation
        uint32_t dmin;
        if (pdm->hasFeasibleSuccessor(route, dmin))
            processTransition14(event, source, route, dmin, neighborId);
        else
            processTransition11(event, source, route, dmin, neighborId);
    }

    if (source->isUnreachable())
        pdm->removeSourceFromTT(source);
}

void EigrpDual::processTransition10(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, int neighborId)
{
    EV << "DUAL: transit from oij=3 (active) to oij=2 (active) by transition 10" << endl;

    route->setQueryOrigin(2);
    EV << "     Reply status = " << route->getReplyStatusSum() << endl;

    // Actualize Dij of route
    route->setDij(source->getMetric());

    if (route->getReplyStatusSum() == 0)
    { // Reply status table is empty, go to passive state or start new diffusing computation
        uint32_t dmin;
        if (pdm->hasFeasibleSuccessor(route, dmin))
            processTransition16(event, source, route, dmin, neighborId);
        else
            processTransition12(event, source, route, dmin, neighborId);
    }

    if (source->isUnreachable())
        pdm->removeSourceFromTT(source);
}

void EigrpDual::processTransition11(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, uint32_t dmin, int neighborId)
{
    EV << "DUAL: transit from oij=0 (active) to oij=1 (active) by transition 11" << endl;

    int numPeers;

    route->setQueryOrigin(1);
    EV << "     Reply status = " << route->getReplyStatusSum() << endl;

    numPeers = pdm->setReplyStatusTable(route, source);
    EV << "DUAL: peers = " << numPeers << endl;
    if (numPeers > 0)
    { // Start new diffusion computation
        int srcNeighbor = (neighborId != IEigrpPdm::UNSPEC_RECEIVER) ? neighborId : IEigrpPdm::UNSPEC_SENDER;
        pdm->sendQuery(IEigrpPdm::UNSPEC_RECEIVER, route, source, srcNeighbor);
    }
    else
    { // Go to passive state
        gotoPassiveFromQo1(event, source, route, neighborId);
        return;
    }

    if (source->isUnreachable())
        pdm->removeSourceFromTT(source);
}

void EigrpDual::processTransition12(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, uint32_t dmin, int neighborId)
{
    EV << "DUAL: transit from oij=2 (active) to oij=3 (active) by transition 12" << endl;

    int numPeers;

    route->setQueryOrigin(3);

    numPeers = pdm->setReplyStatusTable(route, source);
    EV << "DUAL: peers = " << numPeers << endl;
    if (numPeers > 0)
    { // Start new diffusion computation
        pdm->sendQuery(IEigrpPdm::UNSPEC_RECEIVER, route, source);
    }
    else
    { // Go to passive state
        goToPassiveFromQo3(event, source, route, neighborId);
        return;
    }

    if (source->isUnreachable())
        pdm->removeSourceFromTT(source);
}

void EigrpDual::processTransition13(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, int neighborId)
{
    EV << "DUAL: transit from oij=3 (active) to oij=1 (passive) by transition 13" << endl;

    EigrpRouteSource<IPv4Address> *successor;
    // Old successor is originator of Query
    EigrpRouteSource<IPv4Address> *oldSuccessor = pdm->getFirstSuccessor(route);
    uint32_t oldDij = route->getDij();
    uint32_t dmin;

    route->setQueryOrigin(1);
    EV << "     Reply status = " << route->getReplyStatusSum() << endl;

    // Set FD to max
    route->setFd(UINT32_MAX);

    // Find min distance
    dmin = pdm->findRouteDMin(route);

    // Find successor and update distance of the route
    successor = pdm->updateRoute(route, dmin);

    // Send Reply to old successor about new successor or unreachable route
    if (oldSuccessor != NULL)
    {
        ASSERT(successor != NULL);
        pdm->sendReply(route, oldSuccessor->getNexthopId(), successor);
    }

    // Send update about change of metric
    if (oldDij != route->getDij() && successor != NULL && pdm->hasNeighbor(successor))
        pdm->sendUpdate(IEigrpPdm::UNSPEC_RECEIVER, route, successor);

    pdm->purgeTT(route->getRouteId());
}

void EigrpDual::goToPassiveFromQo3(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, int neighborId)
{
    EV << "DUAL: transit to oij=1 (passive) by optimized transition 13" << endl;

    uint32_t dmin;
    uint32_t oldDij = route->getDij();
    EigrpRouteSource<IPv4Address> *successor = NULL;
    // Old successor is originator of Query
    EigrpRouteSource<IPv4Address> *oldSuccessor = pdm->getFirstSuccessor(route);

    route->setQueryOrigin(1);
    // Do not record receive of Reply

    // Set FD to max
    route->setFd(UINT32_MAX);

    // Find min distance
    dmin = pdm->findRouteDMin(route);

    // Find successor and update distance of the route
    successor = pdm->updateRoute(route, dmin);

    // Send Reply to old successor about unreachable route
    if (oldSuccessor != NULL)
    {
        ASSERT(successor != NULL);
        pdm->sendReply(route, oldSuccessor->getNexthopId(), successor, true);
    }

    // Send update about change of metric
    if (oldDij != route->getDij() && successor != NULL && pdm->hasNeighbor(successor))
        pdm->sendUpdate(IEigrpPdm::UNSPEC_RECEIVER, route, successor);

    pdm->purgeTT(route->getRouteId());
}

void EigrpDual::processTransition14(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, uint32_t dmin, int neighborId)
{
    EV << "DUAL: transit from oij=0 (active) to oij=1 (passive) by transition 14" << endl;

    EigrpRouteSource<IPv4Address> *successor;
    uint32_t oldDij = route->getDij();

    route->setQueryOrigin(1);
    EV << "     Reply status = " << route->getReplyStatusSum() << endl;

    // Find successor and update distance of the route
    successor = pdm->updateRoute(route, dmin);
    // Successor may not be null
    if (successor == NULL) successor = source;

    // Do not send Reply (Reply was sent in transition 4)

    // Send update about change of metric
    if (oldDij != route->getDij() && successor != NULL && pdm->hasNeighbor(successor))
        pdm->sendUpdate(IEigrpPdm::UNSPEC_RECEIVER, route, successor);

    pdm->purgeTT(route->getRouteId());
}

void EigrpDual::processTransition15(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, int neighborId)
{
    EV << "DUAL: transit from oij=1 (active) to oij=1 (passive) by transition 15" << endl;

    EigrpRouteSource<IPv4Address> *successor = NULL;
    uint32_t oldDij = route->getDij();
    uint32_t dmin;

    EV << "     Reply status = " << route->getReplyStatusSum() << endl;

    // Set FD to max
    route->setFd(UINT32_MAX);

    // Find min distance
    dmin = pdm->findRouteDMin(route);

    successor = pdm->updateRoute(route, dmin);
    // Successor may not be null
    if (successor == NULL) successor = source;

    // Do not send Reply (Reply was sent in transition 4)

    // Send update about change
    if (oldDij != route->getDij() && successor != NULL && pdm->hasNeighbor(successor))
        pdm->sendUpdate(IEigrpPdm::UNSPEC_RECEIVER, route, successor);

    pdm->purgeTT(route->getRouteId());
}

void EigrpDual::gotoPassiveFromQo1(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, int neighborId)
{
    EV << "DUAL: transit to oij=1 (passive) by optimized transition 15" << endl;

    EigrpRouteSource<IPv4Address> *successor = NULL;
    uint32_t dmin;
    uint32_t oldDij = route->getDij();

    // Do not record receive of Reply

    // Set FD to max
    route->setFd(UINT32_MAX);

    // Find min distance
    dmin = pdm->findRouteDMin(route);

    successor = pdm->updateRoute(route, dmin);

    // Do not send Reply (Reply was sent in transition 4)

    // Send update about change
    if (oldDij != route->getDij() && successor != NULL && pdm->hasNeighbor(successor))
        pdm->sendUpdate(IEigrpPdm::UNSPEC_RECEIVER, route, successor);

    pdm->purgeTT(route->getRouteId());
}

void EigrpDual::processTransition16(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, uint32_t dmin, int neighborId)
{
    EV << "DUAL: transit from oij=2 (active) to oij=1 (passive) by transition 16" << endl;

    EigrpRouteSource<IPv4Address> *successor;
    // Old successor is originator of Query
    EigrpRouteSource<IPv4Address> *oldSuccessor = pdm->getFirstSuccessor(route);
    uint32_t oldDij = route->getDij();

    route->setQueryOrigin(1);
    EV << "     Reply status = " << route->getReplyStatusSum() << endl;

    // Find successor and update distance of the route
    successor = pdm->updateRoute(route, dmin);
    // Successor may not be null
    if (successor == NULL) successor = source;

    // Send Reply to old successor about new successor or unreachable route (successor = NULL)
    if (oldSuccessor != NULL)
    {
        ASSERT(successor != NULL);
        pdm->sendReply(route, oldSuccessor->getNexthopId(), successor);
    }
    // Send update about change of metric
    if (oldDij != route->getDij() && successor != NULL && pdm->hasNeighbor(successor))
        pdm->sendUpdate(IEigrpPdm::UNSPEC_RECEIVER, route, successor);

    pdm->purgeTT(route->getRouteId());
}

void EigrpDual::processTransition17(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, int neighborId)
{
    EV << "DUAL: transit from oij=" << route->getQueryOrigin() << " (active) to oij=" << route->getQueryOrigin() << " (active) by transition 17" << endl;

    // TODO: neměla by se nová vzdálenost přes S zaznamenat do Dij a RDij cesty??? Podle mě jo.
    //        Musím nějak zjistit, jestli se zaznamená do RDij (v 9. přechodu ne)

    // Actualize Dij of route by new distance via successor
    route->setDij(source->getMetric());

    if (source->isUnreachable())
        pdm->removeSourceFromTT(source);
}

void EigrpDual::processTransition18(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, int neighborId)
{
    EV << "DUAL: transit from oij=" << route->getQueryOrigin() << " (active) to oij=" << route->getQueryOrigin() << " (active) by transition 18" << endl;

    EV << "     Reply status = " << route->getReplyStatusSum() << endl;

    if (source->isUnreachable())
        pdm->removeSourceFromTT(source);
}

