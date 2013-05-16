
#ifndef __INET_IGMP_H
#define __INET_IGMP_H

#include "INETDefs.h"
#include "INotifiable.h"
#include "IPv6Address.h"
#include "MLDMessage_m.h"
#include "InterfaceEntry.h"

#include <set>

class IInterfaceTable;
class RoutingTable6;
class NotificationBoard;

class INET_API MLD : public cSimpleModule, protected INotifiable
{
  protected:
    enum RouterState
    {
        MLD_RS_INITIAL,
        MLD_RS_QUERIER,
        MLD_RS_NON_QUERIER,
    };

    enum RouterGroupState
    {
        MLD_RGS_NO_MEMBERS_PRESENT,
        MLD_RGS_MEMBERS_PRESENT,
        MLD_RGS_V1_MEMBERS_PRESENT,
        MLD_RGS_CHECKING_MEMBERSHIP,
    };

    enum HostGroupState
    {
        MLD_HGS_NON_MEMBER,
        MLD_HGS_DELAYING_MEMBER,
        MLD_HGS_IDLE_MEMBER,
    };

    struct HostGroupData
    {
        MLD *owner;
        IPv6Address groupAddr;
        HostGroupState state;
        bool flag;                // true when we were the last host to send a report for this group
        cMessage *timer;

        HostGroupData(MLD *owner, const IPv6Address &group);
        virtual ~HostGroupData();
    };
    typedef std::map<IPv6Address, HostGroupData*> GroupToHostDataMap;

    struct RouterGroupData
    {
        MLD *owner;
        IPv6Address groupAddr;
        RouterGroupState state;
        cMessage *timer;
        cMessage *rexmtTimer;
        //cMessage *v1HostTimer;

        RouterGroupData(MLD *owner, const IPv6Address &group);
        virtual ~RouterGroupData();
    };
    typedef std::map<IPv6Address, RouterGroupData*> GroupToRouterDataMap;

    struct HostInterfaceData
    {
        MLD *owner;
        GroupToHostDataMap groups;

        HostInterfaceData(MLD *owner);
        virtual ~HostInterfaceData();
    };
    typedef std::map<int, HostInterfaceData*> InterfaceToHostDataMap;

    struct RouterInterfaceData
    {
        MLD *owner;
        GroupToRouterDataMap groups;
        RouterState mldRouterState;
        cMessage *mldQueryTimer;

        RouterInterfaceData(MLD *owner);
        virtual ~RouterInterfaceData();
    };
    typedef std::map<int, RouterInterfaceData*> InterfaceToRouterDataMap;

    // Timers
    enum MLDTimerKind
    {
        MLD_QUERY_TIMER,
        MLD_HOSTGROUP_TIMER,
        MLD_LEAVE_TIMER,
        MLD_REXMT_TIMER
    };

    struct MLDHostTimerContext
    {
        InterfaceEntry *ie;
        HostGroupData *hostGroup;
        MLDHostTimerContext(InterfaceEntry *ie, HostGroupData *hostGroup) : ie(ie), hostGroup(hostGroup) {}
    };

    struct MLDRouterTimerContext
    {
        InterfaceEntry *ie;
        RouterGroupData *routerGroup;
        MLDRouterTimerContext(InterfaceEntry *ie, RouterGroupData *routerGroup) : ie(ie), routerGroup(routerGroup) {}
    };

  protected:
    RoutingTable6 *rt;     // cached pointer
    IInterfaceTable *ift;  // cached pointer
    NotificationBoard *nb; // cached pointer

