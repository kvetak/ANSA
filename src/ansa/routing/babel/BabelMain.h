//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
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
/**
* @file BabelMain.h
* @author Vit Rek (rek@kn.vutbr.cz)
* @author Vladimir Vesely (ivesely@fit.vutbr.cz)
* @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
* @brief Babel Main module header file
* @detail Represents Babel routing process
*/

#ifndef __ANSA_BABELMAIN_H_
#define __ANSA_BABELMAIN_H_

#include "inet/common/INETDefs.h"

#include "ansa/routing/babel/BabelDef.h"
#include "ansa/routing/babel/BabelInterfaceTable.h"
#include "ansa/routing/babel/BabelNeighbourTable.h"
#include "ansa/routing/babel/BabelTopologyTable.h"
#include "ansa/routing/babel/BabelSourceTable.h"
#include "ansa/routing/babel/BabelPenSRTable.h"
#include "ansa/routing/babel/BabelFtlv.h"
#include "ansa/routing/babel/BabelBuffer.h"
#include "ansa/routing/babel/BabelToAck.h"


//#include "InterfaceTable.h"
#include "inet/networklayer/common/InterfaceTable.h"
//#include "InterfaceEntry.h"
#include "inet/networklayer/common/InterfaceEntry.h"
//#include "NotificationBoard.h"
//#include "AnsaRoutingTable.h"
//#include "ANSARoutingTable6.h"
#include "inet/networklayer/ipv4/IPv4RoutingTable.h"
#include "inet/networklayer/ipv6/IPv6RoutingTable.h"

namespace inet {

class INET_API BabelMain : protected cListener, public cSimpleModule
//, public INotifiable
{
  protected:
    Babel::rid routerId;    ///< Router's identifier
    uint16_t seqno;         ///< Router's sequence number
    int port;               ///< UDP port number

    cModule *host = nullptr;    // the host module that owns this module
    InterfaceEntry *mainInterface;
    UDPSocket *socket4mcast;                ///< IPv4 socket for receive multicast
    UDPSocket *socket6mcast;                ///< IPv6 socket for receive multicast
    std::vector<Babel::BabelTimer *> timers;       ///< Pointers to all timers //TODO - uz nepouzivane - SMAZAT
    std::vector<BabelBuffer *> buffers;
    Babel::BabelTimer *buffgc;
    std::vector<BabelToAck *> ackwait;

  public:
    IInterfaceTable *ift;
    //NotificationBoard *nb;
    //AnsaRoutingTable *rt4;
    //ANSARoutingTable6 *rt6;
    IPv4RoutingTable *rt4 = nullptr;
    IPv6RoutingTable *rt6 = nullptr;

    BabelInterfaceTable bit;
    BabelNeighbourTable bnt;
    BabelTopologyTable btt;
    BabelSourceTable bst;
    BabelPenSRTable bpsrt;

    int getIntuniform(int a, int b);
    double getUniform(double a, double b);
    //cMersenneTwister* mt = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    //virtual void receiveChangeNotification(int category, const cObject *details);
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG) override;

  public:
    BabelMain() :   seqno(0),
                    port(Babel::defval::PORT),
                    mainInterface(NULL),
                    socket4mcast(NULL),
                    socket6mcast(NULL),
                    buffgc(NULL),
                    ift(NULL)
                    {};
    virtual ~BabelMain();

    void setRouterId(uint32_t h, uint32_t l);
    Babel::rid getRouterId() {return routerId;}

    void setPort(int p) {port = p;}
    int getPort() {return port;}

    void incSeqno();

    void setMainInterface();
    void generateRouterId();
    Babel::BabelTimer *createTimer(short kind, void *context=NULL, const char *suffix=NULL, bool autodelete=true);
    void deleteTimer(Babel::BabelTimer *todel);
    void deleteTimers();

    void processTimer(Babel::BabelTimer *timer);
    void processNeighHelloTimer(BabelNeighbour *neigh);
    void processNeighIhuTimer(BabelNeighbour *neigh);
    void processRouteExpiryTimer(BabelRoute *route);
    void processBefRouteExpiryTimer(BabelRoute *route);
    void processSourceGCTimer(BabelSource *source);
    void processRSResendTimer(BabelPenSR *request);

    UDPSocket *createSocket();

    void activateInterface(BabelInterface *iface);
    void deactivateInterface(BabelInterface *iface);


