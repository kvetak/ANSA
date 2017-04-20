#include "ansa/routing/ospfv3/process/OSPFv3RoutingTableEntry.h"

namespace inet{
OSPFv3RoutingTableEntry::OSPFv3RoutingTableEntry(IInterfaceTable *_ift) :
    ift(_ift),
    destinationType(OSPFv3RoutingTableEntry::NETWORK_DESTINATION),
    area(BACKBONE_AREAID),
    pathType(OSPFv3RoutingTableEntry::INTRAAREA)
{
//    setNetmask(IPv4Address::ALLONES_ADDRESS);
//    setSourceType(IRoute::OSPF);
}

OSPFv3RoutingTableEntry::OSPFv3RoutingTableEntry(const OSPFv3RoutingTableEntry& entry) :
    destinationType(entry.destinationType),
    optionalCapabilities(entry.optionalCapabilities),
    area(entry.area),
    pathType(entry.pathType),
    cost(entry.cost),
    type2Cost(entry.type2Cost),
    linkStateOrigin(entry.linkStateOrigin),
    nextHops(entry.nextHops)
{
//    setDestination(entry.getDestination());
//    setNetmask(entry.getNetmask());
//    setGateway(entry.getGateway());
//    setInterface(entry.getInterface());
//    setSourceType(entry.getSourceType());
//    setMetric(entry.getMetric());
}
}
