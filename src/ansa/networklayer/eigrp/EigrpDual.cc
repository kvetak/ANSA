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

#include "EigrpIpv4Pdm.h"
#include "IPv4Address.h"
#include "EigrpRoute.h"

#define DEBUG

// TODO do TT pridat metodu purge, ktera se provede pri zmene FD pri prechodu do pasivniho stavu

void EigrpDual::printEvent(const char *event, EigrpRoute<IPv4Address> *route, EigrpRouteSource<IPv4Address> *source)
{
    EV << "DUAL: " << event << " for route " << route->getRouteAddress() << " via " << source->getNextHop();
    EV << " (" << source->getMetric() << "/" << source->getRd() << ")" << endl;
}

/**
 * Finds successor and updates distance of route
 */
EigrpRouteSource<IPv4Address> * EigrpDual::updateTopologyTable(EigrpRoute<IPv4Address> *route, uint32_t dmin)
{
    EigrpRouteSource<IPv4Address> *successor;

    if (dmin < route->getFd())
        route->setFd(dmin);

    if (dmin < UINT32_MAX)
    {
        successor = pdm->findSuccessor(route, dmin);

        if (successor != NULL)
        {
            EV << "DUAL found successor " << successor->getNextHop() << " with ";
            EV << successor->getMetric() << "/" << successor->getRd() << endl;

            successor->setSuccessor(true);

            // Actualize Dij and RD
            route->setDij(dmin);
            route->setRdPar(successor->getMetricParams());

            return successor;
        }
    }

    return NULL;
}

void EigrpDual::updateRoutingTable(EigrpRouteSource<IPv4Address> *oldSuccessor, EigrpRouteSource<IPv4Address> *successor)
{
    // Actualize RT
    if (successor != NULL && !successor->isUnreachable())
    {
        if (successor != oldSuccessor)
        { // Add  new successor to RT or replace old successor
            if (oldSuccessor != NULL) oldSuccessor->setSuccessor(false);
            pdm->addRouteToRT(successor);
        }
        else
        { // Update successor
            pdm->updateRouteInRT(successor);
        }
    }
    else
    { // Delete old successor
        oldSuccessor->setSuccessor(false);
        pdm->removeRouteFromRT(oldSuccessor);
    }
}

/*
 */
void EigrpDual::processEvent(DualEvent event, EigrpRouteSource<IPv4Address> *source, bool isSourceNew)
{
    EigrpRoute<IPv4Address> *route = source->getRouteInfo();

    // TODO: ted otestovat interface Up, vyprseni hold timeru a prijeti goodbye
    switch (route->getQueryOrigin())
    {
    case 0: // active state
        break;

    case 1:
        if (route->getReplyStatus() == 0)
        { // passive state
            processQo1Passive(event, source, isSourceNew, route);
        }
        else if (route->getReplyStatus() > 0)
        { // active state
            processQo1Active(event, source, isSourceNew, route);
        }
        break;

    case 2: // active state

        break;

    case 3: // active state
        processQo3(event, source, isSourceNew, route);
        break;

    default:
        throw cRuntimeError("DUAL detected invalid state of route");
        break;
    }
}

//
// States of DUAL
//

void EigrpDual::processQo0(DualEvent event, EigrpRouteSource<IPv4Address> *source, bool isSourceNew, EigrpRoute<IPv4Address> *route)
{
    switch (event)
    {
    case RECV_QUERY:
        printEvent("received query about", route, source);
        if (source->isSuccessor())
        {

        }
        else
        {
            processTransition6(event, source, route);
        }
        break;

    default:
        break;
    }
}

/**
 * @param event Type of input event
 * @param source Data from messages are stored into source. If the source is new,
 *        then it is not inserted into route.
 * @param route Contains actual metric and RD. Dij remains unchanged.
 */
