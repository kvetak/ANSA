
package inet.examples.ansaExamples.MyOSPFCaseStudy;

import inet.linklayer.ethernet.EtherHub;
import inet.ansa.ANSARouter;
import inet.nodes.inet.StandardHost;
import inet.world.ChannelInstaller;
import inet.world.ScenarioManager;
import ned.DatarateChannel;

network CaseStudyNetwork
{
    parameters:
        @display("p=10,10;b=850,360;bgl=24;bgb=746,424");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            per = 0.0;
        }

    submodules:
        channelInstaller: ChannelInstaller {
            parameters:
                channelClass = "ThruputMeteringChannel";
                channelAttrs = "format=#N";
                @display("p=53,35");
        }
        scenarioManager: ScenarioManager {
            parameters:
                @display("p=53,90");
        }
        N1: EtherHub {
            parameters:
                @display("p=194,308");
            gates:
                ethg[3];
        }
        R01: ANSARouter {
            parameters:
                @display("p=342,202");
            gates:
                ethg[3];
        }
        R02: ANSARouter {
            parameters:
                @display("p=437,202");
            gates:
                ethg[3];
        }
        R03: ANSARouter {
            parameters:
                @display("p=272,308");
            gates:
                ethg[4];
        }
        R04: ANSARouter {
            parameters:
                @display("p=502,308");
            gates:
                ethg[5];
        }
        R13: ANSARouter {
            parameters:
                @display("p=342,120");
            gates:
                ethg[3];
        }
        R14: ANSARouter {
            parameters:
                @display("p=437,120");
            gates:
                ethg[3];
        }
        R21: ANSARouter {
            parameters:
                @display("p=132,256");
            gates:
                ethg[2];
        }
        R22: ANSARouter {
            parameters:
                @display("p=132,354");
            gates:
                ethg[2];
        }
        R31: ANSARouter {
            parameters:
                @display("p=597,256");
            gates:
                ethg[3];
        }
        R32: ANSARouter {
            parameters:
                @display("p=597,348");
            gates:
                ethg[3];
        }
        H13: StandardHost {
            parameters:
                @display("i=device/laptop;p=437,50");
            gates:
                ethg[1];
        }
        H14: StandardHost {
            parameters:
                @display("i=device/laptop;p=342,50");
            gates:
                ethg[1];
        }
        H21: StandardHost {
            parameters:
                @display("i=device/laptop;p=54,256");
            gates:
                ethg[1];
        }
        H22: StandardHost {
            parameters:
                @display("i=device/laptop;p=54,354");
            gates:
                ethg[1];
        }
        H31: StandardHost {
            parameters:
                @display("i=device/laptop;p=677,256");
            gates:
                ethg[1];
        }
        H32: StandardHost {
            parameters:
                @display("i=device/laptop;p=677,348");
            gates:
                ethg[1];
        }

    connections:

        R01.ethg[0] <--> C <--> R03.ethg[0];
        R01.ethg[1] <--> C <--> R04.ethg[0];
        R02.ethg[0] <--> C <--> R03.ethg[1];
        R02.ethg[1] <--> C <--> R04.ethg[1];
        R03.ethg[2] <--> C <--> R04.ethg[2];
        R01.ethg[2] <--> C <--> R13.ethg[0];
        R02.ethg[2] <--> C <--> R14.ethg[0];
        R13.ethg[1] <--> C <--> R14.ethg[1];
        R13.ethg[2] <--> C <--> H14.ethg[0];
        R14.ethg[2] <--> C <--> H13.ethg[0];
        R04.ethg[3] <--> C <--> R31.ethg[0];
        R04.ethg[4] <--> C <--> R32.ethg[0];
        R31.ethg[1] <--> C <--> R32.ethg[1];
        R32.ethg[2] <--> C <--> H32.ethg[0];
        R31.ethg[2] <--> C <--> H31.ethg[0];
        R03.ethg[3] <--> C <--> N1.ethg[0];
        N1.ethg[1] <--> C <--> R22.ethg[0];
        N1.ethg[2] <--> C <--> R21.ethg[0];
        R21.ethg[1] <--> C <--> H21.ethg[0];
        R22.ethg[1] <--> C <--> H22.ethg[0];
}
