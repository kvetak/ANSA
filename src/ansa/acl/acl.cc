/*
 * 	acl.cc
 *
 *  Created  on: 24.2.2009
 *  Finished on:  7.5.2009
 *  Author: Tomas Suchomel, xsucho00
 */

#include "acl.h"

Define_Module(acl);

using namespace std;

/*
 * ipIsEqual(): IP IS EQUAL
 * Method that compares IP address from the packet header
 * to IP address presents in current ACL rule.
 * @param ip 		- struct with IP address from ACL rule
 * @param packet 	- struct with IP address from packet
 * @see filterPacket()
 * @return result of IP address comparison
 */
bool acl::ipIsEqual(TIP* ip, TIP* packet)
{
	ev << "ACL rule, ip  : " << ip->ipAddr.str() << endl;
	ev << "ACL rule, mask: " << ip->netmask.str() << endl;
	ev << "packet, ip    : " << packet->ipAddr.str() << endl;
	ev << "packet_masked : " << packet->ipAddr.doAnd(ip->netmask).str() << endl;
	if (ip->ipAddr != (packet->ipAddr.doAnd(ip->netmask)))
	{
		ev << "IP DOESN'T MATCH" << endl;
		return false;
	}
	ev << "IP MATCH OK" << endl;
	return true;
}

/*
 * portIsEqual(): PORT IS EQUAL
 * Method that compares port number(s) from the packet header
 * to port number(s) present in current ACL rule.
 * @param ip 		- struct with ports from ACL rule
 * @param packet 	- struct with ports from packet
 * @see filterPacket()
 * @return result of port comparison
 */
bool acl::portIsEqual(TIP* ip, TIP* packet)
{
	switch (ip->port_op) {
		case PORT_EQ:
			if (ip->portBeg == packet->portBeg)
				return true;
			break;
		case PORT_NEQ:
			if (ip->portBeg != packet->portBeg)
				return true;
			break;
		case PORT_GT:
			if (ip->portBeg < packet->portBeg)
				return true;
			break;
		case PORT_LT:
			if (ip->portBeg > packet->portBeg)
				return true;
			break;
		case PORT_RNG:
		case PORT_NDEF:
			if ((ip->portBeg <= packet->portBeg) && (ip->portEnd >= packet->portBeg))
				return true;
			break;
		default:
			cout << "BUG" << endl;
	}
	return false;
}

/*
 * andIpWithMask(): AND IP WITH MASK
 * Method that does AND operation between IP Address and network mask in current ACL rule.
 * @param rule	- struct with ACL rule
 * @return Negated wildcard mask [bits(O -> 1, 1 -> 0)].
 * @see loadConfigFromXML()
 */
void acl::andIpWithMask(TRule* rule)
{
	rule->source.ipAddr = 	rule->source.ipAddr.doAnd(rule->source.netmask);
	rule->dest.ipAddr = 	rule->dest.ipAddr.doAnd(rule->dest.netmask);
}

/*
 * negateWildcard(): NEGATE WILD CARD MASKs
 * Method that negates all the bits of given wildcard mask in ACL rule.
 * @param wc	- IPAddress structure with wildcard mask
 * @see loadConfigFromXML()
 */
IPv4Address acl::negateWildcard(IPv4Address wc)
{
	return (IPv4Address(~(wc.getInt())));
}

/*
 * getAction(): GET ACTION
 * Method that gets "action" field from XML string and puts it
 * to current ACL rule.
 * @param action	- string with text from XML config file
 * @param rule	 	- struct with ACL rule
 * @see loadConfigFromXML()
 */
void acl::getAction(std::string action, TRule* rule)
{
	if (action == "deny")
		rule->action = A_DENY;
	else if (action == "permit")
		rule->action = A_PERMIT;
}

/*
 * getProtocol(): GET PROTOCOL
 * Method that gets protocol field from XML string and puts it
 * to current ACL rule. It can handle protocol typed as text
 * or as a number (according to RFC).
 * @param pom	- string with text/number from XML config file
 * @param rule	- struct with ACL rule
 * @see loadConfigFromXML()
 */
