/*
 * HELLO_RECEIVED - restart Inactivity timer, transition to INIT
 * START - go to ATTEMPT, start Inactivity timer
 * LL_DOWN - new state DOWN
 * INACTIVITY_TIMER - new state DOWN
 */

#include "ansa/routing/ospfv3/neighbor/OSPFv3Neighbor.h"
#include "ansa/routing/ospfv3/neighbor/OSPFv3NeighborStateDown.h"
#include "ansa/routing/ospfv3/neighbor/OSPFv3NeighborStateInit.h"
#include "ansa/routing/ospfv3/neighbor/OSPFv3NeighborStateAttempt.h"


namespace inet{
void OSPFv3NeighborStateDown::processEvent(OSPFv3Neighbor *neighbor, OSPFv3Neighbor::OSPFv3NeighborEventType event)
{
    /*
      Send an Hello Packet to the neighbor (this neighbor
      is always associated with an NBMA network) and start
      the Inactivity Timer for the neighbor.  The timer's
      later firing would indicate that communication with
      the neighbor was not attained.
    */
    if (event == OSPFv3Neighbor::START) {
        int hopLimit = (neighbor->getInterface()->getType() == OSPFv3Interface::VIRTUAL_TYPE) ? VIRTUAL_LINK_TTL : 1;
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->clearTimer(neighbor->getPollTimer());
        OSPFv3HelloPacket* hello = neighbor->getInterface()->prepareHello();
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->sendPacket(hello, neighbor->getNeighborIP(), neighbor->getInterface()->getIntName().c_str(), hopLimit);
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->setTimer(neighbor->getInactivityTimer(), neighbor->getNeighborDeadInterval());
        this->changeState(neighbor, new OSPFv3NeighborStateAttempt, this);
    }

    /*
      Start the Inactivity Timer for the neighbor.  The
      timer's later firing would indicate that the
      neighbor is dead.
    */
    if (event == OSPFv3Neighbor::HELLO_RECEIVED) {
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->clearTimer(neighbor->getPollTimer());
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->setTimer(neighbor->getInactivityTimer(), neighbor->getNeighborDeadInterval());
        //I should contact the neighbor immediately

        EV_DEBUG << "HELLO_RECEIVED, number of neighbors: " << neighbor->getInterface()->getNeighborCount() << endl;
        OSPFv3HelloPacket* hello = neighbor->getInterface()->prepareHello();
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->sendPacket(hello, neighbor->getNeighborIP(),neighbor->getInterface()->getIntName().c_str(), 1);
        changeState(neighbor, new OSPFv3NeighborStateInit, this);
    }
    if (event == OSPFv3Neighbor::POLL_TIMER) {
        int hopLimit = (neighbor->getInterface()->getType() == OSPFv3Interface::VIRTUAL_TYPE) ? VIRTUAL_LINK_TTL : 1;
        OSPFv3HelloPacket* hello = neighbor->getInterface()->prepareHello();
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->sendPacket(hello, neighbor->getNeighborIP(), neighbor->getInterface()->getIntName().c_str(), hopLimit);
        neighbor->getInterface()->getArea()->getInstance()->getProcess()->setTimer(neighbor->getPollTimer(), neighbor->getNeighborDeadInterval());
    }
}
}//namespace inet
