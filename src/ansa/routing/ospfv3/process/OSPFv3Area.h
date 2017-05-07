#ifndef __ANSA_OSPFV3AREA_H_
#define __ANSA_OSPFV3AREA_H_

#include <omnetpp.h>
#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "ansa/routing/ospfv3/interface/OSPFv3Interface.h"
#include "ansa/routing/ospfv3/OSPFv3Packet_m.h"
#include "ansa/routing/ospfv3/process/OSPFv3LSA.h"
#include "ansa/routing/ospfv3/process/OSPFv3RoutingTableEntry.h"
#include "ansa/routing/ospfv3/neighbor/OSPFv3Neighbor.h"


namespace inet{

class OSPFv3Instance;
class OSPFv3Interface;
class OSPFv3Process;

enum OSPFv3AreaType {
    NORMAL = 0,
    STUB,
    TOTALLY_STUBBY,
    NSSA,
    NSSA_TOTALLY_STUB
};

class INET_API OSPFv3Area : public cObject
{
  public:
    OSPFv3Area(IPv4Address areaID, OSPFv3Instance* containingInstance, OSPFv3AreaType type);
    virtual ~OSPFv3Area();
    IPv4Address getAreaID() const {return this->areaID;}
    OSPFv3AreaType getAreaType() const {return this->areaType;}
    bool hasInterface(std::string);
    void addInterface(OSPFv3Interface*);
    void init();
    void debugDump();
    int getInstanceType(){return this->instanceType;};
    void setiInstanceType(int type){this->instanceType = type;};
    void setExternalRoutingCapability(bool capable){this->externalRoutingCapability=capable;}
    void setStubDefaultCost(int newCost){this->stubDefaultCost=newCost;}
    void setTransitCapability(bool capable){this->transitCapability=capable;}
    OSPFv3Interface* getInterfaceById(int id);
    OSPFv3Interface* getInterfaceByIndex(int id);
    OSPFv3Instance* getInstance() const {return this->containingInstance;};
    bool getExternalRoutingCapability(){return this->externalRoutingCapability;}
    int getStubDefaultCost(){return this->stubDefaultCost;}
    bool getTransitCapability(){return this->transitCapability;}
    OSPFv3Interface* findVirtualLink(IPv4Address routerID);

    OSPFv3Interface* getInterface(int i) const {return this->interfaceList.at(i);}
    int getInterfaceCount() const {return this->interfaceList.size();}

    OSPFv3LSA* getLSAbyKey(LSAKeyType lsaKey);

    void deleteRouterLSA(int index);
    void addRouterLSA(OSPFv3RouterLSA* newLSA){this->routerLSAList.push_back(newLSA);}
    OSPFv3RouterLSA* originateRouterLSA();//this originates one router LSA for one area
    int getRouterLSACount(){return this->routerLSAList.size();}
    OSPFv3RouterLSA* getRouterLSA(int i){return this->routerLSAList.at(i);}
    OSPFv3RouterLSA* getRouterLSAbyKey(LSAKeyType lsaKey);
    bool installRouterLSA(OSPFv3RouterLSA *lsa);
    bool updateRouterLSA(OSPFv3RouterLSA* currentLsa, OSPFv3RouterLSA* newLsa);
    bool routerLSADiffersFrom(OSPFv3RouterLSA* currentLsa, OSPFv3RouterLSA* newLsa);
    IPv4Address getNewRouterLinkStateID();
    IPv4Address getRouterLinkStateID(){return this->routerLsID;}
    uint32_t getCurrentRouterSequence(){return this->routerLSASequenceNumber;}
    void incrementRouterSequence(){this->routerLSASequenceNumber++;}

