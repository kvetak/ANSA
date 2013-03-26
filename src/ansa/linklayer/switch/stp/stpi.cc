/*
 * stpInstance.cpp
 *
 *  Created on: 16.5.2011
 *      Author: aranel
 */

#include "stpi.h"
#include "STPMACCompare.h"

stpi::stpi(unsigned int _vlan, unsigned int _portCount, MACAddress _bridgeAddress, MACTable * _addrTable) {

	portCount = _portCount;
	bridgeAddress = _bridgeAddress; // apply the bridge address
	vlan = _vlan;
	addrTable = _addrTable;
	initPortTable();

	helloTime = 2;
	maxAge = 20;
	fwdDelay = 15;
	bridgePriority = 32768;
	ubridgePriority = bridgePriority;

	isRoot = true;
	topologyChange = 0;
	topologyChangeNotification = false;
	topologyChangeRecvd = false;

	rootPriority = bridgePriority;
	rootID = bridgeAddress;
	rootPathCost = 0;
	rootPort = 0;
	cHelloTime = helloTime;
	cMaxAge = maxAge;
	cFwdDelay = fwdDelay;


	helloTimer = 0;


	allDesignated();

}

stpi::~stpi() {

}

void stpi::send(cMessage * _msg, unsigned int _port) {
	tMsg tmp;
	tmp.port = _port;
	tmp.msg = _msg;
	msgList.push_back(tmp);
}



/* dummy records to portTable to be learned */
void stpi::initPortTable() {
	tPortFlags flag;
	flag.Forwarding = false;
	flag.Learning = false;

	tPort port;

	port.enabled = true;
	port.state = SDISCARDING;
	port.role = RUNKNOWN;
	port.priority = 128;
	port.flags = flag;
	port.rootPathCost = INT16_MAX;
	port.rootPriority = 65536;
	port.rootID = MACAddress("FF-FF-FF-FF-FF-FF");
	port.bridgePriority = 65536;
	port.bridgeID = MACAddress("FF-FF-FF-FF-FF-FF");
	port.portPriority = 256;
	port.portID = 256;
	port.age = 0;
	port.fdWhile = 0;
	port.maxAge = 20;
	port.fwdDelay = 15;
	port.helloTime = 2;
	port.linkCost = 19;

	defaultPort = port;

	portTable.insert(portTable.begin(), portCount, port);

}

void stpi::handleMessage(cMessage *msg)
{
	cMessage * tmp = msg;

	if (!msg->isSelfMessage()) {
		if (dynamic_cast<STPBPDU *>(tmp)){ // Configuration BPDU
			STPBPDU * bpdu = (STPBPDU *) tmp;
			handleBPDU(bpdu);
		} else if (dynamic_cast<STPTCN *>(tmp)){ // Topology Change Notification
			STPTCN * tcn = (STPTCN *) tmp;
			handleTCN(tcn);
		} else { // Rubbish
			delete msg;
		}
	} else { // self rubbish
		delete msg;
	}
}

bool stpi::learning(unsigned int port) {
	return portTable.at(port).flags.Learning;
}

bool stpi::forwarding(unsigned int port) {
	return portTable.at(port).flags.Forwarding;
}

void stpi::handleBPDU(STPBPDU * bpdu) {

	/* get inferior BPDU, reply with superior */
	if (superiorBPDU(bpdu->getArrivalGate()->getIndex(), bpdu) == false) {
		/* ONLY on designated port, because of flapping old root bridge information
		 * between bridges, when only parameters changed
		 */
		if (portTable.at(bpdu->getArrivalGate()->getIndex()).role == RDESIGNATED) {
			generateHelloBPDU(bpdu->getArrivalGate()->getIndex());
		}
	}
	/* relay BPDU from root */
	else if (portTable.at(bpdu->getArrivalGate()->getIndex()).role == RROOT) {

		if (bpdu->getTca()) {
			topologyChangeNotification = false;
		}
		if (bpdu->getTc()) {
			topologyChange++;
		    addrTable->enableFasterAging();
		} else {
		    addrTable->resetAging();
		}

		for (unsigned int i = 0; i < desPorts.size(); i++) {
			generateHelloBPDU(desPorts.at(i));
		}
		if (topologyChangeRecvd) { // TCA with BPDUs
			topologyChangeRecvd = false;
		}
		if (topologyChange > 0) {
			topologyChange--;
		}
	}


	/* --- BEGIN RUBBISH --- */

	tryRoot();

	/* --- END OF RUBBISH --- */

	delete bpdu;
}

void stpi::handleTCN(STPTCN * tcn) {
	topologyChangeNotification = true;
	topologyChangeRecvd = true;
	//delete tcn;
}