void EigrpDual::processQo1Passive(DualEvent event, EigrpRouteSource<IPv4Address> *source, bool isSourceNew, EigrpRoute<IPv4Address> *route)
{
    uint32_t dmin;
    bool hasFs;

    switch (event)
    {
    // Input event: update message, increase or decrease of D in router
    case EigrpDual::RECV_UPDATE:
        printEvent("received update", route, source);

        if (isSourceNew) pdm->addSourceToTT(source);

        if (pdm->hasFeasibleSuccessor(route, dmin))
        {
            processTransition2(event, source, route, dmin);
        }
        else
        {
            processTransition4(event, source, route, dmin);
        }
        break;

    // Input event: interface up
    case INTERFACE_UP:
        printEvent("interface up", route, source);

        if (isSourceNew) pdm->addSourceToTT(source);

        // Only local computation (zatím, případně rozšířit - více sousedů na 1 rozhraní)
        pdm->hasFeasibleSuccessor(route, dmin);
        processTransition2(event, source, route, dmin);
        break;

    // Input event: new network added
    case NEW_NETWORK:
        printEvent("new network", route, source);

        hasFs = pdm->checkFeasibleSuccessor(route);
        if (isSourceNew) pdm->addSourceToTT(source);
        dmin = pdm->findRouteDMin(route);

        if (hasFs)
        { // Exists other routes to the destination

            // Force loss of all sources of the route
            route->setFd(0);
            processTransition4(event, source, route, dmin);
        }
        else
        { // Route is new
            processTransition2(event, source, route, dmin);
        }
        break;

    // Input event: interface down, removing neighbor (hold timer, k-values,
    // goodbye message)
    case NEIGHBOR_DOWN:
        // Set metric and RD of neighbor to max
        source->setRd(UINT32_MAX);
        source->setMetric(UINT32_MAX);

        printEvent("interface down", route, source);

        if (isSourceNew) pdm->addSourceToTT(source);

        if (pdm->hasFeasibleSuccessor(route, dmin))
        { // TODO netestováno!
            processTransition2(event, source, route, dmin);
        }
        else
        {
            processTransition4(event, source, route, dmin);
        }
        break;

    case RECV_QUERY:
        printEvent("received query about", route, source);

        // z debug výpisu je zřejmé, že z Query zpráv lze se učit cesty. Unreachable zdroje ale také mažou
        if (isSourceNew) pdm->addSourceToTT(source);

        if (pdm->hasFeasibleSuccessor(route, dmin))
        {
            if (source->isSuccessor())
                processTransition2(event, source, route, dmin);
            else
                processTransition1(event, source, route, dmin);
        }
        else
        {
            if (source->isSuccessor())
                processTransition3(event, source, route, dmin);
            else
                processTransition4(event, source, route, dmin);
        }
        break;

    default:
        EV << "DUA:L received unknown event, skipped" << endl;
        break;
    }
}

void EigrpDual::processQo1Active(DualEvent event, EigrpRouteSource<IPv4Address> *source, bool isSourceNew, EigrpRoute<IPv4Address> *route)
{
    switch (event)
    {
    case RECV_QUERY:
        printEvent("received query about", route, source);
        if (source->isSuccessor())
        {

        }
        else
        {
            processTransition6(event, source, route);
        }
        break;

    case RECV_REPLY:
        printEvent("received reply about", route, source);

        if (route->getReplyStatus() == 1)
        { // Last reply
            processTransition15(event, source, route);
        }
        else
        { // Not last reply
            processTransition8(event, source, route);
        }
        break;

    default:
        break;
    }
}

void EigrpDual::processQo2(DualEvent event, EigrpRouteSource<IPv4Address> *source, bool isSourceNew, EigrpRoute<IPv4Address> *route)
{
    switch (event)
    {
    case RECV_QUERY:
        printEvent("received query about", route, source);
        if (source->isSuccessor())
        {

        }
        else
        {
            processTransition6(event, source, route);
        }
        break;

    default:
        break;
    }
}

void EigrpDual::processQo3(DualEvent event, EigrpRouteSource<IPv4Address> *source, bool isSourceNew, EigrpRoute<IPv4Address> *route)
{
    switch (event)
    {
    case RECV_QUERY:
        printEvent("received query about", route, source);
        if (source->isSuccessor())
        {

        }
        else
        {
            processTransition6(event, source, route);
        }
        break;

    case RECV_REPLY:
        printEvent("received reply about", route, source);

        if (route->getReplyStatus() == 1)
        { // Last reply
            processTransition13(event, source, route);
        }
        else
        { // Not last reply
            processTransition8(event, source, route);
        }
        break;

    default:
        break;
    }
}

