/*
 * stpInstance.h
 *
 *  Created on: 16.5.2011
 *      Author: aranel
 */

#ifndef STPI_H_
#define STPI_H_

#include "MACAddress.h"
#include "macTable.h"
#include "STPBPDU_m.h"
#include "STPTCN_m.h"


class stpi {
public:
	stpi(unsigned int _vlan, unsigned int portCount, MACAddress _bridgeAddress, MACTable * _addrTable);
	virtual ~stpi();

	/* WARNING SEND HACK */

	typedef struct {
		unsigned int port;
		cMessage * msg;
	} tMsg;

	void send(cMessage *, unsigned int);
	std::vector<tMsg> msgList;

	/* END OF SEND HACK */

	/* According to IEEE802.1D 9.2.9 Encoding of Port Role values */
	typedef enum {
		RUNKNOWN = 0,
		RALTERNATE = 1,
		RDESIGNATED = 2,
		RROOT = 3,
	} tPortRole;

	/* According to IEEE802.1D-1998 8.4 Port States */
	typedef enum {
		SDISABLED = 0, // accept nothing, dispatch nothing
		SDISCARDING = 1, // accept BPDU
		SLEARNING = 3, // learning enabled
		SFORWARDING = 4, // accept all, dispatch all
	} tPortState;

	typedef struct {
		bool Learning;
		bool Forwarding;
	} tPortFlags;

	typedef struct {
		bool enabled;
		tPortState state;
		tPortRole role;

		unsigned int priority; // TODO own vector for RW

		/* FLAGS */
		tPortFlags flags;

		/* discovered values */
		unsigned int rootPathCost;
		unsigned int rootPriority;
		MACAddress rootID;

		unsigned int bridgePriority;
		MACAddress bridgeID;

		unsigned int portPriority; // of designated bridge
		unsigned int portID;

		/* TODO from bandwidth */
		unsigned int linkCost;

		unsigned int age; // age of this information
		unsigned int fdWhile; // forward timer

		/* discovered */
		unsigned int maxAge;
		unsigned int fwdDelay;
		unsigned int helloTime;
	} tPort;

	bool learning(unsigned int);
	bool forwarding(unsigned int);


    void handleBPDU(STPBPDU * bpdu);
    void handleTCN(STPTCN * tcn);

//  protected:
    virtual void handleMessage(cMessage * msg);


    void handleSelfMessage(cMessage * msg);
    void generateHelloBPDU(int port);
    void generateTCN();

    bool superiorBPDU(int port, STPBPDU * bpdu);
    void setSuperiorBPDU(int port, STPBPDU * bpdu);

    /* Generate BPDUs to all interfaces (for Root), isRoot Check commencing */
    void generator();
    void handleTick();
    void checkTimers();
    void checkParametersChange();

    void resetAge(int p);
    void resetFDWhile(int p);
    void initPortTable();
    void setPortState(unsigned int, tPortState);

    /* check if this bridge is root eligible, true is the root */
    bool checkRootEligibility();
    /* try become root and make all designated ports, or
     * will be designated and select root port ...
     */
    void tryRoot();
    /* compare two BridgeIDs (resp. PortID), positive -- first is the superior, zero -- same ID,
     * and negative -- first is inferior */
    int superiorID(unsigned int, MACAddress, unsigned int, MACAddress);
    int superiorPort(unsigned int, unsigned int, unsigned int, unsigned int);
    /* comparsion of all IDs in tPort structure */
    int superiorTPort(tPort, tPort);
    /* select root port */
    void selectRootPort();

    /* select designated ports */
    void selectDesignatedPorts();
    /* set all ports to Designated (for root bridge) */
    void allDesignated();


    /* neighbor lost handling */
    void lostRoot();
    void lostAlternate(int port);

    void reset();


	unsigned int vlan;

	bool isRoot;
	int rootPort;
	std::vector<unsigned int> desPorts; // designated ports

	unsigned int portCount;

	MACAddress bridgeAddress;
	unsigned int bridgePriority;