void acl::getProtocol(std::string pom, TRule* rule)
{
	if (pom == "ip" || pom == "4")
		rule->protocol = PROT_IP;
	else if (pom == "tcp" || pom == "6")
		rule->protocol = PROT_TCP;
	else if (pom == "udp" || pom == "17")
		rule->protocol = PROT_UDP;
	else if (pom == "icmp" || pom == "1")
		rule->protocol = PROT_ICMP;
	else if (pom == "igmp" || pom == "2")
		rule->protocol = PROT_IGMP;
	else if (pom == "eigrp" || pom == "igrp" || pom == "88")
		rule->protocol = PROT_EIGRP;
	else if (pom == "ospf" || pom == "89")
		rule->protocol = PROT_OSPF;
}

/*
 * getPort(): GET PORT
 * Method that gets all port information fields from XML string.
 * It can handle ports typed as text or as a number (according to RFC).
 * @param pom	- string contains the port operator from XML file
 * @param p_beg	- string contains the beginning port no. from XML file
 * @param p_end	- string contains the ending port no. from XML file
 * @param ip	- struct with an IP Address in current ACL rule
 * @see loadConfigFromXML()
 */
void acl::getPort(std::string pom, std::string p_beg, std::string p_end, TIP *ip)
{
	int p1, p2;
	stringstream ss (stringstream::in | stringstream::out);
	/* if port is given as text string, it is converted to short int number */
	if (p_beg == "ftp-data") p1 = 20;
	if (p_beg == "ftp") p1 = 21;
	if (p_beg == "telnet") p1 = 23;
	if (p_beg == "smtp") p1 = 25;
	if (p_beg == "tftp") p1 = 69;
	if ((p_beg == "www") || (p_beg == "http") || (p_beg == "www-http")) p1 = 80;
	if ((p_beg == "pop3") || (p_beg == "pop")) p1 = 110;
	if (p_beg == "snmp") p1 = 161;
	if (p_beg == "irc") p1 = 194;
	if (p_beg == "ipx") p1 = 213;
	if (p_beg == "ldap") p1 = 389;
	if (p_beg == "https") p1 = 443;

	ss << p_beg;
	ss >> p1;
	ss << p_end;
	ss >> p2;

	/* resolution of port operators*/
	if (pom == "eq") 			// EQ
		ip->port_op = PORT_EQ;
	else if (pom == "neq") 		// NEQ
		ip->port_op = PORT_NEQ;
	else if (pom == "gt")		// GT
		ip->port_op = PORT_GT;
	else if (pom == "lt")		// LT
		ip->port_op = PORT_LT;
	else if (pom == "range")	// RANGE
		ip->port_op = PORT_RNG;
	else // if port operator is "" or not defined: NDEF
	{
		ip->port_op = PORT_NDEF;
	}
	switch (ip->port_op) { // based on presence of current port operator
		case PORT_EQ:
		case PORT_NEQ:
		case PORT_GT:
		case PORT_LT: // if operator is any of following [eq, neq, gt, lt]: load only one port no. from portBeg
			ip->portBeg = p1;
			break;
		case PORT_RNG: // if operator = RANGE, load two port number in the range of portBeg to portEnd
			ip->portBeg = p1;
			ip->portEnd = p2;
			break;
		case PORT_NDEF: // if any port operator is present, port range is full range of ports [0 - 65535]
			ip->portBeg = 0;
			ip->portEnd = 65535;
			break;
	}
}

/*
 * loadConfigFromXML(): LOAD CONFIG FROM XML
 * Method that loads configuration from XML file to IP ACL structures.
 * Also binds list of ACL rules to interface and direction.
 * @param fileName	- configuration filename (XML)
 * @see initialize()
 * @return correct/incorrect load of ACL configuration on Router
 */
