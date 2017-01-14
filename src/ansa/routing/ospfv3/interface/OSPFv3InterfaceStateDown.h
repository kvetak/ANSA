#ifndef __ANSA_OSPFV3INTERFACESTATEDOWN_H_
#define __ANSA_OSPFV3INTERFACESTATEDOWN_H_

#include "ansa/routing/ospfv3/interface/OSPFv3InterfaceState.h"
#include "ansa/routing/ospfv3/interface/OSPFv3InterfaceStateWaiting.h"
#include <omnetpp.h>
#include <string>
#include "inet/common/INETDefs.h"


namespace inet{

class INET_API OSPFv3InterfaceStateDown : public OSPFv3InterfaceState
{
  public:
    ~OSPFv3InterfaceStateDown() {};
    void processEvent(OSPFv3Interface* intf,OSPFv3Interface::OSPFv3InterfaceEvent event) override;
    OSPFv3Interface::OSPFv3InterfaceFAState getState() const override { return OSPFv3Interface::INTERFACE_STATE_DOWN; }
    std::string getInterfaceStateString() const {return std::string("OSPFv3InterfaceStateDown");};
};

}//namespace inet

#endif
