#ifndef __ANSA_OSPFV3INTERFACE_H_
#define __ANSA_OSPFV3INTERFACE_H_

#include <omnetpp.h>
#include <string>
//#include <cmodule.h>
#include "inet/common/INETDefs.h"
#include "ansa/routing/ospfv3/OSPFv3Common.h"
//#include "ansa/routing/ospfv3/interface/OSPFv3InterfaceState.h"
#include "ansa/routing/ospfv3/process/OSPFv3Process.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "ansa/routing/ospfv3/OSPFv3Packet_m.h"
#include "ansa/routing/ospfv3/process/OSPFv3Area.h"
#include "ansa/routing/ospfv3/neighbor/OSPFv3Neighbor.h"


namespace inet{

class OSPFv3Area;
class OSPFv3InterfaceState;
class OSPFv3Process;


class INET_API OSPFv3Interface : public cObject
{
  public:
    enum  OSPFv3InterfaceFAState{
        INTERFACE_STATE_DOWN = 0,
        INTERFACE_STATE_LOOPBACK = 1,
        INTERFACE_STATE_WAITING = 2,
        INTERFACE_STATE_POINTTOPOINT = 3,
        INTERFACE_STATE_DROTHER = 4,
        INTERFACE_STATE_BACKUP = 5,
        INTERFACE_STATE_DESIGNATED = 6,
        INTERFACE_PASSIVE = 7
    };

    enum OSPFv3InterfaceEvent{
        INTERFACE_UP_EVENT = 0,
        UNLOOP_IND_EVENT = 1,
        WAIT_TIMER_EVENT = 2,
        BACKUP_SEEN_EVENT = 3,
        NEIGHBOR_CHANGE_EVENT = 4,
        INTERFACE_DOWN_EVENT = 5,
        LOOP_IND_EVENT = 6,
        HELLO_TIMER_EVENT = 7,
        ACKNOWLEDGEMENT_TIMER_EVENT = 8
    };

    enum OSPFv3InterfaceType {
        UNKNOWN_TYPE = 0,
        POINTTOPOINT_TYPE = 1,
        BROADCAST_TYPE = 2,
        NBMA_TYPE = 3,
        POINTTOMULTIPOINT_TYPE = 4,
        VIRTUAL_TYPE = 5
    };

    const char* OSPFv3IntStateOutput[7] = {
            "Down",
            "Loopback",
            "Waiting",
            "Point-to-Point",
            "DROther",
            "Backup",
            "DR"
    };

    const char* OSPFv3IntTypeOutput[6] = {
            "UNKNOWN",
            "POINT-TO-POINT",
            "BROADCAST",
            "NBMA",
            "POINT-TO-MULTIPOINT",
            "VIRTUAL"
    };

    struct AcknowledgementFlags
    {
        bool floodedBackOut;
        bool lsaIsNewer;
        bool lsaIsDuplicate;
        bool impliedAcknowledgement;
        bool lsaReachedMaxAge;
        bool noLSAInstanceInDatabase;
        bool anyNeighborInExchangeOrLoadingState;
    };

    cModule* containingModule=nullptr;
    OSPFv3Process* containingProcess=nullptr;
    IInterfaceTable* ift = nullptr;

  public:
    OSPFv3Interface(const char* name, cModule* routerModule, OSPFv3Process* processModule, OSPFv3InterfaceType interfaceType, bool passive);
    virtual ~OSPFv3Interface();
    std::string getIntName() const {return this->interfaceName;}
    void reset();
    void processEvent(OSPFv3Interface::OSPFv3InterfaceEvent);
    void setRouterPriority(int newPriority){this->routerPriority = newPriority;}
    void setHelloInterval(int newInterval){this->helloInterval=newInterval;}
    void setDeadInterval(int newInterval){this->deadInterval=newInterval;}
    void setInterfaceCost(int newInterval){this->interfaceCost=newInterval;}
    void setInterfaceType(OSPFv3InterfaceType newType){this->interfaceType=newType;}
    void setAckDelay(int newAckDelay){this->ackDelay = newAckDelay;}
    void setDesignatedIP(IPv6Address newIP){this->DesignatedRouterIP = newIP;}
    void setBackupIP(IPv6Address newIP){this->BackupRouterIP = newIP;}
    void setDesignatedID(IPv4Address newID){this->DesignatedRouterID = newID;}
    void setBackupID(IPv4Address newID){this->BackupRouterID = newID;}
    void sendDelayedAcknowledgements();
    void setInterfaceIndex(int newIndex){this->interfaceIndex = newIndex;}
    void setTransitAreaID(IPv4Address areaId) { this->transitAreaID = areaId;}
    int getRouterPriority() const {return this->routerPriority;}
    short getHelloInterval() const {return this->helloInterval;}
    short getDeadInterval() const {return this->deadInterval;}
    short getPollInterval() const {return this->pollInterval;}
    short getTransDelayInterval() const {return this->transmissionDelay;}
    short getRetransmissionInterval() const {return this->retransmissionInterval;}
    short getAckDelay() const {return this->ackDelay;}
    int getInterfaceCost() const {return this->interfaceCost;}
    int getInterfaceId() const {return this->interfaceId;}
    int getInterfaceIndex() const {return this->interfaceIndex;}
    int getNeighborCount() const {return this->neighbors.size();}
    int getInterfaceMTU() const;
    int getInterfaceIndex(){return this->interfaceIndex;}
    IPv6Address getInterfaceIP() const {return this->interfaceIP;}
    bool isInterfacePassive(){return this->passiveInterface;}
    IPv4Address getTransitAreaID() const { return this->transitAreaID; }

