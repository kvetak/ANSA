//
// Copyright (C) 2011 CoCo Communications
// Copyright (C) 2012 Opensim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "MLD.h"
#include "RoutingTable6Access.h"
#include "InterfaceTableAccess.h"
#include "IPv6ControlInfo.h"
#include "IPv6InterfaceData.h"
#include "NotificationBoard.h"

#include <algorithm>

Define_Module(MLD);

// RFC 2236, Section 6: Host State Diagram
//                           ________________
//                          |                |
//                          |                |
//                          |                |
//                          |                |
//                --------->|   Non-Member   |<---------
//               |          |                |          |
//               |          |                |          |
//               |          |                |          |
//               |          |________________|          |
//               |                   |                  |
//               | leave group       | join group       | leave group
//               | (stop timer,      |(send report,     | (send leave
//               |  send leave if    | set flag,        |  if flag set)
//               |  flag set)        | start timer)     |
//       ________|________           |          ________|________
//      |                 |<---------          |                 |
//      |                 |                    |                 |
//      |                 |<-------------------|                 |
//      |                 |   query received   |                 |
//      | Delaying Member |    (start timer)   |   Idle Member   |
// ---->|                 |------------------->|                 |
//|     |                 |   report received  |                 |
//|     |                 |    (stop timer,    |                 |
//|     |                 |     clear flag)    |                 |
//|     |_________________|------------------->|_________________|
//| query received    |        timer expired
//| (reset timer if   |        (send report,
//|  Max Resp Time    |         set flag)
//|  < current timer) |
// -------------------
//
// RFC 2236, Section 7: Router State Diagram
//                                     --------------------------------
//                             _______|________  gen. query timer      |
// ---------                  |                |        expired        |
//| Initial |---------------->|                | (send general query,  |
// ---------  (send gen. q.,  |                |  set gen. q. timer)   |
//       set initial gen. q.  |                |<----------------------
//             timer)         |    Querier     |
//                            |                |
//                       -----|                |<---
//                      |     |                |    |
//                      |     |________________|    |
//query received from a |                           | other querier
//router with a lower   |                           | present timer
//IP address            |                           | expired
//(set other querier    |      ________________     | (send general
// present timer)       |     |                |    |  query,set gen.
//                      |     |                |    |  q. timer)
//                      |     |                |    |
//                       ---->|      Non       |----
//                            |    Querier     |
//                            |                |
//                            |                |
//                       ---->|                |----
//                      |     |________________|    |
//                      | query received from a     |
//                      | router with a lower IP    |
//                      | address                   |
//                      | (set other querier        |
//                      |  present timer)           |
//                       ---------------------------
//
//                              ________________
// ----------------------------|                |<-----------------------
//|                            |                |timer expired           |
//|               timer expired|                |(notify routing -,      |
//|          (notify routing -)|   No Members   |clear rxmt tmr)         |
//|                    ------->|    Present     |<-------                |
//|                   |        |                |       |                |
//|v1 report rec'd    |        |                |       |  ------------  |
//|(notify routing +, |        |________________|       | | rexmt timer| |
//| start timer,      |                    |            | |  expired   | |
//| start v1 host     |  v2 report received|            | | (send g-s  | |
//|  timer)           |  (notify routing +,|            | |  query,    | |
//|                   |        start timer)|            | |  st rxmt   | |
//|         __________|______              |       _____|_|______  tmr)| |
//|        |                 |<------------       |              |     | |
//|        |                 |                    |              |<----- |
//|        |                 | v2 report received |              |       |
//|        |                 | (start timer)      |              |       |
//|        | Members Present |<-------------------|    Checking  |       |
//|  ----->|                 | leave received     |   Membership |       |
//| |      |                 | (start timer*,     |              |       |
//| |      |                 |  start rexmt timer,|              |       |
//| |      |                 |  send g-s query)   |              |       |
//| |  --->|                 |------------------->|              |       |
//| | |    |_________________|                    |______________|       |
//| | |v2 report rec'd |  |                          |                   |
//| | |(start timer)   |  |v1 report rec'd           |v1 report rec'd    |
//| |  ----------------   |(start timer,             |(start timer,      |
//| |v1 host              | start v1 host timer)     | start v1 host     |
//| |tmr    ______________V__                        | timer)            |
//| |exp'd |                 |<----------------------                    |
//|  ------|                 |                                           |
//|        |    Version 1    |timer expired                              |
//|        | Members Present |(notify routing -)                         |
// ------->|                 |-------------------------------------------
//         |                 |<--------------------
// ------->|_________________| v1 report rec'd     |
//| v2 report rec'd |   |   (start timer,          |
//| (start timer)   |   |    start v1 host timer)  |
// -----------------     --------------------------

