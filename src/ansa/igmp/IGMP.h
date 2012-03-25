// IGMP.cc
// Petr Matelesko
// 5. 5. 2010

#ifndef __IGMP_H_
#define __IGMP_H_

#include <vector>
#include <string.h>
#include <omnetpp.h>
#include "INETDefs.h"
#include "IPAddress.h"
#include "IPDatagram.h"
#include "IPControlInfo.h"
#include "IGMPMessage_m.h"
#include "IGMPTimer_m.h"
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"

class IPv4InterfaceData;

/**
 * Struktura pro uchovani prislusnosti rozhrani ke skupinam a casovacum.
 */
struct IGMPGroupMembership
{
    IPAddress multicastGroup;		// adresa mulicastove skupiny
    IPAddress lastReporter;		// adresa Last Reporter
    // ukazatele na casovace
    IGMPTimer *groupMembership;
    IGMPTimer *lastMember;
};

/**
 * Struktura pro uchovani informaci k rozhrani.
 */
struct IGMPInterface
{
    int intID;				// ID rozhrani
    InterfaceEntry *ie;			// ukazatel na popis rozhrani InterfaceEntry
    bool querier;			// querier x non-querier
    IPAddress ipQuerier;		// IP adresa Querier routeru
    
    std::vector<IGMPGroupMembership>	groupMembershipTable;

    /* ukazatele na casovace */
    IGMPTimer *initGeneralQuery;
    IGMPTimer *generalQuery;
    IGMPTimer *otherQuerierPresent;
};

class IGMP : public cSimpleModule
{
  private:

    std::vector<IGMPInterface> 	igmpIft;        /**< Interní tabulka rozhraní. */

  
    int ROBUSTNESS_VARIABLE;		// default: 2
    int QUERY_INTERVAL;			// default: 125 s
    int QUERY_RESPONSE_INTERVAL;	// default: 100 = 10 s
    int QUERY_RESPONSE_INTERVAL_SEC;
    int LAST_MEMBER_QUERY_INTERVAL;	// default: 10 = 1 s
    int LAST_MEMBER_QUERY_INTERVAL_SEC;
    int LAST_MEMBER_QUERY_COUNT;
    int LAST_MEMBER_QUERIER_INETERVAL;
    int GROUP_MEMBERSHIP_INTERVAL;
    int OTHER_QUERIER_PRESENT_INTERVAL;

    // Startup Query Count
    // Pocet poslanych zprav General Query pri inicializaci routeru
    int STARTUP_QUERY_COUNT;
    
    // Startup Query Interval
    // Interval mezi zpravami General Query pri inicializaci routeru
    int STARTUP_QUERY_INTERVAL;

    // tabulka rozhrani
    IInterfaceTable *ift;
    int getIGMPInterfaceByID(int interfaceID);


    // modul je soucasti modulu routeru
    bool isRouter();
    IPAddress getQuerierAddress(IPAddress thisRouterAddress, IPAddress otherRouterAddress);

    // metody pro praci s casovaci
    IGMPTimer * startInterfaceTimer(int timerType, IGMPInterface * igmpInterface);
    IGMPTimer * startGroupMembershipTimer(IGMPInterface * igmpInterface, IPAddress groupAddress, IPAddress lastReporter, int interval);
    IGMPTimer * startLastMemberTimer(IGMPInterface * igmpInterface, IPAddress groupAddress);
    void processIGMPTimer(IGMPTimer *timer);

    
    // tvorba IGMP paketu
    IGMPMessage * createPacket(int type, int maxRespTime, IPAddress groupAddress);

    // vytvoreni a posilani IGMP zprav
    void sendMembershipGeneralQuery(int interfaceID);
    void sendMembershipGroupSpecificQuery(IPAddress groupAddress, int interfaceID);
    void sendMembershipReportV2(IPAddress groupAddress, int interfaceID);
    void sendLeaveGroup(IPAddress groupAddress, int interfaceID);

    // prijem a zpracovani IGMP zprav
    void processIGMPMessage(IGMPMessage *igmpmsg);
    void processMembershipQuery(IGMPMessage *msg);
    void processMembershipReportV2(IGMPMessage *report);
    void processLeaveGroup(IGMPMessage *msg);

    // sprava clenstvi v multicastovych skupinach na rozhrani
    void addGroupToInterface(const int interfaceID, const IPAddress groupAddress);
    void removeGroupFromInterface(const int interfaceID, const IPAddress groupAddress);
    int getGroupMembershipPosition(IPAddress groupAddress, IGMPInterface * igmpInterface);

  
  protected:
    virtual int numInitStages() const  {return 5;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
};


#endif /* __IGMP_H_ */
