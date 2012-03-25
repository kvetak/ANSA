// IGMP.cc
// Petr Matelesko
// 5. 5. 2010

#include "IGMP.h"

#ifndef WITHOUT_IPv4
#include "IPv4InterfaceData.h"
#endif

Define_Module(IGMP);

/** Vypis struktury IGMPInterface */
std::ostream& operator<<(std::ostream& os, const IGMPInterface& e)
{
    os << "Internet address is " << e.ie->ipv4Data()->getIPAddress() << "/" << e.ie->ipv4Data()->getNetmask().getNetmaskLength() << endl;
    os << "IGMP is enabled on interface" << endl;
    os << "Curent IGMP version is 2" << endl;
//     os << "IGMP query interval is " << IGMP::QUERY_INTERVAL << " seconds" << endl;
//     os << "IGMP querier timeout is " << IGMP::OTHER_QUERIER_PRESENT_INTERVAL << " seconds" << endl;
//     os << "IGMP max query response time is " << IGMP::QUERY_RESPONSE_INTERVAL_SEC << " seconds" << endl;
//     os << "Last member query response interval is " << IGMP::QUERY_RESPONSE_INTERVAL_SEC / 1000 << " ms" << endl;
//    os << "IGMP activity: " << e.join << " joins, " << e.leave << " leaves" << endl;
    os << "IGMP querying router is " << e.ipQuerier;
    if (e.querier)
	os << " (this system)";
    os << endl;

    return os;
};

/**
 * isRouter()
 *
 * Funkce zjistuje zda je IGMP modul soucasti modulu routeru
 *
 * @return Vraci true pokud je modul soucasti modulu routeru, jinak false.
 */
bool IGMP::isRouter()
{
    cModule *parentModule = this->getParentModule()->getParentModule();

    std::string moduleName = parentModule->getNedTypeName();

    if (moduleName.find("router") != std::string::npos || moduleName.find("Router") != std::string::npos)
	return true;
    else
	return false;
}

/**
 * getQuerierAddress()
 *
 * Zjistuje, zda je router querier. Porovnava IP adresy. Pokud je IP adresa
 * tohoto routeru vyssi, je querier, jinak je non-querier.
 *
 * @param thisRouterAddress IP adresa tohoto routeru
 * @param otherRouterAddress IP adresa dalsiho routeru
 * @return Vraci IP adresu querier routeru.
 */
IPAddress IGMP::getQuerierAddress(IPAddress thisRouterAddress, IPAddress otherRouterAddress)
{
    if (otherRouterAddress < thisRouterAddress)
	return otherRouterAddress;
    else
	return thisRouterAddress;
}

/**
 * getIGMPInterfaceByID()
 *
 * Metoda vraci index zadaneho rozhrani v tabulce IGMP rozhrani.
 *
 * @param interfaceID ID rozhrani
 * @return Vraci index daneho rozhrani v tabulce IGMP rozhrani, kdyz rozhrani nenajde vraci -1
 */
int IGMP::getIGMPInterfaceByID(int interfaceID)
{
    for (int i = 0; i < (int)igmpIft.size(); i++)
    {
	if (igmpIft[i].intID == interfaceID)
	    return i;
    }
    return -1;
}

/**
 * getGroupMembershipPosition()
 *
 * Zjistuje pritomnost multicastove skupiny na danem rozhrani.
 *
 * @param groupAddress Adresa multicastove skupiny.
 * @param igmpInterface Rozhrani
 * @return Vraci pozici daneho clenstvi, pokud clenstvi neexistuje vraci -1
 */
int IGMP::getGroupMembershipPosition(IPAddress groupAddress, IGMPInterface * igmpInterface)
{

    int n = igmpInterface->groupMembershipTable.size();
    for (int j = 0; j < n; j++)
    {
	if (groupAddress.equals(igmpInterface->groupMembershipTable[j].multicastGroup))
	    return j;
    }
    return -1;
}

/**
 * createPacket ()
 *
 * Vytvori IGMP paket.
 *
 * @param type Typ zpravy.
 * @param maxRespTime MaxResptime
 * @param groupAddress	Adresa multicastove skupiny.
 * @return Vraci vytvoreny IGMP paket.
 */
