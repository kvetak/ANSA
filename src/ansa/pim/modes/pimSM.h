/**
 * @file pimSM.h
 * @date 29.10.2011
 * @author: Veronika Rybova
 * @brief File implements PIM sparse mode.
 * @details Implementation will be done in the future according to RFC4601.
 */

#ifndef HLIDAC_PIMDM
#define HLIDAC_PIMDM

#include <omnetpp.h>
#include "PIMPacket_m.h"
#include "PIMTimer_m.h"

/**
 * @brief Class implements PIM-SM (sparse mode).
 */
class pimSM : public cSimpleModule
{
	protected:
		virtual int numInitStages() const  {return 5;}
		virtual void handleMessage(cMessage *msg);
		virtual void initialize(int stage);
};

#endif
