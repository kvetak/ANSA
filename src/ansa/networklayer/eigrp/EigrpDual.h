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

#ifndef EIGRPDUAL_H_
#define EIGRPDUAL_H_

#include <omnetpp.h>

#include "IPv4Address.h"

#include "EigrpRoute.h"
#include "IEigrpPdm.h"

class EigrpDual : public cObject /* cSimpleModule */
{
  public:
    enum DualEvent
    {
        RECV_UPDATE = 1,   /**< Received update message */
        RECV_QUERY,         /**< Received query message */
        RECV_REPLY,         /**< Received reply message */
        INTERFACE_UP,       /**< Interface went up */
        //INTERFACE_DOWN,    /**< Interface went down */
        NEIGHBOR_DOWN,      /**< Neighbor went down */
        NEW_NETWORK,        /**< New network added */
    };

  protected:
    IEigrpPdm *pdm;     /**< Protocol dependent module interface */

    //virtual void initialize(int stage);
    //virtual void handleMessage(cMessage *msg);
    //virtual int numInitStages() const { return 4; }

    void printEvent(const char *event, EigrpRoute<IPv4Address> *route, EigrpRouteSource<IPv4Address> *source);

    void updateRoutingTable(EigrpRouteSource<IPv4Address> *oldSuccessor, EigrpRouteSource<IPv4Address> *successor);
    EigrpRouteSource<IPv4Address> * updateTopologyTable(EigrpRoute<IPv4Address> *route, uint32_t dmin);

    void processQo1Passive(DualEvent event, EigrpRouteSource<IPv4Address> *source, bool isSourceNew, EigrpRoute<IPv4Address> *route);
    void processQo1Active(DualEvent event, EigrpRouteSource<IPv4Address> *source, bool isSourceNew, EigrpRoute<IPv4Address> *route);
    void processQo3(DualEvent event, EigrpRouteSource<IPv4Address> *source, bool isSourceNew, EigrpRoute<IPv4Address> *route);

    void processTransition1(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, uint32_t dmin);
    void processTransition2(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, uint32_t dmin);
    void processTransition3(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, uint32_t dmin);
    void processTransition4(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route, uint32_t dmin);
    void processTransition8(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route);
    void processTransition13(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route);
    void processTransition15(int event, EigrpRouteSource<IPv4Address> *source, EigrpRoute<IPv4Address> *route);

  public:
    EigrpDual(IEigrpPdm *pdm) { this->pdm = pdm; }

    void processEvent(DualEvent event, EigrpRouteSource<IPv4Address> *source, bool isSourceNew);
};

#endif /* EIGRPDUAL_H_ */