IGMPMessage *IGMP::createPacket(int type, int maxRespTime, IPAddress groupAddress)
{

    IGMPMessage *msg = new IGMPMessage();

    switch (type) {

	case IGMP_MEMBERSHIP_QUERY:
	    msg->setName("IGMP_MEMBERSHIP_QUERY");
	    break;

	case IGMP_MEMBERSHIP_REPORT_V1:
	    msg->setName("IGMP_MEMBERSHIP_REPORT_V1");
	    break;

	case IGMP_MEMBERSHIP_REPORT_V2:
	    msg->setName("IGMP_MEMBERSHIP_REPORT_V2");
	    break;

	case IGMP_LEAVE_GROUP:
	    msg->setName("IGMP_LEAVE_GROUP");
	    break;

    }
    
    msg->setType(type);
    msg->setMaxRespTime(maxRespTime);
    msg->setGroupAddress(groupAddress);

    return msg;
}


/**
 * sendMembershipGeneralQuery()
 *
 * Vytvori a odesle zpravu Membership General Query.
 * Popis Membership General Query:
 * IP src: IP adresa routeru
 * IP dst: 224.0.0.1 - all systems
 * Max Resp Time: 100 (10 s) = QUERY_RESPONSE_INTERVAL
 * Group Address: 0 (ignoruje se)
 *
 * @param interfaceID ID rozhrani na ktere se bude zprava posilat
 */
void IGMP::sendMembershipGeneralQuery(int interfaceID)
{
    IGMPMessage *msg = createPacket(IGMP_MEMBERSHIP_QUERY, QUERY_RESPONSE_INTERVAL_SEC, IPAddress::UNSPECIFIED_ADDRESS);

    IPControlInfo *controlInfo = new IPControlInfo();
    controlInfo->setDestAddr(IPAddress::ALL_HOSTS_MCAST);
    controlInfo->setProtocol(IP_PROT_IGMP);
    controlInfo->setTimeToLive(1);
    controlInfo->setInterfaceId(interfaceID);
    msg->setControlInfo(controlInfo);

    send(msg, "sendOut");
}

/**
 * sendMembershipGroupSpecificQuery()
 *
 * Metoda pro odesilani zprav typu Membership Group-specific Query.
 * src: IP routeru
 * dst: ga
 * ga: ga
 * MaxRespTime: 10 (1 s) = LAST_MEMBER_QUERY_INTERVAL
 *
 * @param groupAddress Adresa multicastove skupiny
 * @param interfaceID ID rozhrani na ktere se bude zprava posilat
 */
void IGMP::sendMembershipGroupSpecificQuery(IPAddress groupAddress, int interfaceID)
{
    IGMPMessage *msg = createPacket(IGMP_MEMBERSHIP_QUERY, LAST_MEMBER_QUERY_INTERVAL_SEC, groupAddress);

    IPControlInfo *controlInfo = new IPControlInfo();
    controlInfo->setDestAddr(groupAddress);
    controlInfo->setProtocol(IP_PROT_IGMP);
    controlInfo->setTimeToLive(1);
    controlInfo->setInterfaceId(interfaceID);
    msg->setControlInfo(controlInfo);

    send(msg, "sendOut");
}

/**
 * sendMembershipReportV2()
 *
 * Vytvori a odesle zpravu Membership Report v2.
 * Popis Membership Report v2:
 * IP src: IP adresa hostu
 * IP dst: adresa multicastove skupiny
 * Max Resp Time: 0 (ignoruje se)
 * Group Address: adresa multicastove skupiny
 *
 * @param groupAddress Adresa multicastove skupiny
 * @param interfaceID ID rozhrani na ktere se bude zprava posilat
 */
void IGMP::sendMembershipReportV2(IPAddress groupAddress, int interfaceID)
{
    IGMPMessage *msg = createPacket(IGMP_MEMBERSHIP_REPORT_V2, 0, groupAddress);

    IPControlInfo *controlInfo = new IPControlInfo();
    controlInfo->setDestAddr(groupAddress);
    controlInfo->setProtocol(IP_PROT_IGMP);
    controlInfo->setTimeToLive(1);
    controlInfo->setInterfaceId(interfaceID);
    msg->setControlInfo(controlInfo);

    send(msg, "sendOut");
}

