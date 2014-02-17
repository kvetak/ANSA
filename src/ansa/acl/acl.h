/*
 * acl.h
 *
 *  Created on: 19.2.2009
 *  Author: Tomas Suchomel, xsucho00
 */

#ifndef ACL_H_
#define ACL_H_

#include <omnetpp.h>
#include "IPv4Address.h"
#include "IPvXAddressResolver.h"
#include "IRoutingTable.h"
#include "RoutingTableAccess.h"
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "IPv4Datagram.h"
#include "TCPSegment.h"
#include "UDPPacket.h"
#include "NotificationBoard.h"

/* VYCET POZADOVANYCH AKCI - DENY (ZAHODIT PAKET) NEBO PERMIT (PROPUSTIT PAKET) */
const bool A_PERMIT = true;
const bool A_DENY = false;

/* VYCET PROTOKOLU - IP, TCP, UDP, ICMP */
enum TProtocol
{
	PROT_ICMP = 1,
	PROT_IGMP = 2,
	PROT_IP = 4,
	PROT_TCP = 6,
	PROT_UDP = 17,
	PROT_EIGRP = 88,
	PROT_OSPF = 89,
	PROT_SCTP = 132
};

/* VYCET OPERATORU PRO PORTY - eq (je roven), neq (neni roven), gt (vetsi nez), lt (mensi nez), range (rozsah portu) */
enum TPortOP
{
	PORT_NDEF, // pokud neni zadny port pritomen v ACL pravidlu (port je optional command)
	PORT_EQ,
	PORT_NEQ,
	PORT_GT,
	PORT_LT,
	PORT_RNG
};

struct TIP
{
	IPv4Address ipAddr, netmask;
	int portBeg, portEnd;
	TPortOP port_op;
};

struct TRule
{
	bool action;
	TProtocol protocol;
	TIP source, dest;
	int* used;
};

typedef std::list<TRule> TACL;
typedef std::list<TRule>::iterator TACL_it;

struct TInterface
{
	int gateIndex;
	bool dir;
	TACL* rules;
};

class Stat
{
public:
	std::string text;
	int used;
};

inline std::ostream& operator<< (std::ostream& ostr, Stat& statistics)
{
    ostr << statistics.text << " (" << statistics.used << " matches)";
    return ostr;
}

class acl : public cSimpleModule, protected INotifiable
{
private:
	bool loadConfigFromXML(const char* filename);
	bool processPacket(IPv4Datagram* packet, TACL* acl);
	TACL* getRules(int gateIndex, bool dir);
	bool filterPacket(TACL* acl, TIP source, TIP dest, int protocol);
	bool ipIsEqual(TIP* ip, TIP* packet);
	bool portIsEqual(TIP* ip, TIP* packet);
	void getAction(std::string action, TRule* rule);
	void getProtocol(std::string pom, TRule* rule);
	void getPort(std::string pom, std::string p_beg, std::string p_end, TIP *ip);
	void andIpWithMask(TRule* rule);
	IPv4Address negateWildcard(IPv4Address wc);

private:
	std::list<TACL> acls;
	std::list<TInterface> interfaces;
	std::list<Stat> stats;
	bool aclEnabled; 		// ACL configuration is present/missing in XML cfg file
	int numPackets; 		// IPDatagrams arrived into ACL filtering module
	int packetsDropped; 	// packets dropped by an ACL action "deny"
	int packetsPermitted; 	// packets permitted by an ACL action "permit"
	int packetsAllowed; 	// without ACL action (e.g. no ACL bound for packet's intf/dir)

protected:
	virtual void handleMessage(cMessage *msg);
	virtual void initialize(int stage);
	virtual void finish();
	NotificationBoard *notificationBoard;
    virtual void receiveChangeNotification(int category, const cPolymorphic *details){}
    virtual int numInitStages() const  { EV << "numinitstages\n"; return 5;}
};

#endif /* ACL_H_ */
