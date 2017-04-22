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

class INET_API RoutingInfo
{
  private:
    std::vector<NextHop> nextHops;
    unsigned long distance;
    OSPFv3LSA *parent;

  public:
    RoutingInfo() : distance(0), parent(nullptr) {}
    RoutingInfo(const RoutingInfo& routingInfo) : nextHops(routingInfo.nextHops), distance(routingInfo.distance), parent(routingInfo.parent) {}
    virtual ~RoutingInfo() {}

    void addNextHop(NextHop nextHop) { nextHops.push_back(nextHop); }
    void clearNextHops() { nextHops.clear(); }
    unsigned int getNextHopCount() const { return nextHops.size(); }
    NextHop getNextHop(unsigned int index) const { return nextHops[index]; }
    void setDistance(unsigned long d) { distance = d; }
    unsigned long getDistance() const { return distance; }
    void setParent(OSPFv3LSA *p) { parent = p; }
    OSPFv3LSA *getParent() const { return parent; }
};

class INET_API OSPFv3RouterNode : public OSPFv3RouterLSA, public LSATrackingInfo, public RoutingInfo
{
  public:
    OSPFv3RouterNode() : OSPFv3RouterLSA(), LSATrackingInfo(), RoutingInfo() {};
    OSPFv3RouterNode(const OSPFv3RouterLSA& lsa) : OSPFv3RouterLSA(lsa), LSATrackingInfo(), RoutingInfo() {};
    OSPFv3RouterNode(const OSPFv3RouterNode& lsa) : OSPFv3RouterLSA(lsa), LSATrackingInfo(lsa), RoutingInfo(lsa) {};
    virtual ~OSPFv3RouterNode(){};

    bool update(const OSPFv3RouterLSA *lsa);
    bool differsFrom(const OSPFv3RouterLSA *routerLSA) const;

    //DEBUG
    std::string dumpRouterLSA() const;
};


class INET_API OSPFv3NetworkNode : public OSPFv3NetworkLSA,
    public RoutingInfo,
    public LSATrackingInfo
{
  public:
    OSPFv3NetworkNode() : OSPFv3NetworkLSA(), RoutingInfo(), LSATrackingInfo() {}
    OSPFv3NetworkNode(const OSPFv3NetworkLSA& lsa) : OSPFv3NetworkLSA(lsa), RoutingInfo(), LSATrackingInfo() {}
    OSPFv3NetworkNode(const OSPFv3NetworkNode& lsa) : OSPFv3NetworkLSA(lsa), RoutingInfo(lsa), LSATrackingInfo(lsa) {}
    virtual ~OSPFv3NetworkNode() {}

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const OSPFv3NetworkLSA *lsa);
    bool differsFrom(const OSPFv3NetworkLSA *networkLSA) const;
};

class INET_API OSPFv3SummaryNode : public OSPFv3InterAreaPrefixLSA,
    public RoutingInfo,
    public LSATrackingInfo
{
  protected:
    bool purgeable;

  public:
    OSPFv3SummaryNode() : OSPFv3InterAreaPrefixLSA(), RoutingInfo(), LSATrackingInfo(), purgeable(false) {}
    OSPFv3SummaryNode(const OSPFv3InterAreaPrefixLSA& lsa) : OSPFv3InterAreaPrefixLSA(lsa), RoutingInfo(), LSATrackingInfo(), purgeable(false) {}
    OSPFv3SummaryNode(const OSPFv3SummaryNode& lsa) : OSPFv3InterAreaPrefixLSA(lsa), RoutingInfo(lsa), LSATrackingInfo(lsa), purgeable(lsa.purgeable) {}
    virtual ~OSPFv3SummaryNode() {}

    bool getPurgeable() const { return purgeable; }
    void setPurgeable(bool purge = true) { purgeable = purge; }

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const OSPFv3InterAreaPrefixLSA *lsa);
    bool differsFrom(const OSPFv3InterAreaPrefixLSA *summaryLSA) const;
};

class INET_API OSPFv3ASExternalNode : public OSPFv3ASExternalLSA,
    public RoutingInfo,
    public LSATrackingInfo
{
  protected:
    bool purgeable;

  public:
    OSPFv3ASExternalNode() : OSPFv3ASExternalLSA(), RoutingInfo(), LSATrackingInfo(), purgeable(false) {}
    OSPFv3ASExternalNode(const OSPFv3ASExternalLSA& lsa) : OSPFv3ASExternalLSA(lsa), RoutingInfo(), LSATrackingInfo(), purgeable(false) {}
    OSPFv3ASExternalNode(const OSPFv3ASExternalNode& lsa) : OSPFv3ASExternalLSA(lsa), RoutingInfo(lsa), LSATrackingInfo(lsa), purgeable(lsa.purgeable) {}
    virtual ~OSPFv3ASExternalNode() {}

    bool getPurgeable() const { return purgeable; }
    void setPurgeable(bool purge = true) { purgeable = purge; }

    bool validateLSChecksum() const { return true; }    // not implemented

    bool update(const OSPFv3ASExternalLSA *lsa);
    bool differsFrom(const OSPFv3ASExternalLSA *asExternalLSA) const;
};

unsigned int calculateLSASize(OSPFv3LSA *routerLSA);
std::ostream& operator<<(std::ostream& ostr, const OSPFv3LSAHeader& lsa);
inline std::ostream& operator<<(std::ostream& ostr, const OSPFv3LSA& lsa) { ostr << lsa.getHeader(); return ostr; }
std::ostream& operator<<(std::ostream& ostr, const OSPFv3NetworkLSA& lsa);
//std::ostream& operator<<(std::ostream& ostr, const TOSData& tos);
//std::ostream& operator<<(std::ostream& ostr, const Link& link);
std::ostream& operator<<(std::ostream& ostr, const OSPFv3RouterLSA& lsa);
std::ostream& operator<<(std::ostream& ostr, const OSPFv3InterAreaPrefixLSA& lsa);
//std::ostream& operator<<(std::ostream& ostr, const ExternalTOSInfo& tos);
std::ostream& operator<<(std::ostream& ostr, const OSPFv3ASExternalLSA& lsa);

}//namespace inet

#endif