void stpi::generateHelloBPDU(int port) {
	STPBPDU * bpdu = new STPBPDU("BPDU", vlan);
	bpdu->setVlan(vlan);
	bpdu->setPortRole(stpi::RDESIGNATED);
	bpdu->setBridgeID(bridgeAddress);
	bpdu->setBridgePriority(bridgePriority);
	bpdu->setRootPathCost(rootPathCost);
	bpdu->setRootID(rootID);
	bpdu->setRootPriority(rootPriority);
	bpdu->setPortID(port);
	bpdu->setPortPriority(portTable.at(port).priority);
	bpdu->setMsgAge(0);
	bpdu->setMaxAge(cMaxAge);
	bpdu->setHelloTime(cHelloTime);
	bpdu->setFwdDelay(cFwdDelay);



	if (topologyChangeRecvd) {
		bpdu->setTca(true);
	} else {
		bpdu->setTca(false);
	}
	if (topologyChange > 0) {
		 /* lowering value is in reply to root bpdu,
		  * for root in generator
		  */
		bpdu->setTc(true);
	} else {
		bpdu->setTc(false);
	}

	send(bpdu, port);
}

void stpi::generateTCN() {
	if (topologyChangeNotification) { // is something to notify
		if (portTable.at(rootPort).role == RROOT) { // exist root port to notifying
			STPTCN * tcn = new STPTCN("TCN", vlan);
			tcn->setVlan(vlan);
			send(tcn, rootPort);
		}
	}

}

/* check of the received BPDU is superior to port information from porttable */
bool stpi::superiorBPDU(int p, STPBPDU * bpdu) {
	tPort port = portTable.at(p);
	tPort xbpdu;
	int result;

	xbpdu.rootPriority = bpdu->getRootPriority();
	xbpdu.rootID = bpdu->getRootID();
	xbpdu.rootPathCost = bpdu->getRootPathCost() + port.linkCost;
	xbpdu.bridgePriority = bpdu->getBridgePriority();
	xbpdu.bridgeID = bpdu->getBridgeID();
	xbpdu.portPriority = bpdu->getPortPriority();
	xbpdu.portID = bpdu->getPortID();

	result = superiorTPort(port, xbpdu);

	if (result > 0) { // port is superior
		return false;
	}
	if (result < 0) { // BPDU is superior
		resetFDWhile(p); // renew info
		setPortState(p, SDISCARDING);
		setSuperiorBPDU(p, bpdu); // renew information
		return true;
	}

	setSuperiorBPDU(p, bpdu); // renew information
	return true;
}

/* set all new information to the port, or renew old one, eg. reset timer */
void stpi::setSuperiorBPDU(int port, STPBPDU * bpdu) {
	if (bpdu->getMsgAge() >= bpdu->getMaxAge()) {
		return;
	}

	portTable.at(port).rootPriority = bpdu->getRootPriority();
	portTable.at(port).rootID = bpdu->getRootID();
	portTable.at(port).rootPathCost = bpdu->getRootPathCost() + portTable.at(port).linkCost;
	portTable.at(port).bridgePriority = bpdu->getBridgePriority();
	portTable.at(port).bridgeID = bpdu->getBridgeID();
	portTable.at(port).portPriority = bpdu->getPortPriority();
	portTable.at(port).portID = bpdu->getPortID();

	portTable.at(port).maxAge = bpdu->getMaxAge();
	portTable.at(port).fwdDelay = bpdu->getFwdDelay();
	portTable.at(port).helloTime = bpdu->getHelloTime();


	resetAge(port); // renew info

}

void stpi::generator() {
	if (!isRoot) {
		return;
	}
	for (unsigned int i = 0; i < portCount; i++) {
		generateHelloBPDU(i);
	}
	if (topologyChangeRecvd) { // TCA with BPDUs
		topologyChangeRecvd = false;
	}
	if (topologyChange > 0) {
		addrTable->enableFasterAging();
		topologyChange--;
	} else {
		addrTable->resetAging();
	}
}


void stpi::handleTick() {
	/* BRIDGE TIMERS */
	if (isRoot) {
		helloTimer++;
	} else {
		helloTimer = 0;
	}

	for (unsigned int i = 0; i < portCount; i++) { // PORT TIMERS
		// DISABLED ports is not operational
		if (portTable.at(i).enabled == false) {
			continue;
		}

		// designated port's origins informations to LAN,
		// cannot aged out yourself
		if (portTable.at(i).role != RDESIGNATED) {
			portTable.at(i).age++;
		}
		if (portTable.at(i).role == RROOT || portTable.at(i).role == RDESIGNATED) {
			portTable.at(i).fdWhile++;
		}
	}

	checkTimers();
	checkParametersChange();

	generateTCN(); // if something to notify
}

