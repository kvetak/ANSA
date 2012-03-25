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

package inet.examples.ansaExamples.SimpleTest;

import inet.ansa.AnsaDualStackRouter;
import inet.ansa.AnsaDualStackHost;

network SimpleTest {
    parameters:
        string configFile;
    gates:
        inout g1;
	submodules:
	    Host: AnsaDualStackHost{
	        configFile = configFile;
	    }
        Router: AnsaDualStackRouter;
    connections allowunconnected:
        Host.ethg++ <--> Router.ethg++;
}
