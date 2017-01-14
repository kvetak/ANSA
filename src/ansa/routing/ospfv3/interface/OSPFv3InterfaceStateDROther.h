#ifndef __ANSA_OSPFV3INTERFACESTATEDROTHER_H_
#define __ANSA_OSPFV3INTERFACESTATEDROTHER_H_

#include <omnetpp.h>
#include <string>
#include "inet/common/INETDefs.h"
#include "ansa/routing/ospfv3/interface/OSPFv3InterfaceState.h"

namespace inet{

/*
 * This router is neither DR nor BDR. The interface is on NBMA Network. It forms adjacencies
 * with both DR and BDR.
 */

class INET_API OSPFv3InterfaceStateDROther : public OSPFv3InterfaceState
{
  public:
    ~OSPFv3InterfaceStateDROther() {};
    virtual void processEvent(OSPFv3Interface* intf, OSPFv3Interface::OSPFv3InterfaceEvent event) override;
    OSPFv3Interface::OSPFv3InterfaceFAState getState() const override { return OSPFv3Interface::INTERFACE_STATE_DROTHER; }
    std::string getInterfaceStateString() const {return std::string("OSPFv3InterfaceStateDROther");};
};

}//namespace inet

#endif
