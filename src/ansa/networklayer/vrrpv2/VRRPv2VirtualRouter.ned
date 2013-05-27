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

package inet.ansa.networklayer.vrrpv2;

//
// TODO auto-generated type
//
simple VRRPv2VirtualRouter
{
    parameters:
        @display("i=block/network2");
        @signal[vrState](type=long);  // enum
        @statistic[vrState](title="State ID";source=state;record=count,vector);
        @statistic[sentARP](title="ARP Gratuitous";source=sentARP;record=count,vector);
        @statistic[sentAdvert](title="Advertisement sent";source=sentAdvert;record=count,vector);
        @statistic[recvAdvert](title="Received Advertisement";source=recvAdvert;record=count,vector);
        string hostname; 
        string deviceId;           
    	string configFile;
		int vrid;
		int interface;

		int arp					= default(2);			// 1 = ARP Request (RFC) 2 = ARP Response (Cisco)
		string multicastIPv4	= default("224.0.0.18");
		string virtualMAC		= default("00-00-5E-00-01-00"); // + {VRID}
		int version 			= default(2);			// current version
		int priorityOwner		= default(255);			// priority of the ip owner
		int priorityDefault		= default(100); 		// default priority
		int timeToLive			= default(255);			// time to live
		int ipProtocol			= default(112);			// IP protocol number 			
		int adverDefault @unit(s) = default(1s);		// advertisement interval (in sec)
		double arpDelay @unit(s)= default(0.0000001s);		// master down interval (in sec)
		bool preemptDefault		= default(true);		// preempt mode           
    	bool learnDefault		= default(false);		

	gates:
        input ipIn @labels(IPv4Datagram);
        output ipOut @labels(IPv4Datagram);   
}