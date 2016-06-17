//
// Copyright (C) 2013 Opensim Ltd.
// Author: Levente Meszaros
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package ansa.networklayer.multi;

import ansa.linklayer.cdp.CDP;
import ansa.linklayer.lldp.LLDP;
import inet.networklayer.contract.INetworkLayerLower;
import inet.networklayer.contract.INetworkLayer;
import inet.networklayer.multi.MultiNetworkLayerUpperMultiplexer;
import inet.networklayer.multi.MultiNetworkLayerLowerMultiplexer;

//
// This module provides network protocol specific multiplexing.
// It's part of MultiNetworkLayer and it's meant to be used there.
//
simple ANSA_MultiNetworkLayerUpperMultiplexer 
{
    parameters:
        @display("i=block/fork");

    gates:
        input transportUpperIn[];
        output transportUpperOut[];
        input transportLowerIn[];
        output transportLowerOut[];
        input pingUpperIn[];
        output pingUpperOut[];
        input pingLowerIn[];
        output pingLowerOut[];    
}

//
// This module provides network protocol specific multiplexing.
// It's part of MultiNetworkLayer and it's meant to be used there.
//
simple ANSA_MultiNetworkLayerLowerMultiplexer 
{
    parameters:
        @display("i=block/fork");

    gates:
        input ifUpperIn[] @labels(INetworkDatagram);
        output ifUpperOut[] @labels(INetworkDatagram);
        input ifLowerIn[] @labels(INetworkDatagram);
        output ifLowerOut[] @labels(INetworkDatagram);    
}

//
// This module supports multiple different network layers simultaneously.
// Packets received through the transport gates are handed over to the
// corresponding network layer based on the attached control info.
// Packets received from the interface gates are also handed over to the
// corresponding network layer based on their runtime type.
//
module ANSA_MultiNetworkLayer like INetworkLayer
{
    parameters:
        @display("i=block/fork");
        bool enableIPv4 = default(haveClass("inet::IPv4RoutingTable"));
        bool enableIPv6 = default(haveClass("inet::IPv6RoutingTable"));
        bool enableCLNS = default(haveClass("inet::GenericRoutingTable"));
        bool enableCDP = default(false);
        bool enableLLDP = default(false);
        string interfaceTableModule;
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        string routingTableModule;
        ipv4.routingTableModule = default(absPath(routingTableModule) + ".ipv4");
        ipv6.routingTableModule = default(absPath(routingTableModule) + ".ipv6");
        clns.routingTableModule = default(absPath(routingTableModule) + ".clns");
        cdp.routingTableModule = default(absPath(routingTableModule) + ".ipv4");
        lldp.routingTableModule = default(absPath(routingTableModule) + ".ipv4");

    gates:
        input ifIn[] @labels(INetworkDatagram);
        output ifOut[] @labels(INetworkDatagram);
        input transportIn[] @labels(IPv4ControlInfo/down);
        output transportOut[] @labels(IPv4ControlInfo/up);
        input pingIn[] @labels(PingPayload/down);
        output pingOut[] @labels(PingPayload/up);

    submodules:
        ipv4: <"IPv4NetworkLayer"> like INetworkLayer if enableIPv4 {
            @display("p=100,300");
        }
        ipv6: <"IPv6NetworkLayer"> like INetworkLayer if enableIPv6 {
            @display("p=300,300");
        }
        clns: <"GenericNetworkLayer"> like INetworkLayer if enableCLNS {
            @display("p=500,300");
        }
        upperMultiplexer: ANSA_MultiNetworkLayerUpperMultiplexer {
            @display("p=300,100");
            gates:
                transportLowerIn[3*sizeof(transportOut)];
                transportLowerOut[3*sizeof(transportIn)];
                pingLowerIn[3*sizeof(pingOut)];
                pingLowerOut[3*sizeof(pingIn)];
        }
        lowerMultiplexer: ANSA_MultiNetworkLayerLowerMultiplexer {
            @display("p=300,500");
            gates:
                ifUpperOut[5*sizeof(ifIn)];
                ifUpperIn[5*sizeof(ifOut)];
        }