/**
 * sendLeaveGroup()
 *
 * Vytvori a odesle zpravu Leave Group.
 * Popis Leave Group:
 * IP src: IP adresa hostu
 * IP dst: 224.0.0.2 - all routers
 * Max Resp Time: 0 (ignoruje se)
 * Group Address: adresa multicastove skupiny
 *
 * @param groupAddress Adresa multicastove skupiny
 * @param interfaceID ID rozhrani na ktere se bude zprava posilat
 */
void IGMP::sendLeaveGroup(IPAddress groupAddress, int interfaceID)
{
    IGMPMessage *msg = createPacket(IGMP_LEAVE_GROUP, 0, groupAddress);

    IPControlInfo *controlInfo = new IPControlInfo();
    controlInfo->setDestAddr(IPAddress::ALL_ROUTERS_MCAST);
    controlInfo->setProtocol(IP_PROT_IGMP);
    controlInfo->setTimeToLive(1);
    controlInfo->setInterfaceId(interfaceID);
    msg->setControlInfo(controlInfo);

    send(msg, "sendOut");    
}

/**
 * addGroupToInterface()
 *
 * Prida adresu multicastove skupiny do promenne 'multicastGroups'
 * na dane rozhrani, kontroluje pripadnou pritomnost skupiny.
 *
 * @param interfaceID ID rozrani
 * @param groupAddress adresa multicastove skupiny
 */
void IGMP::addGroupToInterface(const int interfaceID, const IPAddress groupAddress)
{
    IPv4InterfaceData::IPAddressVector addr;	// vektor IP adres
    int i = getIGMPInterfaceByID(interfaceID);

    // ziska ukazatel na ipv4 data rozhrani
    IPv4InterfaceData *ipv4 = igmpIft[i].ie->ipv4Data();
    
    // pokud neni adresa multicastove skupiny v rozhrani, zapiseme ji
    int n = ipv4->getMulticastGroups().size();
    for (int j = 0; j < n; j++)
    {
	if (groupAddress.equals(ipv4->getMulticastGroups()[j]))
	    return;
    }
    addr = ipv4->getMulticastGroups();
    addr.push_back(groupAddress);
    ipv4->setMulticastGroups(addr);
}

/**
 * removeGroupFromInterface()
 *
 * Odebere adresu multicastove skupiny z promenne 'multicastGroups'
 * daneho rozhrani, kontroluje nepritomnost skupiny.
 *
 * @param interfaceID ID rozrani
 * @param groupAddress adresa multicastove skupiny
 */
void IGMP::removeGroupFromInterface(const int interfaceID, const IPAddress groupAddress)
{
    IPv4InterfaceData::IPAddressVector addr;	// vektor IP adres
    int i = getIGMPInterfaceByID(interfaceID);

    // ziska ukazatel na ipv4 data rozhrani
    IPv4InterfaceData *ipv4 = igmpIft[i].ie->ipv4Data();

    // pokud nalezneme danou multicastovou adresu, odstranime ji
    addr = ipv4->getMulticastGroups();
    
    int n = addr.size();
    for (int j = 0; j < n; j++)
    {
	if (groupAddress.equals(addr[j]))
	{
	    addr.erase(addr.begin() + j);
	    ipv4->setMulticastGroups(addr);
	    return;
	}	   
    }

}

/**
 * processMembershipReportV2()
 *
 * Zpracuje prichozi IGMP zpravy Membership Report v2
 *
 * @param report IGMP zprava
 */
void IGMP::processMembershipReportV2(IGMPMessage *msg)
{
    EV << "ProcessMembershipReportV2()" << endl;

    IPAddress groupAddress;	// skupinova adresa
    IPAddress lastReporter;	// zdrojova IP adresa klienta
    int intID;			// ID rozhrani
    int i;

    /* Ziskani skupiny */
    groupAddress = msg->getGroupAddress();

    /* zjistit z ktereho rozhrani zprava prisla a IP adresa klienta */
    if (dynamic_cast<IPControlInfo *>(msg->getControlInfo())!=NULL)
    {
	IPControlInfo *controlInfo = (IPControlInfo *)msg->getControlInfo();
        lastReporter = controlInfo->getSrcAddr();
	intID = controlInfo->getInterfaceId();
    }

    // pridat info na dane rozrhrani o skupine
    addGroupToInterface(intID, groupAddress);

    i = getIGMPInterfaceByID(intID);
    // zapni casovac clenstvi na skupinach
    startGroupMembershipTimer(&igmpIft[i], groupAddress, lastReporter, GROUP_MEMBERSHIP_INTERVAL);

    // pokud existuje casovac Last Member, tak ho zrus
    int j = getGroupMembershipPosition(groupAddress, &igmpIft[i]);
    if (igmpIft[i].groupMembershipTable[j].lastMember != NULL && igmpIft[i].groupMembershipTable[j].lastMember->isScheduled())
	{
	    cancelEvent(igmpIft[i].groupMembershipTable[j].lastMember);
	    delete igmpIft[i].groupMembershipTable[j].lastMember;
	    igmpIft[i].groupMembershipTable[j].lastMember = NULL;
	}
    
    // TODO notify routing - spoluprace s PIM

    delete msg;
}


