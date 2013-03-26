
// vim: filetype=c
package inet.ansa;

import inet.ansa.linklayer.switch.stp.Stp;
//import inet.nodes.inet.BurstHost;
import inet.base.NotificationBoard;
import inet.ansa.linklayer.ethernet.AnsaEtherMAC;
import inet.ansa.linklayer.switch.ANSASwitchCore;
import inet.ansa.linklayer.switch.MACTable;

import inet.ansa.linklayer.switch.sink;
import inet.ansa.linklayer.switch.VLANTable;


import inet.transport.rtp.RTPAVProfilePayload10Receiver;
import inet.ansa.linklayer.switch.PortTable;
import inet.ansa.acl.acl;
import inet.nodes.inet.NetworkLayer;
//import inet.ansa.TrafficGenerator.TrafGen;
//import inet.ansa.rip.RIPRouting;
import inet.transport.udp.UDP;
import inet.networklayer.ospfv2.OSPFRouting;
//import inet.ansa.InterfaceStateManager.InterfaceStateManager;
import inet.networklayer.common.InterfaceTable;
//import inet.ansa.routingTable.AnsaInterfaceTable;
//import inet.ansa.routingTable.AnsaRoutingTable;
module ANSASwitch
{
    parameters:
        @node();
        string configFile = default("");
        string hostname = default("");
        string switchID = default("");
        @display("i=abstract/switch;bgb=472,322");
    gates:
        inout ethg[];
        //inout pppg[];

    submodules:
        core: ANSASwitchCore {
            parameters:
                @display("p=277,106;is=l");
            gates:
                ifIn[sizeof(ethg)];
                ifOut[sizeof(ethg)];
                stpIn[sizeof(ethg)];
                stpOut[sizeof(ethg)];
        }
        mac[sizeof(ethg)]: AnsaEtherMAC {
            parameters:
                @display("p=71,266,row,110;q=l2queue;is=l");
                promiscuous = true;
        }
        MACTable: MACTable {
            @display("p=53,106;is=l");
        }
        VLANTable: VLANTable {
            parameters:
                configFile = configFile;
                switchID = switchID;
                portCount = sizeof(ethg);
                @display("p=158,106;is=l");
        }
        sink: sink {
            @display("p=379,131");
        }
        notificationBoard: NotificationBoard {
            @display("p=158,33");
        }
        stp: Stp {
            parameters:
                configFile = configFile;
                switchID = switchID;
                portCount = sizeof(ethg);
                @display("p=379,57;is=l");
            gates:
                in[sizeof(ethg)];
                out[sizeof(ethg)];
        }
    connections:

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> mac[i].phys;
            mac[i].upperLayerOut --> core.ifIn[i];
            mac[i].upperLayerIn <-- core.ifOut[i];
            stp.out[i] --> core.stpIn[i];
            stp.in[i] <-- core.stpOut[i];
        }
        core.toSink --> sink.ifIn;
}
