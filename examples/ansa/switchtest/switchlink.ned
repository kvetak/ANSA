

package inet.examples.ansa.switchtest;


import inet.ansa.ANSASwitch;
import inet.nodes.inet.StandardHost;
//import inet.world.ChannelInstaller;
import inet.world.scenario.ScenarioManager;
import ned.DatarateChannel;

network switchtest2
{
    parameters:
        @display("p=10,10;b=712,152");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
        }
    submodules:

        scenarioManager: ScenarioManager {
            parameters:
                @display("p=402,33");
        }

        // Hosts
        Host1: StandardHost {
            parameters:
                @display("p=45,151;i=device/pc2");
            gates:
                ethg[1];
        }
        ansaSwitch: ANSASwitch {
            @display("p=174,56");
        }
        ansaSwitch1: ANSASwitch {
            @display("p=272,56");
        }
        Host2: StandardHost {
            parameters:
            	@display("p=394,151");
            gates:
                ethg[1];
        }
    connections:

        Host1.ethg[0] <--> ansaSwitch.ethg++;
        Host2.ethg[0] <--> ansaSwitch1.ethg++;
        ansaSwitch.ethg++ <--> ansaSwitch1.ethg++;
}



