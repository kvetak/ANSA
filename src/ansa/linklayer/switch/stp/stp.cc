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

#include "stp.h"
#include "ansaSwitchCore.h"
#include "vlanTable.h"
#include "stpXMLparser.h"

Define_Module(Stp);

bool Stp::learning(unsigned int port, unsigned int vlan) {
	Enter_Method_Silent();
	for (unsigned int i = 0; i < instReg.size(); i++) {
		if (instReg.at(i) == vlan) {
			return inst.at(i).learning(port);
		}
	}
	return false;
}

bool Stp::forwarding(unsigned int port, unsigned int vlan) {
	Enter_Method_Silent();
	for (unsigned int i = 0; i < instReg.size(); i++) {
		if (instReg.at(i) == vlan) {
			return inst.at(i).forwarding(port);
		}
	}
	return false;
}

void Stp::dispatchALL() {
	for (unsigned int i = 0; i < inst.size(); i++) {
		dispatch(i);
	}
}

void Stp::dispatch(unsigned int idx) {
	for (unsigned int i = 0; i < inst.at(idx).msgList.size(); i++) {
		send(inst.at(idx).msgList.at(i).msg, "out", inst.at(idx).msgList.at(i).port);
	}
	inst.at(idx).msgList.clear();
}

/* ---- FOR XML CONFIGURATOR ------*/
int Stp::getInstanceIndex(unsigned int _inst) {
	for (unsigned int i = 0; i < instReg.size(); i++) {
			if (instReg.at(i) == _inst) {
				return i;
			}
		}
	return -1;
}

void Stp::setBridgePriority(unsigned int _inst, unsigned int _bridgePriority) {
	int idx = getInstanceIndex(_inst);
	if (idx == -1) {
		return;
	}

	inst.at(idx).bridgePriority = _bridgePriority;

}

/* internal for XML configuration */
void Stp::setPortPriority(unsigned int _idx, unsigned int _port, unsigned int _priority) {
	inst.at(_idx).portTable.at(_port).priority = _priority;
}
void Stp::setLinkCost(unsigned int _idx, unsigned int _port, unsigned int _cost) {
	inst.at(_idx).portTable.at(_port).linkCost = _cost;
}


void Stp::setPortPriority(unsigned int _inst, std::vector<unsigned int>& _portList, std::vector<unsigned int>& _priList) {
	int idx = getInstanceIndex(_inst);
	if (idx == -1) {
		return;
	}

	if (_portList.size() != _priList.size()) {
		return;
	}

	for (unsigned int i = 0; i < _portList.size(); i++) {
		setPortPriority(idx, _portList.at(i), _priList.at(i));
	}

}

void Stp::setLinkCost(unsigned int _inst, std::vector<unsigned int>& _portList, std::vector<unsigned int>& _costList) {
	int idx = getInstanceIndex(_inst);
	if (idx == -1) {
		return;
	}

	if (_portList.size() != _costList.size()) {
		return;
	}

	for (unsigned int i = 0; i < _portList.size(); i++) {
		setLinkCost(idx, _portList.at(i), _costList.at(i));
	}

}

void Stp::setForwardDelay(unsigned int _inst, unsigned int _fwdDelay) {
	int idx = getInstanceIndex(_inst);
	if (idx == -1) {
		return;
	}
	inst.at(idx).fwdDelay = _fwdDelay;

}

void Stp::setMaxAge(unsigned int _inst, unsigned int _maxAge) {
	int idx = getInstanceIndex(_inst);
	if (idx == -1) {
		return;
	}
	inst.at(idx).maxAge = _maxAge;
}

void Stp::setHelloTime(unsigned int _inst, unsigned int _helloTime){
	int idx = getInstanceIndex(_inst);
	if (idx == -1) {
		return;
	}
	inst.at(idx).helloTime = _helloTime;
}


/* ---- END FOR XML CONFIGURATOR ------*/

void Stp::initialize(int stage)
{
	if (stage == 0) {

		portCount = gateSize("out");

		tick = new cMessage("STP_TICK", 0);

		/* connection to MAC Table for faster aging */
		cModule * tmp_macTable = getParentModule()->getSubmodule("MACTable");
		addrTable = check_and_cast<MACTable *>(tmp_macTable);

		WATCH(bridgeAddress);
	}
	else if (stage == 1) {
		/* obtain a Bridge Address from Core and initialize instances*/
		cModule * tmp_core = getParentModule()->getSubmodule("core");
		ANSASwitchCore * core = check_and_cast<ANSASwitchCore *>(tmp_core);
		bridgeAddress = core->getBridgeAddress();

		/* obtain a VLAN list for STP instances creation */
		cModule * tmp_vlantable = getParentModule()->getSubmodule("VLANTable");
		VLANTable * vlantable = check_and_cast<VLANTable *>(tmp_vlantable);
		instReg = vlantable->getVLANList();


		/* call init on all instances */
		for (unsigned int i = 0; i < instReg.size(); i++) {
			stpi tmp(instReg.at(i), portCount, bridgeAddress, addrTable);
			inst.push_back(tmp);
		}

		const char *filename = par("configFile");
		const char *switchID = par("switchID");

		if (*filename == '\0' || *switchID == '\0') {
			EV << "Warning: " << this->getParentModule()->getName() << ": Could not config, config filename or switchID is not set, using Default." << std::endl;
		} else {
			stpXMLparser config(this);
			config.parse(filename, switchID);
		}



		WATCH_VECTOR(inst);
		WATCH_VECTOR(instReg);

		scheduleAt(simTime()+1, tick);
	}
}

void Stp::handleMessage(cMessage *msg)
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
	} else {
		handleSelfMessage(msg);
	}


}

void Stp::finish() {
	cancelAndDelete(tick);
}

void Stp::handleSelfMessage(cMessage * msg) {
	if (msg == tick) {
		for (unsigned int i = 0; i < inst.size(); i++) {
			inst.at(i).handleTick();
		}
		scheduleAt(simTime()+1, tick);
	} else {
		delete msg;
	}
	dispatchALL();
}

void Stp::handleBPDU(STPBPDU * bpdu) {
	unsigned int _vlan = bpdu->getVlan();

	for (unsigned int i = 0; i < instReg.size(); i++) {
		if (instReg.at(i) == _vlan) {
			inst.at(i).handleBPDU(bpdu);
			dispatch(i);
			return;
		}
	}
	delete bpdu;

}

void Stp::handleTCN(STPTCN * tcn) {
	unsigned int _vlan = tcn->getVlan();

	for (unsigned int i = 0; i < instReg.size(); i++) {
		if (instReg.at(i) == _vlan) {
			inst.at(i).handleTCN(tcn);
			dispatch(i);
			return;
		}
	}
	delete tcn;
}