MLD::HostGroupData::HostGroupData(MLD *owner, const IPv6Address &group)
    : owner(owner), groupAddr(group)
{
    ASSERT(owner);
    ASSERT(groupAddr.isMulticast());

    state = MLD_HGS_NON_MEMBER;
    flag = false;
    timer = NULL;
}

MLD::HostGroupData::~HostGroupData()
{
    if (timer)
    {
        delete (MLDHostTimerContext*)timer->getContextPointer();
        owner->cancelAndDelete(timer);
    }
}

MLD::RouterGroupData::RouterGroupData(MLD *owner, const IPv6Address &group)
    : owner(owner), groupAddr(group)
{
    ASSERT(owner);
    ASSERT(groupAddr.isMulticast());

    state = MLD_RGS_NO_MEMBERS_PRESENT;
    timer = NULL;
    rexmtTimer = NULL;
    // v1HostTimer = NULL;
}

MLD::RouterGroupData::~RouterGroupData()
{
    if (timer)
    {
        delete (MLDRouterTimerContext*)timer->getContextPointer();
        owner->cancelAndDelete(timer);
    }
    if (rexmtTimer)
    {
        delete (MLDRouterTimerContext*)rexmtTimer->getContextPointer();
        owner->cancelAndDelete(rexmtTimer);
    }
//    if (v1HostTimer)
//    {
//        delete (IGMPRouterTimerContext*)v1HostTimer->getContextPointer();
//        owner->cancelAndDelete(v1HostTimer);
//    }
}

MLD::HostInterfaceData::HostInterfaceData(MLD *owner)
    : owner(owner)
{
    ASSERT(owner);
}

MLD::HostInterfaceData::~HostInterfaceData()
{
    for (GroupToHostDataMap::iterator it = groups.begin(); it != groups.end(); ++it)
        delete it->second;
}

MLD::RouterInterfaceData::RouterInterfaceData(MLD *owner)
    : owner(owner)
{
    ASSERT(owner);

    mldRouterState = MLD_RS_INITIAL;
    mldQueryTimer = NULL;
}

MLD::RouterInterfaceData::~RouterInterfaceData()
{
    owner->cancelAndDelete(mldQueryTimer);

    for (GroupToRouterDataMap::iterator it = groups.begin(); it != groups.end(); ++it)
        delete it->second;
}

MLD::HostInterfaceData *MLD::createHostInterfaceData()
{
    return new HostInterfaceData(this);
}

MLD::RouterInterfaceData *MLD::createRouterInterfaceData()
{
    return new RouterInterfaceData(this);
}

MLD::HostGroupData *MLD::createHostGroupData(InterfaceEntry *ie, const IPv6Address &group)
{
    HostInterfaceData *interfaceData = getHostInterfaceData(ie);
    ASSERT(interfaceData->groups.find(group) == interfaceData->groups.end());
    HostGroupData *data = new HostGroupData(this, group);
    interfaceData->groups[group] = data;
    return data;
}

MLD::RouterGroupData *MLD::createRouterGroupData(InterfaceEntry *ie, const IPv6Address &group)
{
    RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);
    ASSERT(interfaceData->groups.find(group) == interfaceData->groups.end());
    RouterGroupData *data = new RouterGroupData(this, group);
    interfaceData->groups[group] = data;
    return data;
}