bool acl::loadConfigFromXML(const char* fileName)
{
	/* get access into Routing Table and Interface Table modules */
	IRoutingTable* rt = RoutingTableAccess().get();
	IInterfaceTable* ift = InterfaceTableAccess().get();

	/* resolution of configuration XML file according to specified filename */
	cXMLElement* document = ev.getXMLDocument(fileName);
	if (document == NULL)
	{	/* if no file with such name is provided, error is recognized */
		error("Cannot read AS configuration from file %s", fileName);
		return false;
	}

	/* try to find current router in XML on running instance of this ACL module */
	std::string routerXPath("Router[@id='");
	std::string routerId = rt->getRouterId().str();
	routerXPath += routerId;
	routerXPath += "']";

	cXMLElement* router = document->getElementByPath(routerXPath.c_str());
	if (router == NULL)
	{	/* current router configuration is completely missing in XML configuration - error */
		error("No configuration for Router ID: %s", routerId.c_str());
		this->aclEnabled = false;
		return true;
	}
	cXMLElement* ACLs = router->getFirstChildWithTag("ACLs"); // find ACL configuration part
	if (ACLs == NULL)
	{	/* ACL configuration part is missing in XML configuration */
		//cout << "ACL: ACL is NOT ENABLED on Router id = " << routerId.c_str() << endl;
		this->aclEnabled = false; // ACL indicator is FALSE
		return true;
	}
	cXMLElement* ACL = ACLs->getFirstChildWithTag("ACL");
	if (ACL == NULL)
	{
		//cout << "ACL: ACL is NOT ENABLED on Router id = " << routerId.c_str() << endl;
		this->aclEnabled = false;
		return true;
	}
	else
	{
		this->aclEnabled = true;
		//cout << "ACL: Loading ACL entries for Router id = " << routerId.c_str() << "\n";
	}
	while (ACL != NULL)
	{
		cXMLElement *entry = ACL->getFirstChildWithTag("entry");
		TACL _acl;

		while (entry != NULL)
		{
			cXMLElement *leaf = entry->getFirstChild();
			TRule rule;
			Stat stat;
			stat.used = 0;

			std::string src_port_op = "";
			std::string src_port_beg = "";
			std::string src_port_end = "";
			std::string dst_port_op = "";
			std::string dst_port_beg = "";
			std::string dst_port_end = "";

			/* processing current leaf under XML node <entry> */
			while (leaf != NULL)
			{
				const char* tagName = leaf->getTagName();
				const char* value = leaf->getNodeValue();

				if (strcmp(tagName, "action") == 0)
					getAction(value, &rule);
				else if (strcmp(tagName, "IP_src") == 0)
					rule.source.ipAddr.set(value);
				else if (strcmp(tagName, "WC_src") == 0)
					rule.source.netmask = negateWildcard(IPv4Address(value));
				else if (strcmp(tagName, "protocol") == 0)
					getProtocol(value, &rule);
				else if (strcmp(tagName, "IP_dst") == 0)
					rule.dest.ipAddr.set(value);
				else if (strcmp(tagName, "WC_dst") == 0)
					rule.dest.netmask = negateWildcard(IPv4Address(value));
				else if (strcmp(tagName, "port_op_src") == 0)
					src_port_op = value;
				else if (strcmp(tagName, "port_beg_src") == 0)
					src_port_beg = value;
				else if (strcmp(tagName, "port_end_src") == 0)
					src_port_end = value;
				else if (strcmp(tagName, "port_op_dst") == 0)
					dst_port_op = value;
				else if (strcmp(tagName, "port_beg_dst") == 0)
					dst_port_beg = value;
				else if (strcmp(tagName, "port_end_dst") == 0)
					dst_port_end = value;
				else if (strcmp(tagName, "orig") == 0)
					stat.text = value;
				leaf = leaf->getNextSibling();
			}

			/* AND (&) IP ADDRESS WITH NETWORK MASK IN CURRENT ACL RULE */
			andIpWithMask(&rule);
			/* PROCESS PORT INFORMATION FROM XML CONFIGURATION FILE */
			getPort(src_port_op, src_port_beg, src_port_end, &rule.source);
			getPort(dst_port_op, dst_port_beg, dst_port_end, &rule.dest);

			/* generate statistics for processed rule */
			stats.push_back(stat);
			Stat* pStat = &this->stats.back();
			rule.used = &(pStat->used);

			_acl.push_back(rule); // processed rule is added at the end of ACL list

			entry = entry->getNextSiblingWithTag("entry");
		}
		/* create default rule at the end of the ACL list - deny ip any any */
		TRule rule;
		Stat stat;
		stat.used = 0;
		rule.action = A_DENY;
		rule.protocol = PROT_IP;
		stat.text = "access-list deny ip any any";
		rule.source.ipAddr.set("0.0.0.0");
		rule.dest.ipAddr.set("0.0.0.0");
		rule.source.netmask.set("0.0.0.0");
		rule.dest.netmask.set("0.0.0.0");
		getPort("", "", "", &rule.source);
		getPort("", "", "", &rule.dest);

		/* generate statistics for the default rule*/
		stats.push_back(stat);
		Stat* pStat = &this->stats.back();
		rule.used = &(pStat->used);

		_acl.push_back(rule); // implicit rule is finally added at the end of the list

		this->acls.push_back(_acl); // current ACL list is added at the end of ACLs (list of lists)

		cXMLElement *interfaces = ACL->getFirstChildWithTag("interfaces");
		cXMLElement *interface  = interfaces->getFirstChildWithTag("interface");

		/* processing interface information - binding ACL list to interface and direction */
		while (interface != NULL)
		{
			InterfaceEntry *targetInt = ift->getInterfaceByName(interface->getNodeValue());
			TInterface _if;

			_if.gateIndex = targetInt->getNetworkLayerGateIndex();
			if (interface->getAttribute("dir") == std::string("in"))
				_if.dir = true;
			else
				_if.dir = false;
			_if.rules = &this->acls.back();

			this->interfaces.push_back(_if);
			interface = interface->getNextSibling();
		}

		ACL = ACL->getNextSiblingWithTag("ACL");
	}
	return true;
}