//
// Transitions of DUAL
//

void EigrpDual::processTransition1(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, uint32_t dmin)
{
    EV << "DUAL: transit from oij=1 (passive) to oij=1 (passive) by transition 1" << endl;

    EigrpRouteSource<IPv4Address> *successor = pdm->getFirstSuccessor(route);
    pdm->sendReply(route, source->getSourceId(), successor);

    // Remove source if FC is not satisfied
    if (source->getRd() >= route->getFd())
        pdm->removeSourceFromTT(source);
}

void EigrpDual::processTransition2(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, uint32_t dmin)
{
    EV << "DUAL: transit from oij=1 (passive) to oij=1 (passive) by transition 2" << endl;

    //std::vector<EigrpRouteSource<IPv4Address> *> newSuccessors;
    EigrpRouteSource<IPv4Address> *successor, *oldSuccessor;
    typename std::vector<EigrpRouteSource<IPv4Address> *>::iterator it;
    uint32_t oldDij;    // Dij before the event

    oldDij = route->getDij();
    oldSuccessor = pdm->getFirstSuccessor(route);

    // TODO Dmin se mohlo snizit a nektere zdroje v TT uz nemusi splnovat FC, mely by se smazat
    // TODO kdyz prijde novy zdroj (treba z Update zpravy) nesplnuje FC, ale mame S (=> FC OK)
    // pak by tento zdroj nemel byt v TT (viz napr. pocatecni vymena RT pri ustaveni)

    // Find successor and update route distance
    successor = updateTopologyTable(route, dmin);

    // Actualize RT
    updateRoutingTable(oldSuccessor, successor);

    // TODO: posílá se případný update i uzlu, kterému jsme poslali Reply?
    if (event == RECV_QUERY)
        pdm->sendReply(route, source->getSourceId(), successor);

    // Send Update about new Successor if Dij changed
    if (route->getDij() != oldDij && pdm->getNumPeers() > 0)
        pdm->sendUpdate(route, IEigrpPdm::UNSPEC_RECEIVER, successor);

    // Remove source if FC is not satisfied
    if (source->getRd() >= route->getFd())
        pdm->removeSourceFromTT(source);
}

void EigrpDual::processTransition3(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, uint32_t dmin)
{
    EV << "DUAL: transit from oij=1 (passive) to oij=3 (active) by transition 3" << endl;

    int numPeers = pdm->getNumPeers();

    route->setQueryOrigin(3);

    // Actualize distance of route in TT
    route->setDij(source->getMetric());
    route->setRdPar(source->getMetricParams());
    // Actualize FD in TT and RT
    route->setFd(route->getDij());

    // TODO: posílá se query i tomu, od které jsme přijali Query? Má na to vliv split horizon?

    // Send Query with actual distance via S to all peers
    EV << "DUAL: peers = " << numPeers << endl;
    if (numPeers > 0)
    {
        route->setReplyStatus(numPeers);
        pdm->sendQuery(route, IEigrpPdm::UNSPEC_RECEIVER, source);
    }
    else
    { // Diffusion computation can not be performed, go back to passive state
        // ZATÍM NECHAT:
        processTransition13(event, source, route);
    }
}

