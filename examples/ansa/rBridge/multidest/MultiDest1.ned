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

package inet.examples.ansa.rBridge.multidest;

import ansa.node.ANSA_RBridge;
import ansa.node.ANSA_Host;
import inet.node.ethernet.Eth100M;
//import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;

//
// TODO Auto-generated network
//
network MultiDest1 extends MultiDest_Base
{
    @display("bgb=509,354");
    
//    configurator.config = xml("");
    submodules:
                configurator: IPv4NetworkConfigurator {
            parameters:
                config = xmldoc("./networkConfig.xml");
                assignAddresses = false;
//                assignDisjunctSubnetAddresses = false;
//                addStaticRoutes = false;
//                addDefaultRoutes = false;
//                addSubnetRoutes = false;
//                optimizeRoutes = false;
                addStaticRoutes = false;
                addDefaultRoutes = false;
                addSubnetRoutes = false;
                optimizeRoutes = false;
                dumpRoutes = true;

                @display("p=42,305");
        }
        ansaHost2: ANSA_Host {
            @display("p=113,26");
            gates:
                ethg[1];
        }
        ansaHost1: ANSA_Host {
            @display("p=39,90");
            gates:
                ethg[1];
        }
        
//        configurator:
//        parameters:
//        config = xml("<config>"+
//                            "<interface hosts='ansaHost1' names='eth0' address='192.168.1.1' netmask='255.255.255.0' />"+
//                            "<interface hosts='ansaHost2' names='eth0' address='192.168.1.2' netmask='255.255.255.0' />"+
//                            "</config>");
    connections:
       
        ansaHost1.ethg[0] <--> Eth100M <--> rBridge1.ethg[2];
        ansaHost2.ethg[0] <--> Eth100M <--> rBridge1.ethg[3];
}
