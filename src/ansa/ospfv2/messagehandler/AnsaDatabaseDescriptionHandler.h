#ifndef __INET_DATABASEDESCRIPTIONHANDLER_H
#define __INET_DATABASEDESCRIPTIONHANDLER_H

#include "AnsaIMessageHandler.h"

namespace AnsaOSPF {

class DatabaseDescriptionHandler : public IMessageHandler {
private:
    bool ProcessDDPacket(OSPFDatabaseDescriptionPacket* ddPacket, Interface* intf, Neighbor* neighbor, bool inExchangeStart);

public:
    DatabaseDescriptionHandler(Router* containingRouter);

    void ProcessPacket(OSPFPacket* packet, Interface* intf, Neighbor* neighbor);
};

} // namespace AnsaOSPF

#endif // __INET_DATABASEDESCRIPTIONHANDLER_H

