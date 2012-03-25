// IGMPMessage.msg
// Petr Matelesko
// 5. 5. 2010


cplusplus {{
#include "IPAddress.h"
}};

class noncobject IPAddress;

enum IGMPType
{

    IGMP_MEMBERSHIP_QUERY = 0x11;
    IGMP_MEMBERSHIP_REPORT_V1 = 0x12;
    IGMP_MEMBERSHIP_REPORT_V2 = 0x16;
    IGMP_LEAVE_GROUP = 0x17;
};

//
// IGMP message class
//
packet IGMPMessage
{
    short type enum(IGMPType);
    short maxRespTime;			// jednotka 1/10 s (napr. pro 10 s = 100)
    IPAddress groupAddress;
};
