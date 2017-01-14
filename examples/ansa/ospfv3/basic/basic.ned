package inet.examples.ansa.ospfv3.basic;

import ansa.node.ANSA_OSPFv3Router;
import ansa.node.ANSA_Router;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.networklayer.configurator.ipv6.FlatNetworkConfigurator6;
import inet.common.misc.ThruputMeteringChannel;


network basic
{
    @display("bgb=558,329");
    types:
        channel Channel extends ThruputMeteringChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
            thruputDisplayFormat = "#N";
        }
    submodules:
        configurator6: FlatNetworkConfigurator6 {
            @display("p=281,62");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                config = xml("<config>"+
                            "<interface among='Router1 Router2' address='192.168.1.x' netmask='255.255.255.0' />"+
                            "</config>");
                @display("p=140,62");
        }
        Router1: ANSA_OSPFv3Router {
            parameters:
                @display("p=63,223");
            gates:
                ethg[1];
        }
        Router2: ANSA_OSPFv3Router {
            parameters:
                @display("p=373,223");
            gates:
                ethg[1];
        }
    connections:
        Router1.ethg[0] <--> Channel <--> Router2.ethg[0];
}
