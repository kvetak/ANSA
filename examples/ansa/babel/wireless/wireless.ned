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

package inet.examples.ansa.babel.wireless;

import inet.ansa.applications.babel.BabelRouter;
import inet.ansa.nodes.ANSA_DualStackHost;
import inet.nodes.ethernet.Eth100M;
import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.world.scenario.ScenarioManager;
import inet.world.radio.ChannelControl;

import inet.nodes.wireless.AccessPoint;

network wireless
{
    @display("bgb=521,431");
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
                @display("p=450,290");
        }

        channelControl: ChannelControl {
            parameters:
                @display("p=60,50");
        }

        ap: AccessPoint {
            @display("p=175,196;r=,,#707070");
           // wlan[*].mgmtType = "Ieee80211MgmtAPSimplified";
        }

        R1: BabelRouter {
            parameters:
                @display("p=175,120");
                numRadios = 1;
            gates:
                ethg[2];
        }
        R2: BabelRouter {
            parameters:
                @display("p=279,207");
                numRadios = 1;
            gates:
                ethg[2];
        }
        R3: BabelRouter {
            parameters:
                @display("p=175,263");
                numRadios = 1;
            gates:
                ethg[2];
        }

        R4: BabelRouter {
            parameters:
                @display("p=175,370");
            gates:
                ethg[2];
        }
        LAN1: ANSA_DualStackHost {
            @display("p=175,51;i=cloud_s");
        }
        LAN2: ANSA_DualStackHost {
            @display("p=377,207;i=cloud_s");
        }
        LAN3: ANSA_DualStackHost {
            @display("p=86,263;i=cloud_s");
        }
        LAN4: ANSA_DualStackHost {
            @display("p=86,370;i=cloud_s");
        }
        scenarioManager: ScenarioManager {
            @display("p=450,357");
        }

    connections allowunconnected:
        R1.ethg[0] <--> Eth100M <--> LAN1.ethg++;
        R2.ethg[0] <--> Eth100M <--> LAN2.ethg++;
        R3.ethg[0] <--> Eth100M <--> LAN3.ethg++;
        R4.ethg[0] <--> Eth100M <--> LAN4.ethg++;

		R1.ethg[1] <--> Eth100M <--> R2.ethg[1];
        R3.ethg[1] <--> Eth100M <--> R4.ethg[1];



}

