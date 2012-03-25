//
// Marek Cerny, 2MSK
// FIT VUT 2011
//
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

package inet.examples.ansaExamples.PingDualStack;

import inet.ansa.AnsaDualStackHost;
import inet.ansa.AnsaDualStackRouter;
import inet.nodes.inet.StandardHost;
import inet.ansa.ANSARouter;
import inet.nodes.ethernet.EtherSwitch;

import inet.networklayer.autorouting.FlatNetworkConfigurator6;

import ned.DatarateChannel;

network PingDualStack
{
    types:
        channel ethernetline extends DatarateChannel
        {
            delay = 0.1us;
        }
    submodules:
//        IPv6_conf: FlatNetworkConfigurator6 {

        //            @display("p=50,44");
        //        }
        hostA: AnsaDualStackHost {
            @display("p=66,36;i=device/pc2");
        }
        hostB: AnsaDualStackHost {
            @display("p=48,132;i=device/pc2");
        }
        server: AnsaDualStackHost {
            @display("p=389,109");
        }
        switch: EtherSwitch {
            @display("p=116,78");
        }
        router1: AnsaDualStackRouter {
            @display("p=191,36");
        }
        router2: AnsaDualStackRouter {
            @display("p=305,51");
        }
        router3: AnsaDualStackRouter {
            @display("p=220,101");
        }
    connections allowunconnected:
        router1.ethg++ <--> ethernetline <--> router2.ethg++;
        router1.ethg++ <--> ethernetline <--> router3.ethg++;
        router3.ethg++ <--> ethernetline <--> router2.ethg++;
        hostA.ethg++ <--> ethernetline <--> switch.ethg++;
        hostB.ethg++ <--> ethernetline <--> switch.ethg++;
        switch.ethg++ <--> ethernetline <--> router1.ethg++;
        server.ethg++ <--> ethernetline <--> router2.ethg++;
}
