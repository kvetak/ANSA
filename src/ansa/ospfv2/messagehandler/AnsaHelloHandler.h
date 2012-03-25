#ifndef __INET_HELLOHANDLER_H
#define __INET_HELLOHANDLER_H

#include "AnsaIMessageHandler.h"

namespace AnsaOSPF {

class HelloHandler : public IMessageHandler {
public:
    HelloHandler(Router* containingRouter);

    void    ProcessPacket(OSPFPacket* packet, Interface* intf, Neighbor* unused = NULL);
};

} // namespace AnsaOSPF

#endif // __INET_HELLOHANDLER_H