    bool enabled;
    bool externalRouter;
    int robustness;                          // RFC 2236: Section 8.1
    double queryInterval;                    // RFC 2236: Section 8.2
    double queryResponseInterval;            // RFC 2236: Section 8.3
    double groupMembershipInterval;          // RFC 2236: Section 8.4
    double otherQuerierPresentInterval;      // RFC 2236: Section 8.5
    double startupQueryInterval;             // RFC 2236: Section 8.6
    double startupQueryCount;                // RFC 2236: Section 8.7
    double lastMemberQueryInterval;          // RFC 2236: Section 8.8
    double lastMemberQueryCount;             // RFC 2236: Section 8.9
    double unsolicitedReportInterval;        // RFC 2236: Section 8.10
    //double version1RouterPresentInterval;  // RFC 2236: Section 8.11

    // state variables per interface
    InterfaceToHostDataMap hostData;
    InterfaceToRouterDataMap routerData;

    // group counters
    int numGroups;
    int numHostGroups;
    int numRouterGroups;

    // message counters
    int numQueriesSent;
    int numQueriesRecv;
    int numGeneralQueriesSent;
    int numGeneralQueriesRecv;
    int numGroupSpecificQueriesSent;
    int numGroupSpecificQueriesRecv;
    int numReportsSent;
    int numReportsRecv;
    int numLeavesSent;
    int numLeavesRecv;

  protected:
    virtual int numInitStages() const  {return 6;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual void receiveChangeNotification(int category, const cPolymorphic *details);
    virtual ~MLD();

  protected:
    virtual HostInterfaceData *createHostInterfaceData();
    virtual RouterInterfaceData *createRouterInterfaceData();
    virtual HostGroupData *createHostGroupData(InterfaceEntry *ie, const IPv6Address &group);
    virtual RouterGroupData *createRouterGroupData(InterfaceEntry *ie, const IPv6Address &group);
    virtual HostInterfaceData *getHostInterfaceData(InterfaceEntry *ie);
    virtual RouterInterfaceData *getRouterInterfaceData(InterfaceEntry *ie);
    virtual HostGroupData *getHostGroupData(InterfaceEntry *ie, const IPv6Address &group);
    virtual RouterGroupData *getRouterGroupData(InterfaceEntry *ie, const IPv6Address &group);
    virtual void deleteHostInterfaceData(int interfaceId);
    virtual void deleteRouterInterfaceData(int interfaceId);
    virtual void deleteHostGroupData(InterfaceEntry *ie, const IPv6Address &group);
    virtual void deleteRouterGroupData(InterfaceEntry *ie, const IPv6Address &group);

    virtual void configureInterface(InterfaceEntry *ie);
    virtual void multicastGroupJoined(InterfaceEntry *ie, const IPv6Address& groupAddr);
    virtual void multicastGroupLeft(InterfaceEntry *ie, const IPv6Address& groupAddr);

    virtual void startTimer(cMessage *timer, double interval);
    virtual void startHostTimer(InterfaceEntry *ie, HostGroupData* group, double maxRespTime);

    virtual void sendQuery(InterfaceEntry *ie, const IPv6Address& groupAddr, double maxRespTime);
    virtual void sendReport(InterfaceEntry *ie, HostGroupData* group);
    virtual void sendLeave(InterfaceEntry *ie, HostGroupData* group);
    virtual void sendToIP(MLDMessage *msg, InterfaceEntry *ie, const IPv6Address& dest);

    virtual void processQueryTimer(cMessage *msg);
    virtual void processHostGroupTimer(cMessage *msg);
    virtual void processLeaveTimer(cMessage *msg);
    virtual void processRexmtTimer(cMessage *msg);

    virtual void processMldMessage(MLDMessage *msg);
    virtual void processQuery(InterfaceEntry *ie, const IPv6Address& sender, MLDMessage *msg);
    virtual void processGroupQuery(InterfaceEntry *ie, HostGroupData* group, int maxRespTime);
    //virtual void processV1Report(InterfaceEntry *ie, IGMPMessage *msg);
    virtual void processV2Report(InterfaceEntry *ie, MLDMessage *msg);
    virtual void processLeave(InterfaceEntry *ie, MLDMessage *msg);
};

#endif