/*
 * getRules(): GET RULES
 * Method that gets specific ACL list, which is about to be applied
 * on current in/outgoing packet.
 * @param gateIndex	- index of the arrival gate
 * @param dir		- direction of data communication
 * @see handleMessage()
 * @return ACL list corresponding to packet or NULL if it's not
 */
TACL* acl::getRules(int gateIndex, bool dir)
{
	for (std::list<TInterface>::iterator it = this->interfaces.begin(); it != this->interfaces.end(); it++)
	{
		TInterface* _if = &(*it);
		if (_if->gateIndex == gateIndex && _if->dir == dir)
			return _if->rules;
	}
	return NULL;
}

/*
 * processPacket(): PROCESS PACKET
 * Method that is processing current packet, which is casted to IPDatagram.
 * It decapsulates packet and save the information such as source address,
 * destination address, transport protocol and transfer port numbers.
 * It also decides, whether the packet is about to be sent to upper/lower layer,
 * or is about to be dropped.
 * @param packet	- current IP packet casted to IPDatagram
 * @param acl		- list of ACL rules which fits current packet
 * @return Decision of action taken with current packet (permit/deny)
 */
bool acl::processPacket(IPv4Datagram* packet, TACL* acl)
{
	TIP source, dest;
	source.ipAddr = packet->getSrcAddress();
	dest.ipAddr = packet->getDestAddress();
	int protocol = packet->getTransportProtocol();

	ev << "Packet Source IP Address: " << source.ipAddr.str() << endl;
	ev << "Packet Dest.  IP Address: " << dest.ipAddr.str() << endl;
	ev << "Packet Protocol ID      : " << protocol << endl;

	UDPPacket *udppacket;
	TCPSegment *tcppacket;

	switch (protocol)
	{
		case PROT_UDP: // if protocol is UDP, ports need to be checked
			udppacket = dynamic_cast<UDPPacket *> (packet->decapsulate());
			source.portBeg = udppacket->getSourcePort();
			ev << "UDP packet, source port: " << source.portBeg << endl;
			dest.portBeg = udppacket->getDestinationPort();
			ev << "UDP packet, dest.  port: " << dest.portBeg << endl;
			delete udppacket;
			break;
		case PROT_TCP: // if protocol is TCP, ports need to be checked
			tcppacket = dynamic_cast<TCPSegment *> (packet->decapsulate());
			source.portBeg = tcppacket->getSrcPort();
			ev << "TCP packet, source port: " << source.portBeg << endl;
			dest.portBeg = tcppacket->getDestPort();
			ev << "TCP packet, dest.  port: " << dest.portBeg << endl;
			delete tcppacket;
			break;
		default:
			ev << "Protocol is other than TCP or UDP, ports don't need to be checked..." << endl;
			break;
	}
	if (filterPacket(acl, source, dest, protocol))
	{
		// rule is 'permit'
		this->packetsPermitted++;
		return true;
	}
	else
	{
	    ev << "- ACL ACTION:  DENY, packet dropped   ---" << endl;
	    this->packetsDropped++;
		return false;
	}
}

