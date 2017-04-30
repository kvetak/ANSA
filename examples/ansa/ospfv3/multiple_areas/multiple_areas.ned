package inet.examples.ansa.ospfv3.multiple_areas;

import ansa.node.ANSA_Host;
import ansa.node.ANSA_OSPFv3_Router;
import ansa.node.ANSA_Router;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.networklayer.configurator.ipv6.FlatNetworkConfigurator6;
import inet.common.misc.ThruputMeteringChannel;
import inet.node.ethernet.EtherSwitch;


network multiple_areas
{
    @display("bgb=865,496");
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
                            "<interface among='Router1 Router2 Router3 Router4 Router5' address='192.168.1.x' netmask='255.255.255.0' />"+
							"<interface among='Router1 Router6' address='192.168.10.x' netmask='255.255.255.0' />"+
							"<interface among='Router2 Router7' address='192.168.20.x' netmask='255.255.255.0' />"+
							"<interface among='Router3 Router8' address='192.168.30.x' netmask='255.255.255.0' />"+
							"<interface among='Router4 Router9' address='192.168.40.x' netmask='255.255.255.0' />"+
							"<interface among='Router5 Router10' address='192.168.50.x' netmask='255.255.255.0' />"+
							"<interface among='Router6 Host1' address='192.168.11.x' netmask='255.255.255.0' />"+
							"<interface among='Router7 Host2' address='192.168.12.x' netmask='255.255.255.0' />"+
							"<interface among='Router8 Host3' address='192.168.13.x' netmask='255.255.255.0' />"+
							"<interface among='Router9 Host4' address='192.168.14.x' netmask='255.255.255.0' />"+
							"<interface among='Router10 Host5' address='192.168.15.x' netmask='255.255.255.0' />"+
                            "</config>");
                @display("p=23,24");
        }
        Router1: ANSA_OSPFv3_Router {
            parameters:
                @display("p=308,316");
            gates:
                ethg[2];
        }
        Router2: ANSA_OSPFv3_Router {
            parameters:
                @display("p=410,316");
            gates:
                ethg[2];
        }
        Router4: ANSA_OSPFv3_Router {
            @display("p=474,187");
            gates:
                ethg[2];
        }
        Router3: ANSA_OSPFv3_Router {
            @display("p=510,316");
            gates:
                ethg[2];
        }
        Router5: ANSA_OSPFv3_Router {
            @display("p=347,183");
            gates:
                ethg[2];
        }
        Router6: ANSA_OSPFv3_Router {
            @display("p=165,316");
            gates:
                ethg[2];
        }
        Router7: ANSA_OSPFv3_Router {
            @display("p=410,419");
            gates:
                ethg[2];
        }
        Router8: ANSA_OSPFv3_Router {
            @display("p=654,316");
            gates:
                ethg[2];
        }
        Router9: ANSA_OSPFv3_Router {
            @display("p=579,101");
            gates:
                ethg[2];
        }
        Router10: ANSA_OSPFv3_Router {
            @display("p=234,101");
            gates:
                ethg[2];
        }
        
        etherSwitch: EtherSwitch {
            @display("p=410,239");
        }
        
        Host1: ANSA_Host {
            @display("p=40,316");
            gates:
                ethg[1];
        }
        Host2: ANSA_Host {
            @display("p=305,419");
            gates:
                ethg[1];
        }
        Host4: ANSA_Host {
            @display("p=743,101");
            gates:
                ethg[1];
        }
        Host3: ANSA_Host {
            @display("p=800,316");
            gates:
                ethg[1];
        }
        Host5: ANSA_Host {
            @display("p=103,101");
            gates:
                ethg[1];
        }
    connections:
        Router1.ethg[0] <--> Channel <--> etherSwitch.ethg++;
        Router2.ethg[0] <--> Channel <--> etherSwitch.ethg++;
        Router3.ethg[0] <--> Channel <--> etherSwitch.ethg++;
        Router4.ethg[0] <--> Channel <--> etherSwitch.ethg++;
        Router5.ethg[0] <--> Channel <--> etherSwitch.ethg++;
        
        Router1.ethg[1] <--> Channel <--> Router6.ethg[0];
        Router2.ethg[1] <--> Channel <--> Router7.ethg[0];
        Router3.ethg[1] <--> Channel <--> Router8.ethg[0];
        Router4.ethg[1] <--> Channel <--> Router9.ethg[0];
        Router5.ethg[1] <--> Channel <--> Router10.ethg[0];

        Router6.ethg[1] <--> Channel <--> Host1.ethg[0];
        Router7.ethg[1] <--> Channel <--> Host2.ethg[0];
        Router8.ethg[1] <--> Channel <--> Host3.ethg[0];
        Router9.ethg[1] <--> Channel <--> Host4.ethg[0];
        Router10.ethg[1] <--> Channel <--> Host5.ethg[0];
}
