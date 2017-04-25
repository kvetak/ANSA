#include "ansa/routing/ospfv3/interface/OSPFv3InterfaceStateDown.h"
#include "ansa/routing/ospfv3/interface/OSPFv3Interface.h"
#include "ansa/routing/ospfv3/interface/OSPFv3InterfacePassive.h"

namespace inet{
void OSPFv3InterfacePassive::processEvent(OSPFv3Interface* interface, OSPFv3Interface::OSPFv3InterfaceEvent event)
{
    if (event == OSPFv3Interface::INTERFACE_DOWN_EVENT) {
        interface->reset();
        changeState(interface, new OSPFv3InterfaceStateDown, this);
    }
}//processEvent
}//namespace inet