        cdp: <"CDP"> like INetworkLayerLower if enableCDP {
            @display("p=461,500");
        }

        lldp: <"LLDP"> like INetworkLayerLower if enableLLDP {
            @display("p=461,569");
        }
    connections allowunconnected:
        // transport
        for i=0..sizeof(transportIn)-1 {
            upperMultiplexer.transportLowerOut[0+3*i] --> ipv4.transportIn++ if enableIPv4;
            upperMultiplexer.transportLowerOut[1+3*i] --> ipv6.transportIn++ if enableIPv6;
            upperMultiplexer.transportLowerOut[2+3*i] --> clns.transportIn++ if enableCLNS;
            transportIn[i] --> { @display("m=n"); } --> upperMultiplexer.transportUpperIn++;
        }
        for i=0..sizeof(transportOut)-1 {
            ipv4.transportOut++ --> upperMultiplexer.transportLowerIn[0+3*i] if enableIPv4;
            ipv6.transportOut++ --> upperMultiplexer.transportLowerIn[1+3*i] if enableIPv6;
            clns.transportOut++ --> upperMultiplexer.transportLowerIn[2+3*i] if enableCLNS;
            upperMultiplexer.transportUpperOut++ --> { @display("m=n"); } --> transportOut[i];
        }

        // ping
        for i=0..sizeof(pingIn)-1 {
            upperMultiplexer.pingLowerOut[0+3*i] --> ipv4.pingIn++ if enableIPv4;
            upperMultiplexer.pingLowerOut[1+3*i] --> ipv6.pingIn++ if enableIPv6;
            upperMultiplexer.pingLowerOut[2+3*i] --> clns.pingIn++ if enableCLNS;
            pingIn[i] --> { @display("m=n"); } --> upperMultiplexer.pingUpperIn++;
        }
        for i=0..sizeof(pingOut)-1 {
            ipv4.pingOut++ --> upperMultiplexer.pingLowerIn[0+3*i] if enableIPv4;
            ipv6.pingOut++ --> upperMultiplexer.pingLowerIn[1+3*i] if enableIPv6;
            clns.pingOut++ --> upperMultiplexer.pingLowerIn[2+3*i] if enableCLNS;
            upperMultiplexer.pingUpperOut++ --> { @display("m=n"); } --> pingOut[i];
        }

        // interface
        for i=0..sizeof(ifIn)-1 {
            lowerMultiplexer.ifUpperOut[0+5*i] --> ipv4.ifIn++ if enableIPv4;
            lowerMultiplexer.ifUpperOut[1+5*i] --> ipv6.ifIn++ if enableIPv6;
            lowerMultiplexer.ifUpperOut[2+5*i] --> clns.ifIn++ if enableCLNS;
            lowerMultiplexer.ifUpperOut[3+5*i] --> cdp.ifIn++ if enableCDP;
            lowerMultiplexer.ifUpperOut[4+5*i] --> lldp.ifIn++ if enableLLDP;
            ifIn[i] --> { @display("m=s"); } --> lowerMultiplexer.ifLowerIn++;
        }
        for i=0..sizeof(ifOut)-1 {
            ipv4.ifOut++ --> lowerMultiplexer.ifUpperIn[0+5*i] if enableIPv4;
            ipv6.ifOut++ --> lowerMultiplexer.ifUpperIn[1+5*i] if enableIPv6;
            clns.ifOut++ --> lowerMultiplexer.ifUpperIn[2+5*i] if enableCLNS;
            cdp.ifOut++ --> lowerMultiplexer.ifUpperIn[3+5*i] if enableCDP;
            lldp.ifOut++ --> lowerMultiplexer.ifUpperIn[4+5*i] if enableLLDP;
            lowerMultiplexer.ifLowerOut++ --> { @display("m=s"); } --> ifOut[i];
        }
}