void IGMP::processLeaveGroup(IGMPMessage *msg)
{
    EV << "processLeaveGroup()" << endl;

    IPAddress groupAddress;	// skupinova adresa
    IPAddress lastReporter;	// zdrojova IP adresa klienta
    int intID;			// ID rozhrani
    int i;
    int j;

    /* Ziskani skupiny */
    groupAddress = msg->getGroupAddress();

    /* zjistit z ktereho rozhrani zprava prisla a IP adresa klienta */
    if (dynamic_cast<IPControlInfo *>(msg->getControlInfo())!=NULL)
    {
	IPControlInfo *controlInfo = (IPControlInfo *)msg->getControlInfo();
        lastReporter = controlInfo->getSrcAddr();
	intID = controlInfo->getInterfaceId();
    }

    i = getIGMPInterfaceByID(intID);

    /* Pokud je rozhrani non-querier, tak zpravu zahodime */
    if (!igmpIft[i].querier) {
	delete msg;
	return;
    }

    // pozadovana skupina neni v tabulce, zpravu zahodime
    if ((j = getGroupMembershipPosition(groupAddress, &igmpIft[i])) == -1)
    {
	delete msg;
	return;
    }
    
    // - posle na dany inteface zpravu Membership Query - Group-Specific Query
    // - nastavi timer clenstvi ve skupine na LAST_MEMBER_QUERIER_INETERVAL
    // - nastavi timer pro opakovani Group-Specific Query
    startGroupMembershipTimer(&igmpIft[i], groupAddress, igmpIft[i].groupMembershipTable[j].lastReporter, LAST_MEMBER_QUERIER_INETERVAL);
    startLastMemberTimer(&igmpIft[i], groupAddress);
    sendMembershipGroupSpecificQuery(groupAddress, intID);

    delete msg;
}


/**
 * processMembershipQuery()
 *
 * Zpracuje zpravu Membership Query.
 *
 * @param msg IGMP zprava
 */
void IGMP::processMembershipQuery(IGMPMessage *msg)
{
    //EV << "ProcessMembershipQuery()" << endl; // DEBUG

    IPAddress srcAddr;
    IPAddress myAddr;
    IPAddress querierAddr;
    int intID;
    int i;

    /* zjistit z ktereho rozhrani zprava prisla a IP adresa klienta */
    if (dynamic_cast<IPControlInfo *>(msg->getControlInfo()) != NULL)
    {
	IPControlInfo *controlInfo = (IPControlInfo *)msg->getControlInfo();
        srcAddr = controlInfo->getSrcAddr();
 	intID = controlInfo->getInterfaceId();
    }

    i = getIGMPInterfaceByID(intID);
    myAddr = igmpIft[i].ie->ipv4Data()->getIPAddress();

    // testuju, jestli jsem non-querier
    querierAddr = getQuerierAddress(myAddr, srcAddr);
    
    if (querierAddr != myAddr)
    {
	//EV << "non-querier" << endl; // DEBUG

	// zmenim stav na non-querier
	igmpIft[i].querier = false;
	igmpIft[i].ipQuerier = querierAddr;

	// pokud existuje, zrusime casovac General Query Init
	if (igmpIft[i].initGeneralQuery != NULL && igmpIft[i].initGeneralQuery->isScheduled())
	{
	    cancelEvent(igmpIft[i].initGeneralQuery);
	    delete igmpIft[i].initGeneralQuery;
	    igmpIft[i].initGeneralQuery = NULL;
	}

	// pokud existuje, zrusime casovac General Query
	if (igmpIft[i].generalQuery != NULL && igmpIft[i].generalQuery->isScheduled())
	{
	    cancelEvent(igmpIft[i].generalQuery);
	    delete igmpIft[i].generalQuery;
	    igmpIft[i].generalQuery = NULL;
	}

	// spustime casovac Other Querier Present
	startInterfaceTimer(TIMER_OTHER_QUERIER_PRESENT, &igmpIft[i]);

    }

   delete msg;
}


