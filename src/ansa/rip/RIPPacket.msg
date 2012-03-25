//
// Author: Veronika Rybová
// Contact: xrybov00@stud.fit.vutbr.cz
// Date: 11.5.2009
// Institutuion: Brno University of Technology
// 
// Tento modul vznikl v rámci mé bakaláøské práce.
// This module was created within the scope of my bachelor's thesis.
// 
// Tato tøída implementuje RIP zprávu, kterou využívá modul RIPRouting.
// This class implements RIP message, which is used by module RIPRouting.
//
 
cplusplus {{
#include "IPAddress.h"
}}


//struct IPAddress;
class noncobject IPAddress;

enum RIPCommand
{
    Request = 1;
    Response = 2;
}

struct RouteEntry
{
    short 		addressID;       // 2 pro IP
    short 		mustBeZero2 = 0;
    IPAddress 	ipAddress;
    long 		mustBeZero3 = 0;
    long 		mustBeZero4 = 0;
    long 		metric;
}

message RIPPacket
{
    char 		command enum(RIPCommand);
    char 		version = 1;
    short 		mustBeZero1 = 0;
    RouteEntry 	routeEntry[];
}
