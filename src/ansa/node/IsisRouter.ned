//
// Copyright (C) Marcel Marek
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

package ansa.node;

import ansa.networklayer.isis.ISIS;
import ansa.networklayer.clns.CLNSNetworkLayer;
import inet.node.inet.Router;


//
// TODO auto-generated type
//
module IsisRouter extends Router
{
    parameters:
        @capabilities("Router");
        @capabilitiesEnabled("Router");

        bool hasIsis = true;
        bool hasClns = true;
        xml configData = default(xmldoc("config.xml"));

        hasIpv4 = false;
        *.enableCLNS = hasClns;



        //        hasISIS = true;
        //        enableCLNS = true;
        //        enableIPv4 = false;
        //        enableIPv6 = false;
        isis.configData = configData;
        string deviceId = default("");
        xml configFile = default(xmldoc("config.xml"));
        string deviceType = default("Router");
        **.deviceId = deviceId;
        **.deviceType = deviceType;
        **.configFile = configFile;
        *.routingTableModule = default(absPath(".clns.routingTable"));
//        clns.routingTableModule = default(absPath(routingTableModule) + ".clns");


    submodules:
        isis: ISIS if hasIsis {
            @display("p=953,376");
        }

        clns: CLNSNetworkLayer if hasClns {
        	@display("p=823,376");
        }

    connections allowunconnected:

        isis.lowerLayerOut --> nl.in++ if hasIsis;
        isis.lowerLayerIn <-- nl.out++ if hasIsis;

        clns.ifIn <-- nl.out++ if hasClns;
        clns.ifOut --> nl.in++ if hasClns;
}
