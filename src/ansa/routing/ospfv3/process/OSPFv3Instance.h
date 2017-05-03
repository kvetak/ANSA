#ifndef __ANSA_OSPFV3INSTANCE_H_
#define __ANSA_OSPFV3INSTANCE_H_

#include <omnetpp.h>
#include <string>
#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/common/ModuleAccess.h"

#include "ansa/routing/ospfv3/process/OSPFv3Process.h"
#include "ansa/routing/ospfv3/process/OSPFv3Area.h"
#include "ansa/routing/ospfv3/OSPFv3Common.h"
#include "ansa/routing/ospfv3/OSPFv3Packet_m.h"
#include "ansa/routing/ospfv3/neighbor/OSPFv3Neighbor.h"
#include "ansa/routing/ospfv3/interface/OSPFv3Interface.h"


namespace inet{

class OSPFv3Process;
class OSPFv3Area;
//class OSPFv3Neighbor;

class INET_API OSPFv3Instance : public cObject
{
  public:
    OSPFv3Instance(int instanceId, OSPFv3Process* parentProcess, int addressFamily);
    virtual ~OSPFv3Instance();
    int getInstanceID(){return this->instanceID;};
    int getAddressFamily(){return this->addressFamily;};
    bool hasArea(IPv4Address);
    void addArea(OSPFv3Area*);
    OSPFv3Area* getAreaById(IPv4Address areaId);
    OSPFv3Area* getArea(int i){return this->areas.at(i);}
    void debugDump();
    void processPacket(OSPFv3Packet* packet);
    void init();
    OSPFv3Process* getProcess() const {return this->containingProcess;}
    int getUniqueId(){return OSPFv3IfIndex++;}
    int getAreaCount(){return this->areas.size();}
    void removeFromAllRetransmissionLists(LSAKeyType lsaKey);

    std::string detailedInfo() const override;

  public:
    IInterfaceTable* ift = nullptr;

  private:




  private:
    int addressFamily;
    int instanceID;
    OSPFv3Process* containingProcess;
    std::vector<OSPFv3Area* > areas; //list of areas in this instance
    cModule* containingModule=nullptr;
    std::map<IPv4Address , OSPFv3Area*> areasById; //mapping the area id to area
    int OSPFv3IfIndex = 0; //unique number for interfaces



};

inline std::ostream& operator<<(std::ostream& ostr, const OSPFv3Instance& instance)
{
    ostr << instance.detailedInfo();
    return ostr;
}
}//namespace inet
#endif
