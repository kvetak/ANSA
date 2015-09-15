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

// @file ANSAStandardHost.ned
// @date 21.5.2013
// @author Jiri Trhlik (mailto:jiritm@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
// @details Extended StandardHost with the ANSA device configurator.

package inet.ansa.nodes.inet;

import inet.ansa.util.IPGen.AnsaIPTrafGen;
import inet.node.inet.StandardHost;
import inet.ansa.util.deviceConfigurator.DeviceConfigurator;

//
// TODO auto-generated type
//
module ANSAStandardHost extends StandardHost
{
    parameters:
        @node;
        @labels(node,ethernet-node);
        string configFile = default("config.xml");           // file name of the XML file which contains configuration for the host
        string deviceId;                                     // needed for proper load of the device XML configuration
    submodules:

        deviceConfigurator: DeviceConfigurator {
            deviceId = deviceId;
            deviceType = "Host";
            configFile = configFile;
            @display("p=61,54");
        }

        ipTrafGen: AnsaIPTrafGen {
            parameters:
                @display("p=580,287;i=block/circle");
        }

    connections allowunconnected:
        ipTrafGen.ipOut --> networkLayer.rsvpIn;
        ipTrafGen.ipIn <-- networkLayer.rsvpOut;
}
