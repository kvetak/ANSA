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

class INET_API OSPFv3SPFVertex
{
  private:
    VertexID vertexID;
    OSPFv3LSA* asocLSA;
    int distance; //link state cost of the current set of shortest paths from the root
    uint16_t type; //router or network lsa
    OSPFv3SPFVertex* parent = nullptr;


  public:
    OSPFv3SPFVertex(OSPFv3LSA* asocLSA, int distance);



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
