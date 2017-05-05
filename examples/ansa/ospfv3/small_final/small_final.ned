package inet.examples.ansa.ospfv3.small_final;

import ansa.node.ANSA_Host;
import ansa.node.ANSA_OSPFv3_Router;
import ansa.node.ANSA_Router;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.networklayer.configurator.ipv6.FlatNetworkConfigurator6;
import inet.common.misc.ThruputMeteringChannel;
import inet.node.ethernet.EtherSwitch;


network small_final
{
    @display("bgb=810,392");
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
//                            "<interface among='R1 R2 R3 R4' address='192.168.1.x' netmask='255.255.255.0' />"+
//							"<interface among='R1 R5 R6' address='192.168.10.x' netmask='255.255.255.0' />"+
//							"<interface hosts='R2' names='eth1' address='192.168.20.1' netmask='255.255.255.0' mtu='1500' metric='1'/>" +
//							"<interface hosts='Area2' names='eth0' address='192.168.20.2' netmask='255.255.255.0' mtu='1500' metric='1'/>" +
////							"<interface among='R2 Area2' address='192.168.20.x' netmask='255.255.255.0' />"+
//							"<interface among='R3 Area3' address='192.168.30.x' netmask='255.255.255.0' />"+
//							"<interface among='R4 Area4' address='192.168.40.x' netmask='255.255.255.0' />"+
//							"<interface among='R5 Area5' address='192.168.50.x' netmask='255.255.255.0' />"+
//							"<interface among='R6 Area6' address='192.168.60.x' netmask='255.255.255.0' />"+
                            "</config>");
                assignAddresses = false;
                assignDisjunctSubnetAddresses = false;
                addStaticRoutes = false;
                addDefaultRoutes = false;
                addSubnetRoutes = false;
                optimizeRoutes = false;      
                @display("p=89,31");
        }
        
        etherSwitch: EtherSwitch {
            @display("p=451,181");
        }
        etherSwitch1: EtherSwitch {
            @display("p=214,255");
        }
        
        R1: ANSA_OSPFv3_Router {
            parameters:
                @display("p=339,256");
            gates:
                ethg[2];
        }
        R2: ANSA_OSPFv3_Router {
            parameters:
                @display("p=564,256");
            gates:
                ethg[2];
        }
        R3: ANSA_OSPFv3_Router {
            @display("p=564,103");
            gates:
                ethg[2];
                pppg[1];
        }
        R4: ANSA_OSPFv3_Router {
            @display("p=339,103");
            gates:
                ethg[2];
                pppg[1];
        }
        R5: ANSA_OSPFv3_Router {
            @display("p=120,318");
            gates:
                ethg[2];
        }
        R6: ANSA_OSPFv3_Router {
            @display("p=120,210");
            gates:
                ethg[2];
        }
        
        Area2: ANSA_Host {
            @display("p=689,256");
            gates:
                ethg[1];
        }
        Area4: ANSA_Host {
            @display("p=228,103");
            gates:
                ethg[1];
        }
        Area3: ANSA_Host {
            @display("p=689,103");
            gates:
                ethg[1];
        }
        Area5: ANSA_Host {
            @display("p=228,344");
            gates:
                ethg[1];
        }
        Area6: ANSA_Host {
            @display("p=120,123");
            gates:
                ethg[1];
        }
        
        
    connections:
        R1.ethg[0] <--> Channel <--> etherSwitch.ethg++;
        R1.ethg[1] <--> Channel <--> etherSwitch1.ethg++;
        
        R2.ethg[0] <--> Channel <--> etherSwitch.ethg++;
        R2.ethg[1] <--> Channel <--> Area2.ethg[0];
        
        R3.ethg[0] <--> Channel <--> etherSwitch.ethg++;
        R3.ethg[1] <--> Channel <--> Area3.ethg[0];
        
        R3.pppg[0] <--> Channel <--> R4.pppg[0];
        
        R4.ethg[0] <--> Channel <--> etherSwitch.ethg++;
        R4.ethg[1] <--> Channel <--> Area4.ethg[0];
        
        R5.ethg[0] <--> Channel <--> etherSwitch1.ethg++;
        R5.ethg[1] <--> Channel <--> Area5.ethg[0];
        
        R6.ethg[0] <--> Channel <--> etherSwitch1.ethg++;
        R6.ethg[1] <--> Channel <--> Area6.ethg[0];
}
