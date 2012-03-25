//
// Marek Cerny, 2MSK
// FIT VUT 2011
//
//
// Marek Cerny, xcerny23@stud.fit.vutbr.cz, 2MSK
// FIT VUT v Brne, 2010
//
// Ucelem tohoto ukazkoveho prikadu je demonstrovat minimalni
// konfiguraci, kterou je treba vyplnit pri praci s upravenymi
// moduly z baliku ANSA.
//
// Detailni komentar ke konfiguraci se nachazi v omnetpp.ini
//

package inet.examples.ansaExamples.cfg6;
import inet.ansa.ANSARouter;

import inet.ansa.AnsaDualStackRouter;
import inet.ansa.AnsaDualStackHost;
import inet.nodes.ethernet.EtherSwitch;

import inet.world.ScenarioManager;

network cfg6
{
    submodules:
        scenarioManager: ScenarioManager {
            @display("p=250,60");
        }
        HostA: AnsaDualStackHost {

            @display("p=32,28");
        }
        HostB: AnsaDualStackHost {
            @display("p=32,117");
        }
        HostC: AnsaDualStackHost;
        Switch: EtherSwitch {
            @display("p=149,59");
        }
        Backup: EtherSwitch {
            @display("p=195,148");
        }
        R1: AnsaDualStackRouter {
            @display("p=110,181");
        }
        R2: AnsaDualStackRouter {
            @display("p=270,181");
        }
    connections allowunconnected:
        R1.ethg++ <--> R2.ethg++;
        R1.ethg++ <--> Backup.ethg++;
        R2.ethg++ <--> Backup.ethg++;
        R2.ethg++ <--> HostC.ethg++;
        Switch.ethg++ <--> R1.ethg++;
        HostA.ethg++ <--> Switch.ethg++;
        HostB.ethg++ <--> Switch.ethg++;
}
