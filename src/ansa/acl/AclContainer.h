//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __INET_ACLCONTAINER_H
#define __INET_ACLCONTAINER_H

#include <omnetpp.h>
#include "IPv4Address.h"
#include "IPvXAddressResolver.h"
#include "RoutingTableAccess.h"


#include "IPv4Datagram.h"
#include "TCPSegment.h"
#include "UDPPacket.h"

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

typedef std::list<TRule> TRULES;
typedef std::list<TRule>::iterator TRULES_it;

struct TACL
{
	std::string aclName;
	TRULES rules;
};

typedef std::list<TACL>::iterator TACL_itc;

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

class AclContainer : public cSimpleModule
{
private:
	bool loadConfigFromXML(const char* filename);
	bool processPacket(IPv4Datagram* packet, TRULES* acl);
	bool compareValues(TRULES* acl, TIP source, TIP dest, int protocol);
	TRULES* getRulesByAclName(std::string name);
	bool ipIsEqual(TIP* ip, TIP* packet);
	bool portIsEqual(TIP* ip, TIP* packet);
	void getAction(std::string action, TRule* rule);
	void getProtocol(std::string pom, TRule* rule);
	void getPort(std::string pom, std::string p_beg, std::string p_end, TIP *ip);
	void andIpWithMask(TRule* rule);
	IPv4Address negateWildcard(IPv4Address wc);

private:
	std::list<TACL> acls;
	std::list<Stat> stats;

public:
  bool matchPacketToAcl(std::string name, cMessage *msg);
  bool existAcl(std::string name);

protected:
	virtual void handleMessage(cMessage *msg);
	virtual void initialize(int stage);
  virtual int numInitStages() const  { return 5;}
};

#endif /* __INET_ACLCONTAINER_H */

