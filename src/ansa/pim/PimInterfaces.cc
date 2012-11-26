/*
 * PimInterfaces.cc
 *
 *  Created on: 19.3.2012
 *      Author: Haczek
 */

#include "PimInterfaces.h"

Define_Module(PimInterfaces);

/** Printout of structure PimInterface. */
std::ostream& operator<<(std::ostream& os, const PimInterface& e)
{
    os << "ID = " << e.getInterfaceID() << "; mode = " << e.getMode();
    return os;
};



/** Printout of structure PimInterfaces Table. */
std::ostream& operator<<(std::ostream& os, const PimInterfaces& e)
{
    for (int i = 0; i < e.size(); i++)
    	os << "";
		//os << "ID = " << e.getInterface(i)->getInterfaceID() << "; mode = " << e.getInterface(i)->getMode();
    return os;
};

std::string PimInterface::info() const
{
	std::stringstream out;
	out << "ID = " << intID << "; mode = " << mode;
	return out.str();
}

/** Module does not have any gate, it cannot get messages */
void PimInterfaces::handleMessage(cMessage *msg)
{
    opp_error("This module doesn't process messages");
}

void PimInterfaces::initialize(int stage)
{
	WATCH_VECTOR(pimIft);
}

/** Printout of Table of PIM interfaces */
void PimInterfaces::printPimInterfaces()
{
	for(std::vector<PimInterface>::iterator i = pimIft.begin(); i < pimIft.end(); i++)
	{
		EV << (*i).info() << endl;
	}

}

PimInterface *PimInterfaces::getInterfaceByIntID(int intID)
{
	for(int i = 0; i < getNumInterface(); i++)
	{
		if(intID == getInterface(i)->getInterfaceID())
		{
			return getInterface(i);
			break;
		}
	}
	return NULL;
}
