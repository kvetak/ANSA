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

#include "linkBreaker.h"

Define_Module(LinkBreaker);

void LinkBreaker::initialize() {
	breakEnabled = false;
	WATCH_RW(breakEnabled);

}

void LinkBreaker::handleMessage(cMessage *msg) {

	if (breakEnabled) {
		getDisplayString().parse("i=block/control,red;is=vs");
		delete msg;
		return;
	} else {
		getDisplayString().parse("i=block/control;is=vs");
	}

    if (strcmp(msg->getArrivalGate()->getBaseName(), "A") == 0) {
    	send(msg, "B$o");
    } else {
    	send(msg, "A$o");
    }
}