    OSPFv3InterfaceType getType() const {return this->interfaceType;}
    OSPFv3Neighbor* getNeighbor(int i){return this->neighbors.at(i);}
    IPv6Address getDesignatedIP() const {return this->DesignatedRouterIP;}
    IPv6Address getBackupIP() const {return this->BackupRouterIP;}
    IPv4Address getDesignatedID() const {return this->DesignatedRouterID;}
    IPv4Address getBackupID() const {return this->BackupRouterID;}

    int calculateInterfaceCost();
    cMessage* getWaitTimer(){return this->waitTimer;}
    cMessage* getHelloTimer(){return this->helloTimer;}
    cMessage* getAcknowledgementTimer(){return this->acknowledgementTimer;}
    OSPFv3Area* getArea() const {return this->containingArea;};
    void setArea(OSPFv3Area* area){this->containingArea=area;};

    OSPFv3InterfaceState* getCurrentState(){return this->state;};
    OSPFv3InterfaceState* getPreviousState(){return this->previousState;};
    void changeState(OSPFv3InterfaceState* currentState, OSPFv3InterfaceState* newState);
    OSPFv3Interface::OSPFv3InterfaceFAState getState() const;

    void processHelloPacket(OSPFv3Packet* packet);
    void processDDPacket(OSPFv3Packet* packet);
    bool preProcessDDPacket(OSPFv3DatabaseDescription *ddPacket, OSPFv3Neighbor* neighbor, bool inExchangeStart);
    void processLSR(OSPFv3Packet* packet, OSPFv3Neighbor* neighbor);
    OSPFv3LSUpdate* prepareLSUHeader();
    OSPFv3LSUpdate* prepareUpdatePacket(OSPFv3LSA *lsa, OSPFv3LSUpdate* updatePacket);
    void processLSU(OSPFv3Packet* packet, OSPFv3Neighbor* neighbor);
    void processLSAck(OSPFv3Packet* packet);
    bool floodLSA(OSPFv3LSA* lsa, OSPFv3Interface* interface=nullptr, OSPFv3Neighbor* neighbor=nullptr);
    void removeFromAllRetransmissionLists(LSAKeyType lsaKey);

    OSPFv3HelloPacket* prepareHello();

    OSPFv3Neighbor* getNeighborById(IPv4Address neighborId);
    void addNeighbor(OSPFv3Neighbor*);

    std::string info() const override;
    std::string detailedInfo() const override;
    void acknowledgeLSA(OSPFv3LSAHeader* lsaHeader, AcknowledgementFlags ackFlags, IPv4Address routerID);

    OSPFv3LinkLSA* originateLinkLSA();
    void addLinkLSA(OSPFv3LinkLSA* newLSA){this->linkLSAList.push_back(newLSA);}
    int getLinkLSACount(){return this->linkLSAList.size();}
    OSPFv3LinkLSA* getLinkLSA(int i){return this->linkLSAList.at(i);}
    OSPFv3LinkLSA* getLinkLSAbyKey(LSAKeyType lsaKey);
    bool installLinkLSA(OSPFv3LinkLSA *lsa);
    bool updateLinkLSA(OSPFv3LinkLSA* currentLsa, OSPFv3LinkLSA* newLsa);
    bool linkLSADiffersFrom(OSPFv3LinkLSA* currentLsa, OSPFv3LinkLSA* newLsa);

    void sendLSAcknowledgement(OSPFv3LSAHeader *lsaHeader, IPv6Address destination);
    void addDelayedAcknowledgement(OSPFv3LSAHeader& lsaHeader);

    void setTransitNetInt(bool isTransit){this->transitNetworkInterface=isTransit;}
    bool getTransitNetInt(){return this->transitNetworkInterface;}


  private:
    friend class OSPFv3InterfaceState;

  private:
    IPv6Address interfaceIP;
    std::string interfaceName;
    bool passiveInterface;
    OSPFv3InterfaceState* state;
    OSPFv3InterfaceState* previousState=nullptr;
    int interfaceId;//physical id in the simulation
    int interfaceIndex;//unique value that appears in hello packet
    short helloInterval;
    short deadInterval;
    short pollInterval;
    short transmissionDelay;
    short retransmissionInterval;
    short ackDelay;
    int routerPriority;
    int interfaceCost;
    int mtu;
    OSPFv3Area* containingArea;
    IPv4Address transitAreaID;
    OSPFv3Interface::OSPFv3InterfaceType interfaceType;
    std::vector<OSPFv3Neighbor*> neighbors;
    std::map<IPv4Address, OSPFv3Neighbor*> neighborsById;
    std::map<IPv6Address, std::list<OSPFv3LSAHeader> > delayedAcknowledgements;

    //for Intra-Area-Prefix LSA
    bool transitNetworkInterface;

    std::vector<OSPFv3LinkLSA*> linkLSAList;
    uint32_t linkLSASequenceNumber = 1;

    IPv6Address DesignatedRouterIP;
    IPv6Address BackupRouterIP;
    IPv4Address DesignatedRouterID;
    IPv4Address BackupRouterID;

    cMessage *helloTimer;
    cMessage *waitTimer;
    cMessage *acknowledgementTimer;

    //TODO - E-bit
};

inline std::ostream& operator<<(std::ostream& ostr, const OSPFv3Interface& interface)
{
    ostr << interface.detailedInfo();
    return ostr;
}

}//namespace inet
#endif