void EigrpDual::processTransition4(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, uint32_t dmin)
{
    EV << "DUAL: transit from oij=1 (passive) to oij=1 (active) by transition 4" << endl;

    int numPeers = pdm->getNumPeers();
    EigrpRouteSource<IPv4Address> *successor;

    route->setQueryOrigin(1);

    // Get actual successor
    successor = pdm->getFirstSuccessor(route);

    // Actualize distance of route in TT
    route->setDij(successor->getMetric());
    route->setRdPar(successor->getMetricParams());
    // Actualize FD in TT and RT
    route->setFd(route->getDij());

    // Send Reply with RDij if input event is Query
    if (event == RECV_QUERY)
        pdm->sendReply(route, source->getSourceId(), successor);

    // Start own diffusion computation
    EV << "DUAL: peers = " << numPeers << endl;
    if (numPeers > 0)
    {
        route->setReplyStatus(numPeers);
        pdm->sendQuery(route, IEigrpPdm::UNSPEC_RECEIVER, source);
    }
    else
    { // Go to passive state
        // Tady by podle Cisco DUALu se mělo jen přejít do pasivního stavu (prostě nastavit
        // origin = 1, oznámit, že nejdu do aktivního stavu, a smazat daný zdroj - successora).
        // Tohle platí i pro přechod č. 3 v případě zapnutého split horizon
        // Výsledek je ale stejný
        // ZATÍM TAKHLE:
        route->setReplyStatus(1);
        processTransition15(event, source, route);
    }
}

void EigrpDual::processTransition6(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route)
{
    EV << "DUAL: transit from oij=" << route->getQueryOrigin() << " (active) to oij=" << route->getQueryOrigin() << " (active) by transition 6" << endl;

    pdm->sendReply(route, source->getSourceId(), source);
}

void EigrpDual::processTransition8(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route)
{
    EV << "DUAL: transit from oij=" << route->getQueryOrigin() << " (active) to oij=" << route->getQueryOrigin() << " (active) by transition 8" << endl;

    // Record receipt of message
    int replyStatus = route->decrementReplyStatus();
    EV << "Reply status = " << replyStatus << endl;

    if (event == NEIGHBOR_DOWN) // NEIGHBOR_DOWN = interface down
    { // přesunout tohle o úroveň výše
        // Set Dij = lij = max
        source->setMetric(UINT32_MAX);
        source->setRd(UINT32_MAX);
    }
}

void EigrpDual::processTransition13(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route)
{
    EV << "DUAL: transit from oij=3 (active) to oij=1 (passive) by transition 13" << endl;

    int replyStatus = route->decrementReplyStatus();
    EigrpRouteSource<IPv4Address> *successor;
    EigrpRouteSource<IPv4Address> *oldSuccessor = pdm->getFirstSuccessor(route);
    uint32_t oldDij = route->getDij();
    uint32_t dmin;

    EV << "Reply status = " << replyStatus << endl;

    // Set FD to max
    route->setFd(UINT32_MAX);

    // Find min distance
    dmin = pdm->findRouteDMin(route);

    // Find successor and update distance of the route
    successor = updateTopologyTable(route, dmin);

    updateRoutingTable(oldSuccessor, successor);

    // Send Reply to old successor about new successor or unreachable route (successor = NULL)
    if (oldSuccessor != NULL)
        pdm->sendReply(route, oldSuccessor->getNexthopId(), successor);

    // Send update about change of metric
    if (oldDij != route->getDij() && pdm->getNumPeers() > 0)
        pdm->sendUpdate(route, IEigrpPdm::UNSPEC_RECEIVER, successor);

    // Remove source if FC is not satisfied
    if (source->getRd() >= route->getFd())
        pdm->removeSourceFromTT(source);
}

void EigrpDual::processTransition15(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route)
{
    EV << "DUAL: transit from oij=1 (active) to oij=1 (passive) by transition 15" << endl;

    EigrpRouteSource<IPv4Address> *successor = NULL;
    EigrpRouteSource<IPv4Address> *oldSuccessor = pdm->getFirstSuccessor(route);
    uint32_t oldDij = route->getDij();
    uint32_t dmin;

    route->decrementReplyStatus();

    // Set FD to max
    route->setFd(UINT32_MAX);

    // Find min distance
    dmin = pdm->findRouteDMin(route);

    // Find successor
    successor = updateTopologyTable(route, dmin);

    // Add successor to RT
    updateRoutingTable(oldSuccessor, successor);

    // Send update about change
    if (oldDij != route->getDij() && pdm->getNumPeers() > 0)
        pdm->sendUpdate(route, IEigrpPdm::UNSPEC_RECEIVER, successor);

    // Remove source if FC is not satisfied
    if (source->getRd() >= route->getFd())
        pdm->removeSourceFromTT(source);
}


