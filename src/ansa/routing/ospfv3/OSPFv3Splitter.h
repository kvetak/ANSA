#ifndef __ANSA_OSPFV3SPLITTER_H_
#define __ANSA_OSPFV3SPLITTER_H_

#include <omnetpp.h>
//#include <cmodule.h>
#include <string>
#include "inet/common/INETDefs.h"
#include "ansa/routing/ospfv3/process/OSPFv3Process.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv6/IPv6InterfaceData.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/networklayer/ipv6/IPv6RoutingTable.h"
#include "inet/networklayer/common/IPSocket.h"
#include "inet/networklayer/contract/ipv6/IPv6ControlInfo.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/contract/ipv6/IPv6Address.h"

#include "ansa/routing/ospfv3/OSPFv3Packet_m.h"
#include "ansa/routing/ospfv3/process/OSPFv3Process.h"
#include "ansa/routing/ospfv3/OSPFv3Timers.h"
#include "ansa/routing/ospfv3/OSPFv3Common.h"

namespace inet{

int OSPFv3InterfaceIndex=1;

class INET_API OSPFv3Splitter : protected cListener, public cSimpleModule
{
  public:
    OSPFv3Splitter();
    virtual ~OSPFv3Splitter();
    IInterfaceTable* ift = nullptr;

  private:
    void parseConfig(cXMLElement*, cXMLElement*);
    void parseRouting(cXMLElement*);
    void parseInterfaces(cXMLElement*);
    void addNewProcess(cXMLElement*, cXMLElement*, int);

  private:
    cModule* containingModule=nullptr;
    cModule* routingModule=nullptr;
    std::vector<OSPFv3Process*> processesModules;
    std::map<std::string ,int> processInVector; //processID is mapped to its position in vector of processes in Splitter
    std::map<std::string ,std::pair<int, int>> interfaceToProcess;//name of interface to position in vector - "eth0" : 0
    std::map<char*,char*> processToInterface;//"process101":"eth0", "process100":"eth0"

  protected:
     virtual void initialize(int stage) override;
     virtual void handleMessage(cMessage* msg) override;
     virtual int numInitStages() const override { return NUM_INIT_STAGES; }
};
}//namespace inet
#endif
