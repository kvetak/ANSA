package inet.examples.ansa.ospfv3.v3_2_areas;

import ansa.node.ANSA_Host;
import ansa.node.ANSA_OSPFv3_Router;
import ansa.node.ANSA_Router;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
//import inet.networklayer.configurator.ipv6.FlatNetworkConfigurator6;
import inet.common.misc.ThruputMeteringChannel;
import inet.node.ethernet.EtherSwitch;


network v3_2_areas
{
    types:
        channel Channel extends ThruputMeteringChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
            thruputDisplayFormat = "#N";
        }
    submodules:
        configurator: IPv4NetworkConfigurator {
            parameters:
                config = xml("<config>"+
                            "<interface among='Router1 Router2' address='10.10.10.x' netmask='255.255.255.0' />"+
                            "<interface among='Router2 Router3' address='10.10.20.x' netmask='255.255.255.0' />"+
                            "</config>");
                @display("p=147,54");
        }
        Router1: ANSA_OSPFv3_Router {
            parameters:
                @display("p=75,185");
            gates:
                ethg[1];
        }
        Router2: ANSA_OSPFv3_Router {
            parameters:
                @display("p=244,97");
            gates:
                ethg[2];
        }
        Router3: ANSA_OSPFv3_Router {
            parameters:
                @display("p=365,185");
            gates:
                ethg[1];
        }

    connections:
        Router1.ethg[0] <--> Channel <--> Router2.ethg[0];
        Router2.ethg[1] <--> Channel <--> Router3.ethg[0];
}