/*
 * filterPacket(): FILTER PACKET
 * Method that compares information from the packet with whole
 * ACL list bound to current packet. Depending on this comparison,
 * indication of action is taken. (true = permit, false = deny).
 * @param acl		- list of ACL rules which fits current packet
 * @param source	- structure with source data from packet
 * @param dest		- structure with destination data from packet
 * @param protocol 	- type of transport protocol of the packet
 * @return action taken with the current packet (permit/deny)
 * @see processPacket()
 */
bool acl::filterPacket(TACL* acl, TIP source, TIP dest, int protocol)
{
	for (TACL_it it = acl->begin(); it != acl->end(); it++)
	{
		if (!(it->protocol == protocol || it->protocol == PROT_IP))
		  continue;
		ev << "acl::filterPacket: PROTOCOL MATCH" << endl;
		if (!ipIsEqual(&(it->source), &source))
		  continue;
		ev << "acl::filterPacket: SOURCE IP MATCH" << endl;
		if (it->protocol == PROT_UDP || it->protocol == PROT_TCP)
		{
		  if (!portIsEqual(&(it->source), &source))
			  continue;
		  else
			  ev << "acl::filterPacket: SOURCE PORT MATCH" << endl;
		}
		if (!ipIsEqual(&(it->dest), &dest))
		  continue;
		ev << "acl::filterPacket: DESTINATION IP MATCH" << endl;
		if (it->protocol == PROT_UDP || it->protocol == PROT_TCP)
		{
		  if (!portIsEqual(&(it->dest), &dest))
			  continue;
		  else
			  ev << "acl::filterPacket: DESTINATION PORT MATCH" << endl;
		}
		(*(it->used))++;
		return it->action; // if match is found with current record of ACL, action is returned
	}
	cout << "BUG" << endl;
	return false; // if no match in whole ACL is found
}

/*
 * handleMessage(): HANDLE MESSAGE
 * Method that is processing current message arrived to gate.
 * It checks the index of the arrival gate, its name and index,
 * direction of communication according to which gate a packet came.
 * Some action is done with the message, it is being forwarded if no
 * ACL configuration is found for this router in XML file.
 * It also checks the type of packet. If it differs from IPDatagram,
 * it just forwards the message. Else it casts it to IPDatagram type.
 * @param msg		- original message arriving to gate
 */