MLD::HostInterfaceData *MLD::getHostInterfaceData(InterfaceEntry *ie)
{
    int interfaceId = ie->getInterfaceId();
    InterfaceToHostDataMap::iterator it = hostData.find(interfaceId);
    if (it != hostData.end())
        return it->second;

    // create one
    HostInterfaceData *data = createHostInterfaceData();
    hostData[interfaceId] = data;
    return data;
}

MLD::RouterInterfaceData *MLD::getRouterInterfaceData(InterfaceEntry *ie)
{
    int interfaceId = ie->getInterfaceId();
    InterfaceToRouterDataMap::iterator it = routerData.find(interfaceId);
    if (it != routerData.end())
        return it->second;

    // create one
    RouterInterfaceData *data = createRouterInterfaceData();
    routerData[interfaceId] = data;
    return data;
}

MLD::HostGroupData *MLD::getHostGroupData(InterfaceEntry *ie, const IPv6Address &group)
{
    HostInterfaceData *interfaceData = getHostInterfaceData(ie);
    GroupToHostDataMap::iterator it = interfaceData->groups.find(group);
    return it != interfaceData->groups.end() ? it->second : NULL;
}

MLD::RouterGroupData *MLD::getRouterGroupData(InterfaceEntry *ie, const IPv6Address &group)
{
    RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);
    GroupToRouterDataMap::iterator it = interfaceData->groups.find(group);
    return it != interfaceData->groups.end() ? it->second : NULL;
}

void MLD::deleteHostInterfaceData(int interfaceId)
{
    InterfaceToHostDataMap::iterator interfaceIt = hostData.find(interfaceId);
    if (interfaceIt != hostData.end())
    {
        HostInterfaceData *interface = interfaceIt->second;
        hostData.erase(interfaceIt);
        delete interface;
    }
}

void MLD::deleteRouterInterfaceData(int interfaceId)
{
    InterfaceToRouterDataMap::iterator interfaceIt = routerData.find(interfaceId);
    if (interfaceIt != routerData.end())
    {
        RouterInterfaceData *interface = interfaceIt->second;
        routerData.erase(interfaceIt);
        delete interface;
    }
}

void MLD::deleteHostGroupData(InterfaceEntry *ie, const IPv6Address &group)
{
    HostInterfaceData *interfaceData = getHostInterfaceData(ie);
    GroupToHostDataMap::iterator it = interfaceData->groups.find(group);
    if (it != interfaceData->groups.end())
    {
        HostGroupData *data = it->second;
        interfaceData->groups.erase(it);
        delete data;
    }
}

void MLD::deleteRouterGroupData(InterfaceEntry *ie, const IPv6Address &group)
{
    RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);
    GroupToRouterDataMap::iterator it = interfaceData->groups.find(group);
    if (it != interfaceData->groups.end())
    {
        RouterGroupData *data = it->second;
        interfaceData->groups.erase(it);
        delete data;
    }
}

void MLD::initialize(int stage)
{
    if (stage == 1)
    {
        ift = InterfaceTableAccess().get();
        rt = RoutingTable6Access().get();
        nb = NotificationBoardAccess().get();

        nb->subscribe(this, NF_INTERFACE_DELETED);
        nb->subscribe(this, NF_IPv6_MCAST_JOIN);
        nb->subscribe(this, NF_IPv6_MCAST_LEAVE);

        enabled = par("enabled");
        externalRouter = gate("routerIn")->isPathOK() && gate("routerOut")->isPathOK();
        robustness = par("robustnessVariable");
        queryInterval = par("queryInterval");
        queryResponseInterval = par("queryResponseInterval");
        groupMembershipInterval = par("groupMembershipInterval");
        otherQuerierPresentInterval = par("otherQuerierPresentInterval");
        startupQueryInterval = par("startupQueryInterval");
        startupQueryCount = par("startupQueryCount");
        lastMemberQueryInterval = par("lastMemberQueryInterval");
        lastMemberQueryCount = par("lastMemberQueryCount");
        unsolicitedReportInterval = par("unsolicitedReportInterval");
        //version1RouterPresentInterval = par("version1RouterPresentInterval");

        numGroups = 0;
        numHostGroups = 0;
        numRouterGroups = 0;

        numQueriesSent = 0;
        numQueriesRecv = 0;
        numGeneralQueriesSent = 0;
        numGeneralQueriesRecv = 0;
        numGroupSpecificQueriesSent = 0;
        numGroupSpecificQueriesRecv = 0;
        numReportsSent = 0;
        numReportsRecv = 0;
        numLeavesSent = 0;
        numLeavesRecv = 0;

        WATCH(numGroups);
        WATCH(numHostGroups);
        WATCH(numRouterGroups);

        WATCH(numQueriesSent);
        WATCH(numQueriesRecv);
        WATCH(numGeneralQueriesSent);
        WATCH(numGeneralQueriesRecv);
        WATCH(numGroupSpecificQueriesSent);
        WATCH(numGroupSpecificQueriesRecv);
        WATCH(numReportsSent);
        WATCH(numReportsRecv);
        WATCH(numLeavesSent);
        WATCH(numLeavesRecv);
    }
    else if (stage == 5)
    {
        for (int i = 0; i < (int)ift->getNumInterfaces(); ++i)
        {
            InterfaceEntry *ie = ift->getInterface(i);
            if (ie->isMulticast())
                configureInterface(ie);
        }
        nb->subscribe(this, NF_INTERFACE_CREATED);
    }
}

