
package inet.examples.ansaExamples.acl;

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
                @display("p=282,92;i=device/laptop");
            gates:
                ethg[1];
        }
        R3: ANSARouter {
            parameters:
                @display("p=474,24");
            gates:
                ethg[2];
        }
        R1: ANSARouter {
            parameters:
                @display("p=395,92");
            gates:
                ethg[3];
        }
        R2: ANSARouter {
            parameters:
                @display("p=532,92");
            gates:
                ethg[3];
        }
        H2: StandardHost {
            parameters:
                @display("p=632,92;i=device/laptop");
            gates:
                ethg[1];
        }
    connections:
        H1.ethg[0] <--> C <--> R1.ethg[0];
        R1.ethg[1] <--> C <--> R3.ethg[0];
        R1.ethg[2] <--> C <--> R2.ethg[0];
        R3.ethg[1] <--> C <--> R2.ethg[1];
        R2.ethg[2] <--> C <--> H2.ethg[0];
}

