

package inet.examples.ansa.switchtest;


import inet.ansa.ANSASwitch;
import inet.nodes.inet.StandardHost;
//import inet.world.ChannelInstaller;
//import inet.world.ScenarioManager;
import ned.DatarateChannel;

network basic_switching
{
    parameters:
        @display("p=10,10;b=712,152;bgb=230,161");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
        }
    submodules:

        // Switch
        Switch1: ANSASwitch {
            parameters:
                @display("p=117,33");
            gates:
                ethg[4];
        }

        // Hosts
        Host1: StandardHost {
            parameters:
                @display("p=33,115;i=device/pc2");
            gates:
                ethg[1];
        }
        Host2: StandardHost {
            parameters:
                @display("p=85,115;i=device/pc2");
            gates:
                ethg[1];
        }
        Host3: StandardHost {
            parameters:
                @display("p=138,115;i=device/pc2");
            gates:
                ethg[1];
        }
        Host4: StandardHost {
            parameters:
                @display("p=190,115;i=device/pc2");
            gates:
                ethg[1];
        }
    connections:

        Host1.ethg[0] <--> Switch1.ethg[0];
        Host2.ethg[0] <--> Switch1.ethg[1];
        Host3.ethg[0] <--> Switch1.ethg[2];
        Host4.ethg[0] <--> Switch1.ethg[3];
}



