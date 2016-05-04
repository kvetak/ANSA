#include "GLBPVirtualForwarder.h"

namespace inet {

std::string GLBPVirtualForwarder::info() const
{
    std::stringstream out;
    if (ipAddr.size() > 0)
    {
        out << " state:" << ((state==INIT) ? "Init" : ((state == LISTEN) ? "Listen" : (state == ACTIVE) ? "Active" : "Disabled"));
        out << " fwd:" << forwarder;
        out << " mac:" << macAddr;
        out << " status:" << ((disable) ? "disable" : "enable");
        out << " ip:";
        for (int i = 0; i < (int) ipAddr.size(); ++i)
            out << (i>0?",":"") << ipAddr[i];
//        out << " primary router:" <<*primaryRouter;
        out << " available:"<<((available) ? "yes" : "no");
    }
    return out.str();
}

}
