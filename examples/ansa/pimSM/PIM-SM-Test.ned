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
// @file finalTest.ned
// @date 21.10.2011
// @author: Tomas Prochazka (mailto:xproch21@stud.fit.vutbr.cz), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
// @brief Topology with PIM network

package inet.examples.ansa.pimSM;

import inet.ansa.networklayer.pim.PIMRouter;
import inet.ansa.nodes.inet.AnsaHost;
import inet.nodes.ethernet.Eth100M;
import inet.world.scenario.ScenarioManager;

network pimSMTest
{
    @display("bgb=623,419");

    submodules:
        R0: PIMRouter {
            parameters:
                @display("p=86,336");
            gates:
                ethg[3];
        }
        R1: PIMRouter {
            parameters:
                @display("p=86,218");
            gates:
                ethg[3];
        }
        R2_RP: PIMRouter {
            parameters:
                @display("p=410,218");
            gates:
                ethg[3];
        }
        R3: PIMRouter {
            parameters:
                @display("p=246,113");
            gates:
                ethg[3];
        }
        Source1: AnsaHost {
            parameters:
                @display("p=533,113");
            gates:
                ethg[1];
        }
        Host1: AnsaHost {
            parameters:
                @display("p=410,336");
            gates:
                ethg[1];
        }

    connections allowunconnected:
        R1.ethg[2] <--> Eth100M <--> R2_RP.ethg[1];
        R1.ethg[1] <--> Eth100M <--> R3.ethg[1];
        R2_RP.ethg[0] <--> Eth100M <--> R3.ethg[2];
        R3.ethg[0] <--> Eth100M <--> Source1.ethg[0];
        Host1.ethg[0] <--> Eth100M <--> R0.ethg[0];
        R0.ethg[1] <--> Eth100M <--> R1.ethg[0];
}
