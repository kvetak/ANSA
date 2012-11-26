/*
 * PIM-DM.cc
 *
 *  Created on: 29.10.2011
 *      Author: Haczek
 */

#include "pimDM.h"


Define_Module(pimDM);

void pimDM::handleMessage(cMessage *msg)
{
	EV << "PIMDM::handleMessage" << endl;

	// self message (timer)
   if (msg->isSelfMessage())
   {
	   EV << "PIMDM::handleMessage:Timer" << endl;
	   PIMTimer *timer = check_and_cast <PIMTimer *> (msg);
   }
   else if (dynamic_cast<PIMPacket *>(msg))
   {
	   EV << "PIMDM::handleMessage: PIM-DM packet" << endl;
	   PIMPacket *pkt = check_and_cast<PIMPacket *>(msg);
	   EV << "Verze: " << pkt->getVersion() << ", typ: " << pkt->getType() << endl;
   }
   else
	   EV << "PIMDM::handleMessage: Wrong message" << endl;
}

void pimDM::initialize(int stage)
{
	;
}

