/**
 * @file MulticastIPRoute.cc
 * @brief File contains implementation of multicast route.
 * @author Veronika Rybova
 * @date 10.10.2011
 */

#include "MulticastIPRoute.h"

using namespace std;

MulticastIPRoute::MulticastIPRoute()
{
	inInt.intPtr = NULL;
	grt = NULL;
	sat = NULL;
	srt = NULL;
}

string MulticastIPRoute::info() const
{
    stringstream out;
    out << "(";
    if (source.isUnspecified()) out << "*  "; else out << source << ",  ";
    if (group.isUnspecified()) out << "*  "; else out << group << "),  ";
    if (RP.isUnspecified()) out << "0.0.0.0"<< endl; else out << "RP is " << RP << endl;
    out << "Incoming interface: ";
    if(inInt.intPtr != NULL)
    {
		if (inInt.intPtr) out << inInt.intPtr->getName() << ", ";
		out << "RPF neighbor " << inInt.nextHop << endl;
		out << "Outgoing interface list:" << endl;
    }

    for (InterfaceVector::const_iterator i = outInt.begin(); i < outInt.end(); i++)
    {
    	if ((*i).intPtr) out << (*i).intPtr->getName() << ", ";
    	if (i->forwarding == Forward) out << "Forward/"; else out << "Pruned/";
    	if (i->mode == Densemode) out << "Dense"; else out << "Sparse";
    	out << endl;
    }

    return out.str();
}

string MulticastIPRoute::detailedInfo() const
{
    return string();
}


bool MulticastIPRoute::isFlagSet(flag fl)
{
	for(unsigned int i = 0; i < flags.size(); i++)
	{
		if (flags[i] == fl)
			return true;
	}
	return false;
}

void MulticastIPRoute::addFlag(flag fl)
{
	if (!isFlagSet(fl))
		flags.push_back(fl);
}

void MulticastIPRoute::removeFlag(flag fl)
{
	for(unsigned int i = 0; i < flags.size(); i++)
	{
		if (flags[i] == fl)
		{
			flags.erase(flags.begin() + i);
			return;
		}
	}
}

int MulticastIPRoute::getOutIdByIntId(int intId)
{
	unsigned int i;
	for (i = 0; i < outInt.size(); i++)
	{
		if (outInt[i].intId == intId)
			break;
	}
	return i;
}

bool MulticastIPRoute::isOilistNull()
{
	bool olistNull = true;
	for (unsigned int i = 0; i < outInt.size(); i++)
	{
		if (outInt[i].forwarding == Forward)
		{
			olistNull = false;
			break;
		}
	}
	return olistNull;
}

