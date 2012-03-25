package inet.ansa.rip;

import inet.base.NotificationBoard;
import inet.linklayer.ethernet.EthernetInterface;
import inet.linklayer.ppp.PPPInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.networklayer.ospfv2.OSPFRouting;
import inet.transport.udp.UDP;
import inet.nodes.inet.NetworkLayer;
import inet.ansa.rip.RIPRouting;

//
// \IP router.
//
module RedistrRouter
{
    parameters:
        string routingFile = default("");
        @display("i=srouter");
                             //numUdpApps: numeric const;
                             //udpAppType: string;
    gates:
        inout pppg[];
        inout ethg[];
    submodules:
        notificationBoard: NotificationBoard 
        {
            parameters:
            	@display("p=60,60;i=block/control");
        }
        interfaceTable: InterfaceTable
        {
            parameters:
            	@display("p=150,60;i=block/table");
        }
        routingTable: RoutingTable
        {
            parameters:
                IPForward = true;
                routerId = "auto";
                routingFile = routingFile;
            	@display("p=240,60;i=block/table");
        }
        rip: RIPRouting
        {
            parameters:
                localPort = 520;
                destPort = 520;
            	@display("p=52,180,row;i=block/network2");
        }
        udp: UDP
        {
            parameters:
            	@display("p=136,138;i=block/transport");
        }
        networkLayer: NetworkLayer
        {
            parameters:
                @display("p=224,141;i=block/fork;q=queue");
            gates:
                ifIn[sizeof(pppg)+sizeof(ethg)];
                ifOut[sizeof(pppg)+sizeof(ethg)];
        }
        ppp[sizeof(pppg)]: PPPInterface
        {
            parameters:
            	@display("p=114,257,row,110;q=l2queue;i=block/ifcard");
        }
        eth[sizeof(ethg)]: EthernetInterface
        {
            parameters:
            	@display("p=209,257,row,110;q=l2queue;i=block/ifcard");
        }
    connections allowunconnected:

        rip.udpOut --> udp.appIn[0];
        rip.udpIn <-- udp.appOut[0];

        udp.ipOut --> networkLayer.udpIn;
        udp.ipIn <-- networkLayer.udpOut;

      //rip.ipOut --> networkLayer.RIPIn;
      //rip.ipIn <-- networkLayer.RIPOut;
      //ospf.ipOut --> networkLayer.ospfIn;
      //ospf.ipIn <-- networkLayer.ospfOut;

        // connections to network outside
        
        for i=0..sizeof(pppg)-1 {
            pppg[i] <--> ppp[i].phys;
            ppp[i].netwOut --> networkLayer.ifIn[i];
            ppp[i].netwIn <-- networkLayer.ifOut[i];
        }

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].netwOut --> networkLayer.ifIn[sizeof(pppg)+i];
            eth[i].netwIn <-- networkLayer.ifOut[sizeof(pppg)+i];
        }
}

