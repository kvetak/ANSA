#ifndef __ANSA_OSPFV3NEIGHBOR_H_
#define __ANSA_OSPFV3NEIGHBOR_H_

#include <omnetpp.h>
#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "ansa/routing/ospfv3/OSPFv3Packet_m.h"
#include "ansa/routing/ospfv3/OSPFv3Common.h"
#include "ansa/routing/ospfv3/OSPFv3Timers.h"
//#include "ansa/routing/ospfv3/interface/OSPFv3Interface.h"

namespace inet{

class OSPFv3NeighborState;
class OSPFv3Interface;
//struct OSPFv3DDPacketID;

class OSPFv3Neighbor{
  public:
    enum OSPFv3NeighborEventType {
        HELLO_RECEIVED = 0,
        START = 1,
        TWOWAY_RECEIVED = 2,
        NEGOTIATION_DONE = 3,
        EXCHANGE_DONE = 4,
        BAD_LINK_STATE_REQUEST = 5,
        LOADING_DONE = 6,
        IS_ADJACENCY_OK = 7,
        SEQUENCE_NUMBER_MISMATCH = 8,
        ONEWAY_RECEIVED = 9,
        KILL_NEIGHBOR = 10,
        INACTIVITY_TIMER = 11,
        POLL_TIMER = 12,
        LINK_DOWN = 13,
        DD_RETRANSMISSION_TIMER = 14,
        UPDATE_RETRANSMISSION_TIMER = 15,
        REQUEST_RETRANSMISSION_TIMER = 16
    };

    enum OSPFv3NeighborStateType {
        DOWN_STATE = 0,
        ATTEMPT_STATE = 1,
        INIT_STATE = 2,
        TWOWAY_STATE = 4,
        EXCHANGE_START_STATE = 8,
        EXCHANGE_STATE = 16,
        LOADING_STATE = 32,
        FULL_STATE = 64
    };

    enum OSPFv3DatabaseExchangeRelationshipType {
        MASTER = 0,
        SLAVE = 1
    };

    struct TransmittedLSA{
        LSAKeyType lsaKey;
        int age;
    };

    OSPFv3Neighbor(IPv4Address newId, OSPFv3Interface* parent);
    virtual ~OSPFv3Neighbor();
    void setNeighborPriority(int newPriority){this->neighborRtrPriority=newPriority;}
    void setNeighborDeadInterval(int newInterval){this->neighborsRouterDeadInterval=newInterval;}
    void setNeighborID(IPv4Address newId){this->neighborId=newId;}
    void setNeighborAddress(IPv6Address newAddress){this->neighborIPAddress=newAddress;}
    void setDesignatedRouterID(IPv4Address newId){this->neighborsDesignatedRouter=newId;}
    void setBackupDesignatedRouterID(IPv4Address newId){this->neighborsBackupDesignatedRouter=newId;}
    void setOptions(OSPFv3Options options) { this->neighborOptions = options; }
    void setDatabaseExchangeRelationship(OSPFv3DatabaseExchangeRelationshipType newRelationship){this->databaseExchangeRelationship = newRelationship;}
    void setDDSequenceNumber(unsigned long newSequenceNmr){this->ddSequenceNumber = newSequenceNmr;}
    void setupDesignatedRouters(bool setUp) { this->designatedRoutersSetUp = setUp; }
    void setNeighborInterfaceID(uint32_t newID){this->neighborInterfaceID = newID;}
    bool designatedRoutersAreSetUp() const { return designatedRoutersSetUp; }

    OSPFv3Interface* getInterface(){return this->containingInterface;}
    IPv4Address getNeighborID(){return this->neighborId;}
    IPv4Address getNeighborsDR(){return this->neighborsDesignatedRouter;}
    IPv4Address getNeighborsBackup(){return this->neighborsBackupDesignatedRouter;}
    IPv6Address getNeighborIP(){return this->neighborIPAddress;}
    IPv6Address getNeighborBackupIP(){return this->neighborsBackupIP;};
    IPv6Address getNeighborDesignatedIP(){return this->neighborsDesignatedIP;}
    uint32_t getNeighborInterfaceID(){return this->neighborInterfaceID;}
    OSPFv3Options getOptions() const { return this->neighborOptions; }
    OSPFv3DatabaseExchangeRelationshipType getDatabaseExchangeRelationship(){return this->databaseExchangeRelationship;}
    unsigned long getDDSequenceNumber(){return this->ddSequenceNumber;}
    OSPFv3NeighborStateType getState() const;
    unsigned short getNeighborPriority(){return this->neighborRtrPriority;}
    unsigned short getNeighborDeadInterval(){return this->neighborsRouterDeadInterval;}
    void processEvent(OSPFv3Neighbor::OSPFv3NeighborEventType event);
    void changeState(OSPFv3NeighborState *newState, OSPFv3NeighborState *currentState);
    void reset();

