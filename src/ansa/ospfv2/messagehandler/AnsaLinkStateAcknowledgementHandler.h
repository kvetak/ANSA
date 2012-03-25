#ifndef __INET_LINKSTATEACKNOWLEDGEMENTHANDLER_H
#define __INET_LINKSTATEACKNOWLEDGEMENTHANDLER_H

#include "AnsaIMessageHandler.h"

namespace AnsaOSPF {

class LinkStateAcknowledgementHandler : public IMessageHandler {
public:
    LinkStateAcknowledgementHandler(Router* containingRouter);

    void    ProcessPacket(OSPFPacket* packet, Interface* intf, Neighbor* neighbor);
};

} // namespace AnsaOSPF

#endif // __INET_LINKSTATEACKNOWLEDGEMENTHANDLER_H