/**
 * processIGMPMessage()
 *
 * Zpracuje prichozi IGMP zpravy podle jejich typu.
 *
 * @param igmpmsg Typ IGMP zpravy
 */
void IGMP::processIGMPMessage(IGMPMessage *igmpmsg)
{
    switch (igmpmsg->getType())
    {
	case IGMP_MEMBERSHIP_QUERY:
	    processMembershipQuery(igmpmsg);
	    break;

        case IGMP_MEMBERSHIP_REPORT_V2:
	    processMembershipReportV2(igmpmsg);
            break;

	case IGMP_LEAVE_GROUP:
	    processLeaveGroup(igmpmsg);
            break;
	default:
            EV << "processIGMPMesage(): Unknow IGMP Message" << endl;
    }
}

/**
 * processIGMPTimer()
 *
 * Zpracovani IGMP timeru.
 *
 * @param timer Timer
 */
void IGMP::processIGMPTimer(IGMPTimer *timer)
{
    int i;	// index v tabulce IGMP rozhrani
    i = getIGMPInterfaceByID(timer->getIntID());
    
    switch (timer->getTimerKind())
    {
	case TIMER_INIT_GENERAL_QUERY: 
	    // Casovac Init General Query vyprsel -
	    // - posle General Query
	    // - pokud jsme poslali vsechny Init General Query (STARTUP_QUERY_COUNT)
	    // -- nastavi casovac General Query
	    // - pokud jsme jeste vsechny Init General Query neposlali
	    // -- nastavi casovac Init General Query
	    sendMembershipGeneralQuery(timer->getIntID());

	    if ((timer->getCount() - 1) > 0)
		startInterfaceTimer(TIMER_INIT_GENERAL_QUERY, &igmpIft[i]);
	    else
		startInterfaceTimer(TIMER_GENERAL_QUERY, &igmpIft[i]);
	    break;

	case TIMER_GENERAL_QUERY:
	    // Casovac General Query vyprsel -
	    // - posle zpravu General Query a nastavi casovac General Query
	    sendMembershipGeneralQuery(timer->getIntID());
	    startInterfaceTimer(TIMER_GENERAL_QUERY, &igmpIft[i]);
	    break;
	    
	case TIMER_OTHER_QUERIER_PRESENT:
	    // Casovac pritomnosti jineho Querier routeru vyprsel -
	    // - rozhrani se stava opet Querierem
	    // - posle zpravu General Query a nastavi casovac General Query
	    igmpIft[i].querier = true;
	    igmpIft[i].ipQuerier = igmpIft[i].ie->ipv4Data()->getIPAddress();
	    igmpIft[i].otherQuerierPresent = NULL;
	    sendMembershipGeneralQuery(timer->getIntID());
	    startInterfaceTimer(TIMER_GENERAL_QUERY, &igmpIft[i]);
	    break;

	case TIMER_GROUP_MEMBERSHIP:
	    // Casovac clenstvi vyprsel:
	    // - odebereme multicasatovou skupiny z tabulky rozhrani
	    // - odstranime zaznam ve vlastni tabulce clenstvi
	    removeGroupFromInterface(timer->getIntID(), timer->getGroupAddress());

	    int j;
	    j = getGroupMembershipPosition(timer->getGroupAddress(), &igmpIft[i]);

	    igmpIft[i].groupMembershipTable.erase(igmpIft[i].groupMembershipTable.begin() + j);
	    
	    // TODO - notify routing - spoluprace s PIM
	    break;

	case TIMER_LAST_MEMBER:

	    sendMembershipGroupSpecificQuery(timer->getGroupAddress(), timer->getIntID());

	    if ((timer->getCount() - 1) > 0)
		startLastMemberTimer(&igmpIft[i], timer->getGroupAddress());

	    break;
    }
    delete timer;

}

/**
 * handleMessage()
 *
 * Funkce zpracovava prichozi zpravy. Je rozdelena na 2 casti -
 * pro router a pro host.
 *
 * @param msg Prichozi zprava
 */
