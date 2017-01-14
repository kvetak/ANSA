package ansa.routing.ospfv3;

simple OSPFv3Splitter{
    parameters:
        xml ospfv3RoutingConfig;
        xml ospfv3IntConfig;
    gates:
        input ipIn @labels(OSPFv3Packet/up);
        output ipOut @labels(OSPFv3Packet/down);
        input processIn[];
        output processOut[];
}