    void addNetworkLSA(OSPFv3NetworkLSA* newLSA){this->networkLSAList.push_back(newLSA);}
    OSPFv3NetworkLSA* originateNetworkLSA(OSPFv3Interface* interface);//this originates one router LSA for one area
    int getNetworkLSACount(){return this->networkLSAList.size();}
    OSPFv3NetworkLSA* getNetworkLSA(int i){return this->networkLSAList.at(i);}
    bool installNetworkLSA(OSPFv3NetworkLSA *lsa);
    bool updateNetworkLSA(OSPFv3NetworkLSA* currentLsa, OSPFv3NetworkLSA* newLsa);
    bool networkLSADiffersFrom(OSPFv3NetworkLSA* currentLsa, OSPFv3NetworkLSA* newLsa);
    IPv4Address getNewNetworkLinkStateID();
    IPv4Address getNetworkLinkStateID(){return this->networkLsID;}
    uint32_t getCurrentNetworkSequence(){return this->networkLSASequenceNumber;}
    void incrementNetworkSequence(){this->networkLSASequenceNumber++;}

    /* INTER AREA PREFIX LSA */
    void addInterAreaPrefixLSA(OSPFv3InterAreaPrefixLSA* newLSA){this->interAreaPrefixLSAList.push_back(newLSA);};
    void originateInterAreaPrefixLSA(OSPFv3IntraAreaPrefixLSA* lsa, OSPFv3Area* fromArea);
    int getInterAreaPrefixLSACount(){return this->interAreaPrefixLSAList.size();}
    OSPFv3InterAreaPrefixLSA* getInterAreaPrefixLSA(int i){return this->interAreaPrefixLSAList.at(i);}
    bool installInterAreaPrefixLSA(OSPFv3InterAreaPrefixLSA* lsa);
    bool updateInterAreaPrefixLSA(OSPFv3InterAreaPrefixLSA* currentLsa, OSPFv3InterAreaPrefixLSA* newLsa);
    bool interAreaPrefixLSADiffersFrom(OSPFv3InterAreaPrefixLSA* currentLsa, OSPFv3InterAreaPrefixLSA* newLsa);
    IPv4Address getNewInterAreaPrefixLinkStateID();
    IPv4Address getInterAreaPrefixLinkStateID(){return this->interAreaPrefixLsID;}
    uint32_t getCurrentInterAreaPrefixSequence(){return this->interAreaPrefixLSASequenceNumber;}
    void incrementInterAreaPrefixSequence(){this->interAreaPrefixLSASequenceNumber++;}

    void originateDefaultInterAreaPrefixLSA(OSPFv3Area* toArea);
    void originateInterAreaPrefixLSA(OSPFv3LSA* prefLsa, OSPFv3Area* fromArea);

    //* INTRA AREA PREFIX LSA */
    void addIntraAreaPrefixLSA(OSPFv3IntraAreaPrefixLSA* newLSA){this->intraAreaPrefixLSAList.push_back(newLSA);}
    OSPFv3IntraAreaPrefixLSA* originateIntraAreaPrefixLSA();//this originates one router LSA for one area
    int getIntraAreaPrefixLSACount(){return this->intraAreaPrefixLSAList.size();}
    OSPFv3IntraAreaPrefixLSA* getIntraAreaPrefixLSA(int i){return this->intraAreaPrefixLSAList.at(i);}
    bool installIntraAreaPrefixLSA(OSPFv3IntraAreaPrefixLSA *lsa);
    bool updateIntraAreaPrefixLSA(OSPFv3IntraAreaPrefixLSA* currentLsa, OSPFv3IntraAreaPrefixLSA* newLsa);
    bool intraAreaPrefixLSADiffersFrom(OSPFv3IntraAreaPrefixLSA* currentLsa, OSPFv3IntraAreaPrefixLSA* newLsa);
    IPv4Address getNewIntraAreaPrefixLinkStateID();
    IPv4Address getIntraAreaPrefixLinkStateID(){return this->intraAreaPrefixLsID;}
    uint32_t getCurrentIntraAreaPrefixSequence(){return this->intraAreaPrefixLSASequenceNumber;}
    void incrementIntraAreaPrefixSequence(){this->intraAreaPrefixLSASequenceNumber++;}

