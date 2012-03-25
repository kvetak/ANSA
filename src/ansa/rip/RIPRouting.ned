//
// Author: Veronika Rybová
// Contact: xrybov00@stud.fit.vutbr.cz
// Date: 11.5.2009
// Institutuion: Brno University of Technology
// 
// Tento modul vznikl v rámci mé bakaláøské práce.
// This module was created within the scope of my bachelor's thesis.
// 
// Modul RIPRouting v sobì implementuje protokol RIP a redistribuci
// z protokolu OSPF do protokolu RIP.
// Module RIPRouting implements protocol RIP and redistribution from
// protocol OSPF to protocol RIP.
// 
// Zde uvedené parametry je, pro správnou funkènost modulu RIPRouting,
// zadat do omnetpp.ini souboru ke každé simulaci.
// 


package inet.ansa.rip;

simple RIPRouting
{
    parameters:
       int localPort = default(520);
       int destPort = default(520);
       string configFile;
       string hostname;
       @display("i=block/network2");  
    gates:
        input udpIn;
        output udpOut;
}