    void processMessage(BabelMessage *msg);
    void processAckReqTlv(char *tlv, BabelInterface *iniface, const L3Address& src);
    void processAckTlv(char *tlv, const L3Address& src);
    void processHelloTlv(char *tlv, BabelInterface *iniface, const L3Address& src);
    bool processIhuTlv(char *tlv, BabelInterface *iniface, const L3Address& src, const L3Address& dst);
    bool processUpdateTlv(char *tlv, BabelInterface *iniface, const L3Address& src, const Babel::rid& originator, const L3Address& nh, Babel::netPrefix<L3Address> *prevprefix);
    void processRouteReqTlv(char *tlv, BabelInterface *iniface, const L3Address& src, const L3Address& dst);
    void processSeqnoReqTlv(char *tlv, BabelInterface *iniface, const L3Address& src);

    void sendMessage(L3Address dst, BabelInterface *outIface, BabelMessage *msg);
    void sendTLV(L3Address da, BabelInterface *oi, BabelFtlv *ftlv, double mt=Babel::SEND_BUFFERED);
    void sendTLV(BabelInterface *oi, BabelFtlv *ftlv, double mt=Babel::SEND_BUFFERED);
    void sendHelloTLV(BabelInterface *iface, double mt=Babel::SEND_BUFFERED);
    void sendUpdateTLV(L3Address da, BabelInterface *iface, BabelUpdateFtlv *update, double mt=Babel::SEND_BUFFERED, bool reliably=false);
    void sendUpdateTLV(BabelInterface *iface, BabelUpdateFtlv *update, double mt=Babel::SEND_BUFFERED, bool reliably=false);
    void sendSeqnoReqTLV(L3Address da, BabelInterface *iface, BabelSeqnoReqFtlv *request, BabelNeighbour *recfrom=NULL, double mt=Babel::SEND_URGENT);
    void sendSeqnoReqTLV(BabelInterface *iface, BabelSeqnoReqFtlv *request, BabelNeighbour *recfrom=NULL, double mt=Babel::SEND_URGENT);

    void sendUpdate(L3Address da, BabelInterface *iface, BabelRoute *route, double mt=Babel::SEND_BUFFERED, bool reliably=false);
    void sendUpdate(BabelInterface *iface, BabelRoute *route, double mt=Babel::SEND_BUFFERED, bool reliably=false);
    void sendFullDump(BabelInterface *iface);

    BabelBuffer* findOrCreateBuffer(L3Address da, BabelInterface *oi);
    BabelBuffer* findBuffer(L3Address da, BabelInterface *oi);
    void deleteBuffer(BabelBuffer *todel);
    void deleteUnusedBuffers();
    void deleteBuffers();
    void flushBuffer(BabelBuffer *buff);
    void flushAllBuffers();

    bool addOrUpdateRoute(const Babel::netPrefix<L3Address>& prefix, BabelNeighbour *neigh, const Babel::rid& orig, const Babel::routeDistance& dist, const L3Address& nh, uint16_t interval);
    void addOrupdateSource(const Babel::netPrefix<L3Address>& p, const Babel::rid& orig, const Babel::routeDistance& dist);
    bool isFeasible(const Babel::netPrefix<L3Address>& prefix, const Babel::rid& orig, const Babel::routeDistance& dist);
    void selectRoutes();

    bool prepareToAdd(IRoutingTable* rt, IRoute* ro);
    void addToRT(BabelRoute *route);
    void removeFromRT(BabelRoute *route);
    void updateRT(BabelRoute *route);

    BabelToAck *addToAck(BabelToAck *toadd);
    void deleteToAck(BabelToAck *todel);
    void deleteToAcks();
    BabelToAck *findToAck(uint16_t n);
    void checkAndResendToAck(BabelToAck *toack);
    uint16_t generateNonce();


    bool removeNeighboursOnIface(BabelInterface *iface);
    bool removeRoutesByNeigh(BabelNeighbour *neigh);
/*
    const double roughly(const double value)
    {
        double variance=0.25;
        ASSERT(variance > 0.0 && variance <= 1.0);
        return value * uniform(1.0 - variance, 1.0 + variance);
    }
    */
    template <typename T>
    inline T roughly(T value, double variance=0.25)
    {
        ASSERT(variance > 0.0 && variance <= 1.0);

        return value * getUniform(1.0 - variance, 1.0 + variance);
    }
};
}
#endif
