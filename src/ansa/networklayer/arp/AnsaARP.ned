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

package inet.ansa.networklayer.arp;

import inet.networklayer.arp.ipv4.ARP;

//simple AnsaARP extends ARP {} 

simple AnsaARP
{
    parameters:
        double retryTimeout @unit("s") = default(1s); // number seconds ARP waits between retries to resolve an IPv4 address
        int retryCount = default(3);   // number of times ARP will attempt to resolve an IPv4 address
        double cacheTimeout @unit("s") = default(120s); // number seconds unused entries in the cache will time out
        bool proxyARP = default(true);        // sets proxy \ARP mode (replying to \ARP requests for the addresses for which a routing table entry exists)
        bool globalARP = default(false);
        @display("i=block/layer");
        @statistic[sentReq](title="ARP request sent";record=count,vector);
        @statistic[sentReplies](title="ARP replies sent";record=count,vector);
        @statistic[initiatedResolution](title="ARP initiated resolutions";record=count,vector);
        @statistic[failedResolution](title="ARP failed resolutions";record=count,vector);
    gates:
        input ipIn @labels(ARPPacket,IPv4Datagram);
        output nicOut[] @labels(ARPPacket);
}