
package inet.examples.ansaExamples.OSPFBenchmark2;

import inet.linklayer.ethernet.EtherHub;
import inet.ansa.ANSARouter;
import inet.nodes.inet.StandardHost;
import inet.world.ChannelInstaller;
import inet.world.ScenarioManager;
import ned.DatarateChannel;


network Benchmark2
{
    parameters:
        @display("p=10,10;b=400,152");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
        }
        channel serialLine extends DatarateChannel
        {
            delay = 1us;
            datarate = 64kbps;
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
                @display("p=50,92");
            gates:
                ethg[1];
                pppg[1];
        }
        Collector: ANSARouter {
            parameters:
                @display("p=50,150");
            gates:
                ethg[2];
        }
        Generator: ANSARouter {
            parameters:
                @display("p=196,92");
            gates:
                ethg[2];
                pppg[1];
        }
        LAN: EtherHub {
            parameters:
                @display("p=350,92");
            gates:
                ethg[1];
        }
    connections:
        DUT.pppg[0] <--> serialLine <--> Generator.pppg[0];
        DUT.ethg[0] <--> C <--> Collector.ethg[0];
        Generator.ethg[0] <--> C <--> Collector.ethg[1];
        Generator.ethg[1] <--> C <--> LAN.ethg[0];
}

