// IGMPTimer.msg
// Petr Matelesko
// 5. 5. 2010

cplusplus {{
#include "IPAddress.h"
}};

class noncobject IPAddress;

enum IGMPTimerType
{
   TIMER_INIT_GENERAL_QUERY = 1;
   TIMER_GENERAL_QUERY = 2;
   TIMER_OTHER_QUERIER_PRESENT = 3;
   TIMER_GROUP_MEMBERSHIP = 4;
   TIMER_LAST_MEMBER = 5;
};


message IGMPTimer extends cMessage
{
      char	timerKind enum(IGMPTimerType);
      int	count;
      int 	intID;
      IPAddress groupAddress;
};