void acl::handleMessage(cMessage* msg)
{
	cGate* gate = msg->getArrivalGate();
	string name = gate->getBaseName();
	int index = gate->getIndex();
	bool dir = true; // packet heading from networkLayer IN to interface (pres ACL); ifIn, ifOut (in == true)
	TACL* acl;

	if (name == "toNetworkLayerIn")
		dir = false; // packet heading from interface OUT to networkLayer (pres ACL); toNetworkLayerIn, toNetworkLayerOut
					 // (out == false)
	if (!(this->aclEnabled))
	{ // if ACLs part is NOT listed in XML configuration file for current router
		if (dir)
		{	/* send packet to upper layer: from interface out to networkLayer */
			this->send(msg, "toNetworkLayerOut", index);
			ev << "ACL DISABLED, just directly send from interface out to networkLayer" << endl;
		}
		else
		{	/* send packet to lower layer: from networkLayer out to interface */
			this->send(msg, "ifOut", index);
			ev << "ACL DISABLED, just directly send from networkLayer out to interface" << endl;
		}
	}
	else  /* ACL configuration part is found on current router (however, ACL list still might be empty */
	{
		acl = getRules(index, dir);  // get pointer to ACL list bound within current packet

		cMessage *copy = msg->dup(); // duplicate the message due to instability during operations with an original message

		cPacket *test = (cPacket*)(copy);
		string typ = test->getClassName();
		if (typ != "IPDatagram")
		{	// if the message is not an IP packet, message is just forwarded
			if (dir)
			{
				ev << "Not an IP packet, sending directly from interface out to networkLayer" << endl;
				this->send (msg, "toNetworkLayerOut", index);
			}
			else
			{
				ev << "Not an IP packet, sending directly from networkLayer out to interface..." << endl;
				this->send (msg, "ifOut", index);
			}
		}
		else // if dynamic_cast is correctly done - message is IPDatagram (an IP packet)
		{
			this->numPackets++;
			if (acl == NULL) // doesn't exist ACL for packet's interface and direction
			{
				ev << "No ACL found for this router/interface." << endl;
				this->packetsAllowed++;
				if (dir)
				{	/* send packet to upper layer: from interface out to networkLayer */
					this->send(msg, "toNetworkLayerOut", index);
					ev << "+packet sent to upper layer without ACL action" << endl;
				}
				else
				{	/* send packet to lower layer: from networkLayer out to interface */
					this->send(msg, "ifOut", index);
					ev << "+packet sent to lower layer without ACL action" << endl;
				}
			}
			else // ACL is bound within packet's interface and direction, need to filter
			{
				ev << "An ACL list bound correctly for this router/interface." << endl;
				IPv4Datagram *packet = dynamic_cast<IPv4Datagram*> (copy); // dynamic_cast is taken
				if (processPacket(packet, acl)) // action according to ACL is "permit"
				{
					if (dir)
					{	/* send packet to upper layer: from interface out to networkLayer */
					    this->send(msg, "toNetworkLayerOut", index);
					    ev << "+ACL ACTION:  PERMIT +++ from interface out to networkLayer" << endl;
					}
					else
					{	/* send packet to lower layer: from networkLayer out to interface */
						this->send(msg, "ifOut", index);
					    ev << "+ACL ACTION:  PERMIT +++ from networkLayer out to interface" << endl;
					}
				}
				else // action according to ACL is "deny"
				{
					delete msg;
				}
			}
		}
		delete copy;
	}
}

/*
 * initialize(): INITIALIZE
 * An autonomous method that's been called whenever simulation is started.
 * It initializes the ACL filtering module for the simulation needs.
 * @param stage		- phase of simulation init
 */
void acl::initialize(int stage)
{
	switch (stage)
	{
			case 0: // get access to notification board
				notificationBoard = NotificationBoardAccess().get();
				notificationBoard->subscribe(this, NF_INTERFACE_STATE_CHANGED);
				break;
			case 4: // initialization of the ACL module
				const char *fileName = par("configFile"); // XML file name read from parameter
				if (loadConfigFromXML(fileName)) // success loading configuration data from XML file
				{
					//cout << "ACL: Configuration successfully loaded." << endl;
				}
				else // error loading configuration data from XML file
				{
					//cout << "ACL: Error loading configuration." << endl;
				}
				/* initialization of ACL action counters - usage in statistics */
				this->packetsAllowed = 0;
				this->packetsDropped = 0;
				this->packetsPermitted = 0;
				this->numPackets = 0;

				/* added class of statistics information onto Watch list */
				WATCH_LIST(stats);
	}
}

/*
 * finish(): FINISH
 * An autonomous method that's been called when simulation ends.
 * It displays all possible statistics from ACL module activities on console output.
 */
void acl::finish()
{
  /*
	cout << "IP datagrams received: " << this->numPackets << endl;
	cout << "IP packets permitted without ACL action: " << this->packetsAllowed  << endl;
	cout << "IP packets permitted by ACL action: " << this->packetsPermitted << endl;
	cout << "IP packets denied by an ACL action: " << this->packetsDropped << endl;
	for (std::list<Stat>::iterator it = this->stats.begin(); it != this->stats.end(); it++)
	{
		cout << it->text << " (" << it->used << " matches)" << endl;
	}
	cout << "--------------------------------------------------------" << endl;
	*/
}
