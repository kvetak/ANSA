package inet.examples.ansa.ospfv3.v3_4_routers;

import ansa.node.ANSA_Host;
import ansa.node.ANSA_OSPFv3_Router;
import ansa.node.ANSA_Router;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.common.misc.ThruputMeteringChannel;
import inet.node.ethernet.EtherSwitch;
import inet.common.scenario.ScenarioManager;


network v3_4_routers
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
                            "</config>");
                @display("p=147,54");
        }
        Router1: ANSA_OSPFv3_Router {
            parameters:
                @display("p=56,230");
            gates:
                ethg[1];
        }
        Router2: ANSA_OSPFv3_Router {
            parameters:
                @display("p=213,153");
            gates:
                ethg[2];
        }
        Router3: ANSA_OSPFv3_Router {
            parameters:
                @display("p=376,153");
            gates:
                ethg[2];
        }
        Router4: ANSA_OSPFv3_Router {
            parameters:
                @display("p=519,50");
            gates:
                ethg[1];
        }
        scenarioManager: ScenarioManager {
            @display("p=450,357");
        }
    connections:
        Router1.ethg[0] <--> Channel <--> Router2.ethg[0];
        Router2.ethg[1] <--> Channel <--> Router3.ethg[0];
        Router3.ethg[1] <--> Channel <--> Router4.ethg[0];
}
