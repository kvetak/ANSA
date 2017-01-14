#ifndef __ANSA_OSPFV3LSA_H_
#define __ANSA_OSPFV3LSA_H_

#include <omnetpp.h>
#include "inet/common/INETDefs.h"
#include "ansa/routing/ospfv3/OSPFv3Packet_m.h"
#include "ansa/routing/ospfv3/OSPFv3Common.h"

namespace inet {

class INET_API LSATrackingInfo
{
  public:
    enum InstallSource {
        ORIGINATED = 0,
        FLOODED = 1
    };

  private:
    InstallSource source;
    unsigned long installTime;

  public:
    LSATrackingInfo() : source(FLOODED), installTime(0) {}
    LSATrackingInfo(const LSATrackingInfo& info) : source(info.source), installTime(info.installTime) {}

    void setSource(InstallSource installSource) { source = installSource; }
    InstallSource getSource() const { return source; }
    void incrementInstallTime() { installTime++; }
    void resetInstallTime() { installTime = 0; }
    unsigned long getInstallTime() const { return installTime; }
};

class INET_API OSPFv3RouterLinkState : public OSPFv3RouterLSA, public LSATrackingInfo
{
  public:
    OSPFv3RouterLinkState() : OSPFv3RouterLSA(), LSATrackingInfo() {};
    OSPFv3RouterLinkState(const OSPFv3RouterLSA& lsa) : OSPFv3RouterLSA(lsa), LSATrackingInfo() {};
    std::string dumpRouterLSA() const;
    bool update(const OSPFv3RouterLSA *lsa);
    bool differsFrom(const OSPFv3RouterLSA *routerLSA) const;
};

unsigned int calculateLSASize(OSPFv3LSA *routerLSA);

}//namespace inet

#endif
