//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package ansa.networklayer.clns;

//import inet.networklayer.configurator.ipv4.IPv4NodeConfigurator;
import inet.networklayer.contract.INetworkLayer;


//
// Network layer for CLNS
//

//
module CLNSNetworkLayer like INetworkLayer
{
    parameters:



        string interfaceTableModule;
        string routingTableModule;
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.routingTableModule = default(absPath(routingTableModule));



        @display("i=block/fork");
    gates:
        input ifIn @labels(INetworkDatagram);
        output ifOut @labels(INetworkDatagram);
        input transportIn @labels(CLNSControlInfo/down);
        output transportOut @labels(CLNSControlInfo/up);
        input pingIn[] @labels(PingPayload/down);
        output pingOut[] @labels(PingPayload/up);
//        input igmpIn;
//        output igmpOut;

    submodules:
//        configurator: IPv4NodeConfigurator {
//            @display("p=39,158");
//        }
        clns: CLNS {
            parameters:
                
                @display("p=85,95;q=queue");
            gates:
                queueIn[sizeof(ifIn)];
                queueOut[sizeof(ifOut)];
        }
        

    connections allowunconnected:
        for i=0..sizeof(transportIn)-1 {
            transportIn[i] --> { @display("m=n"); } --> clns.transportIn++;
        }
        for i=0..sizeof(transportOut)-1 {
            clns.transportOut++ --> { @display("m=n"); } --> transportOut[i];
        }

        

        
//        for i=0..sizeof(pingOut)-1 {
//            icmp.pingOut++ --> { @display("m=n"); } --> pingOut[i];
//        }
//        for i=0..sizeof(pingIn)-1 {
//            icmp.pingIn++ <-- { @display("m=n"); } <-- pingIn[i];
//        }

//        icmp.errorOut --> errorHandling.in;

//        ip.arpOut --> arp.netwIn;
//        ip.arpIn <-- arp.netwOut;

//        igmp.routerOut --> igmpOut;
//        igmp.routerIn <-- igmpIn;

        for i=0..sizeof(ifIn)-1 {
            clns.queueIn[i] <-- { @display("m=s"); } <-- ifIn[i];
        }
        for i=0..sizeof(ifOut)-1 {
            clns.queueOut[i] --> { @display("m=s"); } --> ifOut[i];
        }
}

