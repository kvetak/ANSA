package inet.ansa.rip;

import inet.base.NotificationBoard;
import inet.linklayer.ppp.PPPInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.RoutingTable;
import inet.networklayer.ospfv2.OSPFRouting;
import inet.transport.udp.UDP;
import inet.nodes.inet.NetworkLayer;
import inet.ansa.rip.RIPRouting;
import inet.ansa.InterfaceStateManager.InterfaceStateManager;
import inet.ansa.ethernet.AnsaEthernetInterface;

import inet.applications.generic.IPTrafGen;
import inet.applications.generic.IPTrafSink;
//
// \IP router.
//
module RIPGenRouter
{
    parameters:
        @node();
        string routingFile = default("");
        string hostname = default("");
        @display("i=srouter;bgb=366,330");

    gates:
        inout pppg[];
        inout ethg[];
    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=52,118;i=block/control");
        }
        interfaceStateManager: InterfaceStateManager {
            parameters:
                @display("p=145,60");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=52,60;i=block/table");
        }
        routingTable: RoutingTable {
            parameters:
                IPForward = true;
                routerId = "auto";
                routingFile = routingFile;
                @display("p=240,60;i=block/table");
        }
        rip: RIPRouting {
            parameters:
                hostname = hostname;
                @display("p=52,180,row;i=block/network2");
        }
        udp: UDP {
            parameters:
                @display("p=136,138;i=block/transport");
            gates:
                appIn[1];
                appOut[1];
        }
        networkLayer: NetworkLayer {
            parameters:
                @display("p=224,141;i=block/fork;q=queue");
            gates:
                ifIn[sizeof(pppg)+sizeof(ethg)];
                ifOut[sizeof(pppg)+sizeof(ethg)];
        }
        ppp[sizeof(pppg)]: PPPInterface {
            parameters:
                @display("p=114,257,row,110;q=l2queue;i=block/ifcard");
        }
        eth[sizeof(ethg)]: AnsaEthernetInterface {
            parameters:
                @display("p=209,257,row,110;q=l2queue;i=block/ifcard");
        }
        ipTrafGen: IPTrafGen {
            @display("p=302,99");
        }
        ipTrafSink: IPTrafSink {
            @display("p=302,177");
        }
    connections allowunconnected:

        rip.udpOut --> udp.appIn[0];
        rip.udpIn <-- udp.appOut[0];

        udp.ipOut --> networkLayer.udpIn;
        udp.ipIn <-- networkLayer.udpOut;

        ipTrafGen.ipOut --> networkLayer.rsvpIn;
        ipTrafSink.ipIn <-- networkLayer.rsvpOut;
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