void IGMP::handleMessage(cMessage *msg)
{
    /* Implementace IGMP na routeru */
    if (isRouter())
    {
	//EV << "IGMP::handleMessage(): on router" << endl; // DEBUG
    
	/* zpracovani vlastnich zprav - timeru */
	if (msg->isSelfMessage())
	{
	    processIGMPTimer(check_and_cast<IGMPTimer *>(msg));
	   
	  
	}
	/* zpracovani IGMP zprav */
	else
	{
	    processIGMPMessage(check_and_cast<IGMPMessage *>(msg));
	}

    }
    /* Implementace IGMP na hostu */
    else
    {
	//EV << "IGMP::handleMessage(): on host" << endl; // DEBUG

	/* vlastni zpravy */
	if (msg->isSelfMessage())
	{
	    // FIXME zatim zadne vlastni zpravy
	}
	/* IGMP zpravy */
	else
	{
	    IGMPMessage  *mymsg = check_and_cast<IGMPMessage *>(msg);

	    switch (mymsg->getType())
	    {
		case IGMP_MEMBERSHIP_QUERY:

		        if (dynamic_cast<IPControlInfo *>(mymsg->getControlInfo())!=NULL)
			{
			    IPControlInfo *controlInfo = (IPControlInfo *)mymsg->removeControlInfo();
			    EV << "IGMP_MEMBERSHIP_QUERY from " << controlInfo->getSrcAddr() << " to " << controlInfo->getDestAddr() << endl; // FIXME destAddr nevzyplnuje

			    delete controlInfo;
			}
		    
		    break;
	    }

	    delete mymsg;
	    
	}
	
    }

}

/**
 * startGroupMembershipTimer()
  *
 * Funkce pro spusteni ci reset casovace Group Membership
 *
 * @param igmpInterface Ukazatel na IGMP rozhrani
 * @param groupAddress Adresa multicastove skupiny
 * @param lastReporter Adresa Last Reportera
 * @param interval Interval vyprseni casovace
 */
IGMPTimer * IGMP::startGroupMembershipTimer(IGMPInterface * igmpInterface, IPAddress groupAddress, IPAddress lastReporter, int interval)
{
    IGMPTimer *newTimer = new IGMPTimer();
    newTimer->setTimerKind(TIMER_GROUP_MEMBERSHIP);
    newTimer->setName("Timer Group Membership");
    newTimer->setIntID(igmpInterface->intID);
    newTimer->setGroupAddress(groupAddress);
    scheduleAt(simTime() + interval, newTimer);
    
    // prohledame tabulku  clenstvi, pokud uz clenstvi existuje:
    // - zrusime stary timer
    // - zapiseme novy
    // - zapiseme noveho Last Reportera

    int j = getGroupMembershipPosition(groupAddress, igmpInterface);

    if (j != -1)
    {
	cancelEvent(igmpInterface->groupMembershipTable[j].groupMembership);
	delete igmpInterface->groupMembershipTable[j].groupMembership;
	igmpInterface->groupMembershipTable[j].lastReporter = lastReporter;
	igmpInterface->groupMembershipTable[j].groupMembership = newTimer;
	igmpInterface->groupMembershipTable[j].lastMember = NULL;
	return newTimer;
    }
   
    // pokud timer neexistuje, vytvorime nove clenstvi
    // - zapiseme vsechny hodnoty
    // - vlozime do tabulky clenstvi
    IGMPGroupMembership newGroupMembership;
    newGroupMembership.multicastGroup = groupAddress;
    newGroupMembership.lastReporter = lastReporter;
    newGroupMembership.groupMembership = newTimer;
    newGroupMembership.lastMember = NULL;
    igmpInterface->groupMembershipTable.push_back(newGroupMembership);
    
    return newTimer;
}

/**
 * startLastMemberTimer()
 *
 * Funkce pro spusteni ci reset casovace Last Member
 *
 * @param igmpInterface Ukazatel na IGMP rozhrani 
 * @param groupAddress Adresa multicastove skupiny
 */
