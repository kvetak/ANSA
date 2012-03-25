#ifndef __INET_LINKSTATEUPDATEHANDLER_H
#define __INET_LINKSTATEUPDATEHANDLER_H

#include "AnsaIMessageHandler.h"
#include "AnsaOSPFcommon.h"

namespace AnsaOSPF {

class LinkStateUpdateHandler : public IMessageHandler
{
private:
    struct AcknowledgementFlags {
        bool floodedBackOut;
        bool lsaIsNewer;
        bool lsaIsDuplicate;
        bool impliedAcknowledgement;
        bool lsaReachedMaxAge;
        bool noLSAInstanceInDatabase;
        bool anyNeighborInExchangeOrLoadingState;
    };

private:
    bool ValidateLSChecksum(OSPFLSA* lsa) { return true; }   // not implemented
    void AcknowledgeLSA(OSPFLSAHeader& lsaHeader, Interface* intf, AcknowledgementFlags acknowledgementFlags, RouterID lsaSource);

public:
    LinkStateUpdateHandler(Router* containingRouter);

    void    ProcessPacket(OSPFPacket* packet, Interface* intf, Neighbor* neighbor);
};

} // namespace AnsaOSPF

#endif // __INET_LINKSTATEUPDATEHANDLER_H

