
#ifndef HLIDAC_PIMDM
#define HLIDAC_PIMDM

#include <omnetpp.h>
#include "PIMPacket_m.h"
#include "PIMTimer_m.h"


class pimSM : public cSimpleModule
{
	protected:
		virtual int numInitStages() const  {return 5;}
		virtual void handleMessage(cMessage *msg);
		virtual void initialize(int stage);
};

#endif
