#ifndef __ANSA_OSPFV3INTERFACESTATELOOPBACK_H_
#define __ANSA_OSPFV3INTERFACESTATELOOPBACK_H_

#include <omnetpp.h>
#include <string>
#include "inet/common/INETDefs.h"
#include "ansa/routing/ospfv3/interface/OSPFv3InterfaceState.h"

namespace inet{
/*
 * Interface os looped back, it is unsiutable for normal traffic. It is still desirable to gain information
 * about the quality of this interface (ICMP...). Advertised as router LSA with single host route, whose dest IP
 * is the interface IP.
 */

class INET_API OSPFv3InterfaceStateLoopback : public OSPFv3InterfaceState
{
  public:
    ~OSPFv3InterfaceStateLoopback() {};
    virtual void processEvent(OSPFv3Interface* intf, OSPFv3Interface::OSPFv3InterfaceEvent event) override;
    OSPFv3Interface::OSPFv3InterfaceFAState getState() const override { return OSPFv3Interface::INTERFACE_STATE_LOOPBACK; }
    std::string getInterfaceStateString() const {return std::string("OSPFv3InterfaceStateLoopback");};
};

}//namespace inet

#endif
