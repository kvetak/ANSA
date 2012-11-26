
package inet.examples.ansaExamples.MyQosCaseStudy;

import inet.linklayer.ethernet.EtherHub;
import inet.ansa.ANSARouter;
import inet.ansa.ANSATrafGenHost;
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
        Hub: EtherHub {
            parameters:
                @display("p=250,200");
            gates:
                ethg[6];
        }
        Praha: ANSARouter {
            parameters:
                @display("p=320,200");
            gates:
                ethg[1];
                pppg[2];
        }
        Brno: ANSARouter {
            parameters:
                @display("p=320,70");
            gates:
                pppg[2];
        }
        Ostrava: ANSARouter {
            parameters:
                @display("p=480,70");
            gates:
                ethg[1];
                pppg[2];
        }
        Olomouc: ANSARouter {
            parameters:
                @display("p=480,200");
            gates:
                ethg[1];
                pppg[2];
        }
        Pardubice: ANSARouter {
            parameters:
                @display("p=400,280");
            gates:
                ethg[1];
                pppg[2];
        }
        
        Server1: ANSATrafGenHost {
            parameters:
                @display("p=600,70");
            gates:
                ethg[1];
        }
        
        Host1: ANSATrafGenHost {
            parameters:
                @display("p=150,340;i=device/pc");
            gates:
                ethg[1];
        }
        
        Host2: ANSATrafGenHost {
            parameters:
                @display("p=150,270;i=device/pc");
            gates:
                ethg[1];
        }
        
        Host3: ANSATrafGenHost {
            parameters:
                @display("p=150,200;i=device/pc");
            gates:
                ethg[1];
        }
        
        Phone1: ANSATrafGenHost {
            parameters:
                @display("p=150,100;i=dialphone");
            gates:
                ethg[1];
        }
        
        Phone2: ANSATrafGenHost {
            parameters:
                @display("p=150,150;i=dialphone");
            gates:
                ethg[1];
        }
        
        Phone3: ANSATrafGenHost {
            parameters:
                @display("p=600,200;i=dialphone");
            gates:
                ethg[1];
        }
        
        Phone4: ANSATrafGenHost {
            parameters:
                @display("p=400,370;i=dialphone");
            gates:
                ethg[1];
        }
        

    connections:

        Praha.pppg[0] <--> {  delay = 10ms; datarate = 2Mbps; } <--> Brno.pppg[0];
        Brno.pppg[1] <--> {  delay = 10ms; datarate = 2Mbps; } <--> Ostrava.pppg[0];
        Ostrava.pppg[1] <--> {  delay = 10ms; datarate = 512Kbps; } <--> Olomouc.pppg[1];
        Olomouc.pppg[0] <--> {  delay = 10ms; datarate = 512Kbps; } <--> Pardubice.pppg[0];
        Praha.pppg[1] <--> {  delay = 10ms; datarate = 512Kbps; } <--> Pardubice.pppg[1];
        Praha.ethg[0] <--> C <--> Hub.ethg[0];
        Host1.ethg[0] <--> C <--> Hub.ethg[1];
        Host2.ethg[0] <--> C <--> Hub.ethg[2];
        Host3.ethg[0] <--> C <--> Hub.ethg[3];
        Phone1.ethg[0] <--> C <--> Hub.ethg[4];
        Phone2.ethg[0] <--> C <--> Hub.ethg[5];
        Phone3.ethg[0] <--> C <--> Olomouc.ethg[0];
        Phone4.ethg[0] <--> C <--> Pardubice.ethg[0];
        Server1.ethg[0] <--> C <--> Ostrava.ethg[0];

}