MLD::~MLD()
{
    while (!hostData.empty())
        deleteHostInterfaceData(hostData.begin()->first);
    while (!routerData.empty())
        deleteRouterInterfaceData(routerData.begin()->first);
}

void MLD::receiveChangeNotification(int category, const cPolymorphic *details)
{
    Enter_Method_Silent();

    InterfaceEntry *ie;
    int interfaceId;
    IPv6MulticastGroupInfo *info;
    switch (category)
    {
        case NF_INTERFACE_CREATED:
            ie = check_and_cast<InterfaceEntry*>(details);
            if (ie->isMulticast())
                configureInterface(ie);
            break;
        case NF_INTERFACE_DELETED:
            ie = check_and_cast<InterfaceEntry*>(details);
            if (ie->isMulticast())
            {
                interfaceId = ie->getInterfaceId();
                deleteHostInterfaceData(interfaceId);
                deleteRouterInterfaceData(interfaceId);
            }
            break;
        case NF_IPv6_MCAST_JOIN:
            info = check_and_cast<IPv6MulticastGroupInfo*>(details);
            multicastGroupJoined(info->ie, info->groupAddress);
            break;
        case NF_IPv6_MCAST_LEAVE:
            info = check_and_cast<IPv6MulticastGroupInfo*>(details);
            multicastGroupLeft(info->ie, info->groupAddress);
            break;
    }
}

void MLD::configureInterface(InterfaceEntry *ie)
{
    if (enabled && rt->isMulticastForwardingEnabled() /*&& !externalRouter*/)
    {
        // start querier on this interface
        cMessage *timer = new cMessage("MLD query timer", MLD_QUERY_TIMER);
        timer->setContextPointer(ie);
        RouterInterfaceData *routerData = getRouterInterfaceData(ie);
        routerData->mldQueryTimer = timer;
        routerData->mldRouterState = MLD_RS_QUERIER;
        sendQuery(ie, IPv6Address(), queryResponseInterval); // general query
        startTimer(timer, startupQueryInterval);
    }
}

void MLD::handleMessage(cMessage *msg)
{
    if (!enabled)
    {
        if (!msg->isSelfMessage())
        {
            EV << "MLD disabled, dropping packet.\n";
            delete msg;
        }
        return;
    }

    if (msg->isSelfMessage())
    {
        switch (msg->getKind())
        {
            case MLD_QUERY_TIMER:
                processQueryTimer(msg);
                break;
            case MLD_HOSTGROUP_TIMER:
                processHostGroupTimer(msg);
                break;
            case MLD_LEAVE_TIMER:
                processLeaveTimer(msg);
                break;
            case MLD_REXMT_TIMER:
                processRexmtTimer(msg);
                break;
            default:
                ASSERT(false);
                break;
        }
    }
    else if (!strcmp(msg->getArrivalGate()->getName(), "routerIn"))
        send(msg, "ipOut");
    else if (dynamic_cast<MLDMessage *>(msg))
        processMldMessage((MLDMessage *)msg);
    else
        ASSERT(false);
}

