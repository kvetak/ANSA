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

#include "portTable.h"

Define_Module(PortTable);


/* PUBLIC ACCESS */

PortTable::tPort PortTable::getPort(int port) {
	Enter_Method_Silent();
	if (port < 0 || port >= portCount) {
		error("PortTable: exceeded portTable limit in getPort() / getState()");
	}

	return list.at(port);

}
PortTable::tPortState PortTable::getState(int port) {
	Enter_Method_Silent();
	return getPort(port).state;
}

/* STP PROCESS MANIPULATION */
void PortTable::setState(int port, PortTable::tPortState state) {
	Enter_Method_Silent();
	if (port < 0 || port >= portCount) {
		error("PortTable: exceeded portTable limit in setState()");
	}
	list.at(port).state = state;

	return;
}

void PortTable::setState(std::vector<int>& pList, PortTable::tPortState state) {
	Enter_Method_Silent();
	for (unsigned int i = 0; i < pList.size(); i++) {
		setState(pList.at(i), state);
	}
	return;
}

void PortTable::allState(PortTable::tPortState state) {
	Enter_Method_Silent();
	for (unsigned int i = 0; i < (unsigned int)portCount; i++) {
		setState(i, state);
	}
	return;
}

void PortTable::allOff() {
	Enter_Method_Silent();
	allState(OFF);
	return;
}

void PortTable::off(std::vector<int>& pList) {
	Enter_Method_Silent();
		setState(pList, OFF);

	return;
}

void PortTable::forwarding(std::vector<int>& pList) {
	Enter_Method_Silent();
		setState(pList, FORWARDING);

	return;
}


/* -- private -- */

void PortTable::initDefault() {
	tPort tmp;
	tmp.state = FORWARDING;
	list.insert(list.begin(), portCount, tmp);
	return;
}

/* -- protected --*/

void PortTable::initialize()
{
	portCount = par("portCount");

	initDefault();

	WATCH_VECTOR(list);
	return;
}

void PortTable::handleMessage(cMessage *msg)
{

}

