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

package inet.ansa.linklayer.rbridge;

import inet.linklayer.IWiredNic;
import inet.linklayer.IOutputQueue;
import inet.linklayer.ethernet.EtherEncap;
import inet.linklayer.ethernet.EtherMAC;
import inet.ansa.linklayer.ethernet.AnsaEtherMAC;
//
// TODO auto-generated type
//
module RBEthernetInterface  like IWiredNic
{

    parameters:
        string queueType = default("DropTailQueue");
        @display("i=block/ifcard");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        inout phys;
    submodules:
        queue: <queueType> like IOutputQueue {
            parameters:
                @display("p=92,71;q=l2queue");
        }
        mac: EtherMAC {
            parameters:
                queueModule = "queue";
                txQueueLimit = 1; // queue sends one packet at a time
                @display("p=116,231");
        }
//        encap: EtherEncap {
//            parameters:
//                @display("p=116,151");
//        }
    connections:
        upperLayerIn --> queue.in;
//        queue.out --> encap.upperLayerIn;
//        upperLayerOut <-- { @display("m=n"); } <-- encap.upperLayerOut;
//        encap.lowerLayerOut --> mac.upperLayerIn;
//        encap.lowerLayerIn <-- mac.upperLayerOut;
		queue.out --> mac.upperLayerIn;
		upperLayerOut <-- { @display("m=n"); } <--  mac.upperLayerOut;
        mac.phys <--> phys;
}