void MLD::multicastGroupJoined(InterfaceEntry *ie, const IPv6Address& groupAddr)
{
    ASSERT(ie && ie->isMulticast());
    ASSERT(groupAddr.isMulticast());

    if (enabled && !groupAddr.isLinkLocal())    //todo zistiti co tu ma byt to linklocalmulticast
    {
        HostGroupData *groupData = createHostGroupData(ie, groupAddr);
        numGroups++;
        numHostGroups++;

        sendReport(ie, groupData);
        groupData->flag = true;
        startHostTimer(ie, groupData, unsolicitedReportInterval);
        groupData->state = MLD_HGS_DELAYING_MEMBER;
    }
}

void MLD::multicastGroupLeft(InterfaceEntry *ie, const IPv6Address& groupAddr)
{
    ASSERT(ie && ie->isMulticast());
    ASSERT(groupAddr.isMulticast());

    if (enabled && !groupAddr.isLinkLocal())        //todo zase link localmulticast
    {
        HostGroupData *groupData = getHostGroupData(ie, groupAddr);
        if (groupData)
        {
            if (groupData->state == MLD_HGS_DELAYING_MEMBER)
                cancelEvent(groupData->timer);

            if (groupData->flag)
                sendLeave(ie, groupData);
        }

        deleteHostGroupData(ie, groupAddr);
        numHostGroups--;
        numGroups--;
    }
}

void MLD::startTimer(cMessage *timer, double interval)
{
    ASSERT(timer);
    cancelEvent(timer);
    scheduleAt(simTime() + interval, timer);
}

void MLD::startHostTimer(InterfaceEntry *ie, HostGroupData* group, double maxRespTime)
{
    if (!group->timer)
    {
        group->timer = new cMessage("MLD group timer", MLD_HOSTGROUP_TIMER);
        group->timer->setContextPointer(new MLDHostTimerContext(ie, group));
    }

    double delay = uniform(0.0, maxRespTime);
    EV << "setting host timer for " << ie->getName() << " and group " << group->groupAddr.str() << " to " << delay << "\n";
    startTimer(group->timer, delay);
}

void MLD::sendQuery(InterfaceEntry *ie, const IPv6Address& groupAddr, double maxRespTime)
{
    ASSERT(groupAddr.isUnspecified() || groupAddr.isMulticast() /*&& !groupAddr.isLinkLocal())*/); //todo linklocalmulticast
    RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);

    if (interfaceData->mldRouterState == MLD_RS_QUERIER)
    {
        if (groupAddr.isUnspecified())
            EV << "MLD: sending General Membership Query on iface=" << ie->getName() << "\n";
        else
            EV << "MLD: sending Membership Query for group=" << groupAddr << " on iface=" << ie->getName() << "\n";

        MLDMessage *msg = new MLDMessage("MLD query");
        msg->setType(MLD_MEMBERSHIP_QUERY);
        msg->setGroupAddress(groupAddr);
        msg->setMaxRespTime((int)(maxRespTime * 10.0));
        msg->setByteLength(24);
        sendToIP(msg, ie, groupAddr.isUnspecified() ? IPv6Address::ALL_NODES_2 : groupAddr);

        numQueriesSent++;
        if (groupAddr.isUnspecified())
            numGeneralQueriesSent++;
        else
            numGroupSpecificQueriesSent++;
    }
}

void MLD::sendReport(InterfaceEntry *ie, HostGroupData* group)
{
    ASSERT(group->groupAddr.isMulticast() /*&& !group->groupAddr.isLinkLocal()*/);  //todo prekontrolovat preco to bolo v ipv4 a skusit hodit do ipv6 ak to bude nutne

    EV << "MLD: sending Membership Report for group=" << group->groupAddr << " on iface=" << ie->getName() << "\n";
    MLDMessage *msg = new MLDMessage("MLD report");
    msg->setType(MLD_MEMBERSHIP_REPORT);
    msg->setGroupAddress(group->groupAddr);
    msg->setByteLength(8);
    sendToIP(msg, ie, group->groupAddr);
    numReportsSent++;
}

