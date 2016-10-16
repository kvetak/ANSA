//
// Copyright (C) 2009 - today, Brno University of Technology, Czech Republic
// Copyright (C) 2005 Andras Babos
// Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
//               2010 Zoltan Bojthe
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
// @author Jan Bloudicek 
// @author Vit Rek
// @author Vladimir Vesely (ivesely@fit.vutbr.cz)
// @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3

package ansa.node;

package ansa.node;

import ansa.node.ANSA_Router;

module ANSA_ISISRouter extends ANSA_Router
{
    parameters:
        @capabilities("Router");
        @capabilitiesEnabled("Router");
        
        hasISIS = true;
        enableCLNS = true;
        enableIPv4 = false;
        enableIPv6 = false;
		isis.configData = configData;   
		string deviceId = default("");  
        xml configFile = default(xmldoc("config.xml"));
        string deviceType = default("Router"); 
        **.deviceId = deviceId;
        **.deviceType = deviceType;
        **.configFile = configFile;              
}

