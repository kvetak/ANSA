/********** DONE **********/

#ifndef ANSAIMESSAGEHANDLER6_H_
#define ANSAIMESSAGEHANDLER6_H_

#include "ansaOspfPacket6_m.h"

namespace AnsaOspf6 {

class Router;
class Interface;
class Neighbor;

class IMessageHandler {
protected:
    Router* router;

public:
    IMessageHandler(Router* containingRouter)  { router = containingRouter; }
    virtual ~IMessageHandler() {}

    virtual void ProcessPacket(OspfPacket6*, Interface* intf, Neighbor* neighbor) = 0;
};

}

#endif /* ANSAIMESSAGEHANDLER6_H_ */