void MLD::sendLeave(InterfaceEntry *ie, HostGroupData* group)
{
    ASSERT(group->groupAddr.isMulticast() /*&& !group->groupAddr.isLinkLocal()*/);  // todo linklocalmulticast

    EV << "MLD: sending Leave Group for group=" << group->groupAddr << " on iface=" << ie->getName() << "\n";
    MLDMessage *msg = new MLDMessage("MLD leave");
    msg->setType(MLD_LEAVE_GROUP);
    msg->setGroupAddress(group->groupAddr);
    msg->setByteLength(8);
    sendToIP(msg, ie, IPv6Address::ALL_ROUTERS_2);
    numLeavesSent++;
}

void MLD::sendToIP(MLDMessage *msg, InterfaceEntry *ie, const IPv6Address& dest)
{
    ASSERT(ie->isMulticast());

    IPv6ControlInfo *controlInfo = new IPv6ControlInfo();
    controlInfo->setProtocol(IP_PROT_IPv6_ICMP);     //todo mozem pouzit 58 aj ked nebude MLD v ICMPv6???
    controlInfo->setInterfaceId(ie->getInterfaceId());
    controlInfo->setHopLimit(1);
    controlInfo->setDestAddr(dest);
    msg->setControlInfo(controlInfo);

    send(msg, "ipOut");
}

void MLD::processMldMessage(MLDMessage *msg)
{
    IPv6ControlInfo *controlInfo = (IPv6ControlInfo *)msg->getControlInfo();
    InterfaceEntry *ie = ift->getInterfaceById(controlInfo->getInterfaceId());
    switch (msg->getType())
    {
        case MLD_MEMBERSHIP_QUERY:
            processQuery(ie, controlInfo->getSrcAddr(), msg);
            break;
        //case IGMPV1_MEMBERSHIP_REPORT:
        //    processV1Report(ie, msg);
        //    delete msg;
        //    break;
        case MLD_MEMBERSHIP_REPORT:
            processV2Report(ie, msg);
            break;
        case MLD_LEAVE_GROUP:
            processLeave(ie, msg);
            break;
        default:
            if (externalRouter)
                send(msg, "routerOut");
            else
            {
                delete msg;
                throw cRuntimeError("MLD: Unhandled message type (%dq)", msg->getType());
            }
            break;
    }
}

void MLD::processQueryTimer(cMessage *msg)
{
    InterfaceEntry *ie = (InterfaceEntry*)msg->getContextPointer();
    ASSERT(ie);
    EV << "MLD: General Query timer expired, iface=" << ie->getName() << "\n";
    RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);
    RouterState state = interfaceData->mldRouterState;
    if (state == MLD_RS_QUERIER || state == MLD_RS_NON_QUERIER)
    {
        interfaceData->mldRouterState = MLD_RS_QUERIER;
        sendQuery(ie, IPv6Address(), queryResponseInterval); // general query
        startTimer(msg, queryInterval);
    }
}

void MLD::processHostGroupTimer(cMessage *msg)
{
    MLDHostTimerContext *ctx = (MLDHostTimerContext*)msg->getContextPointer();
    EV << "MLD: Host Timer expired for group=" << ctx->hostGroup->groupAddr << " iface=" << ctx->ie->getName() << "\n";
    sendReport(ctx->ie, ctx->hostGroup);
    ctx->hostGroup->flag = true;
    ctx->hostGroup->state = MLD_HGS_IDLE_MEMBER;
}