    cMessage *getInactivityTimer() { return this->inactivityTimer; }
    cMessage *getPollTimer() { return this->pollTimer; }
    cMessage *getDDRetransmissionTimer() { return this->ddRetransmissionTimer; }
    cMessage *getUpdateRetransmissionTimer() { return this->updateRetransmissionTimer; }
    cMessage *getRequestRetransmissionTimer() { return this->requestRetransmissionTimer; }

    bool needAdjacency();
    bool isFirstAdjacencyInited() const { return this->firstAdjacencyInited; }
    void initFirstAdjacency();
    unsigned long getUniqueULong();
    void incrementDDSequenceNumber(){this->ddSequenceNumber++;}
    void startUpdateRetransmissionTimer();
    void clearUpdateRetransmissionTimer();
    void startRequestRetransmissionTimer();
    void clearRequestRetransmissionTimer();

    void sendDDPacket(bool init = false);
    void sendLinkStateRequestPacket();

    void setLastReceivedDDPacket(OSPFv3DatabaseDescription* ddPacket);
    OSPFv3DDPacketID getLastReceivedDDPacket(){return this->lastReceivedDDPacket;}
    bool isLinkStateRequestListEmpty(){return this->linkStateRequestList.empty();}
    bool isRequestRetransmissionTimerActive(){return this->requestRetransmissionTimerActive;}
    bool isUpdateRetransmissionTimerActive() const { return this->updateRetransmissionTimerActive; }
    unsigned long getDatabaseSummaryListCount() const { return databaseSummaryList.size();}
    void createDatabaseSummary();
    void deleteLastSentDDPacket();
    void retransmitUpdatePacket();
    void addToRetransmissionList(OSPFv3LSA *lsa);
    void addToRequestList(OSPFv3LSAHeader *lsaHeader);
    bool retransmitDatabaseDescriptionPacket();
    bool isLSAOnRequestList(LSAKeyType lsaKey);
    OSPFv3LSAHeader *findOnRequestList(LSAKeyType lsaKey);
    void removeFromRequestList(LSAKeyType lsaKey);
    void addToTransmittedLSAList(LSAKeyType lsaKey);
    bool isOnTransmittedLSAList(LSAKeyType lsaKey) const;
    void removeFromRetransmissionList(LSAKeyType lsaKey);


  private:
    OSPFv3NeighborState* state=nullptr;
    OSPFv3NeighborState* previousState=nullptr;
    cMessage *inactivityTimer = nullptr;
    cMessage *pollTimer = nullptr;
    cMessage *ddRetransmissionTimer = nullptr;
    cMessage *updateRetransmissionTimer = nullptr;
    bool updateRetransmissionTimerActive = false;
    cMessage *requestRetransmissionTimer = nullptr;
    bool requestRetransmissionTimerActive = false;
    OSPFv3DatabaseExchangeRelationshipType databaseExchangeRelationship = (OSPFv3DatabaseExchangeRelationshipType)-1;
    bool firstAdjacencyInited = false;
    unsigned long ddSequenceNumber;//TODO - what is the initial number?
    OSPFv3DDPacketID lastReceivedDDPacket;
    unsigned short neighborRtrPriority;//neighbor priority
    IPv4Address neighborId;

    IPv6Address neighborIPAddress;
    OSPFv3Options neighborOptions;//options supported by the neighbor
    IPv4Address neighborsDesignatedRouter;//DR advertised by the neighbor
    IPv4Address neighborsBackupDesignatedRouter;///Backup advertised by the router
    IPv6Address neighborsBackupIP;
    IPv6Address neighborsDesignatedIP;
    uint32_t neighborInterfaceID;
    bool designatedRoutersSetUp = false;
    short neighborsRouterDeadInterval = 0;
    std::list<OSPFv3LSA*> linkStateRetransmissionList;
    std::list<OSPFv3LSAHeader *> databaseSummaryList;//database summary list - complete list of LSAs that make up the area link-state database - the neighbor goes into Database Exchange state
    std::list<OSPFv3LSAHeader *> linkStateRequestList;//link state request list - list of LSAs that need to be received from the neighbor
    std::list<TransmittedLSA > transmittedLSAs;//link state retransmission list - LSA were flooded but not acknowledged, these are retransmitted until acknowledged or until the adjacency ends
    OSPFv3DatabaseDescription *lastTransmittedDDPacket = nullptr;

    OSPFv3Interface* containingInterface;
    static unsigned long ddSequenceNumberInitSeed;



};

}//namespace inet
#endif
