// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
// @file PIM-SM-Scen3.ned
// @date 18.2.2013
// @author: Tomas Prochazka (mailto:xproch21@stud.fit.vutbr.cz), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
// @brief Topology with PIM network

package inet.examples.ansa.pimSM.scenario3;

import inet.ansa.networklayer.pim.PIMRouter;
import inet.ansa.nodes.inet.AnsaHost;
import inet.ansa.nodes.inet.ANSAStandardHost;
import inet.nodes.ethernet.Eth100M;
import inet.world.scenario.ScenarioManager;
import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;

network pimSMScenario3
{
    @display("bgb=623,419");

	submodules:
        configurator: IPv4NetworkConfigurator {
            parameters:
                config = xml("<config></config>");
                assignAddresses = false;
                assignDisjunctSubnetAddresses = false;
                addStaticRoutes = false;
                addDefaultRoutes = false;
                addSubnetRoutes = false;
                optimizeRoutes = false;
                @display("p=39,29");
        }
        DR_R1: PIMRouter {
            parameters:
                @display("p=86,336");
            gates:
                ethg[3];
        }
        DR_R2: PIMRouter {
            parameters:
                @display("p=86,218");
            gates:
                ethg[4];
        }
        RP: PIMRouter {
            parameters:
                @display("p=410,218");
            gates:
                ethg[3];
        }
        DR_S2: PIMRouter {
            parameters:
                @display("p=246,113");
            gates:
                ethg[3];
        }
        DR_S1: PIMRouter {
            parameters:
                @display("p=399,25");
            gates:
                ethg[3];
        }
        Source1: ANSAStandardHost {
            parameters:
                @display("i=device/server;p=543,113");
            gates:
                ethg[1];
        }
        Receiver1: ANSAStandardHost {
            parameters:
                @display("p=410,336");
            gates:
                ethg[1];
        }
        Receiver2: ANSAStandardHost {
            parameters:
                @display("p=86,59");
            gates:
                ethg[1];
        }


    connections allowunconnected:
        DR_R2.ethg[2] <--> Eth100M <--> RP.ethg[1];
        DR_R2.ethg[1] <--> Eth100M <--> DR_S2.ethg[1];
        RP.ethg[0] <--> Eth100M <--> DR_S2.ethg[2];
        DR_S2.ethg[0] <--> Eth100M <--> DR_S1.ethg[1];
        DR_S1.ethg[0] <--> Eth100M <--> Source1.ethg[0];
        DR_R1.ethg[1] <--> Eth100M <--> DR_R2.ethg[0];
        Receiver1.ethg[0] <--> Eth100M <--> DR_R1.ethg[0];
        Receiver2.ethg[0] <--> Eth100M <--> DR_R2.ethg[3];
}
