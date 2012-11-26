/*
 * pim.cc
 *
 *  Created on: 3.12.2011
 *      Author: Haczek
 */

#include "pimSplitter.h"

using namespace std;

Define_Module(pimSplitter);

void pimSplitter::handleMessage(cMessage *msg)
{
	EV << "PIM::handleMessage" << endl;

	// vlastni zprava, tj. nektery z casovacu
   if (msg->isSelfMessage())
   {
	   PIMTimer *timer = check_and_cast <PIMTimer *> (msg);
   }
   else if (dynamic_cast<PIMPacket *>(msg))
   {
	   PIMPacket *pkt = check_and_cast<PIMPacket *>(msg);
	   EV << "Verze: " << pkt->getVersion() << ", typ: " << pkt->getType() << endl;
   }
   else
	   EV << "PIM: spatna zprava" << endl;
}

void pimSplitter::initialize(int stage)
{
	// v stage 2 se registruji interfacy
	if (stage == 3)
	{
		EV << "PIM::sendPacket" << endl;

		rt = RoutingTableAccess().get();
		ift = InterfaceTableAccess().get();

		EV << "Posilam zpravu na adresu: 224.0.0.13" << endl;

		PIMPacket *msg = new PIMPacket();
		msg->setVersion(2);
		msg->setType(Hello);
		msg->setName("PIMmessage");

		int intID;
		IInterfaceTable *ift;
		ift = InterfaceTableAccess().get();
		for (int i = 0; i < ift->getNumInterfaces(); i++)
		{
			if (!ift->getInterface(i)->isLoopback())
			{
				intID = ift->getInterface(i)->getInterfaceId();
				break;
			}
		}

		IPControlInfo *ctrl = new IPControlInfo();
		IPAddress ga1("224.0.0.13");
		ctrl->setDestAddr(ga1);
		ctrl->setProtocol(IP_PROT_PIM);
		ctrl->setTimeToLive(1);
		ctrl->setInterfaceId(intID);
		msg->setControlInfo(ctrl);

		send(msg, "transportOut");
	}
}