	/* discovered values */
	unsigned int rootPathCost;
	unsigned int rootPriority;
	MACAddress rootID;

	/* SET BY MGMT */
	unsigned int maxAge;
	unsigned int fwdDelay;
	unsigned int helloTime;
	unsigned int userPriority;

	/* parameter change detection */
	unsigned int ubridgePriority;

	/* SET BY ROOT BRIDGE */
	unsigned int cMaxAge;
	unsigned int cFwdDelay;
	unsigned int cHelloTime; // c stands for current

	/* BRIDGE TIMERS */
	unsigned int helloTimer;

	/* topology change commencing */
	int topologyChange;
	bool topologyChangeNotification;
	bool topologyChangeRecvd;

	std::vector<tPort> portTable;
	tPort defaultPort;


	MACTable * addrTable;


};

inline std::ostream& operator<<(std::ostream& os, const stpi::tPortRole r) {

	switch (r){
	case stpi::RUNKNOWN:
		os << "Unkn";
		break;
	case stpi::RALTERNATE:
		os << "Altr";
		break;
	case stpi::RDESIGNATED:
		os << "Desg";
		break;
	case stpi::RROOT:
		os << "Root";
		break;
	default:
		os << "<?>";
		break;
	}

	return os;
}

inline std::ostream& operator<<(std::ostream& os, const stpi::tPortState s) {

	switch (s){
	case stpi::SDISABLED:
		os << "---";
		break;
	case stpi::SDISCARDING:
		os << "DIS";
		break;
	case stpi::SLEARNING:
		os << "LRN";
		break;
	case stpi::SFORWARDING:
		os << "FWD";
		break;
	default:
		os << "<?>";
		break;
	}

	return os;
}

inline std::ostream& operator<<(std::ostream& os, const stpi::tPortFlags f) {
	os << "[";
	if (f.Learning) {
		os << "L";
	} else {
		os << "_";
	}
	if (f.Forwarding) {
		os << "F";
	} else {
		os << "_";
	}
	os << "]";
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const stpi::tPort p) {
	/*
	if (p.enabled) {
		os << " + ";
	} else {
		os << " - ";
	}
	*/



	os << p.flags  << " " << p.role << " " << p.state << " ";
	os << p.linkCost << " ";
	os <<  p.priority  << " ";
/*
	os << p.age << " ";
	os << p.fdWhile << " ";
*/
	/* DEBUG
	  os << " "
	   << p.rootPathCost << " " << p.priority << " :: ";

	os << p.rootPriority << "/" << p.rootID << ";"
	   << p.bridgePriority << "/" << p.bridgeID << ";"
	   << p.portPriority << "/" << p.portID ;
   */
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const stpi i) {
	os << "VLAN " << i.vlan << " \n";

	//os << "TC:" << i.topologyChange << " TCN:" << i.topologyChangeNotification <<
	//	" TCR:" << i.topologyChangeRecvd << " \n";


	os << "RootID Priority: " << i.rootPriority << " \n";
	os << "  Address: " << i.rootID << " \n";
	if (i.isRoot) {
		os << "  This bridge is the Root. \n";
	} else {
		os << "  Cost: " << i.rootPathCost << " \n";
		os << "  Port: " << i.rootPort << " \n";
	}
	os << "  Hello Time: " << i.cHelloTime << " \n";
	os << "  Max Age: " << i.cMaxAge << " \n";
	os << "  Forward Delay: " << i.cFwdDelay << " \n";

	os << "BridgeID Priority: " << i.bridgePriority << "\n";
	os << "  Address: " << i.bridgeAddress << " \n";
	//os << "\n";
	os << "  Hello Time: " << i.helloTime << " \n";
	os << "  Max Age: " << i.maxAge << " \n";
	os << "  Forward Delay: " << i.fwdDelay << " \n";
	os << "Port Flag Role State Cost Priority \n";
	os << "-----------------------------------------\n";

	for (unsigned int x = 0; x < i.portCount; x++) {
		os << x << "  " << i.portTable.at(x) << " \n";
	}


	return os;
}


#endif /* STPINSTANCE_H_ */
