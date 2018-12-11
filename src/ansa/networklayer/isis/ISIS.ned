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

package ansa.networklayer.isis;

module ISIS
{
    parameters:
        @display("i=block/network2;bgb=609,214");
        string deviceId;
        string deviceType;
        xml configFile;
        xml configData = default(xmldoc("config.xml"));
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;

    gates:
        input lowerLayerIn;
        output lowerLayerOut;
        input trillIn;
        output trillOut;

    submodules:
        isisMain: ISISMain {
            interfaceTableModule = interfaceTableModule;
            routingTableModule = routingTableModule;
            configData = configData;
            deviceId = deviceId;
            deviceType = deviceType;
            configFile = configFile;
            @display("p=61,63");
        }

    connections:
        
        lowerLayerIn --> isisMain.lowerLayerIn;
        isisMain.lowerLayerOut --> lowerLayerOut;
        
        trillIn --> isisMain.trillIn;
        isisMain.trillOut --> trillOut;

}