void MLD::processLeaveTimer(cMessage *msg)
{
    MLDRouterTimerContext *ctx = (MLDRouterTimerContext*)msg->getContextPointer();
    EV << "MLD: Leave Timer expired, deleting " << ctx->routerGroup->groupAddr << " from listener list of '" << ctx->ie->getName() << "'\n";

    // notify IPv4InterfaceData to update its listener list
    ctx->ie->ipv6Data()->removeMulticastListener(ctx->routerGroup->groupAddr);

    IPv6MulticastGroupInfo info(ctx->ie, ctx->routerGroup->groupAddr);
    nb->fireChangeNotification(NF_IPv6_MCAST_UNREGISTERED, &info);
    numRouterGroups--;

    if (ctx->routerGroup->state ==  MLD_RGS_CHECKING_MEMBERSHIP)
        cancelEvent(ctx->routerGroup->rexmtTimer);

    ctx->routerGroup->state = MLD_RGS_NO_MEMBERS_PRESENT;
    deleteRouterGroupData(ctx->ie, ctx->routerGroup->groupAddr);
    numGroups--;
}

void MLD::processRexmtTimer(cMessage *msg)
{
    MLDRouterTimerContext *ctx = (MLDRouterTimerContext*)msg->getContextPointer();
    EV << "MLD: Rexmt Timer expired for group=" << ctx->routerGroup->groupAddr << " iface=" << ctx->ie->getName() << "\n";
    sendQuery(ctx->ie, ctx->routerGroup->groupAddr, lastMemberQueryInterval);
    startTimer(ctx->routerGroup->rexmtTimer, lastMemberQueryInterval);
    ctx->routerGroup->state = MLD_RGS_CHECKING_MEMBERSHIP;
}

void MLD::processQuery(InterfaceEntry *ie, const IPv6Address& sender, MLDMessage *msg)
{
    ASSERT(ie->isMulticast());

    HostInterfaceData *interfaceData = getHostInterfaceData(ie);

    numQueriesRecv++;

    IPv6Address &groupAddr = msg->getGroupAddress();
    if (groupAddr.isUnspecified())
    {
        // general query
        EV << "MLD: received General Membership Query on iface=" << ie->getName() << "\n";
        numGeneralQueriesRecv++;
        for (GroupToHostDataMap::iterator it = interfaceData->groups.begin(); it != interfaceData->groups.end(); ++it)
            processGroupQuery(ie, it->second, msg->getMaxRespTime());
    }
    else
    {
        // group-specific query
        EV << "MLD: received Membership Query for group=" << groupAddr << " iface=" << ie->getName() << "\n";
        numGroupSpecificQueriesRecv++;
        GroupToHostDataMap::iterator it = interfaceData->groups.find(groupAddr);
        if (it != interfaceData->groups.end())
            processGroupQuery(ie, it->second, msg->getMaxRespTime());
    }

    if (rt->isMulticastForwardingEnabled()) //todo opravit
    {
        //if (externalRouter)
        //{
        //    send(msg, "routerOut");
        //    return;
        //}

        RouterInterfaceData *routerInterfaceData = getRouterInterfaceData(ie);
        if (sender < ie->ipv6Data()->getPreferredAddress()) //todo ->getIPAddress()) nahradene
        {
            startTimer(routerInterfaceData->mldQueryTimer, otherQuerierPresentInterval);
            routerInterfaceData->mldRouterState = MLD_RS_NON_QUERIER;
        }

        if (!groupAddr.isUnspecified() && routerInterfaceData->mldRouterState == MLD_RS_NON_QUERIER) // group specific query
        {
            RouterGroupData *groupData = getRouterGroupData(ie, groupAddr);
            if (groupData->state == MLD_RGS_MEMBERS_PRESENT)
            {
                double maxResponseTime = (double)msg->getMaxRespTime() / 10.0;
                startTimer(groupData->timer, maxResponseTime * lastMemberQueryCount);
                groupData->state = MLD_RGS_CHECKING_MEMBERSHIP;
            }
        }
    }

    delete msg;
}

void MLD::processGroupQuery(InterfaceEntry *ie, HostGroupData* group, int maxRespTime)
{
    double maxRespTimeSecs = (double)maxRespTime / 10.0;

    if (group->state == MLD_HGS_DELAYING_MEMBER)
    {
        cMessage *timer = group->timer;
        simtime_t maxAbsoluteRespTime = simTime() + maxRespTimeSecs;
        if (timer->isScheduled() && maxAbsoluteRespTime < timer->getArrivalTime())
            startHostTimer(ie, group, maxRespTimeSecs);
    }
    else if (group->state == MLD_HGS_IDLE_MEMBER)
    {
        startHostTimer(ie, group, maxRespTimeSecs);
        group->state = MLD_HGS_DELAYING_MEMBER;
    }
    else
    {
        // ignored
    }
}

