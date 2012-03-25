#ifndef __INET_LINKSTATEREQUESTHANDLER_H
#define __INET_LINKSTATEREQUESTHANDLER_H

#include "AnsaIMessageHandler.h"

namespace AnsaOSPF {

class LinkStateRequestHandler : public IMessageHandler {
public:
    LinkStateRequestHandler(Router* containingRouter);

    void    ProcessPacket(OSPFPacket* packet, Interface* intf, Neighbor* neighbor);
};

} // namespace AnsaOSPF

#endif // __INET_LINKSTATEREQUESTHANDLER_H

