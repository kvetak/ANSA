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
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
* @brief Babel Main module header file
* @detail Represents Babel routing process
*/

#ifndef __ANSA_BABELMAIN_H_
#define __ANSA_BABELMAIN_H_

#include <omnetpp.h>

#include "ansa/applications/babel/BabelDef.h"
#include "ansa/applications/babel/BabelInterfaceTable.h"
#include "ansa/applications/babel/BabelNeighbourTable.h"
#include "ansa/applications/babel/BabelTopologyTable.h"
#include "ansa/applications/babel/BabelSourceTable.h"
#include "ansa/applications/babel/BabelPenSRTable.h"
#include "ansa/applications/babel/BabelFtlv.h"
#include "ansa/applications/babel/BabelBuffer.h"
#include "ansa/applications/babel/BabelToAck.h"


#include "networklayer/common/InterfaceTable.h"
#include "networklayer/common/InterfaceEntry.h"
#include "base/NotificationBoard.h"
#include "ansa/networklayer/ipv4/AnsaRoutingTable.h"
#include "ansa/networklayer/ipv6/ANSARoutingTable6.h"


/**
 * TODO - Generated class
 */
class BabelMain : public cSimpleModule, public INotifiable
{
  protected:
    Babel::rid routerId;    ///< Router's identifier
    uint16_t seqno;         ///< Router's sequence number
    int port;               ///< UDP port number


    inet::InterfaceEntry *mainInterface;
    inet::UDPSocket *socket4mcast;                ///< IPv4 socket for receive multicast
    inet::UDPSocket *socket6mcast;                ///< IPv6 socket for receive multicast
    std::vector<Babel::BabelTimer *> timers;       ///< Pointers to all timers //TODO - uz nepouzivane - SMAZAT
    std::vector<BabelBuffer *> buffers;
    Babel::BabelTimer *buffgc;
    std::vector<BabelToAck *> ackwait;

  public:
    inet::IInterfaceTable *ift;
    NotificationBoard *nb;
    AnsaRoutingTable *rt4;
    ANSARoutingTable6 *rt6;

    BabelInterfaceTable bit;
    BabelNeighbourTable bnt;
    BabelTopologyTable btt;
    BabelSourceTable bst;
    BabelPenSRTable bpsrt;



  protected:
    virtual int numInitStages() const {return 4;}
    virtual void receiveChangeNotification(int category, const cObject *details);
    virtual void initialize(int stage);
    virtual void finish();
    virtual void handleMessage(cMessage *msg);

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

    inet::UDPSocket *createSocket();

    void activateInterface(BabelInterface *iface);
    void deactivateInterface(BabelInterface *iface);


    void processMessage(BabelMessage *msg);
    void processAckReqTlv(char *tlv, BabelInterface *iniface, const inet::L3Address& src);
    void processAckTlv(char *tlv, const inet::L3Address& src);
    void processHelloTlv(char *tlv, BabelInterface *iniface, const inet::L3Address& src);
    bool processIhuTlv(char *tlv, BabelInterface *iniface, const inet::L3Address& src, const inet::L3Address& dst);
    bool processUpdateTlv(char *tlv, BabelInterface *iniface, const inet::L3Address& src, const Babel::rid& originator, const inet::L3Address& nh, Babel::netPrefix<inet::L3Address> *prevprefix);
    void processRouteReqTlv(char *tlv, BabelInterface *iniface, const inet::L3Address& src, const inet::L3Address& dst);
    void processSeqnoReqTlv(char *tlv, BabelInterface *iniface, const inet::L3Address& src);

    void sendMessage(inet::L3Address dst, BabelInterface *outIface, BabelMessage *msg);
    void sendTLV(inet::L3Address da, BabelInterface *oi, BabelFtlv *ftlv, double mt=Babel::SEND_BUFFERED);
    void sendTLV(BabelInterface *oi, BabelFtlv *ftlv, double mt=Babel::SEND_BUFFERED);
    void sendHelloTLV(BabelInterface *iface, double mt=Babel::SEND_BUFFERED);
    void sendUpdateTLV(inet::L3Address da, BabelInterface *iface, BabelUpdateFtlv *update, double mt=Babel::SEND_BUFFERED, bool reliably=false);
    void sendUpdateTLV(BabelInterface *iface, BabelUpdateFtlv *update, double mt=Babel::SEND_BUFFERED, bool reliably=false);
    void sendSeqnoReqTLV(inet::L3Address da, BabelInterface *iface, BabelSeqnoReqFtlv *request, BabelNeighbour *recfrom=NULL, double mt=Babel::SEND_URGENT);
    void sendSeqnoReqTLV(BabelInterface *iface, BabelSeqnoReqFtlv *request, BabelNeighbour *recfrom=NULL, double mt=Babel::SEND_URGENT);

    void sendUpdate(inet::L3Address da, BabelInterface *iface, BabelRoute *route, double mt=Babel::SEND_BUFFERED, bool reliably=false);
    void sendUpdate(BabelInterface *iface, BabelRoute *route, double mt=Babel::SEND_BUFFERED, bool reliably=false);
    void sendFullDump(BabelInterface *iface);

    BabelBuffer* findOrCreateBuffer(inet::L3Address da, BabelInterface *oi);
    BabelBuffer* findBuffer(inet::L3Address da, BabelInterface *oi);
    void deleteBuffer(BabelBuffer *todel);
    void deleteUnusedBuffers();
    void deleteBuffers();
    void flushBuffer(BabelBuffer *buff);
    void flushAllBuffers();

    bool addOrUpdateRoute(const Babel::netPrefix<inet::L3Address>& prefix, BabelNeighbour *neigh, const Babel::rid& orig, const Babel::routeDistance& dist, const inet::L3Address& nh, uint16_t interval);
    void addOrupdateSource(const Babel::netPrefix<inet::L3Address>& p, const Babel::rid& orig, const Babel::routeDistance& dist);
    bool isFeasible(const Babel::netPrefix<inet::L3Address>& prefix, const Babel::rid& orig, const Babel::routeDistance& dist);
    void selectRoutes();

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
};

#endif
