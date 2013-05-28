// Copyright (C) 2012 - 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

/**
 * @file MLD.h
 * @author Adam Malik(mailto:towdie13@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 9.5.2013
 * @brief
 * @detail
 */

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
    /**
     * Querier router state
     */
    enum RouterState
    {
        MLD_RS_INITIAL,
        MLD_RS_QUERIER,
        MLD_RS_NON_QUERIER,
    };
    /**
     * Router member state for groups
     */
    enum RouterGroupState
    {
        MLD_RGS_NO_MEMBERS_PRESENT,
        MLD_RGS_MEMBERS_PRESENT,
        MLD_RGS_CHECKING_MEMBERSHIP,
    };
    /**
     * Host member state for groups
     */
    enum HostGroupState
    {
        MLD_HGS_NON_MEMBER,
        MLD_HGS_DELAYING_MEMBER,
        MLD_HGS_IDLE_MEMBER,
    };

    /**
     * Struct for storing group data on hosts interface
     */
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

    /**
     * Struct for storing gtoup data on router interface
     */
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

    /**
     * Data about all interfaces on hosts
     */
    struct HostInterfaceData
    {
        MLD *owner;
        GroupToHostDataMap groups;

        HostInterfaceData(MLD *owner);
        virtual ~HostInterfaceData();
    };
    typedef std::map<int, HostInterfaceData*> InterfaceToHostDataMap;

    /**
     * Data about all interfaces on router
     */
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

    /**
     * Timers
     */
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
    int robustness;
    double queryInterval;
    double queryResponseInterval;
    double groupMembershipInterval;
    double otherQuerierPresentInterval;
    double startupQueryInterval;
    double startupQueryCount;
    double lastMemberQueryInterval;
    double lastMemberQueryCount;
    double unsolicitedReportInterval;

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
    virtual int numInitStages() const  {return 9;}
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
    virtual void multicastGroupJoined(InterfaceEntry *ie, const IPv6Address& groupAddr); /** @brief Function aclled from NB. Sending Join Message */
    virtual void multicastGroupLeft(InterfaceEntry *ie, const IPv6Address& groupAddr);   /** @brief Function called after removing group with leave timer */

    virtual void startTimer(cMessage *timer, double interval);
    virtual void startHostTimer(InterfaceEntry *ie, HostGroupData* group, double maxRespTime);

    virtual void sendQuery(InterfaceEntry *ie, const IPv6Address& groupAddr, double maxRespTime);   /** @brief Function for sending Query Messages */
    virtual void sendReport(InterfaceEntry *ie, HostGroupData* group);                              /** @brief Function for sending Report Messages */
    virtual void sendLeave(InterfaceEntry *ie, HostGroupData* group);                               /** @brief Function for sending Leave Group Message */
    virtual void sendToIP(MLDMessage *msg, InterfaceEntry *ie, const IPv6Address& dest);            /** @brief Function for preparing and sending message to IP module */

    virtual void processQueryTimer(cMessage *msg);          /** @brief Function for processing expired query timer */
    virtual void processHostGroupTimer(cMessage *msg);      /** @brief Function for processing expired Group timer on Hosts */
    virtual void processLeaveTimer(cMessage *msg);          /** @brief Function for processing expired Leave timer */
    virtual void processRexmtTimer(cMessage *msg);          /** @brief Function for processing expired Rexmt timer */

    virtual void processMldMessage(MLDMessage *msg);        /** Function for processing received MLD message(Splitter for another functions) */
    virtual void processQuery(InterfaceEntry *ie, const IPv6Address& sender, MLDMessage *msg);  /** Function for processing received Query Message */
    virtual void processGroupQuery(InterfaceEntry *ie, HostGroupData* group, int maxRespTime);  /** Function for prcessing Group Queries for each group */
    virtual void processV2Report(InterfaceEntry *ie, MLDMessage *msg);                          /** Function for processing Report messages */
    virtual void processLeave(InterfaceEntry *ie, MLDMessage *msg);                             /** Function for processing leave group messages */
};

#endif