void MLD::processV2Report(InterfaceEntry *ie, MLDMessage *msg)
{
    ASSERT(ie->isMulticast());

    IPv6Address &groupAddr = msg->getGroupAddress();

    EV << "MLD: received V2 Membership Report for group=" << groupAddr << " iface=" << ie->getName() << "\n";

    numReportsRecv++;

    HostGroupData *hostGroupData = getHostGroupData(ie, groupAddr);
    if (hostGroupData)
    {
        if (hostGroupData && hostGroupData->state == MLD_HGS_DELAYING_MEMBER)
        {
            cancelEvent(hostGroupData->timer);
            hostGroupData->flag = false;
            hostGroupData->state = MLD_HGS_IDLE_MEMBER;
        }
    }

    if (rt->isMulticastForwardingEnabled()) //todo opravit
    {
        //if (externalRouter)
        //{
        //    send(msg, "routerOut");
        //    return;
        //}

        RouterGroupData* routerGroupData = getRouterGroupData(ie, groupAddr);
        if (!routerGroupData)
        {
            routerGroupData = createRouterGroupData(ie, groupAddr);
            numGroups++;
        }

        if (!routerGroupData->timer)
        {
            routerGroupData->timer = new cMessage("MLD leave timer", MLD_LEAVE_TIMER);
            routerGroupData->timer->setContextPointer(new MLDRouterTimerContext(ie, routerGroupData));
        }
        if (!routerGroupData->rexmtTimer)
        {
            routerGroupData->rexmtTimer = new cMessage("MLD rexmt timer", MLD_REXMT_TIMER);
            routerGroupData->rexmtTimer->setContextPointer(new MLDRouterTimerContext(ie, routerGroupData));
        }

        if (routerGroupData->state == MLD_RGS_NO_MEMBERS_PRESENT)
        {
            // notify IPv4InterfaceData to update its listener list
            ie->ipv6Data()->addMulticastListener(groupAddr);
            // notify routing
            IPv6MulticastGroupInfo info(ie, routerGroupData->groupAddr);    //todo uz to mam vyssie
            nb->fireChangeNotification(NF_IPv6_MCAST_REGISTERED, &info);    //todo pre ipv6
            numRouterGroups++;
        }
        else if (routerGroupData->state == MLD_RGS_CHECKING_MEMBERSHIP)
            cancelEvent(routerGroupData->rexmtTimer);

        startTimer(routerGroupData->timer, groupMembershipInterval);
        routerGroupData->state = MLD_RGS_MEMBERS_PRESENT;
    }

    delete msg;
}

void MLD::processLeave(InterfaceEntry *ie, MLDMessage *msg)
{
    ASSERT(ie->isMulticast());

    EV << "MLD: received Leave Group for group=" << msg->getGroupAddress() << " iface=" << ie->getName() << "\n";

    numLeavesRecv++;

    if (rt->isMulticastForwardingEnabled()) //todo opravit
    {
        //if (externalRouter)
        //{
        //    send(msg, "routerOut");
        //    return;
        //}

        IPv6Address &groupAddr = msg->getGroupAddress();
        RouterInterfaceData *interfaceData = getRouterInterfaceData(ie);
        RouterGroupData *groupData = getRouterGroupData(ie, groupAddr);
        if (groupData)
        {
            if (groupData->state == MLD_RGS_MEMBERS_PRESENT && interfaceData->mldRouterState == MLD_RS_QUERIER)
            {
                startTimer(groupData->timer, lastMemberQueryInterval * lastMemberQueryCount);
                startTimer(groupData->rexmtTimer, lastMemberQueryInterval);
                sendQuery(ie, groupAddr, lastMemberQueryInterval);
                groupData->state = MLD_RGS_CHECKING_MEMBERSHIP;
            }
        }
    }

    delete msg;
}