IGMPTimer * IGMP::startLastMemberTimer(IGMPInterface * igmpInterface, IPAddress groupAddress)
{
    IGMPTimer *newTimer = new IGMPTimer();
    newTimer->setTimerKind(TIMER_LAST_MEMBER);
    newTimer->setName("Timer Last Member");
    newTimer->setIntID(igmpInterface->intID);
    newTimer->setGroupAddress(groupAddress);
    scheduleAt(simTime() + LAST_MEMBER_QUERY_INTERVAL_SEC, newTimer);

    // vytvoreni prvotniho timeru -
    // - pocet Group-Specific Query dle LAST_MEMBER_QUERY_COUNT

    int j = getGroupMembershipPosition(groupAddress, igmpInterface);

    if (j != -1 && igmpInterface->groupMembershipTable[j].lastMember == NULL)
	newTimer->setCount(LAST_MEMBER_QUERY_COUNT - 1);
    // u dalsich timeru snizujeme pocet o 1
    else
	newTimer->setCount(igmpInterface->initGeneralQuery->getCount() - 1);

    igmpInterface->groupMembershipTable[j].lastMember = newTimer;

    return newTimer;
}

/**
 * startInterfaceTimer()
 *
 * Vytvori/upravi casovac zadaneho typu na urcenem rozhrani.
 *
 * @param timerType Typ timeru - TIMER_INIT_GENERAL_QUERY, TIMER_GENERAL_QUERY, TIMER_OTHER_QUERIER_PRESENT
 * @param igmpInterface Ukazatel na IGMP rozhrani
 * @return Vraci ukazatel na nove vytvoreny casovac typu IGMPTimer.
 */
IGMPTimer * IGMP::startInterfaceTimer(int timerType, IGMPInterface * igmpInterface)
{
    IGMPTimer *newTimer = new IGMPTimer();
    newTimer->setTimerKind(timerType);
    newTimer->setIntID(igmpInterface->intID);

    switch (timerType)
    {
	case TIMER_INIT_GENERAL_QUERY:
	     
	    // vytvoreni prvotniho timeru -
	    // - pocet initializacnich General Query dle STARTUP_QUERY_COUNT
	    if (igmpInterface->initGeneralQuery == NULL)
		newTimer->setCount(STARTUP_QUERY_COUNT - 1);
	    // u dalsich timeru snizujeme pocet o 1
	    else
		newTimer->setCount(igmpInterface->initGeneralQuery->getCount() - 1);
	    
    	    newTimer->setName("Timer Init General Query");
	    scheduleAt(simTime() + STARTUP_QUERY_INTERVAL, newTimer);
	    igmpInterface->initGeneralQuery = newTimer;
	    break;

	case TIMER_GENERAL_QUERY:
	    newTimer->setName("Timer General Query");
	    scheduleAt(simTime() + QUERY_INTERVAL, newTimer);
	    igmpInterface->generalQuery = newTimer;
	    break;

	case TIMER_OTHER_QUERIER_PRESENT:
	    // pokud uz nejaky timer bezi, tak ho zrusime
	    if (igmpInterface->otherQuerierPresent != NULL && igmpInterface->otherQuerierPresent->isScheduled())
	    {
		cancelEvent(igmpInterface->otherQuerierPresent);
		delete igmpInterface->otherQuerierPresent;
	    }
	    // vytvorime novy timer
	    newTimer->setName("Timer Other Querier Present");
	    scheduleAt(simTime() + OTHER_QUERIER_PRESENT_INTERVAL, newTimer);
	    igmpInterface->otherQuerierPresent = newTimer;
	    break;
    }
    return newTimer;
}

/**
 * initialize()
 *
 * Inicializace IGMP modulu.
 *
 * @param stage Cislo stage
 */
