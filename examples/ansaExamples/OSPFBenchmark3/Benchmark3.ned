
package inet.examples.ansaExamples.OSPFBenchmark3;

import inet.linklayer.ethernet.EtherHub;
import inet.ansa.ANSARouter;
import inet.nodes.inet.StandardHost;
import inet.world.ChannelInstaller;
import inet.world.ScenarioManager;
import ned.DatarateChannel;


network Benchmark3
{
    parameters:
        @display("p=10,10;b=400,152");
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
        DUT: ANSARouter {
            parameters:
                @display("p=50,110");
            gates:
                ethg[1];
        }
        R1: ANSARouter {
            parameters:
                @display("p=160,110");
            gates:
                ethg[1];
        }
        R2: ANSARouter {
            parameters:
                @display("p=270,110");
            gates:
                ethg[1];
        }
        LAN: EtherHub {
            parameters:
                @display("p=160,230");
            gates:
                ethg[3];
        }
    connections:
        DUT.ethg[0] <--> C <--> LAN.ethg[0];
        R1.ethg[0] <--> C <--> LAN.ethg[1];
        R2.ethg[0] <--> C <--> LAN.ethg[2];
}