    OSPFv3IntraAreaPrefixLSA* originateNetIntraAreaPrefixLSA(OSPFv3NetworkLSA* networkLSA, OSPFv3Interface* interface);//this originates one router LSA for one area
    IPv4Address getNewNetIntraAreaPrefixLinkStateID();
    IPv4Address getNetIntraAreaPrefixLinkStateID(){return this->netIntraAreaPrefixLsID;}
    uint32_t getCurrentNetIntraAreaPrefixSequence(){return this->netIntraAreaPrefixLSASequenceNumber;}
    void incrementNetIntraAreaPrefixSequence(){this->netIntraAreaPrefixLSASequenceNumber++;}

    OSPFv3LSAHeader* findLSA(LSAKeyType lsaKey);
    bool floodLSA(OSPFv3LSA* lsa, OSPFv3Interface* interface=nullptr, OSPFv3Neighbor* neighbor=nullptr);

    void removeFromAllRetransmissionLists(LSAKeyType lsaKey);


    void calculateShortestPathTree(std::vector<OSPFv3RoutingTableEntry* > newTable);
    void calculateInterAreaRoutes(std::vector<OSPFv3RoutingTableEntry* > newTable);
    void recheckSummaryLSAs(std::vector<OSPFv3RoutingTableEntry* > newTable);
    bool hasLink(OSPFv3LSA *fromLSA, OSPFv3LSA *toLSA) const;
    std::vector<NextHop> *calculateNextHops(OSPFv3SPFVertex* destination, OSPFv3SPFVertex *parent) const;
    std::vector<NextHop> *calculateNextHops(OSPFv3LSA *destination, OSPFv3LSA *parent) const;


    std::string detailedInfo() const override;

    void setSpfTreeRoot(OSPFv3RouterLSA* routerLSA){this->spfTreeRoot = routerLSA;};

  private:
    IPv4Address areaID;
    OSPFv3AreaType areaType;
    std::vector<OSPFv3Interface*> interfaceList;//associated router interfaces
    std::map<std::string, OSPFv3Interface*> interfaceByName;//interfaces by ids
    std::map<int, OSPFv3Interface*> interfaceById;
    std::map<int, OSPFv3Interface*> interfaceByIndex;
    int instanceType;
    OSPFv3Instance* containingInstance;
    bool externalRoutingCapability;
    int stubDefaultCost;
    bool transitCapability;

    std::vector<OSPFv3RouterLSA* > routerLSAList;
    IPv4Address routerLsID = IPv4Address::UNSPECIFIED_ADDRESS;
    uint32_t routerLSASequenceNumber = 1;

    std::vector<OSPFv3NetworkLSA* > networkLSAList;
    IPv4Address networkLsID = IPv4Address::UNSPECIFIED_ADDRESS;
    uint32_t networkLSASequenceNumber = 1;

    std::vector<OSPFv3InterAreaPrefixLSA* > interAreaPrefixLSAList;
    IPv4Address interAreaPrefixLsID = IPv4Address::UNSPECIFIED_ADDRESS;
    uint32_t interAreaPrefixLSASequenceNumber = 1;

    std::vector<OSPFv3IntraAreaPrefixLSA*> intraAreaPrefixLSAList;
    IPv4Address intraAreaPrefixLsID = IPv4Address::UNSPECIFIED_ADDRESS;
    uint32_t intraAreaPrefixLSASequenceNumber = 1;

    IPv4Address netIntraAreaPrefixLsID = IPv4Address::UNSPECIFIED_ADDRESS;
    uint32_t netIntraAreaPrefixLSASequenceNumber = 1;

    OSPFv3RouterLSA* spfTreeRoot=nullptr;
    //list of summary lsas
    //shortest path tree
};

inline std::ostream& operator<<(std::ostream& ostr, const OSPFv3Area& area)
{
    ostr << area.detailedInfo();
    return ostr;
}

}//namespace inet

#endif
