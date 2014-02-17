
#include "vlanTable.h"

#include "vlanTableXMLparser.h"



Define_Module(VLANTable);

VLANTable::VLANTable() {
  return;
}
VLANTable::~VLANTable() {
  return;
}

/* --- PUBLIC --- */

/* MGMT */
const VLANTable::VIDTable * VLANTable::getTaggedTable() {
	return &vidTable;
}
const VLANTable::PortVIDTable * VLANTable::getUntaggedTable(){
	return &portVIDTable;
}

void VLANTable::initDefault() {
	regVLAN(1);
	extendTable(1);
	for (unsigned int i = 0; i < (unsigned int)portCount; i++) {
		addPortVID(i,1);
	}
}

/* PUBLIC ACCESS METHODS */
VLANTable::tVIDPortList VLANTable::getPorts(int VID) {
	if (vidTable.size() < VID+1) {
		return empty;
	}
	if (vidTable.at(VID).VID != VID) {// VLAN is not active
		return empty;
	}
	return vidTable.at(VID).portList;
}

int VLANTable::getVID(int Port) {
	if (Port < 0) {
		error("negative port number");
		simulation.endRun();
	}
	if (Port > portCount) {
		error("Portnumber is exceeds port count in getVID");
		simulation.endRun();
	}
	return portVIDTable.at(Port).VID;
}

bool VLANTable::isAllowed(int VID, int _port) {
	if (vidTable.size() < VID+1) {
		return false;
	}
	tVIDRecord tmp = vidTable.at(VID);
	if (tmp.VID != VID) { // VLAN is not active
		return false;
	}

	for (unsigned int i = 0; i < tmp.portList.size(); i++) {
		if (tmp.portList.at(i).port == _port) {
			return true;
		}
	}

	return false;
}

VLANTable::tTagAction VLANTable::getTag(int VID, int _port) {
	tVIDRecord tmp = vidTable.at(VID);
	if (tmp.VID != VID) { // VLAN is not active
		error("query for includeTag on non active vlan");
		simulation.endRun();
		return NONE;
	}

	for (unsigned int i = 0; i < tmp.portList.size(); i++) {
		if (tmp.portList.at(i).port == _port) {
			return tmp.portList.at(i).action;
		}
	}

	error("query for includeTag on non existing (VID, PORT)");
	simulation.endRun();
	return NONE;
}

/* --- PRIVATE --- */

void VLANTable::add(int VID, tVIDPortList& _portList) {
	vidTable.at(VID).VID = VID;
	vidTable.at(VID).portList = _portList;
}

void VLANTable::addTagged(int VID, std::vector<int>& ports) {
	tVIDPortList& currentList = vidTable.at(VID).portList;
	vidTable.at(VID).VID = VID;

	tVIDPort tmpPort;
	tmpPort.action = INCLUDE;

	unsigned int i;
	unsigned int c;
	for(i = 0; i < ports.size(); i++) { // for all input port number
		for(c = 0; c < currentList.size(); c++) { // search through whole port vector
			if (currentList.at(c).port == ports.at(i)) { // if port number match
				currentList.at(c).action = INCLUDE; // modify action
				break;
			}
		}
		if (c == currentList.size()) { // if port record not found
			tmpPort.port = ports.at(i); // set temporary port number
			currentList.push_back(tmpPort); // insert new
		}

	}
}

void VLANTable::addUntagged(int VID, std::vector<int>& ports) {
	tVIDPortList& currentList = vidTable.at(VID).portList;
	vidTable.at(VID).VID = VID;

	tVIDPort tmpPort;
	tmpPort.action = REMOVE;

	unsigned int i;
	unsigned int c;
	for(i = 0; i < ports.size(); i++) { // for all input port number
		for(c = 0; c < currentList.size(); c++) { // search through whole port vector
			if (currentList.at(c).port == ports.at(i)) { // if port number match
				currentList.at(c).action = REMOVE; // modify action
				break;
			}
		}
		if (c == currentList.size()) { // if port record not found
			tmpPort.port = ports.at(i); // set temporary port number
			currentList.push_back(tmpPort); // insert new
		}

	}
}

void VLANTable::setVLANName(int _VID, std::string& _name) {
	vidTable.at(_VID).name = _name;
}

void VLANTable::addPortVID(int _port, int _VID) {
	portVIDTable.at(_port).port = _port;
	portVIDTable.at(_port).VID = _VID;

	std::vector<int> tmp;
	tmp.push_back(_port);
	extendTable(_VID);
	addUntagged(_VID, tmp);

}

void VLANTable::setPortVID(int _port, int _VID) {
	portVIDTable.at(_port).port = _port;
	portVIDTable.at(_port).VID = _VID;
}
/* Delete VLAN-ID from list of existing VLANs */
void VLANTable::delPort(int _port, int _VID) {
	tVIDPortList::iterator it;
	for (it = vidTable.at(_VID).portList.begin(); it != vidTable.at(_VID).portList.end(); it++) {
		if (it->port == _port) {
			vidTable.at(_VID).portList.erase(it);
			return;
		}
	}
}


/* --- PROTECTED --- */

void VLANTable::initialize(int stage) {
	if (stage == 0) {
		portCount = par("portCount"); //TODO from CFG/simulation

		emptyVID.VID = 0;

		tPortVIDRecord empty2;
		empty2.port = 0;
		empty2.VID = 0;

		//vidTable.insert(vidTable.begin(), VLANCOUNT, empty);
		portVIDTable.insert(portVIDTable.begin(), portCount, empty2);



		const char *filename = par("configFile");
		const char *switchID = par("switchID");

		if (*filename == '\0' || *switchID == '\0') {
			EV << "Warning: " << this->getParentModule()->getName() << ": Could not config, config filename or switchID is not set, using Default." << std::endl;
			initDefault();
		} else {
			VLANTableXMLparser config(this);
			config.parse(filename, switchID);
		}

		WATCH(portCount);
		WATCH_VECTOR(vidTable);
		WATCH_VECTOR(portVIDTable);
	}

}

void VLANTable::finish() {

}
/* Register specified VLAN. If such VLAN-ID is in the list, do nothing */
void VLANTable::regVLAN(unsigned int _vlan) {
	for (unsigned int i = 0; i < vlanList.size(); i++) {
		if (vlanList.at(i) == _vlan) {
			return;
		}
	}
	vlanList.push_back(_vlan);
}
std::vector<unsigned int> VLANTable::getVLANList() {
	return vlanList;
}

void VLANTable::extendTable(int VLAN) {

	EV <<"KECYYYYYY !!!!!!!!!!!!!!!!!!" << "\n";
	while (vidTable.size() < (unsigned int) VLAN+2) { // +1 is for index compensation
		vidTable.push_back(emptyVID);

	}
	EV << VLAN << "/" << vidTable.size() << "\n";
}


