package inet.examples.ansa.ospfv3.small;

import ansa.node.ANSA_Host;
import ansa.node.ANSA_OSPFv3_Router;
import ansa.node.ANSA_Router;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.networklayer.configurator.ipv6.FlatNetworkConfigurator6;
import inet.common.misc.ThruputMeteringChannel;
import inet.node.ethernet.EtherSwitch;


network small
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
        configurator: IPv4NetworkConfigurator {
            parameters:
                config = xml("<config>"+
                            "<interface among='Router1 Router2 Router3 Router4' address='192.168.1.x' netmask='255.255.255.0' />"+
							"<interface among='Router1 Host1' address='172.16.10.x' netmask='255.255.255.0' />"+
							"<interface among='Router2 Host2' address='172.16.20.x' netmask='255.255.255.0' />"+
							"<interface among='Router3 Host3' address='172.16.30.x' netmask='255.255.255.0' />"+
							"<interface among='Router4 Host4' address='172.16.40.x' netmask='255.255.255.0' />"+
                            "</config>");
                @display("p=211,62");
        }
        Router1: ANSA_OSPFv3_Router {
            parameters:
                @display("p=180,223");
            gates:
                ethg[2];
        }
        Router2: ANSA_OSPFv3_Router {
            parameters:
                @display("p=405,223");
            gates:
                ethg[2];
        }
        etherSwitch: EtherSwitch {
            @display("p=292,179");
        }
        Router4: ANSA_OSPFv3_Router {
            @display("p=405,140");
            gates:
                ethg[2];
        }
        Router3: ANSA_OSPFv3_Router {
            @display("p=180,140");
            gates:
                ethg[2];
        }
        Host1: ANSA_Host {
            @display("p=51,227");
            gates:
                ethg[1];
        }
        Host2: ANSA_Host {
            @display("p=513,223");
            gates:
                ethg[1];
        }
        Host4: ANSA_Host {
            @display("p=513,140");
            gates:
                ethg[1];
        }
        Host3: ANSA_Host {
            @display("p=51,140");
            gates:
                ethg[1];
        }
    connections:
        Router1.ethg[0] <--> Channel <--> etherSwitch.ethg++;
        Router2.ethg[0] <--> Channel <--> etherSwitch.ethg++;
        Router3.ethg[0] <--> Channel <--> etherSwitch.ethg++;
        Router4.ethg[0] <--> Channel <--> etherSwitch.ethg++;
        
        Router1.ethg[1] <--> Channel <--> Host1.ethg[0];
        Router2.ethg[1] <--> Channel <--> Host2.ethg[0];
        Router3.ethg[1] <--> Channel <--> Host3.ethg[0];
        Router4.ethg[1] <--> Channel <--> Host4.ethg[0];
}