void stpi::checkTimers() {

	/* Hello timer check */
	if (helloTimer >= cHelloTime) {
		generator();
		helloTimer = 0;
	}

	/* information age check */
	for (unsigned int i = 0; i < portCount; i++) {
		if (portTable.at(i).age >= cMaxAge) {
			if (portTable.at(i).role == RROOT) {
				portTable.at(i) = defaultPort;
				lostRoot();
			} else {
				portTable.at(i) = defaultPort;
				lostAlternate(i);
			}
		}
	}


	/* fdWhile timer */
	for (unsigned int i = 0; i < portCount; i++) {
		/* ROOT / DESIGNATED, can transition */
		if (portTable.at(i).role == RROOT || portTable.at(i).role == RDESIGNATED) {
			if (portTable.at(i).fdWhile >= cFwdDelay) {
				switch (portTable.at(i).state) {
				case SDISABLED:
					setPortState(i, SDISCARDING);
					resetFDWhile(i);
					break;
				case SDISCARDING:
					setPortState(i, SLEARNING);
					resetFDWhile(i);
					break;
				case SLEARNING:
					setPortState(i, SFORWARDING);
					resetFDWhile(i);
					break;
				default:
					resetFDWhile(i);
					break;
				}

			}
		} else {
			resetFDWhile(i);
			setPortState(i, SDISCARDING);
		}
	}

	/* TOPOLOGY CHANGE HANDLING */
	if (topologyChangeNotification == true) {
		if (isRoot == true) {
			topologyChange = 5;
			topologyChangeNotification = false;
		}
	}

}

void stpi::checkParametersChange() {
	if (isRoot) {
		cHelloTime = helloTime;
		cMaxAge = maxAge;
		cFwdDelay = fwdDelay;
	}
	if (ubridgePriority != bridgePriority) {
		ubridgePriority = bridgePriority;
		reset();
	}
}


void stpi::resetAge(int p) {
	portTable.at(p).age = 0;
}

void stpi::resetFDWhile(int p) {
	portTable.at(p).fdWhile = 0;
}

bool stpi::checkRootEligibility() {
	for (unsigned int i = 0; i < portCount; i++) {
		if (superiorID(portTable.at(i).rootPriority, portTable.at(i).rootID,
				bridgePriority, bridgeAddress) > 0) {
			/* port information  is superior to bridge ID, is not root eligible */
			return false;
		}
	}
	return true;
}

void stpi::tryRoot() {
	if (checkRootEligibility() == false) {
		isRoot = false;
//		EV << this->getParentModule()->getFullName() << ": not eligible to be root!" << std::endl;
		selectRootPort();
		selectDesignatedPorts();
	} else {

//		EV << this->getParentModule()->getFullName() << ": Iam root!" << std::endl;
		isRoot = true;
		allDesignated();
		rootPriority = bridgePriority;
		rootID = bridgeAddress;
		rootPathCost = 0;
		cHelloTime = helloTime;
		cMaxAge = maxAge;
		cFwdDelay = fwdDelay;
	}


}

int stpi::superiorID(unsigned int APR, MACAddress AID, unsigned int BPR, MACAddress BID) {
	if (APR < BPR) {
		return 1; // A is superior
	} else if (APR > BPR) {
		return -1;
	}

	// APR == BPR
	if (AID < BID) {
		return 1; // A is superior
	} else if (AID > BID) {
		return -1;
	}

	/* A==B
	 * (can happen if bridge have two port connected to one not bridged lan,
	 * "cable loopback")
	 */
	return 0;
}

int stpi::superiorPort(unsigned int APR, unsigned int AID, unsigned int BPR, unsigned int BID) {
	if (APR < BPR) {
		return 1; // A is superior
	} else if (APR > BPR) {
		return -1;
	}

	// APR == BPR
	if (AID < BID) {
		return 1; // A is superior
	} else if (AID > BID) {
		return -1;
	}

	/* A==B */
	return 0;
}



int stpi::superiorTPort(tPort A, tPort B) {
	int result;
	/* ROOT COMPARSION */
	result = superiorID(A.rootPriority, A.rootID,
			            B.rootPriority, B.rootID);
	if (result != 0){ // not same, so pass result
		return result;
	}

	/* PATH COST */
	if (A.rootPathCost < B.rootPathCost) {
		return 1;
	}
	if (A.rootPathCost > B.rootPathCost) {
		return -1;
	}

	/* DESIGNATED BRIDGE */
	result = superiorID(A.bridgePriority, A.bridgeID,
			            B.bridgePriority, B.bridgeID);
	if (result != 0){ // not same, so pass result
		return result;
	}


	/* DESIGNATED PORT OF DESIGNATED BRIDGE*/
	result = superiorPort(A.portPriority, A.portID,
						 B.portPriority, B.portID);
	if (result != 0){ // not same, so pass result
		return result;
	}

	return 0; // same
}

