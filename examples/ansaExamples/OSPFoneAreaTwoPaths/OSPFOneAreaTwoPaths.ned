
package inet.examples.ansaExamples.OSPFoneAreaTwoPaths;

import inet.linklayer.ethernet.EtherHub;
import inet.ansa.ANSARouter;
import inet.nodes.inet.StandardHost;
import inet.world.ChannelInstaller;
import inet.world.ScenarioManager;
import ned.DatarateChannel;


network SimpleTest
{
    parameters:
        @display("p=10,10;b=982,152");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
        }
    submodules:
        channelInstaller: ChannelInstaller {
            parameters:
                channelClass = "ThruputMeteringChannel";
                channelAttrs = "format=#N";
                @display("p=60,40");
        }
	     scenarioManager: ScenarioManager {
            parameters:
                @display("p=200,40");
        }
        H1: StandardHost {
            parameters:
                @display("p=56,92;i=device/laptop");
            gates:
                ethg[1];
        }
        N1: EtherHub {
            parameters:
                @display("p=184,92");
            gates:
                ethg[2];
        }
        R3: ANSARouter {
            parameters:
                @display("p=296,92");
            gates:
                ethg[3];
        }
        R1: ANSARouter {
            parameters:
                @display("p=416,50");
            gates:
                ethg[2];
        }
        R2: ANSARouter {
            parameters:
                @display("p=532,50");
            gates:
                ethg[2];
        }
        R5: ANSARouter {
            parameters:
                @display("p=416,140");
            gates:
                ethg[2];
        }
        R6: ANSARouter {
            parameters:
                @display("p=532,140");
            gates:
                ethg[2];
        }
        R4: ANSARouter {
            parameters:
                @display("p=660,92");
            gates:
                ethg[3];
        }
        N2: EtherHub {
            parameters:
                @display("p=790,92");
            gates:
                ethg[2];
        }
        H2: StandardHost {
            parameters:
                @display("p=920,92;i=device/laptop");
            gates:
                ethg[1];
        }
    connections:
        H1.ethg[0] <--> C <--> N1.ethg[0];
        N1.ethg[1] <--> C <--> R3.ethg[0];
        R3.ethg[1] <--> C <--> R1.ethg[0];
        R3.ethg[2] <--> C <--> R5.ethg[0];
        R1.ethg[1] <--> C <--> R2.ethg[0];
        R5.ethg[1] <--> C <--> R6.ethg[0];
        R2.ethg[1] <--> C <--> R4.ethg[0];
        R6.ethg[1] <--> C <--> R4.ethg[2];
        R4.ethg[1] <--> C <--> N2.ethg[0];
        N2.ethg[1] <--> C <--> H2.ethg[0];
}

