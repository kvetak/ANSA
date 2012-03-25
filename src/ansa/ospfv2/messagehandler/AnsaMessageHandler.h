#ifndef __INET_MESSAGEHANDLER_H
#define __INET_MESSAGEHANDLER_H

#include "AnsaIMessageHandler.h"
#include "AnsaHelloHandler.h"
#include "AnsaDatabaseDescriptionHandler.h"
#include "AnsaLinkStateRequestHandler.h"
#include "AnsaLinkStateUpdateHandler.h"
#include "AnsaLinkStateAcknowledgementHandler.h"
#include "OSPFTimer_m.h"
#include "IPControlInfo.h"
#include "AnsaOSPFInterface.h"
//#include "OSPFNeighbor.h"

namespace AnsaOSPF {

class MessageHandler : public IMessageHandler {
private:
    cSimpleModule*                  ospfModule;

    HelloHandler                    helloHandler;
    DatabaseDescriptionHandler      ddHandler;
    LinkStateRequestHandler         lsRequestHandler;
    LinkStateUpdateHandler          lsUpdateHandler;
    LinkStateAcknowledgementHandler lsAckHandler;

public:
    MessageHandler  (Router* containingRouter, cSimpleModule* containingModule);

    void    MessageReceived(cMessage* message);
    void    HandleTimer     (OSPFTimer* timer);

    void    ProcessPacket   (OSPFPacket* packet, Interface* unused1 = NULL, Neighbor* unused2 = NULL);

    void    SendPacket      (OSPFPacket* packet, IPv4Address destination, int outputIfIndex, short ttl = 1);
    void    ClearTimer      (OSPFTimer* timer);
    void    StartTimer      (OSPFTimer* timer, simtime_t delay);

    void    PrintEvent                          (const char* eventString, const Interface* onInterface = NULL, const Neighbor* forNeighbor = NULL) const;
    void    PrintHelloPacket                    (const OSPFHelloPacket* helloPacket, IPv4Address destination, int outputIfIndex) const;
    void    PrintDatabaseDescriptionPacket      (const OSPFDatabaseDescriptionPacket* ddPacket, IPv4Address destination, int outputIfIndex) const;
    void    PrintLinkStateRequestPacket         (const OSPFLinkStateRequestPacket* requestPacket, IPv4Address destination, int outputIfIndex) const;
    void    PrintLinkStateUpdatePacket          (const OSPFLinkStateUpdatePacket* updatePacket, IPv4Address destination, int outputIfIndex) const;
    void    PrintLinkStateAcknowledgementPacket(const OSPFLinkStateAcknowledgementPacket* ackPacket, IPv4Address destination, int outputIfIndex) const;

    // Authentication not implemented
    bool    AuthenticatePacket  (OSPFPacket* packet)    { return true; }
};

} // namespace AnsaOSPF

#endif // __INET_MESSAGEHANDLER_H

