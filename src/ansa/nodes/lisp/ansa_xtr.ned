//
// Copyright (C) 2013 Brno University of Technology
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
//@author Vladimir Vesely (<a href="mailto:ivesely@fit.vutbr.cz">ivesely@fit.vutbr.cz</a>)

package inet.ansa.nodes.lisp;

import inet.ansa.applications.rip.RIPRouting;
import inet.ansa.applications.ripng.RIPngRouting;
import inet.ansa.networklayer.ipSplitter.IpSplitter;
import inet.ansa.networklayer.lisp.LISPRouting;
import inet.ansa.util.InterfaceStateManager.InterfaceStateManager;
import inet.transport.IUDP;
import inet.util.PcapRecorder;
import inet.networklayer.ipv4.RoutingTable;
import inet.networklayer.common.InterfaceTable;
import inet.mobility.IMobility;
import inet.linklayer.IWirelessNic;
import inet.linklayer.IWiredNic;
import inet.linklayer.IExternalNic;
import inet.base.NotificationBoard;
import inet.nodes.inet.NetworkLayer;
import inet.nodes.ipv6.NetworkLayer6;
import inet.ansa.nodes.ANSA_RouterRIPDual;

module ANSA_xTR extends ANSA_RouterRIPDual
{
    @display("i2=status/busy;bgb=626,569");
    submodules:
        LISP: LISPRouting {
            @display("p=429,311");
            deviceId = deviceId;
            deviceType = "Router";           
        }
    connections:
        UDP.appOut++ --> LISP.udpIn;
        UDP.appIn++ <-- LISP.udpOut;
        ipSplitter.lispIn <-- LISP.ipsplitOut;
}
