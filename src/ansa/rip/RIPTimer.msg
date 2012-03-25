
cplusplus {{
#include "IPAddress.h"
}};

//struct IPAddress;
class noncobject IPAddress;

enum RIPTimerType
{
   hello = 1;
   timeout = 2;
   garbage = 3;
   trigger = 4;
};


message RIPTimer extends cMessage
{
      char 		timerKind enum(RIPTimerType) = hello;
      int   	intID;
};