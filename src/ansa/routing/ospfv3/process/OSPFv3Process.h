#ifndef __ANSA_OSPFV3PROCESS_H_
#define __ANSA_OSPFV3PROCESS_H_

#include <omnetpp.h>
#include <string>
#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/contract/ipv6/IPv6ControlInfo.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv6/IPv6InterfaceData.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/networklayer/ipv6/IPv6RoutingTable.h"

#include "ansa/routing/ospfv3/process/OSPFv3Instance.h"
#include "ansa/routing/ospfv3/process/OSPFv3Area.h"
#include "ansa/routing/ospfv3/OSPFv3Timers.h"
#include "ansa/routing/ospfv3/process/OSPFv3RoutingTableEntry.h"
#include "ansa/routing/ospfv3/OSPFv3Common.h"
#include "ansa/routing/ospfv3/OSPFv3Packet_m.h"

#include "inet/networklayer/ipv6/IPv6Route.h"


namespace inet{

class OSPFv3Instance;

class INET_API OSPFv3Process : protected cListener, public cSimpleModule
{
  public:
    OSPFv3Process();
    virtual ~OSPFv3Process();
    int getProcessID(){return this->processID;};
    IPv4Address getRouterID(){return this->routerID;};
    bool isActivated(){return this->isActive;};
    void activateProcess();
    void setTimer(cMessage* msg, double delay);
    void clearTimer(cMessage* msg){this->cancelEvent(msg);}
    OSPFv3Instance* getInstanceById(int instanceId);
    void addInstance(OSPFv3Instance* newInstance);
    void sendPacket(OSPFv3Packet *packet, IPv6Address destination, const char* ifName, short hopLimit = 1);
    OSPFv3LSA* findLSA(LSAKeyType lsaKey, IPv4Address areaID, int instanceID);
    bool floodLSA(OSPFv3LSA* lsa, IPv4Address areaID=IPv4Address::UNSPECIFIED_ADDRESS, OSPFv3Interface* intf=nullptr, OSPFv3Neighbor* neighbor=nullptr);
    bool installLSA(OSPFv3LSA *lsa, int instanceID, IPv4Address areaID=IPv4Address::UNSPECIFIED_ADDRESS, OSPFv3Interface* intf=nullptr);
    void rebuildRoutingTable();
    void calculateASExternalRoutes(std::vector<OSPFv3RoutingTableEntry* > newTable);

  public:
    IInterfaceTable* ift = nullptr;
    IPv6RoutingTable *rt = nullptr;
    IIPv4RoutingTable *rt4 = nullptr;
    cModule* containingModule=nullptr;


  private:
    void handleTimer(cMessage* msg);


  private:
    std::vector<OSPFv3Instance*> instances;
    std::map<int, OSPFv3Instance*> instancesById;
    int processID;
    IPv4Address routerID;
    bool isActive=false;
    void debugDump();
    std::vector<OSPFv3RoutingTableEntry *> routingTable;


  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage* msg) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    void parseConfig(cXMLElement* areaConfig);


    //backbone area structure
    //virtual links
    //list of external routes
    //list of as-external routes
    //routing table
};

}//namespace inet

#endif
