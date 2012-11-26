/**
 * @file PimInterfaceTable.cc
 * @date 19.3.2012
 * @author: Veronika Rybova
 * @brief File implements table of PIM interfaces.
 * @details PIM interface table contains information about all interfaces which are configured by
 *  PIM protocol. Information are obtained from configuration file.
 */

#include "PimInterfaceTable.h"

Define_Module(PimInterfaceTable);

/** Printout of structure PimInterface. */
std::ostream& operator<<(std::ostream& os, const PimInterface& e)
{
	int i;
	std::vector<IPAddress> intMulticastAddresses = e.getIntMulticastAddresses();

    os << "ID = " << e.getInterfaceID() << "; mode = ";
    if (e.getMode() == Dense)
    	os << "Dense";
    else if (e.getMode() == Sparse)
    	os << "Sparse";
    os << "; Multicast addresses: ";

    int vel = intMulticastAddresses.size();
    if (vel > 0)
    {
		for(i = 0; i < (vel - 1); i++)
			os << intMulticastAddresses[i] << ", ";
		os << intMulticastAddresses[i];
    }
    else
    	os << "Null";
    return os;
};


/** Printout of structure PimInterfaces Table. */
std::ostream& operator<<(std::ostream& os, const PimInterfaceTable& e)
{
    for (int i = 0; i < e.size(); i++)
    	os << "";
		//os << "ID = " << e.getInterface(i)->getInterfaceID() << "; mode = " << e.getInterface(i)->getMode();
    return os;
};

/** Actually not in use */
std::string PimInterface::info() const
{
	std::stringstream out;
	out << "ID = " << intID << "; mode = " << mode;
	return out.str();
}

/**
 * REMOVE INTERFACE MULTICAST ADDRESS
 *
 * The method removes given address from vector of multicast addresses.
 *
 * @param addr IP address which should be deleted.
 */
void PimInterface::removeIntMulticastAddress(IPAddress addr)
{
	for(unsigned int i = 0; i < intMulticastAddresses.size(); i++)
	{
		if (intMulticastAddresses[i] == addr)
		{
			intMulticastAddresses.erase(intMulticastAddresses.begin() + i);
			return;
		}
	}
}

/**
 * DELETE LOCAL IPs
 *
 * The method removes all link local (224.0.0.0 to 224.0.0.255) multicast addresses
 * from the list.
 *
 * @param multicastAddr List of address which has to be checked.
 * @return List of multicast address without link local IPs.
 * @see isLinkLocalMulticast()
 */
std::vector<IPAddress> PimInterface::deleteLocalIPs(std::vector<IPAddress> multicastAddr)
{
	EV << "PimInterface::deleteLocalIPs" << endl;

	for(int j = 0; j < (multicastAddr.size()); j++)
	    	EV << multicastAddr[j] << ", ";
	EV << endl;

	std::vector<IPAddress> newMulticastAddresses;
	for(unsigned int i = 0; i < multicastAddr.size(); i++)
	{
		EV << multicastAddr[i] << endl;
		if (!multicastAddr[i].isLinkLocalMulticast())
		{
			EV << "isLinkLocalMulticast" << endl;
			newMulticastAddresses.push_back(multicastAddr[i]);
		}
	}
	EV << "Velikost vysledku: " << newMulticastAddresses.size() << endl;
	return newMulticastAddresses;
}

/**
 * IS LOCAL INETRFACE MULTICAST ADDRESS
 *
 * The method finds out if IP address is assigned to interface as local multicast address.
 *
 * @param addr Multicast IP address which we are looking for.
 * @return True if method finds the IP address on the list, return false otherwise.
 */
bool PimInterface::isLocalIntMulticastAddress (IPAddress addr)
{
	for(unsigned int i = 0; i < intMulticastAddresses.size(); i++)
	{
		if (intMulticastAddresses[i] == addr)
			return true;
	}
	return false;
}



/**
 * HANDLE MESSAGE
 *
 * Module does not have any gate, it cannot get messages
 */
void PimInterfaceTable::handleMessage(cMessage *msg)
{
    opp_error("This module doesn't process messages");
}

void PimInterfaceTable::initialize(int stage)
{
		WATCH_VECTOR(pimIft);
}

/**
 * PRINT PIM INTERFACES
 *
 * Actually not in use.
 * Printout of Table of PIM interfaces
 */
void PimInterfaceTable::printPimInterfaces()
{
	for(std::vector<PimInterface>::iterator i = pimIft.begin(); i < pimIft.end(); i++)
	{
		EV << (*i).info() << endl;
	}

}

/**
 * GET INTERFACE BY INTERFACE ID
 *
 * The method finds interface in interface table by given interface ID.
 *
 * @param intID ID of interface which is wanted.
 * @return Returns link to wanted record in table.
 * @see getNumInterface()
 * @see getInterface()
 */
PimInterface *PimInterfaceTable::getInterfaceByIntID(int intID)
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
