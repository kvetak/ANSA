
package inet.examples.ansaExamples.QosSimpleTest;


import inet.ansa.ANSARouter;
import inet.world.ChannelInstaller;
import ned.DatarateChannel;
import inet.nodes.inet.StandardHost;

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
	     
        R1: ANSARouter {
            parameters:
                @display("p=416,92");
            gates:
                ethg[1];
                pppg[1];
        }
        R2: ANSARouter {
            parameters:
                @display("p=532,92");
            gates:
                ethg[1];
                pppg[1];
        }
        H1: StandardHost {
            parameters:
                @display("p=56,92;i=device/laptop");
            gates:
                ethg[1];
        }
        H2: StandardHost {
            parameters:
                @display("p=920,92;i=device/laptop");
            gates:
                ethg[1];
        }
    connections:
        
        R1.pppg[0] <--> {  delay = 10ms; datarate = 10Kbps; } <--> R2.pppg[0];
        R1.ethg[0] <--> C <--> H1.ethg[0];
        R2.ethg[0] <--> C <--> H2.ethg[0];
        
}

