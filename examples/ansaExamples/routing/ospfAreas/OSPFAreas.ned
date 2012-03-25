package inet.examples.ansaExamples.routing.ospfAreas;

import inet.nodes.ethernet.EtherSwitch;
import inet.ansa.ANSARouter;
import inet.nodes.inet.StandardHost;
import inet.world.ChannelInstaller;
import ned.DatarateChannel;
import inet.world.ScenarioManager;

network OSPFAreas
{
    parameters:
        @display("p=10,10;b=712,152");
	types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
        }
    submodules:
        channelInstaller: ChannelInstaller
        {
            parameters:
                channelClass = "ThruputMeteringChannel";
                channelAttrs = "format=#N";
            @display("p=242,272;i=block/cogwheel_s");
        }
        scenarioManager: ScenarioManager {
            parameters:
                @display("p=200,40");
        }
        H11: StandardHost
        {
            parameters:
                @display("p=40,52;i=device/pc2");
            gates:
                ethg[1]; 
        }
        S1: EtherSwitch
        {
            parameters:
            	@display("p=168,92;i=device/switch");
            gates:
                ethg[3];
        }
        R1: ANSARouter
        {
            parameters:
            	@display("p=360,92;i=srouter");
            gates:
                ethg[3];
        }
        R1A: ANSARouter
        {
            parameters:
            	@display("p=264,92;i=srouter");
            gates:
                ethg[2];
        }
        R2: ANSARouter
        {
            parameters:
            	@display("p=560,92;i=srouter");
            gates:
                ethg[3];
        }
        R2B: ANSARouter
        {
            parameters:    
            	@display("p=656,92;i=srouter");
            gates:
                ethg[2];
            
        }
        R3: ANSARouter
        {
            parameters:
            	@display("p=460,192;i=srouter");
            gates:
                ethg[3];
        }
        R3C: ANSARouter
        {
            parameters:
            	@display("p=460,272;i=srouter");
            gates:
                ethg[2];
        }
        S2: EtherSwitch
        {
            parameters:
            	@display("p=756,92;i=device/switch");
            gates:
                ethg[3];
        }
        S3: EtherSwitch
        {
            parameters:
            	@display("p=460,358;i=device/switch");
            gates:
                ethg[3];
        }
        H21: StandardHost
        {
            parameters:
            	@display("p=876,52;i=device/pc2");
            gates:
                ethg[1];
        }
        H32: StandardHost
        {
            parameters:
                @display("p=524,462;i=device/pc2");
            gates:
                ethg[1];
        }
        H12: StandardHost
        {
            parameters:
                @display("p=44,142;i=device/pc2");
            gates:
                ethg[1];
        }
        H22: StandardHost
        {
            parameters:
                @display("p=876,150;i=device/pc2");
            gates:
                ethg[1];
        }
        H31: StandardHost
        {
            parameters:
                @display("p=396,462;i=device/pc2");
            gates:
                ethg[1];   
        }
    connections:
        H11.ethg[0] <--> C <--> S1.ethg[1];
        H21.ethg[0] <--> C <--> S2.ethg[1];
        H32.ethg[0] <--> C <--> S3.ethg[1];
        H12.ethg[0] <--> C <--> S1.ethg[2];
        H22.ethg[0] <--> C <--> S2.ethg[2];
        H31.ethg[0] <--> C <--> S3.ethg[2];

        S1.ethg[0] <--> C <--> R1A.ethg[0];
        S2.ethg[0] <--> C <--> R2B.ethg[0];
        S3.ethg[0] <--> C <--> R3C.ethg[0];

        R1.ethg[0] <--> C <--> R1A.ethg[1];
        R1.ethg[1] <--> C <--> R2.ethg[1];
        R1.ethg[2] <--> C <--> R3.ethg[1];
        R2.ethg[2] <--> C <--> R3.ethg[2];
        R2.ethg[0] <--> C <--> R2B.ethg[1];
        R3.ethg[0] <--> C <--> R3C.ethg[1];
}


