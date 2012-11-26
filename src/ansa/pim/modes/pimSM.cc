/**
 * @file pimSM.cc
 * @date 29.10.2011
 * @author: Veronika Rybova
 * @brief File implements PIM sparse mode.
 * @details Implementation will be done in the future according to RFC4601.
 */

#include "pimSM.h"


Define_Module(pimSM);

void pimSM::handleMessage(cMessage *msg)
{
	EV << "PIMSM::handleMessage" << endl;

	// self message (timer)
	if (msg->isSelfMessage())
	{
	   EV << "PIMSM::handleMessage:Timer" << endl;
	   PIMTimer *timer = check_and_cast <PIMTimer *> (msg);
	}
	else if (dynamic_cast<PIMPacket *>(msg))
	{
	   EV << "PIMSM::handleMessage: PIM-SM packet" << endl;
	   PIMPacket *pkt = check_and_cast<PIMPacket *>(msg);
	   EV << "Verze: " << pkt->getVersion() << ", typ: " << pkt->getType() << endl;
	}
	else
	   EV << "PIMSM::handleMessage: Wrong message" << endl;
}

void pimSM::initialize(int stage)
{
	;
}