void stpi::selectRootPort() {
	unsigned int xRootPort = 0;
	int result;
	tPort best = defaultPort; // can be superseded by all real tPort structures
	tPort tmp;

	for (unsigned int i = 0; i < portCount; i++) {
		tmp = portTable.at(i);
		portTable.at(i).role = RUNKNOWN;
		result = superiorTPort(tmp, best);
		if (result > 0) { // new root port
			xRootPort = i;
			best = tmp;
			continue;
		}
		if (result < 0) { // inferior information
			continue;
		}
		/* same info on tPort */
		if (tmp.priority < best.priority) { // better priority
			xRootPort = i;
			best = tmp;
			continue;
		}
		if (tmp.priority > best.priority) { // inferior priority
			continue;
		}
		/* ALL IS THE SAME, index of port resolving stalemate,
		 * because of for cycle is the existing xRootPort lower
		 * so it' remains */
		continue;

	}
	if (rootPort != xRootPort) {
		topologyChangeNotification = true;
	}

	rootPort = xRootPort;
	portTable.at(rootPort).role = RROOT;

	rootPathCost = best.rootPathCost;
	rootID = best.rootID;
	rootPriority = best.rootPriority;

	cMaxAge = best.maxAge;
	cFwdDelay = best.fwdDelay;
	cHelloTime = best.helloTime;

}

/* select designated ports */
void stpi::selectDesignatedPorts() {
/* designated is the best with same Root, OR inferior root */
	std::vector<unsigned int> xDesPorts;
	tPort tmp;
	tPort bridge; // current bridge information
	int result;

	bridge.bridgePriority = bridgePriority;
	bridge.bridgeID = bridgeAddress;
	bridge.rootID = rootID;
	bridge.rootPriority = rootPriority;



	for (unsigned int i = 0; i < portCount; i++) {
		tmp = portTable.at(i);

		if (tmp.role == RROOT) {
			continue;
		}
		if (tmp.enabled == false) {
			continue;
		}

		bridge.portPriority = portTable.at(i).priority;
		bridge.portID = i;

		bridge.rootPathCost = rootPathCost + portTable.at(i).linkCost;

		if (superiorID(rootPriority, rootID, tmp.rootPriority, tmp.rootID) < 0) {
	//		error("cannot be superior Root on port !");
		}
	//	EV << this->getFullName() << "I:" << bridge << " ? " << tmp << std::endl;
		result = superiorTPort(bridge, tmp);
		if (result > 0) {
			xDesPorts.push_back(i);
			portTable.at(i).role = RDESIGNATED;
			continue;
		}
		if (result < 0) {
			portTable.at(i).role = RALTERNATE;
			continue;
		}
	//	error("cannot be the same !");

	}

	desPorts.clear();
	desPorts = xDesPorts;

}

void stpi::allDesignated() {
	std::vector<unsigned int> xDesPorts;
	for (unsigned int i = 0; i < portCount; i++) {
		if (portTable.at(i).enabled == false) {
			continue;
		}
		xDesPorts.push_back(i);
		portTable.at(i).role = RDESIGNATED;
	}
	desPorts.clear();
	desPorts = xDesPorts;
}


/* neighbor lost handling */
void stpi::lostRoot() {
	topologyChangeNotification = true; // information for bridge, that topology change is commited
//	EV << this->getParentModule()->getFullName() << ": ROOT IS DOWN!" << std::endl;
	tryRoot();

}

void stpi::lostAlternate(int port) {
	selectDesignatedPorts();
	topologyChangeNotification = true;
}

void stpi::reset() {
	isRoot = true;
	rootPriority = bridgePriority;
	rootID = bridgeAddress;
	rootPathCost = 0;
	cHelloTime = helloTime;
	cMaxAge = maxAge;
	cFwdDelay = fwdDelay;
	allDesignated();

	for (unsigned int i = 0; i < portCount; i++) {
		portTable.at(i) = defaultPort;
	}
}

void stpi::setPortState(unsigned int i, tPortState state) {
	portTable.at(i).state = state;

	switch (state) {
	case SDISABLED:
	case SDISCARDING:
		portTable.at(i).flags.Learning = false;
		portTable.at(i).flags.Forwarding = false;
		break;
	case SLEARNING:
		portTable.at(i).flags.Learning = true;
		portTable.at(i).flags.Forwarding = false;
		break;
	case SFORWARDING:
		portTable.at(i).flags.Learning = true;
		portTable.at(i).flags.Forwarding = true;
		break;
	default:
		break;
	}

}