void IGMP::initialize(int stage)
{
    if (stage == 4)
    {
	/* IGMP modul routeru */
	if (isRouter())
	{
	    /* pristup k tabulce rozhrani */
	    ift = InterfaceTableAccess().get();

	    /* inicializace timeru a promennych */

	    ROBUSTNESS_VARIABLE = 2;						// default: 2
	    QUERY_INTERVAL = 125;						// default: 125 s
	    QUERY_RESPONSE_INTERVAL = 100;					// default: 100 = 10 s
	    QUERY_RESPONSE_INTERVAL_SEC = QUERY_RESPONSE_INTERVAL / 10;		// prevod na sekundy
	    LAST_MEMBER_QUERY_COUNT = ROBUSTNESS_VARIABLE;
	    LAST_MEMBER_QUERY_INTERVAL = 10;	// default: 10 = 1 s
	    LAST_MEMBER_QUERY_INTERVAL_SEC = LAST_MEMBER_QUERY_INTERVAL / 10;	// prevod na sekundy
	    LAST_MEMBER_QUERIER_INETERVAL = LAST_MEMBER_QUERY_COUNT * LAST_MEMBER_QUERY_INTERVAL_SEC;
	    GROUP_MEMBERSHIP_INTERVAL = (ROBUSTNESS_VARIABLE * QUERY_INTERVAL) + QUERY_RESPONSE_INTERVAL_SEC;
	    STARTUP_QUERY_COUNT = ROBUSTNESS_VARIABLE;
	    STARTUP_QUERY_INTERVAL = QUERY_INTERVAL / 4;
	    OTHER_QUERIER_PRESENT_INTERVAL = (ROBUSTNESS_VARIABLE * QUERY_INTERVAL) + (QUERY_RESPONSE_INTERVAL_SEC / 2);
	    
	    /* naplneni vlastni tabulky rozhrani aktivni IGMP */
	    // - neni loopback
	    // - kazde rozhrani je automaticky Querier
	    // - casovace nejsou nastaveny
	    // TODO:
	    // - ted je IGMP na vsech rozhranich defaultne zaple
	    // - dodelat zapinani IGMP na rozhrani spolu s PIMem
	    
	    for (int i = 0; i < ift->getNumInterfaces(); i++)
	    {
		if (!ift->getInterface(i)->isLoopback())
		{
		    IGMPInterface igmpInt;
		    igmpInt.intID = ift->getInterface(i)->getInterfaceId();	// interface ID
		    igmpInt.ie = ift->getInterface(i);				// interface entry
		    igmpInt.querier = true;					// querier
		    igmpInt.ipQuerier = igmpInt.ie->ipv4Data()->getIPAddress();	// IP adresa Queriera
		    igmpInt.initGeneralQuery = NULL;				// casovac init general query
		    igmpInt.generalQuery = NULL;				// casovac general query
		    igmpInt.otherQuerierPresent = NULL;				// casova other querier present
		    igmpIft.push_back(igmpInt);

		}
	    }
	    
    	    /* sledovani promennych */

	    //WATCH_VECTOR(igmpIft[i].groupMembershipTable);
	    WATCH_VECTOR(igmpIft);
	    
	    /* startup query */
	    // na vsechny rozhrani v tabulce IGMP rozhrani posle General Query a spusti casovac General Query Init
	    for (int j = 0; j < (int)igmpIft.size(); j++)
	    {
		startInterfaceTimer(TIMER_INIT_GENERAL_QUERY, &igmpIft[j]);
		//sendMembershipGeneralQuery(igmpIft[j].intID);
	    }
		
	}


	/* IGMP modul hostu */
	/*else
	{
	    //EV << "IGMP::initialize() on client" << endl; // DEBUG

	    // TEST: pro testovani chovani
	    int intID;
	    ift = InterfaceTableAccess().get();


	    IPAddress ga1("225.0.0.1");
	    IPAddress ga2("225.0.0.2");
	    IPAddress ga3("225.0.0.3");
	    IPAddress ga4("225.0.0.4");

	    
	    for (int i = 0; i < ift->getNumInterfaces(); i++)
	    {
		if (!ift->getInterface(i)->isLoopback())
		{
		    intID = ift->getInterface(i)->getInterfaceId();
		    break;
		}
	    }

	    // test 1: join 225.0.0.1
	    sendMembershipReportV2(ga1, intID);

	    // test 2. joing 225.0.0.2 - prihlaseni dalsi skupiny
	    sendMembershipReportV2(ga2, intID); // join 225.0.0.2
	    
	    // test 3: join 225.0.0.2 - na skupinu, ktere uz je clenem
	    sendMembershipReportV2(ga2, intID);


	    // test 4: leave 225.0.0.1
	    sendLeaveGroup(ga1, intID);		// leave 225.0.0.1

	    // test 5: leave 225.0.0.3 - odhlaseni neprihlasene skupiny
	    sendLeaveGroup(ga3, intID);		// leave 225.0.0.3

	    //
// 	    sendMembership

	    // test 6: odhlaseni skupiny 225.0.0.2 po vyprseni casovace
	    // - neposilame zadnou zpravu

	}*/
   }
   
}
